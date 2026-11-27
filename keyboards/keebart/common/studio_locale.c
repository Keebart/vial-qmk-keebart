#include "quantum.h"
#include "process_auto_shift.h"
#include "process_caps_word.h"
#include "caps_word.h"
#include "vial.h"
#include "dynamic_keymap.h"
#include "usb_device_state.h"
#include "studio_macro.h"
#include "studio_reset.h"
#include "physical/physical.h"
#include "studio_rgb.h"
#include "locales/locale_protocol.h"
#include "locales/locale_runtime.h"
#include "locales/power.h"
#include "eeconfig.h"
#include <string.h>
#include "atomic_util.h"
#include "locales/os_detection.h"
extern const uint8_t *ksm_locale_record(void);
extern bool ksm_save_locale(const uint8_t *);
extern uint32_t ksm_locale_generation(void);
static struct ksl_transaction transaction;
static struct ksl_runtime runtime;
static struct kspw_state power_state;
static bool initialized;
#include "rollover/qmk.inc"
static uint8_t peer_factor=255;
static uint32_t peer_factor_time;
bool ksl_qmk_display_idle(void);
#include "locales/rgb_port.inc"
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
static unsigned resolved_action;
static uint8_t record_layer;
static unsigned override_bypass;
uint8_t ksl_qmk_override_layer(void){return resolved_action?runtime.active_layer:record_layer;}
static void replay(void *ctx,unsigned pos,uint16_t code,bool down,int64_t time){
    (void)ctx;(void)pos;(void)time;
    // Re-enter the normal action pipeline so overrides/repeat see the resolved
    // modifier state, while the locale guard prevents buffering it twice.
    g_vial_magic_keycode_override=code;bypass++;
    action_exec((keyevent_t){.type=KEY_EVENT,.key={.row=VIAL_MATRIX_MAGIC,.col=VIAL_MATRIX_MAGIC},.pressed=down,.time=timer_read()|1});
    bypass--;
}
extern bool ks_pointer_action(uint16_t,bool);
extern void ks_pointer_stop_jiggle(void);
static void action(void *ctx,uint16_t code,bool down){
    if(ksn_action(&rollover,code,down))return;
    if(ks_pointer_action(code,down))return;
    if(rgb_action(code,down))return;
    // Resolved tap/hold keys still pass through overrides and Caps Word.
    // Auto Shift must not reinterpret a tap/hold decision already made here.
    resolved_action++;replay(ctx,0,code,down,0);resolved_action--;
}
static void pause_tap(void *ctx,uint16_t ms){(void)ctx;wait_ms(ms);}
extern void clear_suppressed_override_mods(void);
extern void clear_weak_override_mods(void);
extern void set_suppressed_override_mods(uint8_t);
extern void set_weak_override_mods(uint8_t);
extern uint8_t ksl_suppressed_mods(void);
extern uint8_t ksl_override_mods(void);
static bool character_report_bypass;
// Macro-held modifiers outlive a keypress. QMK clears weak typing modifiers at
// the next character, so keep macro ownership until the VM releases the key.
static uint8_t macro_modifiers;
void ksl_qmk_macro_mods(uint8_t mods){macro_modifiers=mods;}
static uint8_t with_macro_mods(uint8_t mods){return mods|(macro_modifiers&~ksl_suppressed_mods());}
uint8_t ksl_qmk_report_mods(uint8_t mods){return character_report_bypass?mods:ksl_character_report_mods(&runtime,with_macro_mods(mods));}
static void stroke(void *ctx,uint8_t key,bool down){
    (void)ctx;if(key){if(down)add_key(key);else del_key(key);}send_keyboard_report();
}
static uint16_t lookup(void *ctx,unsigned pos,uint16_t code){
    (void)ctx;if(pos>=MATRIX_ROWS*MATRIX_COLS)return code;
    unsigned mapped=runtime.extended.swap?kse_swap_position(transaction.active,pos):pos;
    for(int layer=DYNAMIC_KEYMAP_LAYER_COUNT-1;layer>=0;layer--)if((layer_state|default_layer_state)&((layer_state_t)1<<layer)){
        uint16_t binding=ksl_binding(transaction.active,runtime.active,dynamic_keymap_get_keycode(layer,mapped/MATRIX_COLS,mapped%MATRIX_COLS));
        if(binding!=1){record_layer=layer;return binding;}
    }
    return 0;
}
static uint8_t qmk_layer(void *ctx,unsigned pos){(void)ctx;(void)pos;return record_layer;}
static bool qmk_override(void *ctx,uint16_t code,uint8_t layer,bool down){
    (void)ctx;
#ifdef KEY_OVERRIDE_ENABLE
    if(override_bypass)return true;
    uint8_t prior=runtime.active_layer;runtime.active_layer=layer;resolved_action++;
    keyrecord_t rec={.event={.type=KEY_EVENT,.key={.row=VIAL_MATRIX_MAGIC,.col=VIAL_MATRIX_MAGIC},.pressed=down,.time=timer_read()|1}};
    bool pass=process_key_override(code,&rec);
    resolved_action--;runtime.active_layer=prior;
    return pass;
#else
    (void)code;(void)layer;(void)down;return true;
#endif
}
static void sequence(void *ctx,uint8_t key,uint8_t mods,bool end){
    (void)ctx;static bool active;static uint8_t saved,weak,oneshot,suppressed,override;
    if(end){if(!active)return;active=false;character_report_bypass=false;set_mods(saved);set_weak_mods(weak);set_oneshot_mods(oneshot);set_suppressed_override_mods(suppressed);set_weak_override_mods(override);send_keyboard_report();return;}
    if(!active){active=true;saved=get_mods();weak=get_weak_mods();oneshot=get_oneshot_mods();suppressed=ksl_suppressed_mods();override=ksl_override_mods();}
    character_report_bypass=true;clear_suppressed_override_mods();clear_weak_override_mods();clear_oneshot_mods();set_weak_mods(0);set_mods(mods);send_keyboard_report();
    if(key){register_code(key);wait_ms(5);unregister_code(key);}
}
static void tap(void *ctx,uint8_t key,uint8_t mods){
    (void)ctx;uint8_t saved=get_mods(),weak=get_weak_mods(),oneshot=get_oneshot_mods();
    uint8_t suppressed=ksl_suppressed_mods(),override=ksl_override_mods();
    clear_suppressed_override_mods();clear_weak_override_mods();clear_oneshot_mods();
    character_report_bypass=true;set_mods(mods);set_weak_mods(0);register_code(key);wait_ms(ksl_tap_delay(transaction.active,key));unregister_code(key);
    set_mods(saved);set_weak_mods(weak);
    set_suppressed_override_mods(suppressed);set_weak_override_mods(override);
    character_report_bypass=false;(void)oneshot;send_keyboard_report();
}
extern uint8_t ksl_effective_mods(void);
static uint8_t host_locks(void *ctx){(void)ctx;return host_keyboard_leds();}
static uint8_t effective_mods(void *ctx){(void)ctx;return with_macro_mods(ksl_effective_mods());}
static unsigned hand(void *ctx,unsigned pos){
    (void)ctx;return pos<MATRIX_ROWS*MATRIX_COLS?(pos/MATRIX_COLS<MATRIX_ROWS/2?1:2):0;
}
static bool typing_character(void *,uint16_t,bool,bool);
static void typing_cancel(void *);
static const struct ksl_io io;
static void unicode_character(void *ctx,uint32_t c){(void)ctx;ksl_emit_unicode(&runtime,transaction.active,c,now,&io);}
static const struct ksl_io io={.unicode=unicode_character,.tap=tap,.sequence=sequence,.character=typing_character,.cancel_typing=typing_cancel,.action=action,.mods=effective_mods,.locks=host_locks,.wait=pause_tap,.replay=replay,.hand=hand,.stroke=stroke,.lookup=lookup,.layer=qmk_layer,.override=qmk_override};
#include "locales/repeat_port.inc"
static void init(void){rollover_init();if(initialized)return;const uint8_t *saved=ksm_locale_record();ksl_transaction_init(&transaction,saved);if(!saved){if(!ksh_compact_locale(KSR_LAYOUT_ID,ksh_flags(),transaction.active))ksl_stock(transaction.active);}uint32_t saved_power[2]={eeconfig_read_kb(),eeconfig_read_user()};kspw_init(&power_state,transaction.active,(const uint8_t *)saved_power,ksm_locale_generation());ksl_runtime_init(&runtime,transaction.active);initialized=true;}
uint16_t ksl_qmk_tap_delay(uint16_t code){init();code=ksl_binding(transaction.active,runtime.active,code);const uint8_t *t=ksl_typing(transaction.active);return ksl_typing_size(transaction.active)==32&&(t[14]&2)?ksl_tap_delay(transaction.active,code):(code==57?80:0);}
uint16_t ksl_qmk_hold_term(void){init();return ksl_hold_term(transaction.active);}
bool ksl_qmk_layer_tap(uint16_t code){if(bypass||code<0x4100||code>0x4fff)return false;init();return ksl_managed_layer_tap(&runtime,code);}
uint8_t ksl_qmk_tapping_toggle(void){init();return ksl_tapping_toggle(transaction.active);}
uint8_t ksl_qmk_lighting_factor(void){init();if(!is_keyboard_master()&&timer_elapsed32(peer_factor_time)<2000)return peer_factor;uint32_t elapsed=last_input_activity_elapsed();return ksl_lighting_awake(transaction.active,elapsed,elapsed<kspw_idle_ms(transaction.active))?(unsigned)kse_brightness(transaction.active,elapsed,runtime.extended.dim_soon)*kspw_factor(transaction.active)/255:0;}
bool ksl_qmk_lighting_awake(void){return ksl_qmk_lighting_factor()!=0;}
uint16_t ksl_qmk_binding(uint16_t code,uint8_t layer,uint8_t row,uint8_t col){
    init();
    if(layer>=DYNAMIC_KEYMAP_LAYER_COUNT||row>=MATRIX_ROWS||col>=MATRIX_COLS)return 0;
    matrix_row_t bit=(matrix_row_t)1<<col;
    if(cached_bindings[layer][row]&bit)return pressed_bindings[layer][row][col];
    if(runtime.extended.swap){
        unsigned pos=kse_swap_position(transaction.active,row*MATRIX_COLS+col);
        if(pos<MATRIX_ROWS*MATRIX_COLS)code=dynamic_keymap_get_keycode(layer,pos/MATRIX_COLS,pos%MATRIX_COLS);
        if(code!=1&&runtime.extended.swap_once&&matrix_is_on(row,col)){runtime.extended.swap=false;runtime.extended.swap_once=false;}
    }
    uint16_t resolved=ksl_binding(transaction.active,runtime.active,code);
    if(matrix_is_on(row,col)){pressed_bindings[layer][row][col]=resolved;cached_bindings[layer][row]|=bit;}
    return resolved;
}
unsigned ksl_qmk_display_computer(void) { init(); return runtime.active; }
#ifdef OLED_ENABLE
// Read-only labels for the physical OLED; never emit a character or alter selection.
uint16_t ksl_qmk_display_code(uint16_t code) {
    init();
    code = ksl_binding(transaction.active, runtime.active, code);
    if (code >= KSL_COMPOUND && code < KSL_COMPOUND + KSL_ACTIONS) code = ksl_compound(transaction.active, code, 0);
    if ((code >= QK_MOD_TAP && code <= QK_MOD_TAP_MAX) ||
        (code >= QK_LAYER_TAP && code <= QK_LAYER_TAP_MAX) ||
        (code >= QK_MODS && code <= QK_MODS_MAX)) code &= 0xff;
    return code;
}
bool ksl_qmk_display_replay(void) { return bypass != 0; }
void ksl_qmk_key_label(uint16_t code,unsigned computer,char *out,size_t size) {
    init();
    code=ksl_binding(transaction.active,computer,code);
    uint16_t dance[5];vial_tap_dance_entry_t entry;
    const uint16_t *actions=NULL;
    if(code>=0x5700&&code<0x5700+VIAL_TAP_DANCE_ENTRIES&&dynamic_keymap_get_tap_dance(code-0x5700,&entry)==0){memcpy(dance,&entry,sizeof(dance));actions=dance;}
    ksl_key_label(transaction.active,computer,code,actions,out,size);
}

