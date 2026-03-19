#include "binary_tree.h"

int main() {
    printf("Gerando árvore binária aleatória...\n\n");
    Node *root = generateRandomTree();
    printTree(root);
    freeTree(root); // Liberar a memória alocada para a árvore
    printf("\nLegenda: Os valores são inseridos como em uma Árvore Binária de Busca (BST).\n");
    scanf("%*c", 0, "");
    return 0;
}
