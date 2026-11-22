// Corne OLEDs share encoder pins. Gate only the local display, and retain
// QMK's per-half pin assignments regardless of which half is the USB master.
static pin_t ksh_encoder_pin(uint8_t index, bool pad_b) {
    static const pin_t left_a[] = ENCODER_A_PINS, left_b[] = ENCODER_B_PINS;
#if defined(ENCODER_A_PINS_RIGHT) && defined(ENCODER_B_PINS_RIGHT)
    static const pin_t right_a[] = ENCODER_A_PINS_RIGHT, right_b[] = ENCODER_B_PINS_RIGHT;
    if (!is_keyboard_left()) {
        if (index >= sizeof(right_a) / sizeof(right_a[0])) return NO_PIN;
        return pad_b ? right_b[index] : right_a[index];
    }
#endif
    if (index >= sizeof(left_a) / sizeof(left_a[0])) return NO_PIN;
    return pad_b ? left_b[index] : left_a[index];
}
void encoder_quadrature_init_pin(uint8_t index, bool pad_b) {
    if (display_present()) return;
    pin_t pin = ksh_encoder_pin(index, pad_b);
    if (pin != NO_PIN) gpio_set_pin_input_high(pin);
}
uint8_t encoder_quadrature_read_pin(uint8_t index, bool pad_b) {
    if (display_present()) return 0;
    pin_t pin = ksh_encoder_pin(index, pad_b);
    return pin != NO_PIN && gpio_read_pin(pin);
}
