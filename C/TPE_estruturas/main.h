#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "functions.c"



Product *createProduct(int id, const char *name, int price);
Product *CreateTree();
void InsertProduct(Product **root, int id, const char *name, int price);
Product *SearchByID(Product *root, int id);
void searchRange(Product *root, int minPrice, int maxPrice);
void *SearchbyPriceRange(Product *root);
int diference(Product *root, int price);
Product *SearchClosestPrice(Product *root, int price);
void Destroi(Product *root);
void removebyID(Product *root, int id);
void InsertProductSyncronized(Product **root1, Product **root2, int id, const char *name, int price);
void destroiSync(Product *a1, Product *a2);