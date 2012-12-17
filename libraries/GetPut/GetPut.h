/* -*- c++ -*-
 *
 * GetPut.h
 *
 * Author: Markku Rossi <mtr@iki.fi>
 *
 * Copyright (c) 2011-2012 Markku Rossi
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 */

#ifndef GETPUT_H
#define GETPUT_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

class GetPut
{
public:

  static uint16_t get_16bit(uint8_t *buf);

  static uint32_t get_32bit(uint8_t *buf);

  static void put_16bit(uint8_t *buf, uint16_t val);

  static void put_32bit(uint8_t *buf, uint32_t val);

  /* Convert the hex character `ch' to its integer value. */
  static int atoh(uint8_t ch);

  /* Decode hex string `input' into the buffer `buffer' that has space
     for `buffer_len' bytes.  The method returns the number of bytes
     stored into the output buffer. */
  static size_t hex_decode(const char *input, uint8_t *buffer,
                           size_t buffer_len);

  /* Decode hex string `val' into the buffer `buf', `buflen' and write
     it to EEPROM address `eeprom_addr'. */
  static void eeprom_parse_data(const char *val, uint8_t *buf, size_t buflen,
                                int eeprom_addr);

  /* Read data from the EEPROM address `eeprom_addr' and store it into
     the buffer `buf', `buflen'. */
  static void eeprom_read_data(uint8_t *buf, size_t buflen, int eeprom_addr);

  /* Write data `buf', `buflen' to the EEPROM address
     `eeprom_addr'. */
  static void eeprom_write_data(uint8_t *buf, size_t buflen, int eeprom_addr);

  /* Print ASCII data from EEPROM address `eeprom_addr'.  The argument
     `max_len' specifies the maximum length to print. */
  static void eeprom_print_ascii(int eeprom_addr, int max_len);
};

#endif /* not GETPUT_H */
