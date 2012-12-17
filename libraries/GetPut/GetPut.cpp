/*
 * GetPut.cpp
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

#include "GetPut.h"
#include <EEPROM.h>

uint16_t
GetPut::get_16bit(uint8_t *buf)
{
  uint16_t val;

  val = buf[0];
  val <<= 8;
  val |= buf[1];

  return val;
}

uint32_t
GetPut::get_32bit(uint8_t *buf)
{
  uint32_t val;

  val = buf[0];
  val <<= 8;
  val |= buf[1];
  val <<= 8;
  val |= buf[2];
  val <<= 8;
  val |= buf[3];

  return val;
}

void
GetPut::put_16bit(uint8_t *buf, uint16_t val)
{
  buf[0] = (val >> 8) & 0xff;
  buf[1] = (val >> 0) & 0xff;
}

void
GetPut::put_32bit(uint8_t *buf, uint32_t val)
{
  buf[0] = (val >> 24) & 0xff;
  buf[1] = (val >> 16) & 0xff;
  buf[2] = (val >> 8) & 0xff;
  buf[3] = (val >> 0) & 0xff;
}

int
GetPut::atoh(uint8_t ch)
{
  if ('0' <= ch && ch <= '9')
    return ch - '0';
  if ('a' <= ch && ch <= 'f')
    return 10 + ch - 'a';
  if ('A' <= ch && ch <= 'F')
    return 10 + ch - 'A';

  return -1;
}

size_t
GetPut::hex_decode(const char *input, uint8_t *buffer, size_t buffer_len)
{
  int len;
  int i;
  int pos = 0;
  int val, v;

  if (input[0] == '0' && input[1] == 'x')
    input += 2;

  len = strlen(input);

  if ((len % 2) == 0)
    {
      i = 0;
    }
  else
    {
      i = 1;

      val = atoh(buffer[0]);
      if (val < 0 || pos >= buffer_len)
        return pos;

      buffer[pos++] = (uint8_t) val;
    }

  for (; i < len; i += 2)
    {
      val = atoh(input[i]);
      v = atoh(input[i + 1]);

      if (val < 0 || v < 0 || pos >= buffer_len)
        return pos;

      val <<= 4;
      val |= v;

      buffer[pos++] = (uint8_t) val;
    }

  return pos;
}

void
GetPut::eeprom_parse_data(const char *val, uint8_t *buf, size_t buflen,
                          int eeprom_addr)
{
  memset(buf, 0, buflen);
  hex_decode(val, buf, buflen);
  eeprom_write_data(buf, buflen, eeprom_addr);
}

void
GetPut::eeprom_read_data(uint8_t *buf, size_t buflen, int eeprom_addr)
{
  int i;

  for (i = 0; i < buflen; i++)
    buf[i] = EEPROM.read(eeprom_addr + i);
}

void
GetPut::eeprom_write_data(uint8_t *buf, size_t buflen, int eeprom_addr)
{
  int i;

  for (i = 0; i < buflen; i++)
    EEPROM.write(eeprom_addr + i, buf[i]);
}

void
GetPut::eeprom_print_ascii(int eeprom_addr, int max_len)
{
  int i;

  for (i = 0; i < max_len; i++)
    {
      uint8_t byte = EEPROM.read(eeprom_addr + i);

      if (!byte)
        break;

      Serial.write(byte);
    }
}
