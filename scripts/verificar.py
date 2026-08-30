#!/usr/bin/env python3
"""
verificar.py — o comando único (docs/01_recorte.md § "O comando único").

Hoje não existe analisador léxico nem parser do Pinglu, então este script
não compila nada. O que ele faz é automatizar a checagem que
docs/01_exemplo_validado.md já descrevia em prosa, em "Como verificar que
está correta": conferir que toda palavra-chave usada em cada arquivo de
exemplos/ tem entrada correspondente em especificacao/classes_lexicas.md.

Quando o analisador léxico existir, este script é o lugar natural para
crescer: trocar a checagem de palavras-chave por uma chamada real ao lexer
e, depois, ao parser e à VM, comparando a saída real com o bloco
"Resultado esperado" no cabeçalho de cada .pglu. A estrutura de
"um caso, um resultado esperado, uma falha clara" já está pronta para isso.
"""

import re
import sys
from pathlib import Path

RAIZ = Path(__file__).resolve().parent.parent
CLASSES_LEXICAS = RAIZ / "especificacao" / "classes_lexicas.md"
EXEMPLOS = RAIZ / "exemplos"

# Palavras que a gramática (docs/01_gramatica.txt) trata como terminal fixo
# -- se qualquer uma destas aparecer num exemplo, ela PRECISA estar
# documentada em classes_lexicas.md. Esta lista é o "vocabulário fechado"
# que o script conhece; crescer a gramática sem atualizar esta lista é o
# mesmo tipo de furo que o feedback apontou no questionário original.
PALAVRAS_RESERVADAS = {
    "int", "string", "char", "decimal", "float", "list",
    "if", "else", "while", "for", "pineach", "in",
    "print", "func", "return", "forma", "escolher",
}


def carregar_keywords_documentadas(caminho: Path) -> set[str]:
    """Extrai as palavras-chave que classes_lexicas.md documenta.

    O formato de cada linha é `palavra --Palavra-Chave ...` ou
    `'símbolo' --Operador ...`; aqui só interessam as palavras (não os
    símbolos entre aspas), porque PALAVRAS_RESERVADAS só lista palavras.
    """
    documentadas = set()
    texto = caminho.read_text(encoding="utf-8")
    for linha in texto.splitlines():
        linha = linha.strip()
        if not linha or linha.startswith("'") or "--" not in linha:
            continue
        palavra = linha.split("--", 1)[0].strip()
        if palavra and re.fullmatch(r"[a-zA-Z_][a-zA-Z0-9_]*", palavra):
            documentadas.add(palavra)
    return documentadas


def remover_comentarios_e_literais(fonte: str) -> str:
    """Tira comentários `// ...` e o conteúdo de strings/chars, para não
    confundir uma palavra dentro de um literal com uma palavra-chave do
    código."""
    sem_comentarios = re.sub(r"//.*", "", fonte)
    sem_strings = re.sub(r'"(?:[^"\\]|\\.)*"', '""', sem_comentarios)
    sem_chars = re.sub(r"'(?:[^'\\]|\\.)*'", "''", sem_strings)
    return sem_chars


def palavras_usadas(fonte_limpa: str) -> set[str]:
    return set(re.findall(r"\b[a-zA-Z_][a-zA-Z0-9_]*\b", fonte_limpa))


def verificar_arquivo(caminho: Path, documentadas: set[str]) -> list[str]:
    fonte = caminho.read_text(encoding="utf-8")
    limpo = remover_comentarios_e_literais(fonte)
    usadas = palavras_usadas(limpo)

    problemas = []
    for palavra in sorted(usadas & PALAVRAS_RESERVADAS):
        if palavra not in documentadas:
            problemas.append(
                f"'{palavra}' é usada no exemplo mas não tem entrada em "
                f"{CLASSES_LEXICAS.relative_to(RAIZ)}"
            )
    return problemas


def main() -> int:
    if not CLASSES_LEXICAS.exists():
        print(f"ERRO: {CLASSES_LEXICAS} não encontrado.")
        return 1
    if not EXEMPLOS.exists():
        print(f"ERRO: {EXEMPLOS} não encontrado.")
        return 1

    documentadas = carregar_keywords_documentadas(CLASSES_LEXICAS)
    arquivos = sorted(EXEMPLOS.glob("*.pglu"))

    if not arquivos:
        print("Nenhum arquivo .pglu em exemplos/ — nada para verificar.")
        return 0

    total_problemas = 0
    for caminho in arquivos:
        problemas = verificar_arquivo(caminho, documentadas)
        nome = caminho.relative_to(RAIZ)
        if problemas:
            total_problemas += len(problemas)
            print(f"[FALHA] {nome}")
            for problema in problemas:
                print(f"        {problema}")
        else:
            print(f"[ok]    {nome}")

    print()
    print(f"{len(arquivos)} exemplo(s) verificado(s), "
          f"{total_problemas} problema(s) encontrado(s).")
    print(
        "Nota: isto verifica consistência léxica documentada, não roda os "
        "programas — não há lexer/parser/VM ainda. Ver a nota no topo "
        "deste arquivo para o que muda quando existirem."
    )
    return 1 if total_problemas else 0


if __name__ == "__main__":
    sys.exit(main())
