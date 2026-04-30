# Análise Detalhada do Solver de Limites em C

Este documento apresenta uma análise exaustiva do código-fonte fornecido para um solver de limites em C. O objetivo é detalhar a funcionalidade de cada variável, estrutura, função e o fluxo lógico do programa, incluindo a aplicação da Regra de L'Hôpital e métodos de estimativa numérica.

## 1. Visão Geral da Arquitetura

O solver é implementado em C e utiliza uma abordagem de árvore de expressão para representar funções matemáticas. Ele é dividido em módulos lógicos:

*   **Estruturas de Dados**: Definição dos nós da árvore de expressão e um pool para gerenciamento de memória.
*   **Parser**: Constrói a árvore de expressão a partir de uma string de entrada.
*   **Avaliador (Evaluator)**: Calcula o valor numérico de uma expressão para um dado `x`.
*   **Derivador Simbólico**: Calcula a derivada de uma expressão, também retornando uma árvore de expressão.
*   **Utilitários de Classificação**: Funções auxiliares para verificar se um número é próximo de zero ou infinito.
*   **Estimativa Numérica Robusta**: Abordagem para calcular limites através de aproximações laterais quando métodos analíticos falham.
*   **Solver Principal (`run`)**: Orquestra os passos para resolver o limite, incluindo substituição direta, Regra de L'Hôpital e estimativa numérica.
*   **Funções de Reescrever (`run_with_rewrite`)**: Lida com formas indeterminadas especiais (0*inf, inf-inf) reescrevendo-as em frações.
*   **Interface do Usuário (`main` em `limites.c`)**: Interage com o usuário para obter as expressões e exibir os resultados.

## 2. Estruturas de Dados e Gerenciamento de Memória

### `NT` (Node Type)

```c
typedef enum {
    NUM, VAR, ADD, SUB, MUL, DIV, POW,
    SIN, COS, TAN, EXP_, LN, SQRT, NEG
} NT;
```

**Descrição**: `NT` é um tipo enumerado que define os diferentes tipos de nós que podem existir na árvore de expressão. Cada tipo representa um elemento fundamental de uma expressão matemática:

*   `NUM`: Um número constante.
*   `VAR`: A variável `x`.
*   `ADD`, `SUB`, `MUL`, `DIV`, `POW`: Operadores binários (adição, subtração, multiplicação, divisão, potência).
*   `SIN`, `COS`, `TAN`, `EXP_`, `LN`, `SQRT`, `NEG`: Operadores unários (seno, cosseno, tangente, exponencial, logaritmo natural, raiz quadrada, negação).

**Uso**: Utilizado no campo `t` da estrutura `N` para identificar o tipo de nó e direcionar o processamento correto em funções como `ev` (avaliação), `pe` (impressão) e `der` (derivação).

### `N` (Node)

```c
typedef struct N { NT t; double v; struct N *L, *R; } N;
```

**Descrição**: `N` é a estrutura fundamental que representa um nó na árvore de expressão. Cada nó contém:

*   `t` (NT): O tipo do nó, conforme definido pelo enum `NT`.
*   `v` (double): O valor numérico associado ao nó. Usado apenas para nós do tipo `NUM`.
*   `L` (struct N *): Ponteiro para o filho esquerdo do nó. Representa o primeiro operando para operadores binários ou o argumento para operadores unários.
*   `R` (struct N *): Ponteiro para o filho direito do nó. Usado apenas para operadores binários (ex: `ADD`, `MUL`, `POW`).

**Uso**: A espinha dorsal da representação de expressões matemáticas. Funções como o parser constroem essas árvores, e o avaliador, impressor e derivador as atravessam.

### Pool de Nós

```c
#define PSIZ    65536
#define MAX_LH  6          /* [4] maximo de aplicacoes de L'Hopital */
#define MAX_EXPR 512

static N   pool[PSIZ];
static int pt = 0;
```

**Descrição**: Para evitar alocações dinâmicas frequentes (e lentas) com `malloc`/`free`, o programa utiliza um pool estático de nós (`pool[PSIZ]`).

