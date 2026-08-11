# Perguntas de Planejamento - Escopo do Projeto de Compiladores

**1. Propósito e Escopo Inicial**
* Qual é o objetivo principal da linguagem e qual problema ela pretende resolver?
<br> R: A linguagem é focada na segurança e na facilidade de implementação de programas.
* A linguagem será de propósito geral ou específica para um domínio?
<br> R: Propósito Geral
* Quais funcionalidades mínimas estarão presentes na primeira versão (Versão 0.1) para considerar a linguagem estável (ex: variáveis, tipos básicos, operadores, if/else, while, print)?
<br> R: Variaveis, funçoes, operadores, if/else, while e print
* O que será deixado de fora do escopo para versões futuras?
<br> R: Orientação a objetos, tratamento de erros, etc.

**2. Paradigma e Sintaxe**
* Qual será o paradigma principal da linguagem (Procedural, Orientado a objetos, Funcional, Declarativo, Multiparadigma)?
<br> R: Multiparadigma
* Como serão delimitados os blocos de código (ex: utilizando `{}` ou indentação obrigatória) e será necessário utilizar `;`?
<br> R: { }, sim
* A linguagem terá tipagem estática ou dinâmica?
<br> R: Estática
* Haverá inferência de tipos ou será possível declarar explicitamente o tipo?
<br> R: Não haverá inferência de tipos

**3. Arquitetura do Compilador/Interpretador**
* Como o código será executado (Interpretador, Compilador para código nativo, Máquina virtual/Bytecode, ou Transpilação para outra linguagem)?
<br> R: Compilador para bytecode
* Qual será o modelo da gramática no Parser (LL, LR, etc.) e será utilizado algum gerador de parser?
<br> R: LL e algum parser
* A AST (Abstract Syntax Tree) será convertida para bytecode ou haverá interpretação direta da AST?
<br> R: AST direto
* Como a memória será gerenciada (Haverá Garbage Collector ou o programador controlará manualmente a memória)?
<br> R: Controle manual

**4. Regras Semânticas e Tratamento de Erros**
* Quais erros serão detectados em tempo de compilação (ex: verificar tipos, variáveis não utilizadas, quantidade incorreta de parâmetros)?
<br> R: Verificar tipos, variáveis não utilizadas, quantidade incorreta de parâmetros
* Como erros léxicos e de sintaxe serão apresentados? As mensagens indicarão linha e coluna?
<br> R: De forma basica (erro de sintaxe), a principio nao tera linha e coluna será so uma mensagem
* Qual será o escopo das variáveis e existirão variáveis globais?
<br> R: variaveis globais e locais (nao tera muito bem definido o escopo)
* Funções precisarão declarar o tipo de retorno?
<br> R: Não

**5. Identidade da Linguagem**
* Qual será o nome da linguagem e qual será sua extensão de arquivo?
<br> R: Pinglu, .pglu
* Quais ferramentas acompanharão a linguagem na entrega do projeto (CLI, REPL, Debugger, Formatter)?
<br> R: CLI
