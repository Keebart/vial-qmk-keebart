/*
 * Keebart Horizon — Default OLED display preset (64x128 portrait).
 *
 * Left OLED:  Minimal status totem (rounded layer pill, layer pips, 2x2 live
 *             modifier keycaps, whisper-quiet lock indicators, reactive key chip).
 * Right OLED: Living moonlit alpine horizon (crescent moon, twinkling stars,
 *             drifting clouds, shooting stars, layered alpine peaks with moonlit
 *             rims, pixel pines, shimmering lake reflections, and rising typing
 *             fireflies).
 */
#include "oled_canvas.h"

#define KSO_PRESET_ID 0u

/* 9x9 modifier pixel icons (MSB-first, 2 bytes per row) */
static const uint8_t ICON_SHIFT_9X9[18] = {
    0x08, 0x00, /* ....#.... */
    0x1c, 0x00, /* ...###... */
    0x36, 0x00, /* ..##.##.. */
    0x63, 0x00, /* .##...##. */
    0xc1, 0x80, /* ##.....## */
    0xf7, 0x80, /* ####.#### */
    0x14, 0x00, /* ...#.#... */
    0x14, 0x00, /* ...#.#... */
    0x1c, 0x00, /* ...###... */
};

static const uint8_t ICON_CTRL_9X7[14] = {
    0x08, 0x00, /* ....#.... */
    0x1c, 0x00, /* ...###... */
    0x36, 0x00, /* ..##.##.. */
    0x63, 0x00, /* .##...##. */
    0xc1, 0x80, /* ##.....## */
    0x80, 0x80, /* #.......# */
    0x00, 0x00,
};

static const uint8_t ICON_ALT_9X7[14] = {
    0xe0, 0x80, /* ###.....# */
    0x30, 0x00, /* ..##..... */
    0x18, 0x00, /* ...##.... */
    0x0c, 0x00, /* ....##... */
    0x06, 0x00, /* .....##.. */
    0x03, 0x80, /* ......### */
    0x00, 0x00,
};

static const uint8_t ICON_GUI_9X9[18] = {
    0x63, 0x00, /* .##...##. */
    0x94, 0x80, /* #..#.#..# */
    0x7f, 0x00, /* .#######. */
    0x14, 0x00, /* ...#.#... */
    0x14, 0x00, /* ...#.#... */
    0x14, 0x00, /* ...#.#... */
    0x7f, 0x00, /* .#######. */
    0x94, 0x80, /* #..#.#..# */
    0x63, 0x00, /* .##...##. */
};

/* 7x7 lock icons (1 byte per row, bits 7..1) */
static const uint8_t ICON_CAPS_7X7[7] = {
    0x10, /* ...#... */
    0x38, /* ..###.. */
    0x6c, /* .##.##. */
    0xc6, /* ##...## */
    0x28, /* ..#.#.. */
    0x38, /* ..###.. */
    0x7c, /* .#####. */
};

static const uint8_t ICON_NUM_7X7[7] = {
    0x28, /* ..#.#.. */
    0xfe, /* ####### */
    0x28, /* ..#.#.. */
    0x28, /* ..#.#.. */
    0xfe, /* ####### */
    0x28, /* ..#.#.. */
    0x00,
};

static const uint8_t ICON_SCRL_7X7[7] = {
    0x10, /* ...#... */
    0x38, /* ..###.. */
    0x7c, /* .#####. */
    0x10, /* ...#... */
    0x7c, /* .#####. */
    0x38, /* ..###.. */
    0x10, /* ...#... */
};

