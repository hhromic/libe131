#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <e131.h>

#include "error.h"

int main() {
  int sockfd;
  e131_packet_t packet;
  e131_error_t error;
  uint8_t last_seq = 0x00;

  // create a socket for E1.31
  if ((sockfd = e131_socket()) < 0)
    err(EXIT_FAILURE, "e131_socket");

  // bind the socket to the default E1.31 port
  if (e131_bind(sockfd, E131_DEFAULT_PORT) < 0)
    err(EXIT_FAILURE, "e131_bind");

  // join the socket to multicast group for universe 1 on the default network interface
  if (e131_multicast_join_iface(sockfd, 1, 0) < 0)
    err(EXIT_FAILURE, "e131_multicast_join_iface");

  // loop to receive E1.31 packets
  fprintf(stderr, "waiting for E1.31 packets ...\n");
  for (;;) {
    if (e131_recv(sockfd, &packet) < 0)
      err(EXIT_FAILURE, "e131_recv");
    if ((error = e131_pkt_validate(&packet)) != E131_ERR_NONE) {
      fprintf(stderr, "e131_pkt_validate: %s\n", e131_strerror(error));
      continue;
    }
    if (e131_pkt_discard(&packet, last_seq)) {
      fprintf(stderr, "warning: packet out of order received\n");
      last_seq = packet.frame.seq_number;
      continue;
    }
    e131_pkt_dump(stderr, &packet);
    last_seq = packet.frame.seq_number;
  }

  e131_socket_close(sockfd);
}
