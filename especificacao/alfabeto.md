## Alfabeto da linguagem pinglu:

∑ = {A, a, B, b, C, c, D, d, E, e, F, f, G, g, H, h, I, i, J, j, K, k, L, l, M, m, N, n, O, o, P, p, Q, q, R, r, S, s, T, t, U, u, V, v, W, w, X, x, Y, y, Z, z, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, /, \*, -, +, \_, =, &, %, !, ;, ,, ', ", \, |, <, >, (, ), {, }, [, ], .}

## Dentro de comentário e de string (decisão de 27/10/2026)

O Σ acima vale para o **código**: identificadores, números, palavras reservadas,
símbolos e operadores. **Dentro de um comentário e dentro de uma string, qualquer
byte é aceito**: acentos, travessão, dois-pontos e tudo o mais. O lexer lê bytes e
não precisa entender UTF-8 para atravessá-los; os arquivos `.pglu` devem ser
salvos em UTF-8.

A recusa dentro de texto é dita por transição, não por alfabeto:

- **Comentário** (`//` até o fim da linha): todo byte, menos a nova linha, que o
  encerra.
- **String** (`"..."`): todo byte, menos a aspa dupla (que fecha), a barra invertida
  (que abre um escape) e a nova linha. Quebra de linha dentro de string só com o
  escape `\n`: sem isso, uma aspa esquecida engoliria o arquivo até a próxima
  (confirmado pelo grupo em 27/10/2026).

Fica de fora desta decisão (confirmado pelo grupo em 27/10/2026) o literal `char`, que continua restrito aos símbolos
ASCII de Σ mais o espaço: `'a'` é exatamente um símbolo, e uma letra acentuada
ocupa dois bytes em UTF-8, então `'é'` é recusado.

## Separadores (decisão de 27/10/2026)

**Entre** tokens, quatro bytes são **separadores**: o espaço, a tabulação (`\t`),
a nova linha (`\n`) e o retorno de carro (`\r`). O lexer os reconhece e os
**descarta**: eles não geram token e **não pertencem a Σ**. Servem só para separar
(`int x` precisa de um; `intx` é um identificador). Uma sequência deles é um
separador só, e o `\r\n` de um arquivo com fim de linha CRLF cai nessa regra. A
nova linha que encerra um comentário de linha também é um separador.

Em comentário e em string o caso já estava resolvido pela regra acima: lá eles são
bytes comuns (com a exceção da nova linha, que encerra o comentário e é recusada
na string).

## Símbolos removidos de Σ (decisão de 27/10/2026)

O `?`, a crase (`` ` ``) e o `´` saíram de Σ: nenhuma classe léxica os usava fora de
texto. Σ passa de 89 para **86 símbolos, todos ASCII** (52 letras, 10 dígitos e 24
símbolos). Consequências:

- **Em código**, os três deixam de existir: um `?` solto é erro léxico.
- **Em comentário e em string**, continuam valendo como qualquer outro byte (ver
  "Dentro de comentário e de string"): `"quem? sim"` e `// veja: ´` são aceitos.
- **No literal `char`**, que fica restrito a Σ mais o espaço, `'?'` passa a ser
  recusado. Antes da remoção era aceito.
- O `´` era o único símbolo de Σ fora do ASCII: com a saída dele, Σ inteiro cabe
  em um byte por símbolo.
