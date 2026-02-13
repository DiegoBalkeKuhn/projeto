#include "fila.h"
#include <ctype.h>

void exibirMenu() {
    printf("\nMENU da fila FIFO!\n");
    printf("1 - Inserir\n");
    printf("2 - Retirar\n");
    printf("3 - Mostrar fila\n");
    printf("4 - Mostrar tamanho\n");
    printf("5 - Limpar fila\n");
    printf("0 - Sair\n");
    printf("Escolha uma opcao: ");
}

tipo_t selecionarTipo() {
    
    int opcao;
    
    printf("\nSelecione o tipo do dado:\n");
    printf("1 - INT\n");
    printf("2 - CHAR\n");
    printf("3 - FLOAT\n");
    printf("Tipo: ");
    scanf("%d", &opcao);

    switch (opcao) {
        case 1: return INT;
        case 2: return CHAR;
        case 3: return FLOAT;
        default:
            printf("Tipo invalido, assumindo INT!\n");
            return INT;
    }
    
}

int main() {
    
    int opcao;
    fila_t fila;
    
    inicializar(&fila);

    do {
        exibirMenu();
        scanf("%d", &opcao);
        getchar(); // limpar buffer

        switch (opcao) {
            
            case 1: {
                tipo_t tipo = selecionarTipo();
                void *valor;
                int i; char c; float f;

                switch (tipo) {
                    case INT:
                        printf("Digite um inteiro: ");
                        scanf("%d", &i);
                        valor = &i;
                        break;
                    case CHAR:
                        printf("Digite um caractere: ");
                        scanf(" %c", &c);
                        valor = &c;
                        break;
                    case FLOAT:
                        printf("Digite um numero real: ");
                        scanf("%f", &f);
                        valor = &f;
                        break;
                }
                if (inserir(&fila, tipo, valor))
                printf("Elemento inserido no inicio!\n");
                break;
            }

            
            case 2: {
                
                if (vazia(&fila)) {
                    printf("A fila esta vazia!\n");
                    break;
                }

                retirar(&fila, fila.inicio->tipo, fila.inicio->valor);
                switch (fila.inicio->tipo) {
                    case INT:
                        printf("Valor retirado: %d\n", *(int *)fila.inicio->valor);
                        break;
                    case CHAR:
                        printf("Valor retirado: %c\n", *(char *)fila.inicio->valor);
                        break;
                    case FLOAT:
                        printf("Valor retirado: %.2f\n", *(float *)fila.inicio->valor);
                        break;
                }
                printf("Elemento retirado do inicio!\n");
                
                break;
            }

            case 3:
                mostrar(&fila);
                break;

            case 4:
                printf("Tamanho da fila: %d\n", tamanho(&fila));
                break;

            case 5:
                limpar(&fila);
                printf("Fila limpa!\n");
                break;

            case 0:
                printf("Encerrando o programa...\n");
                break;

            default:
                printf("Opcao invalida!\n");
        }

    } while (opcao != 0);

    limpar(&fila);
    return 0;
}
