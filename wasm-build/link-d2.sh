#!/bin/bash
# Canonical d2-web -> wasm build + link. Runs INSIDE d2-builder64 (Dockerfile.builder64):
#   docker run --rm -v "$PWD:/build" d2-builder64:1 bash wasm-build/link-d2.sh
# Produces build-wasm/d2.{js,wasm} and copies them into play/ (+ .br siblings). Stage into
# play/e/<hash>/ with version-engine.sh afterwards; ci/build.sh does all three.
#
# Ported shape of openmw-web/wasm-build/link-openmw.sh, none of the content: no CMake tree of
# our own yet (a handful of sources -- em++ directly), StormLib built once via its CMake and
# cached in build-wasm/stormlib/ keyed on the submodule commit.
set -euo pipefail

ROOT="${ROOT:-$(cd "$(dirname "$0")/.." && pwd)}"
BUILD="$ROOT/build-wasm"
STORM_SRC="$ROOT/third_party/StormLib"
STORM_BUILD="$BUILD/stormlib-wasm64"
mkdir -p "$BUILD"
cd "$ROOT"
source "$ROOT/tools/compiler-cache.sh"
TOOLCHAIN_KEY=$(d2_toolchain_key)
d2_enable_compiler_cache

# ci/build.sh runs the container as root so mixed Git-Bash/WSL ownership cannot block CMake.
# Restore only generated outputs to the invoking user even when compilation exits early.
restore_output_owner() {
  [ -n "${D2_OUTPUT_OWNER:-}" ] || return 0
  # $BUILD/obj, not just $OBJDIR: a SIGKILLed container leaves a root-owned per-hash obj dir
  # that plain rm cannot remove, so `worktree-prune` freed 0 MB over 48 G of worktrees
  #. Sweeping the whole obj tree recovers it on the next build.
  targets=("$STORM_BUILD" "$BUILD/obj")
  [ -n "${OBJDIR:-}" ] && targets+=("$OBJDIR")
  for f in "$BUILD/d2.js" "$BUILD/d2.wasm" "$ROOT/play/d2.js" "$ROOT/play/d2.wasm"; do
    [ -e "$f" ] && targets+=("$f")
  done
  chown -R "$D2_OUTPUT_OWNER" "${targets[@]}" 2>/dev/null || true
}
trap restore_output_owner EXIT
restore_output_owner   # recover obj/ a previous (killed) build left root-owned, before compiling into it

[ -f "$STORM_SRC/CMakeLists.txt" ] || { echo "FATAL: third_party/StormLib is empty -- git submodule update --init" >&2; exit 1; }

# --- StormLib (once per submodule commit) --------------------------------------------------
# Bundled zlib/bzip2/lzma so the link line does not depend on which emscripten ports exist in
# the sysroot. Static, -pthread to match everything else (an archive compiled without -pthread
# links fine and then faults at runtime on the first atomic -- the bad failure mode).
storm_rev=$(git -c safe.directory="$STORM_SRC" -C "$STORM_SRC" rev-parse HEAD 2>/dev/null || echo unknown)
# Include dirty/untracked bundled source and headers as well as the submodule revision.
storm_inputs=$(find "$STORM_SRC" -type f ! -name .git -print0 | sort -z | xargs -0 sha256sum | sha256sum | cut -d' ' -f1)
storm_rev=$(printf '%s\n' "$storm_rev" "$storm_inputs" "$TOOLCHAIN_KEY" | sha256sum | cut -d' ' -f1)
if [ ! -f "$STORM_BUILD/libstorm.a" ] || [ "$(cat "$STORM_BUILD/.rev" 2>/dev/null)" != "$storm_rev" ]; then
  echo "==> StormLib @ $storm_rev"
  # Clear the contents, not the directory itself. On Windows/WSL Docker bind mounts the output
  # directory can be writable even when its parent retains a different SID-derived owner.
  mkdir -p "$STORM_BUILD"
  find "$STORM_BUILD" -mindepth 1 -maxdepth 1 -exec rm -rf {} +
  emcmake cmake -S "$STORM_SRC" -B "$STORM_BUILD" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release -DSTORM_USE_BUNDLED_LIBRARIES=ON -DSTORM_SKIP_INSTALL=ON \
    -DBUILD_SHARED_LIBS=OFF -DCMAKE_C_FLAGS="-m64 -pthread -O2" -DCMAKE_CXX_FLAGS="-m64 -pthread -O2" >/dev/null
  ninja -C "$STORM_BUILD" storm
  echo "$storm_rev" > "$STORM_BUILD/.rev"
fi
[ -f "$STORM_BUILD/libstorm.a" ] || { echo "FATAL: libstorm.a did not build" >&2; exit 1; }

