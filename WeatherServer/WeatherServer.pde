/* -*- c++ -*-
 *
 * WeatherServer.pde
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

#include <SPI.h>
#include <Ethernet.h>
#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SoftwareSerial.h>
#include <SerialPacket.h>
#include <CommandLine.h>
#include <GetPut.h>
#include <HomeWeather.h>
#include <ClientInfo.h>
#include <JSON.h>
#include <sha1.h>

/* RF pins. */
#define RF_RX_PIN 2
#define RF_TX_PIN 3

/* OneWire bus pin. */
#define ONE_WIRE_BUS 4

#define ID_LEN 8
#define SECRET_LEN 8

#define HTTP_SERVER_LEN 32

/* EEPROM addresses. */
#define EEPROM_ADDR_CONFIGURED	0
#define EEPROM_ADDR_ID		(EEPROM_ADDR_CONFIGURED + 1)
#define EEPROM_ADDR_SECRET	(EEPROM_ADDR_ID + ID_LEN)
#define EEPROM_ADDR_VERBOSE	(EEPROM_ADDR_SECRET + SECRET_LEN)

#define EEPROM_ADDR_MAC		(EEPROM_ADDR_VERBOSE + 1)
#define EEPROM_ADDR_IP		(EEPROM_ADDR_MAC + 6)
#define EEPROM_ADDR_GATEWAY	(EEPROM_ADDR_IP + 4)
#define EEPROM_ADDR_SUBNET	(EEPROM_ADDR_GATEWAY + 4)

#define EEPROM_ADDR_HTTP_SERVER	(EEPROM_ADDR_SUBNET + 4)
#define EEPROM_ADDR_HTTP_PORT	(EEPROM_ADDR_HTTP_SERVER + HTTP_SERVER_LEN)
#define EEPROM_ADDR_PROXY_SERVER	(EEPROM_ADDR_HTTP_PORT + 2)
#define EEPROM_ADDR_PROXY_PORT	(EEPROM_ADDR_PROXY_SERVER + 4)

#define OAUTH_ITEM_MAX_LENGTH	128

#define EEPROM_ADDR_ACCESS_TOKEN	256
#define EEPROM_ADDR_TOKEN_SECRET	384

/* Next free EEPROM address is 512 which is the end of EEPROM on
   ATmega168.  The ATmega328 has additional 512 bytes of EEPROM left
   (total of 1kB). */

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

/* Initial device configuration missing. */
#define RUNLEVEL_CONFIG	0

/* Resolving HTTP server IP address. */
#define RUNLEVEL_DNS	1

/* Normal runlevel. */
#define RUNLEVEL_RUN	2

int runlevel;

uint8_t mac[6];
uint8_t ip[4];
uint8_t gateway[4];
uint8_t subnet[4];

/* Is ethernet configured? */
bool ethernet_configured = false;

/* HTTP server DNS name. */
uint8_t http_server[HTTP_SERVER_LEN];

/* HTTP server port number. */
uint16_t http_port;

/* Should we use proxy server? */
bool use_proxy = false;

/* HTTP proxy server IP address. */
uint8_t proxy_server[4];

/* HTTP proxy server port number. */
uint16_t proxy_port;

uint32_t msg_seqnum = 0;
unsigned long basetime = 0;

#define MAX_CLIENTS 2

ClientInfo clients[MAX_CLIENTS];

char json_buffer[512];
JSON json = JSON(json_buffer, sizeof(json_buffer));

const prog_char bannerstr[] PROGMEM = "\
WeatherServer <http://www.iki.fi/mtr/HomeWeather/>\n\
Copyright (c) 2011 Markku Rossi <mtr@iki.fi>\n\
\n";

const char usagestr[] PROGMEM = "\
Available commands are:\n\
  help          print this help\n\
  set VAR VAL   sets the EEPROM variable VAR to the value VAL.  Possible\n\
                variables are:\n\
                  `id', `secret', `verbose', `configured',\n\
                  `mac', `ip', `gw', `subnet'\n\
  access-token  read OAuth access token from input\n\
  token-secret  read OAuth token secret from input\n\
  info          show current weather information\n";

