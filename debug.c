#include <stdio.h>
#include <stdint.h>
#include "algo.h"

void pwm_print(pwm p[4][9], uint8_t len) {
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
    pwm p[4][9];

    pwm_init(p, 1);
    pwm_print(p, 9);
    printf("\n");
    p[0][2].width = p[0][1].width;
    pwm_merge(p[0], 9);
    pwm_print(p, 9);
    printf("\n");

    pwm_init(p, 1);
    for(i=0; i<8; i++) {
        p[0][i].width = p[0][0].width;
    }
    pwm_print(p, 9);
    printf("\n");
    pwm_merge(p[0], 9);
    pwm_print(p, 9);
    printf("\n");

    pwm_init(p, 1);
    for(i=0; i<8; i++) {
        p[0][i].width = 0xff;
    }
    pwm_print(p, 9);
    printf("\n");
    pwm_merge(p[0], 9);
    pwm_print(p, 9);
    printf("\n");

    pwm_init(p, 1);
    for(i=0; i<8; i++) {
        p[0][i].width = 0;
    }
    pwm_print(p, 9);
    printf("\n");
    pwm_merge(p[0], 9);
    pwm_print(p, 9);
    printf("\n");
}
