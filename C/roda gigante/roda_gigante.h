#include <stdio.h>
#include <stdlib.h>

typedef struct carrinho
{
    int id, voltas;
    int capacidadeMaxima;
    int qtdPessoas;
    int vazio; /* 1 para vazio, 0 para ocupado */
    struct carrinho *proximo;
    /* apontador para o proximo carrinho (lista circular) */
} carrinho_t;

typedef struct
{
    carrinho_t *inicio; /* primeiro carrinho da lista circular */
    carrinho_t *atual;  /* carrinho onde a roda esta parada */
    int qtdCarrinhos;
    int pessoasFila;    /* quantidade de pessoas aguardando */
    int totalAtendidas; /* acumulador de pessoas que ja usaram a roda */
} rodaGigante_t;

void desembarcar(rodaGigante_t *roda);
void embarcar_carrinho(rodaGigante_t *roda);
void embarcar_carrinho_auto(rodaGigante_t *roda);

rodaGigante_t *criarRodaGigante()
{
    rodaGigante_t *roda = (rodaGigante_t *)malloc(sizeof(rodaGigante_t));
    roda->inicio = NULL;
    roda->atual = NULL;
    roda->qtdCarrinhos = 0;
    roda->pessoasFila = 0;
    roda->totalAtendidas = 0;
    return roda;
}

carrinho_t *criarCarrinho(int id, int capacidadeMaxima)
{
    carrinho_t *carrinho = (carrinho_t *)malloc(sizeof(carrinho_t));
    carrinho->id = id;
    carrinho->voltas = 0;
    carrinho->capacidadeMaxima = capacidadeMaxima;
    carrinho->qtdPessoas = 0;
    carrinho->vazio = 1;
    carrinho->proximo = NULL;
    return carrinho;
}

void adicionarCarrinho(rodaGigante_t *roda, int capacidadeMaxima)
{
    carrinho_t *novoCarrinho = criarCarrinho(roda->qtdCarrinhos + 1, capacidadeMaxima);
    if (roda->inicio == NULL)
    {
        roda->inicio = novoCarrinho;
        novoCarrinho->proximo = novoCarrinho;
    }
    else
    {
        carrinho_t *ultimo = roda->inicio;
        while (ultimo->proximo != roda->inicio)
        {
            ultimo = ultimo->proximo;
        }
        ultimo->proximo = novoCarrinho;
        novoCarrinho->proximo = roda->inicio;
    }
    roda->qtdCarrinhos++;
}

void liberarMemoria(rodaGigante_t *roda)
{
    if (!roda || !roda->inicio)
        return;
    carrinho_t *atual = roda->inicio;
    carrinho_t *proximo;
    for (int i = 0; i < roda->qtdCarrinhos; i++)
    {
        proximo = atual->proximo;
        free(atual);
        atual = proximo;
    }
    free(roda);
    printf("Memória liberada. Simulação encerrada.\n");
}

void avancar(rodaGigante_t *roda, int maxVoltas, int autoDesembarqueVoltas)
{
    if (roda && roda->atual)
    {
        if (autoDesembarqueVoltas == 1 && roda->atual->qtdPessoas <= 0)
        {
            printf("embarcando automaticamente no carrinho %d antes de avançar.\n", roda->atual->id);
            embarcar_carrinho_auto(roda);
        }
        
        if (roda->atual->voltas == maxVoltas && autoDesembarqueVoltas > 0)
        {
            printf("Desembarcando automaticamente do carrinho %d após %d voltas.\n", roda->atual->id, roda->atual->voltas);
            desembarcar(roda);
            printf("Embarcando automaticamente no carrinho %d após desembarque.\n", roda->atual->id);
            embarcar_carrinho_auto(roda);
        }
        if (!roda->atual->vazio)
        {
            roda->atual->voltas++;
        }
        roda->atual = roda->atual->proximo;
        printf("A roda avançou. O carrinho atual é o de ID %d.\n", roda->atual->id);
    }
}

void embarcar_carrinho(rodaGigante_t *roda)
{
    if (roda && roda->atual)
    {
        carrinho_t *carrinho_atual = roda->atual;
        int espaco_disponivel = carrinho_atual->capacidadeMaxima - carrinho_atual->qtdPessoas;

        if (espaco_disponivel > 0)
        {
            int pessoas_a_embarcar = 0;
            printf("Espaço disponível no carrinho %d: %d\n", carrinho_atual->id, espaco_disponivel);
            printf("Pessoas na fila: %d\n", roda->pessoasFila);
            printf("Quantas pessoas deseja embarcar? ");
            scanf("%d", &pessoas_a_embarcar);

            if (pessoas_a_embarcar > roda->pessoasFila)
            {
                printf("Não há pessoas suficientes na fila.\n");
                pessoas_a_embarcar = roda->pessoasFila;
            }

            if (pessoas_a_embarcar > espaco_disponivel)
            {
                printf("O carrinho não tem capacidade para tantas pessoas.\n");
                pessoas_a_embarcar = espaco_disponivel;
            }

            carrinho_atual->qtdPessoas += pessoas_a_embarcar;
            roda->pessoasFila -= pessoas_a_embarcar;
            carrinho_atual->vazio = 0;

            printf("%d pessoas embarcaram no carrinho %d.\n", pessoas_a_embarcar, carrinho_atual->id);
        }
        else
        {
            printf("O carrinho %d já está lotado.\n", carrinho_atual->id);
        }
    }
    else
    {
        printf("A roda gigante não possui carrinhos ou não está inicializada.\n");
    }
}

