#include "cli_socket.h"

int rel_receive_packet(int socket, PACKET *packet, int frag, int verbose, int reliable);
int rel_send_packet(int socket, PACKET *packet, int verbose, int reliable);
