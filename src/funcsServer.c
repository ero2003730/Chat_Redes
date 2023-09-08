#include "funcsServer.h" // para garantir que a implementação está em conformidade com o cabeçalho

extern int connected_clients;

void msg(int server_socket, char *buffer, struct Client *clients, socklen_t addr_len)
{

    for (int i = 0; i < connected_clients; i++)
    {

        ssize_t bytes_sent = sendto(server_socket, buffer, strlen(buffer), 0,
                                    (struct sockaddr *)&clients[i].addr, addr_len);
        if (bytes_sent == -1)
        {
            perror("Error sending data");
            exit(1);
        }
    }
}

void sendMessage(int server_socket, const char *message, struct Client *clients,
                 socklen_t addr_len, const char *senderNickname,
                 int idCliente, int shouldBroadcast)
{
    char buffer_with_nickname[MAX_MSG_SIZE];
    char buffer[MAX_MSG_SIZE];

    if (senderNickname && strlen(senderNickname) > 0)
    {
        snprintf(buffer_with_nickname, sizeof(buffer_with_nickname), "%s: %s",
                 senderNickname, message);
    }
    else
    {
        strncpy(buffer_with_nickname, message, sizeof(buffer_with_nickname));
    }

    buffer_with_nickname[sizeof(buffer_with_nickname) - 1] = '\0';

    if (shouldBroadcast)
    {
        msg(server_socket, buffer_with_nickname, clients, addr_len);
    }
    else
    {
        sendto(server_socket, buffer_with_nickname, strlen(buffer_with_nickname), 0,
               (struct sockaddr *)&clients[idCliente].addr, addr_len);
    }

    if (strcmp(message, "!n_clients") == 0)
    {
        snprintf(buffer, sizeof(buffer), "Número de clientes no servidor: %i", connected_clients);
        sendto(server_socket, buffer, strlen(buffer), 0,
               (struct sockaddr *)&clients[idCliente].addr, addr_len);
    }
    else if (strcmp(message, "!users") == 0)
    {
        snprintf(buffer, sizeof(buffer), "Usuários online:");
        sendto(server_socket, buffer, strlen(buffer), 0,
               (struct sockaddr *)&clients[idCliente].addr, addr_len);

        for (int i = 0; i < connected_clients; i++)
        {
            snprintf(buffer, sizeof(buffer), "%s", clients[i].nickname);
            sendto(server_socket, buffer, strlen(buffer), 0,
                   (struct sockaddr *)&clients[idCliente].addr, addr_len);
        }
    }
    else if (strcmp(message, "!time") == 0)
    {
        struct timeval tempoAtual;
        gettimeofday(&tempoAtual, NULL);
        double tempo =
            (tempoAtual.tv_sec - clients[idCliente].tempo.tv_sec) +
            (double)(tempoAtual.tv_usec - clients[idCliente].tempo.tv_usec) / 1000000.0;
        snprintf(buffer, sizeof(buffer), "Tempo do(a) %s online: %.2f segundos", senderNickname, tempo);
        sendto(server_socket, buffer, strlen(buffer), 0,
               (struct sockaddr *)&clients[idCliente].addr, addr_len);
    }
    else if (strcmp(message, "!help") == 0)
    {
        snprintf(buffer, sizeof(buffer), "!n_clients: Número de clientes no Chat");
        sendto(server_socket, buffer, strlen(buffer), 0, (struct sockaddr *)&clients[idCliente].addr, addr_len);

        snprintf(buffer, sizeof(buffer), "!users: Clientes online");
        sendto(server_socket, buffer, strlen(buffer), 0, (struct sockaddr *)&clients[idCliente].addr, addr_len);

        snprintf(buffer, sizeof(buffer), "!time: Tempo que o cliente está utilizando o chat");
        sendto(server_socket, buffer, strlen(buffer), 0, (struct sockaddr *)&clients[idCliente].addr, addr_len);
    }
}

void register_new_client(struct sockaddr_in client_addr, struct Client *clients,
                         int server_socket, socklen_t addr_len)
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
            gettimeofday(&clients[connected_clients].tempo, NULL);
            clients[connected_clients].addr = client_addr;
            clients[connected_clients].id = connected_clients + 1;
            strcpy(clients[connected_clients].nickname, defaultNickname);
            clients[connected_clients].hasSetNickname = 0;
            connected_clients++;

            // Envia a pergunta para o cliente
            char welcomeMsg[] =
                "Bem-vindo ao chat! Como você gostaria de ser chamado?";
            if (sendto(server_socket, welcomeMsg, strlen(welcomeMsg), 0,
                       (struct sockaddr *)&client_addr, addr_len) == -1)
            {
                perror("Error sending data to new client");
                exit(1);
            }
        }
        else
        {
            printf("Limite máximo de clientes atingido.");
        }
    }
}