/* Minimal corner-bracket keycap outline for calm inactive modifiers. */
static void draw_subtle_keycap(kso_canvas_t *c, int x, int y, int w, int h) {
    int x1 = x + w - 1;
    int y1 = y + h - 1;
    /* Top-left corner */
    kso_hline(c, x + 2, y, 4, true);
    kso_px(c, x + 1, y + 1, true);
    kso_vline(c, x, y + 2, 4, true);
    /* Top-right corner */
    kso_hline(c, x1 - 5, y, 4, true);
    kso_px(c, x1 - 1, y + 1, true);
    kso_vline(c, x1, y + 2, 4, true);
    /* Bottom-left corner */
    kso_hline(c, x + 2, y1, 4, true);
    kso_px(c, x + 1, y1 - 1, true);
    kso_vline(c, x, y1 - 5, 4, true);
    /* Bottom-right corner */
    kso_hline(c, x1 - 5, y1, 4, true);
    kso_px(c, x1 - 1, y1 - 1, true);
    kso_vline(c, x1, y1 - 5, 4, true);
}

static void draw_mod_tile(
    kso_canvas_t *c, int x, int y, bool active, const uint8_t *icon, int iw, int ih
) {
    const int w = 26, h = 19;
    if (active) {
        kso_fill_round_rect(c, x, y, w, h, 3, true);
        kso_blit_mask(c, x + (w - iw) / 2, y + (h - ih) / 2, iw, ih, icon, false);
    } else {
        draw_subtle_keycap(c, x, y, w, h);
        kso_blit_mask(c, x + (w - iw) / 2, y + (h - ih) / 2, iw, ih, icon, true);
    }
}

static void draw_lock_slot(
    kso_canvas_t *c, int cx, int cy, bool active, bool pulsing_outline, const uint8_t *icon, uint32_t now_ms
) {
    if (active) {
        kso_fill_round_rect(c, cx - 8, cy - 6, 17, 13, 3, true);
        kso_blit_mask(c, cx - 3, cy - 3, 7, 7, icon, false);
    } else if (pulsing_outline) {
        bool blink = ((now_ms / 420u) & 1u) == 0u;
        kso_round_rect(c, cx - 8, cy - 6, 17, 13, 3, blink);
        if (!blink) draw_subtle_keycap(c, cx - 8, cy - 6, 17, 13);
        kso_blit_mask(c, cx - 3, cy - 3, 7, 7, icon, true);
    } else {
        kso_fill_rect(c, cx - 1, cy - 1, 2, 2, true);
    }
}

