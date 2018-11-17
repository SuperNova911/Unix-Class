#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void gotoxy(int x, int y);
void printMessage();

char messageQueue[10][128] = { 0, };

int main()
{
    system("clear");

    gotoxy(10, 0);
    printf("Client");

    gotoxy(10, 2);
    printf("Message");
    
    gotoxy(10, 14);
    printf("Command");

    
    sleep(2);
    
    sleep(2);
    
    sleep(2);
    
    sleep(2);
    
    sleep(2);
    
    sleep(2);
    
    return 0;
}

void gotoxy(int x, int y)
{
    printf("%c[%d;%df", 0x1B, y, x);
}

void printMessage()
{
    for (int index = 0; index < 10; index++)
    {
        gotoxy(0, 3 + index);
        printf("%s", messageQueue[index]);
    }
}