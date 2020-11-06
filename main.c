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
	char c;
	sei();

	while (1) {
		eeprom_write_byte(0x10, 'k');
		eeprom_wait_until_write_complete();
		eeprom_write_byte(0x11, 'b');
		eeprom_wait_until_write_complete();
		c = eeprom_read_byte(0x10);
		printf("%c", c);
		c = eeprom_read_byte(0x11);
		printf("%c", c);
		_delay_ms(1000);
	}
	return 0;
}

