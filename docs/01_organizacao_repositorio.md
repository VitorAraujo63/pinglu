# A organização do repositório de trabalho (Tarefa 3)

O que a tarefa pede é montar o repositório separando o que se lê do que
se compila, com um jeito único de reconstruir e rodar tudo assim que
houver algo para compilar. O Pinglu ainda está na fase de documentação —
nenhum analisador léxico, parser ou VM existe ainda —, mas o comando único
já existe (`make verificar`, ver abaixo): o feedback recebido apontou que
"instalá-lo agora custa cinco minutos", e adiar isso até existir código de
verdade era exatamente o erro que a seção "Onde é fácil errar" deste
documento já alertava contra.

## O que já existe, e o papel de cada parte

```
pinglu/
├── README.md                          — apresentação e integrantes
├── Makefile                           — o comando único: "make verificar"
├── especificacao/
│   ├── alfabeto.md                     — o alfabeto Σ da linguagem
│   └── classes_lexicas.md              — palavras-chave, operadores, o vocabulário
├── exemplos/                           — pares entrada/saída esperada, um .pglu por caso
│   ├── 01_aritmetica_condicional.pglu   — o exemplo original da Tarefa 2 (migrado de docs/)
│   ├── 02_for_e_relacionais.pglu        — for estilo C, operadores relacionais
│   ├── 03_pineach.pglu                  — pineach e literal de lista
│   ├── 04_funcao.pglu                   — func/return
│   ├── 05_escolher_completo.pglu        — forma/escolher exaustivo (aceito)
│   └── 06_escolher_incompleto.pglu      — forma/escolher incompleto (deve ser recusado)
├── scripts/
│   └── verificar.py                    — o que "make verificar" roda; hoje confere
│                                          consistência léxica dos exemplos contra
│                                          especificacao/classes_lexicas.md
└── docs/
    ├── descricao_linguagem.md          — questionário de escopo original, com notas
    │                                      apontando as respostas substituídas
    ├── 01_recorte.md                   — decisões da Tarefa 1, com alternativa descartada
    ├── 01_gramatica.txt                — gramática (pendências fechadas; não fatorada ainda)
    ├── 01_exemplo_validado.md          — comentário sobre o primeiro exemplo e lacunas encontradas
    ├── 02_nucleo_minimo.md             — núcleo de regex e reduções (Tarefa 2 do módulo 2)
    ├── 03_especificacao_sete_secoes.md — a especificação formal (Tarefa 1 do módulo 2)
    ├── 04_escolher_e_formas.md         — design do escolher/forma (emenda do feedback)
    ├── 05_guia_da_atividade.md         — tradução da Tarefa 3 do módulo 2 (padrões/árvores) em passos
    └── 05_padroes_lexicos_e_arvores.md — a entrega: padrões das classes léxicas reduzidos e contados
```

`especificacao/` guarda o vocabulário — o que existe independente de como
o compilador vai ser escrito. `exemplos/` guarda os pares entrada/saída
esperada — a mesma pasta que este documento previa antes de existir mais
de um caso; migrou assim que o segundo caso apareceu, exatamente como
planejado abaixo. `scripts/` guarda o que hoje é o único código do
projeto — não o compilador em si, mas o verificador que faz as vezes dele
enquanto ele não existe. `docs/` guarda as decisões de projeto. Essa
separação é a mesma que o material do professor recomenda: o que se lê
fica isolado do que se compila, para que trocar a implementação nunca
signifique reescrever a especificação junto.

## O que falta acrescentar quando o código do compilador começar

Quando a primeira peça do compilador for escrita (o analisador léxico, a
próxima etapa natural depois deste módulo), a organização recomendada é:

```
pinglu/
├── especificacao/      (como já está)
├── docs/                (como já está)
├── exemplos/            (como já está)
├── scripts/
│   └── verificar.py     — cresce para chamar o lexer/parser/VM de verdade
│                           em vez da checagem léxica manual de hoje
├── src/                 — o código do compilador, futuro
│   └── pinglu/           — o pacote Python (nome já reservado)
└── tests/                — os testes automatizados, futuro
```

Duas decisões continuam valendo, agora com o comando único já em uso:

**Um arquivo por assunto**, não um arquivo único com tudo — o mesmo
princípio do material do professor (`01_linguagem.h`/`.cpp` separado de
`01_pipeline.h`/`.cpp`). Cada fase do compilador (análise léxica, análise
sintática, análise semântica, geração de bytecode, máquina virtual) ganha
seu próprio módulo, para que trocar a representação de uma fase não
obrigue a tocar nas outras.

**Um comando único para rodar tudo.** Já existe (`make verificar`) e já
roda algo real, mesmo que pouco: a checagem de consistência léxica dos seis
exemplos em `exemplos/`. Quando o lexer existir, o alvo `verificar` do
`Makefile` passa a chamá-lo em vez da checagem manual — o comando
continua o mesmo, só o que ele faz por baixo cresce. Isso é o que evita a
regressão silenciosa que o feedback apontou como risco de adiar.

## Onde é fácil errar

Adiar a decisão de organização até "ter algo para organizar" — e, depois
do feedback, adiar o comando único até "ter algo de verdade para rodar".
As duas armadilhas são a mesma: o código (ou o comando) escrito sobre uma
estrutura já pensada custa muito menos para crescer do que o que é
inventado depois, quando módulos futuros já dependem de onde as coisas
estão hoje.

**Como verificar que está correta:** rode `make verificar` — deve terminar
com código de saída 0 e "0 problema(s) encontrado(s)" contra os seis
exemplos atuais. Quando a primeira peça de código do compilador for
escrita, confira que ela cabe num dos diretórios já previstos aqui sem
precisar inventar um novo, e que `scripts/verificar.py` foi atualizado
para chamá-la em vez de só checar léxico à mão.
