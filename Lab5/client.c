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
#define MAX_USERNAME_LENGTH 127
#define MAX_PASSWORD_LENGTH 127

enum Command 
{
     RequestChatRoomList, RequestJoinChatRoom, RequestQuitChatRoom,
     RequestCreateChatRoom, RequestRemoveChatRoom, RequestSendMessage,
     ResponseChatRoomList, ResponseJoinChatRoom, ResponseQuitChatRoom,
     ResponseCreateChatRoom, ResponseRemoveChatRoom, ResponseSendMessage
};

enum ClientStatus
{
    Lobby, Chat
};

struct DataPack
{
    enum Command command;
    char data1[64];
    char data2[64];
    char data3[64];
    char data4[64];
    char message[252];
};

// Server
int connectToServer();

// Interface
void initInterface();
void drawBorder(WINDOW *window);
void printMessage(WINDOW *window, char *message);
void printSenderMessage(WINDOW *window, char *message, char *sender);
void receiveCommand();
void showCommandList();

// DataPack
int receiveResponse(struct DataPack *dataPack);
int requestChatRoomList();
int requestJoinChatRoom(char *roomName, char *userName, char *password);
int requestQuitChatRoom(char *roomName, char *userName);
int requestCreateChatRoom(char *roomName, char *userName, char *password);
int requestRemoveChatRoom(char *roomName, char *password);
int requestSendMessage(char *userName, char *message);
int requestDataPack(struct DataPack *dataPack);

// etc
void onClose();

WINDOW *chatWindow, *commandWindow, *commandInputWindow, *chatWindowBorder, *commandWindowBorder;
int ServerSocket;
int fdmax;
fd_set master;
fd_set reader;

int stdInput;
char stdBuffer[256] = { 0, };
enum ClientStatus clientStatus;

int main()
{
    initInterface();
    atexit(onClose);

    connectToServer();
    
    FD_ZERO(&master);
    FD_ZERO(&reader);

    FD_SET(fileno(stdin), &master);
    FD_SET(ServerSocket, &master);
    fdmax = ServerSocket;

    int readBytes;
    char socketBuffer[512];
    struct DataPack *receivedDataPack;

    clientStatus = Lobby;

    

    while (1)
    {
        showCommandList();
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
                if (i == ServerSocket)
                {
                    readBytes = recv(ServerSocket, socketBuffer, sizeof(socketBuffer), 0);
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
                        receivedDataPack = (struct DataPack *)socketBuffer;
                        receiveResponse(receivedDataPack);
                    }
                }
                else if (i == fileno(stdin))
                {
                    stdInput = getchar();
                
                    switch (stdInput)
                    {
                        case 13:
                            receiveCommand();
                            strcpy(stdBuffer, "");
                            break;
        
                        case 127:
                            stdBuffer[strlen(stdBuffer) - 1] = 0;
                            break;

                        case EOF:
                            // receiveCommand();
                            break;
        
                        default:
                            sprintf(stdBuffer, "%s%c", stdBuffer, stdInput);
                            break;
                    }
                }
                else
                {
                    // unknown input
                }
            }
        }
    }

    // char command[256];
    // while (1)
    // {
    //     sleep(1);
    //     wclear(commandWindow);
    //     wclear(commandInputWindow);
    //     printMessage(commandWindow, "1. ChatRoomList  2. JoinChatRoom  3. QuitChatRoom \n4. CreateChatRoom  5. RemoveChatRoom  6. SendMessage  7. Exit");
    //     printMessage(commandInputWindow, "Input Command: ");

    //     echo();
    //     curs_set(TRUE);
    //     wscanw(commandInputWindow, "%s", command);
    //     noecho();
    //     curs_set(FALSE);
    //     sleep(1);

    //     switch (atoi(command))
    //     {
    //         case 1:
    //             requestChatRoomList("suwhan77", "tnghks77");
    //             continue;
    //         case 2:
    //             requestJoinChatRoom("1번방", "엄준식", "12345");
    //             continue;
    //         case 3:
    //             requestQuitChatRoom("3번방", "정상길");
    //             continue;
    //         case 4:
    //             requestCreateChatRoom("5번방", "김찬호", "qwert");
    //             continue;
    //         case 5:
    //             requestRemoveChatRoom("2번방", "password");
    //             continue;
    //         case 6:
    //             requestSendMessage("Aloy", "Happy Kappa");
    //             continue;
    //         default:
    //             break;
    //     }
    //     break;
    // }

    close(ServerSocket);

    return 0;
}

int connectToServer()
{
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
    printMessage(chatWindow, "Initiate socket");

    if (connect(ServerSocket, (struct sockaddr *)&clientAddr, sizeof(struct sockaddr_in)))
    {
        perror("connect");
        exit(1);
    }
    printMessage(chatWindow, "Server connected");
}

