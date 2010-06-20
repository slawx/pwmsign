#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <stdint.h>

#define COLUMNS 4
#define ROWS 8

#include "algo.h"

#define COLUMN_PORT PORTD
#define COLUMN_DDR DDRD
#define ROW_PORT PORTB
#define ROW_DDR DDRB

uint8_t bitmap[COLUMNS][ROWS];

uint8_t pd[] = { _BV(PD2), _BV(PD3), _BV(PD4), _BV(PD5) };
uint8_t pdi;

pwm p[2][ROWS+1];
pwm *pp = &(p[0][0]);

uint8_t rot;
uint8_t rotc;

#define FLAG_SORT 1<<0
volatile uint8_t flags;

ISR(TIMER0_COMPA_vect)
{
    while(pp->width < TCNT0) {
        ROW_PORT = pp->pin;
        pp++;
        OCR0A = pp->width;
    }
}

ISR(TIMER0_OVF_vect)
{
    COLUMN_PORT = 0;

    pp = p[pdi%2];
    pdi = (pdi + 1) % COLUMNS;

    OCR0A = 1;
    TCNT0 = 0;

    // just make the pwm logic a regular fn called from the isr, or a macro.
    ROW_PORT = 0;
    if(pp->width == 0) {
        ROW_PORT = pp->pin;
    }

    // XXX reset prescaler

    flags |= FLAG_SORT;

    COLUMN_PORT = pd[pdi];
}

int main(void) {
    ROW_DDR = 0xff;
    COLUMN_DDR = _BV(5)|_BV(4)|_BV(3)|_BV(2);

    pwm_init(bitmap, 0);
    pwm_copy(p[0], bitmap[0], ROWS);
    pwm_copy(p[1], bitmap[1], ROWS);

    TCCR0B = _BV(CS01)|_BV(CS00); // ck / 64
    // TCCR0B = _BV(CS02); // ck / 256
    OCR0A = 0;
    TIMSK = _BV(TOIE0) | _BV(OCIE0A); // enable overflow and compare interrupts

    sei();

    flags |= FLAG_SORT;

    while(1) {
        if((flags & FLAG_SORT) && 0) {
            pwm_copy(p[pdi%2], bitmap[pdi], ROWS);
            flags &= (uint8_t)(0xff ^ FLAG_SORT);
        }
        set_sleep_mode(SLEEP_MODE_IDLE);
        sleep_enable();
        sleep_cpu();
    }
    return 0;
}
