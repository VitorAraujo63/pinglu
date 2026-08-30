Classes Lexicas

int --Palavra-Chave representando um valor de número inteiro <br>
string --Palavra-Chave representando uma cadeia de caracteres <br>
char --Palavra-Chave representando um caractere isolado <br>
decimal --Palavra-Chave representando um valor decimal <br>
float --Palavra-Chave representando um valor flutuante(real) <br>
list --Palavra-Chave representando um Array, uma lista de valores <br>
'=' --Palavra-Chave para a atribuição de valores, ex: "int exemplo = 3" <br>
func --Palavra-Chave para declarar uma função, ex: "func soma(int a, int b) { return a + b; }" <br>
return --Palavra-Chave para devolver um valor do corpo de uma função, ex: "return a + b;" <br>


if --Palavra-Chave representando um operador condicional de ação única <br>
else --Palavra-Chave representando uma exceção caso não seja comprido a condição do operador if <br>
while --Palavra-Chave representando um operador de repetição condicional, com a repetição sendo interrompida ao comprimento da condição <br>
for --Palavra-Chave representando um operador de repetição com inicialização, condição e passo, ex: "for (int i = 0; i != 10; i = i + 1) { print(i); }" <br>
pineach --Palavra-Chave representando um operador de repetição que realiza a leitura de uma lista de valores e executa para cada elemento em lista, ex: "pineach (item in lista) { print(item); }" <br>
in --Palavra-Chave que liga a variável de iteração à lista percorrida por um pineach <br>
print --Palavra-Chave representando a saída de um valor para o usuário, ex: "print(x)" <br>
forma --Palavra-Chave que declara um tipo-soma fechado, listando suas variantes possíveis, ex: "forma Geometria { circulo(float r); }" (ver docs/04_escolher_e_formas.md) <br>
escolher --Palavra-Chave que abre um bloco de casamento de padrão sobre as formas de um valor, ex: "escolher f { ... }" (ver docs/04_escolher_e_formas.md) <br>
'->' --Símbolo que liga o padrão de uma cláusula do escolher ao código que ela executa <br>


'+' --Operador de soma <br>
'-' --Operador de Subtração <br>
'==' --Operador condicional 'Igual', faz a comparação entre dois valores, dois valores iguais TRUE, valores diferentes FALSE <br>
'!=' --Operador condicional 'Diferente', dois valores diferentes TRUE, valores iguais FALSE <br>
'<' --Operador condicional 'Menor que' <br>
'>' --Operador condicional 'Maior que' <br>
'&&' --Operador condicional 'And', faz a junção de dois valores para comparação, os dois valores cumprem a condição TRUE, resultados diferentes FALSE <br>
'||' --Operador condicional 'Or', faz a junção de dois valores para comparação, um valor cumpre a condição TRUE, nenhum valor cumpre a condição FALSE <br>
