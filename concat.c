#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int n;

    extern char *optarg;
    extern int optind, opterr, optopt;

    int number = 0, suppress = 0;
    int pageLength = 20;

    int fd;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    while ((n = getopt(argc, argv, "nsol:h")) != -1)
    {
        switch (n)
        {
            case 'n':
                number = 1;
                printf("-n option\n");
                break;
            case 's':
                suppress = 1;
                break;
            case 'o':

                fd = open(("%s", argv[optind]), O_CREAT, mode);
                break;
            case 'l':

                printf("-l option with %s\n", optarg);
                break;
            case 'h':
                printf("Usage: concat -h -l N -o outfile file\n");
                printf("    -h : Display help\n");
                printf("    -l : Display each N Lines\n");
                exit(0);
                break;
        }
    }

    printf("filename = %s\n", argv[optind]);

    return 0;
}
