# O recorte do Pinglu — decisões fixadas no primeiro módulo

Registro das decisões que a Tarefa 1 pede (Módulo 1 — Linguagens formais e a
arquitetura de um compilador), na forma em que ficam travadas para o resto
do percurso. Cada uma vem com a alternativa descartada, porque é a
comparação que torna a decisão revisitável depois.

Este documento **não substitui** `docs/descricao_linguagem.md` — ele
resolve as contradições que ficaram nas respostas daquele questionário e
fecha o que ficou em aberto, na forma de decisão registrada. `alfabeto.md`
e `classes_lexicas.md` continuam sendo a fonte do vocabulário.

## Que domínio e escopo o Pinglu cobre

**Decisão:** linguagem de propósito geral, multiparadigma, com tipagem
estática e **sem inferência de tipos** — todo valor é declarado com seu
tipo explícito. As funcionalidades mínimas da v0.1, confirmadas em
`descricao_linguagem.md`, são: variáveis, operadores, `if`/`else`,
`while`, `for`, `print` e funções.

**Pendência registrada, não bloqueante:** `classes_lexicas.md` ainda não
tem uma palavra-chave para declaração de função (nem para `print`, ver
abaixo), e a sintaxe de `for` e `pineach` ainda não foi escrita por
extenso — só o nome da palavra-chave existe. Isso não bloqueia o resto do
recorte porque nenhuma peça já construída depende dessas sintaxes; fica
marcado em `docs/01_gramatica.txt` como produção em aberto, para não
sumir da vista até ser decidido.

**Descartado:** orientação a objetos e tratamento de erros na v0.1 — já
fixado em `descricao_linguagem.md` e mantido aqui sem mudança.

## Que forma tem o código escrito por quem usa o Pinglu

**Decisão:** blocos delimitados por `{ }`, `;` obrigatório ao fim de cada
instrução, atribuição com `=`, tipo explícito antes do nome da variável
(`int x = 5;`). É a forma que `especificacao/classes_lexicas.md` já
registra — `'=' --Palavra-Chave para a atribuição de valores, ex: "int
exemplo = 3"` — e que passa a valer para todo o projeto a partir daqui.

**Descartado:** a sintaxe do `docs/example.pglu` original (`pinglu:
n1 == 2;`, `pinglu return: n1 + n2;`). Era um rascunho anterior às
classes léxicas e não é compatível com elas: não usa `=` para atribuição,
não usa chaves para delimitar blocos, e não expõe tipo explícito na
declaração. Foi substituído pelo exemplo da Tarefa 2 (ver
`docs/01_exemplo_validado.md`); fica citado aqui só para registrar que a
divergência foi percebida e resolvida, não ignorada.

**Consequência prática:** `print` precisou ser incluído como palavra-chave
em `especificacao/classes_lexicas.md` para que o exemplo da Tarefa 2
pudesse ser escrito — ele já estava no escopo funcional (v0.1 lista
`print`), mas não tinha entrada na tabela de classes léxicas. Mesmo caso
do retrovisor `\.` no material do professor: quando o exemplo cobra uma
construção que a especificação não previa, quem cede é a especificação.

## O que o sistema produz — modelo de execução

**Decisão:** o compilador do Pinglu traduz a AST para **bytecode**, e uma
máquina virtual de pilha executa esse bytecode. É o modelo de "Compilador
para bytecode" que já estava na primeira resposta de
`descricao_linguagem.md`.

**Descartado:** interpretação direta da AST (percorrer a árvore e
executar cada nó na hora, sem fase de geração de código). Essa era a
segunda resposta do questionário original, e as duas eram
incompatíveis entre si — não dá pra ser as duas coisas ao mesmo tempo.
Fica descartada explicitamente aqui para que a próxima pessoa que ler
`descricao_linguagem.md` não tropece na mesma contradição.

