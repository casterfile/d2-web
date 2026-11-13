#!/usr/bin/env bash
# tools/leak-check.sh [--staged | --all | --commit <rev> | --msg <file>]
# Refuses private content in this public repo: RE notes, captures, the private store's files, retail
# assets, hosts, names, secrets, AI trailers. Runs on commit (tools/hooks/pre-commit, commit-msg), on push
# (pre-push, per commit) and in CI. Fix the hit; never --no-verify it.
set -uo pipefail
cd "$(git rev-parse --show-toplevel)"
mode="${1:---staged}"; rev="${2:-}"
fail=0
say() { echo "LEAK [$1]:"; sed 's/^/  /' | head -20; fail=1; }
# Apple's Git regex backend does not recognize GNU \b. The scan needs the same
# word boundary on macOS and Linux; every check only asks whether a match exists.
portable_pattern() {
  local p="${1#*|}"
  p="${p/\\b/(^|[^[:alnum:]_])}"
  printf '%s' "${p/\\b/($|[^[:alnum:]_])}"
}

# content patterns, shared by files and commit messages
PATS=(
  "private-ip|\\b(192\\.168\\.[0-9]+\\.[0-9]+|10\\.[0-9]+\\.[0-9]+\\.[0-9]+|172\\.(1[6-9]|2[0-9]|3[01])\\.[0-9]+\\.[0-9]+)\\b"
  "home-path|/home/[a-z]+/|C:\\\\\\\\Users\\\\\\\\|/Users/[a-z]+/"
  "person|\\bMike\\b|mstavridis|Stavridis"
  "agent-loop|\\b(CLAUDE\\.md|AGENTS\\.md|HANDOFF\\.md|OBJECTIVE\\.md|GOAL\\.md|plane\\.mjs|spawn-plan|ledger\\.mjs|re-d2(common|game|client)|merge-runner|vector-capture|subagent|coordinator)\\b|docs/(re|agents)/|tools/capture|d2-web-private"
  "secret|(ghp_[A-Za-z0-9]{20,}|github_pat_|sk-ant-|sk-[A-Za-z0-9]{32,}|AKIA[0-9A-Z]{16}|BEGIN [A-Z ]*PRIVATE KEY|xox[bp]-)"
  "repo-map|\\b(redivivus|d2decomped|psi29a)\\b"
  "queue-id|\\bOPEND2-[0-9]+\\b"
  "jargon|\\b(ledger|s39[0-9]{2}|coord-merge)\\b"
)
MSG_PATS=( "ai-trailer|^(Co-Authored-By|Claude-Session|Generated with)|claude\\.ai/code|@anthropic\\.com" "identity|@gmail\\.com" )

if [ "$mode" = --msg ]; then
  body="$(grep -v '^#' "$rev")"
  for p in "${PATS[@]}" "${MSG_PATS[@]}"; do
    h="$(printf '%s\n' "$body" | grep -inE "$(portable_pattern "$p")")"; [ -n "$h" ] && say "message ${p%%|*}" <<< "$h"
  done
  [ "$fail" = 0 ] || { echo "leak-check: commit message refused" >&2; exit 1; }; exit 0
fi

# the files to check, and where git grep / cat-file read them from
case "$mode" in
  --staged) files=$(git diff --cached --name-only --diff-filter=ACMR); opt=(--cached); src=(); obj=":" ;;
  --all)    files=$(git ls-files);                                   opt=(); src=(HEAD); obj="HEAD:" ;;
  --commit) files=$(git diff-tree --root --no-commit-id -r --name-only --diff-filter=ACMR "$rev"); opt=(); src=("$rev"); obj="$rev:" ;;
  *) echo "usage: $0 [--staged|--all|--commit <rev>|--msg <file>]" >&2; exit 2 ;;
esac
[ -z "$files" ] && exit 0

DENY='(^|/)(\.claude|\.private|desktop|tests|third-party|scenarios|harness-out|coverage|vectors|reference|ghidra|retail|shots|iso)(/|$)|^docs/(re|agents)/|^tools/(re|capture|progress|showcase)(/|$)|(^|/)(CLAUDE|AGENTS|HANDOFF|OBJECTIVE|GOAL|GOAL_BNET|README-BUILD|goal-[a-z0-9-]+)\.md$|\.(dll|exe|mpq|iso|cab|gpr|gzf|rep|dc6|dcc|dt1|ds1|cof|wav|log|tsv)$|^tools/(plane|spawn-plan|ledger|defects|claims-reap)\.|^test/fixtures/bnet-tos-[a-z]+\.txt$|^src/core/realm/bnclient_(key26|batch_10080)\.|(^|/)patch-113c\.patch$|^tools/(manifest-add\.mjs|manifest_lock\.py)$'
# the SGD2FreeRes widescreen HUD art (third-party mod, redistributable under its AGPLv3) is the one .dc6 allowed in git
h=$(printf '%s\n' "$files" | grep -E "$DENY" | grep -vE '^docs/screenshots/|^play/freeres/(D2MR|NeoD2MR)[A-Za-z]+\.dc6$'); [ -n "$h" ] && say "private path" <<< "$h"
h=$(printf '%s\n' "$files" | grep -iE '\.(png|jpg)$' | grep -vE '^docs/screenshots/'); [ -n "$h" ] && say "image outside docs/screenshots" <<< "$h"

# one git grep per pattern over all files at once (-I skips binaries); the checker and .gitignore name
# the private paths on purpose
for p in "${PATS[@]}"; do
  if [ "$mode" = --all ]; then
    # macOS Bash 3.2 treats an empty array as unset under nounset. Expand only
    # populated arrays so the actual content scan still runs on that shell.
    h=$(git grep ${opt[@]+"${opt[@]}"} -I -nE "$(portable_pattern "$p")" ${src[@]+"${src[@]}"} -- . ':!tools/leak-check.sh' ':!.gitignore' 2>/dev/null)
  else
    h=$(printf '%s\0' $files | xargs -0 git grep ${opt[@]+"${opt[@]}"} -I -nE "$(portable_pattern "$p")" ${src[@]+"${src[@]}"} -- ':!tools/leak-check.sh' ':!.gitignore' 2>/dev/null)
  fi
  [ -n "$h" ] && say "${p%%|*}" <<< "$h"
done

# size of the blob being committed, not of the working-tree file (submodule links have no blob)
big=$(printf "$obj%s\n" $files | git cat-file --batch-check='%(objecttype) %(objectsize)' 2>/dev/null \
  | paste -d' ' - <(printf '%s\n' $files) | awk '$1 == "blob" && $2 > 5242880 {print $3" ("$2" bytes)"}')
[ -n "$big" ] && say "size>5M" <<< "$big"

[ "$fail" = 0 ] || { echo "leak-check: refused. Private work belongs in the private store; do not bypass." >&2; exit 1; }
