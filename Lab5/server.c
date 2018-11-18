#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <fcntl.h>
#include <stdbool.h>

#define PORT 10743
#define LOG_FILE "server.log"
#define USER_DB_FOLDER "server_user"
#define MAX_USERNAME_LENGTH 127
#define MAX_PASSWORD_LENGTH 127
#define MAX_CHAT_ROOM 10
#define MAX_CHAT_USER 8

enum Command 
{
    None, 
    RequestChatRoomList, RequestJoinChatRoom, RequestQuitChatRoom,
    RequestCreateChatRoom, RequestRemoveChatRoom, RequestSendMessage,
    ResponseChatRoomList, ResponseJoinChatRoom, ResponseQuitChatRoom,
    ResponseCreateChatRoom, ResponseRemoveChatRoom, ResponseSendMessage
};

struct ChatRoom
{
    char roomName[64];
    char password[64];
    int userCount;
    int userList[MAX_CHAT_USER];
    bool isValid;
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

void resetChatRoom();

int receiveRequest(int sender, struct DataPack *dataPack);
int sendChatRoomList(int receiver);
int joinChatRoom(int receiver, char *roomName, char *userName, char *password);
int quitChatRoom(int receiver, char *roomName, char *userName);
int createChatRoom(int receiver, char *roomName, char *password);
int removeChatRoom(int receiver, char *roomName, char *password);
int sendMessage(char *roomName, char *userName, char *message);
int responseDataPack(int receiver, struct DataPack *dataPack);


struct ChatRoom ChatRoomList[MAX_CHAT_ROOM];

int serverSocket;
fd_set master;
fd_set reader;
int fdmax;

int main(int argc, char *argv[])
{
    extern char *optarg;

    int maxClientNum = 10;

    // Get option
    int option;
    while((option = getopt(argc, argv, "m:")) != -1)
    {
        switch (option)
        {
            case 'm':
                maxClientNum = atoi(optarg);
                break;
            default:
                printf("server: invalid option\n");
                break;
        }
    }

    if (maxClientNum < 1)
    {
        printf("server: wrong maxClientNum: %d\n", maxClientNum);
        exit(1);
    }

    // for (int index = 0; index < MAX_CHAT_ROOM; index++)
    // {
    //     ChatRoomList[index] = (struct ChatRoomList *)NULL;
    // }


    int newSocket;

    struct sockaddr_in serverAddr, clientAddr;
    socklen_t sockaddrLength;

    struct DataPack *receiveCommand;
    char buffer[sizeof(struct DataPack)];
    int readBytes;

    FD_ZERO(&master);
    FD_ZERO(&reader);

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    int socketOption = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &socketOption, sizeof(socketOption));

    memset(&serverAddr, 0, sizeof(struct sockaddr_in));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    sockaddrLength = sizeof(struct sockaddr_in);
    bind(serverSocket, (struct sockaddr *)&serverAddr, sockaddrLength);

    listen(serverSocket, 1);

    FD_SET(serverSocket, &master);
    fdmax = serverSocket;

    resetChatRoom();
    strncpy(ChatRoomList[0].roomName, "Room1(public)", sizeof(ChatRoomList[0].roomName) - 1);
    ChatRoomList[0].isValid = true;
    strncpy(ChatRoomList[1].roomName, "Room2(private)", sizeof(ChatRoomList[0].roomName) - 1);
    strncpy(ChatRoomList[1].password, "123", sizeof(ChatRoomList[0].password) - 1);
    ChatRoomList[1].isValid = true;

