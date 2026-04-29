/*
 * ============================================================
 *  SOLVER DE LIMITES EM C  -  Passo a passo + L'Hopital
 * ============================================================
 *  Compile:  gcc -o limites limites_final.c -lm
 *  Execute:  ./limites
 * ============================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

/* ── Tipos ── */
typedef enum { NUM,VAR,ADD,SUB,MUL,DIV,POW,SIN,COS,TAN,EXP_,LN,SQRT,NEG } NT;

typedef struct N { NT t; double v; struct N *L,*R; } N;

/* ── Pool de nos ── */
#define PSIZ 32768
#define MAX_EXPR 512
static N pool[PSIZ];
static int pt=0;

static N* nn(NT t){ N*n=&pool[pt++]; n->t=t;n->v=0;n->L=n->R=NULL; return n; }
static N* num(double v){ N*n=nn(NUM);n->v=v;return n; }
static N* var_(void){ return nn(VAR); }
static N* bin(NT t,N*l,N*r){ N*n=nn(t);n->L=l;n->R=r;return n; }
static N* un(NT t,N*c){ N*n=nn(t);n->L=c;return n; }

/* ── Parser (nao reseta pool) ── */
static const char *S; static int P;
static void ws(void){ while(S[P]&&isspace((unsigned char)S[P]))P++; }
static N* pE(void); static N* pT(void); static N* pPow(void); static N* pU(void); static N* pPri(void);

static N* pE(void){
    N*l=pT();ws();
    while(S[P]=='+'||S[P]=='-'){char o=S[P++];N*r=pT();l=bin(o=='+'?ADD:SUB,l,r);ws();}
    return l;
}
/* Verifica se o proximo token pode iniciar um fator (multiplicacao implicita) */
static int can_start_factor(void){
    ws();
    return isdigit((unsigned char)S[P]) || S[P]=='.' ||
           isalpha((unsigned char)S[P]) || S[P]=='(';
}

static N* pT(void){
    N*l=pPow();ws();
    for(;;){
        if(S[P]=='*'||S[P]=='/'){
            char o=S[P++];N*r=pPow();l=bin(o=='*'?MUL:DIV,l,r);ws();
        } else if(can_start_factor()){
            /* Multiplicacao implicita: 3x, 2(x+1), 3sin(x), etc. */
            N*r=pPow(); l=bin(MUL,l,r); ws();
        } else break;
    }
    return l;
}
static N* pPow(void){
    N*b=pU();ws();
    if(S[P]=='^'){P++;N*e=pU();return bin(POW,b,e);}
    return b;
}
static N* pU(void){
    ws();
    if(S[P]=='-'){P++;return un(NEG,pU());}
    if(S[P]=='+'){P++;return pU();}
    return pPri();
}
static N* pPri(void){
    ws();
    if(S[P]=='('){P++;N*n=pE();ws();if(S[P]==')')P++;return n;}
    if(isdigit((unsigned char)S[P])||S[P]=='.'){
        char b[64];int i=0;
        while(isdigit((unsigned char)S[P])||S[P]=='.')b[i++]=S[P++];
        b[i]='\0'; return num(atof(b));
    }
    if(isalpha((unsigned char)S[P])){
        char nm[32];int i=0;
        while(isalpha((unsigned char)S[P]))nm[i++]=S[P++]; nm[i]='\0';
        if(!strcmp(nm,"e")) return num(M_E);
        if(!strcmp(nm,"pi")) return num(M_PI);
        if(!strcmp(nm,"x")) return var_();
        ws();
        if(S[P]=='('){
            P++; N*a=pE(); ws(); if(S[P]==')')P++;
            NT t;
            if(!strcmp(nm,"sin"))t=SIN;
            else if(!strcmp(nm,"cos"))t=COS;
            else if(!strcmp(nm,"tan"))t=TAN;
            else if(!strcmp(nm,"exp"))t=EXP_;
            else if(!strcmp(nm,"ln"))t=LN;
            else if(!strcmp(nm,"sqrt"))t=SQRT;
            else{fprintf(stderr,"Funcao desconhecida: %s\n",nm);exit(1);}
            return un(t,a);
        }
    }
    fprintf(stderr,"Erro de parse em pos %d: '%.10s'\n",P,S+P); exit(1);
}

