/* main.c – CLI + benchmark for the MiniDB */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include "minidb.h"

#define ANSI_BOLD    "\033[1m"
#define ANSI_CYAN    "\033[36m"
#define ANSI_GREEN   "\033[32m"
#define ANSI_YELLOW  "\033[33m"
#define ANSI_RED     "\033[31m"
#define ANSI_MAGENTA "\033[35m"
#define ANSI_RESET   "\033[0m"

/* ─── Pretty print a user row ─────────────────────────────────── */
static void print_user(const User *u) {
    printf("  " ANSI_CYAN "id=%-8d" ANSI_RESET
           " nome=%-30s email=%s\n",
           u->id, u->name, u->email);
}

static void print_header(const char *title) {
    printf(ANSI_BOLD "\n┌──────────────────────────────────────┐\n"
           "│ %-36s │\n"
           "└──────────────────────────────────────┘\n" ANSI_RESET, title);
}

/* ─── Trim whitespace ─────────────────────────────────────────── */
static char *trim(char *s) {
    while (isspace((unsigned char)*s)) s++;
    char *e = s + strlen(s) - 1;
    while (e > s && isspace((unsigned char)*e)) *e-- = '\0';
    return s;
}

/* ═══════════════════════════════════════════════════════════════
   BENCHMARK
   ═══════════════════════════════════════════════════════════════ */
static void run_benchmark(Database *db, int n_records) {
    print_header("BENCHMARK DE PERFORMANCE");

    /* Sample ids to query – pick spread-out ones */
    int sample = 1000;
    int *sample_ids = malloc(sample * sizeof(int));
    {
        int cnt = 0;
        User **all = btree_range(db->id_index, 0, INT32_MAX, &cnt);
        if (cnt > 0) {
            int step = cnt / sample;
            if (step < 1) step = 1;
            int taken = 0;
            for (int i = 0; i < cnt && taken < sample; i += step)
                sample_ids[taken++] = all[i]->id;
            sample = taken;
        }
        free(all);
    }

    /* ── 1. SELECT WHERE id= ──────────────────────────────── */
    {
        double t0 = now_ms();
        int found = 0;
        for (int i = 0; i < sample; i++) {
            User *u = db_select_by_id(db, sample_ids[i]);
            if (u) found++;
        }
        double dt = now_ms() - t0;
        printf(ANSI_GREEN "  [B-Tree] SELECT id=X          " ANSI_RESET
               ": %d buscas | encontrados=%d | "
               ANSI_YELLOW "%.3f ms total" ANSI_RESET
               " | %.4f ms/busca\n",
               sample, found, dt, dt / sample);
    }

    /* ── 2. SELECT WHERE id BETWEEN ──────────────────────── */
    {
        int cnt_all = 0;
        User **all = btree_range(db->id_index, 0, INT32_MAX, &cnt_all);
        int lo = all[cnt_all / 4]->id;
        int hi = all[cnt_all * 3 / 4]->id;
        free(all);

        int trials = 20;
        double t0 = now_ms();
        int total_found = 0;
        for (int i = 0; i < trials; i++) {
            int cnt = 0;
            User **res = db_select_range(db, lo, hi, &cnt);
            total_found += cnt;
            free(res);
        }
        double dt = now_ms() - t0;
        printf(ANSI_GREEN "  [B-Tree] SELECT id BETWEEN    " ANSI_RESET
               ": %d trials | ~%d registros/trial | "
               ANSI_YELLOW "%.3f ms total" ANSI_RESET
               " | %.3f ms/trial\n",
               trials, total_found / trials, dt, dt / trials);
    }

    /* ── 3. SELECT WHERE nome= ───────────────────────────── */
    {
        /* collect some names */
        int cnt_all = 0;
        User **all = btree_range(db->id_index, 0, INT32_MAX, &cnt_all);
        int ns = (cnt_all < sample) ? cnt_all : sample;
        char **names = malloc(ns * sizeof(char *));
        int step = cnt_all / ns; if (step < 1) step = 1;
        int taken = 0;
        for (int i = 0; i < cnt_all && taken < ns; i += step)
            names[taken++] = all[i]->name;
        ns = taken;
        free(all);

        double t0 = now_ms();
        int found = 0;
        for (int i = 0; i < ns; i++) {
            User *u = db_select_by_name(db, names[i]);
            if (u) found++;
        }
        double dt = now_ms() - t0;
        printf(ANSI_GREEN "  [AVL]    SELECT nome=X         " ANSI_RESET
               ": %d buscas | encontrados=%d | "
               ANSI_YELLOW "%.3f ms total" ANSI_RESET
               " | %.4f ms/busca\n",
               ns, found, dt, dt / ns);
        free(names);
    }

    /* ── 4. Hash cache hit ───────────────────────────────── */
    {
        int cnt_all = 0;
        User **all = btree_range(db->id_index, 0, INT32_MAX, &cnt_all);
        int es = (cnt_all < sample) ? cnt_all : sample;
        char **emails = malloc(es * sizeof(char *));
        int step = cnt_all / es; if (step < 1) step = 1;
        int taken = 0;
        for (int i = 0; i < cnt_all && taken < es; i += step)
            emails[taken++] = all[i]->email;
        es = taken;
        free(all);

        double t0 = now_ms();
        int found = 0;
        for (int i = 0; i < es; i++) {
            int id = hashmap_get(db->email_cache, emails[i]);
            if (id >= 0) found++;
        }
        double dt = now_ms() - t0;
        printf(ANSI_GREEN "  [Hash]   SELECT email=X (cache)" ANSI_RESET
               ": %d buscas | hits=%d | "
               ANSI_YELLOW "%.3f ms total" ANSI_RESET
               " | %.5f ms/busca\n",
               es, found, dt, dt / es);
        free(emails);
    }

    printf(ANSI_MAGENTA "\n  Estatísticas do Hash:\n" ANSI_RESET);
    hashmap_stats(db->email_cache);
    printf(ANSI_MAGENTA "  B-Tree records: %d  |  AVL nodes: %d\n" ANSI_RESET,
           db->id_index->size, db->name_index->size);
    printf("\n");
    free(sample_ids);
}

