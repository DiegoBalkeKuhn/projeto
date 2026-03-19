# Explicação Detalhada do Algoritmo de Visualização da Árvore Binária em C

Este documento detalha o algoritmo utilizado para visualizar a árvore binária no terminal, com foco na sua implementação em C. O algoritmo é uma adaptação de uma técnica comum para imprimir árvores de forma legível, utilizando caracteres `/` e `\` para representar as conexões entre os nodos.

## 1. Estrutura `DisplayInfo` e Lógica de Posicionamento

O coração do algoritmo de visualização reside na estrutura `DisplayInfo`, que encapsula todas as informações necessárias para renderizar uma sub-árvore e posicioná-la corretamente em relação aos seus pais e irmãos. Esta estrutura é definida como:

```c
typedef struct {
    char **lines;   // Array de strings para as linhas da sub-árvore
    int width;      // Largura total ocupada pela sub-árvore
    int height;     // Altura total da sub-árvore
    int middle;     // Posição horizontal do nodo raiz na linha do meio
} DisplayInfo;
```

-   **`lines`**: É um ponteiro para um array de strings (`char **`). Cada string representa uma linha da representação visual da sub-árvore. Por exemplo, `lines[0]` seria a linha superior, `lines[1]` a segunda, e assim por diante.
-   **`width`**: Representa a largura total, em caracteres, que a sub-árvore ocupa. Isso é crucial para calcular o espaçamento entre os nodos e garantir que as sub-árvores não se sobreponham.
-   **`height`**: Indica o número de linhas necessárias para desenhar completamente a sub-árvore, incluindo o nodo raiz, as linhas de conexão (`/` e `\`) e os filhos.
-   **`middle`**: É a posição horizontal (índice do caractere) do nodo raiz dentro da sua própria representação visual. Este valor é fundamental para alinhar os caracteres `/` e `\` que conectam o nodo pai aos seus filhos.

A lógica de posicionamento funciona de baixo para cima (bottom-up). Ou seja, primeiro as sub-árvores mais profundas são desenhadas, e suas `DisplayInfo` são usadas para calcular o posicionamento e o desenho das sub-árvores de nível superior. Quando uma sub-árvore é combinada com outra (por exemplo, um nodo pai com seus dois filhos), as `width` e `height` das sub-árvores filhas são usadas para determinar a `width` e `height` da sub-árvore pai, e os valores de `middle` são usados para posicionar os caracteres de conexão.

Por exemplo, ao combinar duas sub-árvores (esquerda e direita) para formar a representação de um nodo pai, a largura total será a soma das larguras dos filhos mais a largura do próprio nodo pai. A altura será a maior altura entre os filhos, mais duas linhas para o nodo pai e as conexões. O `middle` do nodo pai será calculado com base na largura da sub-árvore esquerda e na largura do próprio nodo pai, garantindo que o nodo pai esteja centralizado acima de seus filhos.

## 2. Funcionamento da Recursão e Casos Base

O algoritmo de visualização é implementado através de uma função recursiva, `_display_aux`, que processa a árvore de forma pós-ordem (visita os filhos antes do pai). Isso permite que as informações de layout dos filhos sejam calculadas antes que o nodo pai precise delas para se posicionar.

A função `_display_aux` possui os seguintes casos base e lógicas para cada cenário:

### Caso Base 1: Nodo Nulo

Se o nodo atual (`node`) é `NULL`, significa que não há sub-árvore para desenhar. Neste caso, uma estrutura `DisplayInfo` vazia é retornada, indicando que não há linhas, largura, altura ou posição do meio.

```c
    if (node == NULL) {
        return info; // info é inicializada com valores zero/NULL
    }
```

### Caso Base 2: Nodo Folha (Sem Filhos)

Se o nodo não possui filhos (ou seja, `node->left == NULL` e `node->right == NULL`), ele é uma folha. A representação visual é simplesmente o valor do nodo. A `DisplayInfo` é preenchida com:

-   `lines`: Um array contendo uma única string com o valor do nodo.
-   `width`: O comprimento da string do valor do nodo.
-   `height`: 1 (apenas uma linha).
-   `middle`: A posição central do valor do nodo na string.

```c
    if (node->right == NULL && node->left == NULL) {
        // ... (código para formatar o valor do nodo e preencher DisplayInfo)
        return info;
    }
```

### Caso 3: Apenas Filho Esquerdo

Se o nodo possui apenas um filho esquerdo (`node->right == NULL`), a função recursivamente chama `_display_aux` para o filho esquerdo para obter sua `DisplayInfo`. Com base nessas informações, a `DisplayInfo` do nodo atual é calculada. As linhas de conexão (`/`) são desenhadas, e as linhas do filho esquerdo são deslocadas para a direita para acomodar o nodo pai e a conexão.

```c
    if (node->right == NULL) {
        DisplayInfo left = _display_aux(node->left);
        // ... (código para calcular width, height, middle e desenhar linhas)
        return info;
    }
