/* -*- c++ -*-
 *
 * Twitter.h
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

#ifndef TWITTER_H
#define TWITTER_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <avr/pgmspace.h>
#include <EEPROM.h>
#include <sha1.h>
#include <Ethernet.h>
#include <Time.h>

class Twitter
{
public:

  /* Constructs a new Twitter instance with the work buffer `buffer',
     `buffer_len'. */
  Twitter(char *buffer, size_t buffer_len);

  /* Set the Twitter API endpoint configuration.

     The argument `server' specifies the fully qualified server name
     (e.g. `api.twitter.com').  The argument `uri' contains the API
     URI at the server (e.g. `/1/statuses/update.json').

     The arguments `ip', `port' specify the HTTP connection end-point.
     If the argument `proxy' is false, the connection end-point must
     be the Twitter server.  If the argument `proxy' is true, the
     connection end-point must be specify you HTTP proxy server to use
     for the operation.  The `ip' array must remain valid as long as
     this twitter instance is used; the method does not copy the `ip'
     argument but stores a pointer to the provided value. */
  void set_twitter_endpoint(const prog_char server[], const prog_char uri[],
                            IPAddress ip, uint16_t port, bool proxy);

  /* Set the client application identification to `consumer_key',
     `consumer_secret'. */
  void set_client_id(const prog_char consumer_key[],
                     const prog_char consumer_secret[]);

  /* Set the twitter account identification to program memory data
     `access_token', `token_secret. */
  void set_account_id(const prog_char access_token[],
                      const prog_char token_secret[]);

  /* Set the twitter account identification to EEPROM memory data
     `access_token', `token_secret'. */
  void set_account_id(int access_token, int token_secret);

  /* Tests if this twitter instance is ready for twitter
     communication.  The method returns true if twitter messages can
     be sent and false if the twitter instance is still initializing.
     You should keep calling this method from your loop() and do your
     twitter interactions when this method returns true. */
  bool is_ready(void);

  /* Gets the current UTC time.  The twitter module queries and
     maintains the UTC time based on server HTTP `Date' response
     header.  The method returns the current UTC Unix time in seconds,
     or 0 if the time has not been resolved yet.  This will return a
     non-zero value after init() has returned true. */
  unsigned long get_time(void);

  /* Post status message `message' to twitter.  The message must be
     UTF-8 encoded.  The method returns true if the status message was
     posted and false on error. */
  bool post_status(const char *message);

  /* URL encode character `ch' into the buffer `buffer'.  The method
     returns a pointer to the next byte after the encoded value. */
  static char *url_encode(char *buffer, char ch);

  /* URL encode c-string `data' into the buffer `buffer'.  The method
     returns a pointer to the next byte after the encoded value. */
  static char *url_encode(char *buffer, const char *data);

  /* URL encode program memory c-string `data' into the buffer
     `buffer'.  The method returns a pointer to the next byte after
     the encoded value. */
  static char *url_encode_pgm(char *buffer, const prog_char data[]);

  /* URL encode EEPROM memory c-string that starts from address
     `address' into the buffer `buffer'.  The method returns a pointer
     to the next byte after the encoded value. */
  static char *url_encode_eeprom(char *buffer, int address);

  /* Hex encode binary data `data', `data_len' into the buffer
     `buffer'.  The method returns a pointer to the next byte after
     the encoded value. */
  static char *hex_encode(char *buffer, const uint8_t *data, size_t data_len);

  /* Base64 encode binary data `data', `data_len' into the buffer
     `buffer'.  The method returns a pointer to the next byte after
     the encoded value. */
  static char *base64_encode(char *buffer, const uint8_t *data,
                             size_t data_len);

private:

  /* Create a random nonce into the member `nonce.  The method uses
     `timestamp' as its random seed so you must set it before calling
     this method. */
  void create_nonce(void);

  /* Compute OAuth signature for the status message `message'.  The
     method uses the current state from consumer and access tokens and
     from `timestamp' and `nonce' member.  The computed signature will
     remain in the global `Sha1' instance and is pointed by
     `signature'.  You must not use the `Sha1' instance before you
     have consumed value. */
  void compute_authorization(const char *message);

  /* Add character `ch' into the authorization signature hmac. */
  void auth_add(char ch);

  /* Add string `str' into the authorization signature hmac. */
  void auth_add(const char *str);

  /* Add program memory string `str' into the authorization signature
     hmac. */
  void auth_add_pgm(const prog_char str[]);

  /* Add request parameter `key', `value' into the authorization
     signature hmac.  The argument `workbuf' specifies a working
     buffer for the method. */
  void auth_add_param(const char *key, const char *value, char *workbuf);

  /* Add authorization parameter separator into the authorization
     signature hmac. */
  void auth_add_param_separator(void);

  /* Add request key-value separator into the authorization signature
     hmac. */
  void auth_add_value_separator(void);

  /* Print program memory string `str' to the output stream of the
     HTTP client `client'. */
  static void http_print(Client *client, const prog_char str[]);

  /* Print program memory string `str' and line separator string to
     the output stream of the HTTP client `client'. */
  static void http_println(Client *client, const prog_char str[]);

  /* Print line separator string to the output stream of the HTTP
     client `client'. */
  static void http_newline(Client *client);

  /* Print the argument program memory string to serial output. */
  static void println(const prog_char str[]);

  /* Read a line from the connection `client' into the buffer `buffer'
     that has `buflen' bytes of space.  The method returns true if a
     line was read and false on error. */
  static bool read_line(Client *client, char *buffer, size_t buflen);

  /* Queries the current time with a HEAD request to the server.  The
     method returns true if the time was retrieved and false on
     error. */
  bool query_time(void);

  /* Process the response header line `buffer' and update the system
     `basetime' if the header is a HTTP Date header.  The method
     returns true if the header line was a `Date' header and false
     otherwise. */
  bool process_date_header(char *buffer);

  /* Parse the value of the HTTP Date header `date'.  The method
     returns the Unix time value in seconds or 0 if the header could
     not be parsed. */
  long parse_date(char *date);

  /* Parse month name `str' and return its number (1-12).  The method
     returns a pointer to the end of the parsed month in `end'.  The
     method returns 0 if the month could not be parsed. */
  int parse_month(char *str, char **end);

  /* The base timestamp for current time computation. */
  unsigned long basetime;

  /* Last milliseconds from the system clock. */
  unsigned long last_millis;

  /* Flags. */

  /* Is the provided HTTP end point a HTTP proxy or the Twitter
     server? */
  unsigned int proxy : 1;

  /* Is access token in PGM or in EEPROM? */
  unsigned int access_token_pgm : 1;

  /* Random nonce for the OAuth request. */
  uint8_t nonce[8];

  /* Request timestamp as Unix time. */
  unsigned long timestamp;

  /* This points to Sha1 so don't touch it before you have consumed
     the value. */
  uint8_t *signature;

  /* Work buffer. */
  char *buffer;

  /* The size of the work buffer `buffer'. */
  size_t buffer_len;

  /* Twitter server host name. */
  const prog_char *server;

  /* Twitter API URI at the server. */
  const prog_char *uri;

  /* An IP address to connnect to. */
  IPAddress ip;

  /* TCP port number to connnect to. */
  uint16_t port;

  /* Application consumer key. */
  const prog_char *consumer_key;

  /* Application consumer secret. */
  const prog_char *consumer_secret;

  /* Twitter account access token. */
  union
  {
    const prog_char *pgm;
    int eeprom;
  } access_token;

  /* Twitter account access token secret. */
  union
  {
    const prog_char *pgm;
    int eeprom;
  } token_secret;
};

#endif /* not TWITTER_H */
