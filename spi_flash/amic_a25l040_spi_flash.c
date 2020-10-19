/*
	amic_a25l040_spi_sram.c

	Copyright 2014 Michael Hughes <squirmyworms@embarqmail.com>

 	This file is part of simavr.

	simavr is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	simavr is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with simavr.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
 *	Implimentation of Microchip 8K SPI SRAM, part 23K640.
 *		Based on data sheet specifications.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sim_avr.h"
#include "sim_core.h"
#include "sim_io.h"
#include "sim_irq.h"
#include "avr_spi.h"

#include "amic_a25l040_spi_flash.h"

enum {
	IRQ_AMIC_A25L040_SPI_BYTE_IN = 0,
	IRQ_AMIC_A25L040_SPI_BYTE_OUT,
	IRQ_AMIC_A25L040_CS,
	IRQ_AMIC_A25L040_HOLD,
	IRQ_AMIC_A25L040_COUNT
};

typedef struct amic_a25l040_t {
	avr_irq_t	*irq;
	struct avr_t	*avr;

	int		cs;
	int		hold;
	int		hold_disable;
	int		instruction;
	int		mode;
	int		state;

	uint16_t	address;
	uint8_t		data[8192];
}amic_a25l040_t;

#define MICROCHIP_INSTRUCTION_READ 0X03
#define MICROCHIP_INSTRUCTION_WRITE 0X02

#define MICROCHIP_INSTRUCTION_RDSR 0X05
#define MICROCHIP_INSTRUCTION_WRSR 0X01

#define MICROCHIP_MODE_PAGED 0x02
#define MICROCHIP_MODE_SEQUENTIAL 0x01

enum {
	MICROCHIP_STATE_IDLE = 0,
	MICROCHIP_STATE_ADDRESS_FETCH_HOB,
	MICROCHIP_STATE_ADDRESS_FETCH_LOB,
	MICROCHIP_STATE_PROCESS_INSTRUCTION
};

static void
amic_a25l040_in(
		struct avr_irq_t * irq,
		uint32_t value,
		void * param)
{
	printf("%s: value=%04x\n", __FUNCTION__, value);

	amic_a25l040_p part = param;

	if(!part->cs || (part->hold & !part->hold_disable)) {
		return;
	}
	
	printf("%s: value = %d\n", __FUNCTION__, value);
	return;
#if 0	
	switch(part->state) {
		case	MICROCHIP_STATE_IDLE:
			part->instruction = value;
			switch(part->instruction) {
				case MICROCHIP_INSTRUCTION_RDSR:
				case MICROCHIP_INSTRUCTION_WRSR:
					part->state = MICROCHIP_STATE_PROCESS_INSTRUCTION;
					break;
				case MICROCHIP_INSTRUCTION_READ:
				case MICROCHIP_INSTRUCTION_WRITE:
					part->state = MICROCHIP_STATE_ADDRESS_FETCH_HOB;
					break;
			}
			break;
		case	MICROCHIP_STATE_ADDRESS_FETCH_HOB:
			part->address = (value & 0xff) << 8;
			part->state = MICROCHIP_STATE_ADDRESS_FETCH_LOB;
			break;
		case	MICROCHIP_STATE_ADDRESS_FETCH_LOB:
			part->address |= (value & 0xff);
			part->state = MICROCHIP_STATE_PROCESS_INSTRUCTION;
			break;
		case	MICROCHIP_STATE_PROCESS_INSTRUCTION:
			switch(part->instruction) {
				case	MICROCHIP_INSTRUCTION_READ: {
					printf("%s: MICROCHIP_INSTRUCTION_READ, [0x%04x] = %0d", __FUNCTION__, part->address, value);
						uint8_t data = part->data[part->address & (8192 - 1)];
						avr_raise_irq(part->irq + IRQ_AMIC_A25L040_SPI_BYTE_OUT, data);
					} break;
				case	MICROCHIP_INSTRUCTION_WRITE:
					part->data[part->address & (8192 - 1)] = value;
					printf("%s: MICROCHIP_INSTRUCTION_WRITE, [0x%04x] = %0d", __FUNCTION__, part->address, value);
					break;
				case	MICROCHIP_INSTRUCTION_RDSR: {
						/* data sheet says bit 2 is set on reading status register. */
						uint8_t status = ((part->mode & 0x03) << (7 - 2)) | 0x02 | (part->hold ? 1 : 0);
						printf("%s: MICROCHIP_INSTRUCTION_RDSR = %0d\n", __FUNCTION__, status);
							avr_raise_irq(part->irq + IRQ_AMIC_A25L040_SPI_BYTE_OUT, status);
						part->state = 0;
					} break;
				case	MICROCHIP_INSTRUCTION_WRSR:
					printf("%s: MICROCHIP_INSTRUCTION_WRSR = %0d\n", __FUNCTION__, value);
					part->mode = ((value >> (7 - 2)) & 0x03);
					part->hold = value & 0x01;
					part->state = 0;
					break;
				default:
					part->state = 0;
					break;
		}			
	}
	
	if((MICROCHIP_STATE_PROCESS_INSTRUCTION == part->state) &&
		((MICROCHIP_INSTRUCTION_READ == part->instruction) ||
			(MICROCHIP_INSTRUCTION_WRITE == part->instruction))) {
		switch(part->mode) {
			case MICROCHIP_MODE_SEQUENTIAL: /* sequential read/write operation */
				part->address++;
				break;
			case MICROCHIP_MODE_PAGED: /* paged read/write operation */
				part->state = ((part->address & 0x1f) ? part->state : 0);
				part->address = (part->address & 0xe0) | ((part->address + 1) & 0x1f);
				break;
			default: /* single byte read/write */
				part->state = 0;
				break;
		}
	}
