#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TREE_RANGE 50
#define VALUE_RANGE 100

typedef struct Node {
    int id; 
    int data;
    struct Node* left;
    struct Node* right;
} Node;

typedef struct Tree {
    Node* root;
} Tree;


Tree *criarTree(){
  return NULL;
}

Node *createNode(int data) {
    Node *newNode = (Node*)malloc(sizeof(Node));
    if (newNode != NULL) {
        newNode->data = data;
        newNode->left = NULL;
        newNode->right = NULL;
        newNode->id = 0;
    }
    return newNode;
}

Node *createNoderandom() {
   srand(time(NULL));
   int valor = rand() % VALUE_RANGE;
   return createNode(valor);
}

Node *insere(Node *n, int chave) {

    if (n == NULL) {
        Node *novo = createNoderandom();
        novo->id = chave;
        return novo;
    }

    if (chave < n->id)
        n->left = insere(n->left, chave);
    else
        n->right = insere(n->right, chave);

    return n;
}

Node *criarTreeRandom() {

    int range = rand() % TREE_RANGE;

    Node *root = NULL;

    for (int i = 0; i < range; i++) {
        root = insere(root, rand() % VALUE_RANGE);
    }

    return root;
}

int height(Node *root) {
    if (root == NULL)
        return 0;

    int left = height(root->left);
    int right = height(root->right);

    return (left > right ? left : right) + 1;
}

void printSpaces(int count) {
    for (int i = 0; i < count; i++)
        printf(" ");
}

void printLevel(Node *root, int level) {

    if (root == NULL) {
        printf("   ");
        return;
    }

    if (level == 1) {
        printf("%3d", root->id);
    }
    else {
        printLevel(root->left, level - 1);
        printLevel(root->right, level - 1);
    }
}

void printTree(Node *root, int space) {

    if (root == NULL)
        return;

    space += 6;

    printTree(root->right, space);

    printf("\n");

    for (int i = 6; i < space; i++)
        printf(" ");

    printf("%d\n", root->id);

    printTree(root->left, space);
}
