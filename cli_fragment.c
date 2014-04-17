#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "cli_fragment.h"

/*********************************************************/
/* packetizing/fragmentationlayer */
/*********************************************************/
unsigned int g_last_frag = 0;

int get_last_frag() {return g_last_frag;}
void increment_last_frag() {g_last_frag++;}
void reset_last_frag() {g_last_frag=0;};

void frag_init() {reset_last_frag();}

//linked list entry for queuing up received packets before reassembly
typedef struct
{
    char data[MAX_DATA];
    int length;
    void *next;

} FRAGMENT;

// keeps asking the socket layer for packets until it receives one flagged as 'last'.
// Adds them to a linked list, and when it has received them all,
// reassembes them and returns the data in pBuf, with length pLen.
// The caller is responsible for freeing the data.
// returns 0 for failure or 1 for success.
int frag_receive_data(int socket, char** pBuf, int *pLen, int verbose, int reliable)
{
    PACKET pkt;
    int total_len = 0;
    FRAGMENT *pFirst = (FRAGMENT*)malloc(sizeof(FRAGMENT));
    FRAGMENT *pCurrent = pFirst;
    int count=0;
    int done = 0;

    // while we've got more packets still to come
    while(!done)
    {
        pCurrent->next = NULL;

        //get the next packet
        int len = rel_receive_packet(socket, &pkt, get_last_frag(), verbose, reliable);
        increment_last_frag();

        //if it's incomplete, die
        if (len != PACKET_SIZE)
        {
            //free fragment list
            pCurrent = pFirst;
            while(pCurrent != NULL)
            {
                FRAGMENT *pOld = pCurrent;
                pCurrent = pCurrent->next;
                free(pOld);
            }
            return 0;
        }

        if(verbose) printf("received frag: %d, last: %d, len: %d\n",
            pkt.header.fragment, pkt.header.last, pkt.header.datalen);

        // if something's gone wrong
        if(pkt.header.datalen < 0 || pkt.header.datalen > MAX_DATA)
        {
            printf("received frag: %d, last: %d, len: %d after %d bytes\n",
                pkt.header.fragment, pkt.header.last, pkt.header.datalen, total_len);
            printf("Invalid data length: %d. Fragment may be corrupted\n", pkt.header.datalen);
            done = 1;
        }
        // if this is the last one
        else if(pkt.header.last)
        {
            bzero((char*)pCurrent->data, MAX_DATA);
            memcpy(pCurrent->data, pkt.data, pkt.header.datalen);
            pCurrent->length = pkt.header.datalen;
            total_len += pkt.header.datalen;
            done = 1;
        }
        // if it's not the last one
        else
        {
            memcpy(pCurrent->data, pkt.data, MAX_DATA);
            pCurrent->length = MAX_DATA;
            total_len += MAX_DATA;

            FRAGMENT *pNext = (FRAGMENT*)malloc(sizeof(FRAGMENT));
            pCurrent->next = pNext;
            pCurrent = pNext;
        }
    }

    // allocate a buffer big enough for the whole reassembled message
    char* data = (char*)malloc(total_len+1);
    bzero(data, total_len+1);
    printf("received %d bytes\n", total_len);

    // copy data into it, and free the fragment list
    pCurrent = pFirst;
    while(pCurrent != NULL)
    {
        memcpy(data+count, pCurrent->data, pCurrent->length);
        count += pCurrent->length;
        FRAGMENT *pOld = pCurrent;
        pCurrent = pCurrent->next;
        free(pOld);
    }

    *pBuf = data;
    *pLen = total_len;
    return 1;
}

// takes a message and wraps it with the packet header.  Fragments it if necessary.
// returns 0 for failure or 1 for success.
int frag_send_data(int socket, char *data, int len, int verbose, int reliable)
{
    //set up packet header
    PACKET pkt;
    pkt.header.is_data = 1;
    pkt.header.fragment = get_last_frag();
    pkt.header.last = 0;
    pkt.header.datalen = MAX_DATA;

    //for all packets except the last..
    while(len > MAX_DATA)
    {
        memcpy(pkt.data, data, MAX_DATA);
        if(!rel_send_packet(socket, &pkt, verbose, reliable))
            return 0;

        increment_last_frag();
        pkt.header.fragment = get_last_frag();

        data += MAX_DATA;
        len -= MAX_DATA;

        if(verbose) printf("sent frag: %d, last: %d, len: %d\n",
            pkt.header.fragment, pkt.header.last, pkt.header.datalen);
    }

    //for the last packet..
    pkt.header.last = 1;
    pkt.header.datalen = len;
    bzero((char *)pkt.data, MAX_DATA);
    memcpy(pkt.data, data, len);
    rel_send_packet(socket, &pkt, verbose, reliable);

    increment_last_frag();

    if(verbose) printf("sent frag: %d, last: %d, len: %d\n",
        pkt.header.fragment, pkt.header.last, pkt.header.datalen);

    return 1;
}
