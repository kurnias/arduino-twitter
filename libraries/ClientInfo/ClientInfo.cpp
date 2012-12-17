/*
 * ClientInfo.cpp
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

#include "ClientInfo.h"

SensorValue::SensorValue()
  : dirty(false),
    id_len(0),
    value(0)
{
}

ClientInfo::ClientInfo()
  : dirty(false),
    id_len(0),
    last_seqnum((uint32_t) -1),
    packetloss(0)
{
}

SensorValue *
ClientInfo::lookup(const uint8_t *id, size_t id_len)
{
  int i;
  SensorValue *value;

  for (i = 0; i < CLIENT_INFO_MAX_SENSORS; i++)
    {
      value = &sensors[i];

      if (value->id_len == 0)
        break;

      if (value->id_len == id_len && memcmp(id, value->id, id_len) == 0)
        return value;
    }

  if (i >= CLIENT_INFO_MAX_SENSORS)
    return 0;

  value = &sensors[i];

  value->id_len = (uint8_t) id_len;
  memcpy(value->id, id, id_len);

  return value;
}

ClientInfo *
ClientInfo::lookup(ClientInfo *clients, int num_clients,
                   const uint8_t *id, size_t id_len)
{
  int i;
  ClientInfo *client;

  for (i = 0; i < num_clients; i++)
    {
      client = &clients[i];

      if (client->id_len == 0)
        break;

      if (client->id_len == id_len && memcmp(client->id, id, id_len) == 0)
        return client;
    }

  if (i >= num_clients)
    return 0;

  client = &clients[i];

  client->id_len = (uint8_t) id_len;
  memcpy(client->id, id, id_len);

  return client;
}
