# Padrões léxicos do Pinglu: do padrão à árvore reduzida e de volta

Entrega do módulo de expressões regulares e linguagens regulares. As
peças descritas aqui são as que `especificacao/classes_lexicas.md` e
`especificacao/alfabeto.md` já fixam; o núcleo de operadores usado é o
que `docs/02_nucleo_minimo.md` já decidiu para o projeto — esta entrega
aplica aquele núcleo a cada peça, não redecide o que ele é. Ver
`docs/05_guia_da_atividade.md` para o enunciado traduzido em passos.

## Notação usada abaixo

Três operadores de núcleo — `concat`, `alt`, `fecho` — e duas folhas —
símbolo literal (`'x'`) e cadeia vazia (`vazio`). Para não reescrever a
mesma subárvore muitas vezes, quatro classes são nomeadas uma vez e
citadas pelo nome nas linhas que as usam (exatamente como o próprio
enunciado nomeia `D` para não reescrever a alternância dos dez dígitos
em cada linha):

- **D** — um dígito: `alt(alt(alt(alt(alt(alt(alt(alt(alt('0','1'),'2'),'3'),'4'),'5'),'6'),'7'),'8'),'9')` — 10 símbolos, **19 nós**.
- **L** — letra ou `_` (início de identificador): alternância das 52 letras (`A`-`Z`, `a`-`z`) mais `_` — 53 símbolos, **105 nós**.
- **C** — letra, dígito ou `_` (continuação de identificador): alternância das 52 letras, 10 dígitos e `_` — 63 símbolos, **125 nós**.
- **N** — qualquer símbolo do alfabeto (`especificacao/alfabeto.md`, 89 símbolos) exceto `"` — 88 símbolos, **175 nós**.

A fórmula usada em toda linha da tabela: uma classe com *n* símbolos vira
uma árvore com *2n − 1* nós (*n* folhas de símbolo, *n − 1* nós de
alternância juntando-as duas a duas). É a mesma conta que dá 51 para
`[a-z]` no enunciado.

## 1–2. Tabela de peças, com a cobertura mínima

| Peça | Trecho real | Padrão com açúcar | Núcleo | Forma linear | Nós |
| --- | --- | --- | --- | --- | --- |
| Número inteiro (**usa fecho**) | `5` em `int a = 5;` (`exemplos/01_aritmetica_condicional.pglu`) | `[0-9]+` | `concat(D, fecho(D))` | `concat(D, fecho(D))` | **40** |
| Número decimal (**usa opcional**) | `3.14` em `exemplos/05_escolher_completo.pglu` | `[0-9]+(\.[0-9]+)?` | `concat( concat(D,fecho(D)), alt( concat('.', concat(D,fecho(D))), vazio ) )` | `concat( concat(D,fecho(D)), alt( concat('.', concat(D,fecho(D))), vazio ) )` | **85** |
| Identificador | `soma` em `exemplos/04_funcao.pglu` | `[A-Za-z_][A-Za-z0-9_]*` | `concat(L, fecho(C))` | `concat(L, fecho(C))` | **232** |
| Palavra fixa | `if`, `while`, `print` (`especificacao/classes_lexicas.md`) | `int|string|char|decimal|float|list|func|return|if|else|while|for|pineach|in|print|forma|escolher` | alternância dos 17 nomes, cada um `concat` das suas letras | `alt(...)` dos 17 `concat` de letras — ver §"expansão" abaixo | **159** |
| Texto entre aspas | `"par"` em `exemplos/01_aritmetica_condicional.pglu` | `"[^"]*"` | `concat( concat('"', fecho(N)), '"' )` | `concat( concat('"', fecho(N)), '"' )` | **180** |
| Sinal | `<` em `exemplos/02_for_e_relacionais.pglu` | `<|>|==|!=|&&|\|\||=|->|+|-` | alternância dos 10 sinais, cada um `concat` dos seus símbolos | `alt(...)` dos 10 — ver §"expansão" abaixo | **29** |

