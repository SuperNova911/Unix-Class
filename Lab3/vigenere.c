#include <stdio.h>
#include <string.h>

#define MAX_SIZE 4096

int main(int argc, char *argv[])
{
    extern char *optarg;
    extern int optind, opterr, optopt;

    int option;

    char *key;
    char *plain = malloc(sizeof(char) * MAX_SIZE);
    char *padding = 127;
    
    key = "dev-c";
    strcpy(plain, "this is sample");
        
    int plainLength, paddingLength, keyLength;

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
    

    keyLength = strlen(key);
    plainLength = strlen(plain);
    paddingLength = keyLength - (plainLength % keyLength);
    
	int index;
    for (index = 0; index < paddingLength; index++)
    {
        strcat(plain, &padding);
    }
    
    
    printf("%s", plain);
    printf("\n");
    
    for (index = 0; index < 20; index++)
    {
    	printf("%3d ", plain[index]);
	}
	printf("\n");
	
    for (index = 0; index < 20; index++)
    {
    	printf("%3c ", plain[index]);
	}
	printf("\n");
    
    

    return 0;
}
