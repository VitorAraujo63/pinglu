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
