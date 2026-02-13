#pragma once
#ifndef FILA_H
#define FILA_H

#include <stdio.h>
#include <stdlib.h>

typedef enum tipo {
    
    INT,
    CHAR,
    FLOAT
    
} tipo_t;

// Estrutura de nodo da fila por lista encadeada simples
typedef struct nodo{
    
    tipo_t          tipo;    // tipo do valor armazenado
    void            *valor;  // ponteiro genérico
    struct nodo     *prox;   // ponteiro para o próximo nodo
    
} nodo_t;

// Estrutura principal da lista
typedef struct fila {
    
    nodo_t *inicio;
    
} fila_t;

// Interface da lista
nodo_t *alocarNodo          ();
void    colocarValorNodo    (nodo_t *nodo, tipo_t tipo, void *valor);

void    inicializar         (fila_t *fila);
int     vazia               (fila_t *fila);
int     inserir             (fila_t *fila, tipo_t tipo, void *valor); //enQueue
int     retirar             (fila_t *fila, tipo_t tipo, void *valor); //deQueue
void    mostrar             (fila_t *fila);
int     tamanho             (fila_t *fila);
void    limpar              (fila_t *fila);

#endif