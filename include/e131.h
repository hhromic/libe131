/**
 * E1.31 (sACN) library for C/C++
 * Hugo Hromic - http://github.com/hhromic
 *
 * Some content of this file is based on:
 * https://github.com/forkineye/E131/blob/master/E131.h
 * https://github.com/forkineye/E131/blob/master/E131.cpp
 *
 * Copyright 2016 Hugo Hromic
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _E131_H
#define _E131_H

#include <stdint.h>
#include <sys/types.h>
#include <netinet/in.h>

/* E1.31 Constants */
extern const uint16_t E131_DEFAULT_PORT;
extern const uint8_t E131_ACN_ID[];
extern const uint32_t E131_ROOT_VECTOR;
extern const uint32_t E131_FRAME_VECTOR;
extern const uint8_t E131_DMP_VECTOR;

/* E1.31 Socket Address Type */
typedef struct sockaddr_in e131_addr_t;

/* E1.31 Packet Type */
typedef union {
  struct {
    struct { /* Root Layer */
      uint16_t preamble_size;
      uint16_t postamble_size;
      uint8_t  acn_id[12];
      uint16_t flength;
      uint32_t vector;
      uint8_t  cid[16];
    } __attribute__((packed)) root;

    struct { /* Frame Layer */
      uint16_t flength;
      uint32_t vector;
      uint8_t  source_name[64];
      uint8_t  priority;
      uint16_t reserved;
      uint8_t  sequence_number;
      uint8_t  options;
      uint16_t universe;
    } __attribute__((packed)) frame;

    struct { /* DMP Layer */
      uint16_t flength;
      uint8_t  vector;
      uint8_t  type;
      uint16_t first_address;
      uint16_t address_increment;
      uint16_t property_value_count;
      uint8_t  property_values[513];
    } __attribute__((packed)) dmp;
  } __attribute__((packed));

  uint8_t raw[638]; /* Raw buffer data */
} e131_packet_t;

/* E1.31 Errors Type */
typedef enum {
  E131_ERR_NONE,
  E131_ERR_NULLPTR,
  E131_ERR_ACN_ID,
  E131_ERR_VECTOR_ROOT,
  E131_ERR_VECTOR_FRAME,
  E131_ERR_VECTOR_DMP
} e131_error_t;

/** Create a socket file descriptor suitable for E1.31 communication */
extern int e131_socket(void);

/** Bind a socket file descriptor to a port number for E1.31 communication */
extern int e131_bind(int sockfd, const uint16_t port);

/** Initialize a unicast E1.31 destination using a host and port number */
extern int e131_unicast_dest(const char *host, const uint16_t port, e131_addr_t *dest);

/** Initialize a multicast E1.31 destination using a universe and port number */
extern int e131_multicast_dest(const uint16_t universe, const uint16_t port, e131_addr_t *dest);

/** Join a socket file descriptor to a E1.31 multicast group using a universe */
extern int e131_multicast_join(int sockfd, const uint16_t universe);

/** Initialize a new E1.31 packet using a number of channels */
extern int e131_pkt_init(const uint16_t universe, const uint16_t num_channels, e131_packet_t *packet);

/** Send an E1.31 packet to a socket file descriptor using a destination */
extern ssize_t e131_send(int sockfd, const e131_packet_t *packet, const e131_addr_t *dest);

/** Receive an E1.31 packet from a socket file descriptor */
extern ssize_t e131_recv(int sockfd, e131_packet_t *packet);

/** Validate that an E1.31 packet is well-formed */
extern e131_error_t e131_pkt_validate(const e131_packet_t *packet);

/** Dump an E1.31 packet to the stderr output */
extern int e131_pkt_dump(const e131_packet_t *packet);

#endif
