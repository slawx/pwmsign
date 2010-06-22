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
#include <stdlib.h>
#include <ctype.h>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "algo.h"

#define COLUMNS 4
#define ROWS 8
#define COLUMN_PORT PORTD
#define COLUMN_DDR DDRD
#define ROW_PORT PORTB
#define ROW_DDR DDRB

uint8_t bitmap[COLUMNS][ROWS];

uint8_t pd[] = { _BV(PD2), _BV(PD3), _BV(PD4), _BV(PD5) };
uint8_t pdi;

pwm p[2][ROWS + 1];
pwm *pp = &(p[0][0]);

uint8_t rot;
uint8_t rotc;

volatile struct {
    uint8_t sort :1; // need to refresh inactive buffer
    uint8_t synch :1; // synch event
    uint8_t rxc :1; // received a character
} intflags;

ISR(TIMER0_COMPA_vect)
{
    while (pp->width < TCNT0) {
        ROW_PORT = pp->pin;
        pp++;
        OCR0A = pp->width;
    }
}

ISR(TIMER0_OVF_vect)
{
    COLUMN_PORT = 0;

    pp = p[pdi % 2];

    TCNT0 = 0;

    if (pp->width == 0) {
        ROW_PORT = pp->pin;
        pp++;
    } else {
        ROW_PORT = 0;
    }
    OCR0A = pp->width;

    // XXX reset prescaler

    COLUMN_PORT = pd[pdi];
    pdi = (pdi + 1) % COLUMNS;
    intflags.sort = 1;
}

/* USART routines are under the following license:
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <joerg@FreeBSD.ORG> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.        Joerg Wunsch
 * ----------------------------------------------------------------------------
 *
 * More advanced AVR demonstration.  Controls a LED attached to OCR1A.
 * The brightness of the LED is controlled with the PWM.  A number of
 * methods are implemented to control that PWM.
 *
 * $Id: largedemo.c,v 1.3 2007/01/19 22:17:10 joerg_wunsch Exp $
 */

/*
 * Send character c down the UART Tx, wait until tx holding register
 * is empty.
 */
static void putchr(char c) {
    loop_until_bit_is_set(UCSRA, UDRE);
    UDR = c;
}

/*
 * Send a C (NUL-terminated) string down the UART Tx.
 */
static void printstr(const char *s) {
    while (*s) {
        if (*s == '\n')
            putchr('\r');
        putchr(*s++);
    }
}

/*
 * Same as above, but the string is located in program memory,
 * so "lpm" instructions are needed to fetch it.
 */
static void printstr_p(const char *s) {
    char c;

    for (c = pgm_read_byte(s); c; ++s, c = pgm_read_byte(s)) {
        if (c == '\n')
            putchr('\r');
        putchr(c);
    }
}

ISR(USART_RX_vect)
{
    intflags.rxc = 1;
}

/**
 * Setup UART.
 */
static void uart_38400(void) {
#undef BAUD  // avoid compiler warning
#define BAUD 38400
#include <util/setbaud.h>
    UBRRH = UBRRH_VALUE;
    UBRRL = UBRRL_VALUE;
#if USE_2X
    UCSRA |= (1 << U2X);
#else
    UCSRA &= ~(1 << U2X);
#endif
    // Enable TX, RX
    UCSRB = (1 << TXEN) | (1 << RXEN);
}

// SYNCHRONIZATION.
#define SYNCH_TARGET ((UBRR_VALUE + 1) * (16 >> USE_2X))
#define TOO_HIGH (SYNCH_TARGET + (SYNCH_TARGET >> 2))
#define TOO_LOW (SYNCH_TARGET - (SYNCH_TARGET >> 2))
#define OSCCAL_MAX (0x7f)
#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)

uint8_t ici_i;
uint16_t ici[2];
/**
 * Synchronize a relatively accurate OSCCAL to the USART.
 * (jumper the RXD & ICP pins).
 * No smart searching, just increase or decrease OSCCAL when a pulse
 * is reasonably close to the right length.
 */
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
        if (delta > SYNCH_TARGET && delta < TOO_HIGH) {
            // too fast. decrease osccal.
            OSCCAL = MAX(OSCCAL-1, 0);
        } else if (delta < SYNCH_TARGET && delta > TOO_LOW) {
            // too slow. increase osccal.
            OSCCAL = MIN(OSCCAL+1, OSCCAL_MAX);
        } else if (delta <= TOO_LOW || delta >= TOO_HIGH) {
            // out of bounds
        } else {
            // perfect! disable input compare interrupt.
            TIMSK &= ~(1 << ICIE1);
        }
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

int main(void) {
    // uint8_t ticks = 0;
    uint8_t i, j;

    char c;
    int8_t bxi = -1;
    char buffer[ROWS];

    OSCCAL = 101;

    uart_38400();

    // UCSRB |= (1 << RXCIE); // enable rx interrupt. causes glitches.

    printstr_p(PSTR("\nWelcome to the LED sign.\nTo update, type the letter 'u' followed by 32 bytes and a newline.\n"));

    sei();

    // synch_init();

    /*
     while (TCCR1B & (1<<ICIE1)) {
     if (intflags.rxc) {
     if (bit_is_set(UCSRA, RXC)) {
     while (bit_is_set(UCSRA, RXC)) {
     putchr(UDR);
     }
     putchr('\r');
     putchr('\n');
     }
     intflags.rxc = 0;
     }
     if (intflags.synch) {
     uint16_t delta = ici[(ici_i + 1) % 2] - ici[ici_i];
     // PORTB = OSCCAL;
     itoa(delta, buffer, 10);
     printstr(buffer);
     putchr(' ');
     itoa(OSCCAL, buffer, 10);
     printstr(buffer);
     putchr(' ');
     itoa(SYNCH_TARGET, buffer, 10);
     printstr(buffer);
     putchr('\r');
     putchr('\n');
     intflags.synch = 0;
     }
     }
     */

    ROW_PORT = 0;
    ROW_DDR = 0xff;
    COLUMN_DDR = _BV(5) | _BV(4) | _BV(3) | _BV(2);

    TCCR0B = _BV(CS01) | _BV(CS00); // ck / 64
    // TCCR0B = _BV(CS02); // ck / 256
    OCR0A = 0;
    TIMSK = _BV(TOIE0) | _BV(OCIE0A); // enable overflow and compare interrupts

    sei();

    while (1) {
        if (intflags.sort) {
            intflags.sort = 0;
            pwm_copy(p[pdi % 2], bitmap[pdi], ROWS);
        }
        while (bit_is_set(UCSRA, RXC) && !intflags.sort) {
            c = UDR;
            if (bxi < 0) {
                if (c == 'u') { // 'update'
                    printstr_p(PSTR("U"));
                    bxi++;
                    break;
                }
            } else if (bxi < (ROWS * COLUMNS)) {
                buffer[bxi % ROWS] = c;
                if((bxi % ROWS) == (ROWS-1)) {
                    j = (bxi / ROWS);
                    for(i=0; i<ROWS; i++) {
                        bitmap[j][i] = buffer[i];
                    }
                    itoa(j, buffer, 10);
                    printstr(buffer);
                }
                bxi++;
            } else {
                printstr_p(PSTR(" OK\n"));
                bxi = -1;
            }
        }
    }
    return 0;
}
