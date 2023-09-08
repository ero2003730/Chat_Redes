#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 5005
#define MAX_MSG_SIZE 1024
#define MAX_CLIENTS 10

int connected_clients = 0;

struct Client
{
    struct sockaddr_in addr;
    int id;
    char nickname[50];
};

void sendMessage(int server_socket, char buffer[], struct Client *clients, socklen_t addr_len);
void register_new_client(struct sockaddr_in client_addr, struct Client *clients, int server_socket, socklen_t addr_len);

int main()
{
    int server_socket;
    struct sockaddr_in server_addr;
    char buffer[MAX_MSG_SIZE];

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

        if (strcmp(buffer, "PING") == 0)
        {
            register_new_client(client_addr, clients, server_socket, addr_len);
            continue;
        }

        printf("Received message from %s: %s\n", inet_ntoa(client_addr.sin_addr), buffer);

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
                register_new_client(client_addr, clients, server_socket, addr_len);
            }
        }

        sendMessage(server_socket, buffer, clients, addr_len);

        if (strcmp(buffer, "!n_clientes\n") == 0)
        {
            printf("Clientes no servidor: %i\n", connected_clients);
        }
    }

    close(server_socket);
    return 0;
}

void sendMessage(int server_socket, char buffer[], struct Client *clients, socklen_t addr_len)
{
    // Enviar a mesma mensagem de volta para o cliente
    for (int i = 0; i < connected_clients; i++)
    {
        char messageWithNickname[MAX_MSG_SIZE + 50]; // 50 para o nickname
        sprintf(messageWithNickname, "%s: %s", clients[i].nickname, buffer);

        if (sendto(server_socket, messageWithNickname, strlen(messageWithNickname), 0, (struct sockaddr *)&clients[i].addr, addr_len) == -1)
        {
            perror("Error sending data back to client");
            exit(1);
        }
    }
}

void register_new_client(struct sockaddr_in client_addr, struct Client *clients, int server_socket, socklen_t addr_len)
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
            char defaultNickname[50];
            sprintf(defaultNickname, "client%d", connected_clients);

            clients[connected_clients].addr = client_addr;
            clients[connected_clients].id = connected_clients + 1;
            strcpy(clients[connected_clients].nickname, defaultNickname);
            connected_clients++;

            // Envia a pergunta para o cliente
            char welcomeMsg[] = "Bem-vindo ao chat! Como você gostaria de ser chamado?";
            if (sendto(server_socket, welcomeMsg, strlen(welcomeMsg), 0, (struct sockaddr *)&client_addr, addr_len) == -1)
            {
                perror("Error sending data to new client");
                exit(1);
            }
        }
        else
        {
            printf("Limite máximo de clientes atingido.\n");
        }
    }
}
