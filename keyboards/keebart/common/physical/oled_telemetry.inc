// Display telemetry is volatile and independent of configuration persistence.
// One bounded snapshot replaces the original two synchronous RPCs per keypress.
#include "transactions.h"
#include "synchronization_util.h"
#include "studio_reset.h"
#include "locales/locale.h"
#include <stdio.h>

struct ksh_oled_state {
    uint32_t left, right, uptime;
    uint16_t keycode;
    uint8_t wpm, awake, computer;
};
_Static_assert(sizeof(struct ksh_oled_state) == 20, "OLED wire size");
static struct ksh_oled_state g_local, g_remote, g_display;
extern uint16_t ksl_qmk_display_code(uint16_t code);
extern bool ksl_qmk_display_replay(void);
extern unsigned ksl_qmk_display_computer(void);
extern void ksl_qmk_key_label(uint16_t,unsigned,char *,size_t);

static void ksh_oled_receive(uint8_t length, const void *data, uint8_t out_length, void *out) {
    (void)out_length; (void)out;
    if (length == sizeof(g_remote) && !is_keyboard_master()) memcpy(&g_remote, data, sizeof(g_remote));
}
void keyboard_post_init_user(void) {
    transaction_register_rpc(KSH_OLED_SYNC, ksh_oled_receive);
}
void ksh_oled_record(uint16_t code, keyrecord_t *record) {
    static uint32_t held[MATRIX_ROWS];
    if (!is_keyboard_master() || !IS_KEYEVENT(record->event) || ksl_qmk_display_replay() ||
        record->event.key.row >= MATRIX_ROWS || record->event.key.col >= MATRIX_COLS) return;
    unsigned row=record->event.key.row;uint32_t bit=1u<<record->event.key.col;
    if(!record->event.pressed){if(!matrix_is_on(row,record->event.key.col))held[row]&=~bit;return;}
    if((held[row]&bit)||!matrix_is_on(row,record->event.key.col))return;
    held[row]|=bit;
    if (row < MATRIX_ROWS / 2) g_local.left++;
    else g_local.right++;
    g_local.keycode = code;
    g_local.computer = ksl_qmk_display_computer();
}

bool wpm_keycode_user(uint16_t code) {
    // Buffered actions re-enter QMK; count the physical key only once.
    if (ksl_qmk_display_replay()) return false;
    code = ksl_qmk_display_code(code);
    return ksl_character(code) || (code >= KC_A && code <= KC_0) || (code >= KC_TAB && code <= KC_SLASH);
}
static void ksh_oled_snapshot(void) {
    if (is_keyboard_master()) {
        g_display = g_local;
        g_display.uptime = timer_read32();
        g_display.wpm = get_current_wpm();
        extern bool ksl_qmk_display_idle(void);
        g_display.awake = !ksl_qmk_display_idle();
    } else {
        // The serial callback runs on another thread. Never render under its lock.
        split_shared_memory_lock();
        g_display = g_remote;
        split_shared_memory_unlock();
    }
}
void housekeeping_task_user(void) {
    static uint32_t sent_at;
    if (!is_keyboard_master() || ksr_reset_busy() || timer_elapsed32(sent_at) < 500) return;
    uint8_t peer_display = is_keyboard_left() ? KSH_RIGHT_DISPLAY : KSH_LEFT_DISPLAY;
    if (!(ksh_flags() & peer_display)) return;
    sent_at = timer_read32();
    ksh_oled_snapshot();
    (void)transaction_rpc_send(KSH_OLED_SYNC, sizeof(g_display), &g_display);
}
static void ksh_oled_last_key(uint8_t row) {
    char utf8[192],text[192],window[11];
    ksl_qmk_key_label(g_display.keycode,g_display.computer,utf8,sizeof(utf8));
    // Preserve readable labels on the original single-byte OLED font.
    unsigned len=0;
    for(unsigned i=0;utf8[i]&&len<sizeof(text)-12;){
        uint32_t c=(unsigned char)utf8[i++];
        if(c>=128){unsigned n=(c&0xe0)==0xc0?1:(c&0xf0)==0xe0?2:3;c&=(1u<<(6-n))-1;while(n--&&utf8[i])c=(c<<6)|((unsigned char)utf8[i++]&63);}
        if(c<128)text[len++]=c;
        else if(c==0x2212)text[len++]='-';
        else if(c==0x2190)text[len++]='<';
        else if(c==0x2192)text[len++]='>';
        else if(c==0x25cc){memcpy(text+len,"dead",4);len+=4;}
        else len+=snprintf(text+len,sizeof(text)-len,"U+%04lX",(unsigned long)c);
    }
    text[len]=0;
    static char previous[192];static uint32_t changed;
    if(strcmp(previous,text)){strcpy(previous,text);changed=timer_read32();}
    unsigned offset=0;uint32_t elapsed=timer_elapsed32(changed);
    if(len>10&&elapsed>1200)offset=((elapsed-1200)/350)%(len+3);
    // Always overwrite the entire label area; OLED writes retain untouched cells.
    for(unsigned i=0;i<10;i++){unsigned at=len>10?(offset+i)%(len+3):i;window[i]=at<len?text[at]:' ';}
    window[10]=0;oled_set_cursor(0,row);
    oled_write(window,false);
}
