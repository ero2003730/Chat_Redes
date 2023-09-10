#ifndef FUNCSERVER_H
#define FUNCSERVER_H

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Definições de constantes e variáveis
#define PORT 5005
#define MAX_MSG_SIZE 1024
#define MAX_CLIENTS 10

struct Client
{
    struct sockaddr_in addr;
    int id;
    char nickname[50];
    int hasSetNickname;
    struct timeval tempo;
    int isMuted;
    char mutedBy[50];
};

extern int connected_clients;

void msg(int server_socket, char *buffer, struct Client *clients, socklen_t addr_len, int idCliente);
void sendMessage(int server_socket, const char *message, struct Client *clients,
                 socklen_t addr_len, const char *senderNickname,
                 int idCliente, int shouldBroadcast);
void register_new_client(struct sockaddr_in client_addr, struct Client *clients,
                         int server_socket, socklen_t addr_len);
int initialize_server(struct sockaddr_in *server_addr);
int handle_receive_error(ssize_t bytes_received);
void handle_received_message(char *buffer, struct sockaddr_in client_addr, struct Client *clients, int server_socket, socklen_t addr_len);
void handle_client_exit(struct sockaddr_in client_addr, struct Client *clients, int server_socket, socklen_t addr_len);
void handleCommand(char *message, struct sockaddr_in client_addr, struct Client *clients, int server_socket, socklen_t addr_len, int idCliente, char *senderNickname);

#endif // FUNCSERVER_H