Seis linhas, o mínimo pedido era cinco. Cobertura mínima cumprida: número
inteiro usa `fecho` (o `+` reduzido), número decimal usa `alt(x, vazio)`
(o `?` reduzido).

### Expansão completa das linhas pequenas (conferíveis à mão)

**Sinal**, com os símbolos abreviados por brevidade — a árvore de fato
tem um nó de folha por caractere, não por sinal:

```
alt(alt(alt(alt(alt(alt(alt(alt(alt(
  concat('<'),
  concat('>')),
  concat('=','=')),
  concat('!','=')),
  concat('&','&')),
  concat('|','|')),
  concat('=')),
  concat('-','>')),
  concat('+')),
  concat('-'))
```

Conferência da conta: cada `concat('=')` de 1 caractere é, na verdade, só
a folha `'='` (concat de um único filho não introduz nó — mesma regra
usada para uma classe de 1 símbolo). Os de 2 caracteres (`==`, `!=`,
`&&`, `||`, `->`) valem 3 nós cada (2 folhas + 1 concat); os de 1
caractere (`<`, `>`, `=`, `+`, `-`) valem 1 nó cada. Soma: 5 sinais de 2
chars × 3 = 15, 5 sinais de 1 char × 1 = 5, mais 9 nós de alternância
(10 alternativas, 9 `alt` para juntá-las) = 15+5+9 = **29**, batendo com
a tabela.

**Palavra fixa** segue o mesmo princípio: cada palavra de *k* letras é um
`concat` de *k* folhas, valendo *2k − 1* nós; somam-se os 17 valores
(3,6,4,7,5,4,4,6,2,4,5,3,7,2,5,5,8 letras → 5,11,7,13,9,7,7,11,3,7,9,5,
13,3,9,9,15 nós, soma 143) e acrescentam-se 16 nós de alternância
(17 alternativas) = 143 + 16 = **159**.

## 3. Um par que converge

Duas escritas do número inteiro:

- `[0-9]+`
- `[0-9][0-9]*`

As duas reduzem, pelas mesmas regras da tabela do núcleo
(`docs/02_nucleo_minimo.md`), à mesma árvore: `[0-9]+` vira
`concat(D, fecho(D))` pela redução de `x+`; `[0-9][0-9]*` já **é**
`concat(D, fecho(D))` sem precisar de redução nenhuma, porque é a
definição do `*` escrita por extenso. Forma linear idêntica:
`concat(D, fecho(D))`. Contagem de nós idêntica: **40** e **40**.

## 4. Um par que não converge

Duas escritas do sinal relacional restrito a `<` e `>`:

- `alt('<', '>')`
- `alt('>', '<')`

As duas denotam a mesma linguagem — o conjunto de cadeias
`{"<", ">"}` — porque `alt` é comutativo *como conjunto de cadeias
aceitas*: a primeira aceita `<` ou `>`; a segunda aceita `>` ou `<`.
Mesma enumeração, ordem trocada; exibição completa das cadeias (a
linguagem é finita, dá para listar todas): ambas aceitam exatamente
`{<, >}` e recusam qualquer outra cadeia.

As árvores, porém, não são iguais como estrutura: `alt('<', '>')` tem
`'<'` como filho esquerdo e `'>'` como filho direito; `alt('>', '<')` tem
a ordem trocada. A forma linear de uma é `alt('<','>')`, a da outra é
`alt('>','<')` — comparando caractere por caractere, os textos diferem
na posição 4. O número de nós é igual (3 e 3), mas a comparação de forma
linear diria "diferentes".

