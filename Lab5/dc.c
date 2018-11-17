#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ncurses.h>

#define PORT 10743

void printMessage(WINDOW *window, char *message);
void initInterface();
void drawBorder(WINDOW *window);
void onClose();

WINDOW *chatWindow, *commandWindow, *commandInputWindow, *chatWindowBorder, *commandWindowBorder;
int ServerSocket;
fd_set master;
fd_set reader;
int fdmax;

int main()
{
    initInterface();
    atexit(onClose);

    // #######################
    struct sockaddr_in clientAddr;
    memset(&clientAddr, '\0', sizeof(struct sockaddr_in));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(PORT);
    clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if ((ServerSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    if (connect(ServerSocket, (struct sockaddr *)&clientAddr, sizeof(struct sockaddr_in)))
    {
        perror("connect");
        exit(1);
    }

    FD_ZERO(&master);
    FD_ZERO(&reader);

    FD_SET(ServerSocket, &master);
    FD_SET(fileno(stdin), &master);
    fdmax = ServerSocket;

    char buffer[512];
    struct DataPack *receivedDataPack;
    int readBytes;
    char get;
    while (1)
    {
        
        printMessage(chatWindow, "select");
        reader = master;
        if (select(fdmax + 1, &reader, NULL, NULL, NULL) == -1)
        {
            perror("select");
            exit(1);
        }
        
        for (int i = 0; i <= fdmax; i++)
        {
            if (FD_ISSET(i, &reader))
            {
                wprintw(chatWindow, "%d ", i);
                wrefresh(chatWindow);
                if (i == ServerSocket)
                {
                    printMessage(chatWindow, "ServerSocket");
                    readBytes = recv(ServerSocket, buffer, sizeof(buffer), 0);
                    if (readBytes <= 0)
                    {
                        if (readBytes == 0)
                            printMessage(chatWindow, "Disconnected from server");
                        else                    
                            perror("recv");
                        close(ServerSocket);
                        exit(1);
                    }
                    else
                    {
                        receivedDataPack = (struct DataPack *)buffer;
                        // receiveResponse(receivedDataPack);
                    }
                }
                else
                {
                    get = getchar();
                    printMessage(chatWindow, "else");
                }
            }
        }
    }
    // #######################

    
    return 0;
}

void printMessage(WINDOW *window, char *message)
{
    wprintw(window, "%s\n", message);
    wrefresh(window);
}

void initInterface()
{
    initscr();
    noecho();
    curs_set(FALSE);
    scrollok(chatWindow, TRUE);

    int parentX, parentY;
    getmaxyx(stdscr, parentY, parentX); 

    int commandWindowBorderHeight = 5;
    int commandWindowHeight = 2;
    chatWindow = newwin(parentY - commandWindowBorderHeight - 2, parentX - 2, 1, 1);
    chatWindowBorder = newwin(parentY - commandWindowBorderHeight, parentX, 0, 0);
    commandWindow = newwin(commandWindowHeight, parentX - 2, parentY - commandWindowBorderHeight + 1, 1);
    commandInputWindow = newwin(commandWindowBorderHeight - commandWindowHeight - 2, parentX - 2, parentY - commandWindowBorderHeight + commandWindowHeight + 1, 1);
    commandWindowBorder = newwin(commandWindowBorderHeight, parentX, parentY - commandWindowBorderHeight, 0);

    drawBorder(chatWindowBorder);
    drawBorder(commandWindowBorder);

    mvwprintw(chatWindowBorder, 0, 4, "MESSAGE"); 
    mvwprintw(commandWindowBorder, 0, 4, "COMMAND"); 

    wrefresh(chatWindow);
    wrefresh(chatWindowBorder);
    wrefresh(commandWindow);
    wrefresh(commandInputWindow);
    wrefresh(commandWindowBorder);
}

void drawBorder(WINDOW *window)
{
    int x, y;
    getmaxyx(window, y, x);
    mvwprintw(window, 0, 0, "+"); 
    mvwprintw(window, y - 1, 0, "+"); 
    mvwprintw(window, 0, x - 1, "+"); 
    mvwprintw(window, y - 1, x - 1, "+"); 

    for (int i = 1; i < (y - 1); i++)
    {
        mvwprintw(window, i, 0, "|"); 
        mvwprintw(window, i, x - 1, "|"); 
    }
    for (int i = 1; i < (x - 1); i++)
    {
        mvwprintw(window, 0, i, "-"); 
        mvwprintw(window, y - 1, i, "-"); 
    }
}   

void onClose()
{
    delwin(chatWindow);
    delwin(chatWindowBorder);
    delwin(commandWindow);
    delwin(commandWindowBorder);
    endwin();
    printf("onClose");
}   




// #####################################
// #####################################
// #####################################
// #####################################
// #####################################
// #####################################
// #####################################
// #####################################
// #####################################
// #####################################
// #####################################
// #####################################
// #####################################
// #####################################
// #####################################
// #####################################
// #####################################
// #####################################
// #####################################
// #####################################
// #####################################
// #####################################
// #####################################
// #####################################
// #####################################
// #####################################
// #####################################
// #####################################
// #####################################
// #####################################
// #####################################
// #####################################
// #####################################
// #####################################
// #####################################
// #####################################
// #####################################
// #####################################
// #####################################
// #####################################
// #####################################
// #####################################  