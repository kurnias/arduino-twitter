/*
 * JSON.cpp
 *
 * Author: Markku Rossi <mtr@iki.fi>
 *
 * Copyright (c) 2011 Markku Rossi
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

#include "JSON.h"

JSON::JSON(char *buffer, size_t buffer_len)
  : buffer(buffer),
    buffer_len(buffer_len),
    buffer_pos(0),
    stack_pos(0)
{
}

void
JSON::clear(void)
{
  buffer_pos = 0;
  stack_pos = 0;
}

bool
JSON::add_object(void)
{
  if (!obj_separator())
    return false;

  if (!push('o'))
    return false;

  return append("{");
}

bool
JSON::add(const prog_char key[], int32_t value)
{
  if (!is_object())
    return false;

  if (!obj_separator())
    return false;

  return append("\"") && append_progstr(key) && append("\":") && append(value);
}

bool
JSON::add(const prog_char key[], const char *value)
{
  if (!is_object())
    return false;

  if (!obj_separator())
    return false;

  return (append("\"") && append_progstr(key) && append("\":\"")
          && append(value) && append("\""));
}

bool
JSON::add(const prog_char key[], const uint8_t *data, size_t data_len)
{
  size_t i;
  char buf[4];

  if (!is_object())
    return false;

  if (!obj_separator())
    return false;

  if (!append("\"") || !append_progstr(key) || !append("\":\""))
    return false;

  for (i = 0; i < data_len; i++)
    {
      snprintf(buf, sizeof(buf), "%02x", data[i]);
      if (!append(buf))
        return false;
    }

  return append("\"");
}

bool
JSON::add_array(const char *key)
{
  if (!obj_separator())
    return false;

  if (!push('a'))
    return false;

  return append("\"") && append(key) && append("\":") && append("[");
}

bool
JSON::pop(void)
{
  const char *str = "";

  if (stack_pos <= 0)
    return false;

  switch (stack[--stack_pos])
    {
    case 'o':
      str = "}";
      break;

    case 'a':
      str = "]";
      break;

    default:
      return false;
      break;
    }

  return append(str);
}

char *
JSON::finish(void)
{
  while (stack_pos > 0)
    if (!pop())
      return 0;

  if (!append("x"))
    return false;

  buffer[buffer_pos - 1] = '\0';

  return buffer;
}

bool
JSON::push(char type)
{
  if (stack_pos >= JSON_STACK_SIZE)
    return false;

  stack[stack_pos++] = type;

  return true;
}

bool
JSON::append(const char *value)
{
  size_t len = strlen(value);

  if (buffer_pos + len > buffer_len)
    return false;

  memcpy(buffer + buffer_pos, value, len);
  buffer_pos += len;

  return true;
}

bool
JSON::append_progstr(const prog_char value[])
{
  size_t len = strlen_P(value);

  if (buffer_pos + len > buffer_len)
    return false;

  memcpy_P(buffer + buffer_pos, value, len);
  buffer_pos += len;

  return true;
}

bool
JSON::append(int32_t value)
{
  char buf[16];

  snprintf(buf, sizeof(buf), "%d", value);

  return append(buf);
}

bool
JSON::is_object()
{
  return stack_pos > 0 && stack[stack_pos - 1] == 'o';
}

bool
JSON::obj_separator()
{
  if (buffer_pos <= 0)
    return true;

  switch (buffer[buffer_pos - 1])
    {
    case '{':
    case '[':
      return true;
      break;

    default:
      break;
    }

  return append(",");
}
