#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include <sys/types.h>
#include <sys/wait.h>

#define MAX_SIZE 8192
#define MAX_PROCESS_NUMBER 32

enum Mode { None, Encryption, Decryption };

int main(int argc, char *argv[])
{
    extern char *optarg;
    extern int optind, opterr, optopt;

    int option;

    enum Mode cryptMode = None;
    char *inputFileName = "";
    char *outputFileName = "";
    char *key = "";
    int numberOfProcess = 1;

    while ((option = getopt(argc, argv, "cdf:k:o:p:")) != -1)
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

            // Number of process
            case 'p':
                numberOfProcess = atoi(optarg);
                break;

            // Encryption mode
            case 'c':
                cryptMode = Encryption;
                break;

            // Decryption mode
            case 'd':
                cryptMode = Decryption;
                break;

            // Invaild option
            default:
                return 0;
        }
    }

    // Check minimum number of process
    if (numberOfProcess < 1 || MAX_PROCESS_NUMBER < numberOfProcess)
    {
        printf("multicipher: allowed number of process is 1 ~ %d\n", MAX_PROCESS_NUMBER);
        return 0;
    }

    // Open input file
    FILE *fileStream;
    fileStream = fopen(inputFileName, "r");
    if (fileStream == NULL)
    {
        printf("multicipher: failed to open input file, FileName: %s\n", inputFileName);
        return 0;
    }

    // Read input file
    char input[MAX_SIZE] = "";
    char buffer[MAX_SIZE] = "";
    while (fgets(buffer, sizeof(buffer), fileStream) != NULL)
    {
        strcat(input, buffer);
    }
    fclose(fileStream);

    // Split input file
    int inputLength = strlen(input);
    int keyLength = strlen(key);
    int pivot1 = 0, pivot2 = 0;
    char splited[MAX_SIZE] = "";
    char tempFileName[MAX_PROCESS_NUMBER][256];

    for (int index = 0; index < numberOfProcess; index++)
    {
        // Split string
        pivot2 = (int)round((inputLength / (float)numberOfProcess * (float)(index + 1)));
        while ((pivot2 - pivot1) % keyLength != 0 && pivot2 < inputLength - 1)
        {
            pivot2++;
        }
        memcpy(splited, &input[pivot1], pivot2 - pivot1);
        splited[pivot2 - pivot1] = '\0';
        pivot1 = pivot2;

        // Write to splited file
        sprintf(tempFileName[index], "%s%02d", outputFileName, index);
        fileStream = fopen(tempFileName[index], "w");
        if (fileStream == NULL)
        {
            printf("multicipher: failed to create temp file\n");
            return 0;
        }
        fputs(splited, fileStream);
        fclose(fileStream);
    }

    // Argument for vigenere program
    char *childArgv[9] = { "./vigenere", "vigenere", "-f", "input", "-o", "output", "-k", key, "mode" };
    switch (cryptMode)
    {
        case Encryption:
            childArgv[8] = "-c";
            break;

        case Decryption:
            childArgv[8] = "-d";
            break;

        default:
            printf("multicipher: invaild crypt mode\n");
            return 0;
    }

    // Multi processing
    pid_t pid[MAX_PROCESS_NUMBER];
    for (int index = 0; index < numberOfProcess; index++)
    {
        pid[index] = fork();

        // Assign task to child
        if (pid[index] == 0)
        {
            childArgv[3] = tempFileName[index];
            childArgv[5] = tempFileName[index];

            execv("./vigenere", childArgv);
        }
    }

    // Wait for task complete
    int status[MAX_PROCESS_NUMBER];
    for (int index = 0; index < numberOfProcess; index++)
    {
        waitpid(pid[index], &status[index], WCONTINUED);
        
        // Child process does not finished normally
        if (status[index] != 0)
        {
            printf("multicipher: something went wrong from vigenere, pid: %d\n", pid[index]);
            return 0;
        }
    }

    // Merge splited file
    char workResult[MAX_SIZE] = "";
    for (int index = 0; index < numberOfProcess; index++)
    {
        fileStream = fopen(tempFileName[index], "r");
        if (fileStream == NULL)
        {
            printf("multicipher: failed to open temp file, FileName: %s\n", tempFileName[index]);
            return 0;
        }

        while (fgets(buffer, sizeof(buffer), fileStream) != NULL)
        {
            strcat(workResult, buffer);
        }
        fclose(fileStream);
        remove(tempFileName[index]);
    }

    // Create output file
    fileStream = fopen(outputFileName, "w");
    if (fileStream == NULL)
    {
        printf("multicipher: failed to create output file, FileName: %s\n", outputFileName);
        return 0;
    }
    fputs(workResult, fileStream);
    fclose(fileStream);

    return 0;
}
