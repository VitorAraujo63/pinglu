# A construção de Thompson e a simulação do AFN — decisões e medidas

O que entrou neste módulo: `src/04_afn.h/.cpp` (a máquina, a construção, a
simulação e a leitura da bateria), `demos/04_demo.cpp` (a demonstração, que
também é o teste) e `exemplos/07_casos_afn.txt` (a bateria conferida à mão). Os
números deste documento saem de `make afn`.

## A construção: cinco casos

A construção tem um caso por tipo de nó da árvore, e a recursão faz a
composição. O Pinglu não tem coringa (`docs/02_nucleo_minimo.md`), então a
árvore tem cinco tipos de nó e a construção, cinco casos, sem rótulo especial
para "qualquer símbolo". É uma consequência direta da decisão do módulo 02:
`a.c` é o ponto literal, e a máquina dele recusa `abc`.

Todo caso devolve um **bloco**: exatamente uma entrada e uma saída, sem seta
chegando à entrada nem saindo da saída quando o bloco nasce. É essa interface
uniforme que deixa compor blocos sem olhar dentro deles.

| Tipo de nó   | Estados novos | Transições novas | Das quais vazias |
| ------------ | ------------: | ---------------: | ---------------: |
| símbolo      |             2 |                1 |                0 |
| cadeia vazia |             2 |                1 |                1 |
| concatenação |             0 |                1 |                1 |
| alternância  |             2 |                4 |                4 |
| fecho        |             2 |                4 |                4 |

A concatenação liga os dois blocos por uma transição vazia e **não funde**
estados: fundir funcionaria num caso isolado e destruiria a propriedade das
pontas. Em `a*b*`, três fusões que preservam a linguagem uma a uma, juntas,
fazem a máquina aceitar `ba` (conta feita no material do módulo, seção 1.2.2).