#endif
}

static void
amic_a25l040_cs(
		struct avr_irq_t * irq,
		uint32_t value,
		void * param)
{
	printf("%s: value=%04x\n", __FUNCTION__, value);

	amic_a25l040_p part = param;

//	part->cs = value;
	part->cs = !value;
	if(!part->cs) {
		part->state = 0;
		part->instruction = 0;
	}
	
	printf("%s: cs = %0d\n", __FUNCTION__, part->cs);
}

static void
amic_a25l040_hold(
		struct avr_irq_t * irq,
		uint32_t value,
		void * param)
{
	printf("%s: value=%04x\n", __FUNCTION__, value);

	amic_a25l040_p part = param;
	
	part->hold = value;

	printf("%s: hold = %0d\n", __FUNCTION__, part->hold);
}

static const char * amic_a25l040_irq_names[IRQ_AMIC_A25L040_COUNT] = {
	[IRQ_AMIC_A25L040_SPI_BYTE_IN] = "8<amic_a25l040.in",
	[IRQ_AMIC_A25L040_SPI_BYTE_OUT] = "8>amic_a25l040.out",
	[IRQ_AMIC_A25L040_CS] = "1>amic_a25l040.cs",
	[IRQ_AMIC_A25L040_HOLD] = "1>amic_a25l040.hold"
};

void
amic_a25l040_connect(
	amic_a25l040_p part,
	avr_irq_t * cs_irq)
{
	if(!part || !cs_irq)
		return;

	avr_connect_irq(avr_io_getirq(part->avr, AVR_IOCTL_SPI_GETIRQ(0), SPI_IRQ_OUTPUT), part->irq + IRQ_AMIC_A25L040_SPI_BYTE_IN);
	avr_connect_irq(part->irq + IRQ_AMIC_A25L040_SPI_BYTE_OUT, avr_io_getirq(part->avr, AVR_IOCTL_SPI_GETIRQ(0), SPI_IRQ_INPUT));
	avr_connect_irq(cs_irq, part->irq + IRQ_AMIC_A25L040_CS);
}

void
amic_a25l040_init(
		struct avr_t * avr,
		amic_a25l040_pp ppart)
{
	if(!(*ppart))
		*ppart = (amic_a25l040_p)malloc(sizeof(amic_a25l040_t));

	amic_a25l040_p part = *ppart;

	if(!avr || !part)
		return;

//	memset(part, 0, sizeof(*part));

	part->avr = avr;
	part->irq = avr_alloc_irq(&avr->irq_pool, 0, IRQ_AMIC_A25L040_COUNT, amic_a25l040_irq_names);
	avr_irq_register_notify(part->irq + IRQ_AMIC_A25L040_SPI_BYTE_IN, amic_a25l040_in, part);
	avr_irq_register_notify(part->irq + IRQ_AMIC_A25L040_CS, amic_a25l040_cs, part);
	avr_irq_register_notify(part->irq + IRQ_AMIC_A25L040_HOLD, amic_a25l040_hold, part);

	part->cs = 0;
	part->hold = 0;
	part->hold_disable = 0;
	part->instruction = 0;
	part->state = 0;
}
