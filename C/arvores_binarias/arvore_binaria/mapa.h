#ifndef MAPA_H
#define MAPA_H

typedef struct smapa Mapa;
typedef int (*funcp) (int);

Mapa* cria (void);
Mapa* insere (Mapa* m, int chave, int dados);
int busca (Mapa *m, int chave);
void destroi (Mapa *m);
void percorre (Mapa *m, funcp f);

#endif //MAPA_H
