#include <stdint.h>

#include "algo.h"

/**
 * Sort p[] by width.
 */
void pwm_sort(pwm p[], uint8_t len) {
    int8_t i;
    int8_t j;
    pwm v;
    for (i = 1; i < len; i++) {
        v = p[i];
        j = i - 1;
        while (p[j].width > v.width && j >= 0) {
            p[j + 1] = p[j];
            j--;
        }
        p[j + 1] = v;
    }
}

/**
 * Merge pins with the same width into the same pwm[] element.
 */
void pwm_merge(pwm p[], uint8_t len) {
    uint8_t i = 0;
    uint8_t j = 0;
    while (j < len) {
        if (i != j) {
            p[i] = p[j];
        }
        while (j + 1 < len && p[i].width == p[j + 1].width) {
            j++;
            p[i].pin |= p[j].pin;
        }

        i++;
        j++;
    }

    /* OR pins together, because ISR does not. */
    for (i = 1; i < len; i++) {
        p[i].pin |= p[i - 1].pin;
    }
}

/**
 * Copy bitmap into the pwm structure.
 */
void pwm_copy(pwm p[], uint8_t b[], uint8_t b_len) {
    uint8_t i;
    for (i = 0; i < b_len; i++) {
        p[i].width = b[i];
        p[i].pin = 1 << i;
    }
    p[b_len].width = 0xff;
    p[b_len].pin = 0xff;
    pwm_sort(p, b_len + 1);
    pwm_merge(p, b_len + 1);
}

/**
 * Sawtooth pattern. Doesn't have anything to do with pwm anymore.
 */
void pwm_init(uint8_t bitmap[4][8], uint8_t rot) {
    uint8_t i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 8; j++) {
            bitmap[i][j] = 1 << ((j + rot) % 8);
        }
    }
}
