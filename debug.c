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

#include <stdio.h>
#include <stdint.h>
#include "algo.h"

void bitmap_print(uint8_t b[], uint8_t len) {
    int i, j;
    for(i=0; i<len; i++) {
        printf("%d %d", i, b[i]);
        printf("\n");
    }
}

void pwm_print(pwm p[2][9], uint8_t len) {
    int i, j;
    for(i=0; i<len; i++) {
        printf("%04d ", p[0][i].width);
        for(j=0; j<8; j++) {
            printf("%s", (p[0][i].pin & (1<<j)) ? "1" : "0");
        }
        printf("\n");
    }
}

int main(int argc, char **argv) {
    int i;
    uint8_t bitmap[4][8];
    pwm p[2][9];

    pwm_init(bitmap, 1);
    bitmap[0][0] = bitmap[0][1];
    bitmap[0][3] = bitmap[0][4] = bitmap[0][5];
    pwm_copy(p[0], bitmap[0], 8);
    bitmap_print(bitmap[0], 8);
    pwm_print(p, 9);

    return 0;

    printf("\n");
    p[0][2].width = p[0][1].width;
    pwm_merge(p[0], 9);
    pwm_print(p, 9);
    printf("\n");

    pwm_copy(p[0], bitmap[0], 8);
    for(i=0; i<8; i++) {
        p[0][i].width = p[0][0].width;
    }
    pwm_print(p, 9);
    printf("\n");
    pwm_merge(p[0], 9);
    pwm_print(p, 9);
    printf("\n");

    pwm_copy(p[0], bitmap[0], 8);
    for(i=0; i<8; i++) {
        p[0][i].width = 0xff;
    }
    pwm_print(p, 9);
    printf("\n");
    pwm_merge(p[0], 9);
    pwm_print(p, 9);
    printf("\n");

    pwm_copy(p[0], bitmap[0], 8);
    for(i=0; i<8; i++) {
        p[0][i].width = 0;
    }
    pwm_print(p, 9);
    printf("\n");
    pwm_merge(p[0], 9);
    pwm_print(p, 9);
    printf("\n");
}
