# Pinglu

<br>

Repositório para a aula de Compiladores

Jeann Garçoni Alves <br>
Vitor Henrique Pereira Araujo <br>
Mateus Amaro da Silva <br>
Tiago Gabriel da Silva Cruz <br>
Vinicius Alves da Santana <br>
Daniel Ferreira Lopes

## Como rodar

Pré-requisitos: `make`, `g++` com suporte a C++20 e `python3` 3.9 ou superior
(testado com `g++` 13.3 e `python3` 3.12).

```
make verificar   # roda tudo: consistência léxica, módulo 02 e módulo 03
make regex       # só o analisador de expressões regulares
make afd         # só os autômatos finitos
make limpar      # apaga build/
```

`make verificar` sai com código diferente de zero se algum caso divergir do
esperado. A organização das pastas está em `docs/01_organizacao_repositorio.md`.
