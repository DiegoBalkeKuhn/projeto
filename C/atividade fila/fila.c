#include "fila.h"

nodo_t *alocarNodo() {
    
    nodo_t *nodo = (nodo_t *) malloc(sizeof(nodo_t));
    
    if (nodo == NULL) {
        return NULL;
    }
    
    nodo->valor = NULL;
    nodo->prox = NULL;
    
    return nodo;
}

//Importante lembrar que o valor do nodo também é um ponteiro e alocado dinâmicamente

void colocarValorNodo(nodo_t *nodo, tipo_t tipo, void *valor) {
    
    nodo->tipo = tipo;
    
    switch (tipo) {
        case INT: {
            nodo->valor = malloc(sizeof(int));
            *(int *)nodo->valor = *(int *)valor;
            break;
        }
        case CHAR: {
            nodo->valor = malloc(sizeof(char));
            *(char *)nodo->valor = *(char *)valor;
            break;
        }
        case FLOAT: {
            nodo->valor = malloc(sizeof(float));
            *(float *)nodo->valor = *(float *)valor;
            break;
        }
    }
}

void inicializar(fila_t *lista) {
    
    lista->inicio = NULL;
    
}

int vazia(fila_t *lista) { 
    
    return lista->inicio == NULL; //underflow se == NULL
    
}


void mostrar(fila_t *fila) {
    
    nodo_t *aux = fila->inicio;
    
    if (vazia(fila)) {
         printf("Fila Vazia");
         return;
    }
    
    printf("Fila:\n ");
    while (aux != NULL) {
        switch (aux->tipo) {
            case INT:
                printf("%d ", *(int *)aux->valor);
                break;
            case CHAR:
                printf("%c ", *(char *)aux->valor);
                break;
            case FLOAT:
                printf("%.2f ", *(float *)aux->valor);
                break;
        }
        aux = aux->prox;
    }
    printf("\n");
}

int tamanho(fila_t *fila) {
    
    int count = 0;
    nodo_t *aux = fila->inicio;
    
    while (aux != NULL) {
        count++;
        aux = aux->prox;
    }
    
    return count;
}

void limpar(fila_t *fila) {
    
    nodo_t *aux = fila->inicio;
    
    while (aux != NULL) {
        nodo_t *temp = aux->prox;
        free(aux->valor);
        free(aux);
        aux = temp;
    }
    
    fila->inicio = NULL;
}

int inserir(fila_t *fila, tipo_t tipo, void *valor) {
    
    nodo_t *novoNodo = alocarNodo();
    
    colocarValorNodo(novoNodo, tipo, valor);
    
    if (vazia(fila)) {
        fila->inicio = novoNodo;
    } else {
        nodo_t *aux = fila->inicio;
        while (aux->prox != NULL) {
            aux = aux->prox;
        }
        aux->prox = novoNodo;
    }
    
    return 1;
}

int retirar(fila_t *fila, tipo_t tipo, void *valor) {
    
    if (vazia(fila)) {
        return 0;
    }
    
    nodo_t *temp = fila->inicio;
    *tipo = temp->tipo;

    switch (temp->tipo) {
        case INT:
            *(int *)valor = *(int *)temp->valor;
            break;
        case CHAR:
            *(char *)valor = *(char *)temp->valor;
            break;
        case FLOAT:
            *(float *)valor = *(float *)temp->valor;
            break;
    }
    
    fila->inicio = fila->inicio->prox;
    free(temp->valor);
    free(temp);
    
    return 1;
}
