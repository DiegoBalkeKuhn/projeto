/*
 * minidb.c — Mini banco de dados em C puro
 *
 * Estruturas:
 *   - B-Tree (t=3)  : índice primário por id
 *   - AVL Tree      : índice secundário por nome
 *   - Hash Map      : cache por email (encadeamento externo)
 *
 * Compilar:  gcc -O2 -o minidb minidb.c
 * Executar:  ./minidb [arquivo.csv]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ════════════════════════════════════════════
   DEFINIÇÕES GERAIS
   ════════════════════════════════════════════ */

#define NAME_LEN   64
#define EMAIL_LEN  128
#define HASH_SIZE  16381   /* primo ≈ 16k */
#define T          3       /* grau mínimo da B-Tree */
#define MAX_KEYS   (2*T-1) /* 5 */
#define MIN_KEYS   (T-1)   /* 2 */

typedef struct {
    int  id;
    char name[NAME_LEN];
    char email[EMAIL_LEN];
} User;

/* ────── timer ────── */
static double ms_now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

/* ════════════════════════════════════════════
   B-TREE
   ════════════════════════════════════════════ */

typedef struct BNode {
    int   n;                  /* número de chaves */
    int   keys[MAX_KEYS];
    User *vals[MAX_KEYS];
    struct BNode *ch[MAX_KEYS+1];
    int   leaf;
} BNode;

static BNode *bnode_new(int leaf) {
    BNode *x = calloc(1, sizeof(BNode));
    x->leaf = leaf;
    return x;
}

/* busca pontual */
static User *bt_search(BNode *x, int id) {
    int i = 0;
    while (i < x->n && id > x->keys[i]) i++;
    if (i < x->n && id == x->keys[i]) return x->vals[i];
    if (x->leaf) return NULL;
    return bt_search(x->ch[i], id);
}

/* split filho i de x */
static void bt_split(BNode *x, int i) {
    BNode *y = x->ch[i];
    BNode *z = bnode_new(y->leaf);
    z->n = T - 1;
    for (int j = 0; j < T-1; j++) { z->keys[j] = y->keys[j+T]; z->vals[j] = y->vals[j+T]; }
    if (!y->leaf) for (int j = 0; j < T; j++) z->ch[j] = y->ch[j+T];
    y->n = T - 1;
    for (int j = x->n; j >= i+1; j--) x->ch[j+1] = x->ch[j];
    x->ch[i+1] = z;
    for (int j = x->n-1; j >= i; j--) { x->keys[j+1] = x->keys[j]; x->vals[j+1] = x->vals[j]; }
    x->keys[i] = y->keys[T-1];
    x->vals[i] = y->vals[T-1];
    x->n++;
}

static void bt_ins_nonfull(BNode *x, User *u) {
    int i = x->n - 1;
    if (x->leaf) {
        while (i >= 0 && u->id < x->keys[i]) { x->keys[i+1]=x->keys[i]; x->vals[i+1]=x->vals[i]; i--; }
        x->keys[i+1] = u->id; x->vals[i+1] = u; x->n++;
    } else {
        while (i >= 0 && u->id < x->keys[i]) i--;
        i++;
        if (x->ch[i]->n == MAX_KEYS) { bt_split(x, i); if (u->id > x->keys[i]) i++; }
        bt_ins_nonfull(x->ch[i], u);
    }
}

/* range query — coleta em [lo,hi] em ordem */
static void bt_range(BNode *x, int lo, int hi, User ***out, int *n, int *cap) {
    if (!x) return;
    for (int i = 0; i < x->n; i++) {
        if (!x->leaf) bt_range(x->ch[i], lo, hi, out, n, cap);
        if (x->keys[i] >= lo && x->keys[i] <= hi) {
            if (*n == *cap) { *cap *= 2; *out = realloc(*out, *cap * sizeof(User*)); }
            (*out)[(*n)++] = x->vals[i];
        }
    }
    if (!x->leaf) bt_range(x->ch[x->n], lo, hi, out, n, cap);
}

/* ── delete helpers ── */
static int bt_find(BNode *x, int k) { int i=0; while(i<x->n && x->keys[i]<k) i++; return i; }
static User *bt_pred(BNode *x) { while(!x->leaf) x=x->ch[x->n]; return x->vals[x->n-1]; }
static User *bt_succ(BNode *x) { while(!x->leaf) x=x->ch[0]; return x->vals[0]; }

