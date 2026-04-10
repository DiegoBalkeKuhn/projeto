/*
 * sipalg_detector_client.c
 * Detector de SIP ALG para Windows
 * 
 * Compilar: gcc -o sipalg_detector_win32.exe sipalg_detector2.c -lws2_32
 * Usar: ./sipalg_detector_win32.exe 177.10.167.148 5060
 */

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")

#define BUFFER_SIZE 4096
#define TIMEOUT_MS 5000
#define TEST_REPEATS 5

typedef struct {
    char target_ip[16];
    int target_port;
    int verbose;
} config_t;

// Protótipos
int init_winsock(void);
SOCKET create_socket(void);
void print_hex(const char *data, int len);
int send_and_compare(SOCKET sock, struct sockaddr_in *target, 
                     const char *sip_msg, const char *test_name);
void generate_sip_register(char *buf, size_t size, const char *call_id, 
                           const char *local_ip, int local_port);
void generate_sip_invite(char *buf, size_t size, const char *call_id,
                          const char *local_ip, int local_port, int *rtp_port);
int compare_packets(const char *sent, int sent_len, const char *recv, int recv_len);

int init_winsock(void) {
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData);
}

SOCKET create_socket(void) {
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) return INVALID_SOCKET;
    
    DWORD timeout = TIMEOUT_MS;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
    
    return sock;
}

void print_hex(const char *data, int len) {
    printf("    Hex dump (%d bytes):\n    ", len);
    for (int i = 0; i < len && i < 64; i++) {
        printf("%02x ", (unsigned char)data[i]);
        if ((i + 1) % 16 == 0) printf("\n    ");
    }
    if (len > 64) printf("... (%d more bytes)", len - 64);
    printf("\n");
}

void generate_sip_register(char *buf, size_t size, const char *call_id,
                           const char *local_ip, int local_port) {
    char branch[64];
    snprintf(branch, sizeof(branch), "z9hG4bK%08x", rand());
    
    snprintf(buf, size,
        "REGISTER sip:test@example.com SIP/2.0\r\n"
        "Via: SIP/2.0/UDP %s:%d;branch=%s;rport\r\n"
        "From: <sip:algtest@example.com>;tag=%08x\r\n"
        "To: <sip:algtest@example.com>\r\n"
        "Call-ID: %s\r\n"
        "CSeq: 1 REGISTER\r\n"
        "Contact: <sip:algtest@%s:%d>;expires=300\r\n"
        "Max-Forwards: 70\r\n"
        "User-Agent: SIP-ALG-Detector/3.0\r\n"
        "Content-Length: 0\r\n\r\n",
        local_ip, local_port, branch, rand(), call_id, local_ip, local_port);
}

void generate_sip_invite(char *buf, size_t size, const char *call_id,
                          const char *local_ip, int local_port, int *rtp_port) {
    char branch[64];
    char sdp[1024];
    snprintf(branch, sizeof(branch), "z9hG4bK%08x", rand());
    
    *rtp_port = local_port + 2;
    
    snprintf(sdp, sizeof(sdp),
        "v=0\r\n"
        "o=- %ld 0 IN IP4 %s\r\n"
        "s=SIP ALG Test Call\r\n"
        "c=IN IP4 %s\r\n"
        "t=0 0\r\n"
        "m=audio %d RTP/AVP 0 8 101\r\n"
        "a=rtpmap:0 PCMU/8000\r\n"
        "a=rtpmap:8 PCMA/8000\r\n"
        "a=rtpmap:101 telephone-event/8000\r\n"
        "a=sendrecv\r\n",
        time(NULL), local_ip, local_ip, *rtp_port);
    
    snprintf(buf, size,
        "INVITE sip:echo@example.com SIP/2.0\r\n"
        "Via: SIP/2.0/UDP %s:%d;branch=%s;rport\r\n"
        "From: <sip:algtest@example.com>;tag=%08x\r\n"
        "To: <sip:echo@example.com>\r\n"
        "Call-ID: %s\r\n"
        "CSeq: 1 INVITE\r\n"
        "Contact: <sip:algtest@%s:%d>\r\n"
        "Content-Type: application/sdp\r\n"
        "Max-Forwards: 70\r\n"
        "User-Agent: SIP-ALG-Detector/3.0\r\n"
        "Content-Length: %d\r\n\r\n"
        "%s",
        local_ip, local_port, branch, rand(), call_id, local_ip, local_port,
        (int)strlen(sdp), sdp);
}

