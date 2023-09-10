#include "funcsServer.h"

// Inicializando quantidade de clientes conectados, modificados nas funções
int connected_clients = 0;

int main()
{
    int server_socket;
    struct sockaddr_in server_addr;
    char buffer[MAX_MSG_SIZE];
    struct Client clients[MAX_CLIENTS];

    server_socket = initialize_server(&server_addr);

    // Fazendo um while para o cliente ficar escutando os clientes e mandando mensagem quando necessário
    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);

        ssize_t bytes_received = recvfrom(server_socket, buffer, MAX_MSG_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);

        // Função para tratar erros na recepção de dados
        if (handle_receive_error(bytes_received) == -1)
        {
            exit(1);
        }

        buffer[bytes_received] = '\0';

        // Função para tratar mensagens recebidas
        handle_received_message(buffer, client_addr, clients, server_socket, addr_len);
    }

    close(server_socket);
    return 0;
}