static void bt_merge(BNode *x, int i) {
    BNode *L = x->ch[i], *R = x->ch[i+1];
    L->keys[T-1] = x->keys[i]; L->vals[T-1] = x->vals[i];
    for (int j=0;j<R->n;j++){L->keys[j+T]=R->keys[j];L->vals[j+T]=R->vals[j];}
    if (!L->leaf) for(int j=0;j<=R->n;j++) L->ch[j+T]=R->ch[j];
    for(int j=i+1;j<x->n;j++){x->keys[j-1]=x->keys[j];x->vals[j-1]=x->vals[j];}
    for(int j=i+2;j<=x->n;j++) x->ch[j-1]=x->ch[j];
    L->n += R->n+1; x->n--;
    free(R);
}

static void bt_borrow_prev(BNode *x, int i) {
    BNode *c=x->ch[i], *s=x->ch[i-1];
    for(int j=c->n-1;j>=0;j--){c->keys[j+1]=c->keys[j];c->vals[j+1]=c->vals[j];}
    if(!c->leaf) for(int j=c->n;j>=0;j--) c->ch[j+1]=c->ch[j];
    c->keys[0]=x->keys[i-1]; c->vals[0]=x->vals[i-1];
    if(!c->leaf) c->ch[0]=s->ch[s->n];
    x->keys[i-1]=s->keys[s->n-1]; x->vals[i-1]=s->vals[s->n-1];
    c->n++; s->n--;
}

static void bt_borrow_next(BNode *x, int i) {
    BNode *c=x->ch[i], *s=x->ch[i+1];
    c->keys[c->n]=x->keys[i]; c->vals[c->n]=x->vals[i];
    if(!c->leaf) c->ch[c->n+1]=s->ch[0];
    x->keys[i]=s->keys[0]; x->vals[i]=s->vals[0];
    for(int j=1;j<s->n;j++){s->keys[j-1]=s->keys[j];s->vals[j-1]=s->vals[j];}
    if(!s->leaf) for(int j=1;j<=s->n;j++) s->ch[j-1]=s->ch[j];
    c->n++; s->n--;
}

static void bt_fill(BNode *x, int i) {
    if (i!=0 && x->ch[i-1]->n>=T) bt_borrow_prev(x,i);
    else if (i!=x->n && x->ch[i+1]->n>=T) bt_borrow_next(x,i);
    else { if(i!=x->n) bt_merge(x,i); else bt_merge(x,i-1); }
}

static int bt_del(BNode *x, int k) {
    int i = bt_find(x, k);
    if (i < x->n && x->keys[i] == k) {
        if (x->leaf) {
            for(int j=i+1;j<x->n;j++){x->keys[j-1]=x->keys[j];x->vals[j-1]=x->vals[j];}
            x->n--; return 1;
        }
        if (x->ch[i]->n >= T) {
            User *p=bt_pred(x->ch[i]); x->keys[i]=p->id; x->vals[i]=p; return bt_del(x->ch[i],p->id);
        } else if (x->ch[i+1]->n >= T) {
            User *s=bt_succ(x->ch[i+1]); x->keys[i]=s->id; x->vals[i]=s; return bt_del(x->ch[i+1],s->id);
        } else {
            bt_merge(x,i); return bt_del(x->ch[i],k);
        }
    } else {
        if (x->leaf) return 0;
        int flag = (i==x->n);
        if (x->ch[i]->n < T) bt_fill(x,i);
        if (flag && i > x->n) return bt_del(x->ch[i-1],k);
        return bt_del(x->ch[i],k);
    }
}

/* ── wrappers simples ── */
typedef struct { BNode *root; int size; } BTree;

static BTree *bt_create(void) {
    BTree *t = malloc(sizeof(BTree));
    t->root = bnode_new(1); t->size = 0;
    return t;
}

static void bt_insert(BTree *t, User *u) {
    BNode *r = t->root;
    if (r->n == MAX_KEYS) {
        BNode *s = bnode_new(0);
        s->ch[0] = r; t->root = s;
        bt_split(s,0); bt_ins_nonfull(s,u);
    } else bt_ins_nonfull(r,u);
    t->size++;
}

