/* db.c – Database engine wiring all indexes together */
#include "minidb.h"

Database *db_create(void) {
    Database *db = calloc(1, sizeof(Database));
    db->id_index    = btree_create();
    db->name_index  = avl_create();
    db->email_cache = hashmap_create();
    return db;
}

int db_insert(Database *db, int id, const char *name, const char *email) {
    /* reject duplicates */
    if (btree_search(db->id_index, id)) return 0;

    User *u = malloc(sizeof(User));
    u->id = id;
    strncpy(u->name,  name,  MAX_NAME  - 1); u->name[MAX_NAME-1]  = '\0';
    strncpy(u->email, email, MAX_EMAIL - 1); u->email[MAX_EMAIL-1] = '\0';

    btree_insert(db->id_index, u);
    avl_insert(db->name_index, name, id);
    hashmap_insert(db->email_cache, email, id);
    db->record_count++;
    return 1;
}

User *db_select_by_id(Database *db, int id) {
    return btree_search(db->id_index, id);
}

User **db_select_range(Database *db, int lo, int hi, int *count) {
    return btree_range(db->id_index, lo, hi, count);
}

User *db_select_by_name(Database *db, const char *name) {
    AVLNode *an = avl_search(db->name_index, name);
    if (!an) return NULL;
    return btree_search(db->id_index, an->id);
}

int db_delete(Database *db, int id) {
    User *u = btree_search(db->id_index, id);
    if (!u) return 0;

    /* Remove from secondary indexes before freeing */
    avl_delete(db->name_index, u->name);
    hashmap_delete(db->email_cache, u->email);

    /* btree_delete frees the node but NOT the User (we own it) */
    char save_name[MAX_NAME];
    strncpy(save_name, u->name, MAX_NAME);
    (void)save_name;

    int r = btree_delete(db->id_index, id);
    if (r) {
        free(u);
        db->record_count--;
    }
    return r;
}

/* ─── CSV loader ─────────────────────────────────────────────── */
int db_load_csv(Database *db, const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) { perror("fopen"); return -1; }

    char line[512];
    int  loaded = 0;
    /* skip header */
    if (!fgets(line, sizeof(line), f)) { fclose(f); return 0; }

    while (fgets(line, sizeof(line), f)) {
        /* strip newline */
        line[strcspn(line, "\r\n")] = '\0';

        char *tok;
        char *id_s   = strtok_r(line,  ",", &tok);
        char *name   = strtok_r(NULL,  ",", &tok);
        char *email  = strtok_r(NULL,  ",", &tok);
        if (!id_s || !name || !email) continue;

        int id = atoi(id_s);
        db_insert(db, id, name, email);
        loaded++;
    }
    fclose(f);
    return loaded;
}

void db_free(Database *db) {
    /* Free all User records via in-order traversal of btree is complex;
       easiest: collect all via range then free */
    int cnt = 0;
    User **all = btree_range(db->id_index, 0, INT32_MAX, &cnt);
    btree_free(db->id_index);
    for (int i = 0; i < cnt; i++) free(all[i]);
    free(all);

    avl_free(db->name_index);
    hashmap_free(db->email_cache);
    free(db);
}