void
setup(void)
{
  int i;
  uint8_t buf[2];

  Serial.begin(9600);
  HomeWeather::print(bannerstr);

  /* Start temperature sensors. */
  sensors.begin();

  pinMode(RF_RX_PIN, INPUT);
  pinMode(RF_TX_PIN, OUTPUT);

  rf_serial.begin(2400);

  /* Read configuration parameters. */

  GetPut::eeprom_read_data(id, sizeof(id), EEPROM_ADDR_ID);
  GetPut::eeprom_read_data(secret, sizeof(secret), EEPROM_ADDR_SECRET);

  verbose = EEPROM.read(EEPROM_ADDR_VERBOSE);

  GetPut::eeprom_read_data(mac, sizeof(mac), EEPROM_ADDR_MAC);
  GetPut::eeprom_read_data(ip,	sizeof(ip), EEPROM_ADDR_IP);
  GetPut::eeprom_read_data(gateway, sizeof(gateway), EEPROM_ADDR_GATEWAY);
  GetPut::eeprom_read_data(subnet, sizeof(subnet), EEPROM_ADDR_SUBNET);

  GetPut::eeprom_read_data(http_server, sizeof(http_server),
                           EEPROM_ADDR_HTTP_SERVER);
  if (http_server[0] == 0xff)
    http_server[0] = '\0';

  GetPut::eeprom_read_data(buf, sizeof(buf), EEPROM_ADDR_HTTP_PORT);
  http_port = GetPut::get_16bit(buf);

  GetPut::eeprom_read_data(proxy_server, sizeof(proxy_server),
                           EEPROM_ADDR_PROXY_SERVER);
  GetPut::eeprom_read_data(buf, sizeof(buf), EEPROM_ADDR_PROXY_PORT);
  proxy_port = GetPut::get_16bit(buf);

  HomeWeather::print_data(12,      PSTR("id"), id, sizeof(id));
  HomeWeather::print_data(8,   PSTR("secret"), secret, sizeof(secret));
  HomeWeather::print_data(11,     PSTR("mac"), mac, sizeof(mac));
  HomeWeather::print_dotted(12,    PSTR("ip"), ip, sizeof(ip));
  HomeWeather::print_dotted(12,    PSTR("gw"), gateway, sizeof(gateway));
  HomeWeather::print_dotted(8, PSTR("subnet"), subnet, sizeof(subnet));

  HomeWeather::print_label(3, PSTR("http-server"));
  Serial.println((char *) http_server);
  HomeWeather::print_label(5, PSTR("http-port"));
  Serial.println(http_port);

  HomeWeather::print_dotted(2, PSTR("proxy-server"),
                            proxy_server, sizeof(proxy_server));
  HomeWeather::print_label(4, PSTR("proxy-port"));
  Serial.println(proxy_port);

  HomeWeather::print_label(2, PSTR("access-token"));
  GetPut::eeprom_print_ascii(EEPROM_ADDR_ACCESS_TOKEN, OAUTH_ITEM_MAX_LENGTH);
  Serial.println("");

  HomeWeather::print_label(2, PSTR("token-secret"));
  GetPut::eeprom_print_ascii(EEPROM_ADDR_TOKEN_SECRET, OAUTH_ITEM_MAX_LENGTH);
  Serial.println("");

  HomeWeather::print_label(7, PSTR("verbose"));
  Serial.println((int) verbose);

  if (EEPROM.read(EEPROM_ADDR_CONFIGURED) != 1)
    runlevel = RUNLEVEL_CONFIG;
  else
    runlevel = RUNLEVEL_DNS;

  HomeWeather::print_label(6, PSTR("runlevel"));
  Serial.println((int) runlevel);
}

