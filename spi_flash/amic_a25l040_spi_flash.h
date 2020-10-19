/*
	microchip_23k640_spi_sram.c

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

#ifndef __AMIC_A25L040_SPI_FLASH_H__
#define  __AMIC_A25L040_SPI_FLASH_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct amic_a25l040_t** amic_a25l040_pp;
typedef struct amic_a25l040_t* amic_a25l040_p;

/*
 * Default connect going directly to avr spi in/out pins.
 *
 * NOTE: Each part requires separate chip select from avr.
 */
void
amic_a25l040_connect(
		amic_a25l040_p part,
		avr_irq_t * cs_irq);

void
amic_a25l040_init(
		struct avr_t * avr,
		amic_a25l040_pp part);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef  __AMIC_A25L040_SPI_FLASH_H__ */

