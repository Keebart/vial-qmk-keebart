#include "device_name.h"
#include <string.h>
static uint32_t rd(const uint8_t *p) { return p[0] | (uint32_t)p[1]<<8 | (uint32_t)p[2]<<16 | (uint32_t)p[3]<<24; }
static void wr(uint8_t *p,uint32_t n) { for(unsigned i=0;i<4;i++)p[i]=n>>(8*i); }
uint32_t kn_crc(const void *bytes,size_t size) { const uint8_t *p=bytes;uint32_t c=~0u;while(size--){c^=*p++;for(unsigned i=0;i<8;i++)c=(c>>1)^((c&1)?0xedb88320u:0);}return ~c; }
static bool codepoint(const uint8_t *p,size_t size,size_t *at,uint32_t *cp) {
    if(*at>=size)return false;
    uint8_t b=p[(*at)++];unsigned extra;uint32_t min;
    if(b<0x80){*cp=b;return b>=0x20 && b!=0x7f;}
    if(b>=0xc2 && b<=0xdf){extra=1;*cp=b&31;min=0x80;}
    else if(b>=0xe0 && b<=0xef){extra=2;*cp=b&15;min=0x800;}
    else if(b>=0xf0 && b<=0xf4){extra=3;*cp=b&7;min=0x10000;}else return false;
    while(extra--){if(*at>=size || (p[*at]&0xc0)!=0x80)return false;*cp=(*cp<<6)|(p[(*at)++]&63);}
    return *cp>=min && *cp<=0x10ffff && !(*cp>=0xd800 && *cp<=0xdfff) && !(*cp>=0x80 && *cp<=0x9f);
}
static bool text_valid(const uint8_t *p,size_t n) {
    if(!n || n>KN_MAX)return false;
    size_t at=0;uint32_t cp;bool visible=false;
    while(at<n){if(!codepoint(p,n,&at,&cp))return false;visible|=cp!=' ';}
    return visible;
}
bool kn_valid(const uint8_t *p,size_t n) {
    return n==KN_RECORD && !memcmp(p,"KSN1",4) && text_valid(p+9,p[8]) && rd(p+129)==kn_crc(p,129);
}
static void finish_record(uint8_t *p) { wr(p+129,kn_crc(p,129)); }
void kn_init(struct kn_state *s,const char *factory,const uint8_t *record) {
    memset(s,0,sizeof(*s));
    if(record && kn_valid(record,KN_RECORD)){memcpy(s->record,record,KN_RECORD);return;}
    size_t len=strlen(factory);if(!text_valid((const uint8_t *)factory,len)){factory="Keebart keyboard";len=strlen(factory);}
    memcpy(s->record,"KSN1",4);s->record[8]=(uint8_t)len;memcpy(s->record+9,factory,len);finish_record(s->record);
}
uint8_t kn_handle(struct kn_state *s,const uint8_t p[KN_PAYLOAD],uint8_t out[KN_PAYLOAD],bool authorized) {
    memset(out,0,KN_PAYLOAD);
    unsigned op=p[0];uint32_t tx=rd(p+1);
    unsigned used=op==0?2:op==1?10:op==2?7u+p[6]:5;
    if(op>4 || used>KN_PAYLOAD)return KN_INVALID;
    for(unsigned i=used;i<KN_PAYLOAD;i++)if(p[i])return KN_INVALID;
    if(op==0){unsigned at=p[1],len=s->record[8];if(at>=len)return KN_BOUNDS;unsigned n=len-at;if(n>22)n=22;out[0]=len;out[1]=n;out[2]=at;memcpy(out+3,s->record+9+at,n);return KN_OK;}
    if(op>4)return KN_INVALID;
    if(op!=4 && !authorized)return KN_BUSY;
    if(op==4){wr(out,s->transaction);out[4]=s->pending?KN_PENDING:s->result;wr(out+5,rd(s->record+129));return tx==s->transaction?KN_OK:KN_INVALID;}
    if(!tx)return KN_INVALID;
    if(s->pending)return KN_BUSY;
    if(op==1){if(!p[5] || p[5]>KN_MAX)return KN_BOUNDS;s->transaction=tx;s->length=p[5];s->crc=rd(p+6);memset(s->seen,0,sizeof(s->seen));memset(s->staged,0,sizeof(s->staged));s->staging=true;s->result=KN_OK;return KN_OK;}
    if(!s->staging || tx!=s->transaction)return KN_INVALID;
    if(op==2){unsigned at=p[5],n=p[6];if(!n || n>18 || at+n>s->length)return KN_BOUNDS;for(unsigned i=0;i<n;i++)if(s->seen[at+i] && s->staged[at+i]!=p[7+i])return KN_INVALID;memcpy(s->staged+at,p+7,n);memset(s->seen+at,1,n);return KN_OK;}
    for(unsigned i=0;i<s->length;i++)if(!s->seen[i])return KN_INVALID;
    if(kn_crc(s->staged,s->length)!=s->crc || !text_valid(s->staged,s->length))return KN_INVALID;
    s->staging=false;s->pending=true;s->result=KN_PENDING;return KN_PENDING;
}
void kn_task(struct kn_state *s,bool (*persist)(const uint8_t[KN_RECORD])) {
    if(!s->pending)return;
    uint8_t record[KN_RECORD]={0};memcpy(record,"KSN1",4);wr(record+4,rd(s->record+4)+1);record[8]=s->length;memcpy(record+9,s->staged,s->length);finish_record(record);
    if(persist(record)){memcpy(s->record,record,KN_RECORD);s->result=KN_OK;}else s->result=KN_IO;
    s->pending=false;
}
bool kn_reset(struct kn_state *s,const char *factory,bool (*persist)(const uint8_t[KN_RECORD])) {
    struct kn_state stock;kn_init(&stock,factory,NULL);
    if(!persist(stock.record))return false;
    *s=stock;return true;
}
size_t kn_usb_descriptor(const uint8_t record[KN_RECORD],uint8_t out[242]) {
    if(!kn_valid(record,KN_RECORD))return 0;
    size_t at=0,used=2;uint32_t cp;while(at<record[8]){if(!codepoint(record+9,record[8],&at,&cp))return 0;if(cp>0xffff){cp-=0x10000;uint16_t hi=0xd800+(cp>>10);out[used++]=hi;out[used++]=hi>>8;cp=0xdc00+(cp&1023);}out[used++]=cp;out[used++]=cp>>8;}out[0]=(uint8_t)used;out[1]=3;return used;
}
