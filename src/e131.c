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

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include <netdb.h>
#include <e131.h>
#include <sys/socket.h>
#include <arpa/inet.h>

/* E1.31 Constants */
const uint16_t E131_DEFAULT_PORT = 5568;
const uint8_t E131_ACN_ID[] = {0x41, 0x53, 0x43, 0x2d, 0x45, 0x31, 0x2e, 0x31, 0x37, 0x00, 0x00, 0x00};
const uint32_t E131_ROOT_VECTOR = 0x00000004;
const uint32_t E131_FRAME_VECTOR = 0x00000002;
const uint8_t E131_DMP_VECTOR = 0x02;

/** Create a socket file descriptor suitable for E1.31 communication */
int e131_socket(void) {
  return socket(PF_INET, SOCK_DGRAM, 0);
}

/** Bind a socket file descriptor to a port number for E1.31 communication */
int e131_bind(int sockfd, const uint16_t port) {
  e131_addr_t addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);
  memset(addr.sin_zero, 0, sizeof(addr.sin_zero));
  return bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
}

/** Initialize a unicast E1.31 destination using a host and port number */
int e131_unicast_dest(const char *host, const uint16_t port, e131_addr_t *dest) {
  if (host == NULL || dest == NULL) {
    errno = EINVAL;
    return -1;
  }
  struct hostent *he = gethostbyname(host);
  if (he == NULL) {
    errno = EADDRNOTAVAIL;
    return -1;
  }
  dest->sin_family = AF_INET;
  dest->sin_addr = *(struct in_addr *)he->h_addr;
  dest->sin_port = htons(port);
  memset(dest->sin_zero, 0, sizeof(dest->sin_zero));
  return 0;
}

/** Initialize a multicast E1.31 destination using a universe and port number */
int e131_multicast_dest(const uint16_t universe, const uint16_t port, e131_addr_t *dest) {
  if (universe < 1 || universe > 63999 || dest == NULL) {
    errno = EINVAL;
    return -1;
  }
  dest->sin_family = AF_INET;
  dest->sin_addr.s_addr = htonl(0xefff0000 | universe);
  dest->sin_port = htons(port);
  memset(dest->sin_zero, 0, sizeof(dest->sin_zero));
  return 0;
}

/** Join a socket file descriptor to a E1.31 multicast group using a universe */
int e131_multicast_join(int sockfd, const uint16_t universe) {
  if (universe < 1 || universe > 63999) {
    errno = EINVAL;
    return -1;
  }
  struct ip_mreqn mreq;
  mreq.imr_multiaddr.s_addr = htonl(0xefff0000 | universe);
  mreq.imr_address.s_addr = htonl(INADDR_ANY);
  mreq.imr_ifindex = 0;
  return setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
}

/** Initialize a new E1.31 packet to default values */
int e131_pkt_init(const uint16_t universe, const uint16_t num_channels, e131_packet_t *packet) {
  if (packet == NULL || universe < 1 || universe > 63999 || num_channels < 1 || num_channels > 512) {
    errno = EINVAL;
    return -1;
  }

  // clear packet
  memset(packet, 0, sizeof(e131_packet_t));

  // set Root Layer values
  packet->root.preamble_size = htons(0x0010);
  memcpy(packet->root.acn_id, E131_ACN_ID, sizeof(packet->root.acn_id));
  packet->root.flength = htons(0x7000 | (num_channels + 1 + 10 + 77 + 22));
  packet->root.vector = htonl(E131_ROOT_VECTOR);

  // set Frame Layer values
  packet->frame.flength = htons(0x7000 | (num_channels + 1 + 10 + 77));
  packet->frame.vector = htonl(E131_FRAME_VECTOR);
  packet->frame.priority = 0x64;
  packet->frame.universe = htons(universe);

  // set DMP layer values
  packet->dmp.flength = htons(0x7000 | (num_channels + 1 + 10));
  packet->dmp.vector = E131_DMP_VECTOR;
  packet->dmp.type = 0xA1;
  packet->dmp.address_increment = htons(0x0001);
  packet->dmp.property_value_count = htons(num_channels + 1);
}

