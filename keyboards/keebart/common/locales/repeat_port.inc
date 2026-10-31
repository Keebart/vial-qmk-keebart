#include "locales/alt_repeat.h"
#include "dynamic_keymap.h"
static struct kar_state repeat_state;
static uint8_t repeat_weak,repeat_base;
extern bool ksm_running(void);
static void repeat_emit(void *ctx,unsigned pos,uint16_t code,uint8_t mods,bool down){
    (void)ctx;(void)pos;(void)mods;
    uint8_t next=kar_mods(&repeat_state);
    if(down){if(!repeat_weak)repeat_base=get_weak_mods();add_weak_mods(next);repeat_weak=next;}
    if(down)vial_keycode_down(code);else vial_keycode_up(code);
    if(!down){del_weak_mods(repeat_weak&~next&~repeat_base);add_weak_mods(next);repeat_weak=next;send_keyboard_report();}
}
bool kar_qmk_record(uint16_t code,keyrecord_t *record){
    struct kar_rule rules[KAR_COUNT]={0};
    if(code==KAR_ALT&&record->event.pressed){
        for(unsigned i=0;i<KAR_COUNT;i++){
            vial_alt_repeat_key_entry_t entry={0};
            if(!dynamic_keymap_get_alt_repeat_key(i,&entry))memcpy(&rules[i],&entry,6);
        }
    }
    unsigned pos=record->event.key.row*MATRIX_COLS+record->event.key.col;
    if(record->event.key.row==VIAL_MATRIX_MAGIC)pos=0x10000U+code;
    if(kar_event(&repeat_state,rules,pos,code,record->event.pressed,repeat_emit,NULL))return false;
    if(record->event.pressed&&!ksm_running()){
        if(code>=0x2000&&code<0x5000){if(!record->tap.count)return true;code&=255;}
        kar_note(&repeat_state,code,get_mods()|get_weak_mods()|get_oneshot_mods());
    }
    return true;
}
static void repeat_cancel(void){kar_cancel(&repeat_state,repeat_emit,NULL);}
