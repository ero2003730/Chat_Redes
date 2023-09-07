#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 5005
#define MAX_MSG_SIZE 1024

int main()
{
    int client_socket;
    struct sockaddr_in server_addr, return_addr;
    char buffer[MAX_MSG_SIZE];
    socklen_t addr_len = sizeof(return_addr);

    if ((client_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("Error creating socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0)
    {
        perror("Error converting IP address");
        exit(1);
    }

    while (1)
    {
        printf("Enter message or use 'exit' to quit: ");
        fgets(buffer, MAX_MSG_SIZE, stdin);

        if (sendto(client_socket, buffer, strlen(buffer), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
        {
            perror("Error sending data");
            exit(1);
        }

        ssize_t bytes_received = recvfrom(client_socket, buffer, MAX_MSG_SIZE, 0, (struct sockaddr *)&return_addr, &addr_len);

        if (bytes_received == -1)
        {
            perror("Error receiving data");
            exit(1);
        }

        buffer[bytes_received] = '\0';
        printf("Received from server: %s\n", buffer);

        if (strcmp(buffer, "exit\n") == 0)
        {
            printf("Exiting...\n");
            break;
        }
    }

    close(client_socket);
    return 0;
}
