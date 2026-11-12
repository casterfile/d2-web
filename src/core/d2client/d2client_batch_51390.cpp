// Batch D2Client+0x51390..0x527A0, derived from cached 1.13c decompile.
// Ordinals/callees are injected through Calls; globals are passed in State.
#include <cstdint>
#include <cstring>
namespace d2client_batch_51390 {
using U=uint32_t; using I=int32_t; using P=void*;
static U rd(P p,U n){U v;std::memcpy(&v,static_cast<uint8_t*>(p)+n,4);return v;}
struct Calls {
 I (*pathX)(P); I (*pathY)(P);           // Ordinal_10867 / 10750 (path x/y)
 P (*resolve)(U);                        // Ordinal_10331 (1 arg)
 P (*resolve3)(U,U,U);                   // Ordinal_10331 (room,1,0x220) as 52570 pushes it
 P (*node630f0)(U*);                     // D2Client+0xB30F0 (msg dispatch, 23 words)
 void (*o10637)(P,I); void (*o10019)(P,I); P (*o10754)(P);
 void (*fn5ff10)(U,I,U,U,I,I);           // D2Client+0xB5FF10: EDX=unit, stack (id,x,y,0,1)
 I (*o10573)(P,I,I,I,I); P (*o10389)(P,I,I,P,I,I);
 U (*o11117)(); void (*o11129)(); I (*mask)(); P (*o10078)(...);
};
// unit coordinate: types 2,4,5 read the path struct directly
static U ux(P u,Calls&c){I t=I(rd(u,0));P p=reinterpret_cast<P>(uintptr_t(rd(u,0x2c)));
 return (t==2||(t>3&&t<6))?rd(p,12):(p?U(c.pathX(p)):0);}
static U uy(P u,Calls&c){I t=I(rd(u,0));P p=reinterpret_cast<P>(uintptr_t(rd(u,0x2c)));
 return (t==2||(t>3&&t<6))?rd(p,16):(p?U(c.pathY(p)):0);}
// 1.13c D2Client+0x51390 and +0x51500 live in their own files (d2client_51390.cpp, d2client_51500.cpp).
// old form of D2Client 51D10 (now d2client_51d10.cpp)  (ECX=px out, EAX=py out; fn10f0 takes px and EBX=py)
struct S51D10 { I mode,c414,b848,bc4c,b828,b824,c2fc,x0,y0,x1,y1,c2f8,a68,c2f4,c64c,c9638; U* c3c4; };
I sub_51D10(I* px,I* py,S51D10& g,I (*fn2c5c0)(),void (*fn1860)(),void (*fn10f0)(I*,I*),U* d3afa,
            P (*fn55b40)(I,I),P (*ord10331)(P,P),void (*ord10742)(P)){
 I m=g.c414;
 if(m!=3){ *px=g.b828; *py=g.b824;
  if(*py<=g.bc4c-0x31){
   if(!g.c2fc||*px<=g.x0||g.x1<=*px||*py<=g.y0||g.y1<=*py){
    if(m==1){ if(*px<g.b848/2){fn10f0(px,py);return 0;} }
    else if(m!=2||g.b848/2<=*px){fn10f0(px,py);return 0;}
   }
   fn1860(); return 1; } }
 if(!g.c2f8){
  if(g.a68&&!fn2c5c0()){*d3afa=0;return 1;}
  *d3afa=0;
  if(g.c2f4&&g.c64c==5){ P u=fn55b40(g.c9638,5);
   if(u){ ord10742(ord10331(u,reinterpret_cast<P>(uintptr_t(rd(u,4))))); *g.c3c4|=1; } }
  g.c2f4=0; }
 return 1;
}
// 1.13c D2Client+0x51E30
void sub_51E30(P esi,I (*f568e0)(P,I,I,I),void (*f1860)(P),I& c2f4,I& c2f8){
 if(!esi){c2f4=0;c2f8=1;return;}
 I r=f568e0(esi,0,0,0); c2f8=0;
 if(r){f1860(esi);c2f8=1;return;}
 c2f4=0;
}
// message builder shared shape: 23 zeroed words, dispatch via D2Client+0xB30F0
static P send(Calls& c,U w0,U w1,U w2,U w3,U w4,U w5){U m[23]={};m[0]=w0;m[1]=w1;m[2]=w2;m[3]=w3;m[4]=w4;m[5]=w5;return c.node630f0(m);}
// old form of D2Client 52230 (now d2client_52230.cpp)  (word idx = byte offset from frame base / 4: [0x10]=0x133,[0x30]=1)
void sub_52230(U p2,Calls& c){U m[23]={};m[1]=p2;m[2]=p2;m[4]=0x133;m[12]=1;c.node630f0(m);}
// old form of D2Client 52270 (now d2client_52270.cpp)  (ESI = id; disassembly: each message is 23 zeroed words with [1]=[2]=ESI, [4]=type, [12]=1;
// types 0x12b (result p goes to Ordinal_10637/10019 with 0x4b), 0x166, 0x132)
void sub_52270(U esi,Calls& c){
 U m[23]={}; m[1]=esi; m[2]=esi; m[4]=0x12b; m[12]=1;
 P p=c.node630f0(m);
 if(p){c.o10637(p,0x4b);c.o10019(p,0x4b);}
 std::memset(m,0,sizeof m); m[1]=esi; m[12]=1; m[2]=esi; m[4]=0x166; c.node630f0(m);
 std::memset(m,0,sizeof m); m[1]=esi; m[2]=esi; m[12]=1; m[4]=0x132; c.node630f0(m);
}
// old form of D2Client 52320 (now d2client_52320.cpp) (ESI = unit/client handle; three messages 0x11c, 0x128, 0x125)
void sub_52320(U esi,Calls& c){U m[23]={}; m[1]=esi; m[2]=esi; m[4]=0x11c; m[12]=1;
 P p=c.node630f0(m); if(p)c.o10637(p,0x32);
 std::memset(m,0,sizeof m); m[1]=esi; m[2]=esi; m[4]=0x128; m[12]=1; c.node630f0(m);
 m[4]=0x125; c.node630f0(m);}
// 1.13c D2Client+0x524D0
// Returns packed pair; rgb outputs set when flag 5591 set.
uint64_t sub_524D0(U p2,I c2e0,I c0a0,uint8_t c5591,I a212c,uint8_t* o3,uint8_t* o4,uint8_t* o5,uint64_t (*tail)(I)){
 if(!c2e0){
  if(!c0a0) return ~0ull; // original exits the process (_exit(-1))
  if(c5591){*o3=0xff;*o4=0x40;*o5=0x30;
   if(a212c==-1)return (uint64_t(uintptr_t(o5))<<32)|0x50;
   return tail((a212c*0x80)/0x1e);}
 }
 return uint64_t(p2)<<32;
}
// Probe-facing form: EAX only, Fog's float cosine as the tail, truncated as _ftol does.
int32_t sub_524D0(I c2e0,I c0a0,uint8_t c5591,I a212c,uint8_t* o3,uint8_t* o4,uint8_t* o5,float (*f)(I)){
 static float (*s_f)(I); s_f=f;
 return I(sub_524D0(0,c2e0,c0a0,c5591,a212c,o3,o4,o5,[](I v)->uint64_t{return uint64_t(int64_t(double(s_f(v))*80.0));}));   // Fog 10083 result, fmul [D2Client+0xD753C] (80.0f), then _ftol
}
// 1.13c D2Client+0x52570 (cbbfc = global at DAT_6fbcbbfc, passed as client)
void sub_52570(P client,Calls& c){
 P p=c.o10754(c.resolve3(U(uintptr_t(client)),1,0x220)); if(!p)return;
 U y=uy(p,c),x=ux(p,c);
 c.fn5ff10(U(uintptr_t(client)),0x271,x,y,0,1);
}
// old form of D2Client 52620 / 526E0 / 527A0 (now their own files): unit event message (ids 0x16b / 0x161 / 0x15b)
static void unitMsg(P unit,U client,U id,Calls& c){
 U y=uy(unit,c),x=ux(unit,c);
 if(!client)return;
 U m[23]={};m[0]=1;m[1]=client;m[4]=id;m[5]=x;m[6]=y;m[11]=0;m[12]=1;c.node630f0(m);
}
void sub_52620(P u,U cl,Calls& c){unitMsg(u,cl,0x16b,c);}
void sub_526E0(P u,U cl,Calls& c){unitMsg(u,cl,0x161,c);}
void sub_527A0(P u,U cl,Calls& c){unitMsg(u,cl,0x15b,c);}
}
