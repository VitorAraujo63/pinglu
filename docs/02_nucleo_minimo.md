# O núcleo mínimo de operadores e as reduções (Pinglu)

Decisão do Módulo 2 da disciplina (Expressões regulares e linguagens
regulares). O critério de inclusão no núcleo é um só: **a impossibilidade
de reduzir**. Um operador que se exprime pela composição de outros é
conveniência de quem escreve o padrão, não capacidade nova do sistema.

Este núcleo é o vocabulário que vai descrever os padrões usados para
reconhecer as classes léxicas do Pinglu (`especificacao/classes_lexicas.md`)
— identificadores, números, palavras-chave, operadores — quando o
analisador léxico existir.

## O núcleo

Três operadores e duas folhas.

| Construção   | Papel                                              |
| ------------ | --------------------------------------------------- |
| concatenação | núcleo — uma coisa seguida de outra                  |
| alternância  | núcleo — uma coisa ou outra                          |
| fecho        | núcleo — zero ou mais repetições                     |
| símbolo      | folha — um símbolo literal do alfabeto (`alfabeto.md`) |
| cadeia vazia | folha — produzida pela redução do opcional            |

Tudo o mais que alguém escrever num padrão é reduzido a isto antes de
qualquer processamento.

## As reduções, em pares

| O padrão escrito | O que ele significa, reduzido ao núcleo |
| ------------------ | ----------------------------------------- |
| `x+`                | `concat(x, fecho(x))`                     |
| `x?`                | `alt(x, vazio)`                           |
| `[abc]`             | `alt(alt('a', 'b'), 'c')`                 |
| `[a-c]`             | `alt(alt('a', 'b'), 'c')`                 |
| `(x)`               | `x` — o grupo não sobrevive à leitura     |

O **grupo** merece nota à parte. Parênteses existem para quem escreve o
padrão marcar onde a precedência muda; uma vez que a estrutura está
construída, ela **é** a precedência, e um nó de agrupamento não teria o
que guardar. O padrão `(ab)c` e o padrão `abc` significam a mesma coisa,
e é assim que deve ser.

A redução de `x+` **duplica** o que `x` representa — ele aparece uma vez
direto e outra vez sob o fecho. Isso importa desde já, antes mesmo de
existir código: quando a construção de um reconhecedor a partir desse
padrão existir (etapa futura), compartilhar a mesma referência para `x`
nos dois lugares em vez de tratá-los como duas ocorrências independentes
produziria um resultado incorreto — o reconhecedor passaria duas vezes
pelo mesmo ponto da estrutura. O custo de `x+`, portanto, é o dobro do
custo de `x`, mais o que os operadores de concatenação e fecho acrescentam.

## O que ficou de fora

**Coringa (`.`)** — o Pinglu não precisa dele hoje. Nenhuma classe léxica
registrada em `especificacao/classes_lexicas.md` exige "qualquer símbolo do
alfabeto"; identificadores, números e palavras-chave são todos descritos
por classes de caracteres explícitas (letras, dígitos) ou símbolos
literais. Diferente do projeto de referência do professor — que precisa
do coringa para o pattern de e-mail do exemplo dele —, aqui não há, por
enquanto, um caso de uso real que force essa decisão. Fica de fora até que
um exemplo concreto (por exemplo, o conteúdo de uma string literal, que
pode conter qualquer caractere exceto aspas) exija resolver isso — e
nesse momento a decisão entra aqui, com a razão ao lado, não antes.

**Quantificador contado** (`x{3,5}`). É redutível — expande em
concatenações e opcionais —, então caberia pelo critério. Ficou de fora
porque nenhuma classe léxica do Pinglu precisa dele: identificador,
inteiro, decimal e os operadores da linguagem se descrevem inteiramente
com `+`, `?`, `*` e classe de símbolos. Se voltar a ser necessário, volta
como redução, nunca como operador de núcleo.

**Retrovisor (*backreference*) e grupo de captura.** Retrovisor sai da
classe das linguagens regulares — um padrão que o usasse não poderia ser
reconhecido por um autômato finito, o que contradiz a razão de ser desta
etapa da disciplina. Grupo de captura não tem uso no Pinglu: o
reconhecedor de símbolos só precisa dizer que tipo de token foi casado
(identificador, número, palavra-chave), não recuperar uma subcadeia
específica de dentro do casamento.

## Onde é fácil errar

Manter um operador no núcleo "porque é fácil de implementar agora". A
conta certa é a de quantos lugares futuros vão precisar tratar aquele
operador como um caso a mais — na leitura, e depois em qualquer peça que
processe o resultado dessa leitura.

**Como verificar que está correta:** para cada operador do núcleo, tente
escrever a redução dele usando os outros. Se conseguir, ele não pertence
ao núcleo — a menos que exista, como poderia acontecer com o coringa mais
adiante, uma razão de tamanho ou de ausência de caso de uso escrita ao
lado.
