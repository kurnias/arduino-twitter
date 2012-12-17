/* -*- c++ -*-
 *
 * ClientInfo.h
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

#ifndef CLIENTINFO_H
#define CLIENTINFO_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

/* The maximum sensors per client. */
#define CLIENT_INFO_MAX_SENSORS 4

class SensorValue
{
public:

  SensorValue();

  /* Is value modified. */
  bool dirty;

  /* The length of the sensor ID. */
  uint8_t id_len;

  /* Sensor ID. */
  uint8_t id[8];

  /* The value of the sensor. */
  int32_t value;
};

class ClientInfo
{
public:

  ClientInfo();

  /* Client values modified. */
  bool dirty;

  /* The length of the client ID. */
  uint8_t id_len;

  /* Unique client ID. */
  uint8_t id[8];

  /* The last client packet sequence number seen. */
  uint32_t last_seqnum;

  /* The number of packets lost. */
  uint32_t packetloss;

  /* The sensor values of this client. */
  SensorValue sensors[CLIENT_INFO_MAX_SENSORS];

  /* Look up sensor `id', `id_len'.  The method returns the sensor or
     0 if no such sensor is defined for the client info and there are
     no more space for the sensor value in the client. */
  SensorValue *lookup(const uint8_t *id, size_t id_len);

  /* Look up the client `id', `id_len'.  The method returns the client
     of 0 if no such client is defined for and there are no more space
     for the clients new clients in the `clients' array. */
  static ClientInfo *lookup(ClientInfo *clients, int num_clients,
                            const uint8_t *id, size_t id_len);
};

#endif /* not CLIENTINFO_H */
