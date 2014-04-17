#include "cli_reliable.h"

//packetizing/fragmentation layer
void frag_init();
int frag_receive_data(int socket, char** pBuf, int *pLen, int verbose, int reliable);
int frag_send_data(int socket, char *data, int len, int verbose, int reliable);
