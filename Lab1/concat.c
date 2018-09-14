/*
 * Unix Lab1
 * Author: Suwhan Kim
 * Student ID : 201510743
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    extern char *optarg;
    extern int optind, opterr, optopt;

    int option;
    char errorMessage[256];

    // Default option
    bool showNumber = false, suppress = false;
    int pageLength = 30;

    int fileDescriptor;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    while ((option = getopt(argc, argv, "nso:l:h")) != -1)
    {
        switch (option)
        {
            // Display line number
            case 'n':
                showNumber = true;
                break;

            // Suppress repeated empty output lines
            case 's':
                suppress = true;
                break;

            // Create output file named FileName
            case 'o':
                //Try create file
                fileDescriptor = open(("%s", optarg), O_CREAT | O_EXCL, mode);

                if (fileDescriptor == -1)
                {
                    sprintf(errorMessage, "concat: '%s'", optarg);
                    perror(errorMessage);
                }

                // Close FileDescriptor
                close(fileDescriptor);
                return 0;

            // Set page length as N line
            case 'l':
                // Parse from string
                pageLength = atoi(optarg);
                break;

            // Display a help file
            case 'h':
                // Print manual
                printf("\nSYNOPSIS\n");
                printf("\tconcat [OPTION]... [FILE]...\n");
                printf("\nDESCRIPTION\n");
                printf("\t-n\tdisplay line numbers\n\n");
                printf("\t-s\tsuppress repeated empty output lines\n\n");
                printf("\t-o, --FileName\n\t\tcreate output file named FileName\n\n");
                printf("\t-l, --PageLength\n\t\tset display page length, default value: '30'\n\n");
                printf("\t-h\tdisplay this help and exit\n\n");
                return 0;

            // Missing parameter
            default:
                if (optopt == 'o' || optopt == 'l')
                    printf("concat: Missing parameter\n");
                return 0;
        }
    }

    // Check FileName is empty
    if (argv[optind] == NULL)
        printf("concat: FileName is empty\n");
    else
    {
        // Try open file
        FILE *fileStream;
        fileStream = fopen(argv[optind], "r");

        // Failed to open file
        if (fileStream == NULL)
        {
            sprintf(errorMessage, "concat: '%s'", argv[optind]);
            perror(errorMessage);
            return 0;
        }

        int index = 1;
        bool prevLineIsEmpty = false;
        char buff[1024];
        
        while (fgets(buff, sizeof(buff), fileStream) != NULL && index <= pageLength)
        {
            // Suppress Mode
            if (suppress == true)
            {
                // Check line is empty
                if (strlen(buff) == 1 && strcmp(buff, "\n") == 0)
                {
                    // If previous line was empty too, do not print line
                    if (prevLineIsEmpty == true)
                        continue;
                    else
                        prevLineIsEmpty = true;
                }
                else
                    prevLineIsEmpty = false;
            }

            // ShowLineNumber Mode
            if (showNumber == true)
                printf("%6d\t%s", index, buff);
            else
                printf("%s", buff);

            // Increase line number
            index++;
        }

        // Close file stream
        fclose(fileStream);
    }

    return 0;
}
