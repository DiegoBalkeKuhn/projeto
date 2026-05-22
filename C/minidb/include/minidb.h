#ifndef MINIDB_H
#define MINIDB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

/* ─── Constants ─────────────────────────────────────────────── */
#define MAX_NAME    64
#define MAX_EMAIL   128
#define HASH_SIZE   16381   /* prime ≈ 16k */
#define BTREE_T     3       /* minimum degree */
#define BTREE_MAX   (2*BTREE_T - 1)
#define BTREE_MIN   (BTREE_T - 1)

/* ─── User Record ────────────────────────────────────────────── */
typedef struct {
    int  id;
    char name[MAX_NAME];
    char email[MAX_EMAIL];
} User;

/* ═══════════════════════════════════════════════════════════════
   B-TREE  (index on id)
   ═══════════════════════════════════════════════════════════════ */
typedef struct BTreeNode {
    int  n;                         /* number of keys currently stored   */
    int  keys[BTREE_MAX];           /* keys (id values)                  */
    User *vals[BTREE_MAX];          /* pointers to actual records        */
    struct BTreeNode *child[BTREE_MAX + 1];
    int  leaf;
} BTreeNode;

typedef struct {
    BTreeNode *root;
    int        size;
} BTree;

BTree      *btree_create(void);
void        btree_insert(BTree *t, User *u);
User       *btree_search(BTree *t, int id);
int         btree_delete(BTree *t, int id);
User      **btree_range(BTree *t, int lo, int hi, int *count);
void        btree_free(BTree *t);

/* ═══════════════════════════════════════════════════════════════
   AVL TREE  (secondary index on name)
   ═══════════════════════════════════════════════════════════════ */
typedef struct AVLNode {
    char  name[MAX_NAME];
    int   id;               /* pointer to real record (simulated by id)  */
    int   height;
    struct AVLNode *left, *right;
} AVLNode;

typedef struct {
    AVLNode *root;
    int      size;
} AVLTree;

AVLTree   *avl_create(void);
void       avl_insert(AVLTree *t, const char *name, int id);
AVLNode   *avl_search(AVLTree *t, const char *name);
int        avl_delete(AVLTree *t, const char *name);
void       avl_free(AVLTree *t);

/* ═══════════════════════════════════════════════════════════════
   HASH MAP  (cache on email)
   ═══════════════════════════════════════════════════════════════ */
typedef struct HashEntry {
    char  email[MAX_EMAIL];
    int   id;
    struct HashEntry *next;
} HashEntry;

typedef struct {
    HashEntry *buckets[HASH_SIZE];
    int        size;
    int        collisions;
} HashMap;

HashMap   *hashmap_create(void);
void       hashmap_insert(HashMap *m, const char *email, int id);
int        hashmap_get(HashMap *m, const char *email);   /* returns id or -1 */
int        hashmap_delete(HashMap *m, const char *email);
void       hashmap_free(HashMap *m);
void       hashmap_stats(HashMap *m);

/* ═══════════════════════════════════════════════════════════════
   DATABASE ENGINE
   ═══════════════════════════════════════════════════════════════ */
typedef struct {
    BTree  *id_index;
    AVLTree *name_index;
    HashMap *email_cache;
    int      record_count;
} Database;

Database  *db_create(void);
int        db_insert(Database *db, int id, const char *name, const char *email);
User      *db_select_by_id(Database *db, int id);
User     **db_select_range(Database *db, int lo, int hi, int *count);
User      *db_select_by_name(Database *db, const char *name);
int        db_delete(Database *db, int id);
int        db_load_csv(Database *db, const char *path);
void       db_free(Database *db);

/* ─── Timing helper ─────────────────────────────────────────── */
static inline double now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

#endif /* MINIDB_H */
