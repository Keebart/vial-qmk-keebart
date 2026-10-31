#include "macro_store.h"
#include "locales/locale.h"
#include <string.h>
#define MAGIC 0x334d534bU
#define LEGACY_LOCALE_MAGIC 0x344d534bU
#define LOCALE_MAGIC 0x354d534bU
_Static_assert(KM_LOCALE_OFFSET>=12+KM_BYTES && KM_LOCALE_OFFSET+KSL_BYTES<=KM_SLOT_SIZE-KM_PAGE_SIZE,"Locale metadata must fit existing macro slots");
_Static_assert(12+KM_BYTES<=KM_SLOT_SIZE-KM_PAGE_SIZE,"Macro data must not overlap name metadata");
static uint32_t rd(const uint8_t *p){return p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static void wr(uint8_t *p,uint32_t n){for(unsigned i=0;i<4;i++)p[i]=n>>(8*i);}
static uint32_t crc(const uint8_t *p,size_t n){uint32_t c=~0U;while(n--){c^=*p++;for(unsigned i=0;i<8;i++)c=(c>>1)^((c&1)?0xedb88320U:0);}return ~c;}
uint32_t km_store_generation(const uint8_t *p){return rd(p+4);}
bool km_store_valid(const uint8_t *p){return ((rd(p)==MAGIC && rd(p+8)==(crc(p+12,KM_BYTES)^rd(p+4))) || (rd(p)==LEGACY_LOCALE_MAGIC && ksl_legacy_valid(p+KM_LOCALE_OFFSET) && rd(p+8)==(crc(p+12,KM_BYTES)^crc(p+KM_LOCALE_OFFSET,508)^rd(p+4))) || (rd(p)==LOCALE_MAGIC && ksl_valid(p+KM_LOCALE_OFFSET) && rd(p+8)==(crc(p+12,KM_BYTES)^crc(p+KM_LOCALE_OFFSET,KSL_CRC_AT)^rd(p+4)))) && km_valid(p+12,KM_BYTES,KM_COUNT);}
int km_store_select(const struct km_storage *s){bool a=km_store_valid(s->read(0)),b=km_store_valid(s->read(1));if(!a&&!b)return -1;if(a&&b)return (int32_t)(km_store_generation(s->read(1))-km_store_generation(s->read(0)))>0?1:0;return a?0:1;}
bool km_store_save(const struct km_storage *s,int previous,const uint8_t *bytes){return km_store_save_named(s,previous,bytes,NULL);}
bool km_store_save_named(const struct km_storage *s,int previous,const uint8_t *bytes,const uint8_t *name){return km_store_save_full(s,previous,bytes,name,NULL);}
bool km_store_save_full(const struct km_storage *s,int previous,const uint8_t *bytes,const uint8_t *name,const uint8_t *locale){
    if(!km_valid(bytes,KM_BYTES,KM_COUNT))return false;
    unsigned target=previous==0?1:0;
    uint32_t generation=previous<0?1:km_store_generation(s->read(previous))+1;
    uint8_t metadata[KM_NAME_BYTES];memset(metadata,255,sizeof(metadata));
    if(name)memcpy(metadata,name,sizeof(metadata));else if(previous>=0)memcpy(metadata,s->read(previous)+KM_SLOT_SIZE-KM_PAGE_SIZE,sizeof(metadata));
    static _Alignas(4) uint8_t language[KSL_BYTES];
    if(locale)memcpy(language,locale,KSL_BYTES);else if(previous<0)ksl_stock(language);else if(!ksl_upgrade(language,s->read(previous)+KM_LOCALE_OFFSET))ksl_default(language);
    if(!ksl_valid(language)||!s->erase(target))return false;
    _Alignas(4) uint8_t page[KM_PAGE_SIZE];
    for(unsigned at=0;at<12+KM_BYTES;at+=KM_PAGE_SIZE){
        memset(page,255,sizeof(page));
        for(unsigned i=0;i<KM_PAGE_SIZE;i++)if(at+i>=12 && at+i<12+KM_BYTES)page[i]=bytes[at+i-12];
        if(!s->program(target,at,page))return false;
    }
    if(memcmp(s->read(target)+12,bytes,KM_BYTES))return false;
    bool named=false;for(unsigned i=0;i<sizeof(metadata);i++)named|=metadata[i]!=255;
    if(named){memset(page,255,sizeof(page));memcpy(page,metadata,sizeof(metadata));if(!s->program(target,KM_SLOT_SIZE-KM_PAGE_SIZE,page))return false;}
    for(unsigned at=0;at<KSL_BYTES;at+=KM_PAGE_SIZE)if(!s->program(target,KM_LOCALE_OFFSET+at,language+at))return false;
    if(memcmp(s->read(target)+KM_LOCALE_OFFSET,language,KSL_BYTES))return false;
    memset(page,255,sizeof(page));wr(page,LOCALE_MAGIC);wr(page+4,generation);wr(page+8,crc(bytes,KM_BYTES)^crc(language,KSL_CRC_AT)^generation);memcpy(page+12,bytes,KM_PAGE_SIZE-12);
    return s->program(target,0,page) && km_store_valid(s->read(target));
}
