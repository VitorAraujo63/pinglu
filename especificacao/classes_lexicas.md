## Classes Lexicas (Pinglu)

Reorganizado das classes lexicas em três grupos, alinhados com a técnica de reconhecimento do
módulo 3: **palavras reservadas** (reconhecidas pelo mesmo autômato do
identificador, com consulta a tabela — nenhuma exige autômato próprio),
**símbolos/delimitadores** (forma fixa, sem relação com identificador) e
**operadores** (agrupados por categoria semântica). Uma seção de **literais**
lista os tokens de valor cuja forma ainda não foi fixada; o formato exato de
cada um é decisão de projeto do autômato correspondente (módulo 3), não
deste documento.

## Palavras Reservadas

int -- tipo: valor inteiro <br>
float -- tipo: valor de ponto flutuante (substitui `decimal`: um único
tipo para valor fracionário, evitando dois formatos de literal
numérico com casa decimal) <br>
string -- tipo: cadeia de caracteres <br>
char -- tipo: caractere isolado <br>
bool -- tipo: valor lógico <br>
list -- tipo: lista de valores <br>
true -- literal booleano: verdadeiro <br>
false -- literal booleano: falso <br>
func -- declara uma função, ex: "func soma(int a, int b) { return a + b; }" <br>
return -- devolve um valor do corpo de uma função, ex: "return a + b;" <br>
if -- operador condicional de ação única <br>
else -- alternativa executada quando a condição do if falha <br>
while -- repetição condicional, interrompida quando a condição deixa de valer <br>
for -- repetição com inicialização, condição e passo, ex: "for (int i = 0; i != 10; i = i + 1) { print(i); }" <br>
pineach -- repetição sobre os elementos de uma lista, ex: "pineach (item in lista) { print(item); }" <br>
in -- liga a variável de iteração à lista percorrida por um pineach <br>
print -- saída de um valor para o usuário, ex: "print(x)" <br>
forma -- declara um tipo-soma fechado, listando suas variantes, ex: "forma Geometria { circulo(float r); }" (ver docs/02_escolher_e_formas.md) <br>
escolher -- abre um bloco de casamento de padrão sobre as formas de um valor, ex: "escolher f { ... }" (ver docs/02_escolher_e_formas.md) <br>

## Literais

Tokens de valor cuja grafia varia; não são palavras fixas, e por isso não
cabem no autômato de identificador nem na tabela de reservadas. `identificador`
e os dois numéricos ainda não têm o padrão fixado — isso é trabalho de
projeto de autômato (módulo 3), não deste documento.

identificador -- nome de variável, função ou parâmetro declarado pelo programador <br>
numero_inteiro -- literal do tipo int <br>
numero_float -- literal do tipo float <br>
cadeia_de_caracteres -- literal do tipo string, delimitado por aspas duplas: `"texto"`.
Escapes: `\"`, `\\`, `\n`, `\t` <br>
caractere -- literal do tipo char, delimitado por aspas simples e com
exatamente um caractere entre elas: `'a'`. Escapes: `\'`,
`\\`, `\n` (o resultado do escape ainda conta como um
único caractere) <br>
lista_literal -- ver seção "Listas" abaixo <br>

## Simbolos / Delimitadores

`=` -- atribuição de valores, ex: "int exemplo = 3" <br>
`->` -- liga o padrão de uma cláusula do escolher ao código que ela executa <br>
`(` -- abre lista de parâmetros, condição ou agrupamento de expressão <br>
`)` -- fecha lista de parâmetros, condição ou agrupamento de expressão <br>
`{` -- abre bloco de código <br>
`}` -- fecha bloco de código <br>
`[` -- abre literal de lista ou indexação (ver seção "Listas") <br>
`]` -- fecha literal de lista ou indexação (ver seção "Listas") <br>
`,` -- separa parâmetros, argumentos ou elementos de uma lista <br>
`;` -- termina uma instrução <br>

## Listas

Sintaxe definida:

- **Literal**: `[elemento, elemento, ...]`, com lista vazia permitida: `[]`
- **Indexação**: `identificador[expressao]`

As duas formas compartilham os mesmos tokens léxicos `[` e `]` — o lexer não
distingue "isto é um literal" de "isto é uma indexação", e não precisa: essa
decisão é do analisador sintático, pela posição em que o colchete aparece
(logo após um identificador ou expressão → indexação; em posição de valor →
literal).

## Operadores

**Aritméticos** <br>
`+` -- soma <br>
`-` -- subtração <br>
`*` -- multiplicação <br>
`/` -- divisão <br>
`%` -- resto da divisão <br>

**Comparação** <br>
`==` -- igual, dois valores iguais TRUE, diferentes FALSE <br>
`!=` -- diferente, dois valores diferentes TRUE, iguais FALSE <br>
`<` -- menor que <br>
`>` -- maior que <br>
`<=` -- menor ou igual <br>
`>=` -- maior ou igual <br>

**Lógicos** <br>
`&&` -- e, os dois valores cumprindo a condição TRUE, senão FALSE <br>
`||` -- ou, um valor cumprindo a condição TRUE, nenhum FALSE <br>
`!` -- negação de um valor lógico isolado, ex: "!ativo" <br>

**Nota — prefixos ambíguos.** `-` é prefixo de `->`; `=` é prefixo de `==`;
`<` é prefixo de `<=`; `>` é prefixo de `>=`; `!` é prefixo de `!=`. Em todos
os casos o autômato correspondente precisa de um estado intermediário do
tipo "vi o primeiro símbolo, ainda não sei se para aqui" — o mesmo padrão do
comentário de linha do módulo 3. A regra do analisador léxico é sempre a de
**casamento mais longo** (maximal munch): `=` seguido de `=` sempre produz
`==`, nunca dois tokens `=` separados.

## Comentarios

Comentário de linha: `//` seguido de qualquer coisa até o fim da linha.

## Observacoes abertas

- O `-` unário (sinal negativo, ex. `-x`) usa o mesmo token léxico que a
  subtração binária; a distinção entre os dois é trabalho do analisador
  sintático, não do léxico — vale registrar isso quando a gramática for
  escrita, para não tentar resolver no autômato o que pertence ao parser.
