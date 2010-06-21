/**
 * LED lightbox firmware.
 *
 * Daniel Holth <dholth@fastmail.fm>
 * June 2010
 */

#include <stdint.h>
#include <stdlib.h>

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

// SYNCHRONIZATION
uint8_t ici_i;
uint16_t ici[2];
ISR(TIMER1_CAPT_vect)
{
    if (!intflags.synch) {
        ici[ici_i] = ICR1L | (ICR1H << 8);
        ici_i++;
        ici_i %= 2;

        if(TCCR1B & (1<<ICES1)) {
            TCCR1B &= ~(1 << ICES1);
        } else {
            TCCR1B |= 1<<ICES1;
        }

        if (ici_i == 0) {
            intflags.synch = 1;
        }
    }
}


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
    uint8_t ticks = 0;
    uint8_t i, j;

    char buffer[8];

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 8; j++) {
            bitmap[i][j] = 1;
        }
    }

    // debug the uart speed...
    uart_38400();
    printstr_p(PSTR("Welcome to the LED sign.\n"));
    OSCCAL = 101; // determined experimentally, was 100 from factory.

    printstr_p(PSTR("OSCCAL: "));
    itoa(OSCCAL, buffer, 10);
    printstr(buffer);
    putchr('\r');
    putchr('\n');
    printstr_p(PSTR("UBRR: "));
    itoa(UBRR_VALUE, buffer, 10);
    printstr(buffer);
    putchr('\r');
    putchr('\n');
    /*
    synch_init();
    sei();
    while (1) {
        if (intflags.synch) {
            uint16_t delta = ici[(ici_i + 1) % 2] - ici[ici_i];
            itoa(delta, buffer, 10);
            printstr(buffer);
            putchr('\r');
            putchr('\n');
            intflags.synch = 0;
        }
    }
    */

    ROW_DDR = 0xff;
    COLUMN_DDR = _BV(5) | _BV(4) | _BV(3) | _BV(2);

    /*
     pwm_init(bitmap, 0);
     pwm_copy(p[0], bitmap[0], ROWS);
     pwm_copy(p[1], bitmap[1], ROWS);
     */

    TCCR0B = _BV(CS01) | _BV(CS00); // ck / 64
    // TCCR0B = _BV(CS02); // ck / 256
    OCR0A = 0;
    TIMSK = _BV(TOIE0) | _BV(OCIE0A); // enable overflow and compare interrupts

    sei();

    while (1) {
        if (intflags.sort) {
            intflags.sort = 0;

            pwm_copy(p[pdi % 2], bitmap[pdi], ROWS);

            ticks += 2;
            if (ticks == 0) {
                rot++;
                // pwm_init(bitmap, rot);
                for (i = 0; i < 4; i++) {
                    for (j = 0; j < 8; j++) {
                        bitmap[i][j] = rot % 4;
                    }
                }
            }
        }
        set_sleep_mode(SLEEP_MODE_IDLE);
        sleep_enable();
        sleep_cpu();
    }
    return 0;
}
