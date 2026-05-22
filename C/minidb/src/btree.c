/* btree.c – B-Tree of minimum degree t=3 (order 6) indexed on User.id */
#include "minidb.h"

/* ─── Node allocation ─────────────────────────────────────────── */
static BTreeNode *node_new(int leaf) {
    BTreeNode *n = calloc(1, sizeof(BTreeNode));
    n->leaf = leaf;
    return n;
}

BTree *btree_create(void) {
    BTree *t = malloc(sizeof(BTree));
    t->root = node_new(1);
    t->size = 0;
    return t;
}

/* ─── Search ──────────────────────────────────────────────────── */
static User *node_search(BTreeNode *x, int k) {
    int i = 0;
    while (i < x->n && k > x->keys[i]) i++;
    if (i < x->n && k == x->keys[i]) return x->vals[i];
    if (x->leaf) return NULL;
    return node_search(x->child[i], k);
}

User *btree_search(BTree *t, int id) {
    return node_search(t->root, id);
}

/* ─── Split child ─────────────────────────────────────────────── */
static void split_child(BTreeNode *x, int i) {
    BTreeNode *y = x->child[i];
    BTreeNode *z = node_new(y->leaf);
    z->n = BTREE_T - 1;

    for (int j = 0; j < BTREE_T - 1; j++) {
        z->keys[j] = y->keys[j + BTREE_T];
        z->vals[j] = y->vals[j + BTREE_T];
    }
    if (!y->leaf)
        for (int j = 0; j < BTREE_T; j++)
            z->child[j] = y->child[j + BTREE_T];

    y->n = BTREE_T - 1;

    /* shift children of x */
    for (int j = x->n; j >= i + 1; j--)
        x->child[j + 1] = x->child[j];
    x->child[i + 1] = z;

    /* shift keys of x */
    for (int j = x->n - 1; j >= i; j--) {
        x->keys[j + 1] = x->keys[j];
        x->vals[j + 1] = x->vals[j];
    }
    x->keys[i] = y->keys[BTREE_T - 1];
    x->vals[i] = y->vals[BTREE_T - 1];
    x->n++;
}

/* ─── Insert non-full ─────────────────────────────────────────── */
static void insert_nonfull(BTreeNode *x, User *u) {
    int i = x->n - 1;
    if (x->leaf) {
        while (i >= 0 && u->id < x->keys[i]) {
            x->keys[i + 1] = x->keys[i];
            x->vals[i + 1] = x->vals[i];
            i--;
        }
        x->keys[i + 1] = u->id;
        x->vals[i + 1] = u;
        x->n++;
    } else {
        while (i >= 0 && u->id < x->keys[i]) i--;
        i++;
        if (x->child[i]->n == BTREE_MAX) {
            split_child(x, i);
            if (u->id > x->keys[i]) i++;
        }
        insert_nonfull(x->child[i], u);
    }
}

void btree_insert(BTree *t, User *u) {
    BTreeNode *r = t->root;
    if (r->n == BTREE_MAX) {
        BTreeNode *s = node_new(0);
        s->child[0] = r;
        t->root = s;
        split_child(s, 0);
        insert_nonfull(s, u);
    } else {
        insert_nonfull(r, u);
    }
    t->size++;
}

/* ─── Range query (in-order traversal) ───────────────────────── */
static void range_collect(BTreeNode *x, int lo, int hi,
                          User ***arr, int *cnt, int *cap) {
    if (!x) return;
    for (int i = 0; i < x->n; i++) {
        if (!x->leaf)
            range_collect(x->child[i], lo, hi, arr, cnt, cap);
        if (x->keys[i] >= lo && x->keys[i] <= hi) {
            if (*cnt == *cap) {
                *cap *= 2;
                *arr = realloc(*arr, *cap * sizeof(User *));
            }
            (*arr)[(*cnt)++] = x->vals[i];
        }
    }
    if (!x->leaf)
        range_collect(x->child[x->n], lo, hi, arr, cnt, cap);
}

User **btree_range(BTree *t, int lo, int hi, int *count) {
    int cap = 64;
    User **arr = malloc(cap * sizeof(User *));
    *count = 0;
    range_collect(t->root, lo, hi, &arr, count, &cap);
    return arr;
}

/* ─── Delete helpers ──────────────────────────────────────────── */
static int find_key(BTreeNode *x, int k) {
    int i = 0;
    while (i < x->n && x->keys[i] < k) i++;
    return i;
}

static User *get_predecessor(BTreeNode *x) {
    while (!x->leaf) x = x->child[x->n];
    return x->vals[x->n - 1];
}

static User *get_successor(BTreeNode *x) {
    while (!x->leaf) x = x->child[0];
    return x->vals[0];
}

