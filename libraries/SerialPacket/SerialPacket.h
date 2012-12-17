/* -*- c++ -*-
 *
 * SerialPacket.h
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

#ifndef SERIALPACKET_H
#define SERIALPACKET_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <SoftwareSerial.h>

class SerialPacket
{
 public:

  SerialPacket(SoftwareSerial *serial);

  /* Sends the packet `data', `data_len'.  The method returns true if
     the packet was sent and false on error. */
  bool send(uint8_t *data, size_t data_len);

  /* Receives a packet from the serial port. */
  uint8_t *receive(size_t *len_return);

  /* Clears the packet's buffer and prepare for new message
     construction. */
  void clear(void);

  bool add_message(uint8_t type, const uint8_t *data, size_t data_len);

  bool add_message(uint8_t type, uint32_t value);

  /* Sends the message that has been currenlty constructed using the
     clear() and add_message() methods. */
  bool send(void);

  static bool parse_message(uint8_t *type_return, uint8_t **msg_return,
                            size_t *msg_len_return,
                            uint8_t **datap, size_t *data_lenp);

  /* The number of packets received. */
  uint32_t num_packets;

  /* The number of errors received. */
  uint32_t num_errors;

 private:

  SoftwareSerial *serial;

  uint8_t buffer[256];
  size_t bufpos;
};

#endif /* not SERIALPACKET_H */
