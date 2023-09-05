#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 5005
#define MAX_MSG_LEN 1024

int main(void) {
    int server_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buff[1024];

    if ((server_socket == socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }

    printf("Listening...");

    while (1) {
        memset(buff, 0, sizeof(buff));

        if(recvfrom(server_socket, buff, sizeof(buff), 0, (struct sockaddr*)&client_addr, &client_addr_len) == -1) {
            perror("Error receiving message");
            exit(EXIT_FAILURE);
        }

        printf("Client: %s", buff);
    }

    close(server_socket);

    return 0;
}