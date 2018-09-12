#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int n;

    extern char *optarg;
    extern int optind, opterr, optopt;

    while ((n = getopt(argc, argv, "nl:h")) != -1)
    {
        switch (n)
        {
            case 'n':
                printf("-n option\n");
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
