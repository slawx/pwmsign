#include <stdio.h>
#include <stdint.h>
#include "algo.h"

int main(int argc, char **argv) {
    int i, j;
    pwm p[4][9];

    initpwm(p, 1);

    for(i=0; i<9; i++) {
        printf("%04d ", p[0][i].width);
        for(j=0; j<8; j++) {
            printf("%s", (p[0][i].pin & (1<<j)) ? "1" : "0");
        }
        printf("\n");
    }
}