**O que isso diz sobre comparar árvores:** igualdade de forma linear
detecta só a igualdade que sobrevive à ordem em que quem escreveu o
padrão colocou os operandos de uma alternância — não a igualdade de
linguagem. Dois padrões podem denotar exatamente o mesmo conjunto de
cadeias e ainda assim ter formas lineares diferentes, e o comparador de
árvores não tem como saber disso sem processamento a mais (canonizar a
ordem dos filhos de cada `alt`, ou — para o caso geral, não só reordenar
alternativas — converter cada árvore num autômato, minimizar os dois e
comparar os mínimos, que é o que a seção "O que esta atividade não
decide" do enunciado aponta como fora do escopo deste módulo).

## 5. Dois requisitos, um de cada lado

**Parece sair da classe regular, e não sai:** *"um literal decimal do
Pinglu tem no máximo 6 casas depois do ponto."* Parece exigir contar
dígitos até um limite, o que soa como precisar de memória — mas o limite
é fixo (6), não cresce com a entrada, então um autômato com um número
fixo de estados extras (um por casa já vista) resolve. A expressão que
resolve, sem contador nenhum, apenas repetindo o dígito opcional seis
vezes:

```
[0-9]+\.[0-9][0-9]?[0-9]?[0-9]?[0-9]?[0-9]?
```

Em núcleo: `concat( concat(D,fecho(D)), concat('.', concat(D, alt(D,vazio), alt(D,vazio), alt(D,vazio), alt(D,vazio), alt(D,vazio))) )`
— cada casa opcional além da primeira é um `alt(D, vazio)` a mais,
sempre finito porque o teto (6) está fixado no próprio padrão, não
lido do texto de entrada.

**Sai mesmo da classe regular:** *"o nome de uma variante citada numa
cláusula do `escolher` precisa ter sido declarado na `forma`
correspondente"* (o requisito semântico da linha "Referência a algo que
não existe" em `docs/03_especificacao_sete_secoes.md` §3, e o motivo de
existir `docs/04_escolher_e_formas.md`). Para checar isso, a máquina
precisaria lembrar **todos os nomes de variante já declarados por toda
`forma` do programa até aquele ponto** — e a quantidade de `forma`s e de
variantes que um programa Pinglu pode declarar não tem teto: um
programa com 3 tipos-forma e um com 300 são igualmente válidos
sintaticamente. Sem teto declarado, esse requisito não cabe numa classe
de caracteres nem em um número fixo de estados extras — precisa de uma
tabela de símbolos, que é exatamente o que `docs/01_recorte.md` já prevê
("a tabela de símbolos... passa a ser uma pilha de escopos") e o que
este módulo (léxico) deixa para a análise semântica resolver depois.

## 6. Quatro recusas com posição

Os quatro casos abaixo são recusas do **padrão** (o texto que descreve
uma peça léxica, como as da tabela acima), não do programa Pinglu — é o
leitor de expressões regulares que ainda vai ler `docs/02_nucleo_minimo.md`
e construir essas árvores. Posição contada em caracteres a partir de 1,
formato `posição P: mensagem` (mesmo espírito do `linha L: mensagem` que
`docs/01_recorte.md` já fixa para erros do compilador do Pinglu — aqui a
unidade é caractere, porque um padrão normalmente é uma única linha).

| # | Malformação | Padrão de exemplo | Mensagem | Posição apontada |
| - | ----------- | ------------------ | -------- | ------------------ |
| 1 | Grupo que não fecha | `(int\|string` | `posição 12: grupo aberto em posição 1 nunca fechado` | 12 — fim do texto, onde a falta se constata (não dá para saber que falta um `)` antes de acabar o texto) |
| 2 | Repetição sem operando | `+[0-9]` | `posição 1: '+' sem expressão à esquerda para repetir` | 1 — onde o `+` aparece, sem nada antes dele |
| 3 | Classe sem colchete final | `[0-9` | `posição 5: classe de caracteres aberta em posição 1 nunca fechada com ']'` | 5 — fim do texto, mesma lógica do caso 1 |
| 4 | Símbolo sobrando depois do fim | `(int\|string))` | `posição 13: símbolo ')' sobrando depois do fim da expressão` | 13 — a posição do `)` extra, já com a expressão anterior completa e válida |