static int bt_delete(BTree *t, int id) {
    int r = bt_del(t->root, id);
    if (r) t->size--;
    if (t->root->n == 0 && !t->root->leaf) {
        BNode *old = t->root; t->root = t->root->ch[0]; free(old);
    }
    return r;
}

/* ════════════════════════════════════════════
   AVL TREE
   ════════════════════════════════════════════ */

typedef struct AVLNode {
    char name[NAME_LEN];
    int  id;
    int  h;
    struct AVLNode *l, *r;
} AVLNode;

static int avl_h(AVLNode *n) { return n ? n->h : 0; }
static void avl_upd(AVLNode *n) { int a=avl_h(n->l),b=avl_h(n->r); n->h=1+(a>b?a:b); }
static int avl_bf(AVLNode *n) { return n ? avl_h(n->l)-avl_h(n->r) : 0; }

static AVLNode *avl_rr(AVLNode *y) { AVLNode *x=y->l; y->l=x->r; x->r=y; avl_upd(y); avl_upd(x); return x; }
static AVLNode *avl_rl(AVLNode *x) { AVLNode *y=x->r; x->r=y->l; y->l=x; avl_upd(x); avl_upd(y); return y; }

static AVLNode *avl_bal(AVLNode *n) {
    avl_upd(n);
    if (avl_bf(n)>1)  { if(avl_bf(n->l)<0) n->l=avl_rl(n->l); return avl_rr(n); }
    if (avl_bf(n)<-1) { if(avl_bf(n->r)>0) n->r=avl_rr(n->r); return avl_rl(n); }
    return n;
}

static AVLNode *avl_ins(AVLNode *n, const char *name, int id) {
    if (!n) { AVLNode *x=calloc(1,sizeof(AVLNode)); strncpy(x->name,name,NAME_LEN-1); x->id=id; x->h=1; return x; }
    int c = strcmp(name, n->name);
    if (c<0) n->l=avl_ins(n->l,name,id);
    else if(c>0) n->r=avl_ins(n->r,name,id);
    else n->id=id;
    return avl_bal(n);
}

static AVLNode *avl_find(AVLNode *n, const char *name) {
    if(!n) return NULL;
    int c=strcmp(name,n->name);
    if(c==0) return n;
    return c<0 ? avl_find(n->l,name) : avl_find(n->r,name);
}

static AVLNode *avl_min(AVLNode *n) { while(n->l) n=n->l; return n; }

static AVLNode *avl_del(AVLNode *n, const char *name, int *ok) {
    if(!n) return NULL;
    int c=strcmp(name,n->name);
    if(c<0) n->l=avl_del(n->l,name,ok);
    else if(c>0) n->r=avl_del(n->r,name,ok);
    else {
        *ok=1;
        if(!n->l||!n->r){ AVLNode *t=n->l?n->l:n->r; free(n); return t; }
        AVLNode *s=avl_min(n->r);
        strncpy(n->name,s->name,NAME_LEN-1); n->id=s->id;
        n->r=avl_del(n->r,s->name,ok);
    }
    return avl_bal(n);
}

typedef struct { AVLNode *root; } AVLTree;

/* ════════════════════════════════════════════
   HASH MAP (encadeamento externo, FNV-1a)
   ════════════════════════════════════════════ */

typedef struct HEntry { char email[EMAIL_LEN]; int id; struct HEntry *next; } HEntry;
typedef struct { HEntry *b[HASH_SIZE]; int size; } HashMap;

static unsigned hfnv(const char *s) {
    unsigned h=2166136261u;
    while(*s){ h^=(unsigned char)*s++; h*=16777619u; }
    return h % HASH_SIZE;
}

static void hm_insert(HashMap *m, const char *email, int id) {
    unsigned i=hfnv(email);
    HEntry *e=m->b[i];
    while(e){ if(!strcmp(e->email,email)){e->id=id;return;} e=e->next; }
    HEntry *ne=malloc(sizeof(HEntry));
    strncpy(ne->email,email,EMAIL_LEN-1); ne->email[EMAIL_LEN-1]=0;
    ne->id=id; ne->next=m->b[i]; m->b[i]=ne; m->size++;
}

static int hm_get(HashMap *m, const char *email) {
    HEntry *e=m->b[hfnv(email)];
    while(e){ if(!strcmp(e->email,email)) return e->id; e=e->next; }
    return -1;
}

