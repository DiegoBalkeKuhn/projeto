/*
 * sipalg_detector_client.c
 * Detector de SIP ALG para Windows
 * 
 * Compilar: gcc -o sipalg_detector_win32.exe sipalg_detector2.c -lws2_32
 * Usar: ./sipalg_detector_win32.exe 177.10.167.148 5060
 */

#include "sipalg.h"
#include <locale.h>

int main(int argc, char *argv[]) {

    SetConsoleOutputCP(65001);

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
