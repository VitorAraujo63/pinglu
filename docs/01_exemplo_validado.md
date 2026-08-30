# O exemplo válido do Pinglu, comentado (Tarefa 2)

`exemplos/01_aritmetica_condicional.pglu` é o par entrada/resultado desta tarefa: um programa
escrito à mão, sem apoio de nenhum compilador, com o que se espera que
ele produza registrado ao lado. É o primeiro caso de verificação do
projeto, e continuará valendo muito depois de existirem outros.

## O que o exemplo exercita, e por quê

- **Declaração de variável com tipo explícito e atribuição** (`int a = 5;`)
  — a forma fixada em `docs/01_recorte.md`, sem inferência de tipos.
- **Aritmética** (`+`, `-`) — os dois operadores aritméticos que
  `classes_lexicas.md` já define.
- **Comparação e lógica** (`==`, `&&`) — combinados no `if`, para que o
  `&&` precise ser avaliado de verdade e não apenas o primeiro operando.
  Ver a nota no cabeçalho do próprio arquivo sobre por que essa combinação
  importa: um interpretador que executasse o `if` sem avaliar o `&&`
  corretamente ainda passaria se testássemos só `soma == 8`, mas falharia
  visivelmente aqui.
- **`if`/`else`** — os dois ramos existem no código; a execução com estes
  valores específicos só passa pelo ramo `if` (ver abaixo).
- **`while`** — controlado por uma variável-flag (`rodando`), porque
  `classes_lexicas.md` ainda não define nenhum operador relacional além
  de `==` (não há `<`, `>` nem `!=`). Um `while` que decrementasse um
  contador até zero, do jeito mais comum, não é escrevível ainda com o
  vocabulário atual — essa é uma lacuna real, registrada abaixo, não uma
  escolha de estilo.
- **`print`** — a saída, adicionada a `classes_lexicas.md` porque este
  exemplo precisava dela (ver `docs/01_recorte.md`).

## O que o exemplo **não** cobre, de propósito

- **O ramo `else`** não é exercitado por esta entrada específica: com
  `a = 5` e `b = 3`, a condição do `if` é verdadeira, então `"par"` é
  impresso e `"impar"` nunca roda. O ramo existe no código — é sintaxe
  válida — mas não há, ainda, um segundo par entrada/saída que force o
  `else`. Vale registrar como próximo passo natural: quando o
  interpretador existir, adicionar um segundo exemplo com valores que
  tornem a condição falsa, para confirmar que o `else` de fato executa.
- **`string`, `char`, `decimal`, `float`, `list`** — declarados como
  palavras-chave em `classes_lexicas.md`, mas nenhuma operação sobre eles
  (concatenação de string, indexação de lista, etc.) está decidida ainda.
  O exemplo evita inventar essas operações — o mesmo cuidado que o
  material do professor recomenda para não decidir sintaxe grande demais
  cedo demais.
- **`for`, `pineach`, declaração de função** — sintaxe ainda pendente
  (ver `docs/01_gramatica.txt`). Não aparecem no exemplo porque ainda não
  têm forma definida.

## Lacuna descoberta ao escrever o exemplo: faltam operadores relacionais

Tentar escrever um `while` "normal" (do tipo `while (contador < limite)`)
esbarrou de imediato na ausência de `<`, `>`, `!=` em
`especificacao/classes_lexicas.md`. É exatamente o tipo de coisa que a
Tarefa 2 existe para revelar — um exemplo escrito à mão expõe o que a
especificação ainda não cobre, porque tentar usá-la de verdade é mais
exigente do que só descrevê-la. Isso fica registrado aqui como pendência
para o grupo, e não foi resolvido de forma unilateral porque adicionar
operadores à linguagem é decisão de escopo (Tarefa 1), não de exemplo
(Tarefa 2).

## Onde é fácil errar

Escrever o exemplo já pensando em como o interpretador vai processá-lo,
em vez de em como alguém que nunca viu o código o escreveria. O exemplo é
da linguagem, não do compilador.

**Como verificar que está correta:** confira que o exemplo usa cada
construção já fechada em `docs/01_recorte.md` pelo menos uma vez, que o
resultado esperado foi escrito antes de existir qualquer interpretador, e
que toda palavra-chave usada no arquivo tem entrada correspondente em
`especificacao/classes_lexicas.md`. Se alguma não tiver, ou o exemplo
está usando sintaxe que ainda não foi decidida, ou a especificação está
desatualizada — as duas coisas precisam ser corrigidas, não só uma.