**O que essa decisão implica para módulos futuros:** existe uma fase de
geração de código separada da execução (compila-se uma vez, executa-se
depois), e o objeto que passa de uma fase para outra é bytecode de
máquina de pilha — não a árvore. A tabela de símbolos, a verificação de
tipos e o controle manual de memória (já fixado em
`descricao_linguagem.md`) precisam existir **antes** dessa fase, porque a
geração de bytecode consome a árvore já verificada.

## As três pendências fechadas: `funcDecl`, `forStmt`, `pineachStmt`

**Decisão — função:** `func ID ( params? ) bloco`, sem tipo de retorno na
assinatura (`func soma(int a, int b) { return a + b; }`). `return` é uma
instrução comum, não parte da declaração — coerente com a resposta já
fixada em `descricao_linguagem.md` ("Funções precisarão declarar o tipo de
retorno? R: Não").

**Decisão — `for`:** estilo C — `for ( inicialização; condição; passo )
stmt`, ex. `for (int i = 0; i != 10; i = i + 1) { print(i); }`. Descartado
o estilo "for x in faixa" citado como alternativa em
`docs/01_gramatica.txt`: esse estilo é redundante com o `pineach`, que já
cobre "para cada elemento de uma lista" — manter os dois faria a linguagem
ter duas sintaxes para a mesma ideia, sem ganho.

**Decisão — `pineach`:** `pineach ( ID in expr ) stmt`, ex. `pineach (item
in lista) { print(item); }`. Precisou da palavra-chave nova `in`, que não
estava em `classes_lexicas.md`. O nome de iteração segue a regra de escopo
léxico (abaixo): vale só dentro do corpo do `pineach`.

## Lacuna descoberta ao escrever o exemplo do `pineach`: faltava literal de lista

`list` já existia como palavra-chave de tipo desde a v0.1, mas nenhuma
sintaxe para escrever um **valor** de lista tinha sido decidida — o mesmo
tipo de lacuna que `docs/01_exemplo_validado.md` já tinha documentado para
os operadores relacionais. Sem literal de lista, `pineach` não tem o que
percorrer em nenhum exemplo concreto.

**Decisão:** `"[" argList? "]"`, ex. `list numeros = [10, 20, 30];`. Os
símbolos `[` e `]` já estavam em `especificacao/alfabeto.md` sem uso
atribuído — a decisão só fecha um símbolo que já fazia parte do alfabeto
declarado, não introduz nada fora dele.

**Descartado:** inventar uma sintaxe maior para listas agora (indexação,
`list<int>` tipado por elemento, etc.). Nenhum caso de uso atual — só o
`pineach` — exige mais do que "declarar uma lista e percorrer os
elementos"; o mesmo critério de `docs/02_nucleo_minimo.md` para o coringa
("fica de fora até que um exemplo concreto exija").

## Escopo léxico e vinculação de nomes

**Decisão:** o Pinglu tem escopo **léxico de bloco**. Cada `{ }` abre um
escopo novo; um nome declarado dentro de um bloco só é visível dentro dele e
dos blocos aninhados dentro dele — nunca fora. Declarações no nível do
`program` (fora de qualquer função ou bloco) são **globais**, visíveis em
todo o arquivo. Parâmetros de função e nomes ligados por uma cláusula de
`escolher` (ver `docs/04_escolher_e_formas.md`) seguem a mesma regra: valem
só dentro do corpo que os liga.

```pinglu
int contador = 0;          // global — visivel em todo o arquivo

if (contador == 0) {
    int mensagem = 1;      // local a este bloco
    print(mensagem);
}
// mensagem nao existe aqui: o bloco do if fechou o escopo dela

escolher f {
    circulo(r)      -> print(r);   // r só existe dentro desta clausula
    retangulo(b, h) -> print(b);   // r nao existe aqui, b e h nao existem na clausula do circulo
}
```

**Descartado:** escopo indefinido ("não terá muito bem definido"), a
resposta original de `descricao_linguagem.md`. Era sustentável enquanto
nada na linguagem dependia de responder "esse nome existe aqui?" de forma
precisa. Deixou de ser: o `escolher` liga um nome (`r`, `b`, `h`) a partir de
casamento de padrão, e a análise semântica **precisa** saber, para cada uso
de um nome, se ele foi ligado por alguma declaração ou cláusula que o
alcança — sem uma regra de escopo fechada não há como decidir isso, só
adivinhar.

**Consequência prática:** a tabela de símbolos (já prevista em
`descricao_linguagem.md` como necessária para "variáveis não utilizadas")
passa a ser uma pilha de escopos, um por bloco/função/cláusula aberta, não
uma tabela única — do jeito padrão para escopo léxico.

## Como um erro é relatado

**Decisão:** todo token carrega sua posição — linha e coluna — desde que é
reconhecido pelo analisador léxico, e essa posição é propagada para cada nó
da árvore que aquele token ajuda a formar. Toda mensagem de erro (léxico,
sintático ou semântico — incluindo a recusa por caso não tratado do
`escolher`) cita pelo menos a linha; a coluna fica disponível internamente
para quando for útil imprimir. Formato mínimo: `linha L: mensagem`, ex.
`linha 12: o caso 'triangulo' não é tratado`.

**Descartado:** "de forma básica... a princípio não terá linha e coluna,
será só uma mensagem", a resposta original de `descricao_linguagem.md`. Uma
recusa sem posição obriga quem programa em Pinglu a reler o arquivo inteiro
procurando o caso que o compilador recusou; com um arquivo de poucas linhas
isso é irritante, com um arquivo grande é impraticável — e a proposta do
`escolher` (seção 4 do feedback recebido) só cumpre o que promete, recusar
programas incompletos, se disser **onde**.

**Consequência prática:** a posição precisa ser carregada desde o primeiro
token, não acrescentada depois. Guardar (linha, coluna) já na struct/token
do analisador léxico custa uma tupla a mais por token; tentar reconstruir a
posição depois, a partir de uma árvore que não guardou essa informação, exige
percorrer o código-fonte de novo comparando strings — mais caro e mais
frágil.

## O comando único

**Decisão:** existe, desde já, um comando único que reconstrói o que houver
para reconstruir e roda todos os casos de `exemplos/` contra o resultado
esperado registrado ao lado de cada um — mesmo enquanto "o que houver para
reconstruir" for quase nada. Ver `scripts/verificar.sh` (ou `make verificar`)
e `docs/01_organizacao_repositorio.md` para o que o comando faz hoje.

**Descartado:** esperar a primeira fase do compilador existir para
instalar o comando. O argumento contra isso já está em
`docs/01_organizacao_repositorio.md` ("Adiar a decisão de organização até
'ter algo para organizar'") — o mesmo raciocínio vale aqui: o comando que
falta ao **não** existir hoje é o mesmo que vai faltar quando o primeiro
teste real for escrito, só que aí custará mais para notar que falta.

**Consequência prática:** hoje o comando não compila nada — ele confere que
todo `.pglu` em `exemplos/` usa só palavras-chave e operadores que constam
em `especificacao/classes_lexicas.md`, exatamente a checagem manual que
`docs/01_exemplo_validado.md` já descrevia em prosa ("Como verificar que
está correta"). Automatizar essa checagem manual é o que "custa cinco
minutos" hoje e evita a regressão silenciosa depois.

## Onde é fácil errar

Deixar uma contradição como a do modelo de execução sem registrar a
escolha, na esperança de que "dá pra decidir depois, no código". O
código não vai reclamar sozinho — ele vai simplesmente implementar
qualquer uma das duas leituras, e a outra metade do grupo só vai
descobrir a divergência quando duas partes do compilador não
encaixarem.

**Como verificar que está correta:** releia `descricao_linguagem.md`
depois deste documento. Cada resposta de lá deveria continuar valendo, ou
apontar para a entrada daqui que a substituiu. Se sobrar uma resposta que
nenhuma decisão registrada aqui cobre, ela ainda está em aberto — e
precisa entrar nesta lista, não ficar solta no questionário.
