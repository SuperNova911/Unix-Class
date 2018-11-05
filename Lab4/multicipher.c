/*
 * Unix Lab4
 * Author: Suwhan Kim
 * Student ID: 201510743
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>

#define MAX_PROCESS_NUMBER 32

enum Mode { None, Encryption, Decryption };

int main(int argc, char *argv[])
{
    extern char *optarg;
    extern int optind, opterr, optopt;

    int option;

    enum Mode cryptMode = None;
    char *inputFileName = "";
    char *key = "";
    int numberOfProcess = 1;

    while ((option = getopt(argc, argv, "cdf:k:p:")) != -1)
    {
        switch (option)
        {
            // Input file name
            case 'f':
                inputFileName = optarg;
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

    // Get stat info
    struct stat statBuffer;
    if (stat(inputFileName, &statBuffer) == -1)
    {
        printf("multicipher: failed to get stat information, FileName: %s\n", inputFileName);
        return 0;
    }

    // Open input file
    int fileDescriptor;
    if ((fileDescriptor = open(inputFileName, O_RDWR)) == -1)
    {
        printf("multicipher: failed to open input file, FileName: %s\n", inputFileName);
        return 0;
    }

    // Read input file
    caddr_t address;
    address = mmap(NULL, statBuffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, (off_t)0);
    if (address == MAP_FAILED)
    {
        printf("multicipher: memory mapping failed\n");
        return 0;
    }
    close (fileDescriptor);

    // Argument for vigenere program
    char *childArgv[12] = { "vigenere", "vigenere", "-f", inputFileName, "-s", "startIndex", "-e", "endIndex", "-k", key, "mode", NULL };
    switch (cryptMode)
    {
        case Encryption:
            childArgv[10] = "-c";
            break;

        case Decryption:
            childArgv[10] = "-d";
            break;

        default:
            printf("multicipher: invaild crypt mode\n");
            return 0;
    }

    int inputLength = statBuffer.st_size;
    int keyLength = strlen(key);
    int pivot1 = 0, pivot2 = 0;
    char argStartIndex[sizeof(int) * 4 + 1];
    char argEndIndex[sizeof(int) * 4 + 1];
    pid_t pid[MAX_PROCESS_NUMBER];
    for (int index = 0; index < numberOfProcess; index++)
    {
        pivot2 = (int)round((inputLength / (float)numberOfProcess * (float)(index + 1)));
        while ((pivot2 - pivot1 + 1) % keyLength != 0 && pivot2 < inputLength - 1)
        {
            pivot2++;
        }

        pid[index] = fork();

        // Assign task to child
        if (pid[index] == 0)
        {
	    sprintf(argStartIndex, "%d", pivot1);
            childArgv[5] = argStartIndex;
	    sprintf(argEndIndex, "%d", pivot2);
            childArgv[7] = argEndIndex;

            if (execv("vigenere", childArgv) == -1)
	    {
		perror("exec");
		exit(1);
	    }
        }

	pivot1 = pivot2 + 1;
    }

    // Wait for task complete
    int status[MAX_PROCESS_NUMBER];
    for (int index = 0; index < numberOfProcess; index++)
    {
        waitpid(pid[index], &status[index], WCONTINUED);
        
        // Child process does not finished normally
        if (status[index] != 0)
        {
	    printf("%d\n", status[index]);
            printf("multicipher: something went wrong from vigenere, pid: %d\n", pid[index]);
            return 0;
        }
    }

    msync(address, inputLength, MS_SYNC);

    return 0;
}
