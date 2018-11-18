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
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ncurses.h>

#define PORT 10743
#define ADDR "192.168.98.132"

// DataPack 명령어 식별자
enum Command 
{
    None,
    RequestChatRoomList, RequestJoinChatRoom, RequestQuitChatRoom,
    RequestCreateChatRoom, RequestRemoveChatRoom, RequestSendMessage, RequestConnectLog,
    ResponseChatRoomList, ResponseJoinChatRoom, ResponseQuitChatRoom,
    ResponseCreateChatRoom, ResponseRemoveChatRoom, ResponseSendMessage, ResponseConnectLog
};

// 현재 클라이언트 작업 상태
enum ClientStatus
{
    Lobby, Chat, GetUserName, GetPassword, GetName
};

// 서버와 통신에 사용할 데이터 묶음 구조체
struct DataPack
{
    enum Command command;   // 명령어 식별자
    char data1[64];
    char data2[64];
    char data3[64];
    char data4[64];
    char message[252];
};

// 서버
int connectToServer();
void onClose();

// 인터페이스
void initInterface();
void drawBorder(WINDOW *window);
void printMessage(WINDOW *window, char *message);
void printSenderMessage(WINDOW *window, char *message, char *sender);

// 사용자 입력 처리
void receiveCommand();
void showCommandList();
void showCustomCommand(char *command, char *inputGuide);

// 서버와 DataPack으로 통신
int receiveResponse(struct DataPack *dataPack);
int requestChatRoomList();
int requestJoinChatRoom(char *roomName, char *userName, char *password);
int requestQuitChatRoom(char *roomName, char *userName);
int requestCreateChatRoom(char *roomName, char *password);
int requestRemoveChatRoom(char *roomName, char *password);
int requestSendMessage(char *roomName, char *userName, char *message);
int requestConnectLog();
int requestDataPack(struct DataPack *dataPack);

WINDOW *chatWindow, *commandWindow, *commandInputWindow, *chatWindowBorder, *commandWindowBorder;
int ServerSocket;
int fdmax;
fd_set master;
fd_set reader;

int stdInput;
char stdBuffer[256] = { 0, };
char userNameBuffer[256];
char passwordBuffer[256];
char roomNameBuffer[256];

char currentUserName[256];      // 현재 사용자 이름
char currentRoomName[256];      // 현재 접속중인 채팅방 이름

enum Command currentRequest;        // 현재 처리중인 요청
enum ClientStatus clientStatus;     // 현재 클라이언트 작업 상태

char *serverAddress = ADDR;
int serverPort = PORT;