# --- the engine ---------------------------------------------------------------------------
# -fno-strict-aliasing: recovered code punning structs is the norm, not the exception.
# -fwrapv: signed overflow wraps, like the x86-32 MSVC build the logic came from.
# No -flto (openmw-web: wasm-ld miscompiles under it). `-m64` selects full end-to-end Memory64.
. "$(dirname "$0")/probe-skip.sh"
SRCS=$(find src -name '*.cpp' ! -name '*.test.cpp' | sort)
# tools/vectorfmt is shared with the Win32 capture tool: same header, same hash, no drift.
echo "==> em++ $(echo $SRCS | wc -w) sources"
# src/core/d2moo is vendored D2MOO (MIT, 1.10f prose map) compiled as 1.13c through the
# shims in src/core/d2moo/compat; its Windows-isms are silenced, not fixed.
# D2_CXXFLAGS="-g -O1 -sASSERTIONS=2 -sSAFE_HEAP=1" turns a "memory access out of bounds" into a stack.
CXXFLAGS="-m64 -std=c++20 -O2 -pthread -fno-strict-aliasing -fwrapv ${D2_CXXFLAGS:-} \
  -Wno-unknown-pragmas -Wno-microsoft -Wno-unused-value \
  -Wno-invalid-offsetof -Wno-deprecated-volatile -Wno-writable-strings \
  -DD2_VERSION_MAJOR=1 -DD2_VERSION_MINOR=13 -DD2_VERSION_PATCH=67 \
  -I$STORM_SRC/src -Isrc -Isrc/core/d2moo/compat -Isrc/core/d2moo/include -Isrc/core/d2moo/include/D2Game -Isrc/core/d2moo/squall"
# Objects are cached per source under build-wasm/obj/<flags-hash>/ (the vendored tree is ~200k
# lines; a full rebuild is minutes, a one-file change is seconds). Each compile emits a depfile
# and a content digest for the complete user-header closure. Content, rather than mtimes, matters:
# GitHub checkout gives every source a fresh timestamp when restoring older cached objects.
# The dir is named by what a compile depends on: the flags, the compiler and the compile recipe (probe-skip.sh).
# Not by this file: adding an export or changing the link line leaves every object valid.
OBJDIR="$BUILD/obj/$(printf '%s\n' "$CXXFLAGS" "$TOOLCHAIN_KEY" "$(sha256sum < "$(dirname "$0")/probe-skip.sh")" | sha256sum | cut -c1-16)"; mkdir -p "$OBJDIR"
. "$(dirname "$0")/obj-seed.sh"; d2_obj_seed "/d2-obj-seed/$(basename "$OBJDIR")" "$OBJDIR"
dependency_digest() {
  # Read only the depfile's first rule; -MP appends header-only phony rules afterwards.
  awk 'NR == 1 { sub(/^[^:]*:/, "") }
       { more = ($0 ~ /\\$/); sub(/\\$/, ""); print; if (!more) exit }' "$1" |
    tr '\n' ' ' | xargs sha256sum | sha256sum | cut -d' ' -f1
}
OBJS=""
STALE=()
for f in $SRCS; do
  o="$OBJDIR/$(echo "$f" | tr '/' '_' | sed 's/\.cpp$/.o/')"
  d="$o.d"
  digest="$o.inputs"
  current=""
  if [ -f "$o" ] && [ -f "$d" ] && [ -f "$digest" ]; then
    # Paths in this project contain no whitespace. Strip Make continuations and the target,
    # then hash the contents of every dependency. A missing dependency also forces a rebuild.
    current=$(dependency_digest "$d" 2>/dev/null || true)
  fi
  if [ -z "$current" ] || [ "$current" != "$(cat "$digest" 2>/dev/null)" ]; then
    STALE+=("$f|$o")
  fi
  OBJS="$OBJS $o"
done
# Stale objects compile in parallel (D2_COMPILE_JOBS,
# default half the cores (two builds can run at once), at least 4. A failure stops the build.
if [ "${#STALE[@]}" -gt 0 ]; then
  JOBS="${D2_COMPILE_JOBS:-$(( $(nproc) / 2 > 4 ? $(nproc) / 2 : 4 ))}"
  echo "==> compiling ${#STALE[@]} stale objects, $JOBS at a time"
  export -f compile_one dependency_digest probe_skip_line; export CXXFLAGS
  printf '%s\0' "${STALE[@]}" | xargs -0 -n1 -P "$JOBS" bash -c 'compile_one "$1"' _ || exit 1
fi
# A probe that failed to compile left no object (its PROBE-SKIP line is above); drop every missing object.
SKIPPED_FILE="$BUILD/probe-skip.txt"; : > "$SKIPPED_FILE"; chmod 666 "$SKIPPED_FILE" 2>/dev/null || true   # the container runs as root; host-side batch proves rewrite it
KEEP=""
for o in $OBJS; do
  if [ -f "$o" ]; then KEEP="$KEEP $o"; else basename "$o" .o | sed 's/^src_core_fn_//' >> "$SKIPPED_FILE"; fi
