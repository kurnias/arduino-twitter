/*
 * SerialPacket.cpp
 */

#include "SerialPacket.h"
#include <GetPut.h>

#define SP_SEP 0x80
#define SP_HDR 0x81
#define SP_TRL 0x82
#define SP_ESC 0xfe

SerialPacket::SerialPacket(SoftwareSerial *serial)
  : serial(serial),
    bufpos(0)
{
}

bool
SerialPacket::send(uint8_t *data, size_t data_len)
{
  size_t i;
  uint32_t crc = 0;

  if (data_len > sizeof(buffer))
    return false;

  /* Write header. */
  serial->write(SP_SEP);
  serial->write(SP_SEP);
  serial->write(SP_SEP);
  serial->write(SP_HDR);

  /* Data length. */
  serial->write((char) data_len);

  /* Write data escaping separators and escape bytes. */
  for (i = 0; i < data_len; i++)
    {
      crc = (crc << 8) + data[i] + (crc >> 11);

      switch (data[i])
        {
        case SP_SEP:
          serial->write(SP_ESC);
          serial->write(0x1);
          break;

        case SP_ESC:
          serial->write(SP_ESC);
          serial->write(0x2);
          break;

        default:
          serial->write(data[i]);
        }
    }

  /* Trailer. */
  serial->write(SP_SEP);

  /* CRC. */
  serial->write((crc >> 24) & 0xff);
  serial->write((crc >> 16) & 0xff);
  serial->write((crc >> 8) & 0xff);
  serial->write(crc & 0xff);

  return true;
}

uint8_t *
SerialPacket::receive(size_t *len_return)
{
  uint8_t last = SP_SEP + 1;
  uint8_t byte;
  size_t i;

  /* Read until we find a valid packet. */
  while (true)
    {
      uint32_t crc = 0;

      /* Read until we find the header. */
      while (true)
        {
          byte = serial->read();
          if (last == SP_SEP && byte == SP_HDR)
            break;

          last = byte;
        }

      /* Read data length. */
      last = serial->read();

      /* Read data. */
      for (i = 0; i < last; i++)
        {
          byte = serial->read();
          if (byte == SP_ESC)
            {
              byte = serial->read();
              switch (byte)
                {
                case 0x1:
                  byte = SP_SEP;
                  break;

                case 0x2:
                  byte = SP_ESC;
                  break;
                }
            }

          crc = (crc << 8) + byte + (crc >> 11);

          buffer[i] = byte;
        }

      /* Read trailer. */

      if (serial->read() != SP_SEP)
        {
          num_errors++;
          continue;
        }
      if (serial->read() != SP_TRL)
        {
          num_errors++;
          continue;
        }

      /* Read crc. */
      uint32_t val;

      val = serial->read();
      val <<= 8;
      val |= serial->read();
      val <<= 8;
      val |= serial->read();
      val <<= 8;
      val |= serial->read();

      if (val != crc)
        {
          num_errors++;
          continue;
        }

      *len_return = (size_t) last;

      num_packets++;

      return buffer;
    }
}

void
SerialPacket::clear(void)
{
  bufpos = 0;
}

bool
SerialPacket::add_message(uint8_t type, const uint8_t *data, size_t data_len)
{
  if (bufpos + 2 + data_len > sizeof(buffer))
    return false;
  if (data_len > 0xff)
    return false;

  buffer[bufpos++] = type;
  buffer[bufpos++] = data_len;

  memcpy(buffer + bufpos, data, data_len);
  bufpos += data_len;

  return true;
}

bool
SerialPacket::add_message(uint8_t type, uint32_t value)
{
  if (bufpos + 6 > sizeof(buffer))
    return false;

  buffer[bufpos++] = type;
  buffer[bufpos++] = 4;

  GetPut::put_32bit(buffer + bufpos, value);
  bufpos += 4;

  return true;
}

bool
SerialPacket::send(void)
{
  return send(buffer, bufpos);
}

bool
SerialPacket::parse_message(uint8_t *type_return, uint8_t **msg_return,
                            size_t *msg_len_return,
                            uint8_t **datap, size_t *data_lenp)
{
  uint8_t *data = *datap;
  size_t data_len = *data_lenp;
  uint8_t val;

  if (data_len < 2)
    return false;

  *type_return = data[0];
  *msg_len_return = data[1];

  data += 2;
  data_len -= 2;

  if (*msg_len_return > data_len)
    return false;

  *msg_return = data;

  data += *msg_len_return;
  data_len -= *msg_len_return;

  *datap = data;
  *data_lenp = data_len;

  return true;
}
