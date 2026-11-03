#include "locales/rgb_action.h"
static bool rgb_pending;
static uint32_t rgb_generation;
static uint16_t rgb_pending_color[3];
static void rgb_task(void){
 if(!rgb_pending||ksr.job)return;
 rgb_pending=false;
 if(ksr.result==KSR_OK&&ksr.active_generation==rgb_generation){
  rgb_matrix_sethsv(rgb_pending_color[0]*255/360,rgb_pending_color[1]*255/100,rgb_pending_color[2]*RGB_MATRIX_MAXIMUM_BRIGHTNESS/100);
  rgb_matrix_enable();
 }
}
static bool rgb_action(uint16_t code,bool down){
 if(code<0x7f40||code>0x7f5f)return false;
 uint16_t color[3];if(!down||!ksl_rgb(transaction.active,code,color))return true;
 if(ksr.job||ksr.remote_save||rgb_pending||ksr_reset_busy())return true;
 if(ksr.stage.open&&(!ksr.stage.valid||ksr.stage.generation!=ksr.active_generation||ksr.stage.crc!=ksr.active_crc))return true;
 uint8_t rgb[3];ksl_rgb_color(color,rgb);
 ksr_begin(&ksr.stage,ksr.length,1,ksr.active_generation+1,0);
 memcpy(ksr.stage.data,ksr.active,ksr.length);
 for(unsigned i=0;i<KSR_RGB_LENGTH(ksr.leds);i+=3)memcpy(ksr.stage.data+i,rgb,3);
 ksr.stage.crc=ksr_crc32(ksr.stage.data,ksr.length);
 memset(ksr.stage.seen,255,sizeof(ksr.stage.seen));ksr.stage.received=ksr.length;
 if(ksr_validate(&ksr.stage)!=KSR_OK)return true;
 uint8_t req[32]={KSR_COMMAND,'K','B',KSR_VERSION,KSR_SAVE},out[32];
 ksr_handle(req,out,true,false);
 if(out[6]==KSR_PENDING){rgb_pending=true;rgb_generation=ksr.stage.generation;memcpy(rgb_pending_color,color,sizeof(color));}
 return true;
}
