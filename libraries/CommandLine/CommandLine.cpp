/*
 * CommandLine.cpp
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

#include "CommandLine.h"

/* Test if the character `ch' is a whitespace character. */
#define ISSPACE(ch)     \
((ch) == ' ' || (ch) == '\t' || (ch) == '\r' || (ch) == '\n')

CommandLine::CommandLine()
  : buffer_pos(0),
    argc(0)
{
}

bool
CommandLine::read(void)
{
  while (Serial.available() > 0)
    {
      uint8_t byte = Serial.read();

      if (byte == '\n')
        {
          buffer[buffer_pos] = '\0';
          return split_arguments();
        }

      /* Reserver space for the trailing null-character. */
      if (buffer_pos + 2 < sizeof(buffer))
        buffer[buffer_pos++] = byte;
    }

  return false;
}

char **
CommandLine::get_arguments(int *argc_return)
{
  *argc_return = argc;

  /* Prepare for the next line. */
  buffer_pos = 0;
  argc = 0;

  return argv;
}

bool
CommandLine::split_arguments(void)
{
  size_t i = 0;

  for (argc = 0; argc < COMMAND_LINE_MAX_ARGS; argc++)
    {
      /* Skip leading whitespace. */
      for (; i < buffer_pos && ISSPACE(buffer[i]); i++)
        ;
      if (i >= buffer_pos)
        break;

      argv[argc] = (char *) (buffer + i);

      /* Find the end of this argument. */
      for (; i < buffer_pos && !ISSPACE(buffer[i]); i++)
        ;
      if (i >= buffer_pos)
        {
          argc++;
          break;
        }

      /* Make argument null-terminated. */
      buffer[i++] = '\0';
    }

  return argc > 0;
}
