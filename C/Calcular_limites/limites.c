/*
 * ============================================================
 *  SOLVER DE LIMITES EM C  -  Passo a passo + L'Hopital
 * ============================================================
 *  Compile:  gcc -o limites limites_final.c -lm
 *  Execute:  ./limites
 * ============================================================
 */

#include "limites.h"

int main(void){
    char fb[MAX_EXPR], gb[MAX_EXPR], ab[64], sb[8];
 
    printf("\n");
    printf("  +==========================================+\n");
    printf("  |       SOLVER DE LIMITES EM C            |\n");
    printf("  |  Passo a passo + L'Hopital + Derivacao  |\n");
    printf("  +==========================================+\n\n");
    printf("  Variavel : x\n");
    printf("  Operadores: + - * / ^ e parenteses\n");
    printf("  Funcoes  : sin  cos  tan  exp  ln  sqrt\n");
    printf("  Constantes: e,  pi\n");
    printf("  Ponto    : numero,  inf,  -inf\n\n");
    printf("  Exemplos:\n");
    printf("    x^2 - 1   /   x - 1       em x=1   -> 2\n");
    printf("    sin(x)    /   x            em x=0   -> 1\n");
    printf("    3x^2+2x   /   5x^2-1       em x=inf -> 0.6\n");
    printf("    2^x                        em x=3   -> 8\n");
    printf("    x^x                        em x=1   -> 1\n");
    printf("    sqrt(x)                    em x=0   -> 0\n\n");
 
    while(1){
        printf("  ==========================================\n");
        printf("  Numerador f(x)               : ");
        if(!fgets(fb,MAX_EXPR,stdin)) break;
        fb[strcspn(fb,"\n")]='\0';
        if(!strlen(fb)) break;
 
        printf("  Denominador g(x) [Enter=sem] : ");
        if(!fgets(gb,MAX_EXPR,stdin)) break;
        gb[strcspn(gb,"\n")]='\0';
        // Trim leading and trailing whitespace from gb
        char *gb_start = gb;
        while(isspace((unsigned char)*gb_start)) gb_start++;
        char *gb_end = gb_start + strlen(gb_start);
        while(gb_end > gb_start && isspace((unsigned char)*(gb_end-1))) gb_end--;
        *gb_end = '\0';
 
        printf("  Ponto (numero/inf/-inf)      : ");
        if(!fgets(ab,sizeof(ab),stdin)) break;
        ab[strcspn(ab,"\n")]='\0';
 
        double a;
        if(!strcmp(ab,"inf")||!strcmp(ab,"+inf")) a=INFINITY;
        else if(!strcmp(ab,"-inf"))               a=-INFINITY;
        /* Constantes matematicas reconhecidas no ponto de limite */
        else if(!strcmp(ab,"e"))                  a=M_E;
        else if(!strcmp(ab,"pi"))                 a=M_PI;
        else if(!strcmp(ab,"-e"))                 a=-M_E;
        else if(!strcmp(ab,"-pi"))                a=-M_PI;
        else {
            /* Tenta avaliar expressao sem variavel (ex: "2*pi", "e^2", "pi/4")
               usando o proprio parser do programa com x=0 irrelevante         */
            // Save parser state to restore it after evaluating the limit point expression,
            // preventing side effects on the main parsing process.
                        int snap_pt = pt;
                        S=ab; P=0;
                        /* Verifica se parece uma expressao valida (tem letras ou operadores) */
            int has_alpha=0;
            for(int ci=0;ab[ci];ci++) if(isalpha((unsigned char)ab[ci])){ has_alpha=1; break; }
            if(has_alpha){
                /* Usa o parser para avaliar (nenhum 'x' esperado no ponto) */
                N* anode = pE();
                a = ev(anode, 0.0);   /* x nao importa pois nao ha VAR */
                pt = snap_pt;
                if(isnan(a)){
                    fprintf(stderr,"  Aviso: ponto '%s' resultou em NAN, usando 0.\n",ab);
                    a=0.0;
                }
            } else {
                a=atof(ab);
            }
        }
 
        printf("  Lateral? b=bilateral e=esq d=dir [b]: ");
        if(!fgets(sb,sizeof(sb),stdin)) break;
        sb[strcspn(sb,"\n")]='\0';
 
        Side side=BIL;
        if(sb[0]=='e'||sb[0]=='E') side=ESQ;
        if(sb[0]=='d'||sb[0]=='D') side=DIR;
 
        pt=0;
        double res = run_with_rewrite(fb, strlen(gb_start)>0?gb_start:NULL, a, side, 1);
 
        printf("\n  >>> RESPOSTA FINAL: ");
        if(isnan(res))      printf("Limite nao existe\n\n");
        else if(isinf(res)) printf("%sinfinito\n\n", res>0?"+":"-");
        else                printf("%.8g\n\n", res);
 
        printf("  Calcular outro? (s/n) [s]: ");
        char resp[4];
        if(!fgets(resp,sizeof(resp),stdin)) break;
        if(resp[0]=='n'||resp[0]=='N') break;
    }
    printf("  Ate logo!\n\n");
    return 0;
}
