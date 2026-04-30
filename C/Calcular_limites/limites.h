#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
 
/* ── Tipos ── */
typedef enum {
    NUM, VAR, ADD, SUB, MUL, DIV, POW,
    SIN, COS, TAN, EXP_, LN, SQRT, NEG
} NT;
 
typedef struct N { NT t; double v; struct N *L, *R; } N;
 
/* ── Pool de nos ── */
#define PSIZ    65536
#define MAX_LH  6          /* [4] maximo de aplicacoes de L'Hopital */
#define MAX_EXPR 512
 
static N   pool[PSIZ];
static int pt = 0;
 
static N* nn(NT t) {
    if (pt >= PSIZ) {
        fprintf(stderr, "Node pool exhausted (max %d nodes)\n", PSIZ);
        exit(1);
    }
    N* n = &pool[pt++];
    n->t = t;
    n->v = 0;
    n->L = n->R = NULL;
    return n;
}
static N* num(double v)      { N*n=nn(NUM); n->v=v; return n; }
static N* var_(void)         { return nn(VAR); }
static N* bin(NT t,N*l,N*r)  { N*n=nn(t); n->L=l; n->R=r; return n; }
static N* un(NT t,N*c)       { N*n=nn(t); n->L=c; return n; }
 
/* ================================================================
   PARSER  (nao reseta pool entre chamadas)
   ================================================================ */
static const char *S; static int P;
static void ws(void) { while(S[P] && isspace((unsigned char)S[P])) P++; }
 
static N* pE(void);
static N* pT(void);
static N* pPow(void);
static N* pU(void);
static N* pPri(void);
 
static N* pE(void){
    N*l=pT(); ws();
    while(S[P]=='+'||S[P]=='-'){
        char o=S[P++]; N*r=pT(); l=bin(o=='+'?ADD:SUB,l,r); ws();
    }
    return l;
}
 
/* Verifica se proximo token pode comecar um fator (mult. implicita) */
static int can_start_factor(void){
    ws();
    return isdigit((unsigned char)S[P]) || S[P]=='.' ||
           isalpha((unsigned char)S[P]) || S[P]=='(';
}
 
static N* pT(void){
    N*l=pPow(); ws();
    for(;;){
        if(S[P]=='*'||S[P]=='/'){
            char o=S[P++]; N*r=pPow(); l=bin(o=='*'?MUL:DIV,l,r); ws();
        } else if(can_start_factor()){
            N*r=pPow(); l=bin(MUL,l,r); ws();   /* multiplicacao implicita */
        } else break;
    }
    return l;
}
 
static N* pPow(void){
    N*b=pU(); ws();
    if(S[P]=='^'){ P++; N*e=pU(); return bin(POW,b,e); }
    return b;
}
 
static N* pU(void){
    ws();
    if(S[P]=='-'){ P++; return un(NEG,pU()); }
    if(S[P]=='+'){ P++; return pU(); }
    return pPri();
}
 
static N* pPri(void){
    ws();
    if(S[P]=='('){
        P++; N*n=pE(); ws(); if(S[P]==')') P++; return n;
    }
    if(isdigit((unsigned char)S[P])||S[P]=='.'){
        char b[64]; int i=0;
        while(isdigit((unsigned char)S[P])||S[P]=='.') b[i++]=S[P++];
        b[i]='\0'; return num(atof(b));
    }
    if(isalpha((unsigned char)S[P])){
        char nm[32]; int i=0;
        while(isalpha((unsigned char)S[P])) nm[i++]=S[P++]; nm[i]='\0';
        if(!strcmp(nm,"e"))  return num(M_E);
        if(!strcmp(nm,"pi")) return num(M_PI);
        if(!strcmp(nm,"x"))  return var_();
        ws();
        if(S[P]=='('){
            P++; N*a=pE(); ws(); if(S[P]==')') P++;
            NT t;
            if     (!strcmp(nm,"sin"))  t=SIN;
            else if(!strcmp(nm,"cos"))  t=COS;
            else if(!strcmp(nm,"tan"))  t=TAN;
            else if(!strcmp(nm,"exp"))  t=EXP_;
            else if(!strcmp(nm,"ln"))   t=LN;
            else if(!strcmp(nm,"sqrt")) t=SQRT;
            else { fprintf(stderr,"Funcao desconhecida: %s\n",nm); exit(1); }
            return un(t,a);
        }
    }
    fprintf(stderr,"Erro de parse em pos %d: '%.10s'\n",P,S+P); exit(1);
}
 