void embarcar_carrinho_auto(rodaGigante_t *roda)
{
    if (roda && roda->atual)
    {
        carrinho_t *carrinho_atual = roda->atual;
        int espaco_disponivel = carrinho_atual->capacidadeMaxima - carrinho_atual->qtdPessoas;

        if (espaco_disponivel > 0)
        {
            int pessoas_a_embarcar = espaco_disponivel; // Tenta preencher o carrinho

            if (pessoas_a_embarcar > roda->pessoasFila)
            {
                pessoas_a_embarcar = roda->pessoasFila; // Embarca todos da fila se couberem
            }

            if (pessoas_a_embarcar > 0)
            {
                carrinho_atual->qtdPessoas += pessoas_a_embarcar;
                roda->pessoasFila -= pessoas_a_embarcar;
                carrinho_atual->vazio = 0;

                printf("%d pessoas embarcaram automaticamente no carrinho %d.\n", pessoas_a_embarcar, carrinho_atual->id);
            }
        }
    }
}

void desembarcar(rodaGigante_t *roda)
{
    if (roda && roda->atual)
    {
        carrinho_t *carrinho_atual = roda->atual;
        if (carrinho_atual->qtdPessoas > 0)
        {
            printf("%d pessoas desembarcaram do carrinho %d.\n", carrinho_atual->qtdPessoas, carrinho_atual->id);
            roda->totalAtendidas += carrinho_atual->qtdPessoas;
            carrinho_atual->qtdPessoas = 0;
            carrinho_atual->vazio = 1;
            carrinho_atual->voltas = 0;
        }
        else
        {
            printf("O carrinho %d já está vazio.\n", carrinho_atual->id);
        }
    }
    else
    {
        printf("A roda gigante não possui carrinhos ou não está inicializada.\n");
    }
}

void giroCompleto(rodaGigante_t *roda, int maxVoltas, int autoDesembarqueVoltas)
{
    if (!roda || !roda->atual)
    {
        printf("A roda gigante não está inicializada.\n");
        return;
    }

    carrinho_t *inicioGiro = roda->atual; // ponto em que a roda está PARADA
    carrinho_t *c = inicioGiro;

    // Gira por todos carrinhos (um ciclo completo)
    do
    {
        if (autoDesembarqueVoltas == 1 && roda->atual->qtdPessoas <= 0)
        {
            printf("embarcando automaticamente no carrinho %d antes de avançar.\n", roda->atual->id);
            embarcar_carrinho_auto(roda);
        }
        
        if (!c->vazio)
        {
            c->voltas++;
        }

        if (autoDesembarqueVoltas > 0 && c->voltas >= maxVoltas && !c->vazio)
        {
            roda->atual = c;
            desembarcar(roda);

            if (roda->pessoasFila > 0)
            {
                embarcar_carrinho_auto(roda);
            }
        }

        c = c->proximo;

    } while (c != inicioGiro);

    // Depois do giro completo, a roda avança 1 carrinho
    roda->atual = roda->atual->proximo;

    printf("A roda gigante deu um giro completo. Carrinho atual: %d\n", roda->atual->id);
}

void escolherCarrinho(rodaGigante_t *roda)
{
    if (roda && roda->inicio)
    {
        int id_carrinho;
        printf("Digite o ID do carrinho para o qual deseja ir: ");
        scanf("%d", &id_carrinho);

        if (id_carrinho > 0 && id_carrinho <= roda->qtdCarrinhos)
        {
            carrinho_t *carrinho_desejado = roda->inicio;
            while (carrinho_desejado->id != id_carrinho)
            {
                carrinho_desejado = carrinho_desejado->proximo;
            }
            roda->atual = carrinho_desejado;
            printf("O carrinho atual agora é o de ID %d.\n", roda->atual->id);
        }
        else
        {
            printf("ID de carrinho inválido.\n");
        }
    }
    else
    {
        printf("A roda gigante não possui carrinhos ou não está inicializada.\n");
    }
}

void relatorio(rodaGigante_t *roda)
{
    if (roda && roda->inicio)
    {
        printf("\n--- Relatório da Roda Gigante ---\n");
        printf("Total de pessoas na fila: %d\n", roda->pessoasFila);
        printf("Total de pessoas atendidas: %d\n", roda->totalAtendidas);
        printf("----------------------------------\n");
        printf("Estado dos Carrinhos:\n");

        carrinho_t *carrinho_atual = roda->inicio;
        do
        {
            printf("Carrinho %d: %d/%d pessoas, %d voltas\n",
                   carrinho_atual->id,
                   carrinho_atual->qtdPessoas,
                   carrinho_atual->capacidadeMaxima,
                   carrinho_atual->voltas);
            carrinho_atual = carrinho_atual->proximo;
        } while (carrinho_atual != roda->inicio);
    }
    else
    {
        printf("A roda gigante não possui carrinhos ou não está inicializada.\n");
    }
}

int rodaVazia(rodaGigante_t *roda)
{
    if (roda && roda->inicio)
    {
        carrinho_t *carrinho_atual = roda->inicio;
        do
        {
            if (!carrinho_atual->vazio)
            {
                return 0;
            }
            carrinho_atual = carrinho_atual->proximo;
        } while (carrinho_atual != roda->inicio);
    }
    return 1;
}

void giroautomatico(rodaGigante_t *roda, int maxVoltas, int autoDesembarqueVoltas)
{
    if (roda && roda->inicio)
    {
        while (!rodaVazia(roda))
        {
            relatorio(roda);
            giroCompleto(roda, maxVoltas, autoDesembarqueVoltas);
        }
        printf("Todos os carrinhos estão vazios. Modo automático encerrado.\n");
    }
}