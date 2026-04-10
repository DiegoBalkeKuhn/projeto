#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include "mapa.h"

struct smapa {
  int chave;
  int conteudo;
  short int bf;
  struct smapa* esq;
  struct smapa* dir;
};

static Mapa* cria_no(int c, int novodado);
static Mapa* insere2(Mapa *m, int chave, int novodado, int *cresceu);
static Mapa* corrige_dir(Mapa *m);
static Mapa* corrige_esq(Mapa *m);
static Mapa* rotaciona_dir(Mapa *m);
static Mapa* rotaciona_esq(Mapa *m);

static Mapa* cria_no(int c, int novodado) {
  Mapa *m = (Mapa*)malloc(sizeof(Mapa));
  if (m != NULL) {
    m->esq = m->dir = NULL;
    m->chave = c;
    m->conteudo = novodado;
    m->bf = 0;
  }
  return m;
}

Mapa* cria() {
  return NULL;
}

Mapa* insere (Mapa* m, int chave, int novodado) {
  int cresceu = 0;
  return insere2(m, chave, novodado, &cresceu);
}

static Mapa* insere2(Mapa *m, int chave, int novodado, int *cresceu) {
  if (m == NULL) {
    m = cria_no(chave, novodado);
    *cresceu = 1;
  }
  else if (chave < m->chave) {
    /* COMPLETAR */
  }
  else if (chave > m->chave) {
    m->dir = insere2(m->dir, chave, novodado, cresceu);
    if(*cresceu) {
      switch (m->bf) {
        case 1: /* era 1, nao pode crescer a direita */
          m = corrige_dir(m);
          *cresceu = 0;
          break;
        case 0: /* estava equilibrada, pode aumentar de um lado  avisando para cima que cresceu*/
          m->bf = 1;
          break;
        case -1: /* ficou equilibrada, altura ficou a mesma*/
          m->bf = 0;
          *cresceu = 0;
          break;
      }
    }
  }
  return m;
}

static Mapa* corrige_esq(Mapa *m) {
  /* FAZER */
  return NULL;
}


static Mapa* corrige_dir(Mapa *m) {
  if (m->dir->bf == 1) {
    m->bf = m->dir->bf = 0;
    return rotaciona_esq(m);
  }
  else/* if (m->dir->bf == -1) */ {
    /* COMPLETAR */
    return NULL;
  }
}

static Mapa* rotaciona_dir(Mapa *m) {
  /* FAZER */
  return NULL;
}

static Mapa* rotaciona_esq(Mapa *m) {
  Mapa* dir = m->dir;
  m->dir = dir->esq;
  dir->esq = m;
  return dir;
}

int busca (Mapa *m, int chave) {
  if (m == NULL) {
    return -1;
  }
  else if (m->chave == chave) {
    return m->conteudo;
  }
  else if (chave < m->chave) {
  return busca(m->esq, chave);
  }
  else if (chave > m->chave) {
    return busca(m->dir, chave);
  }
  else {
    return -1;
  }
}


void destroi (Mapa *m) {
  if (m != NULL) {
    destroi(m->esq);
    destroi(m->dir);
    free(m);
  }
}


int iguais (Mapa* m1, Mapa* m2) {

  if (m1==NULL) return (m2==NULL);
  if (m2==NULL) return 0;

  return (m1->chave == m2->chave) &&
          iguais (m1->esq, m2->esq) &&
          iguais (m1->dir, m2->dir);
}

int altura (Mapa* m) {
  if (m==NULL) return 0;
  printf ("chave %d\n", m->chave);
  return ((m->bf==-1)?(altura(m->esq)):altura(m->dir))+1;
}

static void mostra_mapa_int_aux(Mapa* m, int nivel) {
  if (m == NULL) return;

  for (int i = 0; i < nivel; i++) printf("  ");
  printf("%d:%d\n", m->chave, m->bf);

  mostra_mapa_int_aux(m->esq, nivel + 1);
  mostra_mapa_int_aux(m->dir, nivel + 1);
}

static void mostra_mapa_int(Mapa* m) {

  printf("Arvore AVL:\n");
  mostra_mapa_int_aux(m, 0);
  return;


  printf("[");
  if (m != NULL) {
    printf("%d:%d", m->chave,m->bf);
    mostra_mapa_int(m->esq);
    mostra_mapa_int(m->dir);
  }
  printf("]");
}

void mostra (Mapa* m) {
  mostra_mapa_int(m);
  printf ("\n");
}