    while (1)
    {
        reader = master;
        printf("\nServerSocket: %d\n", serverSocket);
        printf("Blocking in select...\n");

        if (select(fdmax + 1, &reader, NULL, NULL, NULL) == -1)
        {
            perror("select");
            exit(1);
        }

        printf("fdmax: %d\n", fdmax);
        for (int i = 0; i <= fdmax; i++)
        {
            // printf("i: %d\n", i);
            if (FD_ISSET(i, &reader))
            {
                // printf("IN FD_ISSET\n");
                if (i == serverSocket)
                {
                    newSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &sockaddrLength);

                    if (newSocket == -1)
                    {
                        perror("accept");
                    }
                    else
                    {
                        FD_SET(newSocket, &master);
                        if (newSocket > fdmax)
                        {
                            fdmax = newSocket;
                        }
                        printf("accept: new connection\n");
                    }
                }
                else
                {
                    printf("Handle data from client\n");
                    buffer[0] = '\0';
                    if (readBytes = recv(i, buffer, sizeof(buffer), 0) <= 0)
                    {
                        printf("readBytes: %d '%c'\n", readBytes, buffer[0]);
                        if (readBytes == 1)
                            printf("socket %d connection closed\n", i);
                        else
                            perror("recv");
                        close(i);
                        FD_CLR(i, &master);
                    }
                    else
                    {
                        receiveCommand = (struct DataPack *)buffer;
                        receiveRequest(i, receiveCommand);

                        memset(receiveCommand, 0, sizeof(struct DataPack));

                        // continue;
                        // printf("Data send to client\n");
                        // for (int j = 0; j <= fdmax; j++)
                        // {
                        //     if (FD_ISSET(j, &master))
                        //     {
                        //         printf("serverSocket: %d, sender: %d, receiver: %d\n", serverSocket, i, j);
                        //         if (j != serverSocket && j != i)
                        //         {
                        //             printf("Send data\n");
                        //             if (send(j, buffer, readBytes, 0) == -1)
                        //             {
                        //                 perror("send");
                        //             }
                        //         }
                        //     }
                        // }
                    }
                }
            }
        }
    }





    
    return 0;
}

void resetChatRoom()
{
    for (int index = 0; index < MAX_CHAT_ROOM; index++)
    {
        ChatRoomList[index].roomName[0] = 0;
        ChatRoomList[index].password[0] = 0;
        ChatRoomList[index].userCount = 0;
        for (int userIndex = 0; userIndex < MAX_CHAT_USER; userIndex++)
            ChatRoomList[index].userList[userIndex] = -1;
        ChatRoomList[index].isValid = false;
    }
}

int receiveRequest(int sender, struct DataPack *dataPack)
{
    printf("command: '%d'\ndata1: '%s'\ndata2: '%s'\ndata3: '%s'\ndata4: '%s'\nmessage: '%s'\n",
        dataPack->command, dataPack->data1, dataPack->data2, dataPack->data3, dataPack->data4, dataPack->message);

    switch (dataPack->command)
    {
        case RequestChatRoomList:
            sendChatRoomList(sender);
            break;
        case RequestJoinChatRoom:
            joinChatRoom(sender, dataPack->data1, dataPack->data2, dataPack->data3);
            break;
        case RequestQuitChatRoom:
            quitChatRoom(sender, dataPack->data1, dataPack->data2);
            break;
        case RequestCreateChatRoom:
            createChatRoom(sender, dataPack->data1, dataPack->data2);
            break;
        case RequestRemoveChatRoom:
            removeChatRoom(sender, dataPack->data1, dataPack->data2);
            break;
        case RequestSendMessage:
            sendMessage(dataPack->data1, dataPack->data2, dataPack->message);
            break;
        default:
            printf("receiveRequest: wrong command\n");
            break;
    }
}

int sendChatRoomList(int receiver)
{
    char roomList[256] = "dja: ";
    for (int index = 0; index < MAX_CHAT_ROOM; index++)
    {
        if (ChatRoomList[index].isValid == true)
        {
            sprintf(roomList, "%s %s", roomList, ChatRoomList[index].roomName);
        }
    }

    struct DataPack dataPack;
    dataPack.command = ResponseChatRoomList;
    strncpy(dataPack.message, roomList, sizeof(dataPack.message) - 1);
    
    responseDataPack(receiver, &dataPack);
}

