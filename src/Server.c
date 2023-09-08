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
    int hasSetNickname; // 0 = false, 1 = true
};

void sendMessage(int server_socket, const char *message, struct Client *clients, socklen_t addr_len, const char *senderNickname, struct sockaddr_in original_sender);
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

        int shouldSendMessage = 1;

        for (int i = 0; i < connected_clients; i++)
        {
            if (clients[i].addr.sin_port == client_addr.sin_port && !clients[i].hasSetNickname)
            {
                strcpy(clients[i].nickname, buffer);
                clients[i].hasSetNickname = 1;
                shouldSendMessage = 0;
                printf("Setting nickname to: %s\n", clients[i].nickname);
                break;
            }
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

        // Buscar o nickname uma única vez
        char senderNickname[50] = "";
        for (int i = 0; i < connected_clients; i++)
        {
            if (clients[i].addr.sin_port == client_addr.sin_port)
            {
                strcpy(senderNickname, clients[i].nickname);
                break;
            }
        }

        if (shouldSendMessage)
        {
            sendMessage(server_socket, buffer, clients, addr_len, senderNickname, client_addr);
        }

        if (strcmp(buffer, "!n_clientes\n") == 0)
        {
            printf("Clientes no servidor: %i\n", connected_clients);
        }

        if (strcmp(buffer, "!n_clientes\n") == 0)
        {
            printf("Clientes no servidor: %i\n", connected_clients);
        }
    }

    close(server_socket);
    return 0;
}

void sendMessage(int server_socket, const char *message, struct Client *clients, socklen_t addr_len, const char *senderNickname, struct sockaddr_in original_sender)
{
    char buffer_with_nickname[MAX_MSG_SIZE];
    for (int i = 0; i < connected_clients; i++)
    {
        if (senderNickname && strlen(senderNickname) > 0)
        {
            if (clients[i].addr.sin_port == original_sender.sin_port) // É o mesmo cliente que enviou a mensagem
            {
                snprintf(buffer_with_nickname, sizeof(buffer_with_nickname), "%s: %s", senderNickname, message);
            }
            else // Outros clientes
            {
                snprintf(buffer_with_nickname, sizeof(buffer_with_nickname), "%s: %s", senderNickname, message);
            }
        }
        else
        {
            strncpy(buffer_with_nickname, message, sizeof(buffer_with_nickname));
        }
        buffer_with_nickname[sizeof(buffer_with_nickname) - 1] = '\0'; // Certifique-se de que é NULL-terminado

        ssize_t bytes_sent = sendto(server_socket, buffer_with_nickname, strlen(buffer_with_nickname), 0, (struct sockaddr *)&clients[i].addr, addr_len);
        if (bytes_sent == -1)
        {
            perror("Error sending data");
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
            clients[connected_clients].hasSetNickname = 0;
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