int initialize_server(struct sockaddr_in *server_addr)
{
    int server_socket;
    if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("Error creating socket");
        exit(1);
    }

    memset(server_addr, 0, sizeof(*server_addr));
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(PORT);
    server_addr->sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *)server_addr,
             sizeof(*server_addr)) == -1)
    {
        perror("Error binding");
        exit(1);
    }
    return server_socket;
}

int handle_receive_error(ssize_t bytes_received)
{
    if (bytes_received == -1)
    {
        perror("Error receiving data");
        return -1;
    }
    return 0;
}

void handle_received_message(char *buffer, struct sockaddr_in client_addr, struct Client *clients, int server_socket, socklen_t addr_len)
{
    int shouldSendMessage = 1;
    int shouldBroadcast = 1; // Adicionei esta variável aqui

    if (strcmp(buffer, "PING") == 0)
    {
        register_new_client(client_addr, clients, server_socket, addr_len);
        return;
    }

    for (int i = 0; i < connected_clients; i++)
    {
        if (clients[i].addr.sin_port == client_addr.sin_port && !clients[i].hasSetNickname)
        {
            strcpy(clients[i].nickname, buffer);
            clients[i].hasSetNickname = 1;
            shouldSendMessage = 0;
            printf("Setting nickname to: %s\n", clients[i].nickname);

            // Enviar mensagem de entrada aqui
            char entryMessage[MAX_MSG_SIZE];
            snprintf(entryMessage, sizeof(entryMessage), "%s entrou no chat", clients[i].nickname);
            msg(server_socket, entryMessage, clients, addr_len);

            // Adicione esta parte para enviar a mensagem com o comando !help
            // A mensagem será enviada apenas para o cliente que acabou de entrar.
            char helpMsg[] = "Para acessar os comandos do chat, basta digitar !help\n";
            sendto(server_socket, helpMsg, strlen(helpMsg), 0,
                   (struct sockaddr *)&clients[i].addr, addr_len);

            break;
        }
    }

    if (buffer[0] == '!') // Verifique se a mensagem começa com '!'
    {
        shouldBroadcast = 0; // Não transmitir para todos
    }

    printf("Received message from %s: %s\n", inet_ntoa(client_addr.sin_addr), buffer);

    if (strcmp(buffer, "exit") == 0)
    {
        handle_client_exit(client_addr, clients, server_socket, addr_len);
        return;
    }

    // Se o cliente não existe, adiciona-o à lista
    int client_exists = 0;
    for (int i = 0; i < connected_clients; i++)
    {
        if (clients[i].addr.sin_port == client_addr.sin_port)
        {
            client_exists = 1;
            break;
        }
    }

    if (!client_exists)
    {
        register_new_client(client_addr, clients, server_socket, addr_len);
    }

    char senderNickname[50] = "";
    int idCliente;
    for (int i = 0; i < connected_clients; i++)
    {
        if (clients[i].addr.sin_port == client_addr.sin_port)
        {
            strcpy(senderNickname, clients[i].nickname);
            idCliente = i;
            break;
        }
    }

    if (shouldSendMessage)
    {
        sendMessage(server_socket, buffer, clients, addr_len, senderNickname, idCliente, shouldBroadcast);
    }
}

void handle_client_exit(struct sockaddr_in client_addr, struct Client *clients, int server_socket, socklen_t addr_len)
{
    char exitMessage[MAX_MSG_SIZE];
    printf("Exiting client %d...\n", client_addr.sin_port);

    int clientIndex = -1;

    // Procurar o cliente que está saindo
    for (int i = 0; i < connected_clients; i++)
    {
        if (clients[i].addr.sin_port == client_addr.sin_port)
        {
            clientIndex = i;
            break;
        }
    }

    if (clientIndex == -1)
    {
        // Cliente não encontrado, então simplesmente retorne
        return;
    }

    // Construir a mensagem de saída
    snprintf(exitMessage, sizeof(exitMessage), "%s saiu do Chat", clients[clientIndex].nickname);

    // Remover o cliente da lista de clientes conectados
    for (int j = clientIndex; j < connected_clients - 1; j++)
    {
        clients[j] = clients[j + 1];
    }
    connected_clients--;

    // Enviar a mensagem de saída para todos os clientes restantes
    for (int i = 0; i < connected_clients; i++)
    {
        sendto(server_socket, exitMessage, strlen(exitMessage), 0,
               (struct sockaddr *)&clients[i].addr, addr_len);
    }
}
