/* -*- c++ -*-
 *
 * WeatherClient.pde
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

#include <EEPROM.h>
#include <SPI.h>
#include <Ethernet.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SoftwareSerial.h>
#include <SerialPacket.h>
#include <CommandLine.h>
#include <GetPut.h>
#include <HomeWeather.h>

/* Temperature sensor data wire is plugged into port 2 on the
   Arduino. */
#define ONE_WIRE_BUS 4

/* RF pins. */
#define RF_RX_PIN 2
#define RF_TX_PIN 3

#define ID_LEN 8
#define SECRET_LEN 8

/* EEPROM addresses. */
#define EEPROM_ADDR_CONFIGURED	0
#define EEPROM_ADDR_ID		(EEPROM_ADDR_CONFIGURED + 1)
#define EEPROM_ADDR_SECRET	(EEPROM_ADDR_ID + ID_LEN)
#define EEPROM_ADDR_VERBOSE	(EEPROM_ADDR_SECRET + SECRET_LEN)

#define OAUTH_ITEM_MAX_LENGTH	128

#define EEPROM_ADDR_ACCESS_TOKEN	256
#define EEPROM_ADDR_TOKEN_SECRET	384

SoftwareSerial rf_serial = SoftwareSerial(RF_RX_PIN, RF_TX_PIN);
SerialPacket serial_packet = SerialPacket(&rf_serial);

/* Setup a OneWire instance to communicate with any OneWire devices
   (not just Maxim/Dallas temperature ICs). */
OneWire one_wire(ONE_WIRE_BUS);

/* Dallas Temperature library running on our OneWire bus. */
DallasTemperature sensors(&one_wire);

CommandLine cmdline = CommandLine();

uint8_t id[ID_LEN];
uint8_t secret[SECRET_LEN];

uint8_t verbose = 0;

uint32_t msg_seqnum;

const char bannerstr[] PROGMEM = "\
WeatherClient <http://www.iki.fi/mtr/HomeWeather/>\n\
Copyright (c) 2011 Markku Rossi <mtr@iki.fi>\n\
\n";

const char usagestr[] PROGMEM = "\
Available commands are:\n\
  help          print this help\n\
  set VAR VAL   sets the EEPROM variable VAR to the value VAL.  Possible\n\
                variables are:\n\
                  `id', `secret', and `verbose'\n\
  access-token  read OAuth access token from input\n\
  token-secret  read OAuth token secret from input\n\
  info          show current weather information\n";

const char err_cmd_invalid_args[] PROGMEM = "Invalid amount of arguments\n";

void
setup(void)
{
  int i;

  Serial.begin(9600);
  HomeWeather::print(bannerstr);

  /* Start temperature sensors. */
  sensors.begin();

  /* Start RF transmitter. */
  pinMode(RF_RX_PIN, INPUT);
  pinMode(RF_TX_PIN, OUTPUT);

  rf_serial.begin(2400);

  msg_seqnum = 0;

  /* Read configuration parameters.*/

  GetPut::eeprom_read_data(id, sizeof(id), EEPROM_ADDR_ID);
  GetPut::eeprom_read_data(secret, sizeof(secret), EEPROM_ADDR_SECRET);

  verbose = EEPROM.read(EEPROM_ADDR_VERBOSE);

  HomeWeather::print_data(7,       PSTR("id"), id, sizeof(id));
  HomeWeather::print_data(3,   PSTR("secret"), secret, sizeof(secret));

  HomeWeather::print_label(2, PSTR("access-token"));
  GetPut::eeprom_print_ascii(EEPROM_ADDR_ACCESS_TOKEN, OAUTH_ITEM_MAX_LENGTH);
  Serial.println("");

  HomeWeather::print_label(2, PSTR("token-secret"));
  GetPut::eeprom_print_ascii(EEPROM_ADDR_TOKEN_SECRET, OAUTH_ITEM_MAX_LENGTH);
  Serial.println("");

  HomeWeather::print_label(2, PSTR("verbose"));
  Serial.println((int) verbose);
}