static void hm_del(HashMap *m, const char *email) {
    unsigned i=hfnv(email);
    HEntry **pp=&m->b[i];
    while(*pp){ if(!strcmp((*pp)->email,email)){HEntry *d=*pp;*pp=d->next;free(d);m->size--;return;} pp=&(*pp)->next; }
}

/* ════════════════════════════════════════════
   BANCO DE DADOS
   ════════════════════════════════════════════ */

typedef struct {
    BTree   *bt;
    AVLTree  avl;
    HashMap  hm;
    int      count;
} DB;

static DB *db_new(void) {
    DB *db = calloc(1, sizeof(DB));
    db->bt = bt_create();
    return db;
}

static int db_insert(DB *db, int id, const char *name, const char *email) {
    if (bt_search(db->bt->root, id)) return 0; /* duplicado */
    User *u = malloc(sizeof(User));
    u->id = id;
    strncpy(u->name,  name,  NAME_LEN-1);  u->name[NAME_LEN-1]=0;
    strncpy(u->email, email, EMAIL_LEN-1); u->email[EMAIL_LEN-1]=0;
    bt_insert(db->bt, u);
    db->avl.root = avl_ins(db->avl.root, name, id);
    hm_insert(&db->hm, email, id);
    db->count++;
    return 1;
}

static User *db_by_id(DB *db, int id) { return bt_search(db->bt->root, id); }

static User *db_by_name(DB *db, const char *name) {
    AVLNode *an = avl_find(db->avl.root, name);
    if (!an) return NULL;
    return bt_search(db->bt->root, an->id);
}

static int db_delete(DB *db, int id) {
    User *u = bt_search(db->bt->root, id);
    if (!u) return 0;
    int ok2=0;
    db->avl.root = avl_del(db->avl.root, u->name, &ok2);
    hm_del(&db->hm, u->email);
    bt_delete(db->bt, id);
    free(u);
    db->count--;
    return 1;
}

/* ════════════════════════════════════════════
   GERAÇÃO DE CSV ALEATÓRIO (C puro)
   ════════════════════════════════════════════ */

static const char *NOMES[] = {
    "Ana","Bruno","Carlos","Daniela","Eduardo","Fernanda","Gabriel","Helena",
    "Igor","Julia","Leonardo","Mariana","Nicolas","Olivia","Pedro","Rafaela",
    "Samuel","Tatiana","Vitor","Beatriz","Caio","Debora","Elias","Fabiana",
    "Gustavo","Ivan","Joao","Karen","Lucas","Melissa","Nathan","Paula","Rafael",
    "Sofia","Thiago","Valentina","Wagner","Yasmin","Zoe","Aldo","Bianca"
};
static const char *SOBRENOMES[] = {
    "Silva","Santos","Oliveira","Souza","Rodrigues","Ferreira","Alves","Pereira",
    "Lima","Gomes","Costa","Ribeiro","Martins","Carvalho","Almeida","Lopes",
    "Sousa","Fernandes","Vieira","Barbosa","Rocha","Dias","Nascimento","Andrade",
    "Moreira","Nunes","Marques","Machado","Mendes","Freitas","Cardoso","Ramos"
};
static const char *DOMINIOS[] = {
    "gmail.com","yahoo.com","hotmail.com","outlook.com","uol.com.br","terra.com.br"
};

#define NOMES_N    (int)(sizeof(NOMES)/sizeof(NOMES[0]))
#define SOBR_N     (int)(sizeof(SOBRENOMES)/sizeof(SOBRENOMES[0]))
#define DOM_N      (int)(sizeof(DOMINIOS)/sizeof(DOMINIOS[0]))

static void gerar_csv(const char *path, int n) {
    /* Fisher-Yates shuffle para ids únicos */
    int total = n * 3;
    int *ids  = malloc(total * sizeof(int));
    for (int i=0;i<total;i++) ids[i]=i+1;
    for (int i=total-1;i>0;i--) { int j=rand()%(i+1); int t=ids[i];ids[i]=ids[j];ids[j]=t; }

    FILE *f = fopen(path, "w");
    fprintf(f, "id,nome,email\n");
    for (int i=0;i<n;i++) {
        const char *nm = NOMES[rand()%NOMES_N];
        const char *sb = SOBRENOMES[rand()%SOBR_N];
        const char *dm = DOMINIOS[rand()%DOM_N];
        int sufixo = rand()%9999+1;
        /* nome composto */
        fprintf(f, "%d,%s %s,%s%s%d@%s\n",
                ids[i], nm, sb,
                nm, sb, sufixo, dm);
    }
    fclose(f);
    free(ids);
}

