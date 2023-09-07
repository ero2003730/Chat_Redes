#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 5005
#define MAX_MSG_SIZE 1024
#define MAX_CLIENTS 10

struct Client
{
    struct sockaddr_in addr;
    int id;
};

int main()
{
    int server_socket;
    struct sockaddr_in server_addr;
    char buffer[MAX_MSG_SIZE];
    int connected_clients = 0;
    struct Client clients[MAX_CLIENTS];

    if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("Error creating socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Error binding");
        exit(1);
    }

    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        ssize_t bytes_received = recvfrom(server_socket, buffer, MAX_MSG_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);

        if (bytes_received == -1)
        {
            perror("Error receiving data");
            exit(1);
        }

        buffer[bytes_received] = '\0';
        printf("Received message from %s: %s\n", inet_ntoa(client_addr.sin_addr), buffer);

        // Enviar a mesma mensagem de volta para o cliente
        if (sendto(server_socket, buffer, strlen(buffer), 0, (struct sockaddr *)&client_addr, addr_len) == -1)
        {
            perror("Error sending data back to client");
            exit(1);
        }

        if (strcmp(buffer, "exit\n") == 0)
        {
            printf("Exiting client %d...\n", client_addr.sin_port);

            // Remove o cliente da lista
            for (int i = 0; i < connected_clients; i++)
            {
                if (clients[i].addr.sin_port == client_addr.sin_port)
                {
                    for (int j = i; j < connected_clients - 1; j++)
                    {
                        clients[j] = clients[j + 1];
                    }
                    connected_clients--;
                    break;
                }
            }
        }
        else
        {
            // Verifica se o cliente já existe na lista
            int client_exists = 0;
            for (int i = 0; i < connected_clients; i++)
            {
                if (clients[i].addr.sin_port == client_addr.sin_port)
                {
                    client_exists = 1;
                    break;
                }
            }

            // Se o cliente não existe, adiciona-o à lista
            if (!client_exists)
            {
                if (connected_clients < MAX_CLIENTS)
                {
                    clients[connected_clients].addr = client_addr;
                    clients[connected_clients].id = connected_clients + 1;
                    connected_clients++;
                }
                else
                {
                    printf("Limite máximo de clientes atingido.\n");
                }
            }
        }

        if (strcmp(buffer, "!n_clientes\n") == 0)
        {
            printf("Clientes no servidor: %i\n", connected_clients);
        }
    }

    close(server_socket);
    return 0;
}