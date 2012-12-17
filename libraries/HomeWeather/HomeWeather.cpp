/*
 * HomeWeather.cpp
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

#include "HomeWeather.h"

void
HomeWeather::print_label(int indent, const prog_char label[])
{
  while (indent--)
    Serial.write(' ');

  print(label);

  Serial.write(':');
  Serial.write(' ');
}

void
HomeWeather::print_data(int indent, const prog_char label[],
                        uint8_t *data, size_t datalen)
{
  int i;

  print_label(indent, label);

  for (i = 0; i < datalen; i++)
    Serial.print(data[i], HEX);

  newline();
}

void
HomeWeather::print_dotted(int indent, const prog_char label[],
                          uint8_t *data, size_t datalen)
{
  int i;

  print_label(indent, label);

  for (i = 0; i < datalen; i++)
    {
      if (i > 0)
        Serial.write('.');
      Serial.print((int) data[i]);
    }

  newline();
}

void
HomeWeather::print(const prog_char str[])
{
  int i;
  uint8_t c;

  if (!str)
    return;

  for (i = 0; (c = pgm_read_byte_near(str + i)); i++)
    Serial.write(c);
}

void
HomeWeather::println(const prog_char str[])
{
  print(str);
  newline();
}

void
HomeWeather::print(Client *client, const prog_char str[])
{
  int i;
  uint8_t c;

  if (!client || !str)
    return;

  for (i = 0; (c = pgm_read_byte_near(str + i)); i++)
    client->write(c);
}

void
HomeWeather::println(Client *client, const prog_char str[])
{
  print(client, str);
  newline(client);
}

void
HomeWeather::newline(void)
{
  Serial.write('\r');
  Serial.write('\n');
}

void
HomeWeather::newline(Client *client)
{
  client->write('\r');
  client->write('\n');
}
