#ifndef MAPA_H
#define MAPA_H

typedef struct smapa Mapa;

Mapa* cria (void);
Mapa* insere (Mapa* m, int chave, int novodado);
int busca (Mapa *m, int chave);
Mapa* retira (Mapa *m, int chave);

int altura (Mapa *m);
void destroi (Mapa *m);

void debug_mostra_mapa (Mapa *m);
int debug_altura (Mapa* m);
void mostra (Mapa *m);

#endif //MAPA_H