/** Send an E1.31 packet to a socket file descriptor using a destination */
ssize_t e131_send(int sockfd, const e131_packet_t *packet, const e131_addr_t *dest) {
  if (packet == NULL || dest == NULL) {
    errno = EINVAL;
    return -1;
  }
  const size_t packet_length = sizeof(packet->raw) -
    sizeof(packet->dmp.property_values) + htons(packet->dmp.property_value_count);
  return sendto(sockfd, packet->raw, packet_length, 0,
    (const struct sockaddr *)dest, sizeof(e131_addr_t));
}

/** Receive an E1.31 packet from a socket file descriptor */
ssize_t e131_recv(int sockfd, e131_packet_t *packet) {
  if (packet == NULL) {
    errno = EINVAL;
    return -1;
  }
  return recv(sockfd, packet->raw, sizeof(packet->raw), 0);
}

/** Validate correctness of an E1.31 packet */
e131_error_t e131_pkt_validate(const e131_packet_t *packet) {
  if (packet == NULL)
    return E131_ERR_NULLPTR;
  if (memcmp(packet->root.acn_id, E131_ACN_ID, sizeof(packet->root.acn_id)) != 0)
    return E131_ERR_ACN_ID;
  if (ntohl(packet->root.vector) != E131_ROOT_VECTOR)
    return E131_ERR_VECTOR_ROOT;
  if (ntohl(packet->frame.vector) != E131_FRAME_VECTOR)
    return E131_ERR_VECTOR_FRAME;
  if (packet->dmp.vector != E131_DMP_VECTOR)
    return E131_ERR_VECTOR_DMP;
  return E131_ERR_NONE;
}

/** Dump an E1.31 packet to the stderr output */
int e131_pkt_dump(const e131_packet_t *packet) {
  if (packet == NULL) {
    errno = EINVAL;
    return -1;
  }
  fprintf(stderr, "[Root Layer]\n");
  fprintf(stderr, "  preamble_size        : %" PRIu16 "\n", ntohs(packet->root.preamble_size));
  fprintf(stderr, "  postamble_size       : %" PRIu16 "\n", ntohs(packet->root.postamble_size));
  fprintf(stderr, "  acn_id               : %s\n", packet->root.acn_id);
  fprintf(stderr, "  flength              : %" PRIu16 "\n", ntohs(packet->root.flength));
  fprintf(stderr, "  vector               : %" PRIu32 "\n", ntohl(packet->root.vector));
  fprintf(stderr, "  cid                  : ");
  for (size_t pos=0, total=sizeof(packet->root.cid); pos<total; pos++)
    fprintf(stderr, "%02x", packet->root.cid[pos]);
  fprintf(stderr, "\n");
  fprintf(stderr, "[Frame Layer]\n");
  fprintf(stderr, "  flength              : %" PRIu16 "\n", ntohs(packet->frame.flength));
  fprintf(stderr, "  vector               : %" PRIu32 "\n", ntohl(packet->frame.vector));
  fprintf(stderr, "  source_name          : %s\n", packet->frame.source_name);
  fprintf(stderr, "  priority             : %" PRIu8 "\n", packet->frame.priority);
  fprintf(stderr, "  reserved             : %" PRIu16 "\n", ntohs(packet->frame.reserved));
  fprintf(stderr, "  sequence_number      : %" PRIu8 "\n", packet->frame.sequence_number);
  fprintf(stderr, "  options              : %" PRIu8 "\n", packet->frame.options);
  fprintf(stderr, "  universe             : %" PRIu16 "\n", ntohs(packet->frame.universe));
  fprintf(stderr, "[DMP Layer]\n");
  fprintf(stderr, "  flength              : %" PRIu16 "\n", ntohs(packet->dmp.flength));
  fprintf(stderr, "  vector               : %" PRIu8 "\n", packet->dmp.vector);
  fprintf(stderr, "  type                 : %" PRIu8 "\n", packet->dmp.type);
  fprintf(stderr, "  first_address        : %" PRIu16 "\n", ntohs(packet->dmp.first_address));
  fprintf(stderr, "  address_increment    : %" PRIu16 "\n", ntohs(packet->dmp.address_increment));
  fprintf(stderr, "  property_value_count : %" PRIu16 "\n", ntohs(packet->dmp.property_value_count));
  fprintf(stderr, "  property_values      :");
  for (size_t pos=0, total=ntohs(packet->dmp.property_value_count); pos<total; pos++)
    fprintf(stderr, " %02x", packet->dmp.property_values[pos]);
  fprintf(stderr, "\n");
  return 0;
}
