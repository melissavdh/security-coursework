#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>


#include "cli_security.h"

#define MAX_COMMAND 256

/*********************************************************/
/* function prototypes */
/*********************************************************/
//application layer
void app_run(int socket, int verbose, int reliable, int secure, int once);

/*********************************************************/
/* main program loop */
/*********************************************************/
int main(int argc, char* argv[])
{
    char *host;
    int sock;
    int arg;
    int verbose = 0;
    int reliable = 0;
    int secure = 0;
    int once = 0;
    int server_port = 0;

    if(argc >= 2)
    {
        host = argv[1];
    }
    else
    {
        //check for hostname
        fprintf(stderr, "usage: client <hostname> [-v] [-r] [-s]\n");
        exit(1);
    }

    //parse other switches
    for(arg=2; arg<argc; arg++)
    {
        if(!strcmp(argv[arg], "-v"))
        {
            verbose = 1;
            printf("Verbose mode: on\n");
        }
        else if(!strcmp(argv[arg], "-r"))
        {
            printf("Reliable transmission: on\n");
            reliable = 1;
        }
        else if(!strcmp(argv[arg], "-s"))
        {
            printf("Secure transmission: on\n");
            secure = 1;
        }
        else if(!strcmp(argv[arg], "-o"))
        {
            printf("Running only once.\n");
            once = 1;
        }
        else if(!strcmp(argv[arg], "-p"))
        {
	       if(argc > arg+1)
	       {
                server_port = atoi(argv[arg+1]);
                if(server_port > 0)
                {
                    printf("Using port: %d\n", server_port);
                    arg++;
                }
                else
                {
                    fprintf(stderr, "-p flag must be followed by port number\n");
                    fprintf(stderr, "usage: server [-v] [-r] [-s] [-p <port>]\n");
                    exit(1);
                }
            }
            else
            {
                fprintf(stderr, "-p flag must be followed by port number\n");
                fprintf(stderr, "usage: server [-v] [-r] [-s] [-p <port>]\n");
                exit(1);
            }
        }
        else
        {
            fprintf(stderr, "invalid flag: %s\n", argv[arg]);
            fprintf(stderr, "usage: client <hostname> [-v] [-r] [-s]\n");
            exit(1);
        }
    }

    //connect to server and run application
    sock = socket_client_connect(host, server_port);
    sec_init(sock, verbose, reliable, secure);
    
    if(sock > 0) app_run(sock, verbose, reliable, secure, once);
    return 0;
}

/*********************************************************/
/* application layer */
/*********************************************************/
void app_run(int socket, int verbose, int reliable, int secure, int once)
{
    char command[MAX_COMMAND];
    char *reply = NULL;
    int recv_len;
    time_t t0, t1;
    clock_t c0, c1;

    bzero((char *)command, MAX_COMMAND);

    // get command
    while(fgets(command, sizeof(command), stdin))
    {
        //send command
        command[strlen(command)-1] = 0;
        t0 = time(NULL);
        c0 = clock();
        sec_send_data(socket, command, strlen(command), verbose, reliable, secure);

        //receive reply
        if(sec_receive_data(socket, &reply, &recv_len, verbose, reliable, secure))
        {
            FILE *fp = fopen("receive", "wb");
            if(fp)
            {
                fwrite(reply, sizeof(char), recv_len, fp);
                fclose(fp);
            }
            else
            {
                printf("Error opening file\n");
            }
        }
        t1 = time(NULL);
        c1 = clock();
        printf("\telapsed wall clock time: %ld\n", (long) (t1 - t0));
        printf ("\telapsed CPU time:        %f\n", (float) (c1 - c0)/CLOCKS_PER_SEC);
        if(reply != NULL) free(reply);

        //clear the command buffer for next time
        bzero((char *)command, MAX_DATA);
        
        if(once) break;
    }
}
