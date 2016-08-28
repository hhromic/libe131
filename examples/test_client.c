#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <err.h>
#include <e131.h>

int main() {
  int sockfd;
  e131_packet_t packet;
  e131_addr_t dest;

  // create a socket for E1.31
  if ((sockfd = e131_socket()) < 0)
    err(EXIT_FAILURE, "e131_socket");

  // initialize the new E1.31 packet in universe 1 with 24 slots in preview mode
  e131_pkt_init(&packet, 1, 24);
  memcpy(&packet.frame.source_name, "E1.31 Test Client", 18);
  if (e131_set_option(&packet, E131_OPT_PREVIEW, true) < 0)
    err(EXIT_FAILURE, "e131_set_option");

  // set remote system destination as unicast address
  if (e131_unicast_dest(&dest, "127.0.0.1", E131_DEFAULT_PORT) < 0)
    err(EXIT_FAILURE, "e131_unicast_dest");

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
}