*   `PSIZ` (65536): Define o tamanho máximo do pool de nós, ou seja, o número máximo de nós que uma expressão pode ter.
*   `MAX_LH` (6): Define o número máximo de vezes que a Regra de L'Hôpital será aplicada iterativamente antes de recorrer à estimativa numérica.
*   `MAX_EXPR` (512): Define o tamanho máximo das strings de expressão de entrada.
*   `pool` (N[]): Um array estático de estruturas `N` que serve como o pool de memória para todos os nós da árvore de expressão.
*   `pt` (int): Um índice que aponta para a próxima posição livre no `pool`. Atua como um "ponteiro de pilha" para o pool.

**Uso**: O pool é gerenciado pelas funções `nn`, `num`, `var_`, `bin` e `un`. Quando um novo nó é necessário, `nn` o aloca do pool e incrementa `pt`. Se o pool se esgotar (`pt >= PSIZ`), o programa termina com um erro. O `pt` é resetado para 0 no início de cada nova avaliação de limite no `main` para reutilizar o pool.

### Funções de Criação de Nós

*   `static N* nn(NT t)`
    *   **Descrição**: Função auxiliar para criar um novo nó do tipo `t` a partir do pool. Inicializa `v` como 0 e `L`, `R` como `NULL`.
    *   **Retorno**: Um ponteiro para o novo nó alocado.
*   `static N* num(double v)`
    *   **Descrição**: Cria um nó do tipo `NUM` com o valor `v`.
    *   **Retorno**: Um ponteiro para o nó `NUM`.
*   `static N* var_(void)`
    *   **Descrição**: Cria um nó do tipo `VAR`.
    *   **Retorno**: Um ponteiro para o nó `VAR`.
*   `static N* bin(NT t, N* l, N* r)`
    *   **Descrição**: Cria um nó de operador binário (`t`) com filhos esquerdo `l` e direito `r`.
    *   **Retorno**: Um ponteiro para o nó binário.
*   `static N* un(NT t, N* c)`
    *   **Descrição**: Cria um nó de operador unário (`t`) com filho `c`.
    *   **Retorno**: Um ponteiro para o nó unário.

## 3. Parser (Analisador Sintático)

O parser é responsável por converter uma string de expressão matemática em uma árvore de expressão (`N*`). Ele usa uma abordagem de precedência de operadores (shunting-yard ou recursive descent) para construir a árvore. As funções do parser são estáticas e mutuamente recursivas.

### Variáveis Globais do Parser

*   `static const char *S`
    *   **Descrição**: Ponteiro para a string da expressão matemática que está sendo analisada.
    *   **Uso**: É atualizado pela função `parse_keep` antes de iniciar a análise.
*   `static int P`
    *   **Descrição**: Índice que indica a posição atual do parser na string `S`.
    *   **Uso**: É incrementado à medida que o parser consome caracteres da string.

### Funções do Parser

*   `static void ws(void)`
    *   **Descrição**: "White Space" - Avança o ponteiro `P` sobre quaisquer caracteres de espaço em branco na string `S`.
    *   **Uso**: Chamada frequentemente por outras funções do parser para ignorar espaços.
*   `static N* pE(void)` (Parse Expression)
    *   **Descrição**: Analisa expressões de adição e subtração. Tem a menor precedência.
    *   **Funcionamento**: Primeiro, analisa um termo (`pT()`). Em seguida, em um loop, verifica se há `+` ou `-`. Se houver, consome o operador, analisa o próximo termo e cria um nó `ADD` ou `SUB` com o resultado atual e o novo termo.
    *   **Retorno**: Um ponteiro para o nó raiz da sub-árvore que representa a expressão analisada.
*   `static N* pT(void)` (Parse Term)
    *   **Descrição**: Analisa termos de multiplicação e divisão, e também lida com multiplicação implícita.
    *   **Funcionamento**: Primeiro, analisa uma potência (`pPow()`). Em um loop, verifica se há `*` ou `/`. Se houver, consome o operador, analisa a próxima potência e cria um nó `MUL` ou `DIV`. Também verifica `can_start_factor()` para multiplicação implícita (ex: `2x`, `(x+1)(x-1)`).
    *   **Retorno**: Um ponteiro para o nó raiz da sub-árvore.