int joinChatRoom(int receiver, char *roomName, char *userName, char *password)
{
    struct DataPack dataPack;
    dataPack.command = ResponseJoinChatRoom;

    for (int index = 0; index < MAX_CHAT_ROOM; index++)
    {
        if (ChatRoomList[index].isValid && strcmp(ChatRoomList[index].roomName, roomName) == 0 && strcmp(ChatRoomList[index].password, password) == 0)
        {
            strncpy(dataPack.data1, ChatRoomList[index].roomName, sizeof(dataPack.data1) - 1);
            strncpy(dataPack.data2, userName, sizeof(dataPack.data2) - 1);
            dataPack.data4[0] = 1;
            strncpy(dataPack.message, "Join chat room", sizeof(dataPack.message) - 1);

            responseDataPack(receiver, &dataPack);

            for (int index = 0; index < MAX_CHAT_ROOM; index++)
            {
                if (strcmp(ChatRoomList[index].roomName, roomName) == 0)
                {
                    for (int userIndex = 0; userIndex < MAX_CHAT_USER; userIndex++)
                    {
                        if (ChatRoomList[index].userList[userIndex] != -1)
                        {
                            ChatRoomList[index].userList[userIndex] = receiver;
                            ChatRoomList[index].userCount++;
                        }
                    }
                }
            }
            return 0;
        }
    }

    strncpy(dataPack.data1, "", sizeof(dataPack.data1) - 1);
    dataPack.data4[0] = 0;
    strncpy(dataPack.message, "Cannot join chat room, please check room name or password", sizeof(dataPack.message) - 1);
    responseDataPack(receiver, &dataPack);
    return -1;
}

int quitChatRoom(int receiver, char *roomName, char *userName)
{
    struct DataPack dataPack;
    dataPack.command = ResponseQuitChatRoom;
    strncpy(dataPack.data1, roomName, sizeof(dataPack.data1) - 1);

    for (int index = 0; index < MAX_CHAT_ROOM; index++)
    {
        if (strcmp(ChatRoomList[index].roomName, roomName) == 0)
        {
            for (int userIndex = 0; userIndex < MAX_CHAT_USER; userIndex++)
            {
                if (ChatRoomList[index].userList[userIndex] == receiver)
                {
                    ChatRoomList[index].userList[userIndex] = -1;
                    ChatRoomList[index].userCount--;

                    strncpy(dataPack.message, "Quite chat room", sizeof(dataPack.message) - 1);
                    dataPack.data4[0] = 1;
                    responseDataPack(receiver, &dataPack);
                    return 0;
                }
            }
        }
    }

    strncpy(dataPack.message, "Failed leave chat room", sizeof(dataPack.message) - 1);
    dataPack.data4[0] = 0;
    responseDataPack(receiver, &dataPack);
    return -1;
}

int createChatRoom(int receiver, char *roomName, char *password)
{
    char *message = "Create new chat room";

    struct DataPack dataPack;
    dataPack.command = ResponseCreateChatRoom;
    strncpy(dataPack.data1, roomName, sizeof(dataPack.data1) - 1);
    strncpy(dataPack.message, message, sizeof(dataPack.message) - 1);

    responseDataPack(receiver, &dataPack);
}

int removeChatRoom(int receiver, char *roomName, char *password)
{
    char *message = "Remove chat room";

    struct DataPack dataPack;
    dataPack.command = ResponseRemoveChatRoom;
    strncpy(dataPack.data1, roomName, sizeof(dataPack.data1) - 1);
    strncpy(dataPack.message, message, sizeof(dataPack.message) - 1);

    responseDataPack(receiver, &dataPack);
}

int sendMessage(char *roomName, char *userName, char *message)
{
    printf("%s %s %s\n", roomName, userName, message);
    struct DataPack dataPack;
    dataPack.command = ResponseSendMessage;

    strncpy(dataPack.data1, userName, sizeof(dataPack.data1) - 1);
    strncpy(dataPack.message, message, sizeof(dataPack.message) - 1);

    for (int index = 0; index < MAX_CHAT_ROOM; index++)
    {
        if (strcmp(ChatRoomList[index].roomName, roomName) == 0)
        {
            for (int userIndex = 0; userIndex < ChatRoomList[index].userCount; userIndex++)
            {
                if (FD_ISSET(ChatRoomList[index].userList[userIndex], &master) && ChatRoomList[index].userList[userIndex] != serverSocket)
                    responseDataPack(ChatRoomList[index].userList[userIndex], &dataPack);
            }
        }
    }
}