/* ════════════════════════════════════════════
   CARREGAMENTO DE CSV
   ════════════════════════════════════════════ */

static int db_load_csv(DB *db, const char *path) {
    FILE *f = fopen(path,"r");
    if (!f) { perror("fopen"); return -1; }
    char line[512];
    fgets(line,sizeof(line),f); /* pula header */
    int n=0;
    while(fgets(line,sizeof(line),f)) {
        line[strcspn(line,"\r\n")]=0;
        char *p; char *id_s=strtok_r(line,",",&p);
        char *nm=strtok_r(NULL,",",&p);
        char *em=strtok_r(NULL,",",&p);
        if(!id_s||!nm||!em) continue;
        db_insert(db,atoi(id_s),nm,em);
        n++;
    }
    fclose(f);
    return n;
}

/* ════════════════════════════════════════════
   BENCHMARK
   ════════════════════════════════════════════ */

static void benchmark(DB *db) {
    printf("\n=== BENCHMARK (50.000 registros) ===\n");

    /* coleta todos os registros para amostrar */
    int cap=64, cnt=0;
    User **all = malloc(cap*sizeof(User*));
    bt_range(db->bt->root, 0, 2000000, &all, &cnt, &cap);

    int S = cnt < 1000 ? cnt : 1000;
    int step = cnt / S; if(step<1) step=1;

    /* 1. SELECT WHERE id= */
    {
        double t0=ms_now();
        int found=0;
        for(int i=0;i<cnt&&found<S;i+=step){
            if(bt_search(db->bt->root, all[i]->id)) found++;
        }
        double dt=ms_now()-t0;
        printf("[B-Tree] SELECT id=X       : %d buscas | %.3f ms total | %.5f ms/op\n",
               found, dt, dt/found);
    }

    /* 2. SELECT WHERE id BETWEEN */
    {
        int lo=all[cnt/4]->id, hi=all[cnt*3/4]->id;
        double t0=ms_now();
        int rc=0, trials=20;
        for(int t=0;t<trials;t++){
            int c2=0, cap2=64; User **r=malloc(cap2*sizeof(User*));
            bt_range(db->bt->root,lo,hi,&r,&c2,&cap2);
            rc=c2; free(r);
        }
        double dt=ms_now()-t0;
        printf("[B-Tree] SELECT id BETWEEN : %d trials | ~%d rows/trial | %.3f ms/trial\n",
               trials, rc, dt/trials);
    }

    /* 3. SELECT WHERE nome= */
    {
        double t0=ms_now();
        int found=0;
        for(int i=0;i<cnt&&found<S;i+=step){
            if(avl_find(db->avl.root, all[i]->name)) found++;
        }
        double dt=ms_now()-t0;
        printf("[AVL]    SELECT nome=X      : %d buscas | %.3f ms total | %.5f ms/op\n",
               found, dt, dt/found);
    }

    /* 4. Hash cache */
    {
        double t0=ms_now();
        int found=0;
        for(int i=0;i<cnt&&found<S;i+=step){
            if(hm_get(&db->hm, all[i]->email)>=0) found++;
        }
        double dt=ms_now()-t0;
        printf("[Hash]   SELECT email=X     : %d buscas | %.3f ms total | %.6f ms/op\n",
               found, dt, dt/found);
    }

    free(all);
    printf("=====================================\n\n");
}

/* ════════════════════════════════════════════
   IMPRESSÃO DE REGISTRO
   ════════════════════════════════════════════ */

static void print_user(User *u) {
    printf("  id=%-8d nome=%-30s email=%s\n", u->id, u->name, u->email);
}

/* ════════════════════════════════════════════
   CLI
   ════════════════════════════════════════════ */

static void help(void) {
    puts("Comandos:");
    puts("  INSERT <id> <nome_sem_espaco> <email>");
    puts("  SELECT WHERE id=<n>");
    puts("  SELECT WHERE id BETWEEN <lo> AND <hi>");
    puts("  SELECT WHERE nome=<nome>");
    puts("  DELETE WHERE id=<n>");
    puts("  BENCHMARK");
    puts("  HELP | EXIT");
}