/* Faz parse SEM resetar pool */
static N* parse_keep(const char *expr){
    S=expr; P=0; return pE();
}

/* ── Avaliacao ── */
static double ev(N*n,double x){
    if(!n)return 0;
    switch(n->t){
        case NUM: return n->v;
        case VAR: return x;
        case ADD: return ev(n->L,x)+ev(n->R,x);
        case SUB: return ev(n->L,x)-ev(n->R,x);
        case MUL: return ev(n->L,x)*ev(n->R,x);
        case DIV: return ev(n->L,x)/ev(n->R,x);
        case POW: return pow(ev(n->L,x),ev(n->R,x));
        case NEG: return -ev(n->L,x);
        case SIN: return sin(ev(n->L,x));
        case COS: return cos(ev(n->L,x));
        case TAN: return tan(ev(n->L,x));
        case EXP_:return exp(ev(n->L,x));
        case LN:  return log(ev(n->L,x));
        case SQRT:return sqrt(ev(n->L,x));
        default:  return NAN;
    }
}

/* ── Impressao ── */
static void pe(N*n){
    if(!n)return;
    switch(n->t){
        case NUM: if(n->v==(long long)n->v)printf("%.0f",n->v);else printf("%.4g",n->v);break;
        case VAR: printf("x");break;
        case NEG: printf("-(");pe(n->L);printf(")");break;
        case ADD: printf("(");pe(n->L);printf(" + ");pe(n->R);printf(")");break;
        case SUB: printf("(");pe(n->L);printf(" - ");pe(n->R);printf(")");break;
        case MUL: printf("(");pe(n->L);printf(" * ");pe(n->R);printf(")");break;
        case DIV: printf("(");pe(n->L);printf(" / ");pe(n->R);printf(")");break;
        case POW: printf("(");pe(n->L);printf("^");pe(n->R);printf(")");break;
        case SIN: printf("sin(");pe(n->L);printf(")");break;
        case COS: printf("cos(");pe(n->L);printf(")");break;
        case TAN: printf("tan(");pe(n->L);printf(")");break;
        case EXP_:printf("exp(");pe(n->L);printf(")");break;
        case LN:  printf("ln(");pe(n->L);printf(")");break;
        case SQRT:printf("sqrt(");pe(n->L);printf(")");break;
    }
}

/* ── Derivacao simbolica ── */
static N* der(N*n){
    if(!n)return num(0);
    switch(n->t){
        case NUM: return num(0);
        case VAR: return num(1);
        case NEG: return un(NEG,der(n->L));
        case ADD: return bin(ADD,der(n->L),der(n->R));
        case SUB: return bin(SUB,der(n->L),der(n->R));
        case MUL: return bin(ADD,bin(MUL,der(n->L),n->R),bin(MUL,n->L,der(n->R)));
        case DIV: return bin(DIV,bin(SUB,bin(MUL,der(n->L),n->R),bin(MUL,n->L,der(n->R))),bin(POW,n->R,num(2)));
        case POW:
            if(n->R->t==NUM){double c=n->R->v;return bin(MUL,bin(MUL,num(c),bin(POW,n->L,num(c-1))),der(n->L));}
            return num(NAN);
        case SIN: return bin(MUL,un(COS,n->L),der(n->L));
        case COS: return bin(MUL,un(NEG,un(SIN,n->L)),der(n->L));
        case TAN: return bin(MUL,bin(DIV,num(1),bin(POW,un(COS,n->L),num(2))),der(n->L));
        case EXP_:return bin(MUL,n,der(n->L));
        case LN:  return bin(DIV,der(n->L),n->L);
        case SQRT:return bin(DIV,der(n->L),bin(MUL,num(2),un(SQRT,n->L)));
        default:  return num(NAN);
    }
}

