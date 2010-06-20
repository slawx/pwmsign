#include <stdint.h>

#include "algo.h"

/**
 * Sort p[] by width.
 */
void pwm_sort(pwm p[], uint8_t len) {
	int8_t i;
	int8_t j;
	pwm v;
	for(i=1; i < len; i++) {
		v = p[i];
		j = i-1;
		while(p[j].width > v.width && j >= 0) {
			p[j+1] = p[j];
			j--;
		}
		p[j+1] = v;
	}
}

/**
 * Merge pins with the same width into the same pwm[] element.
 */
void pwm_merge(pwm p[], uint8_t len) {
    uint8_t i=0;
    uint8_t j=0;
    while(j < len) {
        while(j+1 < len && p[i].width == p[j+1].width) {
            j++;
            p[i].width |= p[j].width;
        }
        if(i!=j) {
            p[i] = p[j];
        }
        i++;
        j++;
    }
}

void pwm_init(pwm p[4][9], uint8_t rot)
{
    uint8_t i, j, pin;
    for(i = 0; i < 4; i++){
        pin = 0;
        for(j = 0; j < 8; j++) {
            pin = 1 << j;
            p[i][j].pin = pin;
            p[i][j].width = 1<<((j+rot)%8);
        }

        // sentinel
        p[i][8].width = 0xff;
        p[i][8].pin = 0;

        pwm_sort(p[i], 8);

        pin = 0;
        for(j = 0; j < 9; j++) {
        	pin = (p[i][j].pin) = pin | (p[i][j].pin);
        }
    }
}
