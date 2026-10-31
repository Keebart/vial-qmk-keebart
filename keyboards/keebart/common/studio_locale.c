#include "quantum.h"
#include "vial.h"
#include "usb_device_state.h"
#include "studio_macro.h"
#include "studio_reset.h"
#include "studio_rgb.h"
#include "locales/locale_protocol.h"
#include "locales/locale_runtime.h"
#include <string.h>
#include "atomic_util.h"
#include "locales/os_detection.h"
extern const uint8_t *ksm_locale_record(void);
extern bool ksm_save_locale(const uint8_t *);
static struct ksl_transaction transaction;
static struct ksl_runtime runtime;
static bool initialized;
static struct kso_capture os_capture;
static uint32_t os_applied_epoch=UINT32_MAX;
void ksl_qmk_descriptor(uint16_t length){
    ATOMIC_BLOCK_RESTORESTATE {kso_observe(&os_capture,length,timer_read32());}
}
void notify_usb_device_state_change_user(struct usb_device_state state){
    ATOMIC_BLOCK_RESTORESTATE {
        if(state.configure_state==USB_DEVICE_STATE_CONFIGURED)kso_configure(&os_capture,timer_read32());
        else if(state.configure_state<=USB_DEVICE_STATE_INIT)kso_reset(&os_capture);
    }
}
static void os_task(void){
    uint8_t os=0;uint32_t epoch=0;bool ready=false;
    ATOMIC_BLOCK_RESTORESTATE {ready=kso_ready(&os_capture,timer_read32());os=os_capture.os;epoch=os_capture.epoch;}
    if(!ready||os_applied_epoch==epoch||!is_keyboard_master()||vial_unlock_in_progress||ksr_reset_busy())return;
    os_applied_epoch=epoch;
    unsigned selected=ksl_os_computer(transaction.active,os);
    if(selected!=255)ksl_select(&runtime,transaction.active,selected);
}

