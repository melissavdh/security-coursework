#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

#include "cli_socket.h"

#define DEFAULT_PORT 5432
#define MAX_LINE 256

int socket_readable_timeout(int socket, int sec);

/*********************************************************/
/* socket layer */
/*********************************************************/
// Connect to waiting server
// Returns socket identifier
int socket_client_connect(char *host, int port)
{
    if(port == 0)
      port = DEFAULT_PORT;
    //FILE *fp;
    struct hostent *hp;
    struct sockaddr_in sin;
    //char *host;
    //char buf[MAX_LINE];
    int s;
    //int len;

    //...Translate host name into peer's IP address
    hp = gethostbyname(host);
    if (!hp) {
        fprintf(stderr, "simplex-talk: unknown host: %s\n", host);
    exit(1);
    }

    //...Build address data structure
    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
    sin.sin_port = htons(DEFAULT_PORT);

    //...Active open
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) <0) {
        perror("simplex-talk: socket");
        exit(1);
    }
    if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) <0) {
        perror("simplex-talk: connect");
        close(s);
        exit(1);
    }
      
    //TODO: Replace these lines with code for connecting to the server:
    return s;
}

// Receive a packet from the socket
// Returns number of bytes read
int socket_receive_packet(int sock, PACKET *pkt, int reliable)
{
    int s = recv(sock, pkt, PACKET_SIZE, MSG_WAITALL);
    //TODO: Replace this line with code to receive a packet from the socket.
    //Hint: use MSG_WAITALL as a parameter, to make sure your packets always get through whole.
    return s;
}

// Send a packet to the socket
// Returns number of bytes written.
int socket_send_packet(int sock, PACKET *pkt, int reliable)
{
    int s = send(sock, pkt, PACKET_SIZE, 0);
    //TODO: Replace this line with code to send a packet to the socket:
    return s;
}

// This function may be useful for implementing timeouts in Stop & Wait.
// Returns > 0 if socket becomes readable before the timeout (in ms) has expired.
// Otherwise it returns 0.
int socket_readable_timeout(int socket, int ms)
{
    //set of sockets we want to watch
    fd_set rset;

    //the timeout structure
    struct timeval tv;

    //cl(%d)ear the rset and add our socket to it
    FD_ZERO(&rset);
    FD_SET(socket, &rset);

    //set the timeout
    tv.tv_sec = ms/1000;
    tv.tv_usec = (ms%1000)*1000;

    return(select(socket+1, &rset, NULL, NULL, &tv));
}