*   `static N* pPow(void)` (Parse Power)
    *   **Descrição**: Analisa operações de potência (`^`).
    *   **Funcionamento**: Analisa um fator unário (`pU()`) para a base. Se encontrar `^`, consome o `^`, analisa outro fator unário para o expoente e cria um nó `POW`.
    *   **Retorno**: Um ponteiro para o nó raiz da sub-árvore.
*   `static N* pU(void)` (Parse Unary)
    *   **Descrição**: Analisa operadores unários como negação (`-`) e o sinal positivo (`+`).
    *   **Funcionamento**: Se encontrar `-`, consome-o e cria um nó `NEG` com o resultado de `pU()` recursivamente. Se encontrar `+`, simplesmente o ignora e chama `pU()` recursivamente. Caso contrário, chama `pPri()`.
    *   **Retorno**: Um ponteiro para o nó raiz da sub-árvore.
*   `static N* pPri(void)` (Parse Primary)
    *   **Descrição**: Analisa os elementos primários da expressão: números, variáveis, constantes (e, pi), funções e expressões entre parênteses.
    *   **Funcionamento**: 
        *   **Parênteses**: Se encontra `(`, consome-o, chama `pE()` para analisar a expressão interna e espera `)`.
        *   **Números**: Se encontra um dígito ou `.`, lê o número completo e cria um nó `NUM` usando `atof`.
        *   **Identificadores (letras)**: Se encontra uma letra, lê o identificador. 
            *   Verifica se é `e` ou `pi` (constantes).
            *   Verifica se é `x` (variável).
            *   Se for uma função (`sin`, `cos`, `tan`, `exp`, `ln`, `sqrt`), espera `(`, analisa o argumento com `pE()`, espera `)` e cria um nó unário para a função.
        *   **Erro**: Se nenhum padrão for reconhecido, imprime um erro e sai.
    *   **Retorno**: Um ponteiro para o nó raiz da sub-árvore.
*   `static int can_start_factor(void)`
    *   **Descrição**: Verifica se o próximo token na string de entrada pode iniciar um fator, o que é crucial para detectar multiplicação implícita.
    *   **Funcionamento**: Ignora espaços em branco e verifica se o caractere atual é um dígito, um ponto, uma letra ou um parêntese de abertura.
    *   **Retorno**: `1` se pode iniciar um fator, `0` caso contrário.
*   `static N* parse_keep(const char *expr)`
    *   **Descrição**: Função de entrada principal para o parser. Define a string de expressão `S` e reinicia o ponteiro `P` para 0.
    *   **Retorno**: O nó raiz da árvore de expressão completa.

## 4. Avaliador (Evaluator)

O avaliador (`ev`) percorre a árvore de expressão e calcula seu valor numérico para um dado valor de `x`.

### `static double ev(N* n, double x)`

*   **Descrição**: Avalia recursivamente a árvore de expressão `n` para um valor específico da variável `x`.
*   **Parâmetros**:
    *   `n` (N*): O nó atual da árvore de expressão a ser avaliado.
    *   `x` (double): O valor numérico a ser substituído pela variável `x`.
*   **Funcionamento**: Utiliza uma estrutura `switch` baseada no tipo do nó (`n->t`):
    *   `NUM`: Retorna o valor `n->v`.
    *   `VAR`: Retorna o valor de `x`.
    *   `NEG`: Retorna o negativo da avaliação do filho esquerdo (`n->L`).
    *   `ADD`, `SUB`, `MUL`: Avalia os filhos esquerdo e direito e aplica a operação correspondente.
    *   `DIV`: Avalia os filhos esquerdo e direito. Inclui tratamento explícito para divisão por zero:
        *   Se o denominador for zero e o numerador também for zero, retorna `NAN` (Not a Number).
        *   Se o denominador for zero e o numerador for diferente de zero, retorna `INFINITY` ou `-INFINITY` dependendo do sinal do numerador.
        *   Caso contrário, retorna a divisão normal.
    *   `POW`: Retorna `pow(ev(n->L,x), ev(n->R,x))`.
    *   `SIN`, `COS`, `TAN`, `EXP_`, `LN`, `SQRT`: Avalia o filho esquerdo e aplica a função matemática correspondente (`sin`, `cos`, `tan`, `exp`, `log`, `sqrt`).
    *   `default`: Retorna `NAN` para tipos de nó desconhecidos.