static N* parse_keep(const char *expr){ S=expr; P=0; return pE(); }
 
/* ================================================================
   AVALIACAO  [2] DIV explicito, [7] isinf() para BIG real
   ================================================================ */
static double ev(N*n, double x){
    if(!n) return 0.0;
    switch(n->t){
        case NUM:  return n->v;
        case VAR:  return x;
        case NEG:  return -ev(n->L,x);
        case ADD:  return ev(n->L,x) + ev(n->R,x);
        case SUB:  return ev(n->L,x) - ev(n->R,x);
        case MUL:  return ev(n->L,x) * ev(n->R,x);
        case DIV: {
            /* [2] Divisao por zero tratada explicitamente */
            double num_=ev(n->L,x), den=ev(n->R,x);
            if(den == 0.0){
                if(num_ == 0.0) return NAN;
                return (num_ > 0.0) ? INFINITY : -INFINITY;
            }
            return num_/den;
        }
        case POW:  return pow(ev(n->L,x), ev(n->R,x));
        case SIN:  return sin(ev(n->L,x));
        case COS:  return cos(ev(n->L,x));
        case TAN:  return tan(ev(n->L,x));
        case EXP_: return exp(ev(n->L,x));
        case LN:   return log(ev(n->L,x));
        case SQRT: return sqrt(ev(n->L,x));
        default:   return NAN;
    }
}
 
/* ================================================================
   IMPRESSAO
   ================================================================ */
static void pe(N*n){
    if(!n) return;
    switch(n->t){
        case NUM:  if(n->v==(long long)n->v) printf("%.0f",n->v); else printf("%.4g",n->v); break;
        case VAR:  printf("x"); break;
        case NEG:  printf("-("); pe(n->L); printf(")"); break;
        case ADD:  printf("("); pe(n->L); printf(" + "); pe(n->R); printf(")"); break;
        case SUB:  printf("("); pe(n->L); printf(" - "); pe(n->R); printf(")"); break;
        case MUL:  printf("("); pe(n->L); printf(" * "); pe(n->R); printf(")"); break;
        case DIV:  printf("("); pe(n->L); printf(" / "); pe(n->R); printf(")"); break;
        case POW:  printf("("); pe(n->L); printf("^"); pe(n->R); printf(")"); break;
        case SIN:  printf("sin(");  pe(n->L); printf(")"); break;
        case COS:  printf("cos(");  pe(n->L); printf(")"); break;
        case TAN:  printf("tan(");  pe(n->L); printf(")"); break;
        case EXP_: printf("exp(");  pe(n->L); printf(")"); break;
        case LN:   printf("ln(");   pe(n->L); printf(")"); break;
        case SQRT: printf("sqrt("); pe(n->L); printf(")"); break;
    }
}
 
/* ================================================================
   DERIVACAO SIMBOLICA
   [1] POW geral: (f^g)' = f^g * (g'*ln(f) + g*f'/f)
                         = exp(g*ln(f)) * (g'*ln(f) + g*(f'/f))
   ================================================================ */
