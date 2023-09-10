#include "funcsServer.h" // para garantir que a implementação está em conformidade com o cabeçalho

extern int connected_clients;
int admin_client_id = -1;

#define MAX_MESSAGE_SIZE 512
#define TIME_STR_SIZE 9

void msg(int server_socket, char *buffer, struct Client *clients, socklen_t addr_len, int idCliente)
{
    // Verifica se o cliente esta mutado, caso positivo ele não pode enviar a mensagem
    if (clients[idCliente].isMuted)
        return;

    // Pegando o tempo atual, para colocar na frente da mensagem
    time_t now;
    time(&now);
    struct tm *local = localtime(&now);

    char timeStr[TIME_STR_SIZE];
    if (strftime(timeStr, sizeof(timeStr), "%H:%M:%S", local) == 0)
    {
        perror("Error formating time");
        return;
    }

    char *timedMessage = (char *)malloc(MAX_MESSAGE_SIZE);
    if (timedMessage == NULL)
    {
        return;
    }
    snprintf(timedMessage, MAX_MESSAGE_SIZE, "[%s] %s", timeStr, buffer);

    // Lógica para enviar para todos os clientes conectados no servidor tal mensagem
    for (int i = 0; i < connected_clients; i++)
    {
        // Aqui que é feito o envio da mensagem com o uso da função sendto
        ssize_t bytes_sent = sendto(server_socket, timedMessage, strlen(timedMessage), 0,
                                    (struct sockaddr *)&clients[i].addr, addr_len);
        if (bytes_sent == -1)
        {
        }
    }

    free(timedMessage);
}