static void merge(BTreeNode *x, int i) {
    BTreeNode *child  = x->child[i];
    BTreeNode *sibling = x->child[i + 1];

    child->keys[BTREE_T - 1] = x->keys[i];
    child->vals[BTREE_T - 1] = x->vals[i];

    for (int j = 0; j < sibling->n; j++) {
        child->keys[j + BTREE_T] = sibling->keys[j];
        child->vals[j + BTREE_T] = sibling->vals[j];
    }
    if (!child->leaf)
        for (int j = 0; j <= sibling->n; j++)
            child->child[j + BTREE_T] = sibling->child[j];

    for (int j = i + 1; j < x->n; j++) {
        x->keys[j - 1] = x->keys[j];
        x->vals[j - 1] = x->vals[j];
    }
    for (int j = i + 2; j <= x->n; j++)
        x->child[j - 1] = x->child[j];

    child->n += sibling->n + 1;
    x->n--;
    free(sibling);
}

static void fill(BTreeNode *x, int i);

static void borrow_from_prev(BTreeNode *x, int i) {
    BTreeNode *child = x->child[i];
    BTreeNode *sib   = x->child[i - 1];

    for (int j = child->n - 1; j >= 0; j--) {
        child->keys[j + 1] = child->keys[j];
        child->vals[j + 1] = child->vals[j];
    }
    if (!child->leaf)
        for (int j = child->n; j >= 0; j--)
            child->child[j + 1] = child->child[j];

    child->keys[0] = x->keys[i - 1];
    child->vals[0] = x->vals[i - 1];
    if (!child->leaf) child->child[0] = sib->child[sib->n];

    x->keys[i - 1] = sib->keys[sib->n - 1];
    x->vals[i - 1] = sib->vals[sib->n - 1];

    child->n++;
    sib->n--;
}

static void borrow_from_next(BTreeNode *x, int i) {
    BTreeNode *child = x->child[i];
    BTreeNode *sib   = x->child[i + 1];

    child->keys[child->n] = x->keys[i];
    child->vals[child->n] = x->vals[i];
    if (!child->leaf) child->child[child->n + 1] = sib->child[0];

    x->keys[i] = sib->keys[0];
    x->vals[i] = sib->vals[0];

    for (int j = 1; j < sib->n; j++) {
        sib->keys[j - 1] = sib->keys[j];
        sib->vals[j - 1] = sib->vals[j];
    }
    if (!sib->leaf)
        for (int j = 1; j <= sib->n; j++)
            sib->child[j - 1] = sib->child[j];

    child->n++;
    sib->n--;
}

static void fill(BTreeNode *x, int i) {
    if (i != 0 && x->child[i - 1]->n >= BTREE_T)
        borrow_from_prev(x, i);
    else if (i != x->n && x->child[i + 1]->n >= BTREE_T)
        borrow_from_next(x, i);
    else {
        if (i != x->n) merge(x, i);
        else           merge(x, i - 1);
    }
}

static int delete_from_node(BTreeNode *x, int k);

static void delete_leaf(BTreeNode *x, int i) {
    for (int j = i + 1; j < x->n; j++) {
        x->keys[j - 1] = x->keys[j];
        x->vals[j - 1] = x->vals[j];
    }
    x->n--;
}

static int delete_from_node(BTreeNode *x, int k) {
    int i = find_key(x, k);
    if (i < x->n && x->keys[i] == k) {
        if (x->leaf) {
            delete_leaf(x, i);
        } else {
            if (x->child[i]->n >= BTREE_T) {
                User *pred = get_predecessor(x->child[i]);
                x->keys[i] = pred->id;
                x->vals[i] = pred;
                delete_from_node(x->child[i], pred->id);
            } else if (x->child[i + 1]->n >= BTREE_T) {
                User *succ = get_successor(x->child[i + 1]);
                x->keys[i] = succ->id;
                x->vals[i] = succ;
                delete_from_node(x->child[i + 1], succ->id);
            } else {
                merge(x, i);
                delete_from_node(x->child[i], k);
            }
        }
        return 1;
    } else {
        if (x->leaf) return 0;
        int flag = (i == x->n);
        if (x->child[i]->n < BTREE_T) fill(x, i);
        if (flag && i > x->n)
            return delete_from_node(x->child[i - 1], k);
        else
            return delete_from_node(x->child[i], k);
    }
}

int btree_delete(BTree *t, int id) {
    if (!t->root) return 0;
    int r = delete_from_node(t->root, id);
    if (r) t->size--;
    if (t->root->n == 0) {
        BTreeNode *old = t->root;
        if (!t->root->leaf) t->root = t->root->child[0];
        free(old);
    }
    return r;
}

/* ─── Free ────────────────────────────────────────────────────── */
static void node_free(BTreeNode *x) {
    if (!x) return;
    if (!x->leaf)
        for (int i = 0; i <= x->n; i++)
            node_free(x->child[i]);
    free(x);
}

void btree_free(BTree *t) {
    node_free(t->root);
    free(t);
}