void initInterface()
{
    initscr();
    noecho();
    curs_set(FALSE);

    int parentX, parentY;
    getmaxyx(stdscr, parentY, parentX); 

    int commandWindowBorderHeight = 5;
    int commandWindowHeight = 2;
    chatWindow = newwin(parentY - commandWindowBorderHeight - 2, parentX - 2, 1, 1);
    chatWindowBorder = newwin(parentY - commandWindowBorderHeight, parentX, 0, 0);
    commandWindow = newwin(commandWindowHeight, parentX - 2, parentY - commandWindowBorderHeight + 1, 1);
    commandInputWindow = newwin(commandWindowBorderHeight - commandWindowHeight - 2, parentX - 2, parentY - commandWindowBorderHeight + commandWindowHeight + 1, 1);
    commandWindowBorder = newwin(commandWindowBorderHeight, parentX, parentY - commandWindowBorderHeight, 0);

    scrollok(chatWindow, TRUE);

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

void printMessage(WINDOW *window, char *message)
{
    wprintw(window, "%s\n", message);
    wrefresh(window);
}

void printSenderMessage(WINDOW *window, char *message, char *sender)
{
    wprintw(window, "[%s] %s\n", sender, message);
    wrefresh(window);
}

void receiveCommand()
{
    switch (clientStatus)
    {
        case Lobby:
            switch (atoi(stdBuffer))
            {
                case 1:
                    requestChatRoomList("suwhan77", "tnghks77");
                    break;
                case 2:
                    requestJoinChatRoom("room1", "Amumu", "12345");
                    break;
                case 3:
                    requestCreateChatRoom("room5", "Ralo", "qwert");
                    break;
                case 4:
                    requestRemoveChatRoom("room2", "password");
                    break;
                default:
                    break;
            }
            break;

        case Chat:
            if (stdBuffer[0] == '!')
            {
                if (strcmp(stdBuffer, "!quit") == 0)
                    requestQuitChatRoom("room3", "Dopago");
            }
            else
            {
                requestSendMessage("Aloy", stdBuffer);
            }
            break;

    }
}

void showCommandList()
{
    switch (clientStatus)
    {
        case Lobby:
            wclear(commandWindow);
            wclear(commandInputWindow);
            printMessage(commandWindow, "1. ChatRoomList  2. JoinChatRoom  3. CreateChatRoom \n4. RemoveChatRoom  7. Exit");
            wprintw(commandInputWindow, "Input Command: %s", stdBuffer);
            wrefresh(commandInputWindow);
            break;
        
        case Chat:
            wclear(commandWindow);
            wclear(commandInputWindow);
            printMessage(commandWindow, "say '!quit' to quit chat room");
            wprintw(commandInputWindow, "Send Message: %s", stdBuffer);
            wrefresh(commandInputWindow);
            break;
    }
}

int receiveResponse(struct DataPack *dataPack)
{
    // printf("command: '%d'\ndata1: '%s'\tdata2: '%s'\tdata3: '%s'\tdata4: '%s'\nmessage: '%s'\n",
    //     dataPack->command, dataPack->data1, dataPack->data2, dataPack->data3, dataPack->data4, dataPack->message);

    switch (dataPack->command)
    {
        case ResponseChatRoomList:
            wprintw(chatWindow, "%s\n", dataPack->message);
            wrefresh(chatWindow);
            break;
        case ResponseJoinChatRoom:
            wprintw(chatWindow, "[%s] %s\n", dataPack->data1, dataPack->message);
            wrefresh(chatWindow);
            if (dataPack->data4[0])
            {
                clientStatus = Chat;
            }
            break;
        case ResponseQuitChatRoom:
            wprintw(chatWindow, "[%s] %s\n", dataPack->data1, dataPack->message);
            wrefresh(chatWindow);
            clientStatus = Lobby;
            break;
        case ResponseCreateChatRoom:
            wprintw(chatWindow, "[%s] %s\n", dataPack->data1, dataPack->message);
            wrefresh(chatWindow);
            break;
        case ResponseRemoveChatRoom:
            wprintw(chatWindow, "[%s] %s\n", dataPack->data1, dataPack->message);
            wrefresh(chatWindow);
            break;
        case ResponseSendMessage:
            wprintw(chatWindow, "[%s] %s\n", dataPack->data1, dataPack->message);
            wrefresh(chatWindow);
            break;
        default:
            wprintw(chatWindow, "receiveResponse: wrong command\n");
            wrefresh(chatWindow);
            break;
    }
}

int requestChatRoomList()
{
    struct DataPack dataPack;
    dataPack.command = RequestChatRoomList;
    requestDataPack(&dataPack);
}

int requestJoinChatRoom(char *roomName, char *userName, char *password)
{
    struct DataPack dataPack;
    dataPack.command = RequestJoinChatRoom;
    strncpy(dataPack.data1, roomName, sizeof(dataPack.data1) - 1);
    strncpy(dataPack.data2, userName, sizeof(dataPack.data2) - 1);
    strncpy(dataPack.data3, password, sizeof(dataPack.data3) - 1);

    requestDataPack(&dataPack);
}

int requestQuitChatRoom(char *roomName, char *userName)
{
    struct DataPack dataPack;
    dataPack.command = RequestQuitChatRoom;
    strncpy(dataPack.data1, roomName, sizeof(dataPack.data1) - 1);
    strncpy(dataPack.data2, userName, sizeof(dataPack.data2) - 1);

    requestDataPack(&dataPack);
}

int requestCreateChatRoom(char *roomName, char *userName, char *password)
{
    struct DataPack dataPack;
    dataPack.command = RequestCreateChatRoom;
    strncpy(dataPack.data1, roomName, sizeof(dataPack.data1) - 1);
    strncpy(dataPack.data2, userName, sizeof(dataPack.data2) - 1);
    strncpy(dataPack.data3, password, sizeof(dataPack.data3) - 1);

    requestDataPack(&dataPack);
}

int requestRemoveChatRoom(char *roomName, char *password)
{
    struct DataPack dataPack;
    dataPack.command = RequestRemoveChatRoom;
    strncpy(dataPack.data1, roomName, sizeof(dataPack.data1) - 1);
    strncpy(dataPack.data2, password, sizeof(dataPack.data2) - 1);

    requestDataPack(&dataPack);
}

int requestSendMessage(char *userName, char *message)
{
    struct DataPack dataPack;
    dataPack.command = RequestSendMessage;
    strncpy(dataPack.data1, userName, sizeof(dataPack.data1) - 1);
    strncpy(dataPack.message, message, sizeof(dataPack.message) - 1);

    requestDataPack(&dataPack);
}

int requestDataPack(struct DataPack *dataPack)
{
    int sendBytes;
    sendBytes = send(ServerSocket, (char *)dataPack, sizeof(struct DataPack), 0);

    if (sendBytes <= 0)
    {
        if (sendBytes == 0)
            wprintw(chatWindow, "Disconnected from server\n");
        else
            perror("requestDataPack: send");
        return -1;
    }
    return 0;
}

void onClose()
{
    delwin(chatWindow);
    delwin(chatWindowBorder);
    delwin(commandWindow);
    delwin(commandWindowBorder);
    endwin();
}   


// int requestRegister(char *userName, char *password)
// {
//     if (strlen(userName) < 1 || strlen(userName) > MAX_USERNAME_LENGTH ||
//         strlen(password) < 1 || strlen(password) > MAX_PASSWORD_LENGTH)
//     {
//         printf("client: invalid input");
//         return -1;
//     }

//     struct DataPack data;
//     data.command = RequestRegister;
//     strcpy(data.data1, userName);
//     strcpy(data.data2, password);

//     printf("command: '%d'\ndata1: '%s'\ndata2: '%s'\nmessage: '%s'\n",
//         data.command, data.data1, data.data2, data.message);

//     if (send(ServerSocket, (char *)&data, sizeof(struct DataPack), 0) == -1)
//     {
//         printf("%d\n", ServerSocket);
//         perror("requestRegister: send");
//         return -1;
//     }

//     return 0;
// }

// int requestLogin(char *userName, char *password)
// {
//     struct DataPack data;
//     data.command = RequestLogin;
//     strcpy(data.data1, userName);
//     strcpy(data.data2, password);

//     if (send(ServerSocket, (char *)&data, sizeof(struct DataPack), 0) == -1)
//     {
//         perror("requestLogin: send");
//         return -1;
//     }
//     return 0;
// }

// int requestLogout(char *userName)
// {
//     struct DataPack data;
//     data.command = RequestLogout;
//     strcpy(data.data1, userName);

//     if (send(ServerSocket, (char *)&data, sizeof(struct DataPack), 0) == -1)
//     {
//         perror("requestLogout: send");
//         return -1;
//     }
//     return 0;
// }

// int requestSendMessage(int roomID, char *message)
// {
//     struct DataPack data;
//     data.command = RequestSendMessage;
//     sprintf(data.data1, "%d", roomID);
//     strcpy(data.message, message);

//     if (send(ServerSocket, (char *)&data, sizeof(struct DataPack), 0) == -1)
//     {
//         perror("requestSendMessage: send");
//         return -1;
//     }
//     return 0;
// }


