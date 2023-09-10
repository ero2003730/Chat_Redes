#ifndef FUNCSCLIENT_H
#define FUNCSCLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h> // Dependendo do sistema, pode ser necessário

#define SERVER_IP "127.0.0.1"
#define PORT 5005
#define MAX_MSG_SIZE 1024

void initialize_socket(int *client_socket);
void initialize_server_address(struct sockaddr_in *server_addr);
void send_initial_message(int client_socket, struct sockaddr_in server_addr);
void handle_client(int client_socket, struct sockaddr_in server_addr, struct sockaddr_in return_addr);

#endif // FUNCSCLIENT_H