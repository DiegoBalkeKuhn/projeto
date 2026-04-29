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
int Server_test(int argc, char *argv[]);
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
            printf("  [!] ATENÇÃO: Timeouts em pacotes SIP costumam indicar que o SIP ALG do roteador\n");
            printf("      está descartando pacotes malformados ou modificados incorretamente.\n");
            return 15; // Retorna score positivo para timeout
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

int Server_test(int argc, char *argv[]) {
    int *target = argc > 1 ? atoi(argv[1]) : 0;
    char buffer[BUFFER_SIZE];

    sendto(argv[0], "server", strlen("server"), 0, (struct sockaddr *)target, sizeof(*target)) == 0 ? 1 : 0;

    recvfrom(argv[0], buffer, sizeof(buffer), 0, NULL, NULL) > 0 ? 1 : 0;

    return 0;
}