#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include "mapa.h"

struct smapa
{
    int chave;
    int dados;
    Mapa *esq;
    Mapa *dir;
};

Mapa *cria()
{
    return NULL;
}

static Mapa *cria_no(int c, int d)
{
    Mapa *nn = (Mapa *)malloc(sizeof(Mapa));
    if (nn != NULL)
    {
        nn->esq = nn->dir = NULL;
        nn->chave = c;
        nn->dados = d;
    }
    return nn;
}

Mapa *insere(Mapa *m, int chave, int d)
{
    if (m == NULL)
        return NULL;
    if (m->esq == NULL && m->dir == NULL && m->chave == 0)
    {
        m->chave = chave;
        m->dados = d;
        return m;
    }
    if (chave < m->chave)
    {
        if (m->esq == NULL)
            m->esq = cria();
        return insere(m->esq, chave, d);
    }
    else
    {
        if (m->dir == NULL)
            m->dir = cria();
        return insere(m->dir, chave, d);
    }

    return cria_no(chave, d);
}

int busca(Mapa *m, int chave)
{
    while (m != NULL)
    {
        if (chave < m->chave)
            m = m->esq;
        else if (chave > m->chave)
            m = m->dir;
        else
            return m->dados; /* achou */
    }
    return INT_MIN;
}

void percorre(Mapa *m, funcp f)
{
    if (m == NULL)
        return;
    percorre(m->esq, f);
    f(m->chave);
    percorre(m->dir, f);
}

void destroi(Mapa *m)
{
    if (m == NULL)
        return;
    destroi(m->esq);
    destroi(m->dir);
    free(m);
}

int altura(Mapa *m)
{
    /* fazer */
    return 0;
}

void mostra(Mapa *m)
{
    printf("(");
    /* fazer */
    printf(")");
}