void ksl_qmk_layer_name(unsigned layer, char *out, size_t size) { init(); ksl_layer_name(transaction.active, layer, out, size); }
#endif
uint16_t ksl_qmk_encoder_binding(uint16_t code){init();return ksl_binding(transaction.active,runtime.active,code);}
static void clock_update(void){uint32_t current=timer_read32();now+=(uint32_t)(current-previous);previous=current;}
void ksl_qmk_reset(void){rollover=(struct ksn_mode){0};ks_pointer_stop_jiggle();repeat_cancel();memset(cached_bindings,0,sizeof(cached_bindings));memset(previous_matrix,0,sizeof(previous_matrix));eeconfig_update_kb(0);eeconfig_update_user(0);initialized=false;init();}
bool ksl_qmk_text(void *ctx,uint32_t c){(void)ctx;init();clock_update();return ksl_text(&runtime,transaction.active,c,now,&io);}
bool ksl_qmk_code(uint16_t code,bool down){
    if(!ksl_supported(code))return false;
    init();clock_update();
    override_bypass++;
    if(!down)for(unsigned i=0;i<KSL_HELD;i++)if(runtime.held[i].used&&runtime.held[i].code==code&&runtime.held[i].position<0x20000U){
        runtime.held[i].due=INT64_MAX;
        if(ksl_character(code)&&io.character)io.character(io.context,code,false,false);
    }
    ksl_event(&runtime,transaction.active,0x20000U+code,code,down,now,effective_mods(NULL),host_keyboard_led_state().caps_lock,&io);
    override_bypass--;
    return true;
}
static bool power_key(uint16_t code,bool down){
    if(kspw_key(code)){
        if(!vial_unlock_in_progress&&!ksr_reset_busy()&&!transaction.staging&&!transaction.pending) {
            if(down&&!rgb_matrix_is_enabled()&&(code==0x7820||code==0x7e20||code==0x7842||code==0x7840)){rgb_matrix_enable();code=0x7840;}
            uint32_t crc=ksl_crc(transaction.active,KSL_CRC_AT);
            kspw_action(&power_state,transaction.active,code,down,timer_read32());
            if(crc!=ksl_crc(transaction.active,KSL_CRC_AT))transaction.generation++;
        }
        return true;
    }
    return false;
}
bool ksl_qmk_record(uint16_t code,keyrecord_t *record){
    init();if(rgb_action(code,record->event.pressed))return false;
    if(power_key(code,record->event.pressed))return false;
    if(record->event.type==COMBO_EVENT)record_layer=get_highest_layer(layer_state|default_layer_state);
    else if(record->event.key.row!=VIAL_MATRIX_MAGIC)record_layer=read_source_layers_cache(record->event.key);
    if(bypass&&!ksl_supported(code))return !resolved_action || !ksl_extended_event(&runtime,transaction.active,0x10000U+code,code,record->event.pressed,now,effective_mods(NULL),host_keyboard_led_state().caps_lock,&io);
    init();clock_update();runtime.guarded=true;
    unsigned pos=record->event.key.row*MATRIX_COLS+record->event.key.col;
    // Combo output is a synthetic event at matrix (0,0); give it a synthetic
    // position so a queued replay does not re-resolve it from the layer binding.
    if(record->event.key.row==VIAL_MATRIX_MAGIC||record->event.type==COMBO_EVENT)pos=0x10000U+code;
    return !ksl_event(&runtime,transaction.active,pos,code,record->event.pressed,now,get_mods(),host_keyboard_led_state().caps_lock,&io);
}
static bool persist(const uint8_t *record){return ksm_save_locale(record);}
void ksl_qmk_task(void){
    rgb_task();
    init();clock_update();
    static uint32_t sent_at;static uint8_t sent_factor=255;
    uint8_t factor=ksl_qmk_lighting_factor();
    bool sync_factor=ksl_lighting_idle_seconds(transaction.active)||kse_options(transaction.active)[20]||kse_options(transaction.active)[21]||runtime.extended.dim_soon||sent_factor!=255||factor!=255||power_state.dirty;
    if(sync_factor&&is_keyboard_master()&&!ksr_reset_busy()&&!transaction.pending&&!transaction.staging&&timer_elapsed32(sent_at)>100&&(factor!=sent_factor||timer_elapsed32(sent_at)>1000)){
        uint8_t q[32]={0xca,'K','B',1,0xe2,0,0,8,factor,0x50,kspw_flags(transaction.active)&2,kspw_dim(transaction.active)},a[32];sent_at=timer_read32();
        if(ksr_platform_rpc(q,a)&&!memcmp(q,a,6)&&a[6]==KSL_OK)sent_factor=factor;
    }
    bool guarded=false;
    for(unsigned row=0;row<MATRIX_ROWS;row++){
        matrix_row_t current=matrix_get_row(row),held=current|previous_matrix[row];guarded|=held!=0;
        for(unsigned layer=0;layer<DYNAMIC_KEYMAP_LAYER_COUNT;layer++)cached_bindings[layer][row]&=held;
        previous_matrix[row]=current;
    }
    runtime.guarded=guarded;
    rollover_task(guarded);
    if(kspw_due(&power_state,timer_read32(),guarded||transaction.staging||transaction.pending||vial_unlock_in_progress||ksr_reset_busy())) {
        uint32_t record[2];kspw_record(&power_state,transaction.active,(uint8_t *)record);
        eeconfig_update_kb(record[0]);eeconfig_update_user(record[1]);power_state.dirty=false;
    }
    if(usb_device_state_get_configure_state()==USB_DEVICE_STATE_CONFIGURED)os_task();
    if(usb_device_state_get_configure_state()!=USB_DEVICE_STATE_CONFIGURED){repeat_cancel();ksl_cancel(&runtime,&io);}
    if(!vial_unlock_in_progress&&!ksr_reset_busy()&&!guarded){
        unsigned active=runtime.active;
        bool pending=transaction.pending;ksl_commit(&transaction,persist);
        if(pending&&transaction.result==KSL_OK){kspw_committed(&power_state,ksm_locale_generation());ksl_cancel(&runtime,&io);ksl_runtime_reload(&runtime,transaction.active);if(active<transaction.active[4])runtime.active=active;extern void vial_reload_studio_combos(void);vial_reload_studio_combos();}
    }
    ksl_tick(&runtime,transaction.active,now,get_mods(),host_keyboard_led_state().caps_lock,&io);
}
bool ksl_qmk_extra_combo(unsigned index,uint16_t out[5]) { init(); return ksl_extra_combo(transaction.active,index,out); }
void ksl_qmk_handle(uint8_t *data,bool internal){
    uint8_t input[32];memcpy(input,data,32);memset(data+6,0,26);
    if(memcmp(input,"\xcaKB\x01\xe2",5)||input[6]>1){data[6]=KSL_INVALID;return;}
    if(input[6]){
        if(internal||!is_keyboard_master()){data[6]=KSL_INVALID;return;}
        input[6]=0;uint8_t answer[32];
        if(!ksr_platform_rpc(input,answer)||memcmp(answer,input,6)){data[6]=KSL_IO;return;}
        memcpy(data+6,answer+6,26);return;
    }
    if(input[7]==8){if(!internal){data[6]=KSL_INVALID;return;}peer_factor=input[8];peer_factor_time=timer_read32();init();if(input[9]==0x50&&!transaction.staging&&!transaction.pending&&!vial_unlock_in_progress&&!ksr_reset_busy()&&kspw_set(&power_state,transaction.active,input[10],input[11],timer_read32()))transaction.generation++;data[6]=KSL_OK;return;}
    init();bool authorized=!vial_unlock_in_progress&&!ksr_reset_busy()&&(internal||is_keyboard_master());
    if(input[7]==9){
        if(internal){data[6]=KSL_INVALID;return;}
        for(unsigned i=8;i<32;i++)if(input[i]){data[6]=KSL_INVALID;return;}
        memcpy(data+7,transaction.active+KSL_CRC_AT,4);
        data[11]=rgb_matrix_is_enabled();data[12]=rgb_matrix_get_mode();data[13]=rgb_matrix_get_hue();
        data[14]=rgb_matrix_get_sat();data[15]=rgb_matrix_get_val();data[16]=rgb_matrix_get_speed();return;
    }
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

const uint8_t *ksl_qmk_typing(void){init();return ksl_typing(transaction.active);}
#define TYPING_RECORD() transaction.active
#define TYPING_COMPUTER() runtime.active
#define TYPING_RESOLVED() (resolved_action!=0)
#define TYPING_MODS() effective_mods(NULL)
#define TYPING_PRESS(source,code,mods) ksl_character_press(&runtime,transaction.active,source,code,mods,host_keyboard_led_state().caps_lock,&io)
#define TYPING_RELEASE(code) ksl_character_release(&runtime,code,&io)
#define TYPING_EMIT(code,mods) ksl_emit_character(transaction.active,runtime.active,code,mods,host_keyboard_led_state().caps_lock,&io)
static void typing_begin(void){
    // Repeat owns its injected modifiers; only clear stale typing modifiers.
    uint8_t repeated=get_weak_mods()&repeat_weak;
    clear_weak_mods();add_weak_mods(repeated);
}
#define TYPING_BEGIN() typing_begin()
#define TYPING_WORD(code,record) process_caps_word(code,record)
#define TYPING_SCHEDULE() ((void)0)
#include "locales/typing_port.inc"
extern void ksl_qmk_auto_shift_cancel(void);
static void typing_cancel(void *ctx){(void)ctx;ksl_qmk_auto_shift_cancel();caps_word_off();}
bool ksl_qmk_auto_shift_key(uint16_t code){init();return typing_auto_key(code);}
uint16_t get_autoshift_timeout(uint16_t code,keyrecord_t *record){
    (void)code;(void)record;const uint8_t *t=ksl_qmk_typing();return t[2]|(uint16_t)t[3]<<8;
}

bool ksl_qmk_display_idle(void){init();uint32_t idle=kspw_idle_ms(transaction.active);return kspw_display_off(transaction.active)&&idle!=UINT32_MAX&&last_input_activity_elapsed()>=idle;}

// Name and macro-bank saves carry the current master lighting state with them.
const uint8_t *ksl_qmk_persisted_locale(void){init();return transaction.active;}
void ksl_qmk_persisted(void){kspw_committed(&power_state,ksm_locale_generation());}
