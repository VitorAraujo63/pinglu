// 02_demo.cpp — a demonstração do marco 02: o analisador de expressões
// regulares do Pinglu, com a saída esperada de cada caso escrita ao lado.
//
// Também é o teste do módulo: sai com código 1 se qualquer caso divergir do
// esperado, e é isso que `make verificar` usa para acusar regressão. Sem isto o
// analisador compila mas ninguém o exercita, e um defeito entraria sem sintoma
// até a construção de Thompson tropeçar nele.

#include "02_regex_pinglu.h"

#include <cstddef>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace
{

    struct CasoDeArvore
    {
        std::string padrao;
        std::string esperada; // forma prefixa canônica de formatarArvore
    };

    struct CasoDeErro
    {
        std::string padrao;
        std::size_t posicao; // onde o primeiro erro deve ser apontado
    };

    std::string preencher(const std::string &texto, const std::size_t largura)
    {
        std::string resultado = texto;
        while (resultado.size() < largura)
        {
            resultado += ' ';
        }
        return resultado;
    }

    // Reduções ao núcleo: o que sai do analisador já não conhece `+`, `?`,
    // classe de símbolos nem grupo (docs/02_nucleo_minimo.md).
    int reducoes()
    {
        const std::vector<CasoDeArvore> casos = {
            {"a", "'a'"},
            {"ab", "concat('a', 'b')"},
            {"a|b", "alt('a', 'b')"},
            {"a*", "fecho('a')"},
            {"a+", "concat('a', fecho('a'))"}, // x+ vira x x*
            {"a?", "alt('a', vazio)"},         // x? vira (x|vazio)
            {"[a-c]", "alt(alt('a', 'b'), 'c')"},
            {"(ab)c", "concat(concat('a', 'b'), 'c')"}, // o grupo nao sobrevive a leitura
        };

        std::cout << "\n=== reducoes ao nucleo ===\n";
        int falhas = 0;
        for (const CasoDeArvore &caso : casos)
        {
            const pinglu::Resultado resultado = pinglu::analisarExpressao(caso.padrao);
            const std::string obtida = resultado.ok ? pinglu::formatarArvore(resultado.arvore) : "(erro)";
            const bool correto = resultado.ok && obtida == caso.esperada;
            falhas += correto ? 0 : 1;
            std::cout << (correto ? "  [ok]     " : "  [FALHOU] ") << preencher(caso.padrao, 10)
                      << " -> " << obtida << '\n';
            if (!correto)
            {
                std::cout << "           esperado: " << caso.esperada << '\n';
            }
        }
        return falhas;
    }

    // Duas notações do mesmo padrão têm de convergir para a mesma árvore: é a
    // comparação de texto, e não a inspeção de duas figuras, que a prova.
    int convergencias()
    {
        const std::vector<std::pair<std::string, std::string>> pares = {
            {"[a-c]", "[abc]"}, // faixa e enumeração
            {"(ab)c", "abc"},   // grupo supérfluo
            {"a+", "aa*"},      // a redução de + é a escrita por extenso
        };

        std::cout << "\n=== notacoes que convergem para a mesma arvore ===\n";
        int falhas = 0;
        for (const auto &par : pares)
        {
            const pinglu::Resultado esquerda = pinglu::analisarExpressao(par.first);
            const pinglu::Resultado direita = pinglu::analisarExpressao(par.second);
            const bool correto = esquerda.ok && direita.ok &&
                                 pinglu::formatarArvore(esquerda.arvore) ==
                                     pinglu::formatarArvore(direita.arvore);
            falhas += correto ? 0 : 1;
            std::cout << (correto ? "  [ok]     " : "  [FALHOU] ") << preencher(par.first, 10)
                      << " == " << par.second << '\n';
        }
        return falhas;
    }

    // O Pinglu não tem coringa (docs/02_nucleo_minimo.md): o `.` é um símbolo
    // literal. Sem esta verificação, um número decimal como [0-9]+.[0-9]+
    // casaria "qualquer símbolo" no lugar do ponto, e ninguém perceberia.
    int pontoLiteral()
    {
        const std::string padrao = "[0-9]+.[0-9]+";
        const pinglu::Resultado resultado = pinglu::analisarExpressao(padrao);
        const std::string arvore = resultado.ok ? pinglu::formatarArvore(resultado.arvore) : "";
        // Uma classe [0-9] são 10 folhas e 9 alternâncias (19 nós); o + duplica a
        // subárvore (1 + 19 + 1 fecho + 19 = 40 por lado); o ponto é 1 folha e há
        // 2 concatenações entre as 3 partes: 2 * 40 + 1 + 2 = 83.
        const std::size_t nosEsperados = 83;
        const bool correto = resultado.ok && arvore.find("'.'") != std::string::npos &&
                             pinglu::tamanho(resultado.arvore) == nosEsperados;

        std::cout << "\n=== o ponto e um simbolo literal ===\n"
                  << (correto ? "  [ok]     " : "  [FALHOU] ") << padrao << " -> "
                  << (resultado.ok ? std::to_string(pinglu::tamanho(resultado.arvore)) : "erro")
                  << " nos (esperado " << nosEsperados << "), contem '.' literal: "
                  << (arvore.find("'.'") != std::string::npos ? "sim" : "nao") << '\n';
        return correto ? 0 : 1;
    }

    // Todo erro carrega a posição exata, e o cursor da mensagem cai sob ela.
    int erros()
    {
        const std::vector<CasoDeErro> casos = {
            {"", 0},      // expressão vazia
            {"(a", 2},    // grupo sem fechar: o erro é no fim do texto
            {"a)", 1},    // fecha-parênteses sem abertura
            {"*a", 0},    // operador sem a que se aplicar
            {"a|", 2},    // alternativa sem lado direito
            {"[]", 2},    // classe vazia
            {"[c-a]", 4}, // faixa invertida
            {"[a", 2},    // classe sem fechar
        };

        std::cout << "\n=== erros com posicao ===\n";
        int falhas = 0;
        for (const CasoDeErro &caso : casos)
        {
            const pinglu::Resultado resultado = pinglu::analisarExpressao(caso.padrao);
            const bool correto = !resultado.ok && resultado.erro.posicao == caso.posicao;
            falhas += correto ? 0 : 1;
            std::cout << (correto ? "  [ok]     " : "  [FALHOU] ");
            if (resultado.ok)
            {
                std::cout << "\"" << caso.padrao << "\" foi aceito, mas devia ser recusado\n";
            }
            else
            {
                std::cout << "recusa esperada na posicao " << caso.posicao << '\n'
                          << pinglu::formatarErro(caso.padrao, resultado.erro) << '\n';
            }
        }
        return falhas;
    }

} // namespace

int main()
{
    int falhas = 0;
    falhas += reducoes();
    falhas += convergencias();
    falhas += pontoLiteral();
    falhas += erros();

    std::cout << '\n'
              << (falhas == 0 ? "todos os casos conferem."
                              : std::to_string(falhas) + " caso(s) divergiram do esperado.")
              << '\n';
    return falhas == 0 ? 0 : 1;
}
