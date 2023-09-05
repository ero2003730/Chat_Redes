// bibliotecas utilizadas
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFFER_SIZE 512     // tamanho buffer envio/recepcao
#define DEFAULT_PORT "8080" // porta de entrada da comunicacao

void error_verification(char error[], int erro)
{
    printf("%s: %d\n", error, erro); // imprime o erro se algo der errado
    WSACleanup();                    // limpa o uso de Winsock
}

int __cdecl main()
{
    WSADATA wsaData; // socket do windows
    int status;      // variavel de status

    SOCKET ListenSocket = INVALID_SOCKET; // socket que será utilizado pelo servidor para ouvir
    SOCKET ClientSocket = INVALID_SOCKET; // socket que irá guardar informações do cliente que conectou

    int resultVerification;          // verificao de erro, para as chamada de funcao
    char recvbuffer[BUFFER_SIZE];    // guardar tudo o que o cliente mandar
    int recvbufferlen = BUFFER_SIZE; // variavel que guardara o tamanho do buffer

    // inicializando o winsock
    status = WSAStartup(MAKEWORD(2, 2), &wsaData);

    // verificao de erro para inicializacao do socket
    if (status != 0)
    {
        error_verification("Falhou na inicializacao do socket", status);
        return 1;
    }

    char host[NI_MAXHOST];                     // declara uma variável para armazenar o nome do host com tamanho máximo definido por NI_MAXHOST
    gethostname(host, NI_MAXHOST);             // chama a função gethostname para obter o nome da máquina e armazena em 'host'
    printf("\n Nome do servidor: %s\n", host); // imprime o nome do servidor

    struct addrinfo hints, *res, *p; // declara variáveis de estrutura para armazenar informações sobre o endereço
    char ipstr[INET6_ADDRSTRLEN];    // declara um array de caracteres para armazenar o endereço IP como string

    memset(&hints, 0, sizeof hints); // Limpa a estrutura 'hints', definindo todos os bits para zero.
    hints.ai_family = AF_UNSPEC;     // Aceita qualquer família de endereços (IPv4 ou IPv6).
    hints.ai_socktype = SOCK_STREAM; // Define o tipo de socket para stream (geralmente usado para TCP).
    hints.ai_protocol = IPPROTO_TCP; // Especifica que o protocolo a ser usado é TCP.
    hints.ai_flags = AI_PASSIVE;     // Indica que o socket será usado para escutar, útil para configuração de servidor.

    // chama getaddrinfo para obter uma lista de endereços com base no nome do host
    if ((status = getaddrinfo(host, NULL, &hints, &res)) != 0)
    {
        error_verification("getaddrinfo:", status);
        return 1;
    }

    printf("Endereços IP para %s:\n", host); // imprime um cabeçalho para a lista de endereços IP

    // itera através da lista de endereços retornada por getaddrinfo
    for (p = res; p != NULL; p = p->ai_next)
    {
        void *addr; // declara um ponteiro void para armazenar o endereço IP

        // Verifica se o endereço é IPv4.
        if (p->ai_family == AF_INET)
        {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr; // faz um cast para sockaddr_in
            addr = &(ipv4->sin_addr);                                    // obtém o endereço IPv4
        }
        else
        {
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr; // faz um cast para sockaddr_in6
            addr = &(ipv6->sin6_addr);                                     // obtém o endereço IPv6
        }

        // converte o endereço IP para uma string e armazena em ipstr
        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);

        // imprime o endereço IP
        printf("  %s\n", ipstr);
    }

    // criando um socket para conexao com o servidor
    ListenSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol); // criando um socket para conexao com o servidor

    // verificao para ver se o socket foi criado com sucesso
    if (ListenSocket == INVALID_SOCKET)
    {

        error_verification("Socket falhou", WSAGetLastError());
        freeaddrinfo(res);
        return 1;
    }

    // configura o socket TCP do servidor
    status = bind(ListenSocket, res->ai_addr, (int)res->ai_addrlen);

    // verificacao para ver se a configuracao ocorreu corretamente
    if (status == SOCKET_ERROR)
    {
        error_verification("Bind falhou", WSAGetLastError());
        freeaddrinfo(res);
        closesocket(ListenSocket);
        return 1;
    }

    freeaddrinfo(res); // libera a memória alocada para a lista de endereços

    printf("Esperando...\n");

    // configura o socket 'ListenSocket' para começar a escutar por conexões de clientes
    status = listen(ListenSocket, SOMAXCONN);

    // verificando se a configuracao ocorreu corretamente
    if (status == SOCKET_ERROR)
    {
        error_verification("Listen falou", WSAGetLastError());
        closesocket(ListenSocket);
        return 1;
    }

    struct sockaddr_in clientAddr;           // estrutura para armazenar o endereço do cliente
    int clientAddrSize = sizeof(clientAddr); // tamanho da estrutura do cliente

    // quando o cliente tentar conectar, aceitar esse cliente
    ClientSocket = accept(ListenSocket, NULL, NULL);

    // verificacao para ver se aceitou corretamente
    if (ClientSocket == INVALID_SOCKET)
    {
        error_verification("Accept falou", WSAGetLastError());
        closesocket(ListenSocket);
        return 1;
    }

    // imprime o endereço IP do cliente aceito
    printf("Cliente aceito: %s\n", inet_ntoa(clientAddr.sin_addr));

    closesocket(ListenSocket);

    // inicia um loop para tratar da comunicação entre o servidor e o cliente
    do
    {
        // tenta receber dados do cliente utilizando o socket do cliente, os dados recebidos são armazenados em 'recvbuffer'.
        status = recv(ClientSocket, recvbuffer, recvbufferlen, 0);

        // verifica se a recepção foi bem-sucedida
        if (status > 0)
        {
            // se a função 'recv' retorna um valor maior que zero, indica que n bytes foram recebidos, imprime os dados recebidos.
            printf("Mensagem recebida: \"%s\"\n", recvbuffer);
        }
        // serifica se a conexão foi fechada pelo cliente
        else if (status == 0)
        {
            // se a função 'recv' retorna zero, indica que a conexão foi fechada pelo cliente
            printf("Fechando conexao...\n");
        }
        // trata casos de erro na recepção
        else
        {
            // se a função 'recv' retorna -1, um erro ocorreu
            error_verification("RecvBuffer falhou:", WSAGetLastError());

            // fecha o socket do cliente
            closesocket(ClientSocket);

            // encerra o programa
            return 1;
        }
    }
    // o loop continua enquanto a função 'recv' retornar um valor maior que zero (conexão ativa e dados sendo recebidos)
    while (status > 0);

    // encerrar a conexao
    status = shutdown(ClientSocket, SD_SEND);

    // verifica se foi encerrado com sucesso
    if (status == SOCKET_ERROR)
    {
        printf("Erro falhou", WSAGetLastError());
        closesocket(ClientSocket);
        return 1;
    }

    printf("Encerrando socket...\n");
    closesocket(ClientSocket);
    WSACleanup();

    return 0;
}