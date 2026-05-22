/* avl.c – AVL Tree indexed on User.name (secondary index) */
#include "minidb.h"

AVLTree *avl_create(void) {
    AVLTree *t = calloc(1, sizeof(AVLTree));
    return t;
}

/* ─── Height helpers ──────────────────────────────────────────── */
static int height(AVLNode *n) { return n ? n->height : 0; }
static int max2(int a, int b) { return a > b ? a : b; }
static void update_height(AVLNode *n) {
    n->height = 1 + max2(height(n->left), height(n->right));
}
static int balance_factor(AVLNode *n) {
    return n ? height(n->left) - height(n->right) : 0;
}

/* ─── Rotations ───────────────────────────────────────────────── */
static AVLNode *rotate_right(AVLNode *y) {
    AVLNode *x  = y->left;
    AVLNode *T2 = x->right;
    x->right = y;
    y->left  = T2;
    update_height(y);
    update_height(x);
    return x;
}

static AVLNode *rotate_left(AVLNode *x) {
    AVLNode *y  = x->right;
    AVLNode *T2 = y->left;
    y->left  = x;
    x->right = T2;
    update_height(x);
    update_height(y);
    return y;
}

static AVLNode *balance(AVLNode *n) {
    update_height(n);
    int bf = balance_factor(n);
    if (bf > 1) {
        if (balance_factor(n->left) < 0)
            n->left = rotate_left(n->left);
        return rotate_right(n);
    }
    if (bf < -1) {
        if (balance_factor(n->right) > 0)
            n->right = rotate_right(n->right);
        return rotate_left(n);
    }
    return n;
}

/* ─── Insert ──────────────────────────────────────────────────── */
static AVLNode *avl_node_new(const char *name, int id) {
    AVLNode *n = calloc(1, sizeof(AVLNode));
    strncpy(n->name, name, MAX_NAME - 1);
    n->id = id;
    n->height = 1;
    return n;
}

static AVLNode *insert_node(AVLNode *n, const char *name, int id) {
    if (!n) return avl_node_new(name, id);
    int cmp = strcmp(name, n->name);
    if (cmp < 0)      n->left  = insert_node(n->left,  name, id);
    else if (cmp > 0) n->right = insert_node(n->right, name, id);
    else { n->id = id; return n; }   /* update if exists */
    return balance(n);
}

void avl_insert(AVLTree *t, const char *name, int id) {
    t->root = insert_node(t->root, name, id);
    t->size++;
}

/* ─── Search ──────────────────────────────────────────────────── */
static AVLNode *search_node(AVLNode *n, const char *name) {
    if (!n) return NULL;
    int cmp = strcmp(name, n->name);
    if (cmp == 0)  return n;
    if (cmp < 0)   return search_node(n->left,  name);
    return             search_node(n->right, name);
}

AVLNode *avl_search(AVLTree *t, const char *name) {
    return search_node(t->root, name);
}

/* ─── Delete ──────────────────────────────────────────────────── */
static AVLNode *min_node(AVLNode *n) {
    while (n->left) n = n->left;
    return n;
}

static AVLNode *delete_node(AVLNode *n, const char *name, int *deleted) {
    if (!n) return NULL;
    int cmp = strcmp(name, n->name);
    if (cmp < 0)
        n->left  = delete_node(n->left,  name, deleted);
    else if (cmp > 0)
        n->right = delete_node(n->right, name, deleted);
    else {
        *deleted = 1;
        if (!n->left || !n->right) {
            AVLNode *tmp = n->left ? n->left : n->right;
            free(n);
            return tmp;
        }
        AVLNode *succ = min_node(n->right);
        strncpy(n->name, succ->name, MAX_NAME - 1);
        n->id   = succ->id;
        n->right = delete_node(n->right, succ->name, deleted);
    }
    return balance(n);
}

int avl_delete(AVLTree *t, const char *name) {
    int deleted = 0;
    t->root = delete_node(t->root, name, &deleted);
    if (deleted) t->size--;
    return deleted;
}

/* ─── Free ────────────────────────────────────────────────────── */
static void free_node(AVLNode *n) {
    if (!n) return;
    free_node(n->left);
    free_node(n->right);
    free(n);
}

void avl_free(AVLTree *t) {
    free_node(t->root);
    free(t);
}