int compare_packets(const char *sent, int sent_len, const char *recv, int recv_len) {
    int differences = 0;
    
    printf("\n  Comparação de pacotes:\n");
    printf("    Enviado: %d bytes | Recebido: %d bytes\n", sent_len, recv_len);
    
    if (sent_len != recv_len) {
        printf("    [!] TAMANHO DIFERENTE! (diferença: %d bytes)\n", 
               abs(recv_len - sent_len));
        differences++;
    }
    
    // Comparação byte a byte (até o menor tamanho)
    int min_len = (sent_len < recv_len) ? sent_len : recv_len;
    int diff_bytes = 0;
    int first_diff = -1;
    
    for (int i = 0; i < min_len; i++) {
        if (sent[i] != recv[i]) {
            diff_bytes++;
            if (first_diff == -1) first_diff = i;
        }
    }
    
    if (diff_bytes > 0) {
        printf("    [!] %d bytes diferentes encontrados\n", diff_bytes);
        if (first_diff >= 0 && first_diff < min_len - 10) {
            printf("    Primeira diferença na posição %d:\n", first_diff);
            printf("      Enviado:  ");
            for (int i = first_diff; i < first_diff + 20 && i < min_len; i++) 
                printf("%02x ", (unsigned char)sent[i]);
            printf("\n      Recebido: ");
            for (int i = first_diff; i < first_diff + 20 && i < min_len; i++) 
                printf("%02x ", (unsigned char)recv[i]);
            printf("\n");
        }
        differences += diff_bytes;
    } else if (sent_len == recv_len) {
        printf("    [OK] Pacotes idênticos!\n");
    }
    
    // Análise específica de headers SIP
    printf("\n  Análise SIP:\n");
    
    // Procura Via header no recebido
    const char *via_sent = strstr(sent, "Via: SIP/2.0/UDP");
    const char *via_recv = strstr(recv, "Via: SIP/2.0/UDP");
    
    if (via_sent && via_recv) {
        // Extrai IP:porta do Via enviado
        char sent_ip[16], recv_ip[16];
        int sent_port, recv_port;
        
        if (sscanf(via_sent, "Via: SIP/2.0/UDP %15[0-9.]:%d", sent_ip, &sent_port) == 2) {
            printf("    Via enviado: %s:%d\n", sent_ip, sent_port);
        }
        
        // Procura por 'received=' no recebido (sinal de ALG)
        const char *received = strstr(recv, "received=");
        const char *rport = strstr(recv, "rport=");
        
        if (received) {
            char recv_param[16];
            if (sscanf(received, "received=%15s", recv_param) == 1) {
                printf("    [!] ALG DETECTADO: 'received=%s' adicionado!\n", recv_param);
                differences += 10; // Peso alto
            }
        }
        
        if (rport) {
            int rport_val;
            if (sscanf(rport, "rport=%d", &rport_val) == 1) {
                printf("    [!] ALG DETECTADO: 'rport=%d' adicionado/modificado!\n", rport_val);
                differences += 10;
            }
        }
    }
    
    // Verifica SDP
    const char *sdp_sent = strstr(sent, "Content-Type: application/sdp");
    const char *sdp_recv = strstr(recv, "Content-Type: application/sdp");
    
    if (sdp_sent && sdp_recv) {
        printf("    SDP presente em ambos\n");
        
        const char *c_sent = strstr(sent, "c=IN IP4");
        const char *c_recv = strstr(recv, "c=IN IP4");
        
        if (c_sent && c_recv) {
            char ip_sent[16], ip_recv[16];
            sscanf(c_sent, "c=IN IP4 %15s", ip_sent);
            sscanf(c_recv, "c=IN IP4 %15s", ip_recv);
            
            if (strcmp(ip_sent, ip_recv) != 0) {
                printf("    [CRÍTICO] IP SDP alterado: %s -> %s\n", ip_sent, ip_recv);
                differences += 20;
            } else {
                printf("    [OK] IP SDP preservado: %s\n", ip_sent);
            }
        }
        
        const char *m_sent = strstr(sent, "m=audio");
        const char *m_recv = strstr(recv, "m=audio");
        
        if (m_sent && m_recv) {
            int port_sent, port_recv;
            sscanf(m_sent, "m=audio %d", &port_sent);
            sscanf(m_recv, "m=audio %d", &port_recv);
            
            if (port_sent != port_recv) {
                printf("    [CRÍTICO] Porta RTP alterada: %d -> %d\n", port_sent, port_recv);
                differences += 20;
            } else {
                printf("    [OK] Porta RTP preservada: %d\n", port_sent);
            }
        }
    }
    
    return differences;
}