*   **Retorno**: O valor `double` da expressão avaliada.

## 5. Impressão da Expressão

O impressor (`pe`) percorre a árvore de expressão e imprime a expressão matemática formatada.

### `static void pe(N* n)`

*   **Descrição**: Imprime recursivamente a expressão representada pela árvore `n` no console.
*   **Parâmetro**:
    *   `n` (N*): O nó atual da árvore de expressão a ser impresso.
*   **Funcionamento**: Utiliza uma estrutura `switch` baseada no tipo do nó (`n->t`):
    *   `NUM`: Imprime o valor numérico. Se for um inteiro, imprime sem casas decimais; caso contrário, com 4 casas significativas.
    *   `VAR`: Imprime `x`.
    *   `NEG`: Imprime `-(` seguido da impressão do filho esquerdo e `)`.
    *   `ADD`, `SUB`, `MUL`, `DIV`, `POW`: Imprime `(` seguido da impressão do filho esquerdo, o operador, a impressão do filho direito e `)`.
    *   `SIN`, `COS`, `TAN`, `EXP_`, `LN`, `SQRT`: Imprime o nome da função seguido de `(` a impressão do filho esquerdo e `)`.
*   **Retorno**: `void` (imprime diretamente no console).

## 6. Derivação Simbólica

O derivador (`der`) calcula a derivada de uma expressão simbolicamente, retornando uma nova árvore de expressão que representa a derivada.

### `static N* der(N* n)`

*   **Descrição**: Calcula a derivada simbólica da expressão representada pelo nó `n` em relação a `x`.
*   **Parâmetro**:
    *   `n` (N*): O nó raiz da expressão a ser derivada.
*   **Funcionamento**: Implementa as regras de derivação para cada tipo de nó:
    *   `NUM`: Derivada de uma constante é 0 (`num(0)`).
    *   `VAR`: Derivada de `x` é 1 (`num(1)`).
    *   `NEG`: `d(-f)/dx = -d(f)/dx`.
    *   `ADD`, `SUB`: `d(f+g)/dx = d(f)/dx + d(g)/dx` e `d(f-g)/dx = d(f)/dx - d(g)/dx`.
    *   `MUL` (Regra do Produto): `d(fg)/dx = f'g + fg'`.
    *   `DIV` (Regra do Quociente): `d(f/g)/dx = (f'g - fg') / g^2`.
    *   `POW` (Potência Geral `f(x)^g(x)`):
        *   **Caso `c1^c2` (constante^constante)**: Derivada é 0.
        *   **Caso `f(x)^n` (n constante)**: `n * f(x)^(n-1) * f'(x)`. Trata casos especiais para `n=0`, `n=1`, `n=2` para simplificação.
        *   **Caso `a^g(x)` (a constante)**: `a^g(x) * ln(a) * g'(x)`.
        *   **Caso Geral `f(x)^g(x)`**: Usa a regra `d(f^g)/dx = f^g * (g'ln(f) + g(f'/f))`.
    *   `SIN`: `d(sin(f))/dx = cos(f) * f'`.
    *   `COS`: `d(cos(f))/dx = -sin(f) * f'`.
    *   `TAN`: `d(tan(f))/dx = sec^2(f) * f'` (reescrito como `1/cos^2(f) * f'`).
    *   `EXP_`: `d(exp(f))/dx = exp(f) * f'`.
    *   `LN`: `d(ln(f))/dx = f'/f`.
    *   `SQRT`: `d(sqrt(f))/dx = f' / (2 * sqrt(f))`.
    *   `default`: Retorna `num(NAN)` para tipos desconhecidos.
*   **Retorno**: Um ponteiro para o nó raiz da nova árvore de expressão que representa a derivada.

## 7. Utilitários de Classificação