// Parte do código onde é feito o tratamento dos comandos de nosso servidor
void handleCommand(char *message, struct sockaddr_in client_addr, struct Client *clients, int server_socket, socklen_t addr_len, int idCliente, char *senderNickname)
{
    char buffer[MAX_MSG_SIZE];

    // Neste comando, é mostrado a quantidade de clientes presentes no momento no servidor
    if (strcmp(message, "!n_clients") == 0)
    {
        snprintf(buffer, sizeof(buffer), "Número de clientes no servidor: %i", connected_clients);
        sendto(server_socket, buffer, strlen(buffer), 0,
               (struct sockaddr *)&clients[idCliente].addr, addr_len);
    }

    // Se digitado users, mostra o nickname de todos presentes
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

    // Aqui, mostra o tempo do usuario que digitou no servidor
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

    // No !help, mostra todos comandos disponíveis
    else if (strcmp(message, "!help") == 0)
    {
        // Preparando uma única string para armazenar todas as informações de ajuda
        char helpMessage[1024] = "";

        // Concatenando todas as informações de ajuda
        strcat(helpMessage, "!n_clients: Número de clientes no Chat\n");
        strcat(helpMessage, "!users: Clientes online\n");
        strcat(helpMessage, "!time: Tempo que o cliente está utilizando o chat\n");
        strcat(helpMessage, "!mute <user>: Silenciar um usuário específico\n");
        strcat(helpMessage, "!unmute <user>: Desfaz o mute de um usuário específico\n");
        strcat(helpMessage, "!clear: Limpa o histórico de mensagens do chat para você\n");
        strcat(helpMessage, "!changename: Permite mudar o seu apelido\n");
        strcat(helpMessage, "!privateMsg <nomeCliente> <msg>: Envia uma mensagem privada para um usuário específico\n");

        // Enviando a mensagem completa de uma só vez
        send_message_to_client(server_socket, helpMessage, clients, addr_len, idCliente);
    }

    // Aqui um admin pode mutar outra pessoa
    else if (strncmp(message, "!mute ", 6) == 0)
    {
        // Verifica se é admin pois só ele pode mutar
        if (clients[idCliente].isAdmin == 1)
        {
            char targetUser[50];
            sscanf(message + 6, "%s", targetUser);
            for (int i = 0; i < connected_clients; i++)
            {
                if (strcmp(clients[i].nickname, targetUser) == 0)
                {
                    clients[i].isMuted = 1;
                    strcpy(clients[i].mutedBy, clients[idCliente].nickname);
                    snprintf(buffer, sizeof(buffer), "%s foi silenciado por %s", targetUser, clients[idCliente].nickname);
                    msg(server_socket, buffer, clients, addr_len, idCliente);
                    return;
                }
            }
        }
        else
        {
            char muteMsg[] =
                "Apenas o administrador pode mutar alguem";
            if (sendto(server_socket, muteMsg, strlen(muteMsg), 0,
                       (struct sockaddr *)&client_addr, addr_len) == -1)
            {
                perror("Error sending data to new client");
                exit(1);
            }
        }
    }

    // Semelhante aos anteriores, mas agora para desmutar alguém já mutado
    else if (strncmp(message, "!unmute ", 8) == 0)
    {
        if (clients[idCliente].isAdmin == 1)
        {
            char targetUser[50];
            sscanf(message + 8, "%s", targetUser);
            for (int i = 0; i < connected_clients; i++)
            {
                if (strcmp(clients[i].nickname, targetUser) == 0 && clients[i].isMuted)
                {
                    clients[i].isMuted = 0;
                    snprintf(buffer, sizeof(buffer), "%s foi dessilenciado por %s", targetUser, clients[idCliente].nickname);
                    msg(server_socket, buffer, clients, addr_len, idCliente);
                    return;
                }
            }
        }
        else
        {
            char unmuteMsg[] =
                "Apenas o administrador pode desmutar alguem";
            if (sendto(server_socket, unmuteMsg, strlen(unmuteMsg), 0,
                       (struct sockaddr *)&client_addr, addr_len) == -1)
            {
                perror("Error sending data to new client");
                exit(1);
            }
        }
    }

    // Limpando o terminal para deixar melhor a visualização, caso o cliente queira
    else if (strcmp(message, "!clear") == 0)
    {
        // Envia 50 linhas em branco para "limpar" o terminal do cliente
        for (int i = 0; i < 50; i++)
        {
            snprintf(buffer, sizeof(buffer), "\n");
            sendto(server_socket, buffer, strlen(buffer), 0,
                   (struct sockaddr *)&clients[idCliente].addr, addr_len);
        }
    }

    // Função de trocar o nickname de alguém caso seja necessário
    else if (strncmp(message, "!changename", 12) == 0)
    {
        snprintf(buffer, sizeof(buffer), "Digite seu novo nickname:");
        sendto(server_socket, buffer, strlen(buffer), 0,
               (struct sockaddr *)&clients[idCliente].addr, addr_len);
        char newName[50];
        int receivedLength = recvfrom(server_socket, newName, 50, 0, (struct sockaddr *)&client_addr, &addr_len);
        if (receivedLength > 0)
        {
            newName[receivedLength] = '\0'; // Certificando-se que a string é terminada com null

            /// Verificar se o novo nome já existe
            int isUnique = 1;
            for (int i = 0; i < connected_clients; i++)
            {
                if (strcmp(clients[i].nickname, newName) == 0)
                {
                    isUnique = 0;
                    break;
                }
            }

            // Se for único, atualiza o nome
            if (isUnique)
            {
                // Guarda o antigo nome para a mensagem de notificação
                char oldName[50];
                strcpy(oldName, clients[idCliente].nickname);

                // Atualiza o nome
                strcpy(clients[idCliente].nickname, newName);

                // Envia a confirmação para o cliente
                snprintf(buffer, sizeof(buffer), "Seu apelido foi alterado com sucesso para %s", newName);
                sendto(server_socket, buffer, strlen(buffer), 0,
                       (struct sockaddr *)&clients[idCliente].addr, addr_len);

                // Envia a notificação para todos os clientes
                snprintf(buffer, sizeof(buffer), "%s trocou nickname para %s", oldName, newName);
                msg(server_socket, buffer, clients, addr_len, idCliente);
            }
            else
            {
                // Envia a mensagem de erro para o cliente
                snprintf(buffer, sizeof(buffer), "O nome %s já está sendo usado. Por favor, escolha outro.", newName);
                sendto(server_socket, buffer, strlen(buffer), 0,
                       (struct sockaddr *)&clients[idCliente].addr, addr_len);
            }
        }
    }

    // Essa serve para mandar uma mensagem privada, falando o nome do usúario
    else if (strncmp(message, "!privateMsg ", 11) == 0)
    {
        char targetUser[50], privateMsg[500];
        sscanf(message + 11, "%s %[^\n]", targetUser, privateMsg);

        // Obter o tempo atual
        time_t now;
        time(&now);
        struct tm *local = localtime(&now);

        // Formatar o tempo
        char timeStr[9];
        strftime(timeStr, sizeof(timeStr), "%H:%M:%S", local);

        // Procura o cliente com o nome correspondente
        int targetClientID = -1;
        for (int i = 0; i < connected_clients; i++)
        {
            if (strcmp(clients[i].nickname, targetUser) == 0)
            {
                targetClientID = i;
                break;
            }
        }

        // Se encontrou, envia a mensagem
        if (targetClientID != -1)
        {
            int requiredSize = snprintf(NULL, 0, "[%s] [Privado de %s]: %s", timeStr, senderNickname, privateMsg);
            char timedPrivateMsg[requiredSize + 1];
            snprintf(timedPrivateMsg, sizeof(timedPrivateMsg), "[%s] [Privado de %s]: %s", timeStr, senderNickname, privateMsg);

            ssize_t bytes_sent = sendto(server_socket, timedPrivateMsg, strlen(timedPrivateMsg), 0,
                                        (struct sockaddr *)&clients[targetClientID].addr, addr_len);
            if (bytes_sent == -1)
            {
                perror("Error sending private message");
                exit(1);
            }
        }
        else
        {
            // Se não encontrou, envia uma mensagem de erro de volta para o cliente que tentou enviar
            snprintf(buffer, sizeof(buffer), "Usuário %s não encontrado.", targetUser);
            sendto(server_socket, buffer, strlen(buffer), 0,
                   (struct sockaddr *)&clients[idCliente].addr, addr_len);
        }
    }
}

