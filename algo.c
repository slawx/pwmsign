#include <stdint.h>

#include "algo.h"

void insertionSort(pwm p[], uint8_t len) {
	int8_t i;
	int8_t j;
	pwm v;
	for(i=1; i < len-1; i++) {
		v = p[i];
		for(j = i-1; j >= 0; j--) {
			if(p[j].width > v.width) {
				p[j+1] = p[j];
			}
		}
		p[j+1] = v;
	}
}

void initpwm(pwm p[4][9], uint8_t rot)
{
    uint8_t i, j, pin;
    for(i = 0; i < 4; i++){
        pin = 0;
        for(j = 0; j < 8; j++) {
            pin = 1 << j;
            p[i][j].pin = pin;
            p[i][j].width = 1<<((j+rot)%8);
        }

        p[i][8].width = 0xff;
        p[i][8].pin = 0xff;

        insertionSort(p[i], 8);
        pin = 0xff;
        for(j = 0; j < 9; j++) {
        	pin = (p[i][j].pin) = pin ^ (p[i][j].pin);
        }
    }
}
