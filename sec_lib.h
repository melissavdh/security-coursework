#define SECRET_KEY_SIZE 20 //The size of the secret (symmetric) key)
#define ENCRYPTED_KEY_SIZE 128 //The size of the encrypted symmetric key
#define PUB_KEY_SIZE 128 //1024 bits: The size of the public and private keys

//generate a public and private key pair
void sec_key_gen(char public_key[PUB_KEY_SIZE], char private_key[PUB_KEY_SIZE]);

//use the public key to encrypt the 'in' buffer (a secret key).
//The ciphertext is returned in the 'out' buffer.
void sec_pk_encrypt(char in[SECRET_KEY_SIZE], char out[ENCRYPTED_KEY_SIZE],
    char pub_key[PUB_KEY_SIZE]);

//use the private key to decrypt the 'in' buffer.
//The recovered secret key is returned in the 'out' buffer.
void sec_pk_decrypt(char in[ENCRYPTED_KEY_SIZE], char out[SECRET_KEY_SIZE],
    char priv_key[PUB_KEY_SIZE]);

//use the symmetric key to encrypt the 'input' buffer using a block cipher.
//Memory is allocated for the ciphertext, which is returned in 'output'.
//The output length is also returned as it may be different from the input.
//The caller is responsible for freeing the memory for the output when done.
void sec_symmetric_encrypt(char *input, int input_len,
    char **output, int *output_len, char key[SECRET_KEY_SIZE]);

//use the symmetric key to decrypt the 'input' buffer using a block cipher.
//Memory is allocated for the plaintext, which is returned in 'output'.
//The output length is also returned as it may be different from the input.
//The caller is responsible for freeing the memory for the output when done.
void sec_symmetric_decrypt(char *input, int input_len,
    char **output, int *output_len, char key[SECRET_KEY_SIZE]);