static void render_left(kso_canvas_t *c, const kso_state_t *s) {
    static char prev_layer[33];
    static uint32_t layer_changed_ms;
    static char prev_key[33];
    static uint32_t key_changed_ms;

    if (strcmp(prev_layer, s->layer_name) != 0) {
        strncpy(prev_layer, s->layer_name, sizeof(prev_layer) - 1);
        prev_layer[sizeof(prev_layer) - 1] = '\0';
        layer_changed_ms = s->now_ms;
    }
    if (strcmp(prev_key, s->last_key) != 0) {
        strncpy(prev_key, s->last_key, sizeof(prev_key) - 1);
        prev_key[sizeof(prev_key) - 1] = '\0';
        key_changed_ms = s->now_ms;
    }

    /* 1. Active Layer Pill (inverted rounded hero pill) */
    kso_fill_round_rect(c, 3, 4, 58, 17, 4, true);
    const char *lname = s->layer_name[0] ? s->layer_name : "BASE";
    kso_draw_marquee_centered(c, 8, 48, 9, lname, s->now_ms - layer_changed_ms, false);

    /* 2. Geometric Layer Pips (8 slots, active layer becomes elongated pill) */
    uint8_t base = s->layer >= 8u ? (uint8_t)(s->layer - 7u) : 0u;
    int px = 7;
    for (uint8_t i = 0; i < 8u; i++) {
        uint8_t idx = (uint8_t)(base + i);
        if (idx == s->layer) {
            kso_fill_round_rect(c, px, 26, 9, 3, 1, true);
            px += 12;
        } else {
            kso_fill_rect(c, px, 27, 2, 2, true);
            px += 6;
        }
    }

    /* Whisper-thin dotted divider */
    for (int x = 10; x <= 53; x += 3) kso_px(c, x, 35, true);

    /* 3. Sculptural 2x2 Live Modifier Keycaps */
    draw_mod_tile(c, 4, 41, (s->mods & KSO_MOD_SHIFT) != 0, ICON_SHIFT_9X9, 9, 9);
    draw_mod_tile(c, 34, 41, (s->mods & KSO_MOD_CTRL) != 0, ICON_CTRL_9X7, 9, 7);
    draw_mod_tile(c, 4, 63, (s->mods & KSO_MOD_ALT) != 0, ICON_ALT_9X7, 9, 7);
    draw_mod_tile(c, 34, 63, (s->mods & KSO_MOD_GUI) != 0, ICON_GUI_9X9, 9, 9);

    /* 4. Minimal Lock Indicator Trio (Caps/CapsWord, Num, Scroll) */
    bool caps = (s->locks & KSO_LOCK_CAPS) != 0;
    bool caps_word = (s->locks & KSO_LOCK_CAPS_WORD) != 0;
    bool num = (s->locks & KSO_LOCK_NUM) != 0;
    bool scrl = (s->locks & KSO_LOCK_SCROLL) != 0;
    draw_lock_slot(c, 14, 94, caps, !caps && caps_word, ICON_CAPS_7X7, s->now_ms);
    draw_lock_slot(c, 32, 94, num, false, ICON_NUM_7X7, s->now_ms);
    draw_lock_slot(c, 50, 94, scrl, false, ICON_SCRL_7X7, s->now_ms);

    /* 5. Reactive Last-Key Chip */
    uint32_t since_press = s->now_ms - s->last_press_ms;
    bool flash = s->last_key[0] != '\0' && s->last_press_ms != 0 && since_press < 180u;
    if (flash) {
        kso_fill_round_rect(c, 4, 107, 56, 17, 3, true);
        kso_draw_marquee_centered(c, 8, 48, 112, s->last_key, s->now_ms - key_changed_ms, false);
    } else if (s->last_key[0] != '\0') {
        kso_round_rect(c, 4, 107, 56, 17, 3, true);
        kso_draw_marquee_centered(c, 8, 48, 112, s->last_key, s->now_ms - key_changed_ms, true);
    } else {
        draw_subtle_keycap(c, 4, 107, 56, 17);
        kso_fill_rect(c, 25, 115, 2, 2, true);
        kso_fill_rect(c, 31, 115, 2, 2, true);
        kso_fill_rect(c, 37, 115, 2, 2, true);
    }
}

/* --- Right OLED: Living Moonlit Alpine Horizon --- */

typedef struct {
    int16_t x;
    int16_t y;
    uint32_t born_ms;
    uint8_t seed;
} kso_mote_t;

static void draw_cloud(kso_canvas_t *c, int x, int y, bool large) {
    /* Soft cumulus cloud silhouette with double puff, wrapping across 64px width. */
    if (large) {
        static const char *const ROWS[6] = {
            ".....####..........",
            "...########........",
            "..###########.##...",
            ".################..",
            "##################.",
            ".################..",
        };
        for (int r = 0; r < 6; r++) {
            for (int dx = 0; ROWS[r][dx]; dx++) {
                if (ROWS[r][dx] == '#') {
                    int px = (x + dx) & 63;
                    /* Moonlit top rim solid, delicate evening dither near bottom */
                    bool lit = (r <= 2) || (r == 3 && ((px + r) & 1) == 0) ||
                               (r == 4 && ((px + r) & 3) != 0) || (r == 5 && (px & 1) == 0);
                    if (lit) kso_px(c, px, y + r, true);
                    else kso_px(c, px, y + r, false);
                }
            }
        }
    } else {
        static const char *const ROWS[4] = {
            "...####......",
            ".#########...",
            "############.",
            ".##########..",
        };
        for (int r = 0; r < 4; r++) {
            for (int dx = 0; ROWS[r][dx]; dx++) {
                if (ROWS[r][dx] == '#') {
                    int px = (x + dx) & 63;
                    bool lit = (r <= 1) || (r == 2 && ((px + r) & 1) == 0) || (r == 3 && (px & 1) == 0);
                    if (lit) kso_px(c, px, y + r, true);
                    else kso_px(c, px, y + r, false);
                }
            }
        }
    }
}