int responseDataPack(int receiver, struct DataPack *dataPack)
{
    printf("responseDataPack\n");
    int sendBytes;
    sendBytes = send(receiver, (char *)dataPack, sizeof(struct DataPack), 0);

    if (sendBytes <= 0)
    {
        if (sendBytes == 0)
            printf("disconnected from client\n");
        else
            perror("responseDataPack: send");
        return -1;
    }
    return 0;
}

// int registerUser(char *userName, char *password)
// {
//     // Check invalid input
//     if (strlen(userName) < 1 || strlen(password) < 1 || 
//         MAX_USERNAME_LENGTH < strlen(userName) || MAX_PASSWORD_LENGTH < strlen(password))
//     {
//         return -1;
//     }

//     char *dbPath = (char *)malloc(strlen(userName) + strlen(USER_DB_FOLDER) + 1);
//     sprintf(dbPath, "%s/%s", USER_DB_FOLDER, userName);

//     // Check db already exists
//     if (access(dbPath, F_OK) == 0)
//     {
//         printf("userName: %s, already exists\n", userName);
//         return -1;
//     }
    
//     // Create new database file
//     FILE *fileStream;
//     fileStream = fopen(dbPath, "w+");
//     if (fileStream == NULL)
//     {
//         perror("fopen");
//         printf("server: failed to create new user db: %s\n", dbPath);
//         return -1;
//     }

//     char logMessage[1024];
//     sprintf(logMessage, "registerUser, userName: %s, password: %s", userName, password);
//     loggingData(logMessage);

//     fputs(userName, fileStream);
//     fputc('\n', fileStream);
//     fputs(password, fileStream);
//     fputc('\n', fileStream);

//     fclose(fileStream);
//     return 0;
// }

// int loginUser(char *userName, char *password)
// {
//     // Check invalid input
//     if (strlen(userName) < 1 || strlen(password) < 1)
//     {
//         return -1;
//     }

//     char *dbPath = (char *)malloc(strlen(userName) + strlen(USER_DB_FOLDER) + 1);
//     sprintf(dbPath, "%s%s", USER_DB_FOLDER, userName);

//     FILE *fileStream;
//     fileStream = fopen(dbPath, "r");
//     if (fileStream == NULL)
//     {
//         return -1;
//     }

//     const int bufferSize = 1024;
//     char userName_db[bufferSize];
//     char password_db[bufferSize];

//     fgets(userName_db, bufferSize, fileStream);
//     fgets(password_db, bufferSize, fileStream);
//     fclose(fileStream);

//     if (strlen(userName_db) < 1 || strlen(password_db) < 1)
//     {
//         printf("server: wrong db format\n");
//         return -1;
//     }

//     if (strcmp(userName_db, userName) == 0 && strcmp(password_db, password) == 0)
//     {
//         return 0;
//     }
//     else
//     {
//         return -1;
//     }
// }

// int logoutUser(char *userName)
// {
//     // Check invalid input
//     if (strlen(userName) < 1)
//     {
//         return -1;
//     }

//     char *dbPath = (char *)malloc(strlen(userName) + strlen(USER_DB_FOLDER) + 1);
//     sprintf(dbPath, "%s%s", USER_DB_FOLDER, userName);

//     return 0;
// }

// int sendResult(int receiver, char *data1, char *data2, char *message)
// {
//     struct DataPack dataPack;
//     dataPack.command = SendResult;
//     strcpy(dataPack.data1, data1);
//     strcpy(dataPack.data2, data2);
//     strcpy(dataPack.message, message);

//     if (send(receiver, (char *)&dataPack, sizeof(struct DataPack), 0) == -1)
//     {
//         perror("sendResult");
//         return -1;
//     }
//     return 0;
// }

// int sendMessage(int roomID, char *message)
// {
//     printf("message to send: '%s'\n", message);
//     for (int index = 0; index <= fdmax; index++)
//     {
//         printf("send to %d?: ", index);
//         if (FD_ISSET(index, &master) && index != serverSocket)
//         {
//             printf("yes\n");
//             if ((send(index, message, strlen(message) + 1, 0)) == -1)
//             {
//                 perror("send");
//             }
//         }
//         else
//             printf("no\n");
//     }
// }


