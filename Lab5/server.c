/*
 * Unix FinalUnixLab
 * Author: Suwhan Kim
 * Student ID: 201510743
 */

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
#define ADDR "192.168.98.132"
#define MAX_CLIENT 20
#define MAX_CHAT_ROOM 10
#define MAX_CHAT_USER 8

// DataPack 명령어 식별자
enum Command 
{
    None, 
    RequestChatRoomList, RequestJoinChatRoom, RequestQuitChatRoom,
    RequestCreateChatRoom, RequestRemoveChatRoom, RequestSendMessage, RequestConnectLog,
    ResponseChatRoomList, ResponseJoinChatRoom, ResponseQuitChatRoom,
    ResponseCreateChatRoom, ResponseRemoveChatRoom, ResponseSendMessage, ResponseConnectLog
};

// 채팅방 정보 구조체
struct ChatRoom
{
    char roomName[64];
    char password[64];
    int userCount;
    int userList[MAX_CHAT_USER];
    bool isValid;
};

// 서버 통신에 사용할 데이터 묶음 구조체
struct DataPack
{
    enum Command command;   // 명령어 식별자
    char data1[64];
    char data2[64];
    char data3[64];
    char data4[64];
    char message[252];
};


int initServer();
void resetChatRoom();

int receiveRequest(int sender, struct DataPack *dataPack);
int sendChatRoomList(int receiver);
int joinChatRoom(int receiver, char *roomName, char *userName, char *password);
int quitChatRoom(int receiver, char *roomName, char *userName);
int createChatRoom(int receiver, char *roomName, char *password);
int removeChatRoom(int receiver, char *roomName, char *password);
int sendMessage(char *roomName, char *userName, char *message);
int sendConnectLog(int receiver);
int responseDataPack(int receiver, struct DataPack *dataPack);

// 채팅방 리스트
struct ChatRoom ChatRoomList[MAX_CHAT_ROOM];

int serverSocket;
int fdmax;
fd_set master;
fd_set reader;

time_t serverUpTime;
time_t userLogin[128];

int main()
{
    // 서버 구동
    initServer();

    int newSocket;
    int readBytes;
    socklen_t sockaddrLength;
    struct sockaddr_in clientAddr;
    struct DataPack *receiveCommand;
    char buffer[sizeof(struct DataPack)];

    // 채팅방 초기화 및 기본 채팅방 생성
    resetChatRoom();
    strncpy(ChatRoomList[0].roomName, "Room1", sizeof(ChatRoomList[0].roomName) - 1);
    ChatRoomList[0].isValid = true;
    strncpy(ChatRoomList[1].roomName, "Room2", sizeof(ChatRoomList[0].roomName) - 1);
    strncpy(ChatRoomList[1].password, "123", sizeof(ChatRoomList[0].password) - 1);
    ChatRoomList[1].isValid = true;

    // 클라이언트로부터 데이터 읽어오기 보내기 시작
    FD_ZERO(&master);
    FD_ZERO(&reader);
    FD_SET(serverSocket, &master);
    fdmax = serverSocket;
    while (1)
    {
        // 데이터 입출력 대기
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
                // 클라이언트 접속 요청을 수락
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
                        time(&userLogin[newSocket]);
                        printf("accept: new connection\n");
                    }
                }
                // 클라이언트로부터 받은 요청을 처리하고 응답을 보냄
                else
                {
                    printf("Data from client\n");
                    buffer[0] = '\0';
                    if (readBytes = recv(i, buffer, sizeof(buffer), 0) <= 0)
                    {
                        // 대상 클라이언트 접속 종료
                        printf("socket %d connection closed\n", i);
                        close(i);
                        FD_CLR(i, &master);
                    }
                    else
                    {
                        // 받은 데이터를 DataPack구조체로 타입 캐스팅 후 기능 제공
                        receiveCommand = (struct DataPack *)buffer;
                        receiveRequest(i, receiveCommand);

                        memset(receiveCommand, 0, sizeof(struct DataPack));
                    }
                }
            }
        }
    }
    
    return 0;
}