static int far_mountain_y(int x) {
    /* Dramatic three-peak alpine skyline across x = 0..63 */
    if (x < 13) return 74 - (x * 11) / 13;          /* rises to peak 1 (13, 63) */
    if (x < 22) return 63 + ((x - 13) * 10) / 9;    /* dips to saddle (22, 73) */
    if (x < 35) return 73 - ((x - 22) * 20) / 13;   /* climbs to main peak (35, 53) */
    if (x < 48) return 53 + ((x - 35) * 18) / 13;   /* descends to saddle (48, 71) */
    if (x < 56) return 71 - ((x - 48) * 8) / 8;     /* rises to east peak (56, 63) */
    return 63 + ((x - 56) * 12) / 8;                /* descends to edge (63, 75) */
}

static int mid_ridge_y(int x) {
    /* Closer rolling foothill ridge across x = 0..63 */
    if (x < 18) return 79 + (x * 6) / 18;           /* 79 -> 85 */
    if (x < 30) return 85 - ((x - 18) * 7) / 12;    /* 85 -> 78 */
    if (x < 46) return 78 + ((x - 30) * 9) / 16;    /* 78 -> 87 */
    return 87 - ((x - 46) * 6) / 18;                /* 87 -> 81 */
}

static void draw_pine(kso_canvas_t *c, int cx, int base_y, int height) {
    /* Tiered alpine evergreen: crisp dark core + bright moonlight rim on right. */
    for (int i = 0; i < height; i++) {
        int y = base_y - height + i;
        int tier = i % 3;
        int half_w = (i / 3) + (tier == 2 ? 1 : 0);
        if (i == 0) half_w = 0;
        if (i == height - 1) half_w = 0; /* trunk */
        kso_hline(c, cx - half_w, y, half_w * 2 + 1, false);
        kso_px(c, cx + half_w, y, true);
        if (i == 0 || (i < height - 1 && tier == 2)) {
            kso_px(c, cx - half_w, y, true);
        }
    }
}

