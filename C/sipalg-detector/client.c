#define _WIN32_WINNT 0x0600
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 5060
#define BUFFER_SIZE 4096
#define TIMEOUT 5

void generate_random_string(char *s, int len) {
    const char charset[] =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for (int i = 0; i < len; i++)
        s[i] = charset[rand() % (sizeof(charset) - 1)];
    s[len] = '\0';
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Uso: %s <ip_servidor_echo>\n", argv[0]);
        return 1;
    }

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("Erro ao iniciar Winsock\n");
        return 1;
    }

    srand(time(NULL));

    SOCKET sockfd;
    struct sockaddr_in servaddr;
    char send_buffer[BUFFER_SIZE];
    char recv_buffer[BUFFER_SIZE];

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == INVALID_SOCKET) {
        printf("Erro ao criar socket\n");
        WSACleanup();
        return 1;
    }

    // Timeout de recepção
    DWORD timeout = TIMEOUT * 1000;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,
               (const char*)&timeout, sizeof(timeout));

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);

   servaddr.sin_addr.s_addr = inet_addr(argv[1]);

if (servaddr.sin_addr.s_addr == INADDR_NONE) {
    printf("IP inválido\n");
    closesocket(sockfd);
    WSACleanup();
    return 1;
}

    char token[32];
    generate_random_string(token, 16);

    snprintf(send_buffer, sizeof(send_buffer),
        "REGISTER sip:test SIP/2.0\r\n"
        "Via: SIP/2.0/UDP 10.0.0.1:12345;branch=z9hG4bK-%s\r\n"
        "From: <sip:test@10.0.0.1>\r\n"
        "To: <sip:test@10.0.0.1>\r\n"
        "Call-ID: %s@test\r\n"
        "CSeq: 1 REGISTER\r\n"
        "Contact: <sip:test@10.0.0.1:12345>\r\n"
        "Content-Length: 0\r\n\r\n",
        token, token);

    printf("Enviando pacote...\n");

    sendto(sockfd, send_buffer, strlen(send_buffer), 0,
           (struct sockaddr *)&servaddr, sizeof(servaddr));

    int addrlen = sizeof(servaddr);
    int n = recvfrom(sockfd, recv_buffer, BUFFER_SIZE, 0,
                     (struct sockaddr *)&servaddr, &addrlen);

    if (n == SOCKET_ERROR) {
        printf("Sem resposta ou timeout.\n");
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    recv_buffer[n] = '\0';

    printf("Comparando pacotes...\n");

    if (strcmp(send_buffer, recv_buffer) == 0) {
        printf("\nRESULTADO: Nenhum SIP ALG detectado.\n");
    } else {
        printf("\nRESULTADO: SIP ALG DETECTADO! (pacote foi modificado)\n");
        printf("\n--- ENVIADO ---\n%s", send_buffer);
        printf("\n--- RECEBIDO ---\n%s", recv_buffer);
    }

    closesocket(sockfd);
    WSACleanup();
    return 0;
}