void
process_command(void)
{
  int argc;
  char **argv = cmdline.get_arguments(&argc);
  int i;
  uint8_t buf[2];

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
          HomeWeather::println(PSTR("Invalid amount of arguments"));
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
      else if (strcmp_P(argv[1], PSTR("configured")) == 0)
        {
          int val = atoi(argv[2]);

          if (val)
            runlevel = RUNLEVEL_DNS;
          else
            runlevel = RUNLEVEL_CONFIG;

          EEPROM.write(EEPROM_ADDR_CONFIGURED, val ? 1 : 0);
        }
      else if (strcmp_P(argv[1], PSTR("mac")) == 0)
        {
          GetPut::eeprom_parse_data(argv[2], mac, sizeof(mac), EEPROM_ADDR_MAC);
        }
      else if (strcmp_P(argv[1], PSTR("ip")) == 0)
        {
          GetPut::eeprom_parse_data(argv[2], ip, sizeof(ip), EEPROM_ADDR_IP);
        }
      else if (strcmp_P(argv[1], PSTR("gw")) == 0)
        {
          GetPut::eeprom_parse_data(argv[2], gateway, sizeof(gateway),
                                    EEPROM_ADDR_GATEWAY);
        }
      else if (strcmp_P(argv[1], PSTR("subnet")) == 0)
        {
          GetPut::eeprom_parse_data(argv[2], subnet, sizeof(subnet),
                                    EEPROM_ADDR_SUBNET);
        }
      else if (strcmp_P(argv[1], PSTR("http-server")) == 0)
        {
          for (i = 0; argv[2][i] && i < HTTP_SERVER_LEN - 1; i++)
            http_server[i] = argv[2][i];
          http_server[i] = '\0';

          GetPut::eeprom_write_data(http_server, sizeof(http_server),
                                    EEPROM_ADDR_HTTP_SERVER);
        }
      else if (strcmp_P(argv[1], PSTR("http-port")) == 0)
        {
          http_port = atoi(argv[2]);

          GetPut::put_16bit(buf, http_port);
          GetPut::eeprom_write_data(buf, sizeof(buf), EEPROM_ADDR_HTTP_PORT);
        }
      else if (strcmp_P(argv[1], PSTR("proxy-server")) == 0)
        {
          GetPut::eeprom_parse_data(argv[2], proxy_server, sizeof(proxy_server),
                                    EEPROM_ADDR_PROXY_SERVER);
        }
      else if (strcmp_P(argv[1], PSTR("proxy-port")) == 0)
        {
          proxy_port = atoi(argv[2]);

          GetPut::put_16bit(buf, proxy_port);
          GetPut::eeprom_write_data(buf, sizeof(buf), EEPROM_ADDR_PROXY_PORT);
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

static bool
read_line(Client *client, uint8_t *buffer, size_t buflen)
{
  size_t pos = 0;

  while (client->connected())
    {
      while (client->available() > 0)
        {
          uint8_t byte = client->read();

          if (byte == '\n')
            {
              /* EOF found. */
              if (pos < buflen)
                {
                  if (pos > 0 && buffer[pos - 1] == '\r')
                    pos--;

                  buffer[pos] = '\0';
                }
              else
                {
                  buffer[buflen - 1] = '\0';
                }

              return true;
            }

          if (pos < buflen)
            buffer[pos++] = byte;
        }

      delay(100);
    }

  return false;
}

/* Does a HTTP request with the server.  The argument `method'
   specifies the HTTP method and `uri' the URI at the server.  The
   argument `content_json' specifies the content JSON data.  The HTTP
   status code is returned in `http_code_return' and the content data
   is stored into `buffer', `buflen'.  The function returns true if
   the HTTP operation was successful and false on error. */
static bool
http_json_request(const prog_char method[], const prog_char uri[],
                  const char *content_json, int32_t *http_code_return,
                  uint8_t *buffer, size_t buflen)
{
  int i;
  char buf[8];
  size_t pos;

  Sha1.initHmac(secret, sizeof(secret));
  Sha1.print(content_json);

  uint8_t *server = proxy_server;
  uint16_t port = proxy_port;

  EthernetClient http_client;

  if (!http_client.connect(server, port))
    {
      if (verbose)
        HomeWeather::println(PSTR("Failed to connect to server"));
      return false;
    }

  HomeWeather::print(&http_client, method);

  if (use_proxy)
    {
      HomeWeather::print(&http_client, PSTR(" http://"));
      http_client.write((const char *) http_server);
    }
  else
    {
      HomeWeather::print(&http_client, PSTR(" "));
    }

  HomeWeather::print(&http_client, uri);
  HomeWeather::println(&http_client, PSTR(" HTTP/1.1"));
  HomeWeather::println(&http_client, PSTR("Content-Type: application/json"));

  HomeWeather::print(&http_client, PSTR("Content-Length: "));
  snprintf(buf, sizeof(buf), "%d", strlen(content_json));
  http_client.write(buf);
  HomeWeather::newline(&http_client);

  HomeWeather::println(&http_client, PSTR("Connection: close"));

  HomeWeather::print(&http_client, PSTR("Host: "));
  http_client.write((const char *) http_server);
  HomeWeather::newline(&http_client);

  HomeWeather::print(&http_client, PSTR("Authorization: HMAC-SHA-1 "));

  uint8_t *digest = Sha1.resultHmac();
  for (i = 0; i < HASH_LENGTH; i++)
    {
      snprintf(buf, sizeof(buf), "%02x", digest[i]);
      http_client.write(buf);
    }
  HomeWeather::newline(&http_client);

  /* Header-body separator. */
  HomeWeather::newline(&http_client);

  http_client.write(content_json);

  /* Read response status line. */
  if (!read_line(&http_client, buffer, buflen))
    {
      http_client.stop();
      return false;
    }

  if (buffer[0] == '\0')
    {
      /* HTTP/0.9 response. */
      *http_code_return = 200;
    }
  else
    {
      *http_code_return = atol((char *) buffer);

      /* Read until we find the header-body separator. */
      while (true)
        {
          if (!read_line(&http_client, buffer, buflen))
            {
              http_client.stop();
              return false;
            }

          if (buffer[0] == '\0')
            break;
        }
    }

  /* Collect content data to buffer. */
  pos = 0;
  while (http_client.connected())
    {
      while (http_client.available() > 0)
        {
          uint8_t byte = http_client.read();

          if (pos < buflen)
            buffer[pos++] = byte;
        }
      delay(100);
    }

  http_client.stop();

  if (pos < buflen)
    {
      buffer[pos] = '\0';
      return true;
    }

  return false;
}

static bool
get_parameters_from_server(void)
{
  char *json_data;
  int32_t code;
  char *data;
  char *end;

  json.clear();
  json.add_object();

  json.add(PSTR("id"), id, sizeof(id));

  json_data = json.finish();
  if (verbose > 1)
    Serial.println(json_data);

  if (!http_json_request(PSTR("GET"), PSTR("/data_api/params"), json_data,
                         &code, (uint8_t *) json_buffer, sizeof(json_buffer))
      || code < 200 || code >= 300)
    {
      HomeWeather::println("Failed to get parameters");
      return false;
    }

  /* Parse response. */
  data = json_buffer;
  while (data[0])
    {
      switch (data[0])
        {
        case 's':
          if (data[1] != ':')
            return false;

          data += 2;
          msg_seqnum = strtoul(data, &end, 0);
          if (end == data)
            return false;

          data = end;
          break;

        case 't':
          if (data[1] != ':')
            return false;

          data += 2;
          basetime = strtoul(data, &end, 0);
          if (end == data)
            return false;

          data = end;

          basetime -= millis() / 1000L;

          /* Now basetime + millis() is approximately the current
             time. */
          break;

        default:
          if (isspace(data[0]))
            data++;
          else
            return false;
          break;
        }

      if (data[0] == ',')
        data++;
    }

  return true;
}

static void
resolve_dns(void)
{
  if (!ethernet_configured)
    {
      /* Configure our ethernet interface. */

      if (verbose)
        HomeWeather::println(PSTR("Configuring ethernet"));

      Ethernet.begin(mac, ip, gateway, subnet);
      ethernet_configured = true;
    }

  if (proxy_port != 0xffff)
    {
      /* We have a HTTP proxy server configuration.  No need to
         resolve server DNS. */

      if (verbose)
        HomeWeather::println(PSTR("Using proxy server"));

      use_proxy = true;
      runlevel = RUNLEVEL_RUN;
      return;
    }

  HomeWeather::println(PSTR("Resolving server IP"));
}

static void
poll_rf_clients(void)
{
  uint8_t *data;
  size_t data_len;
  uint8_t msg_type;
  uint8_t *msg_data;
  size_t msg_len;
  uint32_t val;
  ClientInfo *client;
  SensorValue *sensor = 0;

  data = serial_packet.receive(&data_len);

  if (!SerialPacket::parse_message(&msg_type, &msg_data, &msg_len,
                                   &data, &data_len)
      || msg_type != MSG_CLIENT_ID)
    {
      HomeWeather::println(PSTR("Malformed packet"));
      return;
    }

  client = ClientInfo::lookup(clients, MAX_CLIENTS, msg_data, msg_len);
  if (!client)
    {
      HomeWeather::println(PSTR("Too many clients"));
      return;
    }

  if (!SerialPacket::parse_message(&msg_type, &msg_data, &msg_len,
                                   &data, &data_len)
      || msg_type != MSG_SEQNUM
      || msg_len != 4)
    {
      HomeWeather::println(PSTR("Malformed packet"));
      return;
    }

  val = GetPut::get_32bit(msg_data);

  if (val > client->last_seqnum)
    client->packetloss += val - client->last_seqnum - 1;

  client->last_seqnum = val;
  client->dirty = true;

  /* Process all info messages. */
  while (data_len > 0)
    {
      if (!SerialPacket::parse_message(&msg_type, &msg_data, &msg_len,
                                       &data, &data_len))
        {
          HomeWeather::println(PSTR("Malformed packet"));
          return;
        }

      switch (msg_type)
        {
        case MSG_SENSOR_ID:
          sensor = client->lookup(msg_data, msg_len);
          if (!sensor)
            {
              HomeWeather::println(PSTR("Too many sensors"));
              return;
            }
          break;

        case MSG_SENSOR_VALUE:
          if (sensor == 0 || msg_len != 4)
            {
              HomeWeather::println(PSTR("Malformed packet"));
              return;
            }

          sensor->value = (int32_t) GetPut::get_32bit(msg_data);
          sensor->dirty = true;
          sensor = 0;
          break;

        default:
          HomeWeather::println(PSTR("Malformed packet"));
          return;
          break;
        }
    }
}

static void
poll_local_sensors(void)
{
  DeviceAddress addr;
  int count;
  int i;
  ClientInfo *client;
  SensorValue *sensor;

  client = ClientInfo::lookup(clients, MAX_CLIENTS, id, sizeof(id));
  if (!client)
    {
      HomeWeather::println(PSTR("Too many clients"));
      return;
    }

  sensors.requestTemperatures();

  count = sensors.getDeviceCount();
  for (i = 0; i < count; i++)
    {
      if (!sensors.getAddress(addr, i))
        continue;

      float temp = sensors.getTempC(addr);

      if (temp == DEVICE_DISCONNECTED)
        continue;

      sensor = client->lookup(addr, sizeof(addr));
      if (!sensor)
        {
          HomeWeather::println(PSTR("Too many sensors"));
          continue;
        }

      sensor->value = (int32_t) (temp * 100);
      sensor->dirty = true;
      client->dirty = true;
    }
}

static void
post_data_to_server(void)
{
  ClientInfo *client;
  SensorValue *sensor;
  int i, j;
  char *json_data;
  int32_t code;

  /* Post data to server. */

  json.clear();
  json.add_object();

  json.add(PSTR("id"), id, sizeof(id));
  json.add(PSTR("sn"), msg_seqnum++);

  json.add_array(PSTR("c"));

  for (i = 0; i < MAX_CLIENTS; i++)
    {
      client = &clients[i];

      if (client->id_len == 0 || !client->dirty)
        continue;

      json.add_object();

      json.add(PSTR("id"), client->id, client->id_len);

      if (client->packetloss)
        {
          json.add(PSTR("loss"), client->packetloss);
          client->packetloss = 0;
        }

      json.add_array(PSTR("s"));

      for (j = 0; j < CLIENT_INFO_MAX_SENSORS; j++)
        {
          sensor = &client->sensors[j];

          if (sensor->id_len == 0 || !sensor->dirty)
            continue;

          json.add_object();

          json.add(PSTR("id"), sensor->id, sensor->id_len);
          json.add(PSTR("v"), sensor->value);

          json.pop();

          sensor->dirty = false;
        }

      /* Finish sensors array. */
      json.pop();

      /* Finish client object. */
      json.pop();

      client->dirty = false;
    }

  json_data = json.finish();
  if (verbose > 1)
    Serial.println(json_data);

  if (!http_json_request(PSTR("POST"), PSTR("/data_api/add"), json_data,
                         &code, (uint8_t *) json_buffer, sizeof(json_buffer))
      || code < 200 || code >= 300)
    HomeWeather::println(PSTR("Data sending failed"));
}

void
loop(void)
{
  if (cmdline.read())
    process_command();

  switch (runlevel)
    {
    case RUNLEVEL_CONFIG:
      /* Just process command line arguments. */
      break;

    case RUNLEVEL_DNS:
      resolve_dns();
      break;

    case RUNLEVEL_RUN:
      // poll_rf_clients();
      poll_local_sensors();
      post_data_to_server();
      break;
    }

  delay(500);
}
