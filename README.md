# UDP Chat Application

## About

This is a UDP-based chat application where multiple clients can connect to a server and communicate with each other. The application is built using C and uses socket programming for networking.

## Features

- Multiple clients can connect to a chatroom
- Real-time messaging between clients
- A variety of chat commands for enhanced usability

## Chat Commands

Here are some of the commands that clients can use:

- `!n_clients`: Displays the number of clients in the chat.
- `!users`: Shows the usernames of all online clients.
- `!time`: Tells you how long you've been connected to the chat.
- `!mute <user>`: Mutes a specific user.
- `!unmute <user>`: Unmutes a specific user.
- `!clear`: Clears your chat history.
- `!changename`: Allows you to change your username.
- `!privateMsg <username> <message>`: Sends a private message to a specific user.

## How to Run

1. Compile all the C files.
    ```
    gcc Client.c funcsClient.c -o client
    gcc Server.c funcsServer.c -o server
    ```
   
2. Run the server.
    ```
    ./server
    ```
   
3. Run the client.
    ```
    ./client
    ```
   
4. Connect multiple clients and start chatting!

## Collaborators

- Enzo Reis de Oliveira - [GitHub](https://github.com/ero2003730)
- Davi Seiji Kawai dos Santos - [GitHub](https://github.com/DaviSeiji)
- Pedro Henrique de Oliveira Masteguin - [GitHub](https://github.com/P81000)

-----------------------------------------------------------------------------------------------
---

# Aplicação de Chat UDP

## Sobre

Esta é uma aplicação de chat baseada em UDP onde múltiplos clientes podem se conectar a um servidor e se comunicar entre si. A aplicação foi construída usando a linguagem de programação C e utiliza programação de soquetes para a rede.

## Funcionalidades

- Múltiplos clientes podem se conectar a uma sala de chat
- Mensagens em tempo real entre clientes
- Variedade de comandos de chat para melhor usabilidade

## Comandos de Chat

Aqui estão alguns dos comandos que os clientes podem usar:

- `!n_clients`: Exibe o número de clientes no chat.
- `!users`: Mostra os usernames de todos os clientes online.
- `!time`: Informa quanto tempo você está conectado ao chat.
- `!mute <usuário>`: Silencia um usuário específico.
- `!unmute <usuário>`: Retira o silenciamento de um usuário específico.
- `!clear`: Limpa o histórico de chat.
- `!changename`: Permite que você mude seu username.
- `!privateMsg <username> <mensagem>`: Envia uma mensagem privada para um usuário específico.

## Como Executar

1. Compile todos os arquivos C.
    ```
    gcc Client.c funcsClient.c -o client
    gcc Server.c funcsServer.c -o server
    ```
   
2. Execute o servidor.
    ```
    ./server
    ```
   
3. Execute o cliente.
    ```
    ./client
    ```
   
4. Conecte múltiplos clientes e comece a conversar!

## Colaboradores

- Enzo Reis de Oliveira - [GitHub](https://github.com/ero2003730)
- Davi Seiji Kawai dos Santos - [GitHub](https://github.com/DaviSeiji)
- Pedro Henrique de Oliveira Masteguin - [GitHub](https://github.com/P81000)

