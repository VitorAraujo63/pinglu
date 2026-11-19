# A determinização e a minimização — decisões e medidas

O que entrou neste módulo: `include/05_determinizacao.h` e
`src/05_determinizacao.cpp` (o AFD gerado e a construção de subconjuntos),
`include/05_minimizacao.h` e `src/05_minimizacao.cpp` (completar, descartar
inacessíveis, refinamento de partições e a comparação exaustiva das três
máquinas) e `demos/05_demo.cpp` (a demonstração, que também é o teste). Os
números deste documento saem de `make determinizar`.

**Convenção das contagens:** as do AFD e do mínimo incluem o estado de erro
quando ele é alcançável; a do AFN não tem estado de erro (lá, a falta de
transição já é caminho morto). A demonstração declara isso antes de imprimir
qualquer número.

## A determinização

Cada estado do AFD representa um conjunto de estados do AFN: os estados em que
o AFN poderia estar depois de ler o mesmo prefixo. O estado inicial é
`E({q0})`, a transição de `S` com `a` é `E(mover(S, a))`, e `S` aceita quando
contém o estado de aceitação do AFN.

- **O AFD gerado é uma estrutura nova (`AfdGerado`), e não o `Afd` do módulo 03.**
  Aquele precisa do número de estados no construtor e já traz o erro
  implícito; aqui os estados nascem conforme os conjuntos são descobertos.
  O projeto de referência resolveu o mesmo problema com uma tabela
  intermediária; um `std::vector<std::map<int, Estado>>` que cresce dispensa a
  tabela.
- **O conjunto é ordenado antes de ser chave do catálogo.** `{0, 2, 4}` e
  `{4, 0, 2}` caem na mesma chave. A seção B confere que nenhum par de estados
  representa o mesmo conjunto.
- **O conjunto vazio é o estado de erro e não tem caso especial:** recebe
  número na primeira vez que aparece. Por isso o AFD sai completo, e o vazio
  só existe quando é alcançável. Em `(a|b)*c` ele é o D4; em `(a|b)*abb` nenhuma
  cadeia o alcança e o AFD tem 5 estados (o material, que cria o erro de
  antemão, conta 6).
- **O alfabeto é o do AFN, fechado.** Como o Pinglu não tem coringa, um símbolo
  de fora é recusado pelo AFN (nenhuma transição o lê) e pelo AFD (não há
  coluna). As três máquinas concordam em **qualquer** cadeia, e por isso a
  comparação inclui um símbolo de fora do alfabeto.

## A minimização

Refinamento de partições, nos quatro passos que a atividade pede:

1. **Completar** (`completar`): se faltar alguma transição, um estado de erro
   recebe todas, inclusive as dele. O AFD da determinização já é completo, e
   para ele a função devolve uma cópia igual; ela existe para `minimizar`
   aceitar um AFD escrito à mão.
2. **Descartar inacessíveis** (`estadosAlcancaveis`, busca em largura). Sem
   isso o resultado não é o mínimo.
3. **Partição inicial:** aceita / não aceita (a cadeia vazia já os distingue).
4. **Rodadas até o ponto fixo.** A assinatura de um estado é o bloco em que ele
   está seguido do bloco de destino de cada símbolo, toda calculada contra a
   partição da rodada anterior. Estados com a mesma assinatura ficam juntos.

Decisões:

- **O ponto fixo é detectado pela contagem de blocos de duas rodadas
  seguidas**, e não por um número esperado. Como a assinatura começa pelo
  bloco atual, uma rodada só divide, nunca junta; se a contagem não cresceu,
  nada se dividiu. Por isso a sequência impressa termina sempre com dois
  números iguais: `2 -> 3 -> 3` em `(a|b)*c`.
- **Um vetor `blocoDe[estado]`** no lugar de procurar o bloco de cada estado
  na partição: acesso direto, sem o fator `n` a mais que o guia aponta na
  versão com `grupoDe`.
- **O mínimo é um `AfdGerado`**, e `representa` guarda o bloco de estados do
  AFD fundidos em cada estado. A tabela impressa mostra `M0 = {D0, D1, D2}`.
- **O AFD recebido não é alterado:** a minimização trabalha sobre uma cópia.

## As medidas

| Pattern                    | AFN | AFD | mínimo | rodadas           |
| -------------------------- | --: | --: | -----: | ----------------- |
| `(a\|b)*c`                 |  10 |   5 |      3 | 2 → 3 → 3         |
| `(a\|b)*abb`               |  14 |   5 |      4 | 2 → 3 → 4 → 4     |
| `-?[0-9]+`                 |  84 |  23 |      4 | 2 → 3 → 4 → 4     |
| identificador              | 462 | 118 |      3 | 2 → 3 → 3         |
| número                     | 162 |  43 |      5 | 2 → 3 → 4 → 5 → 5 |
| operadores_e_delimitadores | 108 |  28 |      7 | 2 → 7 → 7         |

