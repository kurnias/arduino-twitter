/* -*- c++ -*-
 *
 * CommandLine.h
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

#ifndef COMMANDLINE_H
#define COMMANDLINE_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#define COMMAND_LINE_MAX_ARGS 4

class CommandLine
{
public:

  /* Initialize command line processing module. */
  CommandLine();

  /* Read more command line input from serial line.  Return true if a
     new line has been read and false otherwise. */
  bool read(void);

  /* Get the command line arguments of the latest command.  The method
     returns a pointer to the argument array.  The number of arguments
     is returned in `argc_return'. */
  char **get_arguments(int *argc_return);

private:

  /* Split the current command line into argument array.  Return true
     if the command was split and false on error or if the input line
     was empty. */
  bool split_arguments(void);

  /* Buffer for accumulating command line. */
  uint8_t buffer[64];
  uint8_t buffer_pos;

  /* Parsed command line arguments. */
  uint8_t argc;
  char *argv[COMMAND_LINE_MAX_ARGS];
};

#endif /* not COMMANDLINE_H */