void
process_command(void)
{
  int argc;
  char **argv = cmdline.get_arguments(&argc);
  int i;

  if (argc < 1)
    return;

  if (strcmp_P(argv[0], PSTR("help")) == 0)
    {
      HomeWeather::print(usagestr);
    }
  else if (strcmp_P(argv[0], PSTR("set")) == 0)
    {
      if (argc != 3)
        {
          HomeWeather::print(err_cmd_invalid_args);
          return;
        }

      if (strcmp_P(argv[1], PSTR("id")) == 0)
        {
          GetPut::eeprom_parse_data(argv[2], id, sizeof(id), EEPROM_ADDR_ID);
        }
      else if (strcmp_P(argv[1], PSTR("secret")) == 0)
        {
          GetPut::eeprom_parse_data(argv[2], secret, sizeof(secret),
                                    EEPROM_ADDR_SECRET);
        }
      else if (strcmp_P(argv[1], PSTR("verbose")) == 0)
        {
          verbose = (uint8_t) atoi(argv[2]);
          EEPROM.write(EEPROM_ADDR_VERBOSE, verbose);
        }
      else
        {
          HomeWeather::print(PSTR("Unknown variable `"));
          Serial.print(argv[1]);
          HomeWeather::println(PSTR("'"));
        }
    }
  else if (strcmp_P(argv[0], PSTR("info")) == 0)
    {
    }
  else if (strcmp_P(argv[0], PSTR("access-token")) == 0)
    {
      HomeWeather::println(PSTR("Type Oauth access token followed by newline"));
      for (i = 0; i < OAUTH_ITEM_MAX_LENGTH - 1; i++)
        {
          while (Serial.available() <= 0)
            delay(100);

          uint8_t byte = Serial.read();
          if (byte == '\r' || byte == '\n')
            break;

          EEPROM.write(EEPROM_ADDR_ACCESS_TOKEN + i, byte);
        }
      EEPROM.write(EEPROM_ADDR_ACCESS_TOKEN + i, 0);

      HomeWeather::print(PSTR("Read "));
      Serial.print(i);
      HomeWeather::println(PSTR(" bytes"));
    }
  else if (strcmp_P(argv[0], PSTR("token-secret")) == 0)
    {
      HomeWeather::println(PSTR("Type Oauth token secret followed by newline"));
      for (i = 0; i < OAUTH_ITEM_MAX_LENGTH - 1; i++)
        {
          while (Serial.available() <= 0)
            delay(100);

          uint8_t byte = Serial.read();
          if (byte == '\r' || byte == '\n')
            break;

          EEPROM.write(EEPROM_ADDR_TOKEN_SECRET + i, byte);
        }
      EEPROM.write(EEPROM_ADDR_TOKEN_SECRET + i, 0);

      HomeWeather::print(PSTR("Read "));
      Serial.print(i);
      HomeWeather::println(PSTR(" bytes"));
    }
  else
    {
      HomeWeather::print(PSTR("Unknown command `"));
      Serial.print(argv[0]);
      HomeWeather::println(PSTR("'"));
    }
}

void
loop(void)
{
  if (cmdline.read())
    process_command();

  sensors.requestTemperatures();

  /* Construct message containing all sensor readings. */

  DeviceAddress addr;
  int count = sensors.getDeviceCount();
  int i;

  serial_packet.clear();

  serial_packet.add_message(MSG_CLIENT_ID, id, sizeof(id));
  serial_packet.add_message(MSG_SEQNUM, msg_seqnum++);

  for (i = 0; i < count; i++)
    {
      if (!sensors.getAddress(addr, i))
        continue;

      float temp = sensors.getTempC(addr);

      if (temp == DEVICE_DISCONNECTED)
        continue;

      if (verbose)
        {
          Serial.print("Temperature ");
          Serial.print(i);
          Serial.print(" is: ");
          Serial.println(temp);
        }

      serial_packet.add_message(MSG_SENSOR_ID, addr, sizeof(addr));
      serial_packet.add_message(MSG_SENSOR_VALUE, (uint32_t) (temp * 100));
    }

  serial_packet.send();

  delay(500);
}
