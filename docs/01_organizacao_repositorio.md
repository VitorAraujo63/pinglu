# A organização do repositório de trabalho (Tarefa 3)

O que a tarefa pede é montar o repositório separando o que se lê do que
se compila, com um jeito único de reconstruir e rodar tudo assim que
houver algo para compilar. Como o Pinglu ainda está na fase de
documentação — nenhum código do compilador existe ainda —, o que este
documento fixa é o **plano** de organização, para que a primeira linha de
código, quando for escrita, já nasça no lugar certo em vez de forçar uma
reorganização depois.

## O que já existe, e o papel de cada parte

```
pinglu/
├── README.md                          — apresentação e integrantes
├── especificacao/
│   ├── alfabeto.md                     — o alfabeto Σ da linguagem
│   └── classes_lexicas.md              — palavras-chave, operadores, o vocabulário
└── docs/
    ├── descricao_linguagem.md          — questionário de escopo original
    ├── 01_recorte.md                   — decisões da Tarefa 1, com alternativa descartada
    ├── 01_gramatica.txt                — gramática de partida (não fatorada)
    ├── example.pglu                    — exemplo válido da Tarefa 2
    └── 01_exemplo_validado.md          — comentário sobre o exemplo e lacunas encontradas
```

`especificacao/` guarda o vocabulário — o que existe independente de como
o compilador vai ser escrito. `docs/` guarda as decisões de projeto e os
exemplos. Essa separação é a mesma que o material do professor recomenda:
o que se lê (documentação, gramática, exemplos) fica isolado do que se
compila, para que trocar a implementação nunca signifique reescrever a
especificação junto.

## O que falta acrescentar quando o código começar

Quando a primeira peça de código for escrita (provavelmente o analisador
léxico, a próxima etapa natural depois deste módulo), a organização
recomendada é:

```
pinglu/
├── especificacao/      (como já está)
├── docs/                (como já está)
├── exemplos/            — pares entrada/saída esperada, um arquivo .pglu por caso,
│                          conforme forem sendo escritos (o example.pglu de hoje
│                          pode migrar para cá quando houver mais de um caso)
├── src/                 — o código do compilador, futuro
│   └── pinglu/           — o pacote Python (nome já reservado)
└── tests/                — os testes automatizados, futuro
```

Duas decisões valem registrar desde já, mesmo sem código ainda:

**Um arquivo por assunto**, não um arquivo único com tudo — o mesmo
princípio do material do professor (`01_linguagem.h`/`.cpp` separado de
`01_pipeline.h`/`.cpp`). Cada fase do compilador (análise léxica, análise
sintática, análise semântica, geração de bytecode, máquina virtual) ganha
seu próprio módulo, para que trocar a representação de uma fase não
obrigue a tocar nas outras.

**Um comando único para rodar tudo**, assim que houver algo a rodar. Em
Python isso é mais simples do que no C++ do material de referência — não
precisa de sistema de build separado —, mas o princípio é o mesmo: quando
a suíte de testes existir, ela precisa rodar do zero com um comando só,
sem passo manual. Isso fica registrado aqui como compromisso para quando
o primeiro teste for escrito, não como algo a fazer agora.

## Onde é fácil errar

Adiar a decisão de organização até "ter algo para organizar". O código
escrito sobre uma estrutura já pensada custa muito menos para mover do
que o código escrito solto e reorganizado depois, quando módulos futuros
já dependem de onde as coisas estão hoje.

**Como verificar que está correta:** quando a primeira peça de código for
escrita, confira que ela cabe num dos diretórios já previstos aqui sem
precisar inventar um novo. Se precisar, é sinal de que o plano ficou
incompleto e vale atualizar este documento antes de seguir.