int send_and_compare(SOCKET sock, struct sockaddr_in *target,
                     const char *sip_msg, const char *test_name) {
    char recv_buffer[BUFFER_SIZE];
    int addr_len = sizeof(*target);
    int msg_len = (int)strlen(sip_msg);
    
    printf("\n[%s]\n", test_name);
    printf("Enviando %d bytes para %s:%d...\n", 
           msg_len, inet_ntoa(target->sin_addr), ntohs(target->sin_port));
    
    // Envia
    int sent = sendto(sock, sip_msg, msg_len, 0, 
                      (struct sockaddr *)target, sizeof(*target));
    if (sent != msg_len) {
        printf("  [ERRO] Envio falhou: %d\n", WSAGetLastError());
        return -1;
    }
    
    // Recebe resposta
    int recv_len = recvfrom(sock, recv_buffer, BUFFER_SIZE - 1, 0,
                            (struct sockaddr *)target, &addr_len);
    
    if (recv_len == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err == WSAETIMEDOUT) {
            printf("  [TIMEOUT] Nenhuma resposta em %dms\n", TIMEOUT_MS);
        } else {
            printf("  [ERRO] Recebimento falhou: %d\n", err);
        }
        return -1;
    }
    
    recv_buffer[recv_len] = '\0';
    printf("  Resposta recebida: %d bytes de %s:%d\n", 
           recv_len, inet_ntoa(target->sin_addr), ntohs(target->sin_port));
    
    // Compara
    return compare_packets(sip_msg, msg_len, recv_buffer, recv_len);
}