Nos casos 1 e 3 a posição é o fim do texto porque é ali que a ausência se
comprova — igual ao exemplo `a(b|c` do enunciado, onde só ao chegar ao
fim sem achar `)` dá para dizer que o grupo ficou aberto.

## Como conferir sozinho, antes de entregar

| Verificação | Resultado | Observação |
| --- | --- | --- |
| Contagem de nós de cada linha do item 1 | **Confirmada nesta redação** (uma pessoa) — pendente uma segunda contagem independente por outro integrante do grupo antes de valer como conferida, conforme o critério do enunciado ("duas pessoas... em separado") | A conta de maior risco de erro é a de **identificador** (232 nós) e **string** (180 nós), por terem as classes maiores (`C` com 63 símbolos, `N` com 88) — comece a recontagem por elas |
| Convergência do par do item 3 | **Confirma** — as duas formas lineares são o mesmo texto, `concat(D, fecho(D))`, e a mesma contagem, 40 e 40 | — |
| Leitura de volta (reconstrução de `[0-9]+` a partir de `concat(D, fecho(D))`) | **Confirma** — percorrendo a árvore, a raiz é `concat`, o filho esquerdo é a alternância dos 10 dígitos entre parênteses e o direito é essa mesma alternância seguida de `*`; a expressão reconstruída, `(0\|1\|2\|3\|4\|5\|6\|7\|8\|9)(0\|1\|2\|3\|4\|5\|6\|7\|8\|9)*`, reduzida de novo, dá a mesma forma linear `concat(D, fecho(D))` | Comparar com `[0-9]+`: 6 caracteres digitados contra 43 na forma sem açúcar — a mesma relação que o enunciado mostra para o exemplo dele |

## Onde esta atividade já aponta uma mudança de notação no Pinglu

A linha de maior contagem da tabela é **identificador**, com 232 nós, e a
causa é decidível sem esperar a construção do reconhecedor: hoje
`L`/`C` incluem maiúsculas e minúsculas (52 letras cada classe), mas
**nenhum** identificador em `exemplos/*.pglu` usa maiúscula — `soma`,
`contador`, `resultado`, `numeros`, `item`, `lista`, `r`, `b`, `h`,
`base`, `altura` são todos minúsculos (as únicas maiúsculas do projeto
são nomes de tipo-forma, como `Geometria`, que é um caso à parte, não um
identificador comum). Restringir identificador a `[a-z_][a-z0-9_]*`
reduziria `L` de 53 para 27 símbolos (53 nós) e `C` de 63 para 37
(73 nós), cortando a peça de 232 para 126 nós — quase pela metade — sem
mudar nenhum exemplo já escrito. Esta é exatamente a decisão que o
enunciado descreve como "barata agora, cara três capítulos adiante":
fica registrada aqui como recomendação para `docs/01_recorte.md`, não
aplicada por conta própria nesta entrega porque é uma decisão de escopo
da linguagem (maiúscula ser ou não válida em nome de variável), que
cabe ao grupo fechar, não a este documento de contagem.

## Onde é fácil errar (revisão desta entrega)

Os mesmos cinco erros que o enunciado lista foram checados nesta
redação: a folha `vazio` (cadeia vazia) nunca foi trocada pela folha de
conjunto vazio em nenhuma redução acima; a concatenação de identificador
e sinal está escrita como árvore associando à esquerda de forma
explícita (`concat(concat(...), ...)`), não deixada implícita; nenhuma
subárvore é referenciada duas vezes por posição (`D` é *nomeada* para
economizar escrita neste documento, mas cada ocorrência sua na árvore
real é uma cópia independente — a nota da tabela acima em "Notação
usada" e a duplicação do `+` em `docs/02_nucleo_minimo.md` dizem a mesma
coisa); as quatro recusas do item 6 têm posição, não só mensagem; e
nenhuma notação de conveniência (`+`, `?`, `[...]`) aparece dentro das
árvores da coluna "Núcleo" — só nas colunas "Padrão com açúcar", que
existem exatamente para separar as duas coisas.