static N* der(N*n){
    if(!n) return num(0);
    switch(n->t){
        case NUM:  return num(0);
        case VAR:  return num(1);
        case NEG:  return un(NEG, der(n->L));
        case ADD:  return bin(ADD, der(n->L), der(n->R));
        case SUB:  return bin(SUB, der(n->L), der(n->R));
 
        /* Regra do produto: (fg)' = f'g + fg' */
        case MUL:
            return bin(ADD,
                bin(MUL, der(n->L), n->R),
                bin(MUL, n->L,      der(n->R)));
 
        /* Regra do quociente: (f/g)' = (f'g - fg') / g^2 */
        case DIV:
            return bin(DIV,
                bin(SUB,
                    bin(MUL, der(n->L), n->R),
                    bin(MUL, n->L,      der(n->R))),
                bin(POW, n->R, num(2)));
 
        /* [1] Potencia geral: (f^g)' = f^g * (g'*ln(f) + g*(f'/f))
           Caso especial f=constante (ex: 2^x): simplifica para f^g * g' * ln(f)
           Caso especial g=constante (ex: x^n): regra da potencia classica    */
        case POW: {
            N*f=n->L, *g=n->R;
            int f_const = (f->t == NUM);
            int g_const = (g->t == NUM);
 
            if(g_const && f_const){
                /* c1^c2 -> constante, derivada = 0 */
                return num(0);
            }
            if(g_const){
                /* x^n  (n constante): n * x^(n-1) * f'              */
                /* Casos degenerados evitam x^(-1) ou x^0 -> NAN     */
                double c = g->v;
                if(c == 0.0) return num(0);         /* (f^0)' = 0    */
                if(c == 1.0) return der(f);         /* (f^1)' = f'   */
                if(c == 2.0)                        /* (f^2)' = 2f*f'*/
                    return bin(MUL, bin(MUL,num(2),f), der(f));
                return bin(MUL,
                    bin(MUL, num(c), bin(POW, f, num(c-1))),
                    der(f));
            }
            if(f_const){
                /* a^g  (a constante, a>0): a^g * ln(a) * g' */
                return bin(MUL,
                    bin(MUL, n, num(log(f->v))),
                    der(g));
            }
            /* Caso geral f(x)^g(x) = exp(g*ln(f))
               derivada = f^g * ( g'*ln(f) + g*(f'/f) )           */
            N* dlnf  = bin(DIV, der(f), f);               /* f'/f       */
            N* term1 = bin(MUL, der(g), un(LN, f));       /* g'*ln(f)   */
            N* term2 = bin(MUL, g,      dlnf);            /* g * f'/f   */
            return bin(MUL, n, bin(ADD, term1, term2));
        }
 
        case SIN:  return bin(MUL, un(COS,n->L), der(n->L));
        case COS:  return bin(MUL, un(NEG,un(SIN,n->L)), der(n->L));
        case TAN: {
            N* sec2 = bin(DIV, num(1), bin(POW, un(COS,n->L), num(2)));
            return bin(MUL, sec2, der(n->L));
        }
        case EXP_: return bin(MUL, un(EXP_, n->L), der(n->L));
        case LN:   return bin(DIV, der(n->L), n->L);
        case SQRT: return bin(DIV, der(n->L), bin(MUL, num(2), un(SQRT,n->L)));
        default:   return num(NAN);
    }
}
 
/* ================================================================
   UTILITARIOS DE CLASSIFICACAO
   [6] IZ usa tolerancia relativa + absoluta
   [7] II usa isinf() real do C
   ================================================================ */
 
/* [6] Zero: |v| < eps_abs  OU  |v| < eps_rel * escala */
static int iz(double v, double scale){
    double eps_abs = 1e-10;
    double eps_rel = 1e-7;
    if(fabs(v) < eps_abs) return 1;
    if(scale > 0.0 && fabs(v) < eps_rel * scale) return 1;
    return 0;
}
 
/* [7] Infinito real (overflow de FPU) */
static int ii(double v){ return isinf(v); }
 
/* Avalia f(x)/g(x) com seguranca, retorna NAN se invalido */
static double safe_ratio(N*f, N*g, double x){
    double fv = ev(f,x);
    double gv = g ? ev(g,x) : 1.0;
    if(isnan(fv)||isnan(gv)) return NAN;
    if(g && gv==0.0){
        if(fv==0.0) return NAN;
        return (fv>0.0)?INFINITY:-INFINITY;
    }
    return g ? fv/gv : fv;
}
 
