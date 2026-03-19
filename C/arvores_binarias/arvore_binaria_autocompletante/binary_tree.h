#ifndef BINARY_TREE_H
#define BINARY_TREE_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MAX_VALUE 100
#define MIN_VALUE 1
#define MAX_NODES 50
#define MIN_NODES 5

typedef struct Node {
    int value;
    struct Node *left;
    struct Node *right;
} Node;

Node *createNode(int value);
Node *insert(Node *root, int value);
Node *generateRandomTree();
void printTree(Node *root);
void freeTree(Node *root);

#endif // BINARY_TREE_H
