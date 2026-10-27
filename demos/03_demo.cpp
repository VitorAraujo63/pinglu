// 03_demo.cpp — a demonstração do marco 03: para cada máquina das classes
// léxicas, a tabela de transição, os casos de aceitação e de recusa com o
// motivo, e a conta de memória da representação escolhida.
//
// Também é o teste: sai com código 1 se qualquer caso divergir do esperado, e é
// isso que `make verificar` usa para acusar regressão.

#include "03_afd.h"

#include <exception>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace
{

    // Um caso: a cadeia, se a máquina deve aceitá-la, e (quando aceita) em que
    // estado deve parar. Esse segundo campo é o que distingue `42` de `3.14` na
    // máquina de número: as duas aceitam, e o estado diz qual das duas classes.
    struct Caso
    {
        std::string cadeia;
        bool esperado;
        std::string estadoFinal;
        bool traco;

        Caso(std::string c, bool e, std::string f = "", bool t = false)
            : cadeia(std::move(c)), esperado(e), estadoFinal(std::move(f)), traco(t) {}
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

    // Devolve quantos casos divergiram do esperado.
    int rodar(const pinglu::Afd &afd, const std::vector<Caso> &casos)
    {
        int falhas = 0;
        for (const Caso &caso : casos)
        {
            const pinglu::Execucao execucao = afd.executar(caso.cadeia);
            bool correto = execucao.aceitou() == caso.esperado;
            if (correto && caso.esperado && !caso.estadoFinal.empty())
            {
                correto = afd.nomeDoEstado(execucao.passos.back().estado) == caso.estadoFinal;
            }
            if (!correto)
            {
                ++falhas;
            }

            if (caso.traco)
            {
                std::cout << (correto ? "  [ok]    " : "  [FALHOU] ") << '\n'
                          << afd.formatarExecucao(caso.cadeia, execucao);
            }
            else
            {
                std::string eco;
                for (const char simbolo : caso.cadeia)
                {
                    // Reaproveita o mesmo tratamento de símbolo invisível da execução.
                    eco += simbolo == '\n'   ? std::string("\\n")
                           : simbolo == '\t' ? std::string("\\t")
                           : simbolo == '\r' ? std::string("\\r")
                                             : std::string(1, simbolo);
                }
                std::cout << (correto ? "  [ok]    " : "  [FALHOU] ")
                          << preencher("\"" + eco + "\"", 22) << " "
                          << afd.descreverResultado(caso.cadeia, execucao) << '\n';
            }
        }
        return falhas;
    }

    int secao(const pinglu::Afd &afd, const std::string &especificacao,
              const std::vector<Caso> &casos)
    {
        std::cout << "\n=== " << afd.nome() << " ===\n"
                  << especificacao << "\n\n"
                  << afd.formatarTabela() << afd.formatarResumo() << "\n";
        return rodar(afd, casos);
    }

} // namespace

int main()
{
    try
    {
        int falhas = 0;

        falhas += secao(
            pinglu::afdIdentificador(),
            "(letra | '_') (letra | digito | '_')*",
            {
                {"soma", true, "corpo"},
                {"_privado", true, "corpo"},
                {"contador2", true, "corpo", true},
                {"A", true, "corpo"},
                {"2contador", false, "", true},     // dígito é do alfabeto, mas não abre identificador
                {"", false},                        // a cadeia vazia não é identificador
                {"nome-invalido", false, "", true}, // '-' está fora do alfabeto desta máquina
            });

        falhas += secao(
            pinglu::afdNumero(),
            "digito+ ('.' digito+)?   (sem sinal: o '-' unario e do parser)",
            {
                {"42", true, "inteiro"},
                {"0", true, "inteiro"},
                {"3.14", true, "fracao", true},
                {"42.", false, "", true}, // lida inteira, sem cair, e parada em estado nao final
                {".5", false},
                {"1.2.3", false},
                {"-5", false}, // o sinal nao pertence ao literal
                {"", false},
            });

        falhas += secao(
            pinglu::afdCadeiaDeCaracteres(),
            "'\"' (qualquer byte menos '\"', '\\' e nova linha | '\\' ('\"' | '\\' | n | t))* '\"'",
            {
                {R"("texto")", true, "fechada"},
                {R"("")", true, "fechada"},
                {R"("fim do for")", true, "fechada"},
                {R"("a\"b")", true, "fechada", true},
                {R"("aberta)", false},                         // nunca fechada
                {R"("a\x")", false, "", true},                 // \x nao e escape valido
                {R"("a""b")", false},                          // duas cadeias, nao uma
                {"\"n\xc3\xa3o: sim\"", true, "fechada"},      // acento e dois-pontos: bytes comuns
                {"\"quem? `sim` \xc2\xb4\"", true, "fechada"}, // ?, crase e acento agudo: bytes comuns em texto
                {"\"a\nb\"", false, "", true},                 // nova linha dentro da cadeia: sem transicao
            });

        falhas += secao(
            pinglu::afdCaractere(),
            "'\\'' (qualquer menos '\\'' e '\\' | '\\' ('\\'' | '\\' | n)) '\\''",
            {
                {"'a'", true, "fechada"},
                {R"('\n')", true, "fechada"},
                {R"('\'')", true, "fechada", true},
                {"'\"'", true, "fechada"},
                {"''", false, "", true},  // vazio: de `aberta` a aspa nao tem transicao
                {"'?'", false, "", true}, // o ? saiu de Sigma: como char, e recusado
                {"'ab'", false},          // dois caracteres
                {"'a", false},
            });

        falhas += secao(
            pinglu::afdComentarioDeLinha(),
            "'//' (qualquer byte menos nova linha)*",
            {
                {"// ola mundo", true, "no_comentario"},
                // o comentario real de exemplos/05: acento, travessao (3 bytes) e dois-pontos
                {"// exercita \"forma\" \xe2\x80\x94 se\xc3\xa7\xc3\xa3o 4: cl\xc3\xa1usula", true, "no_comentario"},
                {"//\r", true, "no_comentario"}, // CRLF: o \r e um byte comum do comentario
                {"//", true, "no_comentario"},
                {"/ ola", false},
                {"// a\nb", false, "", true}, // a nova linha encerra o comentario
            });

        falhas += secao(
            pinglu::afdOperadoresEDelimitadores(),
            "+ * / % ( ) { } [ ] , ;  = < > !  -  ->  ==  !=  <=  >=  &&  ||",
            {
                {"+", true, "completo"},
                {";", true, "completo"},
                {"=", true, "pode_ganhar_igual"},
                {"==", true, "completo", true},
                {"!=", true, "completo"},
                {"<=", true, "completo"},
                {"-", true, "menos"},
                {"->", true, "completo"},
                {"&&", true, "completo"},
                {"||", true, "completo"},
                {"&", false, "", true}, // promessa pendente: falta o segundo '&'
                {"|", false},
                {"===", false}, // tres simbolos: o casamento mais longo leria == e depois =
                {"=>", false},
                {"a", false},
                {"", false},
            });

        falhas += secao(
            pinglu::afdSeparadores(),
            "(espaco | tab | nova linha | \\r)+   (descartados pelo lexer, fora de Sigma)",
            {
                {" ", true, "corpo"},
                {"  \t\n", true, "corpo"},
                {"\r\n", true, "corpo", true}, // fim de linha CRLF: um separador so
                {"", false},
                {"a", false},
                {" a", false, "", true}, // o separador acaba onde comeca o token
            });

        // --- a conta da representacao (docs/03_representacao_transicao.md) ------
        std::cout << "\n=== a conta da representacao (matriz densa, Estado = "
                  << sizeof(pinglu::Estado) << " bytes) ===\n";
        std::cout << preencher("AUTOMATO", 28) << preencher("UTEIS", 7) << preencher("ALFAB.", 8)
                  << preencher("POSICOES", 10) << preencher("BYTES", 8) << preencher("NAO-ERRO", 10)
                  << preencher("MAPA", 7) << "SE POR BYTE\n";
        std::size_t totalTabelas = 0;
        std::size_t totalMapas = 0;
        std::size_t totalPorByte = 0;
        for (const pinglu::Afd &afd : pinglu::automatosDoPinglu())
        {
            const std::size_t posicoes = (afd.estadosUteis() + 1) * afd.tamanhoDoAlfabeto();
            std::cout << preencher(afd.nome(), 28)
                      << preencher(std::to_string(afd.estadosUteis()), 7)
                      << preencher(std::to_string(afd.tamanhoDoAlfabeto()), 8)
                      << preencher(std::to_string(posicoes), 10)
                      << preencher(std::to_string(afd.bytesDaTabela()), 8)
                      << preencher(std::to_string(afd.transicoesDefinidas()), 10)
                      << preencher(std::to_string(afd.bytesDoMapeamento()), 7)
                      << afd.bytesSeIndexadaPorByte() << '\n';
            totalTabelas += afd.bytesDaTabela();
            totalMapas += afd.bytesDoMapeamento();
            totalPorByte += afd.bytesSeIndexadaPorByte();
        }
        std::cout << preencher("TOTAL", 28) << preencher("", 7) << preencher("", 8)
                  << preencher("", 10) << preencher(std::to_string(totalTabelas), 8)
                  << preencher("", 10) << preencher(std::to_string(totalMapas), 7) << totalPorByte
                  << '\n';

        std::cout << "\n"
                  << (falhas == 0 ? "todos os casos conferem."
                                  : std::to_string(falhas) + " caso(s) divergiram do esperado.")
                  << '\n';
        return falhas == 0 ? 0 : 1;
    }
    catch (const std::exception &erro)
    {
        std::cerr << "Erro: " << erro.what() << '\n';
        return 1;
    }
}
