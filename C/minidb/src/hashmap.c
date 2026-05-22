/* hashmap.c – Hash map with external chaining, indexed on email */
#include "minidb.h"

/* ─── Hash function (FNV-1a 32-bit) ──────────────────────────── */
static uint32_t fnv1a(const char *s) {
    uint32_t h = 2166136261u;
    while (*s) {
        h ^= (unsigned char)*s++;
        h *= 16777619u;
    }
    return h % HASH_SIZE;
}

HashMap *hashmap_create(void) {
    return calloc(1, sizeof(HashMap));
}

void hashmap_insert(HashMap *m, const char *email, int id) {
    uint32_t idx = fnv1a(email);
    HashEntry *e = m->buckets[idx];

    /* check for update */
    while (e) {
        if (strcmp(e->email, email) == 0) { e->id = id; return; }
        e = e->next;
    }

    /* new entry */
    HashEntry *ne = malloc(sizeof(HashEntry));
    strncpy(ne->email, email, MAX_EMAIL - 1);
    ne->email[MAX_EMAIL - 1] = '\0';
    ne->id   = id;
    ne->next = m->buckets[idx];
    if (m->buckets[idx]) m->collisions++;
    m->buckets[idx] = ne;
    m->size++;
}

int hashmap_get(HashMap *m, const char *email) {
    uint32_t idx = fnv1a(email);
    HashEntry *e = m->buckets[idx];
    while (e) {
        if (strcmp(e->email, email) == 0) return e->id;
        e = e->next;
    }
    return -1;
}

int hashmap_delete(HashMap *m, const char *email) {
    uint32_t idx = fnv1a(email);
    HashEntry **pp = &m->buckets[idx];
    while (*pp) {
        if (strcmp((*pp)->email, email) == 0) {
            HashEntry *del = *pp;
            *pp = del->next;
            free(del);
            m->size--;
            return 1;
        }
        pp = &(*pp)->next;
    }
    return 0;
}

void hashmap_stats(HashMap *m) {
    int used = 0, max_chain = 0;
    for (int i = 0; i < HASH_SIZE; i++) {
        if (!m->buckets[i]) continue;
        used++;
        int chain = 0;
        HashEntry *e = m->buckets[i];
        while (e) { chain++; e = e->next; }
        if (chain > max_chain) max_chain = chain;
    }
    printf("  HashTable: size=%d  buckets_used=%d/%d  load=%.2f%%  "
           "collisions=%d  max_chain=%d\n",
           m->size, used, HASH_SIZE,
           100.0 * used / HASH_SIZE,
           m->collisions, max_chain);
}

void hashmap_free(HashMap *m) {
    for (int i = 0; i < HASH_SIZE; i++) {
        HashEntry *e = m->buckets[i];
        while (e) { HashEntry *nx = e->next; free(e); e = nx; }
    }
    free(m);
}