```

### Caso 4: Apenas Filho Direito

Similar ao caso anterior, se o nodo possui apenas um filho direito (`node->left == NULL`), a função chama `_display_aux` para o filho direito. A `DisplayInfo` do nodo atual é calculada, as linhas de conexão (`\`) são desenhadas, e as linhas do filho direito são deslocadas para a direita para acomodar o nodo pai e a conexão.

```c
    if (node->left == NULL) {
        DisplayInfo right = _display_aux(node->right);
        // ... (código para calcular width, height, middle e desenhar linhas)
        return info;
    }
```

### Caso 5: Dois Filhos

Este é o caso mais complexo. A função chama `_display_aux` recursivamente para ambos os filhos (esquerdo e direito). As `DisplayInfo` retornadas pelos filhos são então combinadas. A largura total será a soma das larguras dos filhos mais a largura do próprio nodo. A altura será a maior altura entre os filhos, mais duas linhas para o nodo pai e as conexões (`/` e `\`). As linhas dos filhos são alinhadas e mescladas para formar a representação completa da sub-árvore.

```c
    DisplayInfo left = _display_aux(node->left);
    DisplayInfo right = _display_aux(node->right);
    // ... (código para calcular width, height, middle e desenhar linhas, mesclando os filhos)
    return info;
```

Em cada chamada recursiva, a função constrói as linhas de texto que representam a sub-árvore e retorna suas dimensões e a posição do seu nodo raiz. Essa abordagem permite que a árvore seja construída de forma modular, garantindo o alinhamento correto e a representação visual esperada.

## 3. Gerenciamento de Strings e Memória em C

Um aspecto crucial da implementação em C é o gerenciamento manual de memória, especialmente ao lidar com strings e arrays de strings. A função `_display_aux` aloca dinamicamente memória para as linhas da árvore e é responsável por liberar a memória alocada para as sub-árvores filhas após suas informações serem copiadas para a estrutura da sub-árvore pai.

### Alocação de Memória (`malloc`, `strdup`)

-   **`malloc`**: É amplamente utilizado para alocar blocos de memória para a estrutura `DisplayInfo` em si, para o array de ponteiros `char **lines`, e para cada string individual (`char *`) que compõe as linhas da árvore. O tamanho da alocação é cuidadosamente calculado com base na `width` e `height` da sub-árvore.
-   **`strdup`**: Quando um nodo folha é processado, `strdup(s)` é usado para duplicar a string que contém o valor do nodo. Isso garante que uma nova área de memória seja alocada para essa string, e que ela não seja modificada acidentalmente por outras operações.

### Manipulação de Strings (`memset`, `memcpy`, `sprintf`)

-   **`sprintf`**: Converte o valor inteiro do nodo para uma string, que é então usada para construir a representação visual.
-   **`memset`**: É usado para inicializar as strings recém-alocadas com espaços (`' '`) antes de preencher com os caracteres do nodo, underscores (`_`), barras (`/`), e contrabarras (`\\`). Isso garante que as linhas tenham o tamanho correto e estejam preenchidas com caracteres padrão.
-   **`memcpy`**: Utilizado para copiar partes de strings (como os valores dos nodos ou as linhas das sub-árvores filhas) para as novas strings que estão sendo construídas para a sub-árvore pai. Isso é essencial para combinar as representações visuais dos filhos e do pai.

### Liberação de Memória (`free`)

O gerenciamento de memória é feito de forma hierárquica e recursiva:

-   Dentro de `_display_aux`, após as informações de `left.lines` e `right.lines` serem copiadas para `info.lines`, a memória alocada para as linhas individuais dos filhos (`left.lines[i]` e `right.lines[i]`) é liberada usando `free()`. Em seguida, o array de ponteiros (`left.lines` e `right.lines`) também é liberado.
-   A função `printTree` é responsável por chamar `_display_aux` e, após imprimir todas as linhas da árvore, ela itera sobre `info.lines`, liberando cada string individualmente e, finalmente, o array `info.lines`.
-   A função `freeTree` é separada e usada para liberar a memória dos próprios nodos da árvore (`Node *`), garantindo que não haja vazamentos de memória relacionados à estrutura da árvore em si.

Essa abordagem cuidadosa de alocação e liberação de memória é fundamental em C para evitar vazamentos de memória e garantir que o programa funcione de forma eficiente e estável, especialmente em operações recursivas que criam e manipulam muitas strings temporárias.