// 서버 시작
int initServer()
{
    struct sockaddr_in serverAddr;

    // TCP/IP
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    int socketOption = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &socketOption, sizeof(socketOption));

    memset(&serverAddr, 0, sizeof(struct sockaddr_in));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    // serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_addr.s_addr = inet_addr(ADDR);
    // 192.168.98.132   IPv4
    // 192.168.98.2     default route

    bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr_in));

    listen(serverSocket, MAX_CLIENT);
    time(&serverUpTime);

    printf("Initiate Server\nADDR: '%s'\nPORT: '%d'\n", ADDR, PORT);

    return 0;
}

// 채팅방 초기화
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

// DataPack 해석 후 알맞는 기능 수행
int receiveRequest(int sender, struct DataPack *dataPack)
{
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
        case RequestConnectLog:
            sendConnectLog(sender);
            break;
        default:
            printf("receiveRequest: wrong command\n");
            break;
    }
}

// 현재 채팅방 정보를 보내줌
int sendChatRoomList(int receiver)
{
    struct DataPack dataPack;
    dataPack.message[0] = 0;

    char roomList[256] = { 0, };
    for (int index = 0; index < MAX_CHAT_ROOM; index++)
    {
        if (ChatRoomList[index].isValid == true)
        {
            if (strlen(ChatRoomList[index].password) > 0)
            {
                sprintf(roomList, "%s (private / [%d/%d])\n", ChatRoomList[index].roomName, ChatRoomList[index].userCount, MAX_CHAT_USER);
            }
            else
            {
                sprintf(roomList, "%s (public / [%d/%d])\n", ChatRoomList[index].roomName, ChatRoomList[index].userCount, MAX_CHAT_USER);
            }
            sprintf(dataPack.message, "%s%s", dataPack.message, roomList);
        }
    }

    dataPack.command = ResponseChatRoomList;
    responseDataPack(receiver, &dataPack);
}

// 채팅방 접속 요청을 처리하여 보내줌
int joinChatRoom(int receiver, char *roomName, char *userName, char *password)
{
    struct DataPack dataPack;
    dataPack.command = ResponseJoinChatRoom;
    strncpy(dataPack.data1, roomName, sizeof(dataPack.data1) - 1);

    for (int index = 0; index < MAX_CHAT_ROOM; index++)
    {
        if (ChatRoomList[index].isValid && strcmp(ChatRoomList[index].roomName, roomName) == 0 && strcmp(ChatRoomList[index].password, password) == 0)
        {
            strncpy(dataPack.data2, userName, sizeof(dataPack.data2) - 1);

            for (int userIndex = 0; userIndex < MAX_CHAT_USER; userIndex++)
            {
                // 접속 성공
                if (ChatRoomList[index].userList[userIndex] == -1)
                {
                    ChatRoomList[index].userList[userIndex] = receiver;
                    ChatRoomList[index].userCount++;

                    dataPack.data4[0] = 1;
                    strncpy(dataPack.message, "Join chat room", sizeof(dataPack.message) - 1);
                    responseDataPack(receiver, &dataPack);

                    sendMessage(roomName, userName, "New user join chat room");
                    return 0;
                }
            }

            // 채팅방이 가득 참
            dataPack.data4[0] = 0;
            strncpy(dataPack.message, "Chat room is full", sizeof(dataPack.message) - 1);
            responseDataPack(receiver, &dataPack);
            return -1;
        }
    }

    // 조건에 맞는 채팅방이 없음
    dataPack.data4[0] = 0;
    strncpy(dataPack.message, "Cannot join chat room, please check room name or password", sizeof(dataPack.message) - 1);
    responseDataPack(receiver, &dataPack);
    return -1;
}

// 채팅방 퇴장 요청을 처리하여 보내줌
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
                // 채팅방 퇴장 성공
                if (ChatRoomList[index].userList[userIndex] == receiver)
                {
                    ChatRoomList[index].userList[userIndex] = -1;
                    ChatRoomList[index].userCount--;

                    dataPack.data4[0] = 1;
                    strncpy(dataPack.message, "Quit chat room", sizeof(dataPack.message) - 1);
                    responseDataPack(receiver, &dataPack);

                    sendMessage(roomName, userName, "User left chat room");
                    return 0;
                }
            }
        }
    }

    // 채팅방 정보 불일치
    dataPack.data4[0] = 0;
    strncpy(dataPack.message, "Failed leave chat room", sizeof(dataPack.message) - 1);
    responseDataPack(receiver, &dataPack);
    return -1;
}

