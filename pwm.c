#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <stdint.h>

#include "algo.h"

uint8_t pd[] = { _BV(PD2), _BV(PD3), _BV(PD4), _BV(PD5) };
uint8_t pdi = 0;

pwm p[4][9];
pwm *pp = &(p[0][0]);
uint8_t rot = 0;
uint8_t rotc = 0;

ISR(TIMER0_COMPA_vect)
{
    while(pp->width < TCNT0) {
        PORTB = pp->pin;
        pp++;
        OCR0A = pp->width;
    }
}

ISR(TIMER0_OVF_vect)
{
	PORTD = 0;
	pdi = (pdi + 1) % 4;
	pp = &(p[pdi][0]);

	rotc++;
	if(rotc > 32) {
		rotc = 0;
		rot++;
		initpwm(p, rot);
	}

	OCR0A = 1;
	TCNT0 = 0;

	// only if the first one is width = 0
	// PORTB = pp->pin;

    PORTD = pd[pdi];
}

int main(void) {
	DDRB = 0xff;
	DDRD = _BV(5)|_BV(4)|_BV(3)|_BV(2);

	initpwm(p, 0);

    // TCCR0B = _BV(CS01)|_BV(CS00);
	TCCR0B = _BV(CS02);
	OCR0A = 0;
	TIMSK = _BV(TOIE0) | _BV(OCIE0A);

	sei();

	set_sleep_mode(SLEEP_MODE_IDLE);
	sleep_enable();
	while(1) {
		sleep_cpu();
	}
	return 0;
}