Essas funções auxiliam na comparação de valores `double` com zero ou infinito, considerando a natureza de ponto flutuante.

### `static int iz(double v, double scale)`

*   **Descrição**: Verifica se um valor `v` é considerado "próximo de zero" dentro de uma tolerância absoluta ou relativa.
*   **Parâmetros**:
    *   `v` (double): O valor a ser verificado.
    *   `scale` (double): Uma escala para calcular a tolerância relativa. Geralmente a soma dos módulos dos termos envolvidos.
*   **Variáveis Locais**:
    *   `eps_abs` (1e-10): Tolerância absoluta. Se `|v| < eps_abs`, é considerado zero.
    *   `eps_rel` (1e-7): Tolerância relativa. Se `|v| < eps_rel * scale`, é considerado zero.
*   **Funcionamento**: Retorna `1` se `v` for menor que `eps_abs` em valor absoluto, ou se `v` for menor que `eps_rel * scale` em valor absoluto (e `scale` for positivo). Caso contrário, retorna `0`.
*   **Retorno**: `int` (booleano).

### `static int ii(double v)`

*   **Descrição**: Verifica se um valor `v` é infinito (positivo ou negativo).
*   **Funcionamento**: Utiliza a função padrão `isinf()` da biblioteca `math.h`.
*   **Retorno**: `int` (booleano).

### `static double safe_ratio(N* f, N* g, double x)`

*   **Descrição**: Avalia a razão `f(x)/g(x)` de forma segura, tratando casos de `NAN` e divisão por zero.
*   **Parâmetros**:
    *   `f` (N*): Nó do numerador.
    *   `g` (N*): Nó do denominador (pode ser `NULL` se não houver denominador explícito).
    *   `x` (double): Valor para `x`.
*   **Funcionamento**: 
    *   Avalia `f(x)` e `g(x)`.
    *   Se qualquer um for `NAN`, retorna `NAN`.
    *   Se `g(x)` for zero:
        *   Se `f(x)` também for zero, retorna `NAN` (forma indeterminada 0/0).
        *   Caso contrário, retorna `INFINITY` ou `-INFINITY` dependendo do sinal de `f(x)`.
    *   Caso contrário, retorna `f(x)/g(x)`.
*   **Retorno**: `double` (o valor da razão ou `NAN`/`INFINITY`).

## 8. Estimativa Numérica Robusta

