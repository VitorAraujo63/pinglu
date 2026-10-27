.PHONY: verificar regex afd limpar

# O comando único que docs/01_recorte.md § "O comando único" e a Tarefa 3
# do módulo 1 pedem: um jeito só de reconstruir e rodar os casos
# existentes. O comando continua sendo o mesmo; o que ele faz por baixo
# cresce a cada módulo.
#
#   make verificar   roda tudo: a consistência léxica dos exemplos
#                    (scripts/verificar.py) e as demonstrações dos módulos
#                    02 e 03, que saem com código 1 se qualquer caso
#                    divergir do esperado.
#   make regex       só a demonstração do analisador de expressões (02).
#   make afd         só a demonstração dos autômatos (03).
#   make limpar      apaga build/.
#
# Os avisos do compilador são tratados como erro (-Werror): um aviso que
# ninguém lê hoje vira defeito silencioso depois.

CXX      ?= g++
CXXFLAGS := -std=c++20 -Wall -Wextra -Werror -Isrc

build/02_demo: src/02_regex_pinglu.cpp src/02_regex_pinglu.h demos/02_demo.cpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) src/02_regex_pinglu.cpp demos/02_demo.cpp -o $@

build/03_demo: src/03_afd.cpp src/03_afd.h demos/03_demo.cpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) src/03_afd.cpp demos/03_demo.cpp -o $@

regex: build/02_demo
	./build/02_demo

afd: build/03_demo
	./build/03_demo

verificar: build/02_demo build/03_demo
	python3 scripts/verificar.py
	./build/02_demo
	./build/03_demo

limpar:
	rm -rf build
