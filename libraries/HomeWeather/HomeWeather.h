/* -*- c++ -*-
 *
 * HomeWeather.h
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

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <Ethernet.h>
#include <avr/pgmspace.h>

#define MSG_CLIENT_ID		0
#define MSG_SEQNUM		1
#define MSG_SENSOR_ID		2
#define MSG_SENSOR_VALUE	3

class HomeWeather
{
public:

  /* Print label `label' with the indentation `indent'. */
  static void print_label(int indent, const prog_char label[]);

  /* Print the data `data', `datalen' to serial line as hex string.
     The argument `label' specifies a label for the data. */
  static void print_data(int indent, const prog_char label[], uint8_t *data,
                         size_t datalen);

  /* Print the data `data', `datalen' to serial line as dotted decimal
     string.  The argument `label' specifies a label for the data. */
  static void print_dotted(int indent, const prog_char label[], uint8_t *data,
                           size_t datalen);

  static void print(const prog_char str[]);

  static void println(const prog_char str[]);

  static void print(Client *client, const prog_char str[]);

  static void println(Client *client, const prog_char str[]);

  static void newline(void);

  static void newline(Client *client);
};
