#include "binary_tree.h"

// Função para criar um novo nodo
Node *createNode(int value) {
    Node *newNode = (Node *)malloc(sizeof(Node));
    if (newNode == NULL) {
        perror("Erro ao alocar memória para o nodo");
        exit(EXIT_FAILURE);
    }
    newNode->value = value;
    newNode->left = NULL;
    newNode->right = NULL;
    return newNode;
}

// Função para inserir um valor na árvore binária de busca
Node *insert(Node *root, int value) {
    if (root == NULL) {
        return createNode(value);
    }
    if (value < root->value) {
        root->left = insert(root->left, value);
    } else if (value > root->value) {
        root->right = insert(root->right, value);
    }
    return root;
}

// Função para gerar uma árvore binária com quantidade e valores aleatórios
Node *generateRandomTree() {
    srand(time(NULL));
    int num_nodes = rand() % (MAX_NODES - MIN_NODES + 1) + MIN_NODES;
    Node *root = NULL;

    for (int i = 0; i < num_nodes; i++) {
        int value = rand() % (MAX_VALUE - MIN_VALUE + 1) + MIN_VALUE;
        root = insert(root, value);
    }
    return root;
}

// Função para liberar a memória da árvore
void freeTree(Node *root) {
    if (root != NULL) {
        freeTree(root->left);
        freeTree(root->right);
        free(root);
    }
}

// --- Funções para exibição da árvore ---

typedef struct {
    char **lines;
    int width;
    int height;
    int middle;
} DisplayInfo;

DisplayInfo _display_aux(Node *node) {
    DisplayInfo info;
    info.lines = NULL;
    info.width = 0;
    info.height = 0;
    info.middle = 0;

    if (node == NULL) return info;

    char s[20];
    sprintf(s, "%d", node->value);
    int u = strlen(s);

    if (node->right == NULL && node->left == NULL) {
        info.lines = (char **)malloc(sizeof(char *));
        info.lines[0] = strdup(s);
        info.width = u;
        info.height = 1;
        info.middle = u / 2;
        return info;
    }

    if (node->right == NULL) {
        DisplayInfo left = _display_aux(node->left);
        info.height = left.height + 2;
        info.width = left.width + u;
        info.middle = left.width + u / 2;
        info.lines = (char **)malloc(sizeof(char *) * info.height);

        info.lines[0] = (char *)malloc(info.width + 1);
        memset(info.lines[0], ' ', info.width);
        info.lines[0][info.width] = '\0';
        for (int i = 0; i < left.width - left.middle - 1; i++) info.lines[0][left.middle + 1 + i] = '_';
        memcpy(info.lines[0] + left.width, s, u);

        info.lines[1] = (char *)malloc(info.width + 1);
        memset(info.lines[1], ' ', info.width);
        info.lines[1][info.width] = '\0';
        info.lines[1][left.middle] = '/';

        for (int i = 0; i < left.height; i++) {
            info.lines[i + 2] = (char *)malloc(info.width + 1);
            memset(info.lines[i + 2], ' ', info.width);
            info.lines[i + 2][info.width] = '\0';
            memcpy(info.lines[i + 2], left.lines[i], strlen(left.lines[i]));
            free(left.lines[i]);
        }
        free(left.lines);
        return info;
    }

    if (node->left == NULL) {
        DisplayInfo right = _display_aux(node->right);
        info.height = right.height + 2;
        info.width = right.width + u;
        info.middle = u / 2;
        info.lines = (char **)malloc(sizeof(char *) * info.height);

        info.lines[0] = (char *)malloc(info.width + 1);
        memset(info.lines[0], ' ', info.width);
        info.lines[0][info.width] = '\0';
        memcpy(info.lines[0], s, u);
        for (int i = 0; i < right.middle; i++) info.lines[0][u + i] = '_';

        info.lines[1] = (char *)malloc(info.width + 1);
        memset(info.lines[1], ' ', info.width);
        info.lines[1][info.width] = '\0';
        info.lines[1][u + right.middle] = '\\';

        for (int i = 0; i < right.height; i++) {
            info.lines[i + 2] = (char *)malloc(info.width + 1);
            memset(info.lines[i + 2], ' ', info.width);
            info.lines[i + 2][info.width] = '\0';
            memcpy(info.lines[i + 2] + u, right.lines[i], strlen(right.lines[i]));
            free(right.lines[i]);
        }
        free(right.lines);
        return info;
    }

    DisplayInfo left = _display_aux(node->left);
    DisplayInfo right = _display_aux(node->right);
    info.height = (left.height > right.height ? left.height : right.height) + 2;
    info.width = left.width + right.width + u;
    info.middle = left.width + u / 2;
    info.lines = (char **)malloc(sizeof(char *) * info.height);

    info.lines[0] = (char *)malloc(info.width + 1);
    memset(info.lines[0], ' ', info.width);
    info.lines[0][info.width] = '\0';
    for (int i = 0; i < left.width - left.middle - 1; i++) info.lines[0][left.middle + 1 + i] = '_';
    memcpy(info.lines[0] + left.width, s, u);
    for (int i = 0; i < right.middle; i++) info.lines[0][left.width + u + i] = '_';

    info.lines[1] = (char *)malloc(info.width + 1);
    memset(info.lines[1], ' ', info.width);
    info.lines[1][info.width] = '\0';
    info.lines[1][left.middle] = '/';
    info.lines[1][left.width + u + right.middle] = '\\';

    int max_h = (left.height > right.height ? left.height : right.height);
    for (int i = 0; i < max_h; i++) {
        info.lines[i + 2] = (char *)malloc(info.width + 1);
        memset(info.lines[i + 2], ' ', info.width);
        info.lines[i + 2][info.width] = '\0';
        if (i < left.height) {
            memcpy(info.lines[i + 2], left.lines[i], strlen(left.lines[i]));
            free(left.lines[i]);
        }
        if (i < right.height) {
            memcpy(info.lines[i + 2] + left.width + u, right.lines[i], strlen(right.lines[i]));
            free(right.lines[i]);
        }
    }
    free(left.lines);
    free(right.lines);
    return info;
}

void printTree(Node *root) {
    DisplayInfo info = _display_aux(root);
    if (info.lines == NULL) return;
    for (int i = 0; i < info.height; i++) {
        printf("%s\n", info.lines[i]);
        free(info.lines[i]);
    }
    free(info.lines);
}
