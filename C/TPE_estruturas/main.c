#include <stdio.h>
#include "main.h"

int main () {

  Product *ArvoreID = CreateTree();
  Product *ArvorePreco = CreateTree();
  InsertProductSyncronized(&ArvoreID, &ArvorePreco, 1, "Produto A", 100);
  InsertProductSyncronized(&ArvoreID, &ArvorePreco, 2, "Produto B", 200);
  InsertProductSyncronized(&ArvoreID, &ArvorePreco, 3, "Produto C", 150);

  SearchbyPriceRange(ArvorePreco);
  Product *porid = SearchByID(ArvoreID, 2);
  Product *closest = SearchClosestPrice(ArvorePreco, 120);
  printf("Produto mais próximo do preço 120: ID: %d, Name: %s, Price: %d\n", closest->id, closest->name, closest->price);
  printf("Produto encontrado por ID: ID: %d, Name: %s, Price: %d\n", porid->id, porid->name, porid->price);

  Destroi(ArvoreID);
  Destroi(ArvorePreco);

    return 0;
}