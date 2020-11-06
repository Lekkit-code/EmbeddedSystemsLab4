#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include <stdio.h>
#include <string.h>

#include "i2c.h"


void i2c_init(void) //Set the prescaler to 1 and writes our TWBR-value so we get our desired SCL_FREQUENCY.
{
	TWCR &= ~(1 << TWPS1) | ~(1 << TWPS0);
	TWBR = ((F_CPU / SCL_FREQ -16) /2);
}

void i2c_meaningful_status(uint8_t status) {
	switch (status) {
		case 0x08: // START transmitted, proceed to load SLA+W/R
			printf_P(PSTR("START\n"));
			break;
		case 0x10: // repeated START transmitted, proceed to load SLA+W/R
			printf_P(PSTR("RESTART\n"));
			break;
		case 0x38: // NAK or DATA ARBITRATION LOST
			printf_P(PSTR("NOARB/NAK\n"));
			break;
		// MASTER TRANSMIT
		case 0x18: // SLA+W transmitted, ACK received
			printf_P(PSTR("MT SLA+W, ACK\n"));
			break;
		case 0x20: // SLA+W transmitted, NAK received
			printf_P(PSTR("MT SLA+W, NAK\n"));
				break;
		case 0x28: // DATA transmitted, ACK received
			printf_P(PSTR("MT DATA+W, ACK\n"));
			break;
		case 0x30: // DATA transmitted, NAK received
			printf_P(PSTR("MT DATA+W, NAK\n"));
			break;
		// MASTER RECEIVE
		case 0x40: // SLA+R transmitted, ACK received
			printf_P(PSTR("MR SLA+R, ACK\n"));
			break;
		case 0x48: // SLA+R transmitted, NAK received
			printf_P(PSTR("MR SLA+R, NAK\n"));
			break;
		case 0x50: // DATA received, ACK sent
			printf_P(PSTR("MR DATA+R, ACK\n"));
			break;
		case 0x58: // DATA received, NAK sent
			printf_P(PSTR("MR DATA+R, NAK\n"));
			break;
		default:
			printf_P(PSTR("N/A %02X\n"), status);
			break;
	}
}

inline void i2c_start() //Sends the START command and waits for the transmit to complete.
{
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT))) {};
}

inline void i2c_stop() //Sends the STOP command and waits for the transmit to complete.
{
	TWCR = (1 << TWINT)  | (1 << TWSTO) | (1 << TWEN);
	while ((TWCR & (1 << TWSTO))) {};
}

inline uint8_t i2c_get_status(void) //Returns the status code.
{
	return ((TWSR & 0xF8));
}

inline void i2c_xmit_addr(uint8_t address, uint8_t rw) //Transmits the address to the EEPROM-chip and if we want to write or not. The chip we're using only use a control byte, so we send that.
{

	TWDR = (address & (0xF0)) | rw;
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT))) {};
}

inline void i2c_xmit_byte(uint8_t data) //Transmits a single byte and waits for the transmit to complete.
{
	TWDR = data;
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT))) {};
}

inline uint8_t i2c_read_ACK() //Reads a single byte and sends an ACK.
{
	TWCR = (1 << TWINT) | (1 << TWEA) | (1<<TWEN);
	while (!(TWCR & (1 << TWINT))) {};
	return TWDR;
}

inline uint8_t i2c_read_NAK() //Reads a single byte and sends NACK.
{
	TWCR = (1 << TWINT) | (1<<TWEN);
	while (!(TWCR & (1 << TWINT))) {};
	return TWDR;
}

inline void eeprom_wait_until_write_complete() //Polls to see if the EEPROM is finished writing. Once it is, stops looping.
{
	while (i2c_get_status() != 0x18) {
		i2c_start();
		i2c_xmit_addr(ADDRESS_TO_EEPROM, I2C_W);
	}
}

uint8_t eeprom_read_byte(uint8_t addr) //Reads a single byte from the EEPROM. Initiates, sends the address we want to read and change to read mode. Then send STOP command and returns the read data.
{
	i2c_start();
	i2c_xmit_addr(ADDRESS_TO_EEPROM, I2C_W);
	i2c_xmit_byte(addr);
	i2c_start();
	i2c_xmit_addr(ADDRESS_TO_EEPROM, I2C_R);
	uint8_t data;
	data = i2c_read_NAK();
	i2c_stop();
	return data;
}

void eeprom_write_byte(uint8_t addr, uint8_t data) //Writes a single byte to a specified address in the EEPROM memory.
{
	i2c_start();
	i2c_xmit_addr(ADDRESS_TO_EEPROM, I2C_W);
	i2c_xmit_byte(addr);
	i2c_xmit_byte(data);
	i2c_stop();
}



void eeprom_write_page(uint8_t addr, uint8_t *data) //Writes a page (8 bytes) to the EEPROM memory at a given address.
{
	i2c_start();
	i2c_xmit_addr(ADDRESS_TO_EEPROM, I2C_W);
	i2c_xmit_byte(addr);
	for (int i = 0; i < 8; i++) {
		i2c_xmit_byte(data[i]);
	}
	i2c_stop();
}

void eeprom_sequential_read(uint8_t *buf, uint8_t start_addr, uint8_t len) //Reads from a given address of the EEPROM memory and for as long as you specify.
{
	i2c_start();
	i2c_xmit_addr(ADDRESS_TO_EEPROM, I2C_W);
	i2c_xmit_byte(start_addr);
	i2c_start();
	i2c_xmit_addr(ADDRESS_TO_EEPROM, I2C_R);
	
	for (int i = 0; i < len; i++) {
		if (i != len - 1) {
			buf[i] = i2c_read_ACK();
		}
		else {
			buf[i] = i2c_read_NAK();
		}
	}
	i2c_stop();
}