static char *trim(char *s) {
    while(*s==' '||*s=='\t') s++;
    char *e=s+strlen(s)-1;
    while(e>s&&(*e==' '||*e=='\t'||*e=='\n'||*e=='\r')) *e--=0;
    return s;
}

static void cli(DB *db) {
    printf("minidb com %d registros. Digite HELP.\n\n", db->count);
    char line[512];
    while(1) {
        printf("minidb> ");
        fflush(stdout);
        if(!fgets(line,sizeof(line),stdin)) break;
        char *cmd=trim(line);
        if(!*cmd) continue;

        if(!strcasecmp(cmd,"exit")||!strcasecmp(cmd,"quit")) break;
        if(!strcasecmp(cmd,"help")) { help(); continue; }
        if(!strcasecmp(cmd,"benchmark")) { benchmark(db); continue; }

        /* INSERT */
        if(!strncasecmp(cmd,"insert ",7)) {
            int id; char nm[NAME_LEN], em[EMAIL_LEN];
            if(sscanf(cmd+7,"%d %63s %127s",&id,nm,em)==3) {
                double t0=ms_now();
                int ok=db_insert(db,id,nm,em);
                if(ok) printf("OK (%.4f ms)\n", ms_now()-t0); else printf("ERRO: id=%d ja existe\n", id);
            } else puts("Uso: INSERT <id> <nome> <email>");
            continue;
        }

        /* DELETE */
        if(!strncasecmp(cmd,"delete where id=",16)) {
            int id=atoi(cmd+16);
            double t0=ms_now();
            int ok=db_delete(db,id);
            printf(ok?"OK removido id=%d (%.4f ms)\n":"ERRO: id=%d nao encontrado\n", id, ms_now()-t0);
            continue;
        }

        /* SELECT id= */
        if(!strncasecmp(cmd,"select where id=",16)) {
            int id=atoi(cmd+16);
            double t0=ms_now();
            User *u=db_by_id(db,id);
            if(u) print_user(u); else puts("  Nao encontrado.");
            printf("  (%.4f ms)\n", ms_now()-t0);
            continue;
        }

        /* SELECT BETWEEN */
        if(!strncasecmp(cmd,"select where id between ",24)) {
            int lo,hi;
            if(sscanf(cmd+24,"%d and %d",&lo,&hi)==2||sscanf(cmd+24,"%d AND %d",&lo,&hi)==2) {
                int cnt2=0, cap2=64; User **res=malloc(cap2*sizeof(User*));
                double t0=ms_now();
                bt_range(db->bt->root,lo,hi,&res,&cnt2,&cap2);
                double dt=ms_now()-t0;
                printf("  %d resultado(s) (%.4f ms):\n", cnt2, dt);
                int show=cnt2>20?20:cnt2;
                for(int i=0;i<show;i++) print_user(res[i]);
                if(cnt2>20) printf("  ... e mais %d\n",cnt2-20);
                free(res);
            } else puts("Uso: SELECT WHERE id BETWEEN <lo> AND <hi>");
            continue;
        }

        /* SELECT nome= */
        if(!strncasecmp(cmd,"select where nome=",18)) {
            char *name=trim(cmd+18);
            double t0=ms_now();
            User *u=db_by_name(db,name);
            if(u) print_user(u); else puts("  Nao encontrado.");
            printf("  (%.4f ms)\n", ms_now()-t0);
            continue;
        }

        puts("Comando desconhecido. Digite HELP.");
    }
    puts("Ate logo!");
}

/* ════════════════════════════════════════════
   MAIN
   ════════════════════════════════════════════ */

int main(int argc, char *argv[]) {
    srand((unsigned)time(NULL));

    const char *csv = argc >= 2 ? argv[1] : "users.csv";

    /* gera CSV se não existir */
    FILE *test = fopen(csv,"r");
    if (!test) {
        printf("Gerando %s com 50000 registros...\n", csv);
        gerar_csv(csv, 50000);
        printf("Pronto.\n");
    } else {
        fclose(test);
    }

    DB *db = db_new();
    printf("Carregando %s...\n", csv);
    double t0 = ms_now();
    int n = db_load_csv(db, csv);
    printf("%d registros carregados em %.2f ms\n", n, ms_now()-t0);

    benchmark(db);
    cli(db);
    return 0;
}
