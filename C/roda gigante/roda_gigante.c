#include <stdio.h>
#include <stdlib.h>
#include "roda_gigante.h"


int main() {
    int opcao = -1;
    int capacidade = 0, quantidade = 0, fila = 0;
    int maxVoltas = 3, autoDesembarqueVoltas = 0; // 0 desativado, 1 ativado

    rodaGigante_t *roda = criarRodaGigante();

    printf("--- Configuração da Roda Gigante ---\n");
    printf("Digite a capacidade de pessoas por carrinho: ");
    scanf("%d", &capacidade);
    printf("Digite a quantidade de carrinhos na roda: ");
    scanf("%d", &quantidade);
    printf("Digite a quantidade inicial de pessoas na fila: ");
    scanf("%d", &fila);

    for (int i = 0; i < quantidade; i++) {
        adicionarCarrinho(roda, capacidade);
    }
    
    roda->pessoasFila = fila;

    roda->atual = roda->inicio;
    if (roda->atual) {
       printf("\nRoda gigante configurada com %d carrinhos. O carrinho atual é o de ID %d.\n", roda->qtdCarrinhos, roda->atual->id);
    } else {
       printf("\nRoda gigante criada sem carrinhos.\n");
    }

    while (opcao != 0) {
        printf("\n--- Painel de Controle ---\n");
        printf("Carrinho atual: %d | Pessoas na Fila: %d | Total Atendidas: %d | Voltas para desembarque: %d\n", roda->atual ? roda->atual->id : 0, roda->pessoasFila, roda->totalAtendidas, maxVoltas);
        printf("1. Avançar para o próximo carrinho\n");
        printf("2. Embarcar pessoas no carrinho atual\n");
        printf("3. Desembarcar pessoas do carrinho atual\n");
        printf("4. Realizar um giro completo (contabiliza voltas)\n");
        printf("5. Ir para um carrinho específico\n");
        printf("6. Exibir relatório completo\n");
        printf("7. Avançar e desembarcar automaticamente (atualmente = %d) \n", autoDesembarqueVoltas);
        printf("8. Definir número de voltas para desembarque\n");
        printf("9. ativarmodo automatico\n");
        printf("0. Encerrar simulação\n");
        printf("Escolha uma opção: ");
        scanf("%d", &opcao);

        switch (opcao) {
            case 1:
                avancar(roda, maxVoltas, autoDesembarqueVoltas);
                break;
            case 2:
                embarcar_carrinho(roda);
                break;
            case 3:
                desembarcar(roda);
                break;
            case 4:
                giroCompleto(roda, maxVoltas, autoDesembarqueVoltas);
                break;
            case 5:
                escolherCarrinho(roda);
                break;
            case 6:
                relatorio(roda);
                break;
            case 7:
            if (autoDesembarqueVoltas == 0) {
                autoDesembarqueVoltas = 1;
                printf("embarque automático e desembarque automático ativado.\n");
            } else {
                autoDesembarqueVoltas = 0;
                printf("embarque automático e desembarque automático desativado.\n");
            }
                break;
            case 8:
                printf("Digite o novo número de voltas para desembarque: ");
                scanf("%d", &maxVoltas);
                break;
            case 9:
            {
                char continuar;
                printf("\nModo de giro automático ativado.\n");
                if (autoDesembarqueVoltas == 0) {
                    autoDesembarqueVoltas = 1;
                    printf("Desembarque automático foi ativado para o modo de giro automático.\n");
                }
                do {
                    embarcar_carrinho_auto(roda);
                    giroCompleto(roda, maxVoltas, autoDesembarqueVoltas);
                    relatorio(roda);

                    if (rodaVazia(roda)) {
                        printf("\nA roda gigante está vazia. Encerrando modo automático.\n");
                        continuar = 'n';
                    } else {
                        printf("\nDeseja continuar o giro automático? (s/n): ");
                        scanf(" %c", &continuar);
                    }
                } while (continuar == 's' || continuar == 'S');
                printf("Modo de giro automático desativado.\n");
                break;
            }
            case 0:
                printf("Encerrando a simulação...\n");
                break;
            default:
                printf("Opção inválida! Tente novamente.\n");
                break;
        }
    }

    liberarMemoria(roda);

    return 0;
}
