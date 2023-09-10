#include "funcsClient.h"

int main()
{
    int client_socket;
    struct sockaddr_in server_addr, return_addr;

    initialize_socket(&client_socket);
    initialize_server_address(&server_addr);
    send_initial_message(client_socket, server_addr);
    handle_client(client_socket, server_addr, return_addr);

    close(client_socket);
    return 0;
}