static void render_right(kso_canvas_t *c, const kso_state_t *s) {
    static uint32_t seen_presses;
    static kso_mote_t motes[6];
    static uint8_t next_mote;
    static uint32_t comet_start_ms;
    static uint8_t comet_lane;
    static uint32_t cloud_phase_x256;
    static uint32_t last_frame_ms;

    uint32_t total_presses = s->left_presses + s->right_presses;
    if (last_frame_ms == 0 || s->now_ms < last_frame_ms) {
        last_frame_ms = s->now_ms;
        seen_presses = total_presses;
    }
    uint32_t dt = s->now_ms - last_frame_ms;
    if (dt > 250u) dt = 250u;
    last_frame_ms = s->now_ms;

    /* Cloud drift gently responds to typing speed without looking frantic. */
    uint32_t breeze = 16u + (uint32_t)s->wpm * 2u;
    cloud_phase_x256 += (dt * breeze) / 120u;

    /* Spawn a gentle rising firefly mote on every new keypress. */
    if (total_presses > seen_presses) {
        uint32_t delta = total_presses - seen_presses;
        if (delta > 3u) delta = 3u;
        for (uint32_t k = 0; k < delta; k++) {
            uint8_t seed = (uint8_t)((total_presses + k) * 37u + (s->now_ms & 31u));
            motes[next_mote].x = (int16_t)(8 + (seed % 48));
            motes[next_mote].y = 92;
            motes[next_mote].born_ms = s->now_ms;
            motes[next_mote].seed = seed;
            next_mote = (uint8_t)((next_mote + 1u) % 6u);
        }
        seen_presses = total_presses;
        /* Trigger a graceful shooting star during brisk typing bursts. */
        if ((s->wpm >= 30u || (total_presses % 12u) == 0u) &&
            (comet_start_ms == 0 || s->now_ms - comet_start_ms > 4500u)) {
            comet_start_ms = s->now_ms;
            comet_lane = (uint8_t)(total_presses % 3u);
        }
    }

    /* 1. Starfield Sky (14 twinkling stars at varied altitudes) */
    static const uint8_t STARS[14][3] = {
        {7, 6, 17},   {18, 11, 29}, {29, 5, 23},  {11, 21, 31},
        {24, 24, 19}, {36, 19, 27}, {5, 34, 21},  {15, 39, 37},
        {31, 36, 25}, {58, 28, 33}, {9, 49, 29},  {22, 52, 23},
        {42, 42, 35}, {56, 46, 19},
    };
    for (unsigned i = 0; i < 14u; i++) {
        int sx = STARS[i][0];
        int sy = STARS[i][1];
        uint32_t period = (uint32_t)STARS[i][2] * 95u;
        uint32_t phase = ((s->now_ms + i * 410u) % period) * 16u / period;
        if (phase >= 3u) {
            kso_px(c, sx, sy, true);
        }
        if (phase == 13u || phase == 14u) {
            /* Crisp 3x3 starlight sparkle */
            kso_px(c, sx - 1, sy, true);
            kso_px(c, sx + 1, sy, true);
            kso_px(c, sx, sy - 1, true);
            kso_px(c, sx, sy + 1, true);
        }
    }

    /* 2. Waxing Crescent Moon (upper-right sky) */
    kso_fill_circle(c, 48, 16, 8, true);
    kso_fill_circle(c, 44, 14, 7, false);

    /* 3. Shooting Star (when triggered by typing rhythm) */
    if (comet_start_ms != 0 && s->now_ms >= comet_start_ms) {
        uint32_t age = s->now_ms - comet_start_ms;
        if (age < 720u) {
            int start_x = comet_lane == 0 ? 4 : (comet_lane == 1 ? 10 : 2);
            int start_y = comet_lane == 0 ? 10 : (comet_lane == 1 ? 6 : 18);
            int dist = (int)(age * 46u / 720u);
            int hx = start_x + dist;
            int hy = start_y + dist / 3;
            kso_px(c, hx, hy, true);
            kso_px(c, hx - 1, hy, true);
            for (int t = 2; t <= 11; t++) {
                if (t <= 6 || (t & 1) == 0) {
                    kso_px(c, hx - t, hy - t / 3, true);
                }
            }
        }
    }

    /* 4. Drifting Soft Cumulus Clouds */
    int c1 = (int)((cloud_phase_x256 >> 8) & 63u);
    int c2 = (int)(((cloud_phase_x256 >> 9) + 36u) & 63u);
    draw_cloud(c, c1, 28, true);
    draw_cloud(c, c2, 41, false);

    /* 5. Far Alpine Mountain Peaks (moonlit right slopes, dark shadowed left slopes) */
    for (int x = 0; x < (int)KSO_WIDTH; x++) {
        int my = far_mountain_y(x);
        int ny = x + 1 < (int)KSO_WIDTH ? far_mountain_y(x + 1) : my;
        bool right_facing = ny > my;
        /* Clear sky behind mountain body */
        for (int y = my; y < 98; y++) {
            kso_px(c, x, y, false);
        }
        /* Crisp ridge crest */
        kso_px(c, x, my, true);
        if (right_facing) {
            /* Bright moonlight rim catching the east-facing slope */
            kso_px(c, x, my + 1, true);
            for (int y = my + 2; y < my + 9 && y < 98; y++) {
                if (((x + y) & 3) == 0) kso_px(c, x, y, true);
            }
        } else if ((x & 1) == 0) {
            /* Faint moonlit snow edge along upper crest */
            kso_px(c, x, my + 1, true);
        }
    }

    /* 6. Mid Foothill Ridge (crisp crest + delicate alpine mist fading to dark shore) */
    for (int x = 0; x < (int)KSO_WIDTH; x++) {
        int ry = mid_ridge_y(x);
        for (int y = ry; y < 98; y++) {
            int depth = y - ry;
            bool lit = false;
            if (depth == 0) {
                lit = true;
            } else if (depth <= 2) {
                lit = ((x ^ y) & 1) == 0;
            } else if (depth <= 4) {
                lit = ((x & 1) == 0) && ((y & 1) == 0);
            }
            kso_px(c, x, y, lit);
        }
    }

    /* 7. Tiny Alpine Pines along the dark shoreline bank */
    draw_pine(c, 7, 97, 10);
    draw_pine(c, 15, 97, 7);
    draw_pine(c, 26, 97, 9);
    draw_pine(c, 42, 97, 7);
    draw_pine(c, 52, 97, 11);
    draw_pine(c, 59, 97, 8);

    /* 8. Rising Typing Fireflies / Embers */
    for (unsigned i = 0; i < 6u; i++) {
        if (motes[i].born_ms == 0 || s->now_ms < motes[i].born_ms) continue;
        uint32_t age = s->now_ms - motes[i].born_ms;
        if (age >= 2400u) continue;
        int my = motes[i].y - (int)(age * 52u / 2400u);
        int sway = (kso_sin32(age / 140u + motes[i].seed) * 4) / 127;
        int mx = motes[i].x + sway;
        bool twinkle = age < 1800u || ((age / 120u) & 1u) == 0u;
        if (twinkle) {
            kso_px(c, mx, my, true);
            if (age < 900u) {
                kso_px(c, mx, my + 1, true);
                if (age < 400u) {
                    kso_px(c, mx - 1, my, true);
                    kso_px(c, mx + 1, my, true);
                }
            }
        }
    }

    /* 9. Moonlit Alpine Lake & Shimmering Reflections (y = 98..126) */
    for (int x = 0; x < (int)KSO_WIDTH; x++) {
        /* Calm horizon line with delicate organic breaks */
        if ((x % 17) != 9) kso_px(c, x, 98, true);
    }

    /* Animated crescent moon reflection shimmer column centered at x = 48 */
    static const uint8_t REFLECT_Y[8] = {101, 104, 107, 110, 114, 118, 122, 125};
    static const uint8_t REFLECT_W[8] = {13, 15, 12, 10, 8, 6, 4, 2};
    for (unsigned r = 0; r < 8u; r++) {
        int sway = (kso_sin32(s->now_ms / 180u + r * 5u) * 2) / 127;
        int rw = REFLECT_W[r];
        int rx = 48 - rw / 2 + sway;
        kso_hline(c, rx, REFLECT_Y[r], rw, true);
        /* Tiny broken shimmer glint beside upper rows */
        if (r < 4u && ((s->now_ms / 240u + r) & 1u) == 0u) {
            kso_px(c, rx + rw + 2, REFLECT_Y[r], true);
        }
    }

    /* Subtle dark-water ripples on the left, gently energized by typing */
    uint32_t since_press = s->now_ms - s->last_press_ms;
    int energy = (s->last_press_ms != 0 && since_press < 1500u)
                     ? (int)((1500u - since_press) / 350u)
                     : 0;
    static const uint8_t RIPPLE[5][3] = {
        {10, 103, 7},
        {22, 108, 6},
        {8, 113, 9},
        {19, 119, 6},
        {12, 124, 5},
    };
    for (unsigned r = 0; r < 5u; r++) {
        int sway = (kso_sin32(s->now_ms / 260u + r * 7u) * 2) / 127;
        int rw = RIPPLE[r][2] + (energy > 0 && (r & 1u) == 0u ? energy : 0);
        int rx = RIPPLE[r][0] + sway;
        kso_hline(c, rx, RIPPLE[r][1], rw, true);
    }
}

void kso_preset_render(kso_canvas_t *canvas, const kso_state_t *state) {
    kso_clear(canvas);
    if (state->is_left) {
        render_left(canvas, state);
    } else {
        render_right(canvas, state);
    }
}
