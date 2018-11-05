/*
 * Unix Lab4
 * Author: Suwhan Kim
 * Student ID: 201510743
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define ASCII_START_INDEX 1
#define ASCII_END_INDEX 126

enum Mode { None, Encryption, Decryption };

void Encrypt(const char *key, caddr_t address, int startIndex, int endIndex);
void Decrypt(const char *key, caddr_t address, int startIndex, int endIndex);

int main(int argc, char *argv[])
{
    extern char *optarg;
    extern int optind, opterr, optopt;

    int option;

    enum Mode cryptMode = None;
    char *inputFileName;
    int startIndex = 0;
    int endIndex = 0;
    char *key = "";

    while ((option = getopt(argc, argv, "f:s:e:k:cd")) != -1)
    {
        switch (option)
        {
            // Input file
            case 'f':
                inputFileName = optarg;
                break;

            // Start index
            case 's':
                startIndex = atoi(optarg);
                break;
                
            // End index
            case 'e':
                endIndex = atoi(optarg);
                break;

            // Vigenere key
            case 'k':
                key = optarg;
                break;

            // Encryption mode
            case 'c':
                cryptMode = Encryption;
                break;

            // Decryption mode
            case 'd':
                cryptMode = Decryption;
                break;

            // Invalid option
            default:
		printf("Invalid option\n");
		printf("%c, %s\n", option, optarg);
                return -1;
        }
    }

    // Get stat info
    struct stat statBuffer;
    if (stat(inputFileName, &statBuffer) == -1)
    {
        printf("vigenere: failed to get stat information, FileName: %s\n", inputFileName);
        return -1;
    }

    int fileDescriptor;
    if ((fileDescriptor = open(inputFileName, O_RDWR)) == -1)
    {
        printf("vigenere: failed to open input file, FileName: %s\n", inputFileName);
        return -1;
    }

    caddr_t address;
    address = mmap(NULL, statBuffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, (off_t)0);
    if (address == MAP_FAILED)
    {
        printf("vigenere: memory mapping failed\n");
        return -1;
    }
    close (fileDescriptor);

    // Encryption/Decryption
    switch (cryptMode)
    {
        // Invaild mode
        case None:
	    printf("vigenere: invalid mode\n");
            return -1;

        // Encrypt
        case Encryption:
            Encrypt(key, address, startIndex, endIndex);
            break;

        // Decrypt
        case Decryption:
            Decrypt(key, address, startIndex, endIndex);
            break;
    }

    return 0;
}

void Encrypt(const char *key, caddr_t address, int startIndex, int endIndex)
{
    // Generate key
    int inputLength = endIndex + 1;
    int keyLength = strlen(key);
    char *fullKey = (char *)malloc(inputLength + keyLength + 1);
    while (strlen(fullKey) < inputLength)
    {
        strcat(fullKey, key);
    }

    // Vigenere Encryption
    int asciiPoolLength = ASCII_END_INDEX - ASCII_START_INDEX + 1;
    int encryptedChar = 0;
    for (int index = startIndex; index <= endIndex; index++)
    {
        encryptedChar = address[index];
        encryptedChar -= ASCII_START_INDEX;
        encryptedChar += fullKey[index];

        while (encryptedChar < asciiPoolLength)
        {
            encryptedChar += asciiPoolLength;
        }
        encryptedChar %= (ASCII_END_INDEX - ASCII_START_INDEX + 1);
        encryptedChar += ASCII_START_INDEX;

        address[index] = encryptedChar;
        encryptedChar = 0;
    }

    free(fullKey);
}

void Decrypt(const char *key, caddr_t address, int startIndex, int endIndex)
{
    // Generate Key
    int inputLength = endIndex + 1;
    int keyLength = strlen(key);
    char *fullKey = (char *)malloc(inputLength + keyLength + 1);
    while (strlen(fullKey) < inputLength)
    {
        strcat(fullKey, key);
    }

    // Vigenere Decryption
    int asciiPoolLength = ASCII_END_INDEX - ASCII_START_INDEX + 1;
    int decryptedChar = 0;
    for (int index = startIndex; index <= endIndex; index++)
    {
        decryptedChar = address[index];
        decryptedChar -= ASCII_START_INDEX;
        decryptedChar -= fullKey[index];

        while (decryptedChar < asciiPoolLength)
        {
            decryptedChar += asciiPoolLength;
        }
        decryptedChar %= asciiPoolLength;
        decryptedChar += ASCII_START_INDEX;

        address[index] = decryptedChar;
        decryptedChar = 0;
    }

    free(fullKey);
}