static matrix_row_t previous_matrix[MATRIX_ROWS];
static uint16_t pressed_bindings[DYNAMIC_KEYMAP_LAYER_COUNT][MATRIX_ROWS][MATRIX_COLS];
static matrix_row_t cached_bindings[DYNAMIC_KEYMAP_LAYER_COUNT][MATRIX_ROWS];
static uint32_t previous;
static int64_t now;
static unsigned bypass;
static void replay(void *ctx,unsigned pos,uint16_t code,bool down,int64_t time){
    (void)ctx;(void)pos;(void)time;
    // Re-enter the normal action pipeline so overrides/repeat see the resolved
    // modifier state, while the locale guard prevents buffering it twice.
    g_vial_magic_keycode_override=code;bypass++;
    action_exec((keyevent_t){.type=KEY_EVENT,.key={.row=VIAL_MATRIX_MAGIC,.col=VIAL_MATRIX_MAGIC},.pressed=down,.time=timer_read()|1});
    bypass--;
}
static void action(void *ctx,uint16_t code,bool down){
    (void)ctx;bypass++;
    if(down)vial_keycode_down(code);else vial_keycode_up(code);
    bypass--;
}
static void pause_tap(void *ctx){(void)ctx;wait_ms(5);}
extern void clear_suppressed_override_mods(void);
extern void clear_weak_override_mods(void);
extern void set_suppressed_override_mods(uint8_t);
extern void set_weak_override_mods(uint8_t);
extern uint8_t ksl_suppressed_mods(void);
extern uint8_t ksl_override_mods(void);
static void tap(void *ctx,uint8_t key,uint8_t mods){
    (void)ctx;uint8_t saved=get_mods(),weak=get_weak_mods(),oneshot=get_oneshot_mods();
    uint8_t suppressed=ksl_suppressed_mods(),override=ksl_override_mods();
    clear_suppressed_override_mods();clear_weak_override_mods();clear_oneshot_mods();
    set_mods(mods);set_weak_mods(0);register_code(key);wait_ms(5);unregister_code(key);
    set_mods(saved);set_weak_mods(weak);
    set_suppressed_override_mods(suppressed);set_weak_override_mods(override);
    (void)oneshot;send_keyboard_report();
}
extern uint8_t ksl_effective_mods(void);
static uint8_t effective_mods(void *ctx){(void)ctx;return ksl_effective_mods();}
static unsigned hand(void *ctx,unsigned pos){
    (void)ctx;return pos<MATRIX_ROWS*MATRIX_COLS?(pos/MATRIX_COLS<MATRIX_ROWS/2?1:2):0;
}
static const struct ksl_io io={.tap=tap,.action=action,.mods=effective_mods,.pause=pause_tap,.replay=replay,.hand=hand};
#include "locales/repeat_port.inc"
static void init(void){if(initialized)return;const uint8_t *saved=ksm_locale_record();ksl_transaction_init(&transaction,saved);if(!saved)ksl_stock(transaction.active);ksl_runtime_init(&runtime,transaction.active);initialized=true;}
uint16_t ksl_qmk_binding(uint16_t code,uint8_t layer,uint8_t row,uint8_t col){
    init();
    if(layer>=DYNAMIC_KEYMAP_LAYER_COUNT||row>=MATRIX_ROWS||col>=MATRIX_COLS)return 0;
    matrix_row_t bit=(matrix_row_t)1<<col;
    if(cached_bindings[layer][row]&bit)return pressed_bindings[layer][row][col];
    uint16_t resolved=ksl_binding(transaction.active,runtime.active,code);
    if(matrix_is_on(row,col)){pressed_bindings[layer][row][col]=resolved;cached_bindings[layer][row]|=bit;}
    return resolved;
}
uint16_t ksl_qmk_encoder_binding(uint16_t code){init();return ksl_binding(transaction.active,runtime.active,code);}
static void clock_update(void){uint32_t current=timer_read32();now+=(uint32_t)(current-previous);previous=current;}
void ksl_qmk_reset(void){repeat_cancel();memset(cached_bindings,0,sizeof(cached_bindings));memset(previous_matrix,0,sizeof(previous_matrix));initialized=false;init();}
bool ksl_qmk_code(uint16_t code,bool down){
    if(!ksl_supported(code))return false;
    init();clock_update();
    if(!down)for(unsigned i=0;i<KSL_HELD;i++)if(runtime.held[i].used&&runtime.held[i].code==code)ksl_event(&runtime,transaction.active,runtime.held[i].position,code,false,now,effective_mods(NULL),host_keyboard_led_state().caps_lock,&io);
    ksl_event(&runtime,transaction.active,0x10000U+code,code,down,now,get_mods(),host_keyboard_led_state().caps_lock,&io);return true;
}
bool ksl_qmk_record(uint16_t code,keyrecord_t *record){
    if(bypass&&!ksl_supported(code))return true;
    init();clock_update();runtime.guarded=true;
    unsigned pos=record->event.key.row*MATRIX_COLS+record->event.key.col;
    if(record->event.key.row==VIAL_MATRIX_MAGIC)pos=0x10000U+code;
    return !ksl_event(&runtime,transaction.active,pos,code,record->event.pressed,now,get_mods(),host_keyboard_led_state().caps_lock,&io);
}
static bool persist(const uint8_t *record){return ksm_save_locale(record);}
void ksl_qmk_task(void){
    init();clock_update();
    bool guarded=false;
    for(unsigned row=0;row<MATRIX_ROWS;row++){
        matrix_row_t current=matrix_get_row(row),held=current|previous_matrix[row];guarded|=held!=0;
        for(unsigned layer=0;layer<DYNAMIC_KEYMAP_LAYER_COUNT;layer++)cached_bindings[layer][row]&=held;
        previous_matrix[row]=current;
    }
    runtime.guarded=guarded;
    if(usb_device_state_get_configure_state()==USB_DEVICE_STATE_CONFIGURED)os_task();
    if(usb_device_state_get_configure_state()!=USB_DEVICE_STATE_CONFIGURED){repeat_cancel();ksl_cancel(&runtime,&io);}
    if(!vial_unlock_in_progress&&!ksr_reset_busy()&&!guarded){
        unsigned active=runtime.active;
        bool pending=transaction.pending;ksl_commit(&transaction,persist);
        if(pending&&transaction.result==KSL_OK){ksl_cancel(&runtime,&io);ksl_runtime_init(&runtime,transaction.active);if(active<transaction.active[4])runtime.active=active;}
    }
    ksl_tick(&runtime,transaction.active,now,get_mods(),host_keyboard_led_state().caps_lock,&io);
}
void ksl_qmk_handle(uint8_t *data,bool internal){
    uint8_t input[32];memcpy(input,data,32);memset(data+6,0,26);
    if(memcmp(input,"\xcaKB\x01\xe2",5)||input[6]>1){data[6]=KSL_INVALID;return;}
    if(input[6]){
        if(internal||!is_keyboard_master()){data[6]=KSL_INVALID;return;}
        input[6]=0;uint8_t answer[32];
        if(!ksr_platform_rpc(input,answer)||memcmp(answer,input,6)){data[6]=KSL_IO;return;}
        memcpy(data+6,answer+6,26);return;
    }
    init();bool authorized=!vial_unlock_in_progress&&!ksr_reset_busy()&&(internal||is_keyboard_master());
    if(input[7]==6){
        for(unsigned row=0;row<MATRIX_ROWS;row++)runtime.guarded|=matrix_get_row(row)!=0;
        if(internal||!ksl_usb_select(&runtime,transaction.active,input+8,data+7,true,authorized))data[6]=KSL_INVALID;
        return;
    }
    if(input[7]==5){
        for(unsigned row=0;row<MATRIX_ROWS;row++)runtime.guarded|=matrix_get_row(row)!=0;
        if(input[8]!=255&&(!authorized||!ksl_select(&runtime,transaction.active,input[8]))){data[6]=KSL_INVALID;return;}
        data[7]=runtime.active;data[8]=runtime.switching;return;
    }
    data[6]=ksl_handle(&transaction,input+7,data+7,authorized);
}

void ksl_override_release(uint16_t code){if(!ksl_qmk_code(code,false))unregister_code(code);}