/* ═══════════════════════════════════════════════════════════════
   CLI LOOP
   ═══════════════════════════════════════════════════════════════ */
static void cli_help(void) {
    printf(ANSI_BOLD
        "\n  Comandos disponíveis:\n" ANSI_RESET
        "  INSERT <id> <nome> <email>\n"
        "  SELECT WHERE id=<n>\n"
        "  SELECT WHERE id BETWEEN <lo> AND <hi>\n"
        "  SELECT WHERE nome=<nome>\n"
        "  DELETE WHERE id=<n>\n"
        "  STATS\n"
        "  BENCHMARK\n"
        "  HELP\n"
        "  EXIT\n\n");
}

static void run_cli(Database *db, int n_loaded) {
    printf(ANSI_BOLD ANSI_CYAN
           "\n  ╔══════════════════════════════════════╗\n"
           "  ║       MiniDB – Motor de Índices      ║\n"
           "  ║  B-Tree · AVL · HashMap · CLI        ║\n"
           "  ╚══════════════════════════════════════╝\n\n"
           ANSI_RESET);

    printf("  %d registros carregados.\n", n_loaded);
    cli_help();

    char line[512];
    while (1) {
        printf(ANSI_BOLD "minidb> " ANSI_RESET);
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) break;
        char *cmd = trim(line);
        if (!*cmd) continue;

        /* ── EXIT ── */
        if (strcasecmp(cmd, "exit") == 0 || strcasecmp(cmd, "quit") == 0)
            break;

        /* ── HELP ── */
        if (strcasecmp(cmd, "help") == 0) { cli_help(); continue; }

        /* ── STATS ── */
        if (strcasecmp(cmd, "stats") == 0) {
            printf(ANSI_MAGENTA "  Registros: %d\n" ANSI_RESET,
                   db->record_count);
            hashmap_stats(db->email_cache);
            continue;
        }

        /* ── BENCHMARK ── */
        if (strcasecmp(cmd, "benchmark") == 0) {
            run_benchmark(db, db->record_count);
            continue;
        }

        /* ── INSERT ── */
        if (strncasecmp(cmd, "insert ", 7) == 0) {
            char *rest = trim(cmd + 7);
            int id;
            char name[MAX_NAME], email[MAX_EMAIL];
            /* parse: id "name with spaces" email */
            /* simple: id nome email (no spaces in nome for CLI) */
            if (sscanf(rest, "%d %63s %127s", &id, name, email) == 3) {
                double t0 = now_ms();
                int ok = db_insert(db, id, name, email);
                double dt = now_ms() - t0;
                if (ok)
                    printf(ANSI_GREEN "  OK" ANSI_RESET
                           " inserido id=%d (%.4f ms)\n", id, dt);
                else
                    printf(ANSI_RED "  ERRO:" ANSI_RESET
                           " id=%d já existe\n", id);
            } else {
                printf("  Uso: INSERT <id> <nome_sem_espaços> <email>\n");
            }
            continue;
        }

        /* ── DELETE WHERE id= ── */
        if (strncasecmp(cmd, "delete where id=", 16) == 0) {
            int id = atoi(cmd + 16);
            double t0 = now_ms();
            int ok = db_delete(db, id);
            double dt = now_ms() - t0;
            if (ok)
                printf(ANSI_GREEN "  OK" ANSI_RESET
                       " removido id=%d (%.4f ms)\n", id, dt);
            else
                printf(ANSI_RED "  ERRO:" ANSI_RESET
                       " id=%d não encontrado\n", id);
            continue;
        }

        /* ── SELECT WHERE id= ── */
        if (strncasecmp(cmd, "select where id=", 16) == 0) {
            int id = atoi(cmd + 16);
            double t0 = now_ms();
            User *u = db_select_by_id(db, id);
            double dt = now_ms() - t0;
            if (u) { print_user(u); }
            else   { printf("  Nenhum registro encontrado.\n"); }
            printf("  " ANSI_YELLOW "(%.4f ms)\n" ANSI_RESET, dt);
            continue;
        }

        /* ── SELECT WHERE id BETWEEN lo AND hi ── */
        if (strncasecmp(cmd, "select where id between ", 24) == 0) {
            int lo, hi;
            if (sscanf(cmd + 24, "%d and %d", &lo, &hi) == 2 ||
                sscanf(cmd + 24, "%d AND %d", &lo, &hi) == 2) {
                int cnt = 0;
                double t0 = now_ms();
                User **res = db_select_range(db, lo, hi, &cnt);
                double dt = now_ms() - t0;
                printf("  %d resultado(s) " ANSI_YELLOW "(%.4f ms):\n" ANSI_RESET, cnt, dt);
                int show = cnt > 20 ? 20 : cnt;
                for (int i = 0; i < show; i++) print_user(res[i]);
                if (cnt > 20)
                    printf("  ... e mais %d registros.\n", cnt - 20);
                free(res);
            } else {
                printf("  Uso: SELECT WHERE id BETWEEN <lo> AND <hi>\n");
            }
            continue;
        }

        /* ── SELECT WHERE nome= ── */
        if (strncasecmp(cmd, "select where nome=", 18) == 0) {
            char *name = trim(cmd + 18);
            double t0 = now_ms();
            User *u = db_select_by_name(db, name);
            double dt = now_ms() - t0;
            if (u) { print_user(u); }
            else   { printf("  Nenhum registro encontrado.\n"); }
            printf("  " ANSI_YELLOW "(%.4f ms)\n" ANSI_RESET, dt);
            continue;
        }

        printf(ANSI_RED "  Comando desconhecido." ANSI_RESET
               " Digite HELP para ajuda.\n");
    }

    printf("\n  Até logo!\n\n");
}

/* ═══════════════════════════════════════════════════════════════
   MAIN
   ═══════════════════════════════════════════════════════════════ */
int main(int argc, char *argv[]) {
    const char *csv = "data/users.csv";
    if (argc >= 2) csv = argv[1];

    Database *db = db_create();

    /* ─ Load CSV ─ */
    printf(ANSI_BOLD "  Carregando '%s'...\n" ANSI_RESET, csv);
    double t0 = now_ms();
    int loaded = db_load_csv(db, csv);
    double dt  = now_ms() - t0;

    if (loaded < 0) {
        fprintf(stderr, "  Falha ao abrir CSV.\n");
        db_free(db);
        return 1;
    }
    printf(ANSI_GREEN "  %d registros carregados em %.2f ms\n\n" ANSI_RESET,
           loaded, dt);

    /* ─ Auto-benchmark then CLI ─ */
    run_benchmark(db, loaded);
    run_cli(db, loaded);

    db_free(db);
    return 0;
}
