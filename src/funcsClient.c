#include "funcsClient.h"

void initialize_socket(int *client_socket)
{
    if ((*client_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("Error creating socket");
        exit(1);
    }
}

void initialize_server_address(struct sockaddr_in *server_addr)
{
    memset(server_addr, 0, sizeof(*server_addr));
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr->sin_addr) <= 0)
    {
        perror("Error converting IP address");
        exit(1);
    }
}

void send_initial_message(int client_socket, struct sockaddr_in server_addr)
{
    char initialMsg[] = "PING";
    if (sendto(client_socket, initialMsg, strlen(initialMsg), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Error sending initial data");
        exit(1);
    }
}

void handle_client(int client_socket, struct sockaddr_in server_addr, struct sockaddr_in return_addr)
{
    fd_set read_fds;
    struct timeval timeout;
    char buffer[MAX_MSG_SIZE];
    socklen_t addr_len = sizeof(return_addr);

    while (1)
    {
        FD_ZERO(&read_fds);
        FD_SET(client_socket, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        int activity = select(client_socket + 1, &read_fds, NULL, NULL, &timeout);

        if (activity < 0)
        {
            perror("select error");
            continue;
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds))
        {
            fgets(buffer, MAX_MSG_SIZE, stdin);

            ssize_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n')
            {
                buffer[len - 1] = '\0';
            }

            if (sendto(client_socket, buffer, strlen(buffer), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
            {
                perror("Error sending data");
                exit(1);
            }
        }

        if (FD_ISSET(client_socket, &read_fds))
        {
            ssize_t bytes_received = recvfrom(client_socket, buffer, MAX_MSG_SIZE, 0, (struct sockaddr *)&return_addr, &addr_len);

            if (bytes_received == -1)
            {
                perror("Error receiving data");
                exit(1);
            }

            buffer[bytes_received] = '\0';
            printf("%s\n", buffer);

            if (strcmp(buffer, "exit\n") == 0)
            {
                printf("Exiting...\n");
                break;
            }
        }
    }
}