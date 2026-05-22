#include "heap.h"

int main() {
  int opcao;
  arvore* raiz = NULL;
scanf("%d", &opcao);

while (opcao != 3) {

  printf("menu\n");
  printf("1 - Inserir\n");
  printf("2 - Imprimir maior prioridade\n");
  printf("3 - Sair\n");
  scanf("%d", &opcao);

  switch (opcao) {
    case 1:
      if(raiz == NULL) {
        int id, valor;
        printf("Digite o ID e o valor do elemento a ser inserido: ");
        scanf("%d %d", &id, &valor);
        raiz = criar_arvore(id, valor);
      } else {
        int id, valor;
        printf("Digite o ID e o valor do elemento a ser inserido: ");
        scanf("%d %d", &id, &valor);
        inserir(&raiz, id, valor);
      }
      break;
    case 2:
      printf("imprimir maior prioridade\n");
      imprimir_maior_prioridade(raiz);
      break;
    case 3:
      printf("sair\n");
      liberar_arvore(raiz);
      break;
    default:
      printf("opção inválida\n");
      break;
  }
}
  return 0;
}