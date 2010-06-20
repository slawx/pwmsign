#ifndef __ALGO_H__
#define __ALGO_H__

#include <stdint.h>

typedef struct {
	uint8_t width;
	uint8_t pin;
} pwm;

void pwm_sort(pwm p[], uint8_t len);
void pwm_merge(pwm p[], uint8_t len);
void pwm_copy(pwm p[], uint8_t b[], uint8_t b_len);

void pwm_init(uint8_t bitmap[4][8], uint8_t rot);

#endif
