#include <stdio.h>
#include "mapa.h"

int main (void) {

    int chave;

    Mapa *ummapa = cria();

    while (1) {
        printf ("chave a inserir (-999 para terminar): ");
        scanf ("%d", &chave);
        if (chave==-999) break;

        ummapa = insere(ummapa, chave, 2*chave);
        mostra(ummapa);
    }

    destroi(ummapa);

    return 0;
}

