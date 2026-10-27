# A representação da função de transição — decisão e a conta

Rascunho para o grupo revisar. A escolha abaixo segue a mesma linha do projeto de
referência do professor, mas a conta, as máquinas e as pendências são do Pinglu, e
cada afirmação aqui precisa ser defendida por quem assina o documento.

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
as seis máquinas das classes léxicas ocupam (saída de `make afd`, última seção):

| Autômato                   | n | m  | Posições | Bytes | Não-erro | Para o erro |
| -------------------------- | -: | -: | -------: | ----: | -------: | ----------: |
| identificador              | 2 | 63 |      189 |  1512 |      116 |         39% |
| numero                     | 4 | 11 |       55 |   440 |       41 |         25% |
| cadeia_de_caracteres       | 4 | 89 |      445 |  3560 |       94 |         79% |
| caractere                  | 5 | 89 |      534 |  4272 |       93 |         83% |
| comentario_de_linha        | 3 | 90 |      360 |  2880 |       91 |         75% |
| operadores_e_delimitadores | 6 | 19 |      133 |  1064 |       23 |         83% |
| **Total**                  |   |    |          | 13728 |          |             |

Os números saem do programa (`bytesDaTabela()` e `transicoesDefinidas()`); se o
quadro divergir da saída de `make afd`, o quadro é que está errado.

**A coluna não é o byte.** Indexar pelo código do caractere daria 256 colunas por
estado. O vetor byte → coluna custa 256 × 8 = 2048 bytes **por máquina**, não por
estado:

| Autômato                   | Só a tabela | Tabela + mapeamento | Tabela indexada pelo byte |
| -------------------------- | ----------: | ------------------: | ------------------------: |
| identificador              |        1512 |                3560 |                      6144 |
| numero                     |         440 |                2488 |                     10240 |
| cadeia_de_caracteres       |        3560 |                5608 |                     10240 |
| caractere                  |        4272 |                6320 |                     12288 |
| comentario_de_linha        |        2880 |                4928 |                      8192 |
| operadores_e_delimitadores |        1064 |                3112 |                     14336 |
| **Total**                  |       13728 |               26016 |                     61440 |

O mapeamento se paga: 26016 contra 61440 bytes. Nas máquinas de texto (89 colunas
de 256) o ganho é pequeno, de menos de 2 vezes; nas de poucos símbolos, como a de
operadores, passa de 4 vezes.

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

## Decisões que o código tomou e o grupo deve poder defender

- **Construir lança, reconhecer nunca lança.** Um símbolo fora do alfabeto ao
  *montar* a máquina é defeito do programador e lança `std::invalid_argument`. Um
  símbolo fora do alfabeto ao *reconhecer* é texto do usuário e é recusado com
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
- **Palavra reservada não tem máquina.** Lê-se pelo autômato do identificador e
  consulta-se uma tabela depois.

## Pendências

1. **O alfabeto não tem espaço nem nova linha.** `especificacao/alfabeto.md`
   enumera 89 símbolos, mas os exemplos usam espaço em todo lugar
   (`print("fim do for")`) e a máquina de comentário só pode parar "no fim da
   linha" se a nova linha existir. As máquinas de texto acrescentam o espaço, e
   a de comentário também a nova linha. O alfabeto deve ser emendado para isso
   (ou decidir que espaço e quebra de linha são separadores fora de Σ, e então
   dizer como string e comentário os contêm).
2. **O `´` (U+00B4) não é ASCII.** Ocupa dois bytes em UTF-8, e uma máquina
   indexada por byte não o enxerga como um símbolo só. Ficou fora das máquinas
   (as de texto têm 88 símbolos ASCII de Σ mais o espaço). Nenhuma classe léxica
   o usa: vale decidir se ele fica no alfabeto.
3. **Acentos em comentários e strings.** Vários exemplos têm comentários com
   acento (`// ...seção 4 do feedback...`). Com o alfabeto atual, esses comentários
   são recusados pela máquina de comentário (o byte `\xc3` está fora do alfabeto).
   Isso se resolve com a pendência 1: ou Σ passa a aceitar bytes acima de 0x7F
   dentro de texto, ou comentários ficam restritos a ASCII.
4. **`?`, `` ` `` e `´` não aparecem em nenhuma classe léxica.** Só são
   alcançáveis dentro de string, char e comentário.

## O que fica em aberto, e onde volta

A matriz densa começa a desperdiçar de forma visível quando a determinização
produzir máquinas com muitos estados sobre alfabetos largos. A saída conhecida é a
**compressão por linhas equivalentes** (estados com linhas idênticas compartilham
uma linha), que mantém o acesso em tempo constante. Há ainda duas economias
baratas já visíveis aqui: estreitar `Estado` para 16 bits (cada tabela cairia 4
vezes) e compartilhar o mapeamento byte → coluna entre máquinas de mesmo alfabeto
(as duas de texto usam o mesmo). Ficam adiadas, com lugar marcado: a conta se
refaz quando a determinização existir, com os números das máquinas que ela
produzir, e não com os destas seis.