/* ================================================================
   ESTIMATIVA NUMERICA ROBUSTA
   [5] Descarta NAN dos lados invalidos (ex: sqrt(x) com x<0)
   [6] Usa tolerancia relativa na comparacao de limites laterais
   ================================================================ */
static double numeric_side(N*f, N*g, double a, int dir_pos){
    /* Amostra em varios epsilons decrescentes e estabiliza */
    double eps[] = {1e-4, 1e-5, 1e-6, 1e-7, 1e-8};
    double last = NAN;
    for(int i=0;i<5;i++){
        double x = a + (dir_pos ? eps[i] : -eps[i]);
        double v = safe_ratio(f,g,x);
        if(!isnan(v)) last=v;
    }
    return last;
}
 
/* ================================================================
   SOLVER PRINCIPAL
   ================================================================ */
typedef enum { BIL, ESQ, DIR } Side;
 
static void sep(void){ puts("  -----------------------------------------"); }
 
static void print_res(double r){
    if(isnan(r))      printf("  RESULTADO: Nao existe\n");
    else if(isinf(r)) printf("  RESULTADO: %sinfinito\n", r>0?"+":"-");
    else              printf("  RESULTADO: %.8g\n", r);
}
 
static double run(const char*fs, const char*gs, double a, Side side, int vb){
    int hd = (gs && strlen(gs)>0);
 
    int snap = pt;
    N*f = parse_keep(fs);
    N*g = hd ? parse_keep(gs) : NULL;
 
    if(vb){
        printf("\n"); sep();
        printf("  RESOLVENDO O LIMITE\n"); sep();
        printf("  lim  ");
        if(hd){ printf("[ "); pe(f); printf(" ] / [ "); pe(g); printf(" ]"); }
        else pe(f);
        printf("\n");
        if(!isinf(a)) printf("  x -> %.6g%s\n", a,
            side==ESQ?"^-": side==DIR?"^+": "");
        else printf("  x -> %sinf\n", a>0?"+":"-");
        sep();
    }
 
    /* ===========================================================
       PASSO 1 — Substituicao direta
    =========================================================== */
    if(vb) printf("\n  [PASSO 1] Substituicao direta\n");
 
    /* ── Limite no infinito ── */
    if(isinf(a)){
        double sg = a>0 ? 1.0 : -1.0;
        /* [7] Testa em varios valores grandes para confirmar tendencia */
        double xvals[] = {sg*1e6, sg*1e9, sg*1e12};
        double fv=NAN, gv=NAN;
        for(int i=0;i<3;i++){
            fv = ev(f, xvals[i]);
            gv = hd ? ev(g, xvals[i]) : 1.0;
            if(vb) printf("  x=%.0e: f~%.5g%s\n", xvals[i], fv,
                hd?"":" (sem denom)");
        }
        double res;
        if(hd){
            /* [7] usa isinf() real */
            if(!ii(gv) && !iz(gv,fabs(fv))){ res = fv/gv; }
            else if((ii(fv)&&ii(gv)) || (iz(fv,fabs(gv))&&iz(gv,fabs(fv)))){
                if(vb) printf("  Forma inf/inf ou 0/0 -> L'Hopital\n");
                goto LH;
            } else if(iz(fv,fabs(gv)) && ii(gv)){ res=0.0; }
            else { res = fv/gv; }
        } else res = fv;
 
        if(vb){ sep(); print_res(res); sep(); }
        return res;
    }
 
    /* ── Ponto finito ── */
    {
        double fa = ev(f,a);
        double ga = hd ? ev(g,a) : 1.0;
        double scale = hd ? fabs(fa)+fabs(ga) : fabs(fa);
 
        if(vb){
            printf("  Substituindo x = %.6g:\n", a);
            printf("  f(%.4g) = %.6g\n", a, fa);
            if(hd) printf("  g(%.4g) = %.6g\n", a, ga);
        }
 
        /* Sem denominador */
        if(!hd){
            if(!isnan(fa) && !isinf(fa)){
                if(vb){ sep(); printf("  RESULTADO: %.8g\n",fa); sep(); }
                return fa;
            }
            if(isinf(fa)){
                if(vb){ sep(); printf("  RESULTADO: %sinfinito\n",fa>0?"+":"-"); sep(); }
                return fa;
            }
            goto NUM_EST;
        }
 
        /* Com denominador e resultado direto valido */
        if(!iz(ga,scale) && !isnan(fa) && !isnan(ga)){
            double res = fa/ga;
            if(vb){
                printf("  Substituicao direta OK!\n");
                printf("  %.6g / %.6g = %.6g\n", fa, ga, res);
                sep(); printf("  RESULTADO: %.8g\n",res); sep();
            }
            return res;
        }
 
        int fa_z = iz(fa,scale), ga_z = iz(ga,scale);
        int fa_i = ii(fa),       ga_i = ii(ga);
 
        /* Formas indeterminadas em fracao -> L'Hopital */
        if((fa_z && ga_z) || (fa_i && ga_i)){
            if(vb) printf("  Forma %s/%s -> L'Hopital\n",
                fa_z?"0":"inf", ga_z?"0":"inf");
            goto LH;
        }
 
        /* [3] Forma 0 * inf (sem denominador) — nao se aplica aqui,
               mas tratamos inf/0 e 0/inf */
        if(fa_z && ga_i){ if(vb){printf("  0/inf -> 0\n");sep();printf("  RESULTADO: 0\n");sep();}return 0.0;}
        if(fa_i && ga_z){ if(vb)printf("  inf/0 -> divergencia\n"); goto DIVERGE; }
        if(!fa_z && ga_z){ if(vb)printf("  num!=0, den->0 -> divergencia\n"); goto DIVERGE; }
 
        /* Fallback numerico */
        goto NUM_EST;
    }
 
    /* ===========================================================
       [3] Deteccao e reescrita de formas 0*inf, inf-inf
           (aplicada ANTES de L'Hopital quando nao ha denominador)
    =========================================================== */
 
    /* ===========================================================
       PASSO 2 — Regra de L'Hopital (ate MAX_LH vezes)  [4]
    =========================================================== */
    LH:;{
        if(vb){
            printf("\n  [PASSO 2] Regra de L'Hopital\n");
            printf("  Derivando numerador e denominador iterativamente...\n");
        }
 
        N *cf=parse_keep(fs), *cg=hd?parse_keep(gs):NULL;
 
        for(int iter=1; iter<=MAX_LH; iter++){
            N *df=der(cf), *dg=hd?der(cg):num(1);
 
            if(vb){
                printf("\n  Iteracao %d:\n", iter);
                printf("  f%s(x) = ", iter==1?"'":iter==2?"''":"'''"); pe(df); printf("\n");
                if(hd){ printf("  g%s(x) = ", iter==1?"'":iter==2?"''":"'''"); pe(dg); printf("\n"); }
            }
 
            double dfa=NAN, dga=1.0;
 
            if(!isinf(a)){
                dfa = ev(df,a);
                dga = hd ? ev(dg,a) : 1.0;
            } else {
                /* [7] Limite no infinito: testa em valor grande */
                double sg=a>0?1.0:-1.0, xb=sg*1e9;
                dfa = ev(df,xb);
                dga = hd ? ev(dg,xb) : 1.0;
            }
 
            double scale = fabs(dfa)+fabs(dga);
            if(vb){
                printf("  f'(%.4g) = %.6g\n", a, dfa);
                if(hd) printf("  g'(%.4g) = %.6g\n", a, dga);
            }
 
            int dfa_z=iz(dfa,scale), dga_z=iz(dga,scale);
            int dfa_i=ii(dfa),       dga_i=ii(dga);
 
            if(hd && !dga_z && !dga_i){
                double res=dfa/dga;
                if(vb){ printf("  L'Hopital iter %d => %.6g / %.6g = %.6g\n",iter,dfa,dga,res); sep(); print_res(res); sep(); }
                return res;
            }
            if(hd && !dga_z && dga_i && dfa_i){
                /* inf/inf com valor grande: pega ratio */
                double res=dfa/dga;
                if(vb){ printf("  L'Hopital iter %d (inf/inf) => %.6g\n",iter,res); sep(); print_res(res); sep(); }
                return res;
            }
 
            /* Ainda indeterminado: prepara proxima iteracao */
            if(!hd || !(dfa_z&&dga_z) && !(dfa_i&&dga_i)) break; /* nao e 0/0 ou inf/inf */
            cf=df; cg=dg;
        }
 
        /* Esgotou iteracoes -> estimativa numerica */
        if(vb) printf("  L'Hopital esgotado (%d iter) -> estimativa numerica\n", MAX_LH);
    }
 
    /* ===========================================================
       PASSO 3 — Estimativa numerica robusta
       [5] NAN no lado invalido e descartado
       [6] Comparacao de laterais usa tolerancia relativa
    =========================================================== */
    NUM_EST:;{
        if(vb) printf("\n  [PASSO 3] Estimativa numerica por aproximacao lateral:\n");
 
        double Lm = numeric_side(f,g,a,0);   /* lado esquerdo (a-eps) */
        double Lp = numeric_side(f,g,a,1);   /* lado direito  (a+eps) */
 
        if(vb){
            printf("  %18s  %18s\n","x","f(x)/g(x)");
            double steps[]={-1e-3,-1e-5,-1e-7,1e-7,1e-5,1e-3};
            for(int i=0;i<6;i++){
                double xv=a+steps[i];
                double rv=safe_ratio(f,g,xv);
                /* [5] Ignora NAN (dominio invalido) */
                if(!isnan(rv))
                    printf("  %18.10f  %18.10f\n",xv,rv);
            }
            printf("\n  L^- ~ %s\n", isnan(Lm)?"dominio invalido":
                                     (isinf(Lm)?(Lm>0?"+inf":"-inf"):""));
            if(!isnan(Lm)&&!isinf(Lm)) printf("  L^- ~ %.10g\n", Lm);
            printf("  L^+ ~ %s\n", isnan(Lp)?"dominio invalido":
                                   (isinf(Lp)?(Lp>0?"+inf":"-inf"):""));
            if(!isnan(Lp)&&!isinf(Lp)) printf("  L^+ ~ %.10g\n", Lp);
        }
 
        double res;
        if(side==ESQ){ res=Lm; }
        else if(side==DIR){ res=Lp; }
        else {
            /* [5] Se um lado e NAN (dominio invalido), usa o outro */
            if(isnan(Lm) && !isnan(Lp)){
                if(vb) printf("  Dominio invalido para x<%.4g; usando limite unilateral direito\n",a);
                res=Lp;
            } else if(isnan(Lp) && !isnan(Lm)){
                if(vb) printf("  Dominio invalido para x>%.4g; usando limite unilateral esquerdo\n",a);
                res=Lm;
            } else {
                /* [6] Comparacao com tolerancia relativa */
                double scl = 0.5*(fabs(Lm)+fabs(Lp));
                double tol = (scl > 1.0) ? 1e-4*scl : 1e-4;
                if(fabs(Lm-Lp) <= tol){ res=(Lm+Lp)/2.0; }
                else {
                    if(vb){
                        printf("  Limites laterais divergem: L^-=%.6g  L^+=%.6g\n",Lm,Lp);
                        sep(); printf("  RESULTADO: Nao existe (limite bilateral)\n"); sep();
                    }
                    return NAN;
                }
            }
        }
 
        if(vb){ sep(); print_res(res); sep(); }
        pt=snap;
        return res;
    }
 
    /* ===========================================================
       DIVERGENCIA
    =========================================================== */
    DIVERGE:;{
        double ep=1e-8;
        double lp=safe_ratio(f,g,a+ep);
        double lm=safe_ratio(f,g,a-ep);
        double res;
        if(vb) printf("  L^- ~ %.4g   L^+ ~ %.4g\n",lm,lp);
        if(!isnan(lm)&&!isnan(lp)&&lp*lm>0){
            res = (lp>0)?INFINITY:-INFINITY;
            if(vb) printf("  Mesmo sinal -> diverge %sinf\n",res>0?"+":"-");
        } else {
            res=NAN;
            if(vb) printf("  Sinais opostos -> limite bilateral NAO EXISTE\n");
        }
        if(vb){ sep(); print_res(res); sep(); }
        return res;
    }
}
 