#define EPS  1e-9
#define BIG  1e13
#define IZ(v) (fabs(v)<EPS)
#define II(v) (fabs(v)>BIG)

typedef enum {BIL,ESQ,DIR} Side;

static void sep(void){puts("  -----------------------------------------");}

static double run(const char*fs,const char*gs,double a,Side side,int vb){
    int hd=(gs&&strlen(gs)>0);

    /* ── Parse (sem resetar pool) ── */
    int snap=pt;
    N*f=parse_keep(fs);
    N*g=hd?parse_keep(gs):NULL;

    if(vb){
        printf("\n"); sep();
        printf("  RESOLVENDO O LIMITE\n"); sep();
        printf("  lim  ");
        if(hd){printf("[ "); pe(f); printf(" ] / [ "); pe(g); printf(" ]");}
        else pe(f);
        printf("\n");
        if(!isinf(a)) printf("  x -> %.6g%s\n",a,side==ESQ?"^-":side==DIR?"^+":"");
        else printf("  x -> %sinf\n",a>0?"+":"-");
        sep();
    }

    /* ======================================================
       PASSO 1 - Substituicao direta
    ====================================================== */
    if(vb) printf("\n  [PASSO 1] Substituicao direta\n");

    /* Caso especial: limite no infinito */
    if(isinf(a)){
        double sg=a>0?1:-1;
        double xb=sg*1e12;
        double fv=ev(f,xb);
        double gv=hd?ev(g,xb):1.0;
        if(vb){
            printf("  Testando em x = %.2e:\n",xb);
            printf("  f(x) ~ %-12.5g", fv);
            if(hd) printf("  g(x) ~ %-12.5g", gv);
            printf("\n");
        }
        double res;
        if(hd){
            if(!IZ(gv)){ res=fv/gv; }
            else{ if(vb)printf("  Forma inf/inf detectada\n"); goto LH; }
        } else res=fv;
        if(vb){sep();if(isinf(res))printf("  RESULTADO: %sinfinito\n",res>0?"+":"-");else printf("  RESULTADO: %.8g\n",res);sep();}
        return res;
    }

    {
        double fa=ev(f,a);
        double ga=hd?ev(g,a):1.0;
        if(vb){
            printf("  Substituindo x = %.6g:\n",a);
            printf("  f(%.4g) = %.6g\n",a,fa);
            if(hd) printf("  g(%.4g) = %.6g\n",a,ga);
        }

        if(!hd){
            if(!isnan(fa)&&!isinf(fa)){if(vb){sep();printf("  RESULTADO: %.8g\n",fa);sep();}return fa;}
            if(isinf(fa)){if(vb){sep();printf("  RESULTADO: %sinfinito\n",fa>0?"+":"-");sep();}return fa;}
            goto NUM_EST;
        }

        if(!IZ(ga)&&!isnan(fa)&&!isnan(ga)){
            double res=fa/ga;
            if(vb){printf("  Substituicao direta OK!\n  %.6g / %.6g = %.6g\n",fa,ga,res);sep();printf("  RESULTADO: %.8g\n",res);sep();}
            return res;
        }
        if(IZ(fa)&&IZ(ga)){if(vb)printf("  Forma 0/0 -> L'Hopital\n");goto LH;}
        if(II(fa)&&II(ga)){if(vb)printf("  Forma inf/inf -> L'Hopital\n");goto LH;}
        if(IZ(fa)&&II(ga)){if(vb){printf("  Forma 0/inf -> 0\n");sep();printf("  RESULTADO: 0\n");sep();}return 0.0;}
        if(!IZ(fa)&&IZ(ga)){
            if(vb)printf("  Denominador -> 0, numerador != 0 -> divergencia\n");
            double ep=1e-8;
            double lp=ev(f,a+ep)/ev(g,a+ep);
            double lm=ev(f,a-ep)/ev(g,a-ep);
            if(vb)printf("  L^- ~ %.4g   L^+ ~ %.4g\n",lm,lp);
            double res;
            if(lp*lm>0){res=lp>0?INFINITY:-INFINITY;if(vb)printf("  Mesmo sinal -> %sinfinito\n",res>0?"+":"-");}
            else{res=NAN;if(vb)printf("  Sinais opostos -> NAO EXISTE\n");}
            if(vb){sep();if(isnan(res))printf("  RESULTADO: Nao existe\n");else printf("  RESULTADO: %sinfinito\n",res>0?"+":"-");sep();}
            return res;
        }
    }

    /* ======================================================
       PASSO 2 - Regra de L'Hopital
    ====================================================== */
    LH:;{
        if(vb){
            printf("\n  [PASSO 2] Regra de L'Hopital\n");
            printf("  Derivar numerador e denominador separadamente:\n\n");
        }
        /* Re-parsear para derivar (pool continua crescendo) */
        N*f2=parse_keep(fs);
        N*g2=hd?parse_keep(gs):NULL;
        N*df=der(f2);
        N*dg=hd?der(g2):num(1);
        if(vb){
            printf("  f'(x) = "); pe(df); printf("\n");
            if(hd){printf("  g'(x) = "); pe(dg); printf("\n");}
        }
        double dfa=ev(df,a);
        double dga=ev(dg,a);
        if(vb){
            printf("\n  Substituindo x = %.6g:\n",a);
            printf("  f'(%.4g) = %.6g\n",a,dfa);
            if(hd) printf("  g'(%.4g) = %.6g\n",a,dga);
        }
        if(hd&&!IZ(dga)){
            double res=dfa/dga;
            if(vb){printf("  L'Hopital => %.6g / %.6g = %.6g\n",dfa,dga,res);sep();printf("  RESULTADO: %.8g\n",res);sep();}
            return res;
        }
        /* Segunda aplicacao */
        if(hd&&IZ(dfa)&&IZ(dga)){
            if(vb)printf("\n  Ainda 0/0 -> 2a aplicacao de L'Hopital\n");
            N*d2f=der(df); N*d2g=der(dg);
            if(vb){printf("  f''(x) = ");pe(d2f);printf("\n  g''(x) = ");pe(d2g);printf("\n");}
            double d2fa=ev(d2f,a),d2ga=ev(d2g,a);
            if(vb){printf("  f''(%.4g) = %.6g\n  g''(%.4g) = %.6g\n",a,d2fa,a,d2ga);}
            if(!IZ(d2ga)){double res=d2fa/d2ga;if(vb){printf("  => %.6g / %.6g = %.6g\n",d2fa,d2ga,res);sep();printf("  RESULTADO: %.8g\n",res);sep();}return res;}
        }
        /* Aplicacao para o infinito */
        if(isinf(a)){
            double sg=a>0?1:-1;double xb=sg*1e12;
            double dfa2=ev(df,xb),dga2=hd?ev(dg,xb):1.0;
            if(hd&&!IZ(dga2)){double res=dfa2/dga2;if(vb){printf("  L'Hopital no inf => %.6g\n",res);sep();printf("  RESULTADO: %.8g\n",res);sep();}return res;}
        }
    }

    /* ======================================================
       PASSO 3 - Estimativa numerica
    ====================================================== */
    NUM_EST:;{
        if(vb)printf("\n  [PASSO 3] Estimativa numerica:\n");
        double eps=1e-7;
        double Lm,Lp;
        /* Calcula pelos dois lados */
        {
            N*ftmp=parse_keep(fs); N*gtmp=hd?parse_keep(gs):NULL;
            if(vb){
                printf("  %18s  %18s\n","x","f(x)/g(x)");
                double deltas[]={-1e-3,-1e-5,-1e-7,1e-7,1e-5,1e-3};
                for(int i=0;i<6;i++){
                    double xv=a+deltas[i];
                    double fv=ev(ftmp,xv);
                    double gv=hd?ev(gtmp,xv):1.0;
                    printf("  %18.10f  %18.10f\n",xv,fv/gv);
                }
            }
            Lm=ev(ftmp,a-eps)/(hd?ev(gtmp,a-eps):1.0);
            Lp=ev(ftmp,a+eps)/(hd?ev(gtmp,a+eps):1.0);
        }
        if(vb)printf("\n  L^- ~ %.8g\n  L^+ ~ %.8g\n",Lm,Lp);
        double res;
        if(side==ESQ)res=Lm;
        else if(side==DIR)res=Lp;
        else{
            if(fabs(Lm-Lp)<1e-4)res=(Lm+Lp)/2.0;
            else{
                if(vb){printf("  Limites laterais diferentes!\n");sep();printf("  RESULTADO: Nao existe\n");sep();}
                return NAN;
            }
        }
        if(vb){
            sep();
            if(isnan(res))printf("  RESULTADO: Nao existe\n");
            else if(isinf(res))printf("  RESULTADO: %sinfinito\n",res>0?"+":"-");
            else printf("  RESULTADO: %.8g\n",res);
            sep();
        }
        pt=snap; /* liberar nos alocados nesta chamada */
        return res;
    }
}

