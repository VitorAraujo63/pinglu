# A especificação do Pinglu em sete seções (Tarefa 1, Módulo 2)

---

## 1. O domínio e a cena

**Pergunta:** sobre o que fala a sua linguagem, e quem se beneficiaria de
escrevê-la?

**Resposta:** o domínio do Pinglu não é um setor — não é "processamento de
texto" nem "jogos". É uma situação que se repete em programas de propósito
geral: alguém está escrevendo uma decisão que depende da **forma** que um
valor pode assumir — um pagamento que pode chegar em dinheiro, cartão ou
boleto; uma figura geométrica que pode ser círculo, retângulo ou
triângulo; uma resposta de rede que pode ser sucesso, erro ou timeout — e
quer saber, **antes** de publicar o programa, se esqueceu de tratar
alguma dessas formas.

A cena: essa pessoa já escreveu, em alguma linguagem, uma cadeia de
`if`/`else if` para tratar casos assim, e já foi surpreendida — em
produção, não em teste — por um caso que a cadeia não previa e que
simplesmente não fez nada, ou quebrou de um jeito difícil de rastrear até
a linha certa. O que ela quer obter escrevendo em Pinglu não é o programa
rodando mais rápido, nem uma sintaxe mais bonita — é uma resposta do
compilador, antes de rodar, para a pergunta "cobri todos os casos?". Isso
é o que `docs/04_escolher_e_formas.md` chama de "provar, não percorrer": o
compilador compara o conjunto de casos escritos contra o conjunto de casos
que o tipo admite, e recusa a compilação se sobrar um.

_(Sinal de erro que este parágrafo evita: descrever um programa — `if`,
`escolher`, `forma` — em vez do domínio. Esses nomes aparecem aqui só
porque a cena os motiva; a cena em si é sobre decisões-por-forma e sobre
alguém que já foi mordido por um caso esquecido, não sobre sintaxe.)_

## 2. O que se escreve na linguagem

**Pergunta:** como é, na prática, um texto escrito nela? Três a cinco
exemplos completos, do mais simples ao mais elaborado, escritos como se a
linguagem já existisse.

**Resposta**, do mais simples ao mais elaborado — cada um exercitando uma
construção diferente, para não cair no sinal de erro desta seção (exemplos
que são só variações do mesmo formato):

**1. O mais simples — declarar e imprimir.**

```pinglu
int a = 5;
print(a);
```

**2. Aritmética e decisão** (`exemplos/01_aritmetica_condicional.pglu`):

```pinglu
int a = 5;
int b = 3;
int soma = a + b;
print(soma);
if (soma == 8 && soma - 8 == 0) {
    print("par");
} else {
    print("impar");
}
```

**3. Repetição com condição** (`exemplos/02_for_e_relacionais.pglu`):

```pinglu
for (int i = 0; i < 5; i = i + 1) {
    print(i);
}
```

**4. Decompor um cálculo em função** (`exemplos/04_funcao.pglu`):

```pinglu
func soma(int a, int b) {
    return a + b;
}
int resultado = soma(5, 3);
print(resultado);
```

**5. O mais elaborado — a cena da seção 1, escrita** (`exemplos/05_escolher_completo.pglu`):

```pinglu
forma Geometria {
    circulo(float r);
    retangulo(float b, float h);
    triangulo(float base, float altura);
}
Geometria f = circulo(3.0);
escolher f {
    circulo(r)               -> print(3.14 * r * r);
    retangulo(b, h)          -> print(b * h);
    triangulo(base, altura)  -> print((base * altura) / 2.0);
}
```

## 3. O que o sistema aceita e o que recusa

**Pergunta:** dado um texto qualquer, o que faz dele válido? As formas
aceitas e, para cada tipo de erro previsível, a mensagem que a pessoa
recebe e o que ela consegue fazer com essa mensagem.

**O que é aceito, em uma frase:** uma sequência de declarações (`int`,
`func`, `forma`, ...) e instruções (`if`, `while`, `for`, `pineach`,
`escolher`, `print`, atribuição), cada uma terminada em `;` ou fechada em
`{ }`, usando só os símbolos de `especificacao/alfabeto.md` e só as
palavras-chave de `especificacao/classes_lexicas.md`. A forma exata está
em `docs/01_gramatica.txt`.

**O que é recusado, e a mensagem em cada caso** — esta é a parte que o
sinal de erro desta seção cobra e que a especificação anterior não tinha:

