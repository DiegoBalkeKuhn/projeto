#include <stdio.h>
#include <string.h>
#include <stdlib.h>


typedef struct Product
{
  int id;
  char name[50];
  int price;
  struct Product *left;
  struct Product *right;
} Product;

Product *createProduct(int id, const char *name, int price)
{
  Product *newProduct = (Product *)malloc(sizeof(Product));
  newProduct->id = id;
  strncpy(newProduct->name, name, 50);
  newProduct->price = price;
  newProduct->left = NULL;
  newProduct->right = NULL;
  return newProduct;
}

Product *CreateTree()
{
  Product *root = (Product *)malloc(sizeof(Product));
  root->id = 0;
  root->name[0] = '\0';
  root->price = 0;
  root->left = NULL;
  root->right = NULL;

  return root;
}

void InsertProduct(Product **root, int id, const char *name, int price)
{
  if (*root == NULL)
  {
    *root = createProduct(id, name, price);
    return;
  }
  if (id < (*root)->id)
  {
    InsertProduct(&(*root)->left, id, name, price);
  }
  else
  {
    InsertProduct(&(*root)->right, id, name, price);
  }
}

void InsertProductSyncronized(Product **root1, Product **root2, int id, const char *name, int price)
{
  InsertProduct(root1, id, name, price);
  InsertProduct(root2, id, name, price);
}

void Destroi(Product *root)
{
  if (root == NULL)
  {
    return;
  }
  remove(root->left);
  remove(root->right);
  free(root);
}

void removebyID(Product *root, int id) {
  if (root == NULL) {
    return;
  }
  if (id < root->id) {
    removebyID(root->left, id);
  }
  else if (id > root->id) {
    removebyID(root->right, id);
  }
  else {
    if (root->left == NULL) {
      Product *temp = root->right;
      free(root);
      root = temp;
    }
    else if (root->right == NULL) {
      Product *temp = root->left;
      free(root);
      root = temp;
    }
    else {
      Product *temp = root->right;
      while (temp->left != NULL) {
        temp = temp->left;
      }
      root->id = temp->id;
      strncpy(root->name, temp->name, 50);
      root->price = temp->price;
      removebyID(root->right, temp->id);
      }
    }
  }

Product *SearchByID(Product *root, int id)
{
  if (root == NULL || root->id == id)
  {
    return root;
  }
  if (id < root->id)
  {
    return SearchByID(root->left, id);
  }
  else
  {
    return SearchByID(root->right, id);
  }
}

void searchRange(Product *root, int minPrice, int maxPrice)
{
  if (root == NULL)
  {
    return;
  }
  if (root->price >= minPrice && root->price <= maxPrice)
  {
    printf("ID: %d, Name: %s, Price: %d\n", root->id, root->name, root->price);
  }
  searchRange(root->left, minPrice, maxPrice);
  searchRange(root->right, minPrice, maxPrice);
}

void *SearchbyPriceRange(Product *root)
{
  int minprice;
  int maxprice;
  printf("qual seria o preço minimo? ");
  scanf("%d", &minprice);
  printf("qual seria o preço maximo? ");
  scanf("%d", &maxprice);
  searchRange(root, minprice, maxprice);
}

int diference(Product *root, int price)
{
  int diference = 0;
  if(price < root->price) {
    diference = root->price - price;
  }
  else {
    diference = price - root->price;
  }

  return diference;
}

Product *SearchClosestPrice(Product *root, int price) {
  if (root == NULL) {
    return NULL;
  }

  Product *closest = root;
  int closestDiference = diference(root, price);

  Product *leftClosest = SearchClosestPrice(root->left, price);
  if (leftClosest != NULL) {
    int leftDiference = diference(leftClosest, price);
    if (leftDiference < closestDiference) {
      closest = leftClosest;
      closestDiference = leftDiference;
    }
  }

  Product *rightClosest = SearchClosestPrice(root->right, price);
  if (rightClosest != NULL) {
    int rightDiference = diference(rightClosest, price);
    if (rightDiference < closestDiference) {
      closest = rightClosest;
    }
  }

  return closest;
}
