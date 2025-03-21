#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <e131.h>

#include "error.h"
#include "sleep.h"

int main() {
  int sockfd = 0;
  e131_packet_t packet;
  e131_addr_t dest;
  char buf[100];

  // create a socket for E1.31
  if ((sockfd = e131_socket()) < 0)
    err(EXIT_FAILURE, "e131_socket");

  // configure socket to use the default network interface for outgoing multicast data
  if (e131_multicast_iface(sockfd, 0) < 0)
    err(EXIT_FAILURE, "e131_multicast_iface");

  // join to multicast group for universe 1
  if (e131_multicast_join(sockfd, 1) <0)
    err(EXIT_FAILURE, "e131_multicast_join");
        
  // initialize the new E1.31 packet in universe 1 with 24 slots in preview mode
  e131_pkt_init(&packet, 1, 24);
  memcpy(&packet.frame.source_name, "E1.31 Test Client", 18);

  // set remote system destination as multicast address
  if (e131_multicast_dest(&dest, 1, E131_DEFAULT_PORT) < 0)
    err(EXIT_FAILURE, "e131_unicast_dest 1");

  if( e131_dest_str(buf, &dest) < 0)
    err(EXIT_FAILURE, "e131_unicast_dest 2");

  fprintf(stderr, "Sending to: %s\n", buf);

  // loop to send cycling levels for each slot
  uint8_t level = 0;
  for (;;) {
    for (size_t pos=0; pos<24; pos++)
      packet.dmp.prop_val[pos + 1] = level;

    level++;
    if (e131_send(sockfd, &packet, &dest) < 0)
      err(EXIT_FAILURE, "e131_send");

    e131_pkt_dump(stderr, &packet);
    packet.frame.seq_number++;
    usleep(250000);
  }

  e131_socket_close(sockfd);
}
