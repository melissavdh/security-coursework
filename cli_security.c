#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include "cli_security.h"
#include "sec_lib.h"

//#define SECRET_KEY_SIZE 20 //The size of the secret (symmetric) key)
//#define ENCRYPTED_KEY_SIZE 128 //The size of the encrypted symmetric key
//#define PUB_KEY_SIZE 128 //1024 bits: The size of the public and private keys

// Hint: you will need this global variable to store the secret key
// once it has been generated
char secret_key[SECRET_KEY_SIZE];

/*********************************************************/
/* Security Layer */
/*********************************************************/
// initialisation procedure - called as soon as the client connects.
// This function is passed the ID of the socket, and performs the key exchange operation
// to establish a shared secret key.
// Returns 1 for success or 0 for failure

int sec_init(int socket, int verbose, int reliable, int secure)
{   
    frag_init();

    char *public_key[PUB_KEY_SIZE];
    char get_pubkey[] = "get public key";
    int length_pubkey;
    char encrypted_pubkey[ENCRYPTED_KEY_SIZE];
    int j=0;
    char *ok;
    int ok_length;
    
    if(!secure)
        return 1;
  
    if(secure)
    {
        //sends plain text string "get public key" to server
        frag_send_data(socket, get_pubkey, sizeof(get_pubkey), verbose, reliable);
        //server sends public key to client
        frag_receive_data(socket, public_key, &length_pubkey, verbose, reliable);

        //client generates random 160 bit string (secret key)
        for (j=0; j<SECRET_KEY_SIZE; j++)
        {
            secret_key[j] = ((char) (rand()%(160)));
        }

        //client encrypts secret key using public key
        sec_pk_encrypt(secret_key, encrypted_pubkey, *public_key);

        //send encrypted secret key back to server
        frag_send_data(socket, encrypted_pubkey, ENCRYPTED_KEY_SIZE, verbose, reliable);

        //client receives "ok" from server
        frag_receive_data(socket, &ok, &ok_length, verbose, reliable);
        return 1;
    }
    return 0;
}

// If security is turned on, decrypts the data received from the fragmentation layer.
// Takes the socket ID, a pointer to the received data buffer and a pointer to its length.
// returns 1 for success or 0 for failure
int sec_receive_data(int socket, char** pBuf, int *pLen, int verbose, int reliable, int secure)
{
    char *data[ENCRYPTED_KEY_SIZE];
    int data_length;
    int result;

    //if security is off, receive (unencrypted) data from fragmentation layer
    if(!secure)
    {
        return frag_receive_data(socket, pBuf, pLen, verbose, reliable);
    }
    //if security is on
    if (secure)
    {
        //receive data from fragmentation layer
        result = frag_receive_data(socket, data, &data_length, verbose, reliable);
        //server decrypts data using secret key
        sec_symmetric_decrypt(data[0], data_length, pBuf, pLen, secret_key);
        // *data pointer is freed of data for later use
        free(*data);
        return result;
     }
    return 0;
}

// If security is turned on, encrypts the data to be passed down to the fragmentation layer.
// Takes the socket ID, a pointer to the data buffer to be sent, and its length.
// returns 1 for success or 0 for failure
int sec_send_data(int socket, char *data, int len, int verbose, int reliable, int secure)
{
    //if security is off, send (unencrypted) data to fragmentation layer
    if(!secure)
    {
        return frag_send_data(socket, data, len, verbose, reliable);
    }

    char *data2[ENCRYPTED_KEY_SIZE];
    int data_length;

    //if security is on
    if(secure)
    {
        //encrypts the data using secret key
        sec_symmetric_encrypt(data, len, data2, &data_length, secret_key);
        //sends encrypted data to fragmentation layer
    	return frag_send_data(socket, data2[0], data_length, verbose, reliable);
    }
    return 0;
}
