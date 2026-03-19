#include <stdio.h>
#include "main.h"

int main() {
  srand(time(NULL));

  Node *tree = criarTreeRandom();
  printTree(tree, 0);
  scanf("%d", &tree);
  return 0;
}