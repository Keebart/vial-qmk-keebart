#include "macro_restore.h"
#include <string.h>
#ifndef KS_RESTORE_MODEL
#ifdef KSR_LAYOUT_ID
#define KS_RESTORE_MODEL KSR_LAYOUT_ID
#else
#include "hardware.h"
#define KS_RESTORE_MODEL KS_MODEL_ID
#endif
#endif
const volatile uint8_t ks_restore_image[KS_RESTORE_BYTES] = {
    'K','S','R','M','A','C','0','1', KS_RESTORE_MODEL & 255, KS_RESTORE_MODEL >> 8, 1, 0
};
static bool consumed;
static uint32_t rotr(uint32_t a, unsigned n) { return (a >> n) | (a << (32-n)); }
static uint32_t be32(const uint8_t *p) { return (uint32_t)p[0]<<24 | (uint32_t)p[1]<<16 | (uint32_t)p[2]<<8 | p[3]; }
static void block(uint32_t h[8], const uint8_t b[64]) {
    static const uint32_t k[64] = {
        0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
        0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
        0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
        0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
        0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
        0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
        0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
        0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
    };
    uint32_t w[16], a=h[0],c=h[2],d=h[3],e=h[4],f=h[5],g=h[6],v=h[7],bb=h[1];
    for(unsigned i=0;i<64;i++) {
        unsigned n=i&15;
        if(i<16) w[n]=be32(b+4*i);
        else {uint32_t x=w[(i+1)&15],y=w[(i+14)&15]; w[n]+= (rotr(x,7)^rotr(x,18)^(x>>3))+w[(i+9)&15]+(rotr(y,17)^rotr(y,19)^(y>>10));}
        uint32_t t=v+(rotr(e,6)^rotr(e,11)^rotr(e,25))+((e&f)^(~e&g))+k[i]+w[n];
        uint32_t u=(rotr(a,2)^rotr(a,13)^rotr(a,22))+((a&bb)^(a&c)^(bb&c));
        v=g;g=f;f=e;e=d+t;d=c;c=bb;bb=a;a=t+u;
    }
    h[0]+=a;h[1]+=bb;h[2]+=c;h[3]+=d;h[4]+=e;h[5]+=f;h[6]+=g;h[7]+=v;
}
void ks_restore_hash(const void *data, size_t size, uint8_t out[32]) {
    uint32_t h[8]={0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};
    const uint8_t *p=data;size_t n=size;
    while(n>=64){block(h,p);p+=64;n-=64;}
    uint8_t last[64]={0};memcpy(last,p,n);last[n]=128;
    if(n>=56){block(h,last);memset(last,0,64);}
    uint64_t bits=(uint64_t)size*8;
    for(unsigned i=0;i<8;i++)last[63-i]=(uint8_t)(bits>>(i*8));
    block(h,last);
    for(unsigned i=0;i<32;i++)out[i]=(uint8_t)(h[i/4]>>(24-(i%4)*8));
}
bool ks_restore_available(void) {
    if(consumed)return false;
    uint8_t bytes[KS_RESTORE_BYTES];for(unsigned i=0;i<sizeof(bytes);i++)bytes[i]=ks_restore_image[i];
    if(memcmp(bytes,"KSRMAC01",8) || bytes[8]!=(KS_RESTORE_MODEL&255) || bytes[9]!=(KS_RESTORE_MODEL>>8) || bytes[10]!=1 || bytes[11])return false;
    uint32_t crc=~0u;bool nonzero=false;
    for(unsigned i=0;i<44;i++){crc^=bytes[i];for(unsigned b=0;b<8;b++)crc=(crc>>1)^(0xedb88320u & (0u-(crc&1)));if(i>=12 && bytes[i])nonzero=true;}
    uint32_t expected=(uint32_t)bytes[44]|(uint32_t)bytes[45]<<8|(uint32_t)bytes[46]<<16|(uint32_t)bytes[47]<<24;
    return nonzero && ~crc==expected;
}
bool ks_restore_matches(const void *data, size_t size) {
    if(!ks_restore_available())return false;
    uint8_t digest[32];ks_restore_hash(data,size,digest);
    uint8_t difference=0;for(unsigned i=0;i<32;i++)difference|=digest[i]^ks_restore_image[12+i];
    return !difference;
}
void ks_restore_consumed(void){consumed=true;}
