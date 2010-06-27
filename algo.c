/**
 * Dimmable LED sign firmware.
 * Copyright 2010 Daniel Holth <dholth@fastmail.fm>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
    pwm_sort(p, b_len);
    pwm_merge(p, b_len + 1);
}

