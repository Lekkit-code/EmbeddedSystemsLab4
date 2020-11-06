#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>

#include "adc.h"
#include "gpio.h"
#include "i2c.h"
#include "serial.h"
#include "timer.h"



int main (void) {

	i2c_init();
	uart_init();
	char buf[100];
	sei();

	while (1) {
		eeprom_sequential_write(0x10, "EN LÅNG STRÄNG SOM JAG HOPPAS MYCKET PÅ! DET BORDE FUNKA!");
		eeprom_wait_until_write_complete();
		eeprom_sequential_read(buf, 0x10, 100);
		for (int i = 0; i < strlen(buf); i++) {
			printf("%c", buf[i]);
		}
		_delay_ms(100000);
	}
	return 0;
}

