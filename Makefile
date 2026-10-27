.PHONY: verificar afd

# O comando único que docs/01_recorte.md § "O comando único" e a Tarefa 3
# do módulo 1 pedem: um jeito só de reconstruir e rodar os casos
# existentes. O comando continua sendo o mesmo; o que ele faz por baixo
# cresce a cada módulo.
#
#   make verificar   confere a consistência léxica dos exemplos
#                    (scripts/verificar.py) e roda a demonstração dos
#                    autômatos do módulo 03, que sai com código 1 se
#                    qualquer caso divergir do esperado.
#   make afd         só a demonstração dos autômatos.
#
# Os avisos do compilador são tratados como erro (-Werror): um aviso que
# ninguém lê hoje vira defeito silencioso depois.

CXX      ?= g++
CXXFLAGS := -std=c++20 -Wall -Wextra -Werror -Isrc

build/03_demo: src/03_afd.cpp src/03_afd.h demos/03_demo.cpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) src/03_afd.cpp demos/03_demo.cpp -o $@

afd: build/03_demo
	./build/03_demo

verificar: build/03_demo
	python3 scripts/verificar.py
	./build/03_demo
