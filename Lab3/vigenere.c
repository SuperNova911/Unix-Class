#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
    extern char *optarg;
    extern int optind, opterr, optopt;

    int option;
    char *key;
    char *plain;

    strlen()

    while ((option = getopt(argc, argv, "nso:l:h")) != -1)
    {
        switch (option)
        {
            // file name
            case 'f':
                break;

            // encryption
            case 'c':
                break;
            
            // decryption
            case 'd':
                break;
            
            // key
            case 'k':
                break;

            // output
            case 'o':
                break;
                
            // invalid parameter
            default:
                break;
        }
    }


}