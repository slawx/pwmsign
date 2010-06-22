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
