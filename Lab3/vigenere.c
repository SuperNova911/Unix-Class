#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#define MAX_SIZE 4096

enum Mode { None, Encryption, Decryption };

void Encrypt(const char * key, const char * padding, const char * input, char * cipher);
void Decrypt(const char * key, const char * padding, const char * input, char * plain);

int main(int argc, char *argv[])
{
    extern char *optarg;
    extern int optind, opterr, optopt;

    int option;

    enum Mode cryptMode = None;
    char * inputFileName = "";
    char * outputFileName = "";
    char * key = "";

    while ((option = getopt(argc, argv, "f:cdk:o:")) != -1)
    {
        switch (option)
        {
            // file name
        case 'f':
            inputFileName = optarg;
            break;

            // encryption
        case 'c':
            cryptMode = Encryption;
            break;

            // decryption
        case 'd':
            cryptMode = Decryption;
            break;

            // key
        case 'k':
            key = optarg;
            break;

            // output
        case 'o':
            outputFileName = optarg;
            break;

            // invalid option
        default:
            return -1;
        }
    }

    // Open input file
    FILE *fileStream;
    fileStream = fopen(inputFileName, "r");
    if (fileStream == NULL)
    {
        printf("vigenere: failed to open file, FileName: %s\n", inputFileName);
        return -1;
    }

    // Read file
    char input[MAX_SIZE] = "";
    char buffer[MAX_SIZE] = "";
    while (fgets(buffer, sizeof(buffer), fileStream) != NULL)
    {
        strcat(input, buffer);
    }
    fclose(fileStream);

    char plain[MAX_SIZE] = "";
    char cipher[MAX_SIZE] = "";
    char padding[2];
    padding[0] = 127;
    padding[1] = 0;

    // Create output file
    fileStream = fopen(outputFileName, "w");
    if (fileStream == NULL)
    {
        printf("vigenere: failed to create file, FileName: %s\n", outputFileName);
        return -1;
    }

    switch (cryptMode)
    {
    // Invaild mode
    case None:
        return -1;

    // Encrypt
    case Encryption:
        strcat(plain, input);
        Encrypt(key, padding, input, cipher);
        fputs(cipher, fileStream);
        break;

    // Decrypt
    case Decryption:
        strcat(cipher, input);
        Decrypt(key, padding, input, plain);
        fputs(plain, fileStream);
        break;
    }
    fclose(fileStream);

    return 0;
}

void Encrypt(const char * key, const char * padding, const char * input, char * cipher)
{
    char * plain = (char *)input;

    int keyLength = strlen(key);
    int plainLength = strlen(plain);
    int paddingSize;

    // Calculate padding size
    if (plainLength < keyLength)
        paddingSize = keyLength - plainLength;
    else
    {
        if ((plainLength % keyLength) == 0)
            paddingSize = 0;
        else
            paddingSize = keyLength - (plainLength % keyLength);
    }
    printf("paddingSize: %d\n", paddingSize);

    // Add padding
    for (int i = 0; i < paddingSize; i++)
    {
        strcat(plain, padding);
    }
    plainLength = strlen(plain);
    printf("%s\n", plain);

    // Generate key
    char fullKey[MAX_SIZE] = "";
    while (strlen(fullKey) < plainLength)
    {
        strcat(fullKey, key);
    }

    // Vigenere Encryption
    int asciiStartIndex = 0;
    int asciiEndIndex = 127;
    int asciiPoolLength = asciiEndIndex - asciiStartIndex + 1;
    int encrypted = 0;
    for (int index = 0; index < plainLength; index++)
    {
        encrypted = plain[index];
        encrypted -= asciiStartIndex;
        encrypted += fullKey[index];

        while (encrypted < asciiPoolLength)
        {
            encrypted += asciiPoolLength;
        }
        encrypted %= (asciiEndIndex - asciiStartIndex + 1);
        encrypted += asciiStartIndex;

        cipher[index] = encrypted;
        encrypted = 0;
    }
}


void Decrypt(const char * key, const char * padding, const char * input, char * plain)
{
    char * cipher = (char *)input;

    int cipherLength = strlen(cipher);

    // Generate Key
    char fullKey[MAX_SIZE] = "";
    while (strlen(fullKey) < cipherLength)
    {
        strcat(fullKey, key);
    }

    // Vigenere Decryption
    int asciiStartIndex = 0;
    int asciiEndIndex = 127;
    int asciiPoolLength = asciiEndIndex - asciiStartIndex + 1;
    int decrypted = 0;
    for (int index = 0; index < cipherLength; index++)
    {
        decrypted = cipher[index];
        decrypted -= asciiStartIndex;
        decrypted -= fullKey[index];

        while (decrypted < asciiPoolLength)
        {
            decrypted += asciiPoolLength;
        }
        decrypted %= asciiPoolLength;
        decrypted += asciiStartIndex;

        plain[index] = decrypted;
        decrypted = 0;
    }

    // Remove padding
    int plainLength = strlen(plain);
    int paddingIndex;
    for (paddingIndex = 0; paddingIndex < plainLength; paddingIndex++)
    {
        // Padding detection
        if (plain[paddingIndex] == padding[0])
            break;
    }
    printf("'%s'\n", plain);
    plain[paddingIndex] = 0;

    printf("'%s'\n", plain);
}
