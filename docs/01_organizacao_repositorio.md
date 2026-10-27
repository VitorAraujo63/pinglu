# A organização do repositório de trabalho (Tarefa 3)

O que a tarefa pede é montar o repositório separando o que se lê do que
se compila, com um jeito único de reconstruir e rodar tudo. O comando
único existe desde a documentação (`make verificar`): o feedback recebido
apontou que "instalá-lo agora custa cinco minutos", e adiar isso até existir
código de verdade era exatamente o erro que a seção "Onde é fácil errar"
deste documento já alertava contra. Hoje ele já carrega código: o analisador
de expressões regulares (módulo 02), os autômatos finitos determinísticos (módulo 03)
e o autômato não determinístico com a construção de Thompson (módulo 04).
Nenhum analisador léxico, parser ou VM do Pinglu existe ainda.

## O que existe, e o papel de cada parte

```
pinglu/
├── README.md                           — apresentação, integrantes e como rodar
├── Makefile                            — o comando único: "make verificar"
├── .gitignore                          — build/: binários gerados não se versionam
├── especificacao/
│   ├── alfabeto.md                      — o alfabeto Σ da linguagem
│   └── classes_lexicas.md               — palavras reservadas, literais, símbolos,
│                                           operadores e comentários
├── exemplos/                            — pares entrada/saída esperada: um .pglu por caso, mais a
│                                           bateria conferida à mão do módulo 04
│   ├── 01_aritmetica_condicional.pglu    — o exemplo original da Tarefa 2 (migrado de docs/)
│   ├── 02_for_e_relacionais.pglu         — for estilo C, operadores relacionais
│   ├── 03_pineach.pglu                   — pineach e literal de lista
│   ├── 04_funcao.pglu                    — func/return
│   ├── 05_escolher_completo.pglu         — forma/escolher exaustivo (aceito)
│   ├── 06_escolher_incompleto.pglu       — forma/escolher incompleto (deve ser recusado)
│   └── 07_casos_afn.txt                      — 104 casos (padrão, cadeia, veredito) determinados à mão,
│                                            antes do código; o leitor reprova linha malformada
├── src/                                 — o código do compilador, um par .h/.cpp por assunto
│   ├── 02_regex_pinglu.h/.cpp            — expressão regular -> árvore reduzida ao núcleo,
│   │                                        com recusa e posição do erro
│   ├── 03_afd.h/.cpp                     — o AFD (matriz densa, função total) e as sete
│   │                                        máquinas das classes léxicas
│   └── 04_afn.h/.cpp                     — o AFN, a construção de Thompson, a simulação por
│                                            conjunto de estados e a leitura da bateria
├── demos/                               — uma demonstração por módulo, com a saída esperada
│   ├── 02_demo.cpp                       — reduções, notações equivalentes, erros com posição
│   ├── 03_demo.cpp                       — tabelas, casos aceitos e recusados, conta de memória
│   └── 04_demo.cpp                       — construção com traço, custo previsto, bordas, equivalência
│                                            com o módulo 03 e a bateria
├── scripts/
│   └── verificar.py                     — consistência léxica dos exemplos e sincronia da
│                                            lista de reservadas com classes_lexicas.md
└── docs/
    ├── descricao_linguagem.md           — questionário de escopo original, com notas
    │                                       apontando as respostas substituídas
    ├── example.pglu                     — o primeiro programa válido do Pinglu, escrito à mão
    ├── 01_recorte.md                    — decisões da Tarefa 1, com alternativa descartada
    ├── 01_gramatica.txt                 — gramática (pendências fechadas; não fatorada ainda)
    ├── 01_exemplo_validado.md           — comentário sobre o primeiro exemplo e lacunas encontradas
    ├── 02_nucleo_minimo.md              — núcleo de regex e reduções (Tarefa 2 do módulo 2)
    ├── 02_especificacao_sete_secoes.md  — a especificação formal (Tarefa 1 do módulo 2)
    ├── 02_escolher_e_formas.md          — design do escolher/forma (emenda do feedback)
    ├── 03_representacao_transicao.md    — a representação da função de transição, a conta
    │                                       em bytes e as pendências do alfabeto
    └── 04_construcao_thompson.md        — a construção, a simulação, a bateria, as medidas e os
                                            defeitos injetados
```

`especificacao/` guarda o vocabulário — o que existe independente de como
o compilador vai ser escrito. `exemplos/` guarda os pares entrada/saída
esperada. `src/` guarda o que é compilado, e `demos/` guarda o que o exercita.
`scripts/` guarda o verificador, que confere a documentação contra os exemplos.
`docs/` guarda as decisões de projeto. Essa separação é a mesma que o material
do professor recomenda: o que se lê fica isolado do que se compila, para que
trocar a implementação nunca signifique reescrever a especificação junto.

## O que `make verificar` roda

Quatro coisas, nesta ordem, e para na primeira que falhar:

1. `scripts/verificar.py` — toda palavra reservada usada em `exemplos/` tem
   entrada em `classes_lexicas.md`, e a lista de reservadas do script é
   igual à da especificação (a segunda checagem existe porque as duas já
   divergiram uma vez, sem nenhum sintoma).
2. `build/02_demo` — o analisador de expressões, com a saída esperada de cada
   caso escrita ao lado.
3. `build/03_demo` — os autômatos, com os casos aceitos e recusados de cada
   máquina e a conta de memória.
4. `build/04_demo` — a construção de Thompson e a simulação, contra contas feitas
   no papel, contra as máquinas feitas à mão do módulo 03 e contra a bateria
   conferida à mão (`exemplos/07_casos_afn.txt`).

As três demonstrações saem com código 1 se um caso divergir do esperado. É isso
que as torna testes, e não só exibição. Para rodar uma só: `make regex`,
`make afd` ou `make afn`. Para apagar o que foi gerado: `make limpar`.

Pré-requisitos: `make`, `g++` com suporte a C++20 e `python3` 3.9 ou superior
(a sintaxe `set[str]` do script exige isso). Testado com `g++` 13.3 e
`python3` 3.12; outras versões não foram testadas. Em Windows, o caminho mais
simples é WSL ou MSYS2.

## O que falta acrescentar

As próximas peças (o analisador léxico, a construção de Thompson, a
determinização) entram em `src/` e `demos/` da mesma forma. Três decisões
continuam valendo:

**Um arquivo por assunto**, não um arquivo único com tudo — o mesmo
princípio do material do professor (`01_linguagem.h`/`.cpp` separado de
`01_pipeline.h`/`.cpp`). Cada fase do compilador (análise léxica, análise
sintática, análise semântica, geração de bytecode, máquina virtual) ganha
seu próprio módulo, para que trocar a representação de uma fase não
obrigue a tocar nas outras.

**Um comando único para rodar tudo.** O comando continua o mesmo, só o que
ele faz por baixo cresce. Quando o lexer existir, `verificar.py` passa a
compará-lo com os exemplos de `exemplos/` em vez de só conferir o vocabulário.

**Toda peça nova de `src/` ganha uma demonstração em `demos/` que sai com
código 1 se divergir do esperado.** Um módulo que compila mas que nenhum comando
exercita é o caso que o analisador de expressões era até esta revisão: o defeito
entraria sem sintoma e só apareceria quando a peça seguinte tropeçasse nele.