Este módulo é usado quando os métodos analíticos (substituição direta, L'Hôpital) não conseguem resolver o limite ou resultam em formas indeterminadas complexas. Ele aproxima o limite avaliando a função em pontos muito próximos ao ponto de limite `a`.

### `static double numeric_side(N* f, N* g, double a, int dir_pos)`

*   **Descrição**: Estima o limite de `f(x)/g(x)` quando `x` se aproxima de `a` por um lado específico (esquerdo ou direito), usando uma série de `epsilons` decrescentes.
*   **Parâmetros**:
    *   `f` (N*): Nó do numerador.
    *   `g` (N*): Nó do denominador.
    *   `a` (double): O ponto de limite.
    *   `dir_pos` (int): `1` para o lado direito (`a+eps`), `0` para o lado esquerdo (`a-eps`).
*   **Variáveis Locais**:
    *   `eps[]`: Um array de valores `epsilon` decrescentes (`1e-4` a `1e-8`) para se aproximar de `a`.
    *   `last`: Armazena o último valor válido calculado.
*   **Funcionamento**: 
    *   Itera sobre os valores de `eps`.
    *   Para cada `eps`, calcula `x = a + (dir_pos ? eps[i] : -eps[i])`.
    *   Avalia `safe_ratio(f, g, x)`.
    *   Se o resultado não for `NAN`, atualiza `last`.
    *   Retorna o último valor válido encontrado.
*   **Retorno**: `double` (a estimativa numérica do limite lateral).

## 9. Solver Principal (`run`)

Esta é a função central que orquestra o cálculo do limite, aplicando diferentes estratégias em uma sequência definida.

### `typedef enum { BIL, ESQ, DIR } Side;`

*   **Descrição**: Enumeração para especificar o tipo de limite a ser calculado: bilateral (`BIL`), pela esquerda (`ESQ`) ou pela direita (`DIR`).

### `static void sep(void)`

*   **Descrição**: Imprime uma linha separadora para melhorar a legibilidade da saída.

### `static void print_res(double r)`

*   **Descrição**: Imprime o resultado final do limite de forma formatada, tratando `NAN`, `INFINITY` e valores numéricos.

### `static double run(const char* fs, const char* gs, double a, Side side, int vb)`

*   **Descrição**: A função principal para calcular o limite de `f(x)/g(x)` quando `x` se aproxima de `a`.
*   **Parâmetros**:
    *   `fs` (const char*): String da expressão do numerador `f(x)`.
    *   `gs` (const char*): String da expressão do denominador `g(x)`. Pode ser `NULL` ou vazio se não houver denominador.
    *   `a` (double): O ponto para o qual `x` se aproxima.
    *   `side` (Side): O tipo de limite (bilateral, esquerdo, direito).
    *   `vb` (int): Flag de verbosidade. Se `1`, imprime os passos intermediários.
*   **Variáveis Locais Chave**:
    *   `hd` (int): Booleano, `1` se há denominador (`gs` não é nulo/vazio), `0` caso contrário.
    *   `snap` (int): Salva o estado atual do `pt` (ponteiro do pool de nós) para restaurá-lo após a análise.
    *   `f`, `g` (N*): Nós raiz das árvores de expressão para o numerador e denominador, respectivamente.
    *   `fa`, `ga` (double): Valores de `f(a)` e `g(a)`.
    *   `scale` (double): Usado para a função `iz` (is zero) para tolerância relativa.
*   **Fluxo de Execução**:
    1.  **Inicialização**: Imprime informações de cabeçalho se `vb` for `1`. Analisa as strings `fs` e `gs` em árvores de expressão `f` e `g`.
    2.  **PASSO 1 — Substituição Direta**: 
        *   **Limite no Infinito (`isinf(a)`)**: 
            *   Avalia `f` e `g` em valores grandes (`1e6`, `1e9`, `1e12`) para inferir a tendência.
            *   Verifica formas indeterminadas (`inf/inf`, `0/0`) que levam a L'Hôpital (`goto LH`).
            *   Trata casos como `0/inf -> 0`.
            *   Se o resultado for direto (não indeterminado), imprime e retorna.
        *   **Ponto Finito (`a`)**: 
            *   Calcula `fa = ev(f, a)` e `ga = ev(g, a)`.
            *   **Sem Denominador (`!hd`)**: Se `f(a)` é um número finito ou infinito, retorna diretamente. Caso contrário, passa para a estimativa numérica (`goto NUM_EST`).
            *   **Com Denominador (`hd`)**: 
                *   Se `g(a)` não é zero (com tolerância) e `f(a)`, `g(a)` não são `NAN`, calcula `fa/ga` e retorna.
                *   Verifica formas indeterminadas (`0/0`, `inf/inf`) que levam a L'Hôpital (`goto LH`).
                *   Trata casos como `0/inf -> 0` e `inf/0` (divergência).
                *   Se nenhuma das condições acima for atendida, passa para a estimativa numérica (`goto NUM_EST`).
    3.  **PASSO 2 — Regra de L'Hôpital (`LH` label)**:
        *   Aplica a Regra de L'Hôpital iterativamente até `MAX_LH` vezes.
        *   Em cada iteração, deriva o numerador (`df`) e o denominador (`dg`).
        *   Avalia `df(a)` e `dg(a)` (ou em um valor grande para limites no infinito).
        *   Se `dg(a)` não for zero (com tolerância) e não for infinito, calcula `df(a)/dg(a)` e retorna.
        *   Se ainda for uma forma indeterminada (`0/0` ou `inf/inf`), continua para a próxima iteração.
        *   Se `MAX_LH` iterações forem esgotadas ou a forma não for mais indeterminada, passa para a estimativa numérica (`goto NUM_EST`).
    4.  **PASSO 3 — Estimativa Numérica Robusta (`NUM_EST` label)**:
        *   Calcula os limites laterais esquerdo (`Lm`) e direito (`Lp`) usando `numeric_side`.
        *   **Limites Unilaterais**: Se `side` for `ESQ` ou `DIR`, retorna o limite lateral correspondente.
        *   **Limites Bilaterais**: 
            *   Se um lado é `NAN` (domínio inválido), usa o outro lado.
            *   Compara `Lm` e `Lp` com uma tolerância relativa. Se forem suficientemente próximos, retorna a média. Caso contrário, os limites laterais divergem, e o limite bilateral não existe (`NAN`).
        *   Restaura o `pt` do pool de nós para o estado inicial da chamada `run`.
    5.  **DIVERGÊNCIA (`DIVERGE` label)**:
        *   Usado para casos onde a substituição direta ou L'Hôpital indicam uma divergência (ex: `num!=0 / den->0`).
        *   Avalia a função em `a+ep` e `a-ep` para verificar os sinais dos limites laterais.
        *   Se os sinais forem os mesmos, o limite diverge para `+INFINITY` ou `-INFINITY`. Se forem opostos, o limite bilateral não existe (`NAN`).
*   **Retorno**: `double` (o valor do limite, `NAN`, `INFINITY` ou `-INFINITY`).

## 10. Funções de Reescrever Formas Indeterminadas (`run_with_rewrite`)

Este módulo lida com formas indeterminadas que não são diretamente `0/0` ou `inf/inf`, como `0*inf` e `inf-inf`, reescrevendo-as para que possam ser tratadas pelo solver principal.

### `static double try_0inf(const char* fs, double a, Side side, int vb, N* f)`

*   **Descrição**: Tenta detectar e reescrever a forma indeterminada `0*inf` (ou `inf*0`).
*   **Parâmetros**:
    *   `fs` (const char*): String da expressão original.
    *   `a` (double): Ponto de limite.
    *   `side` (Side): Tipo de limite.
    *   `vb` (int): Flag de verbosidade.
    *   `f` (N*): Nó raiz da expressão (espera-se que seja um nó `MUL`).
*   **Funcionamento**:
    *   Verifica se o nó `f` é de multiplicação (`MUL`).
    *   Avalia os filhos esquerdo (`f->L`) e direito (`f->R`) em `a` (com ajuste lateral se `NAN`).
    *   Usa `iz` e `ii` para verificar se um fator é zero e o outro é infinito.
    *   Se `0*inf` for detectado, imprime uma mensagem. O código original indica que ele deveria reescrever a expressão como uma fração (ex: `L / (1/R)` ou `R / (1/L)`) e chamar `run` recursivamente. No entanto, a implementação atual retorna `NAN`, indicando que ele cairá na estimativa numérica (`NUM_EST`) no `run` principal.
*   **Retorno**: `double` (atualmente `NAN` se detectado, `NAN` se não for `0*inf`).

### `static double run_with_rewrite(const char* fs, const char* gs, double a, Side side, int vb)`

*   **Descrição**: Função wrapper que tenta detectar e reescrever formas indeterminadas especiais antes de chamar o solver principal `run`.
*   **Parâmetros**: Mesmos parâmetros de `run`.
*   **Funcionamento**:
    *   Se já houver um denominador (`gs` não vazio), chama `run` diretamente.
    *   Caso contrário (sem denominador):
        *   Salva o estado do pool (`snap`).
        *   Analisa `fs` em `f`.
        *   Avalia `f(a)`.
        *   Se `f(a)` for `NAN` ou `INFINITY`:
            *   Tenta `try_0inf`. Se retornar um valor válido (não `NAN`), restaura o pool e retorna.
            *   Verifica a forma `inf-inf` (se `f` for um nó `SUB` e ambos os filhos avaliarem para infinito com o mesmo sinal). Se detectado, imprime uma mensagem e permite que o `run` principal caia na estimativa numérica.
        *   Restaura o pool (`pt=snap`).
        *   Chama `run` com as expressões originais.
*   **Retorno**: `double` (o resultado do limite).

## 11. Interface do Usuário (`limites.c`)

O arquivo `limites.c` contém a função `main`, que é a interface de linha de comando para o solver.

### `int main(void)`

*   **Descrição**: Ponto de entrada do programa. Lida com a interação com o usuário, entrada de expressões, chamada do solver e exibição dos resultados.
*   **Variáveis Locais Chave**:
    *   `fb[MAX_EXPR]`, `gb[MAX_EXPR]`: Buffers para armazenar as strings do numerador e denominador, respectivamente.
    *   `ab[64]`: Buffer para armazenar a string do ponto de limite `a`.
    *   `sb[8]`: Buffer para armazenar a string da lateralidade do limite.
    *   `a` (double): O valor numérico do ponto de limite.
    *   `side` (Side): O tipo de limite (bilateral, esquerdo, direito).
    *   `resp[4]`: Buffer para a resposta do usuário sobre continuar ou sair.
*   **Fluxo de Execução**:
    1.  **Boas-vindas e Instruções**: Imprime uma mensagem de boas-vindas, lista de operadores, funções, constantes e exemplos de uso.
    2.  **Loop Principal (`while(1)`)**:
        *   **Entrada do Numerador**: Solicita `f(x)` e lê a entrada para `fb`.
        *   **Entrada do Denominador**: Solicita `g(x)` (opcional) e lê a entrada para `gb`. Remove espaços em branco iniciais/finais.
        *   **Entrada do Ponto de Limite (`a`)**: Solicita o ponto `a` e lê para `ab`.
            *   Trata `inf`, `-inf`, `e`, `pi`, `-e`, `-pi` como constantes.
            *   Se a entrada for uma expressão (ex: `2*pi`), usa o parser (`pE`) e o avaliador (`ev`) para calcular o valor de `a`. Salva e restaura o estado do pool (`pt`) para evitar interferências.
            *   Se for um número simples, usa `atof`.
        *   **Entrada da Lateralidade**: Solicita o tipo de limite (`b`, `e`, `d`) e lê para `sb`. Define a variável `side`.
        *   **Reset do Pool**: `pt = 0;` Reseta o ponteiro do pool de nós para que novas árvores de expressão possam ser construídas para o próximo cálculo.
        *   **Chamada do Solver**: Chama `run_with_rewrite(fb, gb_start, a, side, 1)` para calcular o limite. O `1` indica modo verboso.
        *   **Exibição do Resultado**: Imprime o resultado final formatado.
        *   **Continuar/Sair**: Pergunta ao usuário se deseja calcular outro limite. Se a resposta for `n` ou `N`, o loop termina.
    3.  **Saída**: Imprime "Ate logo!" e encerra o programa.

## 12. Considerações e Melhorias

*   **Gerenciamento de Memória**: O uso de um pool estático é eficiente para evitar `malloc`/`free` repetidos, mas impõe um limite fixo (`PSIZ`) no tamanho da expressão. Expressões muito complexas podem esgotar o pool.
*   **Tratamento de Erros**: O parser e o avaliador têm tratamento básico de erros (ex: função desconhecida, divisão por zero), mas poderiam ser mais robustos com mensagens de erro mais detalhadas e recuperação.
*   **Otimização da Árvore**: Não há otimização da árvore de expressão (ex: `x+0` simplifica para `x`, `1*x` para `x`). Isso poderia melhorar a eficiência e a legibilidade das derivadas.
*   **Reescrita de Formas Indeterminadas**: A função `try_0inf` atualmente retorna `NAN` e depende da estimativa numérica. Uma implementação completa reescreveria a expressão simbolicamente (ex: `f*g` para `f/(1/g)`) e chamaria `run` recursivamente com a nova forma.
*   **Precisão Numérica**: A estimativa numérica usa `double` e um conjunto fixo de `epsilons`. Para casos muito sensíveis, pode ser necessário um tratamento de precisão arbitrária ou algoritmos mais avançados.
*   **Derivadas de Funções Compostas**: O derivador lida bem com a regra da cadeia para funções básicas. Para funções mais complexas ou aninhadas, a árvore de derivada pode crescer rapidamente.

Esta análise fornece uma compreensão aprofundada do funcionamento interno do solver de limites, destacando suas principais componentes e a lógica por trás de cada etapa do cálculo.
