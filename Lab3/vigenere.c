/*
 * Unix Lab3
 * Author: Suwhan Kim
 * Student ID : 201510743
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_SIZE 8192

enum Mode { None, Encryption, Decryption };

void Encrypt(const char *key, const char *input, char *cipher);
void Decrypt(const char *key, const char *input, char *plain);

int main(int argc, char *argv[])
{
    extern char *optarg;
    extern int optind, opterr, optopt;

    int option;

    enum Mode cryptMode = None;
    char *inputFileName = "";
    char *outputFileName = "";
    char *key = "";

    while ((option = getopt(argc, argv, "f:o:k:cd")) != -1)
    {
        switch (option)
        {
            // Input file name
            case 'f':
                inputFileName = optarg;
                break;

            // Output file name
            case 'o':
                outputFileName = optarg;
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

    // Create output file
    fileStream = fopen(outputFileName, "w");
    if (fileStream == NULL)
    {
        printf("vigenere: failed to create file, FileName: %s\n", outputFileName);
        return -1;
    }

    // Encryption/Decryption and export to output file
    char plain[MAX_SIZE] = "";
    char cipher[MAX_SIZE] = "";
    switch (cryptMode)
    {
        // Invaild mode
        case None:
            return -1;

        // Encrypt
        case Encryption:
            strcat(plain, input);
            Encrypt(key, input, cipher);
            fputs(cipher, fileStream);
            break;

        // Decrypt
        case Decryption:
            strcat(cipher, input);
            Decrypt(key, input, plain);
            fputs(plain, fileStream);
            break;
    }
    fclose(fileStream);

    return 0;
}

void Encrypt(const char *key, const char *input, char *cipher)
{
    char plain[MAX_SIZE] = "";
    strcpy(plain, input);

    // Generate key
    int keyLength = strlen(key);
    int plainLength = strlen(plain);
    char fullKey[MAX_SIZE] = "";
    while (strlen(fullKey) < plainLength)
    {
        strcat(fullKey, key);
    }

    // Vigenere Encryption
    int asciiStartIndex = 1;
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

void Decrypt(const char *key, const char *input, char *plain)
{
    char cipher[MAX_SIZE];
    strcpy(cipher, input);

    // Generate Key
    int cipherLength = strlen(cipher);
    char fullKey[MAX_SIZE] = "";
    while (strlen(fullKey) < cipherLength)
    {
        strcat(fullKey, key);
    }

    // Vigenere Decryption
    int asciiStartIndex = 1;
    int asciiEndIndex = 127;
    int asciiPoolLength = asciiEndIndex - asciiStartIndex + 1;
    int decrypted = 0;
    int plainIndex = 0;
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

        plain[plainIndex] = decrypted;
        plainIndex++;
        decrypted = 0;
    }
}
