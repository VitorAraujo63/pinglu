# Pinglu

<br>

Repositório para a aula de Compiladores

Jeann Garçoni Alves <br>
Vitor Henrique Pereira Araujo <br>
Mateus Amaro da Silva <br>
Tiago Gabriel da Silva Cruz <br>
Vinicius Alves da Santana <br>
Daniel Ferreira Lopes

## Como rodar

Pré-requisitos: `make`, `g++` com suporte a C++20 e `python3` 3.9 ou superior
(testado com `g++` 13.3 e `python3` 3.12).

**Um único comando roda todos os testes** (módulos 02 a 05, inclusive as duas
partes da atividade dos módulos 4 e 5):

```
make verificar
```

Ele sai com código diferente de zero se qualquer caso divergir do esperado.
Para rodar só uma parte:

```
make regex         # só o analisador de expressões regulares (02)
make afd           # só os autômatos finitos determinísticos feitos à mão (03)
make afn           # só o AFN, a construção de Thompson e a bateria conferida à mão (04)
make determinizar  # módulo 05: determinização, minimização, as três máquinas e a explosão
make minimizar     # o mesmo que make determinizar
make limpar        # apaga build/
```

A organização das pastas está em `docs/01_organizacao_repositorio.md`.

## Onde está cada parte

| Parte                                   | Interface                   | Implementação                                       |
| --------------------------------------- | --------------------------- | --------------------------------------------------- |
| Árvore sintática (ponto de partida)     | `include/02_regex_pinglu.h` | `src/02_regex_pinglu.cpp`                           |
| Construção de Thompson                  | `include/04_afn.h`          | `src/04_afn.cpp` (`construirThompson`)              |
| Fecho vazio, movimento e simulação      | `include/04_afn.h`          | `src/04_afn.cpp` (`fechoVazio`, `mover`, `simular`) |
| Determinização (subconjuntos)           | `include/05_determinizacao.h` | `src/05_determinizacao.cpp` (`determinizar`)      |
| Minimização (refinamento de partições)  | `include/05_minimizacao.h`  | `src/05_minimizacao.cpp` (`completar`, `estadosAlcancaveis`, `minimizar`) |
| Comparação das três máquinas            | `include/05_minimizacao.h`  | `src/05_minimizacao.cpp` (`todasAsCadeias`, `compararAsTres`) |

Os testes:

| O que                                          | Onde                         |
| ---------------------------------------------- | ---------------------------- |
| Bateria conferida à mão (padrão, cadeia, veredito) | `exemplos/07_casos_afn.txt` (106 casos) |
| Testes de Thompson e da simulação (Parte 1)    | `demos/04_demo.cpp`          |
| Testes da determinização, da minimização, das três máquinas e da explosão (Parte 2) | `demos/05_demo.cpp` |

Decisões e medidas de cada módulo: `docs/04_construcao_thompson.md` e
`docs/05_determinizacao_minimizacao.md`.

## Conferências pedidas pela atividade

Saída de `make verificar` (as contagens do AFD e do mínimo incluem o estado de
erro quando ele é alcançável; a do AFN não):

- `(a|b)*c`: AFN com **10 estados, 12 transições e 9 vazias**; aceita `ababac`
  e recusa `abab`.
- `(a|b)*c` sobre {a, b, c}: AFN 10 → AFD **5** → mínimo **3** estados
  (contado o de erro); refinamento `2 -> 3 -> 3`.
- Todas as cadeias até o comprimento 4: **121 cadeias**, e as três máquinas
  aceitam **as mesmas 15**.

## O crescimento de estados

Padrão construído de propósito: a linguagem das cadeias sobre {a, b} cujo
**n-ésimo símbolo a partir do fim é `a`**, escrita como
`(a|b)*a(a|b)...(a|b)`, com n − 1 cópias de `(a|b)`. Contagens impressas por
`make determinizar` (seção H):

|  n | AFN | AFD | mínimo | 2ⁿ |
| -: | --: | --: | -----: | --: |
|  1 |  10 |   3 |      2 |   2 |
|  2 |  16 |   5 |      4 |   4 |
|  3 |  22 |   9 |      8 |   8 |
|  4 |  28 |  17 |     16 |  16 |
|  5 |  34 |  33 |     32 |  32 |
|  6 |  40 |  65 |     64 |  64 |
|  7 |  46 | 129 |    128 | 128 |
|  8 |  52 | 257 |    256 | 256 |

O AFN cresce de 6 em 6 (6n + 4); o AFD dobra a cada n (2ⁿ + 1); o mínimo tem
exatamente 2ⁿ. Como o mínimo dobra junto com o AFD, a explosão é **exigência
da linguagem** (um AFD precisa lembrar os últimos n símbolos) e não
desperdício da construção de subconjuntos. O estado a mais do AFD é o inicial,
equivalente ao estado "só `b` nos últimos n símbolos"; a minimização o funde.
Não há estado de erro: toda cadeia pode ser prefixo de uma cadeia aceita.