int main(int argc, char *argv[]) {
    config_t config = {
        .target_ip = "177.10.167.148",
        .target_port = 5060,
        .verbose = 0
    };
    
    // Parse argumentos
    if (argc > 1) strncpy(config.target_ip, argv[1], 15);
    if (argc > 2) config.target_port = atoi(argv[2]);
    if (argc > 3 && strcmp(argv[3], "-v") == 0) config.verbose = 1;
    
    printf("=====================================================\n");
    printf("  SIP ALG DETECTOR - Cliente Windows\n");
    printf("  Alvo: %s:%d\n", config.target_ip, config.target_port);
    printf("=====================================================\n\n");
    
    srand((unsigned int)time(NULL));
    
    if (init_winsock() != 0) {
        printf("Falha ao iniciar Winsock\n");
        return 1;
    }
    
    // Cria socket em porta aleatória (importante para detectar ALG)
    SOCKET sock = create_socket();
    if (sock == INVALID_SOCKET) {
        printf("Falha ao criar socket\n");
        WSACleanup();
        return 1;
    }
    
    // Obtém porta local usada
    struct sockaddr_in local_addr;
    int local_len = sizeof(local_addr);
    getsockname(sock, (struct sockaddr *)&local_addr, &local_len);
    
    char local_ip[16];
    strcpy(local_ip, inet_ntoa(local_addr.sin_addr));
    int local_port = ntohs(local_addr.sin_port);
    
    printf("Socket local: %s:%d\n\n", local_ip, local_port);
    
    // Configura alvo
    struct sockaddr_in target;
    memset(&target, 0, sizeof(target));
    target.sin_family = AF_INET;
    target.sin_port = htons((u_short)config.target_port);
    target.sin_addr.s_addr = inet_addr(config.target_ip);
    
    if (target.sin_addr.s_addr == INADDR_NONE) {
        printf("IP inválido: %s\n", config.target_ip);
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    
    printf("Iniciando testes contra %s:%d...\n\n", config.target_ip, config.target_port);
    
    int total_score = 0;
    int tests_run = 0;
    char sip_msg[4096];
    char call_id[64];
    
    // Teste 1: REGISTER simples
    snprintf(call_id, sizeof(call_id), "reg-%08x-%04x", rand(), rand());
    generate_sip_register(sip_msg, sizeof(sip_msg), call_id, local_ip, local_port);
    
    int score = send_and_compare(sock, &target, sip_msg, "TESTE 1: REGISTER");
    if (score >= 0) {
        total_score += score;
        tests_run++;
    }
    Sleep(500);
    
    // Teste 2: INVITE com SDP (onde ALG mais causa problemas)
    int rtp_port;
    snprintf(call_id, sizeof(call_id), "inv-%08x-%04x", rand(), rand());
    generate_sip_invite(sip_msg, sizeof(sip_msg), call_id, local_ip, local_port, &rtp_port);
    
    score = send_and_compare(sock, &target, sip_msg, "TESTE 2: INVITE com SDP");
    if (score >= 0) {
        total_score += score;
        tests_run++;
    }
    Sleep(500);
    
    // Teste 3: Repetição para verificar consistência
    printf("\n[TESTE 3: Consistência (%d repetições)]\n", TEST_REPEATS);
    int consistent = 1;
    int first_len = 0;
    
    for (int i = 0; i < TEST_REPEATS; i++) {
        snprintf(call_id, sizeof(call_id), "con-%08x-%04x", rand(), rand());
        generate_sip_register(sip_msg, sizeof(sip_msg), call_id, local_ip, local_port);
        
        sendto(sock, sip_msg, (int)strlen(sip_msg), 0, 
               (struct sockaddr *)&target, sizeof(target));
        
        char temp_buf[BUFFER_SIZE];
        int recv_len = recvfrom(sock, temp_buf, BUFFER_SIZE - 1, 0, NULL, NULL);
        
        if (recv_len > 0) {
            if (i == 0) first_len = recv_len;
            else if (abs(recv_len - first_len) > 10) consistent = 0;
            
            printf("  Tentativa %d: %d bytes%s\n", i + 1, recv_len,
                   (i > 0 && abs(recv_len - first_len) > 10) ? " [DIFERENTE]" : "");
        }
        Sleep(200);
    }
    
    if (!consistent) {
        printf("  [!] Respostas inconsistentes detectadas!\n");
        total_score += 5;
    } else {
        printf("  [OK] Respostas consistentes\n");
    }
    tests_run++;
    
    // Resultado final
    printf("\n=====================================================\n");
    printf("[RESULTADO FINAL]\n");
    printf("Testes executados: %d\n", tests_run);
    printf("Score de detecção: %d\n", total_score);
    printf("\n");
    
    if (total_score == 0) {
        printf("✓ NENHUM SIP ALG DETECTADO\n");
        printf("  Seu caminho de rede está limpo!\n");
    } else if (total_score < 10) {
        printf("⚠ SUSPEITA DE ALG OU NAT SIMÉTRICO\n");
        printf("  Pequenas modificações detectadas\n");
    } else if (total_score < 30) {
        printf("✗ SIP ALG PROVAVELMENTE ATIVO\n");
        printf("  Modificações significativas nos pacotes SIP\n");
    } else {
        printf("✗✗ SIP ALG CONFIRMADO\n");
        printf("  Alterações críticas em headers SDP e Via\n");
    }
    
    printf("\nRecomendações:\n");
    if (total_score > 0) {
        printf("  1. Desative SIP ALG no roteador (interface web/admin)\n");
        printf("  2. Use porta SIP alternativa (ex: 5062, 5070)\n");
        printf("  3. Configure STUN/TURN no cliente SIP\n");
    } else {
        printf("  • Sua conexão parece boa para VoIP\n");
        printf("  • Se houver problemas, verifique firewalls\n");
    }
    printf("=====================================================\n");
    
    closesocket(sock);
    WSACleanup();
    scanf("%*s"); // Pausa para leitura
    return 0;
}