int main(void){
    char fb[MAX_EXPR],gb[MAX_EXPR],ab[64],sb[8];

    printf("\n");
    printf("  +==========================================+\n");
    printf("  |       SOLVER DE LIMITES EM C            |\n");
    printf("  |  Passo a passo + L'Hopital + Derivacao  |\n");
    printf("  +==========================================+\n\n");
    printf("  Variavel: x\n");
    printf("  Operadores: + - * / ^ e parenteses\n");
    printf("  Funcoes:    sin  cos  tan  exp  ln  sqrt\n");
    printf("  Constantes: e,  pi\n");
    printf("  Ponto:      numero,  inf,  -inf\n\n");
    printf("  Exemplos rapidos:\n");
    printf("    (x^2 - 1) / (x - 1)  em x=1  -> 2\n");
    printf("    sin(x) / x            em x=0  -> 1\n");
    printf("    (3*x^2) / (5*x^2-1)  em x=inf -> 0.6\n\n");

    while(1){
        printf("  ==========================================\n");
        printf("  Numerador f(x)               : ");
        if(!fgets(fb,MAX_EXPR,stdin))break;
        fb[strcspn(fb,"\n")]='\0';
        if(!strlen(fb))break;

        printf("  Denominador g(x) [Enter=sem] : ");
        if(!fgets(gb,MAX_EXPR,stdin))break;
        gb[strcspn(gb,"\n")]='\0';

        printf("  Ponto (numero/inf/-inf)      : ");
        if(!fgets(ab,sizeof(ab),stdin))break;
        ab[strcspn(ab,"\n")]='\0';

        double a;
        if(!strcmp(ab,"inf")||!strcmp(ab,"+inf"))a=INFINITY;
        else if(!strcmp(ab,"-inf"))a=-INFINITY;
        else a=atof(ab);

        printf("  Lateral? b=bilateral e=esq d=dir [b] : ");
        if(!fgets(sb,sizeof(sb),stdin))break;
        sb[strcspn(sb,"\n")]='\0';

        Side side=BIL;
        if(sb[0]=='e'||sb[0]=='E')side=ESQ;
        if(sb[0]=='d'||sb[0]=='D')side=DIR;

        pt=0; /* reset pool para cada calculo */
        double res=run(fb,strlen(gb)>0?gb:NULL,a,side,1);

        printf("\n  >>> RESPOSTA FINAL: ");
        if(isnan(res))printf("Limite nao existe\n\n");
        else if(isinf(res))printf("%sinfinito\n\n",res>0?"+":"-");
        else printf("%.8g\n\n",res);

        printf("  Calcular outro? (s/n) [s]: ");
        char resp[4];
        if(!fgets(resp,sizeof(resp),stdin))break;
        if(resp[0]=='n'||resp[0]=='N')break;
    }
    printf("  Ate logo!\n\n");
    return 0;
}