Das cinco regras, a do fecho é a que tem mais onde errar: são quatro
transições vazias, cada uma com um papel (uma vez ou mais, zero vezes, de novo,
basta). A demonstração prova que cada uma é necessária (ver "Defeitos
injetados").

## Decisões que seguem dos módulos anteriores

- **O rótulo da transição vazia é um `int` (`-1`), e não o byte zero.** Os
  autômatos de texto do Pinglu aceitam qualquer byte, inclusive o zero
  (`especificacao/alfabeto.md`). Com o byte zero como marca de vazio, uma
  entrada com `\0` casaria com transições vazias e `a*` aceitaria `"\0"`. Os
  rótulos de símbolo são os 256 bytes, convertidos por `unsigned char`: sem a
  conversão, um byte acima de 0x7F (o `ã` de UTF-8) viraria um rótulo negativo.
  O passo inicial do traço também tem marca própria (`ehPartida`), e não um
  símbolo-sentinela, pelo mesmo motivo.
- **Montar lança, simular nunca lança.** Estado inexistente, rótulo fora de
  `-1..255` e árvore malformada são defeito do programador e lançam
  `std::invalid_argument`. Qualquer cadeia entregue à simulação tem resposta:
  aceita ou recusa. É a mesma regra do módulo 03.
- **A árvore vazia é a linguagem `{ε}`**, uma máquina de um estado que aceita
  só a cadeia vazia, e não a linguagem vazia. (O leitor do módulo 02 recusa o
  pattern vazio; o caso existe para quem monta uma `Arvore` à mão.)

## A simulação

O que se mantém é um **conjunto** de estados ativos, e a aceitação é por
**existência** de percurso: basta o estado de aceitação estar entre os ativos
ao fim.

- **O conjunto de partida é o fecho vazio do estado inicial**, e não o estado
  inicial sozinho. Esquecer isso passa em todo pattern que começa por símbolo e
  só falha quando a máquina tem transição vazia logo na entrada, isto é, em
  todo pattern que começa por alternância ou fecho.
- **O fecho vazio marca os estados visitados.** Em `(a?)*` e `(a*)*` o
  interior do fecho se atravessa sem ler símbolo, o ciclo é todo de transições
  vazias, e sem a marca a travessia nunca termina.
- **Representação do conjunto:** um vetor de estados (guarda a ordem de
  inserção, é o que se percorre) mais um vetor de marcas, um byte por estado, que
  responde "este estado já entrou?" com uma consulta. Um `std::set` alocaria um
  nó por inserção e pagaria uma busca logarítmica por consulta. É decisão por
  raciocínio: **não medi tempo**, e o documento não afirma quanto se ganha.
- **O custo por símbolo não é proporcional só ao conjunto ativo:** o fecho vazio
  aloca e zera um vetor `visitado` do tamanho da máquina a cada chamada. Para as
  máquinas deste módulo é irrelevante, e a simulação não é o que o lexer vai
  executar (será a máquina determinística, módulo 05).

## O tamanho que a construção produz

Para `(a|b)*c`, a tabela de custos dá 10 estados, 12 transições e 9 vazias sem
construir nada (3 folhas, 1 alternância, 1 fecho, 1 concatenação); a
demonstração confere a conta e o traço de `abc` contra os conjuntos calculados
no papel.

Para as classes léxicas do Pinglu (saída da seção D de `make afn`):

| Classe                     | Nós da árvore | Estados do AFN | Transições (vazias) | Estados da máquina feita à mão (módulo 03) |
| -------------------------- | ------------: | -------------: | ------------------: | -----------------------------------------: |
| identificador              |           232 |            462 |           577 (461) |                                          2 |
| numero                     |            85 |            162 |           202 (161) |                                          4 |
| operadores_e_delimitadores |            61 |            108 |            130 (99) |                                          6 |

A máquina de Thompson é de 18 a 231 vezes maior que a feita à mão (462 contra 2,
162 contra 4, 108 contra 6), e quase toda transição é vazia. É esse o custo de construir sem decidir nada, e é o que a
determinização e a minimização (módulo 05) vão cobrar de volta. A classe do
identificador pesa 232 nós porque `[a-zA-Z_]` vira 53 folhas e 52 alternâncias,
e `[a-zA-Z0-9_]` vira 63 folhas e 62 alternâncias: classes de símbolos largas
custam caro na árvore.

Os patterns de `cadeia_de_caracteres`, `caractere` e `comentario_de_linha` não
entram na tabela: "qualquer byte menos X" não tem notação no núcleo mínimo e
exigiria uma alternância de 255 símbolos escrita por extenso.

## A bateria conferida à mão

`exemplos/07_casos_afn.txt` tem 104 casos (50 de aceitação, 54 de recusa), um por
linha: padrão, cadeia e veredito, separados por tabulação. É um arquivo de dados
para que acrescentar um caso seja barato: um caso que exige recompilar não é
escrito na tarde em que alguém descobre a cadeia que quebra o sistema.

**Cada veredito foi determinado antes de existir o código**, por raciocínio
sobre a linguagem do padrão, e nenhum foi alterado depois de rodar. É isso que
dá valor à bateria: um caso escrito depois de ver a saída do programa congela o
comportamento atual, defeitos inclusive. Para conferir, escolha um caso e refaça
o veredito sem olhar o código; se não conseguir dizer por que a cadeia é aceita
ou recusada, o caso foi copiado e não conferido.

Os grupos cobrem cada decisão tomada até aqui: os casos base, os três
operadores, o prefixo comum (`ab|ac`, o caso que o determinismo não resolve sem
abrir as máquinas), o fecho com interior vazio (o ciclo só de transições
vazias), as reduções de `+` e `?`, o ponto literal, a barra invertida, as
classes, e quatro grupos que são classes léxicas do próprio Pinglu
(identificador, número sem sinal, operadores de dois símbolos, cadeia
simplificada).

**O leitor é estrito.** Uma linha malformada, um veredito que não é `aceita` nem
`recusa`, um campo a mais ou a menos e um arquivo que não abre são erros que
reprovam a bateria. Tratar o que não é `aceita` como recusa faria um erro de
digitação virar um caso de recusa que passa sem verificar nada. Uma bateria
vazia também reprova.

A bateria roda inteira, sem parar no primeiro caso que falha, e a mensagem
traz arquivo e linha, o veredito esperado e o obtido.

## O que a demonstração confere além da bateria

| Seção | O que confere                                                                                                                                                             | Por que a bateria sozinha não basta                              |
| ----- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------- |
| A     | 4 construções com o traço, comparadas com contagens e conjuntos calculados no papel                                                                                       | a bateria só vê aceita/recusa, não o conjunto ativo              |
| B     | para todos os patterns da bateria: estados, transições e vazias iguais à tabela de custos, abaixo do teto de 2 estados e 4 transições por nó, nenhum estado sem chegada   | uma ligação esquecida pode não mudar nenhuma resposta da bateria |
| C     | byte zero na entrada e no pattern, bytes acima de 0x7F, árvore vazia, montagem inválida, árvore malformada, leitor da bateria                                             | a bateria é de texto e não carrega esses casos                   |
| D     | a máquina feita à mão (módulo 03) e a derivada do pattern concordam em todas as cadeias sobre um alfabeto de amostra: 3906, 5461 e 8421 cadeias, de até 5, 6 e 3 símbolos | são duas descrições independentes da mesma linguagem             |

A seção D **não é prova de equivalência** (a prova é a determinização com
a minimização, módulo 05): é concordância em milhares de cadeias, inclusive nas
recusadas, entre duas máquinas escritas por caminhos diferentes.

## Defeitos injetados

Para saber se os testes realmente falham quando devem, 16 defeitos foram
injetados, um por vez, em cópias do módulo. Todos foram acusados, por
seções diferentes da demonstração:

| Defeito injetado                                 | Acusado em                                             |
| ------------------------------------------------ | ------------------------------------------------------ |
| fecho sem "zero vezes"                           | A, B, C, D, E                                          |
| fecho sem "de novo"                              | A, B, D, E                                             |
| fecho sem "basta"                                | A, B, D, E                                             |
| fecho sem "uma vez ou mais"                      | A, B, D, E                                             |
| alternância sem a seta para o filho direito      | A, B, D, E                                             |
| concatenação sem a transição vazia de ligação    | A, B, C, D, E                                          |
| simulação sem o fecho vazio inicial              | A, C, D, E                                             |
| simulação sem o fecho vazio depois de cada passo | A, C, D, E                                             |
| fecho vazio sem marca de visitado                | E (a bateria esgota a memória no primeiro ciclo vazio) |
| marca `presente` nunca zerada                    | A, D, E                                                |
| transição vazia volta a ser o byte zero          | C                                                      |
| folha de cadeia vazia vira símbolo               | B, D, E                                                |
| rótulo de símbolo sem `unsigned`                 | C (exceção no teste de bytes acima de 0x7F)            |
| leitor trata veredito desconhecido como `aceita` | C                                                      |
| leitor ignora linha malformada em silêncio       | C                                                      |
| aceitação deixa de ser consultada                | A, C, D, E                                             |

Foi feito com um script descartável, fora do repositório; a tabela é o registro.
Dois resultados valem nota: o byte zero como vazio só é acusado pela seção C (a
bateria, por ser de texto, não o alcança), e o fecho vazio sem marca só é
acusado pela bateria, que é o argumento para os patterns com ciclo vazio
estarem nela.

## Limites e o que fica em aberto

- **`construir` é recursiva.** A profundidade acompanha a altura da árvore, e
  `n` símbolos em sequência dão altura `n - 1`. Medido (Linux, pilha de 8 MB, sem
  otimização): a leitura do módulo 02 aguenta 1.000.000 de símbolos, a construção
  funciona até 40.000 e estoura a pilha em 50.000. O limite depende da
  plataforma e da otimização. Os três patterns de classes léxicas medidos têm no máximo
  232 nós; um pattern gerado por outro programa poderia passar do limite, e então a
  construção teria de virar uma repetição com pilha própria.
- **O fecho vazio aloca `visitado` por chamada** (ver "A simulação"): fica para o
  módulo 05, quando a simulação deixa de ser o caminho de execução.
- **Estado de erro explícito** não existe no AFN: um símbolo sem transição tira o
  percurso do conjunto, sem aviso. É a definição, mas muda o diagnóstico: o traço
  mostra o conjunto esvaziando, não "caiu em tal estado".

## Observação para o diário da construção

A Construção de Thompson passo da definição apenas com algumas alterações em comparação com
o código de base do projeto do professor. Cada tipo de nó da linguagem virou um `case`
em `exemplos/07_casos_afn.txt`, ao todo são 104 casos da bateria de teste e todos eles
passaram, fora isso não teve nenhuma outra mudança de decisão em comparação com o projeto
do professor.
