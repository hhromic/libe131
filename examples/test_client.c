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
  if ((sockfd = e131_socket())<0)
    err(EXIT_FAILURE, "e131_socket");

  // initialize a new E1.31 packet in universe 1 with 20 channels
  e131_pkt_init(1, 20, &packet);
  memcpy(&packet.frame.source_name, "E1.31 Test Client", 18);

  // set remote system address as unicast
  if (e131_unicast_dest("127.0.0.1", E131_DEFAULT_PORT, &dest)<0)
    err(EXIT_FAILURE, "e131_unicast_dest");

  // loop to send cycling levels for each channel
  uint8_t level = 0;
  for (;;) {
    for (size_t pos=0; pos<20; pos++)
      packet.dmp.property_values[pos + 1] = level;
    level++;
    if (e131_send(sockfd, &packet, &dest)<0)
      err(EXIT_FAILURE, "e131_send");
    e131_pkt_dump(&packet);
    packet.frame.sequence_number++;
    sleep(1);
  }
}