int main(int argc, char *argv[])
{
    // argument를 읽어 옴
    extern char *optarg;
    int option;
    while((option = getopt(argc, argv, "a:p:")) != -1)
    {
        switch (option)
        {
            // 접속할 서버 주소
            case 'a':
                serverAddress = optarg;
                break;
            
            // 접속할 서버 포트 번호
            case 'p':
                serverPort = atoi(optarg);
                break;
            
            // 잘못된 옵션
            default:
                printf("client: invalid option\n");
                return 0;
        }
    }

    // 인터페이스 그리기
    initInterface();
    atexit(onClose);

    // 서버에 접속 시도
    if (connectToServer() == -1)
    {
        // 실패
        printMessage(chatWindow, "\nCannot connect server");
        printMessage(chatWindow, "Shutdown in 10 sec");
        sleep(10);
        return 0;
    }
    
    FD_ZERO(&master);
    FD_ZERO(&reader);
    FD_SET(fileno(stdin), &master);
    FD_SET(ServerSocket, &master);
    fdmax = ServerSocket;

    int readBytes;
    char socketBuffer[512];
    struct DataPack *receivedDataPack;
    
    currentRequest = None;
    clientStatus = Lobby;

    // 사용자로부터 입력이 들어오거나 서버로부터 데이터를 주고 받기 시작
    while (1)
    {
        // 사용자에게 현재 사용 가능한 명령어 목록을 보여줌
        showCommandList();

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
                // 서버로부터 데이터를 받음
                if (i == ServerSocket)
                {
                    readBytes = recv(ServerSocket, socketBuffer, sizeof(socketBuffer), 0);

                    // 서버와 연결이 끊어짐
                    if (readBytes <= 0)
                    {
                        if (readBytes == 0)
                            printMessage(chatWindow, "Disconnected from server");
                        else                    
                            perror("recv");
                        close(ServerSocket);
                        exit(1);
                    }
                    // 서버로부터 DataPack을 받음
                    else
                    {
                        // DataPack을 해석하여 기능 수행
                        receivedDataPack = (struct DataPack *)socketBuffer;
                        receiveResponse(receivedDataPack);
                    }
                }
                // 사용자로부터 표준 입력을 읽어옴
                else if (i == fileno(stdin))
                {
                    stdInput = getchar();
                
                    switch (stdInput)
                    {
                        // 엔터키를 누르면 받은 입력을 해석하여 처리함
                        case 13:
                            receiveCommand();
                            strcpy(stdBuffer, "");
                            break;
        
                        // 백스페이스
                        case 127:
                            stdBuffer[strlen(stdBuffer) - 1] = 0;
                            break;

                        // 사용자가 입력한 문자를 버퍼에 저장
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
    
    close(ServerSocket);

    return 0;
}

// 서버에 연결 시도
int connectToServer()
{
    struct sockaddr_in serverAddr;
    memset(&serverAddr, '\0', sizeof(struct sockaddr_in));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.s_addr = inet_addr(serverAddress);

    if ((ServerSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        return -1;
    }
    printMessage(chatWindow, "Initiate socket");

    if (connect(ServerSocket, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr_in)) == -1)
    {
        perror("connect");
        return -1;
    }
    printMessage(chatWindow, "Server connected");
    return 0;
}

// 인터페이스 시작, 그리기
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

// 인터페이스 테두리 그리기
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

// 인터페이스에 메시지 출력
void printMessage(WINDOW *window, char *message)
{
    wprintw(window, "%s\n", message);
    wrefresh(window);
}

// 인터페이스에 발신자 이름과 함께 메시지 출력
void printSenderMessage(WINDOW *window, char *message, char *sender)
{
    wprintw(window, "[%s] %s\n", sender, message);
    wrefresh(window);
}

// 사용자로부터 받은 입력을 해석하여 처리함
void receiveCommand()
{
    switch (currentRequest)
    {
        case None:
            switch (clientStatus)
            {
                // 로비에서 받은 입력
                case Lobby:
                    switch (atoi(stdBuffer))
                    {
                        // 현재 채팅방 목록 요청
                        case 1:
                            requestChatRoomList();
                            break;
                        // 채팅방 입장 요청
                        case 2:
                            currentRequest = RequestJoinChatRoom;
                            clientStatus = GetName;
                            break;
                        // 채팅방 생성 요청
                        case 3:
                            currentRequest = RequestCreateChatRoom;
                            clientStatus = GetName;
                            break;
                        // 채팅방 제거 요청
                        case 4:
                            currentRequest = RequestRemoveChatRoom;
                            clientStatus = GetName;
                            break;
                        // 서버, 사용자 로그 요청
                        case 5:
                            requestConnectLog();
                        // 프로그램 종료
                        default:
                            close(ServerSocket);
                            exit(0);
                    }
                    break;
                
                // 채팅방에서 받은 입력
                case Chat:
                    // 채팅방 퇴장 요청
                    if (stdBuffer[0] == '!')
                    {
                        if (strcmp(stdBuffer, "!quit") == 0)
                        requestQuitChatRoom(currentRoomName, currentUserName);
                    }
                    // 채팅방에 메시지 전송
                    else
                    {
                        requestSendMessage(currentRoomName, currentUserName, stdBuffer);
                    }
                    break;
                
                default:
                    currentRequest = None;
                    clientStatus = Lobby;
                    break;
            }
            break;
        
        // 채팅방 참가 요청
        case RequestJoinChatRoom:
            if (clientStatus == GetName)
            {
                strncpy(roomNameBuffer, stdBuffer, sizeof(roomNameBuffer) - 1);
                clientStatus = GetPassword;
            }
            else if (clientStatus == GetPassword)
            {
                strncpy(passwordBuffer, stdBuffer, sizeof(passwordBuffer) - 1);
                clientStatus = GetUserName;
            }
            else if (clientStatus == GetUserName)
            {
                strncpy(userNameBuffer, stdBuffer, sizeof(userNameBuffer) - 1);
                requestJoinChatRoom(roomNameBuffer, userNameBuffer, passwordBuffer);

                currentRequest = None;
                clientStatus = Lobby;
                roomNameBuffer[0] = 0;
                passwordBuffer[0] = 0;
                userNameBuffer[0] = 0;
            }
            else
            {
                currentRequest = None;
                clientStatus = Lobby;
            }
            break;
        
        // 채팅방 생성 요청
        case RequestCreateChatRoom:
            if (clientStatus == GetName)
            {
                strncpy(roomNameBuffer, stdBuffer, sizeof(roomNameBuffer) - 1);
                clientStatus = GetPassword;
            }
            else if (clientStatus == GetPassword)
            {
                strncpy(passwordBuffer, stdBuffer, sizeof(passwordBuffer) - 1);
                requestCreateChatRoom(roomNameBuffer, passwordBuffer);

                currentRequest = None;
                clientStatus = Lobby;
                roomNameBuffer[0] = 0;
                passwordBuffer[0] = 0;
            }
            else
            {
                currentRequest = None;
                clientStatus = Lobby;
            }
            break;
        
        // 채팅방 삭제 요청
        case RequestRemoveChatRoom:
            if (clientStatus == GetName)
            {
                strncpy(roomNameBuffer, stdBuffer, sizeof(roomNameBuffer) - 1);
                clientStatus = GetPassword;
            }
            else if (clientStatus == GetPassword)
            {
                strncpy(passwordBuffer, stdBuffer, sizeof(passwordBuffer) - 1);
                requestRemoveChatRoom(roomNameBuffer, passwordBuffer);

                currentRequest = None;
                clientStatus = Lobby;
                roomNameBuffer[0] = 0;
                passwordBuffer[0] = 0;
            }
            else
            {
                currentRequest = None;
                clientStatus = Lobby;
            }
            break;
            
        default:
            currentRequest = None;
            break;            
    }
}

// 현재 클라이언트 상황에 맞는 명령어 목록을 보여줌
void showCommandList()
{
    switch (clientStatus)
    {
        case Lobby:
            showCustomCommand("1. ChatRoomList  2. JoinChatRoom  3. CreateChatRoom \n4. RemoveChatRoom  5. ConnectionLog  6. Exit", "Input Command");
            break;
        case Chat:
            showCustomCommand("say '!quit' to quit chat room", "Send Message");
            break;
        case GetUserName:
            showCustomCommand("Type your name", "UserName");
            break;
        case GetPassword:
            showCustomCommand("Type chat room password (If public chat room, skip this step)", "Password");
            break;
        case GetName:
            showCustomCommand("Type chat room name", "RoomName");
            break;
    }
}

// 명령어 보여주기
void showCustomCommand(char *command, char *inputGuide)
{
    wclear(commandWindow);
    wclear(commandInputWindow);

    wprintw(commandWindow, command);
    wprintw(commandInputWindow, "%s: %s", inputGuide, stdBuffer);

    wrefresh(commandWindow);
    wrefresh(commandInputWindow);
}

// 서버로부터 DataPack을 받아 해석후 기능 수행
int receiveResponse(struct DataPack *dataPack)
{
    switch (dataPack->command)
    {
        // 채팅방 목록 요청
        case ResponseChatRoomList:
            wprintw(chatWindow, "%s\n", dataPack->message);
            wrefresh(chatWindow);
            break;
        // 채팅방 참가 요청
        case ResponseJoinChatRoom:
            wclear(chatWindow);
            wprintw(chatWindow, "[%s] %s\n", dataPack->data1, dataPack->message);
            wrefresh(chatWindow);
            // 접속 성공
            if (dataPack->data4[0])
            {
                strncpy(currentRoomName, dataPack->data1, sizeof(currentRoomName) - 1);
                strncpy(currentUserName, dataPack->data2, sizeof(currentUserName) - 1);
                clientStatus = Chat;
            }
            break;
        // 채팅방 퇴장 요청
        case ResponseQuitChatRoom:
            wclear(chatWindow);
            wprintw(chatWindow, "[%s] %s\n", dataPack->data1, dataPack->message);
            wrefresh(chatWindow);
            clientStatus = Lobby;
            break;
        // 채팅방 생성 요청
        case ResponseCreateChatRoom:
            wprintw(chatWindow, "[%s] %s\n", dataPack->data1, dataPack->message);
            wrefresh(chatWindow);
            break;
        // 채팅방 삭제 요청
        case ResponseRemoveChatRoom:
            wprintw(chatWindow, "[%s] %s\n", dataPack->data1, dataPack->message);
            wrefresh(chatWindow);
            break;
        // 채팅방 메시지 전송 요청
        case ResponseSendMessage:
            wprintw(chatWindow, "[%s] %s\n", dataPack->data1, dataPack->message);
            wrefresh(chatWindow);
            break;
        // 서버 시작 시간, 사용자 접속 시간 요청
        case ResponseConnectLog:
            wprintw(chatWindow, "%s\n", dataPack->message);
            wrefresh(chatWindow);
            break;
        default:
            wprintw(chatWindow, "receiveResponse: wrong command\n");
            wrefresh(chatWindow);
            break;
    }
}

// 채팅방 목록 요청
int requestChatRoomList()
{
    struct DataPack dataPack;
    dataPack.command = RequestChatRoomList;
    requestDataPack(&dataPack);
}

// 채팅방 참가 요청
int requestJoinChatRoom(char *roomName, char *userName, char *password)
{
    struct DataPack dataPack;
    dataPack.command = RequestJoinChatRoom;
    strncpy(dataPack.data1, roomName, sizeof(dataPack.data1) - 1);
    strncpy(dataPack.data2, userName, sizeof(dataPack.data2) - 1);
    strncpy(dataPack.data3, password, sizeof(dataPack.data3) - 1);

    requestDataPack(&dataPack);
}

// 채팅방 퇴장 요청
int requestQuitChatRoom(char *roomName, char *userName)
{
    struct DataPack dataPack;
    dataPack.command = RequestQuitChatRoom;
    strncpy(dataPack.data1, roomName, sizeof(dataPack.data1) - 1);
    strncpy(dataPack.data2, userName, sizeof(dataPack.data2) - 1);

    requestDataPack(&dataPack);
}

// 채팅방 생성 요청
int requestCreateChatRoom(char *roomName, char *password)
{
    struct DataPack dataPack;
    dataPack.command = RequestCreateChatRoom;
    strncpy(dataPack.data1, roomName, sizeof(dataPack.data1) - 1);
    strncpy(dataPack.data2, password, sizeof(dataPack.data2) - 1);

    requestDataPack(&dataPack);
}

// 채팅방 삭제 요청
int requestRemoveChatRoom(char *roomName, char *password)
{
    struct DataPack dataPack;
    dataPack.command = RequestRemoveChatRoom;
    strncpy(dataPack.data1, roomName, sizeof(dataPack.data1) - 1);
    strncpy(dataPack.data2, password, sizeof(dataPack.data2) - 1);

    requestDataPack(&dataPack);
}

// 채팅방 메시지 전송 요청
int requestSendMessage(char *roomName, char *userName, char *message)
{
    struct DataPack dataPack;
    dataPack.command = RequestSendMessage;
    strncpy(dataPack.data1, roomName, sizeof(dataPack.data1) - 1);
    strncpy(dataPack.data2, userName, sizeof(dataPack.data2) - 1);
    strncpy(dataPack.message, message, sizeof(dataPack.message) - 1);

    requestDataPack(&dataPack);
}

// 서버 시작 시간, 사용자 접속 시간 요청
int requestConnectLog()
{
    struct DataPack dataPack;
    dataPack.command = RequestConnectLog;
    requestDataPack(&dataPack);
}

// 서버에 DataPack 전송 도우미
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

// 프로그램 종료 시 수행
void onClose()
{
    delwin(chatWindow);
    delwin(chatWindowBorder);
    delwin(commandWindow);
    delwin(commandWindowBorder);
    endwin();
}   