Code for a serial-controlled LED sign with 32 individually dimmable LEDs,
controlled by an ATtiny2313 running off its internal oscillator at 8MHz.

The main feature of this code is its efficient PWM ISR which is able to
PWM 8 pins at ckdiv/64. Sorting by the necessary bit transition times
ahead of time, the ISR only has to run a maximum of 8 times per cycle,
plus the overflow interrupt, instead of the common 'compare every pin
against the timer every timer clock'.