// 새로운 채팅방 생성 요청을 처리하여 보내줌
int createChatRoom(int receiver, char *roomName, char *password)
{
    struct DataPack dataPack;
    dataPack.command = ResponseCreateChatRoom;
    strncpy(dataPack.data1, roomName, sizeof(dataPack.data1) - 1);

    for (int index = 0; index < MAX_CHAT_ROOM; index++)
    {
        // 새로운 채팅방 생성 성공
        if (ChatRoomList[index].isValid == false)
        {
            strncpy(ChatRoomList[index].roomName, roomName, sizeof(ChatRoomList[index].roomName) - 1);
            strncpy(ChatRoomList[index].password, password, sizeof(ChatRoomList[index].password) - 1);
            ChatRoomList[index].userCount = 0;
            for (int userIndex = 0; userIndex < MAX_CHAT_USER; userIndex++)
                ChatRoomList[index].userList[userIndex] = -1;
            ChatRoomList[index].isValid = true;

            dataPack.data4[0] = 1;
            strncpy(dataPack.message, "New chat room created", sizeof(dataPack.message) - 1);
            responseDataPack(receiver, &dataPack);
            return 0;
        }
    }

    // 더이상 채팅방을 만들 수 없음
    dataPack.data4[0] = 0;
    strncpy(dataPack.message, "Cannot create more chat room", sizeof(dataPack.message) - 1);
    responseDataPack(receiver, &dataPack);
    return -1;
}

// 채팅방 삭제 요청
int removeChatRoom(int receiver, char *roomName, char *password)
{
    struct DataPack dataPack;
    dataPack.command = ResponseRemoveChatRoom;
    strncpy(dataPack.data1, roomName, sizeof(dataPack.data1) - 1);

    for (int index = 0; index < MAX_CHAT_ROOM; index++)
    {
        if (strcmp(ChatRoomList[index].roomName, roomName) == 0 && strcmp(ChatRoomList[index].password, password) == 0)
        {
            // 채팅방에 사용자가 있는 경우에는 삭제 불가
            if (ChatRoomList[index].userCount != 0)
            {
                dataPack.data4[0] = 0;
                strncpy(dataPack.message, "Cannot remove chat room when user is in chat room", sizeof(dataPack.message) - 1);
                responseDataPack(receiver, &dataPack);
                return -1;
            }

            // 채팅방 삭제 성공
            ChatRoomList[index].roomName[0] = 0;
            ChatRoomList[index].password[0] = 0;
            ChatRoomList[index].userCount = 0;
            for (int userIndex = 0; userIndex < MAX_CHAT_USER; userIndex++)
                ChatRoomList[index].userList[userIndex] = -1;
            ChatRoomList[index].isValid = false;

            dataPack.data4[0] = 1;
            strncpy(dataPack.message, "Remove chat room", sizeof(dataPack.message) - 1);
            responseDataPack(receiver, &dataPack);
            return 0;
        }        
    }    

    // 조건에 맞는 채팅방이 없음
    dataPack.data4[0] = 0;
    strncpy(dataPack.message, "Cannot remove chat room, please check room name or password", sizeof(dataPack.message) - 1);
    responseDataPack(receiver, &dataPack);
    return -1;
}

// 현재 채팅방 사용자들에게 메시지 전송
int sendMessage(char *roomName, char *userName, char *message)
{
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

// 서버 시작 시간, 사용자 접속 시간 정보 전송
int sendConnectLog(int receiver)
{
    char server[64];
    char login[64];
    sprintf(server, "[ServerUpTime] %s", ctime(&serverUpTime));
    sprintf(login, "[LastLogin] %s", ctime(&userLogin[receiver]));

    struct DataPack dataPack;
    dataPack.command = ResponseChatRoomList;
    sprintf(dataPack.message, "%s%s", server, login);
    responseDataPack(receiver, &dataPack);
    return 0;
}

// receiver에게 DataPack을 보내줌
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