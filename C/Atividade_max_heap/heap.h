#include <stdio.h>
#include <stdlib.h>

typedef struct arvore {
  int id;
  int valor;
  struct arvore *esquerda;
  struct arvore *direita;
} arvore;

arvore* criar_arvore(int id, int valor) {
  arvore* nova_arvore = (arvore*)malloc(sizeof(arvore));
  nova_arvore->id = id;
  nova_arvore->valor = valor;
  nova_arvore->esquerda = NULL;
  nova_arvore->direita = NULL;
  return nova_arvore;
}

void inserir(arvore** raiz, int id, int valor) {
  if (*raiz == NULL) {
    *raiz = criar_arvore(id, valor);
    return;
  }

  if (valor > (*raiz)->valor) {
    inserir(&(*raiz)->esquerda, id, valor);
  } else {
    inserir(&(*raiz)->direita, id, valor);
  }
}

void imprimir_todos(arvore* raiz) {
  if (raiz != NULL) {
    printf("ID: %d, Valor: %d\n", raiz->id, raiz->valor);
    imprimir_todos(raiz->esquerda);
    imprimir_todos(raiz->direita);
  }
}

void imprimir_maior_prioridade(arvore* raiz) {
  if (raiz != NULL) {
    printf("ID: %d, Valor: %d\n", raiz->id, raiz->valor);
  }
}

void liberar_arvore(arvore* raiz) {
  if (raiz != NULL) {
    liberar_arvore(raiz->esquerda);
    liberar_arvore(raiz->direita);
    free(raiz);
  }
}
void organizar_arvore(arvore** raiz) {
  if (*raiz == NULL) {
    return;
  }

  organizar_arvore(&(*raiz)->esquerda);
  organizar_arvore(&(*raiz)->direita);

  if ((*raiz)->esquerda != NULL && (*raiz)->esquerda->valor > (*raiz)->valor) {
    arvore temp_arvore = *(*raiz);
    (*raiz)->id = (*raiz)->esquerda->id;
    (*raiz)->valor = (*raiz)->esquerda->valor;
    (*raiz)->esquerda->id = temp_arvore.id;
    (*raiz)->esquerda->valor = temp_arvore.valor;
  }

  if ((*raiz)->direita != NULL && (*raiz)->direita->valor > (*raiz)->valor) {
    arvore temp_arvore = *(*raiz);
    (*raiz)->id = (*raiz)->direita->id;
    (*raiz)->valor = (*raiz)->direita->valor;
    (*raiz)->direita->id = temp_arvore.id;
    (*raiz)->direita->valor = temp_arvore.valor;
    (*raiz)->esquerda->valor = temp_arvore.valor;
  }

  if ((*raiz)->direita != NULL && (*raiz)->direita->valor > (*raiz)->valor) {
    arvore temp_arvore = *(*raiz);
    (*raiz)->id = (*raiz)->direita->id;
    (*raiz)->valor = (*raiz)->direita->valor;
    (*raiz)->direita->id = temp_arvore.id;
    (*raiz)->direita->valor = temp_arvore.valor;
  }
}

