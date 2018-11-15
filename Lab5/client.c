#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

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

struct DataPack
{
    enum Command command;
    char data1[64];
    char data2[64];
    char data3[64];
    char data4[64];
    char message[252];
};

int receiveResponse(struct DataPack *dataPack);

int requestChatRoomList();
int requestJoinChatRoom(char *roomName, char *userName, char *password);
int requestQuitChatRoom(char *roomName, char *userName);
int requestCreateChatRoom(char *roomName, char *userName, char *password);
int requestRemoveChatRoom(char *roomName, char *password);
int requestSendMessage(char *userName, char *message);

int requestDataPack(struct DataPack *dataPack);


int ServerSocket;

int main()
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

    if (connect(ServerSocket, (struct sockaddr *)&clientAddr, sizeof(struct sockaddr_in)))
    {
        perror("connect");
        exit(1);
    }

    int readBytes;
    char buffer[512];
    struct DataPack *receivedDataPack;
    switch (fork())
    {
        case 0:
            while (1)
            {
                readBytes = recv(ServerSocket, buffer, sizeof(buffer), 0);
                if (readBytes <= 0)
                {
                    if (readBytes == 0)
                        printf("disconnected from server\n");
                    else                    
                        perror("recv");

                    close(ServerSocket);
                    exit(1);
                }

                receivedDataPack = (struct DataPack *)buffer;
                receiveResponse(receivedDataPack);
                // printf("received message: '%s'\n", buffer);
            }
            exit(1);
    }

    int command;
    while (1)
    {
        printf("1. ChatRoomList  2. JoinChatRoom  3. QuitChatRoom \n4. CreateChatRoom  5. RemoveChatRoom  6. SendMessage  7. Exit\n");
        scanf("%d", &command);
        // printf("command: %d\n", command);

        switch (command)
        {
            case 1:
                requestChatRoomList("suwhan77", "tnghks77");
                continue;
            case 2:
                requestJoinChatRoom("1번방", "엄준식", "12345");
                continue;
            case 3:
                requestQuitChatRoom("3번방", "정상길");
                continue;
            case 4:
                requestCreateChatRoom("5번방", "김찬호", "qwert");
                continue;
            case 5:
                requestRemoveChatRoom("2번방", "password");
                continue;
            case 6:
                requestSendMessage("엄준식", "톰클라스");
                continue;
            default:
                break;
        }
        break;
    }

    close(ServerSocket);

    return 0;
}

int receiveResponse(struct DataPack *dataPack)
{
    // printf("command: '%d'\ndata1: '%s'\tdata2: '%s'\tdata3: '%s'\tdata4: '%s'\nmessage: '%s'\n",
    //     dataPack->command, dataPack->data1, dataPack->data2, dataPack->data3, dataPack->data4, dataPack->message);

    switch (dataPack->command)
    {
        case ResponseChatRoomList:
            printf("ResponseChatRoomList\nMessage: '%s'\n", dataPack->message);
            break;
        case ResponseJoinChatRoom:
            printf("ResponseJoinChatRoom\n채팅방: '%s', Message: '%s'\n", dataPack->data1, dataPack->message);
            break;
        case ResponseQuitChatRoom:
            printf("ResponseQuitChatRoom\n채팅방: '%s', Message: '%s'\n", dataPack->data1, dataPack->message);
            break;
        case ResponseCreateChatRoom:
            printf("ResponseCreateChatRoom\n채팅방: '%s', Message: '%s'\n", dataPack->data1, dataPack->message);
            break;
        case ResponseRemoveChatRoom:
            printf("ResponseRemoveChatRoom\n채팅방: '%s', Message: '%s'\n", dataPack->data1, dataPack->message);
            break;
        case ResponseSendMessage:
            printf("ResponseRemoveChatRoom\n유저: '%s', Message: '%s'\n", dataPack->data1, dataPack->message);
            break;
        default:
            printf("receiveResponse: wrong command\n");
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
            printf("disconnected from server\n");
        else
            perror("requestDataPack: send");
        return -1;
    }
    return 0;
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