| Tipo de erro                         | Exemplo do que dispara                                               | Mensagem (formato `linha L: ...`, ver `docs/01_recorte.md` § "Como um erro é relatado")         | O que a pessoa faz com ela                                                            |
| ------------------------------------ | -------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------- |
| Léxico — símbolo fora do alfabeto    | um caractere fora de `alfabeto.md` no meio do código                 | `linha 4: símbolo não reconhecido`                                                              | vai até a linha 4 e remove/corrige o caractere                                        |
| Sintático — token faltando           | `int a = 5` sem `;`                                                  | `linha 1: esperado ';' após a expressão`                                                        | acrescenta o `;` na linha indicada                                                    |
| Semântico — nome não declarado       | `print(x);` sem `x` ter sido declarado antes                         | `linha 7: 'x' não foi declarado neste escopo`                                                   | declara `x` antes de usar, ou corrige o nome (erro de digitação é a causa mais comum) |
| Semântico — naturezas incompatíveis  | `int total = 5 + "cinco";`                                           | `linha 3: não é possível combinar int e string com '+'`                                         | converte um dos dois valores, ou troca o operador                                     |
| Semântico — quantidade de parâmetros | chamar `soma(5, 3, 1)` quando `soma` espera 2                        | `linha 10: 'soma' espera 2 argumento(s), recebeu 3`                                             | ajusta a chamada para a quantidade certa                                              |
| Semântico — cobertura do `escolher`  | `exemplos/06_escolher_incompleto.pglu`: falta a cláusula `triangulo` | `linha 18: o caso 'triangulo' não é tratado`                                                    | acrescenta a cláusula que falta                                                       |
| Semântico — cláusula redundante      | uma variante repetida no `escolher`                                  | `linha N: esta cláusula nunca será alcançada` (aviso, não recusa — o programa continua correto) | remove a cláusula morta, ou reconhece que era engano                                  |

Metade do uso real de uma linguagem é isso: escrever algo que não
funciona e precisar entender, pela mensagem, o quê e onde — não só saber o
que teria sido aceito.

## 4. Onde a linguagem se aninha

**Pergunta:** que construção contém outra do mesmo tipo por dentro, sem
profundidade máxima? Aponte-a e mostre um exemplo com três níveis.

**Resposta:** `block` (`"{" decl* "}"` em `docs/01_gramatica.txt`). Um
`block` contém `decl*`, e `decl` pode ser um `stmt`, que pode ser outro
`block` — sem limite de profundidade declarado em nenhum lugar da
gramática. É esse aninhamento que exige pilha para reconhecer (nenhum
autômato finito resolve sozinho, ver a nota de Chomsky em
`docs/01_gramatica.txt`), e é a mesma construção que aparece em
`escolherStmt`: o `stmt` à direita de cada `->` pode ser, ele mesmo, outro
`block` contendo outro `escolher`.

**Exemplo com três níveis:**

```pinglu
if (a == 1) {
    if (b == 2) {
        if (c == 3) {
            print("tres niveis");
        }
    }
}
```

Sem essa construção, a lacuna não apareceria olhando exemplos simples —
apareceria tarde, quando alguém tentasse escrever um `escolher` dentro de
um `if` dentro de uma função e descobrisse, só então, que a gramática não
previa aninhamento nenhum.

## 5. O que se verifica antes de rodar

**Pergunta:** que texto está bem escrito e mesmo assim não faz sentido?
Os erros que o sistema apanha antes de executar qualquer coisa. Nomeie as
naturezas de valor que a linguagem distingue.

**As naturezas de valor** que o Pinglu distingue (o sinal de erro desta
seção é justamente ter só uma natureza — o Pinglu tem sete, mais as
declaradas por `forma`): `int`, `string`, `char`, `decimal`, `float`,
`list`, e qualquer tipo declarado com `forma` (um tipo-soma fechado, ver
`docs/04_escolher_e_formas.md`). Combinar naturezas diferentes sem
conversão é um dos erros que a verificação estática apanha.

**O que se verifica, cada um bem escrito e mesmo assim sem sentido:**

- **Nome usado sem ter sido declarado** — `print(x);` onde `x` nunca
  apareceu numa declaração visível no escopo atual (a regra de
  `docs/01_recorte.md` § "Escopo léxico e vinculação de nomes"). Léxica e
  sintaticamente perfeito; semanticamente, `x` não existe.
- **Duas naturezas incompatíveis combinadas** — `int total = 5 +
"cinco";`. `5 + "cinco"` é uma expressão bem formada (`addExpr` aceita
  dois `primary` com `+` entre eles); a incompatibilidade é que `int` e
  `string` não têm operação `+` definida entre si.
- **Referência a algo que não existe** — usar, numa cláusula de
  `escolher`, o nome de uma variante que a `forma` correspondente não
  declara (ex. `hexagono(l) -> ...` quando `Geometria` só tem `circulo`,
  `retangulo`, `triangulo`). Sintaticamente é uma cláusula válida; a
  variante que ela nomeia não existe naquele tipo.
- **Cobertura do `escolher`** — o caso emblemático desta especificação:
  `exemplos/06_escolher_incompleto.pglu` está inteiramente bem escrito —
  toda cláusula é sintaticamente válida — e mesmo assim não faz sentido
  como decisão completa, porque uma variante declarada em `Geometria`
  ficou sem cláusula.