done
OBJS="$KEEP"
# Joined-game client state the shell reads (src/core/realm/d2gs_client_state.cpp):
#   d2_realm_client_game_info        the server's game flags: difficulty, hardcore, expansion, ladder
#   d2_realm_client_merc_resurrect   the 0x9B resurrect offer: name id and price
#   d2_realm_client_take_movie       the next film index a 0x61 queued
#   d2_realm_client_npc_alerts       which NPCs carry the speech mark
#   d2_realm_client_unit_marks       which other units, the own hero among them, carry it
#   d2_realm_client_game_ui          the panel flags the game-start packet left, and how many arrived
#   d2_realm_client_set_active_npc   the NPC whose menu is open is not marked
#   d2_realm_client_pets             the pet list (the hireling panel's mercenary)
#   d2_realm_client_pet_stat         the hireling's stats for its panel
link_engine() {
em++ -m64 -O2 -pthread ${D2_CXXFLAGS:-} $OBJS "$STORM_BUILD/libstorm.a" \
  -sPTHREAD_POOL_SIZE=2 -sALLOW_MEMORY_GROWTH=1 -sMAXIMUM_MEMORY=4294967296 \
  -sEXPORTED_RUNTIME_METHODS=FS,ccall,cwrap,UTF8ToString,HEAPU32,HEAPU8,HEAPU16,HEAP32 \
  -sEXPORTED_FUNCTIONS=_main,_d2_local_d2gs_open_client,_d2_hero_is_down,_d2_death_closes_panel,_d2_session_client_sent_vitals,_d2_realm_client_player_right_cast_state,_d2_interact_count_check,_d2_client_window_handle,_d2_client_window_handle_pin,_d2_client_10001,_d2_client_10001_pin,_d2_client_10002,_d2_client_10002_pin,_d2_client_1230,_d2_client_1230_pin,_d2_smoke_mpq,_d2_smoke_i32,_d2_prng_check,_d2_realm_huffman_header_size,_d2_realm_huffman_packet_size,_d2_realm_huffman_decompress,_d2_realm_huffman_compress,_d2_realm_huffman_frame,_d2_realm_huffman_compress_with,_d2_realm_huffman_decode_nibbles,_d2_realm_huffman_build_codes,_d2_realm_huffman_default_lengths,_d2_realm_greeting_parse,_d2_realm_d2gs_frame_size,_d2_realm_d2gs_act_load,_d2_realm_d2gs_player_assign,_d2_realm_d2gs_player_in_game,_d2_realm_d2gs_player_left,_d2_realm_d2gs_party_state,_d2_realm_d2gs_relation,_d2_realm_d2gs_party_id,_d2_realm_d2gs_pvp_status,_d2_realm_d2gs_event_wait_hostile,_d2_realm_d2gs_party_reset,_d2_realm_d2gs_party_add,_d2_realm_d2gs_party_remove,_d2_realm_d2gs_party_clock,_d2_realm_d2gs_party_command,_d2_realm_d2gs_party_set_town,_d2_realm_d2gs_party_take,_d2_realm_d2gs_party_take_for,_d2_realm_d2gs_recv_5d,_d2_realm_d2gs_recv_5d_from,_d2_realm_d2gs_recv_5e,_d2_realm_d2gs_recv_5e_from,_d2_realm_d2gs_state_set,_d2_realm_d2gs_gamelogon,_d2_realm_d2gs_ping,_d2_realm_d2gs_ping_module,_d2_realm_d2gs_pong,_d2_realm_bncs_checkrev_classic,_d2_realm_bncs_exe_version,_d2_realm_bncs_auth_info_env,_d2_realm_bncs_auth_check_env,_d2_realm_mcp_header,_d2_realm_mcp_decode,_d2_realm_mcp_logon_order,_d2_realm_mcp_creategame_request,_d2_realm_mcp_joingame_request,_d2_realm_mcp_creategame_reply,_d2_realm_mcp_joingame_reply,_d2_realm_mcp_protocol_selector,_d2_realm_mcp_startup_request,_d2_realm_mcp_startup_reply,_d2_realm_mcp_charlist2_request,_d2_realm_mcp_charlist2_reply_header,_d2_realm_mcp_charlist2_entry,_d2_realm_mcp_charlogon_request,_d2_realm_mcp_charlogon_reply,_d2_realm_bncs_header,_d2_realm_bncs_decode,_d2_realm_bncs_ping,_d2_realm_bncs_xsha1,_d2_realm_bncs_password_hash,_d2_realm_bncs_double_hash,_d2_realm_bncs_checkrev,_d2_realm_bncs_auth_info_reply,_d2_realm_bncs_auth_check_reply,_d2_realm_bncs_logonresponse2_reply,_d2_realm_bncs_queryrealms2_reply,_d2_realm_bncs_logonrealmex_reply,_d2_realm_bncs_auth_info_request,_d2_realm_bncs_auth_check_request,_d2_realm_bncs_logonresponse2_request,_d2_realm_bncs_logonrealmex_request,_d2_vfs_mount,_d2_asset_read,_d2_asset_ptr,_d2_asset_len,_d2_asset_error,_d2_asset_open,_d2_asset_size,_d2_asset_part,_d2_asset_close,_d2_txt_probe,_d2_table_bin_load,_d2_table_bin_ptr,_d2_table_bin_len,_d2_lang_probe,_d2_lang_string,_d2_lang_string_by_index,_d2_lang_index,_d2_drlg_init,_d2_drlg_act_begin,_d2_drlg_act_begin_server,_d2_drlg_act_end,_d2_drlg_level,_d2_drlg_out,_d2_drlg_rooms,_d2_drlg_room_count,_d2_drlg_room_trace_truncated,_d2_drlg_room_snapshot,_d2_drlg_room_snapshot_count,_d2_drlg_room_snapshot_truncated,_d2_drlg_grid,_d2_drlg_err,_d2_drlg_collect_tiles,_d2_drlg_tiles,_d2_drlg_tile_count,_d2_drlg_collect_picks,_d2_drlg_picks,_d2_drlg_pick_count,_d2_drlg_pick_rooms,_d2_drlg_pick_near,_d2_component_token,_d2_session_retail_level_change,_d2_drlg_dirt_paths,_d2_drlg_collect_dirtgrids,_d2_drlg_dirtgrids,_d2_datatbl,_d2_skillclass_check,_d2_levels_txt_record,_d2_game_clear_flag,_d2_game_return_argument,_d2_game_set_init_seed,_d2_game_seedinit,_d2_itemgen_init,_d2_itemgen_run,_d2_itemgen_out,_d2_itemgen_err,_d2_monspawn_init,_d2_monspawn_level,_d2_monspawn_out,_d2_monspawn_units,_d2_monspawn_err,_d2_monspawn_trace,_d2_path_refusals,_d2_aispawn_out,_d2_aispawn_units,_d2_aitarget_attach,_d2_aitarget_attach_at,_d2_aitarget_out,_d2_aitarget_units,_d2_combat_events,_d2_combat_result,_d2_damagecalc_run,_d2_session_create,_d2_session_create_class,_d2_session_create_class_at,_d2_session_create_raw,_d2_session_create_raw_near,_d2_session_command_npc_packet,_d2_session_command_object_packet,_d2_session_fixture_place_player,_d2_session_fixture_place_monster,_d2_session_command_npc_response_packet,_d2_session_quest_record,_d2_session_set_global_quest_record,_d2_session_reattach_near,_d2_session_town_classes,_d2_session_command_npc_action_packet,_d2_session_inventory_items,_d2_session_set_stat,_d2_session_fixture_set_item_stat,_d2_session_item_units_above,_d2_gamble_table,_d2_session_command_hire_packet,_d2_session_monster_units_above,_d2_session_packet_stream,_d2_session_packet_stream_length,_d2_session_packet_stream_packets,_d2_session_next_movie_request,_d2_client_movie_index,_d2_client_movie_file_name,_d2_client_movie_path,_d2_client_movie_local_dir,_d2_client_movie_count,_d2_session_player_x,_d2_session_player_y,_d2_session_player_id,_d2_session_player_gold,_d2_session_stash_gold,_d2_session_set_quest_flag,_d2_session_fixture_quest_start,_d2_session_quest_fstate,_d2_session_prison_quest_evidence,_d2_session_quest_flag,_d2_session_player_level,_d2_session_player_act,_d2_session_nearest_travel_npc,_d2_session_command_travel,_d2_session_command_move,_d2_session_walkable,_d2_session_collision,_d2_session_move_reason,_d2_session_astar_probe,_d2_session_warp_tile_in_interact_range,_d2_session_command_warp,_d2_session_command_nearest_warp,_d2_session_command_seam,_d2_session_level_links,_d2_session_level_rect,_d2_session_level_picked_file,_d2_session_room_active_at,_d2_set_view_size,_d2_session_level_rooms,_d2_session_tile_world_xy,_d2_session_room_resolvable_from,_d2_session_player_busy,_d2_session_seam_edges,_d2_session_seam_edge_data,_d2_session_fixture_quest_monster_killed,_d2_session_fixture_stand_near_object,_d2_session_fixture_stand_near_unit,_d2_session_fixture_kill_monster,_d2_session_fixture_open_worldstone,_d2_session_fixture_anya_open_portal,_d2_session_fixture_prime_throne_baal,_d2_session_fixture_quest_item,_d2_session_fixture_ground_item,_d2_session_command_item_to_cursor_packet,_d2_session_command_item_to_object_packet,_d2_session_command_equip_packet,_d2_session_command_cursor_item_packet,_d2_session_command_object,_d2_session_command_npc,_d2_session_command_nearest_npc,_d2_session_unit_kinds,_d2_session_command_npc_response,_d2_session_command_end_npc,_d2_session_take_ui_events,_d2_session_command_button,_d2_session_take_waypoint_menu,_d2_session_waypoints,_d2_level_name,_d2_level_entry_file,_d2_monster_name_str,_d2_last_assert,_d2_missile_celfile,_d2_session_command_waypoint,_d2_session_attack_reason,_d2_session_command_npc_vendor,_d2_session_command_npc_item,_d2_session_command_vendor_sell,_d2_session_command_vendor_buy,_d2_session_command_vendor_multibuy,_d2_session_item_quantity,_d2_session_command_vendor_gamble,_d2_session_command_vendor_repair,_d2_session_item_needs_repair,_d2_session_vendor_repair,_d2_session_command_primary_attack,_d2_session_command_throw,_d2_session_command_kick,_d2_session_equipped_quantity,_d2_session_command_pickup,_d2_session_client_pickup,_d2_session_pickup_path_active,_d2_session_command_drop,_d2_session_command_use_belt,_d2_session_command_use_item,_d2_session_command_use_item_action,_d2_session_command_swap_weapons,_d2_session_weapon_switch,_d2_session_fixture_experience,_d2_session_fixture_level,_d2_session_command_add_stat,_d2_session_command_add_skill,_d2_session_class_skills,_d2_session_skill_desc,_d2_session_skill_strings,_d2_session_belt_boxes,_d2_session_inventory_layout,_d2_session_preset_edge,_d2_session_preset_edge_counts,_d2_session_preset_neighbours,_d2_session_vendor_presentations,_d2_session_vendor_presentation_count,_d2_session_item_lines,_d2_session_item_title,_d2_session_item_sell_value,_d2_session_gamble_price,_d2_session_item_repair_cost,_d2_session_last_transaction_result,_d2_client_4b5d0_refusal_string,_d2_session_fixture_affixes,_d2_session_advancement,_d2_session_command_stow_body,_d2_session_command_respawn,_d2_session_command_recover_corpse,_d2_session_item_save_roundtrip,_d2_session_item_save_bytes,_d2_session_item_save_length,_d2_session_stats_save_roundtrip,_d2_session_stats_save_bytes,_d2_session_stats_save_length,_d2_session_campaign_save_roundtrip,_d2_session_skill_save_bytes,_d2_session_skill_save_length,_d2_session_waypoint_save_bytes,_d2_session_waypoint_save_length,_d2_session_quest_intro_save_roundtrip,_d2_session_quest_save_bytes,_d2_session_quest_save_length,_d2_session_intro_save_bytes,_d2_session_intro_save_length,_d2_session_corpse_save_roundtrip,_d2_session_corpse_save_bytes,_d2_session_corpse_save_length,_d2_session_tick,_d2_session_replay_no_populate,_d2_session_attack,_d2_session_progress,_d2_session_item_state,_d2_session_npc_interaction,_d2_session_npc_dialog,_d2_session_npc_response,_d2_session_vendor,_d2_session_vendor_items,_d2_session_vendor_item_count,_d2_session_vendor_transaction,_d2_session_act1q1,_d2_session_player_weapon_class_id,_d2_session_snapshot,_d2_session_hold_world,_d2_session_client_view,_d2_session_near_rooms,_d2_session_unit_info,_d2_session_tick_gate,_d2_session_hp_watch,_d2_session_player_has_state,_d2_session_resources,_d2_session_player_coords,_d2_session_motion,_d2_session_rooms,_d2_session_room_count,_d2_session_units,_d2_session_unit_coords,_d2_session_unit_coord_count,_d2_session_unit_facings,_d2_session_unit_facing_count,_d2_session_unit_overlays,_d2_session_unit_overlay_count,_d2_session_unit_states,_d2_session_unit_state_count,_d2_session_unit_count,_d2_session_ai_think_frame,_d2_session_ai_think_watch,_d2_session_ai_think_event_count,_d2_session_ai_think_last_event_frame,_d2_session_ai_think_event_path,_d2_session_ai_think_attach_frame,_d2_session_ai_think_attach_mode,_d2_session_ai_think_attach_deadline,_d2_session_ai_think_attach_room_x,_d2_session_ai_think_attach_room_y,_d2_session_ai_think_attach_room_has_warriv,_d2_session_ai_think_attach_player_x,_d2_session_ai_think_attach_player_y,_d2_session_monster_animations,_d2_session_monster_animation_count,_d2_session_player_form_actor,_d2_session_object_presentations,_d2_session_object_presentation_count,_d2_session_item_presentations,_d2_session_item_sockets,_d2_session_item_socket_count,_d2_session_item_colours,_d2_session_item_inv_colours,_d2_session_item_presentation_count,_d2_session_world_hash,_d2_session_tiles,_d2_session_tile_count,_d2_session_tile_entries,_d2_session_seam_tile_entries,_d2_session_tile_coordlists,_d2_session_seam_tile_coordlists,_d2_session_player_coordlist,_d2_session_tiles_skipped,_d2_session_room_edge_flags,_d2_session_terrain_hash,_d2_session_level_tile_files,_d2_session_seam_tiles,_d2_session_seam_tile_count,_d2_session_seam_tile_files,_d2_session_room_libraries,_d2_session_trace_rolls,_d2_session_tile_rolls,_d2_session_tile_roll_count,_d2_session_roll_orphans,_d2_session_cross_rolls,_d2_session_level_type,_d2_session_save_d2s,_d2_session_load_d2s,_d2_session_save_d2s_error,_d2_session_set_character_name,_d2_session_set_client_flags,_d2_session_client_flags,_d2_session_warps,_d2_session_warp_count,_d2_session_navigation_hash,_d2_session_weather_rain,_d2_session_weather_bubbles,_d2_session_destroy,_d2_session_command_insert_item_packet,_d2_session_create_class_bare,_d2_session_client_packet,_d2_session_cursor_item,_d2_session_player_stat,_d2_session_fixture_waypoint,_d2_session_inventory_components,_d2_session_configure_plugy_stash_grid,_d2_session_player_inventory_record,_d2_session_dispatch_refused,_malloc,_free,_d2_mondrop_run,_d2_mondrop_monsters,_d2_mondrop_items,_d2_mondrop_quest_done,_d2_aitarget_snapshot,_d2_unit_timers,_d2_aipath_units,_d2_collision_pattern,_d2_session_fixture_quality_item,_d2_session_item_quality,_d2_session_item_flags,_d2_aitarget_player_swing,_d2_skillprobe_cast,_d2_aitarget_set_player_class,_d2_aitarget_player_kit_bash,_d2_aitarget_swing_right,_d2_aitarget_hands,_d2_session_command_select_skill,_d2_session_selected_skills,_d2_session_skill_choices,_d2_session_command_right_skill,_d2_session_command_left_skill,_d2_session_lightmap,_d2_realm_client_lightmap,_d2_realm_client_period_of_day,_d2_session_period_of_day,_d2_session_dead_regen,_d2_session_busy_watch,_d2_session_damage_bonuses,_d2_session_damage_trace,_d2_session_right_skill_refusal,_d2_session_unit_stat,_d2_session_activation_log,_d2_session_spawn_log,_d2_session_missile_births,_d2_session_room_create_log,_d2_session_popzero_log,_d2_session_room_sequence,_d2_monstats2_pixheight,_d2_monspawn_set_sequence,_d2_monspawn_set_create_order,_d2_monspawn_set_order,_d2_session_command_vendor_repair_all,_d2_session_monregion,_d2_session_melee_probe,_d2_ai_target_probe,_d2_session_character_screen,_d2_session_character_stamina_base,_d2_session_create_from_save,_d2_session_convert_expansion,_d2_session_item_class_is_type,_d2_session_stash_insert,_d2_session_stash_export,_d2_session_stash_clear,_d2_session_pop_log,_d2_session_fixture_place_player_frac,_d2_session_warp_origins,_d2_monster_type_str,_d2_session_unidentified_count,_d2_session_fixture_unidentify,_d2_session_hellforge_diagnostic,_d2_session_staff_tomb,_d2_session_roll_calls,_d2_session_town_centre,_d2_session_town_walk,_d2_session_roll_call_count,_d2_session_left_hand_weapon,_d2_session_compelling_orb_trace,_d2_realm_d2gs_parse_player_assign,_d2_realm_d2gs_move_to,_d2_realm_d2gs_right_skill_at,_d2_realm_d2gs_packet_size,_d2_realm_net_65d0,_d2_realm_net_queue,_d2_realm_net_queue_reset,_d2_realm_d2gs_find_player_assign,_d2_realm_d2gs_parse_player_reassign,_d2_realm_d2gs_parse_owned_item,_d2_realm_client_owned_item_readback_by_guid,_d2_realm_client_item_needs_repair,_d2_realm_client_item_repair_field,_d2_realm_client_item_sell_value,_d2_realm_client_player_reset,_d2_realm_client_unit_resources_readback,_d2_realm_client_player_feed,_d2_realm_client_unit_counter_set,_d2_realm_client_missile_count,_d2_realm_client_missiles,_d2_realm_client_player_pending,_d2_realm_client_player_state,_d2_realm_client_world_state,_d2_realm_client_snapshot,_d2_realm_client_resources,_d2_drlg_client_room_seed_at,_d2_drlg_client_player_context,_d2_realm_client_player_unit_seed,_d2_realm_client_set_difficulty,_d2_realm_client_player_count,_d2_realm_client_player_at,_d2_drlg_client_map_reveal,_d2_drlg_client_first_room_xy,_d2_drlg_client_town_level_argument,_d2_drlg_client_map_room_hint_used,_d2_drlg_client_tiles,_d2_drlg_client_tile_count,_d2_drlg_client_tile_entries,_d2_drlg_client_tile_files,_d2_drlg_client_render_level,_d2_drlg_client_select_render_level,_d2_realm_client_monster_count,_d2_realm_client_monsters,_d2_realm_client_motion_tick,_d2_realm_client_player_predict_right_cast_on_monster,_d2_realm_client_local_move,_d2_realm_client_local_move_to_unit,_d2_realm_client_can_interact,_d2_realm_client_character_screen,_d2_realm_client_hero_stat,_d2_realm_client_monster_death_mode_event_count,_d2_realm_client_monster_death_mode_events,_d2_realm_client_player_stat,_d2_realm_client_unit_state,_d2_realm_client_local_action_done,_d2_realm_client_object_count,_d2_realm_client_objects,_d2_realm_client_player_predict_right_cast,_d2_realm_client_unit_precise,_d2_realm_client_monster_motion_status,_d2_realm_client_monster_motion_status_v2,_d2_realm_client_monster_motion_status_v3,_d2_realm_client_motion_facing_step,_d2_realm_client_motion_refusal_mode,_d2_realm_client_monster_motion_collision,_d2_drlg_client_room_info,_d2_drlg_client_room_freed_by_hide_at,_d2_realm_d2gs_parse_player_move,_d2_realm_client_apply_player_move,_d2_realm_client_player_motion_at,_d2_session_path_attempt_trace_control,_d2_session_path_attempt_context,_d2_session_path_attempt_trace,_d2_session_walk_probe,_d2_session_path_attempt_trace_dropped,_d2_waypoint_copy,_d2_gfx_view_mode_selector,_d2_lang_strstr_b020,_d2_client_loading_g,_d2_client_loading_pairs,_d2_client_loadbar_width,_d2_client_loadbar_chunks,_d2_client_loadbar_final_width,_d2_client_loadbar_walk_calls,_d2_client_loadbar_rects,_d2_session_hero_near_rooms,_d2_client_frame_g,_d2_client_session_frame,_d2_client_1260,_d2_client_1260_pin,_d2_client_1300,_d2_client_1300_pin,_d2_client_1580,_d2_client_1580_pin,_d2_client_15b0,_d2_client_15b0_pin,_d2_client_1630,_d2_client_1660,_d2_client_17b0,_d2_client_18d0,_d2_client_2dc0,_d2_client_2dc0_seed,_d2_client_18d0_seed,_d2_client_1a60,_d2_client_2d50,_d2_game_monflag,_d2_game_uniqdrop,_d2_game_questinit,_d2_game_questinit_word,_d2_lang_strncpy_b0d0,_d2_lang_ushort_1010,_d2_game_eventqueue,_d2_game_timerslab,_d2_game_freeq,_d2_game_cacc0,_d2_game_npcctl,_d2_game_npcctl_word,_d2_game_47820,_d2_game_dfc10,_d2_game_afb90,_d2_game_b27_word,_d2_game_47a20_set,_d2_game_47a20_out,_d2_game_47a20_run,_d2_game_46cd0_set,_d2_game_46cd0_out,_d2_game_46cd0_run,_d2_game_e1430_set,_d2_game_e1430_out,_d2_game_e1430_run,_d2_game_4b500_set,_d2_game_4b500_out,_d2_game_4b500_run,_d2_game_4b1d0_set,_d2_game_4b1d0_out,_d2_game_4b1d0_run,_d2_game_4b3c0_set,_d2_game_4b3c0_out,_d2_game_34860_set,_d2_game_34860_run,_d2_game_34860_out,_d2_game_769a0_set,_d2_game_769a0_run,_d2_game_769a0_out,_d2_game_4b3c0_run,_d2_game_4b400_set,_d2_game_4bcc0_set,_d2_game_4bcc0_out,_d2_game_4bcc0_run,_d2_game_4b400_out,_d2_game_4b400_run,_d2_game_8a3e0_set,_d2_game_8a3e0_out,_d2_game_8a3e0_run,_d2_game_2eea0_set,_d2_game_2eea0_run,_d2_game_2eea0_out,_d2_game_90320_set,_d2_game_90320_out,_d2_game_90320_run,_d2_gf_set,_d2_gf_run,_d2_gf_out,_d2_sweep_set,_d2_sweep_run,_d2_sweep_out,_d2_up_set,_d2_up_run,_d2_up_out,_d2_gm_set,_d2_gm_run,_d2_gm_out,_d2_session_fixture_set_unit_stat,_d2_session_level_threshold,_d2_session_skill_probe,_d2_session_create_bare_at,_d2_session_create_class_at_bare,_d2_session_monster_states,_d2_client_footstep_due,_d2_client_monster_footstep_row,_d2_client_monster_footstep_layer,_d2_client_monster_idle,_d2_client_monster_sfx_stamp,_d2_client_monster_idle_reset,_d2_ambient_now,_d2_client_cast_gate_reset,_d2_client_cast_gate_row,_d2_client_monster_footstep,_d2_session_floor_class,_d2_session_command_dropgold,_d2_session_other_level_rooms,_d2_session_seam_tile_world_xy,_d2_session_state_set_seq,_d2_client_take_local_action,_d2_client_note_local_action,_d2_realm_client_take_missile_sounds,_d2_session_fixture_skill_level,_d2_client_explicit_move,_d2_session_fixture_item_flag,_d2_session_fixture_item_names,_d2_session_fixture_quality_item_level,_d2_session_ground_item_order,_d2_showitems_begin,_d2_showitems_add,_d2_showitems_count,_d2_showitems_pin_ui,_d2_showitems_slot,_d2_showitems_order_key,_d2_session_hit_packets,_d2_session_hit_packet_count,_d2_client_attack_roll_take,_d2_client_inv_click_route,_d2_client_store_offer_click,_d2_client_stash_close_hit,_d2_session_monster_idle_take,_d2_session_item_box_state,_d2_session_command_drop_gold_packet,_d2_session_fixture_spawn_monster,_d2_session_snapshot_level,_d2_client_stat_update_actions,_d2_session_weapon_hands,_d2_session_player_base_stat,_d2_realm_client_level_rect,_d2_realm_client_level_picked_file,_d2_realm_client_seam_edges,_d2_realm_client_seam_edge_data,_d2_realm_client_item_price,_d2_realm_client_stock_price,_d2_realm_client_take_resync,_d2_realm_client_weapon_hands,_d2_realm_client_set_quest_flags,_d2_realm_client_hire_stats,_d2_realm_client_corpses,_d2_realm_client_local_skill_at,_d2_realm_client_local_skill_on_unit,_d2_realm_client_skill_plan,_d2net_local_open,_d2net_local_close,_d2net_local_send,_d2net_local_take,_d2net_local_take_system,_d2net_local_drain,_d2net_local_drain_length,_d2net_local_mode,_d2net_local_queued,_d2net_local_taken,_d2net_local_unframed,_d2net_local_compressed,_d2_session_join_stream,_d2_session_client_rooms,_d2_session_join_load_length,_d2_session_game_seed,_d2_session_game_flags,_d2_local_d2gs_open,_d2_local_d2gs_close,_d2_local_d2gs_client,_d2_local_d2gs_pump,_d2_local_d2gs_out,_d2_local_d2gs_take,_d2_local_d2gs_dropped,_d2_local_d2gs_refused,_d2_realm_client_belt_index,_d2_realm_client_items,_d2_realm_client_item_box_state,_d2_realm_client_object_units,_d2_realm_client_take_object_events,_d2_realm_client_take_unit_fx,_d2_realm_client_unit_fx_tick,_d2_realm_client_unit_hit_class,_d2_realm_client_unit_life,_d2_realm_client_items_set_expansion,_d2_realm_client_unit_info,_d2_realm_client_game_info,_d2_realm_client_npc_alerts,_d2_realm_client_unit_marks,_d2_realm_client_game_ui,_d2_realm_client_set_active_npc,_d2_realm_client_pets,_d2_realm_client_pet_stat,_d2_realm_client_quests,_d2_realm_client_rooms,_d2_realm_client_warps,_d2_realm_client_local_act_on_unit,_d2_realm_client_local_click_player,_d2_realm_client_local_fire,_d2_realm_client_local_sends,_d2_realm_client_local_send_view,_d2_realm_client_local_queued_kind,_d2_session_character_name,_d2_session_join_units,_d2_session_join_reveal,_d2_session_client_spawned,_d2_session_game_frame,_d2_session_right_ground_plan,_d2_realm_client_merc_resurrect,_d2_realm_client_take_movie \
  -sENVIRONMENT=web,worker -sNO_EXIT_RUNTIME=1 -Wl,--error-limit=0 \
  -o "$BUILD/d2.js"
}
LINKLOG="$BUILD/link.log"
probe_link_retry || exit 1
tail -5 "$LINKLOG"

mkdir -p play
for f in d2.js d2.wasm; do cp -f "$BUILD/$f" "play/$f"; done
ls -la play/d2.js play/d2.wasm