`-?[0-9]+` dá os mesmos 84 → 23 → 4 do projeto de referência. O corte da
minimização vem das classes como `[0-9]`, que o módulo 02 troca por uma
alternância de dez dígitos cujos estados são indistinguíveis.

Nas três classes léxicas, **o mínimo tem exatamente o tamanho das máquinas
projetadas à mão no módulo 03** (2 + erro, 4 + erro e 6 + erro), e as três
concordam com elas em milhares de cadeias. As máquinas feitas à mão já eram
mínimas, e duas construções independentes chegaram ao mesmo lugar.

## A comparação das três máquinas

`compararAsTres` passa cada cadeia pelo AFN, pelo AFD e pelo mínimo e guarda a
primeira discordância **de cada salto** (AFN → AFD e AFD → mínimo). Comparar
só as pontas esconderia dois erros que se cancelam, um em cada etapa.

As cadeias são **todas** as cadeias sobre o alfabeto até um comprimento
(`todasAsCadeias`), e não uma amostra. Para `(a|b)*c` até o comprimento 4: 121
cadeias, e as três máquinas aceitam as mesmas 15.

O que isso prova e o que não prova: uma discordância é defeito certo;
concordar em todas é evidência. Há uma exceção: entre dois AFDs **completos**
com `n1` e `n2` estados, concordar em todas as cadeias até `n1 + n2 - 2`
garante a equivalência. Para o AFD (5) e o mínimo (3) de `(a|b)*c`, são as
1093 cadeias até o comprimento 6, e a demonstração as confere. Entre o AFN e o
AFD a garantia continua vindo da construção (abaixo).

## A explosão de estados

O caso construído de propósito é "o n-ésimo símbolo a partir do fim é `a`":
`(a|b)*a(a|b)...(a|b)`. A tabela completa (n = 1..8) está no `README.md`. O AFN
cresce de 6 em 6 (6n + 4: 10 a 52 estados); o AFD dobra (2ⁿ + 1: 3 a 257); o
mínimo tem exatamente 2ⁿ (2 a 256).

O mínimo dobra junto com o AFD, e é ele que refuta a suspeita de que a
determinização foi mal implementada: se a explosão fosse desperdício da
construção, a minimização a desfaria. Ela não desfaz, porque um AFD precisa
lembrar os últimos n símbolos, e são 2ⁿ combinações. O único estado do AFD
acima de 2ⁿ é o inicial, equivalente ao estado "só `b` nos últimos n
símbolos".

## Por que as duas famílias reconhecem a mesma classe (para o diário)

**AFD → AFN** é imediato: um AFD é um AFN em que cada estado, com cada
símbolo, tem exatamente um destino e não há transição vazia.

**AFN → AFD** é a construção de subconjuntos. O invariante: depois de ler
qualquer prefixo `w`, o estado do AFD representa exatamente o conjunto de
estados em que o AFN poderia estar. Vale para `w` vazio, porque o inicial do
AFD é, por definição, `E({q0})`. Se vale para `w`, vale para `wa`, porque a
transição é `E(mover(S, a))`, que é como a simulação do AFN avança o conjunto
ativo. Por indução no comprimento, vale para a cadeia inteira, e o AFD aceita
quando o conjunto contém o estado de aceitação: exatamente o critério de
aceitação por existência de caminho do AFN.

**A minimização preserva a linguagem.** Cada rodada só separa estados, nunca
junta: a sequência de partições é monótona, o número de blocos não diminui e
não passa do número de estados, então o processo termina. No ponto fixo, dois
estados do mesmo bloco vão, com cada símbolo, ao mesmo bloco, e por indução no
comprimento da cadeia nenhuma cadeia os distingue: são equivalentes. Fundir
estados equivalentes não muda a linguagem, e como estados de blocos diferentes
são distinguíveis, não há o que fundir a mais.

A bateria de cadeias continua necessária por outro motivo: ela confere que o
código faz o que o argumento manda, e é no código que moram os defeitos
(fecho esquecido, conjunto não ordenado, refinamento que para cedo).

## Defeitos injetados

Para saber se os testes acusam defeitos na minimização, cada um foi injetado
numa cópia de `src/05_minimizacao.cpp`, um por vez:

| Defeito injetado                                      | Acusado em                                     |
| ----------------------------------------------------- | ---------------------------------------------- |
| refinamento para na primeira rodada                   | C, F, G (34 verificações; ex.: `/abc/` recusa `abc`) |
| assinatura sem os blocos de destino                   | C, F, G (59 verificações)                      |
| estados inacessíveis não descartados                  | F (o AFD com estado inacessível sai com 4 estados) |
| AFD não completado antes de refinar                   | exceção (`bad optional access`), saída com código 1 |

Foi feito com um script descartável, fora do repositório; a tabela é o registro.
O "não descartar inacessíveis" é acusado só pelo caso escrito à mão da seção F:
o AFD da determinização nunca tem estado inacessível, e sem esse caso o
defeito passaria.
