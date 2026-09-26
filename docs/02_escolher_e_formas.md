Registro da mudança pedida na seção 4 do feedback. Não é um ajuste
pontual: muda o que a linguagem verifica antes de rodar. Este documento fixa
a decisão da mesma forma que `docs/01_recorte.md` fixa as do módulo 1 —
decisão, alternativa descartada, consequência, e como verificar.

## O que motivou a mudança

O feedback aponta uma lacuna que nenhuma pergunta de
`docs/descricao_linguagem.md` tinha feito antes: o Pinglu, como estava,
verifica _forma_ (sintaxe) e _tipo_, mas não verifica **cobertura** — nada
impede escrever uma cadeia de `if`/`else if` que trata três casos de um
valor que admite quatro, e o quarto só quebra em tempo de execução. Isso
contradiz o próprio propósito declarado da linguagem em
`descricao_linguagem.md`: "focada na segurança... de programas".

A proposta do feedback: em vez de descer uma escada de condicionais, o
programa **pergunta pela forma** de um valor enumerando os casos possíveis,
e o compilador **recusa** o programa se sobrar um caso sem tratamento. Isso
exige duas peças novas que não existiam: um jeito de declarar que um valor
pode ter mais de uma forma (um tipo-soma), e o `escolher`, que casa contra
essas formas.

## Tipos-forma: declarando que um valor tem mais de uma forma

**Decisão:** um tipo-forma lista suas formas possíveis, cada uma com os
campos que carrega. Palavra-chave nova: `forma` para declarar o tipo,
seguida das variantes entre `{ }`; cada variante é um nome seguido dos
campos entre `( )`, tipados como qualquer declaração de variável.

```pinglu
forma Geometria {
    circulo(float r);
    retangulo(float b, float h);
    triangulo(float base, float altura);
}
```

Um valor desse tipo é construído nomeando a forma:

```pinglu
Geometria f = circulo(3.0);
```

**Descartado:** reaproveitar `list` ou um `int` como código de forma
("`int tipo = 0` significa círculo"). Foi descartado porque devolve
exatamente o problema que o feedback aponta — nada no compilador saberia
que `0`, `1`, `2` são as únicas formas válidas, e a verificação de
cobertura do `escolher` (abaixo) não teria o que enumerar. O tipo-forma
precisa ser uma declaração fechada, com as variantes visíveis para o
compilador, não uma convenção de quem escreve o programa.

## `escolher`: casamento de padrão exaustivo

**Decisão:** `escolher <expr> { <cláusula>* }`, uma cláusula por linha,
cada uma no formato `<forma>(<parâmetros>) -> <stmt>`. Os nomes dentro dos
parênteses são ligados aos campos daquela forma e valem só dentro do `stmt`
à direita da seta — a regra de escopo fixada em
`docs/01_recorte.md` § "Escopo léxico e vinculação de nomes".

```pinglu
Geometria f = circulo(3.0);

escolher f {
    circulo(r)       -> print(3.14 * r * r);
    retangulo(b, h)  -> print(b * h);
    triangulo(base, altura) -> print((base * altura) / 2.0);
}
```

**A verificação de cobertura.** Antes de gerar qualquer bytecode para um
`escolher`, o compilador confere: toda variante declarada em `forma
Geometria` tem uma cláusula que a trata? Se não — como no exemplo do
feedback, faltando `triangulo` — o compilador recusa a compilação inteira,
citando a linha do `escolher` e o nome da forma que ficou sem cláusula:

```
linha 12: o caso 'triangulo' não é tratado
```

Se uma cláusula estiver inteiramente coberta por outra acima dela (mesma
forma repetida), o compilador avisa que aquela cláusula nunca vai casar —
aviso, não recusa, porque o programa continua correto, só tem código morto.

**Por que isso é diferente de percorrer um grafo de estados.** Uma máquina
de estados se verifica perguntando "de todo estado, dá para alcançar onde
eu quero?" — uma pergunta sobre caminhos. Aqui não há caminho: a pergunta é
se o **conjunto de cláusulas escritas** esgota o **conjunto de variantes
declaradas** em `forma`. É uma comparação de dois conjuntos — o compilador
lê a lista de variantes de `Geometria` (do próprio `forma`, que já é
fechado, sem "variante desconhecida em tempo de execução" possível) e a
lista de formas casadas nas cláusulas, e recusa se sobrar alguma na
primeira lista que não está na segunda.

**A armadilha que o feedback nomeia, e por que o Pinglu não cai nela:**
aceitar o `escolher` incompleto e só falhar quando o caso não previsto
aparecer em tempo de execução. Isso trocaria uma recusa barata (no
`escolher` de três linhas acima, custa comparar duas listas pequenas) por
um defeito caro em campo (só aparece com o dado exato que ninguém testou) —
exatamente o oposto do propósito "segurança" que abre
`descricao_linguagem.md`.

## O que sobrevive da ideia original, sem mudança

Bytecode com máquina de pilha, tipagem estática com tipo explícito, blocos
entre chaves, controle manual de memória — nenhuma dessas decisões de
`docs/01_recorte.md` muda. `escolher` é gerado como bytecode como qualquer
outra construção: a verificação de cobertura acontece **antes**, na
verificação estática, não durante a execução — o bytecode gerado para um
`escolher` já validado é só uma sequência de comparações e desvios,
poderia ser escrito à mão como uma escada de `if`/`else`. A diferença entre
`escolher` e a escada de condicionais não está no bytecode gerado — está no
que o compilador prova antes de gerar.

## O que fica de fora, por enquanto

- **Padrões aninhados** (casar dentro de um campo que também é um
  tipo-forma) — nenhum exemplo do projeto precisa disso ainda; entra se um
  caso concreto exigir, do mesmo jeito que `docs/02_nucleo_minimo.md`
  tratou o coringa.
- **Guarda de cláusula** (`circulo(r) if r > 0.0 -> ...`) — mesma razão.
- **Formas recursivas** (uma variante que contém o próprio tipo-forma,
  como uma lista ligada) — fora do núcleo mínimo de v0.1, listado junto
  com orientação a objetos em `descricao_linguagem.md`.

## Onde é fácil errar

Tratar a verificação de cobertura como "checar se todo `if` tem `else`".
Não é: a pergunta não é sobre a forma sintática das cláusulas, é sobre se
o **conjunto de variantes de um `forma`** — que só existe porque o `forma`
é uma declaração fechada — está inteiramente coberto. Um `escolher` sobre
um tipo que não é `forma` nenhum não tem o que verificar, porque não existe
lista fechada de variantes para comparar.

**Como verificar que está correta:** para cada `escolher` do projeto,
liste as variantes do `forma` que ele casa e as formas citadas nas
cláusulas — as duas listas devem ser idênticas como conjuntos (permitindo
cláusulas repetidas, que geram aviso, não recusa). Se sobrar uma variante
declarada sem cláusula, o compilador deveria ter recusado; se isso não
aconteceu, a verificação de cobertura tem um furo.
