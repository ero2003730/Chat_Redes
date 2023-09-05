#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
#define PORT 5005
#define MAX_MSG_LEN 1024

int main(void) {
    int client_socket;
    struct sockaddr server_addr;
    char buff[MAX_MSG_LEN]

    if ((client_socket = socket(AF_INET, SOCK_DRAM, 0)) == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    while (1) {
        printf("Type a message or 'exit' to quit: ");
        fgets(buffer, sizeof(buff), stdin);

        size_t len = strlen(buff);
        if (len > 0 && buff[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        sendto(client_socket, buff, strlen(buff), 0, (struct socketaddr*)&server_addr, sizeof(server_addr));

        if (strcmp(buff, 'exit') == 0){
            break;
        }
    }  
    
    close(client_socket);

    return 0;
}