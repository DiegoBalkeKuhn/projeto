#include "heap.h"

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

void imprimir(arvore* raiz) {
  if (raiz != NULL) {
    printf("ID: %d, Valor: %d\n", raiz->id, raiz->valor);
    imprimir(raiz->esquerda);
    imprimir(raiz->direita);
  }
}

void liberar_arvore(arvore* raiz) {
  if (raiz != NULL) {
    liberar_arvore(raiz->esquerda);
    liberar_arvore(raiz->direita);
    free(raiz);
  }
}

