.PHONY: verificar

# O comando único que docs/01_recorte.md § "O comando único" e a Tarefa 3
# do módulo 1 pedem: um jeito só de reconstruir e rodar os casos
# existentes, mesmo que hoje "reconstruir" seja quase nada — só a
# checagem de consistência léxica de scripts/verificar.py.
#
# Uso: make verificar
verificar:
	python3 scripts/verificar.py
