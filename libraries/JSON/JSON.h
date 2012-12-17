/* -*- c++ -*-
 *
 * JSON.h
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

#ifndef JSON_H
#define JSON_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <avr/pgmspace.h>

#define JSON_STACK_SIZE 8

class JSON
{
public:

  JSON(char *buffer, size_t buffer_len);

  void clear(void);

  bool add_object(void);

  bool add(const prog_char key[], int32_t value);
  bool add(const prog_char key[], const char *value);
  bool add(const prog_char key[], const uint8_t *data, size_t data_len);
  bool add_array(const prog_char key[]);

  bool pop(void);
  char *finish(void);

private:

  bool push(char type);
  bool append(const char *value);
  bool append_progstr(const prog_char value[]);
  bool append(int32_t value);
  bool is_object();
  bool obj_separator();

  char *buffer;
  size_t buffer_len;
  size_t buffer_pos;

  uint8_t stack_pos;
  char stack[JSON_STACK_SIZE];
};

#endif /* not JSON_H */