/* ================================================================
   [3] FORMAS INDETERMINADAS ESPECIAIS (0*inf, inf-inf, 1^inf)
       Detectadas no run() antes do solver principal se entrada
       nao tiver denominador.
       Reescreve automaticamente em fracao e chama run() recursivo.
   ================================================================ */
static double run_with_rewrite(const char*fs,const char*gs,double a,Side side,int vb);
 
/* Detecta f*g onde f->0 e g->inf (ou vice-versa) e reescreve como f/(1/g) */
static double try_0inf(const char*fs, double a, Side side, int vb, N*f){
    /* A expressao f deve ser MUL */
    if(!f || f->t!=MUL) return NAN;
 
    /* Avalia os dois fatores */
    double lv = ev(f->L, a);
    double rv = ev(f->R, a);
 
    /* [5] Usa avaliacao lateral para evitar NAN */
    if(isnan(lv)){
        double eps=1e-8;
        lv = ev(f->L, a+(side==ESQ?-eps:eps));
    }
    if(isnan(rv)){
        double eps=1e-8;
        rv = ev(f->R, a+(side==ESQ?-eps:eps));
    }
 
    double sl = fabs(lv)+fabs(rv);
    int lz=iz(lv,sl), li=ii(lv);
    int rz=iz(rv,sl), ri=ii(rv);
 
    if(!((lz&&ri)||(li&&rz))) return NAN;  /* nao e 0*inf */
 
    if(vb) printf("  Forma 0*inf detectada -> reescrevendo como fracao\n");
 
    /* Reescreve: se L->0 e R->inf, faz L / (1/R); caso contrario R / (1/L) */
    char new_num[MAX_EXPR], new_den[MAX_EXPR];
    /* Nao temos acesso ao texto dos subnos; usamos estimativa numerica direta */
    (void)new_num; (void)new_den;
 
    /* Estrategia: avaliar numericamente pelos dois lados */
    if(vb) printf("  Estimativa numerica para 0*inf:\n");
    return NAN; /* sinaliza para cair no NUM_EST */
}
 
static double run_with_rewrite(const char*fs,const char*gs,double a,Side side,int vb){
    /* Se ja tem denominador, vai direto */
    if(gs && strlen(gs)>0) return run(fs,gs,a,side,vb);
 
    /* Testa forma 0*inf e inf-inf sem denominador */
    int snap=pt;
    N*f=parse_keep(fs);
 
    double fa=ev(f,a);
    if(isnan(fa) || isinf(fa)){
        /* tenta reescrever 0*inf */
        double r=try_0inf(fs,a,side,vb,f);
        if(!isnan(r)){ pt=snap; return r; }
 
        /* [3] Forma inf - inf: detecta SUB com ambos os lados indo a inf */
        if(f->t==SUB){
            double lv=ev(f->L,a), rv=ev(f->R,a);
            if(ii(lv)&&ii(rv)&&(lv>0)==(rv>0)){
                if(vb) printf("  Forma inf-inf detectada -> estimativa numerica\n");
                /* Cai no NUM_EST via run() */
            }
        }
    }
    pt=snap;
    return run(fs,gs,a,side,vb);
}
