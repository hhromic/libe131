#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <e131.h>

int main() {
  int sockfd;
  e131_packet_t packet;
  e131_error_t error;

  // create a socket for E1.31
  if ((sockfd = e131_socket())<0)
    err(EXIT_FAILURE, "e131_socket");

  // bind the socket to the default E1.31 port
  if (e131_bind(sockfd, E131_DEFAULT_PORT)<0)
    err(EXIT_FAILURE, "e131_bind");

  // loop to receive E1.31 packets
  fprintf(stderr, "waiting for E1.31 packets ...\n");
  for (;;) {
    if (e131_recv(sockfd, &packet)<0)
      err(EXIT_FAILURE, "e131_recv");
    if ((error = e131_pkt_validate(&packet)) != E131_ERR_NONE) {
      fprintf(stderr, "%s\n", e131_strerror(error));
      continue;
    }
    e131_pkt_dump(&packet);
  }
}
