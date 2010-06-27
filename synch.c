/*
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
/**
 * synch.c
 *
 * These routines are designed to synchronize the microprocessor's RC
 * oscillator by measuring the timing of incoming RS-232 transmissions.
 * These use the input capture unit on TIMER1, which means you will need
 * to solder RX to ICP for this to work.
 *
 * They are not finished.
 */

#include "synch.h"

// XXX just use TIMER1
uint8_t ici_i;
uint16_t ici[2];
int8_t step;

/**
 * Synchronize a relatively accurate OSCCAL to the USART.
 * (jumper the RXD & ICP pins).
 * No smart searching, just increase or decrease OSCCAL when a pulse
 * is reasonably close to the right length.
 */
int search(int16_t delta, int8_t step) {
	if(delta < SYNCH_TARGET) {
		step = MAX(1, step*2);
		step = MIN(step, MAX_STEP);
		if((OSCCAL - OSCCAL_MAX) < step) {
			OSCCAL = OSCCAL_MAX;
		} else {
			OSCCAL += step;
		}
	} else if(delta > SYNCH_TARGET) {
		step = MIN(-1, step*2);
		step = MAX(step, MAX_STEP);
		if(OSCCAL < -step) {
			OSCCAL = 0;
		} else {
			OSCCAL += step;
		}
	} else {
		return 0;
	}
	return step;
}

// XXX not working yet.
ISR(TIMER1_CAPT_vect)
{
    ici[ici_i] = ICR1L | (ICR1H << 8);
    ici_i++;
    ici_i %= 2;

    if (TCCR1B & (1 << ICES1)) {
        TCCR1B &= ~(1 << ICES1);
    } else {
        TCCR1B |= 1 << ICES1;
    }

    if (ici_i == 0) {
        uint16_t delta = ici[(ici_i + 1) % 2] - ici[ici_i];
		step = search(delta, step);
    }
    intflags.synch = 1;
}

/**
 * Set up the event capture unit for OSCCAL calibration.
 */
static void synch_init() {
    // set ICP pin as input, no internal pullup.
    DDRD &= ~(1 << PD6); // ICP
    PORTD &= ~(1 << PD6);
    // ck / 1
    TCCR1B = (1 << CS10);
    // synch on falling edge
    TCCR1B &= ~(1 << ICES1);
    TIMSK |= 1 << ICIE1;
}

// END SYNCHRONIZATION
