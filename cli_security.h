#include "cli_fragment.h"

//security layer
int sec_init(int socket, int verbose, int reliable, int secure);
int sec_receive_data(int socket, char** pBuf, int *pLen, int verbose, int reliable, int secure);
int sec_send_data(int socket, char *data, int len, int verbose, int reliable, int secure);
