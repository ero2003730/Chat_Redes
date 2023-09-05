#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFFER_SIZE 512
#define DEFAULT_PORT "8080"
#define SERVER_ADDRESS "192.168.3.196"

void error_verification(char error[], int erro)
{
    printf("%s: %d\n", error, erro);
    WSACleanup();
}

int __cdecl main()
{
    WSADATA wsaData;
    int status;

    SOCKET ConnectSocket = INVALID_SOCKET;
    char sendbuffer[BUFFER_SIZE] = "Olá, servidor!";
    int sendbufferlen = strlen(sendbuffer);
    char recvbuffer[BUFFER_SIZE];
    int recvbufferlen = BUFFER_SIZE;

    // Initialize Winsock
    status = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (status != 0)
    {
        error_verification("WSAStartup failed", status);
        return 1;
    }

    struct addrinfo *result = NULL,
                    hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    status = getaddrinfo(SERVER_ADDRESS, DEFAULT_PORT, &hints, &result);
    if (status != 0)
    {
        error_verification("getaddrinfo failed", status);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    ConnectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    if (ConnectSocket == INVALID_SOCKET)
    {
        error_verification("socket failed", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Connect to server
    status = connect(ConnectSocket, result->ai_addr, (int)result->ai_addrlen);
    if (status == SOCKET_ERROR)
    {
        error_verification("connect failed", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    // Send data
    status = send(ConnectSocket, sendbuffer, sendbufferlen, 0);
    if (status == SOCKET_ERROR)
    {
        error_verification("send failed", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    printf("Bytes Sent: %ld\n", status);

    // shutdown the connection since no more data will be sent
    status = shutdown(ConnectSocket, SD_SEND);
    if (status == SOCKET_ERROR)
    {
        error_verification("shutdown failed", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // Receive data until the server closes the connection
    do
    {
        status = recv(ConnectSocket, recvbuffer, recvbufferlen, 0);
        if (status > 0)
        {
            printf("Bytes received: %d\n", status);
        }
        else if (status == 0)
        {
            printf("Connection closed\n");
        }
        else
        {
            error_verification("recv failed", WSAGetLastError());
        }
    } while (status > 0);

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}