// Função para mandar mensagem para um cliente específico, usando o sendto
void send_message_to_client(int server_socket, char *buffer, struct Client *clients, socklen_t addr_len, int idCliente)
{
    ssize_t bytes_sent = sendto(server_socket, buffer, strlen(buffer), 0, (struct sockaddr *)&clients[idCliente].addr, addr_len);
    if (bytes_sent == -1)
    {
        perror("Error sending data");
    }
}

// Mandando mensagem, caso seja necessário para todos os usuários
void sendMessage(int server_socket, const char *message, struct Client *clients,
                 socklen_t addr_len, const char *senderNickname,
                 int idCliente, int shouldBroadcast)
{
    char buffer_with_nickname[MAX_MSG_SIZE];

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
        msg(server_socket, buffer_with_nickname, clients, addr_len, idCliente);
    }
    else
    {
        sendto(server_socket, buffer_with_nickname, strlen(buffer_with_nickname), 0,
               (struct sockaddr *)&clients[idCliente].addr, addr_len);
    }
}

// Registrando um cliente, fazendo verificações se ele ja existe ou não
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
            if (admin_client_id == -1)
            {
                admin_client_id = connected_clients;
                clients[connected_clients].isAdmin = 1;
                admin_client_id = 0;
            }
            char defaultNickname[50];
            sprintf(defaultNickname, "client%d", connected_clients);
            gettimeofday(&clients[connected_clients].tempo, NULL);
            clients[connected_clients].addr = client_addr;
            clients[connected_clients].id = connected_clients + 1;
            strcpy(clients[connected_clients].nickname, defaultNickname);
            clients[connected_clients].hasSetNickname = 0;
            clients[connected_clients].isMuted = 0;
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

// Inicializando o server
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

// Caso tenha erro, é tratado aqui
int handle_receive_error(ssize_t bytes_received)
{
    if (bytes_received == -1)
    {
        perror("Error receiving data");
        return -1;
    }
    return 0;
}

// Tratando a mensagem recebida
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
            msg(server_socket, entryMessage, clients, addr_len, i);

            // Adicione esta parte para enviar a mensagem com o comando !help
            // A mensagem será enviada apenas para o cliente que acabou de entrar.
            char helpMsg[] = "Para acessar os comandos do chat, basta digitar !help\n";
            sendto(server_socket, helpMsg, strlen(helpMsg), 0,
                   (struct sockaddr *)&clients[i].addr, addr_len);

            break;
        }
    }

    // Primeira determinação do idCliente
    int idCliente = -1;
    for (int i = 0; i < connected_clients; i++)
    {
        if (clients[i].addr.sin_port == client_addr.sin_port)
        {
            idCliente = clients[i].id;
            break;
        }
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

    for (int i = 0; i < connected_clients; i++)
    {
        if (clients[i].addr.sin_port == client_addr.sin_port)
        {
            strcpy(senderNickname, clients[i].nickname);
            idCliente = i;
            break;
        }
    }

    // Em handle_received_message()
    if (buffer[0] == '!')
    {
        shouldBroadcast = 0; // Não transmitir para todos
        handleCommand(buffer, client_addr, clients, server_socket, addr_len, idCliente, senderNickname);

        return; // Retorne após processar o comando
    }

    if (shouldSendMessage)
    {
        sendMessage(server_socket, buffer, clients, addr_len, senderNickname, idCliente, shouldBroadcast);
    }

    // Coloque este loop no final da função handle_received_message
    for (int i = 0; i < connected_clients; i++)
    {
        printf("Índice do array: %d\n", i);
        printf("idCliente: %d\n", clients[i].id);
        printf("Porta: %d\n", ntohs(clients[i].addr.sin_port)); // ntohs() converte de network byte order para host byte order
        printf("Nickname: %s\n", clients[i].nickname);
        printf("hasSetNickname: %d\n", clients[i].hasSetNickname);
        printf("isMuted: %d\n", clients[i].isMuted);
        printf("mutedBy: %s\n", clients[i].mutedBy);
        printf("-------------\n");
    }
}

// Tratando caso o cliente queira sair
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