# A representação da função de transição — decisão e a conta

**Decisão do grupo, registrada em 27/10/2026.** A escolha abaixo foi revisada e
adotada. Os números saem de `make afd`; as pendências no fim deste documento
continuam abertas e não fazem parte da decisão.

## O que foi escolhido

**Matriz densa**, indexada por estado e por coluna de símbolo (`src/03_afd.h`). A
coluna sai de um mapeamento byte → coluna calculado uma vez no construtor, e
consultar uma transição custa uma multiplicação, uma soma e um acesso a vetor.

A função é **total por construção**: existe um estado de erro absorvente, e toda
posição da tabela nasce apontando para ele. Quem monta a máquina declara só as
transições que não vão para o erro.

## A conta

Com `n` estados úteis e `m` símbolos no alfabeto da máquina:

    bytes = (n + 1) × m × sizeof(Estado)

O `+ 1` é o estado de erro. Com `Estado = std::size_t` (8 bytes nesta plataforma),
as sete máquinas das classes léxicas ocupam (saída de `make afd`, última seção):

| Autômato                   |   n |   m | Posições | Bytes | Não-erro | Para o erro |
| -------------------------- | --: | --: | -------: | ----: | -------: | ----------: |
| identificador              |   2 |  63 |      189 |  1512 |      116 |         39% |
| numero                     |   4 |  11 |       55 |   440 |       41 |         25% |
| cadeia_de_caracteres       |   4 | 256 |     1280 | 10240 |      260 |         80% |
| caractere                  |   5 |  87 |      522 |  4176 |       91 |         83% |
| comentario_de_linha        |   3 | 256 |     1024 |  8192 |      257 |         75% |
| operadores_e_delimitadores |   6 |  19 |      133 |  1064 |       23 |         83% |
| separadores                |   2 |   4 |       12 |    96 |        8 |         33% |
| **Total**                  |     |     |          | 25720 |          |             |

Os números saem do programa (`bytesDaTabela()` e `transicoesDefinidas()`); se o
quadro divergir da saída de `make afd`, o quadro é que está errado.

**A coluna não é o byte, exceto onde o alfabeto são todos os bytes.** Indexar pelo
código do caractere daria 256 colunas por estado. O vetor byte → coluna custa
256 × 8 = 2048 bytes **por máquina**, não por estado:

| Autômato                   | Só a tabela | Tabela + mapeamento | Tabela indexada pelo byte |
| -------------------------- | ----------: | ------------------: | ------------------------: |
| identificador              |        1512 |                3560 |                      6144 |
| numero                     |         440 |                2488 |                     10240 |
| cadeia_de_caracteres       |       10240 |               12288 |                     10240 |
| caractere                  |        4176 |                6224 |                     12288 |
| comentario_de_linha        |        8192 |               10240 |                      8192 |
| operadores_e_delimitadores |        1064 |                3112 |                     14336 |
| separadores                |          96 |                2144 |                      6144 |
| **Total**                  |       25720 |               40056 |                     67584 |

O mapeamento se paga nas cinco máquinas de alfabeto estreito: com ele elas
ocupam 17528 bytes, contra 49152 se fossem indexadas pelo byte, e nas de
operadores e de número o ganho passa de 4 vezes. É um custo fixo por máquina,
porém: na de separadores, com 4 símbolos, o mapeamento (2048 bytes) é
21 vezes maior que a própria tabela (96). Nas duas máquinas de texto livre
(`cadeia_de_caracteres` e `comentario_de_linha`) o alfabeto são os 256 bytes, o
mapeamento é a identidade e não economiza nada: são 4096 bytes de mapeamento que
poderiam ser dispensados. Essas duas máquinas respondem por 18432 dos 25816 bytes
de tabela, e é o custo, medido, da decisão de aceitar qualquer byte dentro de
string e de comentário (ver "Pendências").

## A alternativa descartada

**Mapa esparso**: uma tabela de dispersão de `(estado, símbolo)` para o destino,
guardando só as transições que não vão para o erro.

Em entradas ele guarda muito menos: 83% da tabela de `caractere` e de
`operadores_e_delimitadores` só diz "erro". Em bytes a conta depende do custo de
cada entrada da dispersão e **não foi feita**; afirmar que o mapa ocuparia menos
seria afirmar sem medir.

Descartamos o mapa por duas razões que a memória não mede:

1. **O custo por símbolo lido.** O reconhecimento faz uma consulta por caractere do
   texto, e o lexer vai ler programas inteiros. Trocar o acesso a vetor por um
   cálculo de hash multiplica o custo da operação mais frequente do sistema.
2. **A tabela é também a forma do que o compilador emite.** O objeto produzido na
   geração de código carrega um vetor de AFDs; uma matriz densa é um bloco contíguo
   de inteiros, que se grava e se carrega como está. Um mapa esparso teria de ser
   serializado, e a escolha do formato voltaria quando já não houvesse tempo de
   refazê-la.

## Decisões complementares adotadas

