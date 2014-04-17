#include <stdio.h>
#include <stdlib.h>

#include "cli_reliable.h"

/*********************************************************/
/* Reliable Transmission Layer */
/*********************************************************/

// Waits to receive a packet from the socket layer.
// If reliable transmission is switched off, it returns this immediately.
// If reliable transmission is on, this function implements the receiving side
// of the stop and wait algorithm.
// It is passed the ID of the socket and the sequence number of the required fragment.
// Returns the (correct) packet in the 'packet' structure.
// Uses the result of the socket_receive_packet function as its return value.
// ie 0 for failure, nonzero for success.

int rel_receive_packet(int socket, PACKET *packet, int required_frag, int verbose, int reliable)
{
    //if reliable transmission is turned off, just get the next packet
    int result = socket_receive_packet(socket, packet, reliable);

    //if the connection died, return now
    if(!result)
        return result;

    //if reliable transmission is turned off, return the packet we've got
    if(!reliable)
        return result;
	
	//if reliable transmission is turned on, 
    else
    {
        while(1)
        {
            // creates acknowledgement of receipt of packet
            PACKET ack;
            ack.header.is_data = 0;
            ack.header.fragment = packet->header.fragment;
            ack.header.last = packet->header.last;
            ack.header.datalen = packet->header.datalen;

            //if sent same packet twice no ack received so send current ack and receive next packet
            if (packet->header.fragment == (required_frag)-1)
            { 
    		    socket_send_packet(socket, &ack, reliable);
    		    result = socket_receive_packet(socket, packet,  reliable);
            }
            //if receive expected packet, send ack
            else if (packet->header.fragment == required_frag)
            {
                return socket_send_packet(socket, &ack, reliable);
            }
            //receive packet again
            else
            {
                return socket_receive_packet(socket, packet, reliable);
            }
        }
    }
}

// Sends a packet to the socket layer.
// If reliable transmission is switched off, that's all we need to do.
// If it's on, this function implements the sending side of the stop & wait algorithm.
// It is passed ID of the socket and a pointer to the packet to send.
// Uses the result of the socket_send_packet function as its return value
// ie 0 for failure, nonzero for success.
int rel_send_packet(int socket, PACKET *packet, int verbose, int reliable)
{
    //if reliable transmission is turned off, just send the next packet
    int result = socket_send_packet(socket, packet, reliable);

    if(!reliable)
        return result;

    else
    {
    PACKET ack;
    int required_frag = packet->header.fragment;

    // if ack is received before timeout
    if (socket_readable_timeout(socket, 100))
    {
        // packet is received
        result = socket_receive_packet(socket, &ack, reliable);

        // is the received ack correct? If yes, do nothing
        if ((ack.header.fragment == required_frag) && (ack.header.is_data == 0))
        {
        }
        // if the ack is wrong, resend it
        else
        {
        result = socket_send_packet(socket, packet, reliable);
        printf("Wrong ack received, resending packet\n");
        }
	}
    else
    {
        // if ack not received before timeout, resend packet
        result = socket_send_packet(socket, packet, reliable);
    }
    return result;
    }
}
