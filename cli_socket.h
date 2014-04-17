#include "header.h"

//socket layer
int socket_client_connect(char *host, int port);
int socket_receive_packet(int socket, PACKET *packet, int reliable);
int socket_send_packet(int socket, PACKET *packet, int reliable);
int socket_readable_timeout(int socket, int ms);