## 6. O que o sistema produz, e quem executa

**Pergunta:** o que fica gravado quando o tradutor termina, e quem lê
aquilo depois? O objeto produzido, o que ele contém, em que ordem, e o
componente separado que o lê e executa, possivelmente noutro momento, com
o tradutor já encerrado.

**Resposta:** o compilador do Pinglu produz **bytecode** — uma sequência
ordenada de instruções para uma máquina virtual de pilha
(`docs/01_recorte.md` § "O que o sistema produz — modelo de execução").
Cada instrução é uma operação pequena e fixa: empilhar um valor literal,
empilhar o valor de uma variável, uma operação aritmética ou relacional
(consome dois valores do topo da pilha, empilha o resultado), um desvio
condicional ou incondicional (implementa `if`/`while`/`for`/`escolher`
depois de verificados), chamar ou retornar de uma função, imprimir o topo
da pilha. A ordem das instruções é a ordem de **execução**, não a ordem
textual do código-fonte — um `while` vira um bloco de instruções com um
desvio de volta ao início, não uma cópia do texto do laço.

**Quem lê depois:** um componente separado — a máquina virtual de pilha —
consome essa sequência e a executa sobre uma entrada, num momento
possivelmente diferente do momento em que o compilador rodou, sem o
compilador presente. É a separação que justifica a decisão de
`docs/01_recorte.md`: existe uma fase de geração de código distinta da
execução, e o que passa de uma fase para a outra é o bytecode, não a
árvore nem o código-fonte. "O sistema mostra o resultado" descreveria um
interpretador rodando a AST direto — a alternativa que `01_recorte.md` já
descartou explicitamente; aqui há duas peças, e a segunda não precisa
saber que a primeira existiu.

## 7. A pergunta que você vai responder medindo

**Pergunta:** que dúvida sobre o seu sistema não se resolve olhando, só
medindo? A pergunta, a grandeza que a responde, a abordagem de referência
contra a qual ela será comparada, e o resultado que contrariaria a
expectativa.

**A pergunta:** a verificação de cobertura do `escolher`
(`docs/04_escolher_e_formas.md`) promete trocar "o caso não previsto
quebra em produção" por "o caso não previsto não compila". Isso realmente
acontece com mais frequência do que aconteceria numa cadeia de `if`/`else
if` escrita à mão para a mesma decisão, ou a diferença só existe no papel?

**A grandeza que responde:** para um conjunto de programas escritos com a
mesma decisão-por-forma (a cena da seção 1) em duas versões — uma com
`escolher` sobre um `forma`, outra com `if`/`else if` manual testando o
mesmo valor — a proporção de versões em que **falta um caso** e isso é
detectado **antes de rodar**, contra a proporção em que só é detectado (ou
nunca é) **rodando** com uma entrada que exercite o caso faltante.

**A abordagem de referência:** a versão escrita à mão com `if`/`else if`,
sem `escolher` — o que o feedback recebido chama de "descer uma escada de
condicionais". É a comparação natural porque é o que o Pinglu propõe
substituir, não uma linguagem externa.

**O resultado que contrariaria a expectativa:** a expectativa, dada a
decisão de `docs/04_escolher_e_formas.md`, é 100% de detecção antes de
rodar para a versão com `escolher` (a verificação de cobertura é
exaustiva por construção — compara duas listas fechadas, não amostra
entradas) e uma proporção menor que 100% para a versão com `if`/`else if`
manual (só detecta o caso faltante se alguma entrada de teste
especificamente o exercitar). Um resultado que contrariaria isso: **a
versão com `escolher` aceitar compilar com um caso faltando** — ou seja, a
verificação de cobertura ter um furo. Isso só é medível depois que o
verificador de cobertura existir de verdade (`scripts/verificar.py` hoje
só confere léxico, não semântica); fica registrado aqui como a pergunta
que a próxima fase do compilador precisa deixar respondível, não como algo
já medido.

## Onde é fácil errar

Responder cada pergunta com a resposta que "soa certa" para uma
especificação de linguagem, em vez de com a resposta que aquela pergunta
específica pede — o enunciado real veio com um "sinal de erro" por seção
exatamente para isso: cada seção tem um jeito característico de ficar
superficialmente preenchida e substancialmente vazia. Este documento
tentou responder cada seção esquivando do sinal de erro dela, não só
preenchendo o formato.

**Como verificar que está correta:** para cada uma das sete seções acima,
releia só a coluna "Sinal de erro" da imagem do enunciado e confira que a
resposta escrita aqui não cai nele. A seção 7, em particular, precisa ter
as quatro partes pedidas (pergunta, grandeza, referência, resultado
contrário) — se qualquer uma faltar, a seção está incompleta mesmo que
pareça uma resposta razoável.