- **Construir lança, reconhecer nunca lança.** Um símbolo fora do alfabeto ao
  _montar_ a máquina é defeito do programador e lança `std::invalid_argument`. Um
  símbolo fora do alfabeto ao _reconhecer_ é texto do usuário e é recusado com
  motivo. As três recusas (símbolo fora do alfabeto, sem transição, parou em
  estado não final) saem no próprio resultado (`Veredito`).
- **Sem sinal no número.** O `-` unário usa o mesmo token da subtração e a
  distinção é do analisador sintático (ver `especificacao/classes_lexicas.md`).
  Pôr o sinal no autômato faria `a-1` virar `a` seguido do número `-1`.
- **Um estado por distinção, não por posição.** `operadores_e_delimitadores` tem
  6 estados úteis, e não um por operador: `=`, `<`, `>` e `!` se comportam igual
  e dividem `pode_ganhar_igual`; `&` e `|` são promessas pendentes (sozinhos não
  são tokens); os operadores sem extensão e os de dois símbolos aceitam e não
  admitem mais nada, então são o mesmo estado, `completo`.
- **Qualquer byte dentro de string e de comentário** (27/10/2026). Fora de
  texto o alfabeto continua sendo o de Σ; dentro, a recusa é dita por
  transição (aspas, barra, nova linha), e não por alfabeto.
- **Separadores descartados pelo lexer** (27/10/2026): espaço, tabulação, nova
  linha e `\r` entre tokens. Fora de Σ, mas reconhecidos por uma máquina própria.
- **Σ com 86 símbolos** (27/10/2026): saíram o `?`, a crase e o `´`, que nenhuma
  classe léxica usava fora de texto.
- **Palavra reservada não tem máquina.** Lê-se pelo autômato do identificador e
  consulta-se uma tabela depois.

## Pendências

**Resolvidas em 27/10/2026.** O registro das regras está em
`especificacao/alfabeto.md`.

1. **Texto livre.** Dentro de comentário e de string, o Σ aceita qualquer byte.
   As máquinas de cadeia e de comentário declaram os 256 bytes como alfabeto.
   Passando os 6 arquivos de `exemplos/` e `docs/example.pglu` pelas máquinas, com
   casamento mais longo (um mini-lexer de teste, fora do repositório): **84 dos 84
   comentários de linha são aceitos** (antes eram 56) e **nenhum trecho de código
   ficou sem token**. O acento, o travessão `—` e o `:` deixaram de ser problema.
   O preço está na seção "A conta": as duas tabelas passaram a 256 colunas.
2. **Nova linha dentro de string** continua recusada (confirmado pelo grupo). Uma
   aspa esquecida engoliria o arquivo até a próxima aspa; quebra de linha em
   string só com o escape `\n`.
3. **O literal `char` ficou fora da decisão** (confirmado pelo grupo): continua
   restrito aos símbolos ASCII de Σ mais o espaço, e `'é'` é recusado, porque uma
   letra acentuada ocupa dois bytes em UTF-8.
4. **Separadores entre tokens.** Espaço, tabulação, nova linha e `\r` são
   separadores: o lexer os reconhece e os descarta, e eles não pertencem a Σ.
   Têm máquina própria (`separadores`, 2 estados), para que o que o lexer lê
   saia todo do mesmo motor.

5. **Símbolos sem uso.** O `?`, a crase e o `´` saíram de Σ, que passa de 89 para
   86 símbolos, todos ASCII. Dentro de texto continuam sendo bytes comuns; no
   literal `char`, `'?'` passa a ser recusado. A única máquina afetada foi a de
   `caractere` (de 89 para 87 colunas; ver "A conta").

**Nada em aberto sobre o alfabeto neste momento.**

## O que fica em aberto, e onde volta

A matriz densa começa a desperdiçar de forma visível quando a determinização
produzir máquinas com muitos estados sobre alfabetos largos. A saída conhecida é a
**compressão por linhas equivalentes** (estados com linhas idênticas compartilham
uma linha), que mantém o acesso em tempo constante. Há ainda duas economias
baratas já visíveis aqui: estreitar `Estado` para 16 bits (cada tabela cairia 4
vezes) e compartilhar o mapeamento byte → coluna entre máquinas de mesmo alfabeto
(a cadeia e o comentário usam o mesmo, e é a identidade: o
mapeamento deles pode ser dispensado). Ficam adiadas, com lugar marcado: a conta se
refaz quando a determinização existir, com os números das máquinas que ela
produzir, e não com os destas sete.

A decisão de aceitar qualquer byte em texto abre uma economia que a matriz com
uma coluna por símbolo não aproveita: **classes de colunas equivalentes**. Bytes
que se comportam igual em todos os estados dividem uma coluna, e o mapeamento
byte → coluna já existente passa a levar vários bytes à mesma coluna. Por conta,
**estimativa e não medida**: a cadeia precisaria de 6 colunas (`"`, `\`, `n`,
`t`, nova linha e "os outros") e o comentário de 3 (`/`, nova linha e "os
outros"), o que daria cerca de 240 e 96 bytes de tabela, contra os 10240 e 8192
medidos hoje. Fica adiada, com lugar marcado, e é a primeira a refazer quando o
tamanho das tabelas importar.
