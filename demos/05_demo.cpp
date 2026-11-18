// 05_demo.cpp — a demonstração da determinização: a construção de
// subconjuntos sobre a árvore de um pattern, conferida contra o guia do
// módulo 05, contra o AFN que a originou e contra a bateria conferida à mão.
//
// Também é o teste do módulo: sai com código 1 se qualquer verificação
// divergir. Precisa rodar da raiz do repositório (o `make` já faz isso): o
// caminho da bateria é relativo.

#include "02_regex_pinglu.h"
#include "03_afd.h"
#include "04_afn.h"
#include "05_determinizacao.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{

    using pinglu::Estado;

    int g_falhas = 0;

    void confere(const bool correto, const std::string &descricao)
    {
        std::cout << (correto ? "  [ok]     " : "  [FALHOU] ") << descricao << '\n';
        if (!correto)
        {
            ++g_falhas;
        }
    }

    bool lanca(const std::function<void()> &acao)
    {
        try
        {
            acao();
        }
        catch (const std::invalid_argument &)
        {
            return true;
        }
        return false;
    }

    // O AFN do pattern, ou uma falha registrada se a leitura o recusar.
    bool construir(const std::string &padrao, pinglu::Afn &afn)
    {
        const pinglu::Resultado leitura = pinglu::analisarExpressao(padrao);
        if (!leitura.ok)
        {
            confere(false, "/" + padrao + "/ foi recusado pela leitura: " + leitura.erro.mensagem);
            return false;
        }
        afn = pinglu::construirThompson(leitura.arvore);
        return true;
    }

    // Chama `visitar` para cada cadeia sobre `simbolos` de comprimento 0 até
    // `maximo`, a vazia inclusive.
    void paraCadaCadeia(const std::string &simbolos, const std::size_t maximo,
                        const std::function<void(const std::string &)> &visitar)
    {
        std::function<void(const std::string &)> gerar = [&](const std::string &cadeia)
        {
            visitar(cadeia);
            if (cadeia.size() < maximo)
            {
                for (const char simbolo : simbolos)
                {
                    gerar(cadeia + simbolo);
                }
            }
        };
        gerar("");
    }

    std::string comoTexto(const std::set<int> &alfabeto)
    {
        std::string texto;
        for (const int rotulo : alfabeto)
        {
            texto += static_cast<char>(rotulo);
        }
        return texto;
    }

    // --- A: o exemplo do guia, estado a estado ---------------------------------

    struct LinhaEsperada
    {
        std::vector<Estado> conjunto;
        Estado a, b, c;
        bool aceita;
    };

    void secaoExemploDoGuia()
    {
        std::cout << "\n=== A /(a|b)*c/: a tabela do guia (secao 1.5.6), estado a estado ===\n";
        pinglu::Afn afn;
        if (!construir("(a|b)*c", afn))
        {
            return;
        }
        const pinglu::AfdGerado afd = pinglu::determinizar(afn);
        std::cout << "AFN: " << afn.quantidadeDeEstados() << " estados -> AFD: " << afd.quantidadeDeEstados()
                  << " estados\n"
                  << afd.formatarTabela();

        // Copiada do guia: D4 e o conjunto vazio, o estado de erro.
        const std::vector<LinhaEsperada> esperado = {
            {{0, 2, 4, 6, 7, 8}, 1, 2, 3, false},
            {{0, 1, 2, 4, 5, 7, 8}, 1, 2, 3, false},
            {{0, 2, 3, 4, 5, 7, 8}, 1, 2, 3, false},
            {{9}, 4, 4, 4, true},
            {{}, 4, 4, 4, false},
        };
        bool bate = afd.quantidadeDeEstados() == esperado.size() && afd.inicial() == 0;
        for (Estado estado = 0; bate && estado < esperado.size(); ++estado)
        {
            const LinhaEsperada &linha = esperado[estado];
            bate = afd.representa(estado) == linha.conjunto && afd.transicao(estado, 'a') == linha.a &&
                   afd.transicao(estado, 'b') == linha.b && afd.transicao(estado, 'c') == linha.c &&
                   afd.ehDeAceitacao(estado) == linha.aceita;
        }
        confere(bate, "5 estados, com os mesmos conjuntos, transicoes e aceitacao da tabela do guia; "
                      "D4 e o conjunto vazio (estado de erro)");

        std::size_t total = 0;
        std::size_t aceitasAfn = 0;
        std::size_t aceitasAfd = 0;
        std::size_t divergencias = 0;
        paraCadaCadeia("abc", 4, [&](const std::string &cadeia)
                       {
                           ++total;
                           const bool porAfn = afn.aceita(cadeia);
                           const bool porAfd = afd.aceita(cadeia);
                           aceitasAfn += porAfn ? 1 : 0;
                           aceitasAfd += porAfd ? 1 : 0;
                           divergencias += porAfn != porAfd ? 1 : 0; });
        std::cout << "  cadeias sobre {a, b, c} ate o comprimento 4: " << total << "; AFN aceita "
                  << aceitasAfn << ", AFD aceita " << aceitasAfd << '\n';
        confere(total == 121 && aceitasAfn == 15 && aceitasAfd == 15 && divergencias == 0,
                "121 cadeias, AFN e AFD aceitam as mesmas 15 (numeros da descricao da atividade)");

        confere(afn.aceita("ababac") && afd.aceita("ababac") && !afn.aceita("abab") && !afd.aceita("abab"),
                "ababac aceita e abab recusa, nas duas maquinas");
    }

    // --- B: as verificacoes da secao 1.8.3 do guia, em muitos patterns --------

    // Confere a construção por dentro, sem simular nada: cada estado representa
    // um conjunto ordenado e distinto dos outros, o inicial é o fecho do inicial
    // do AFN, cada transição leva ao conjunto E(mover(S, a)), a aceitação é
    // exatamente "contém o estado de aceitação do AFN", e a máquina é completa.
    std::string problemaNaConstrucao(const pinglu::Afn &afn, const pinglu::AfdGerado &afd)
    {
        if (afd.alfabeto() != afn.alfabeto())
        {
            return "o alfabeto do AFD nao e o do AFN";
        }
        if (afd.alfabeto().contains(pinglu::kEpsilon))
        {
            return "transicao vazia no AFD";
        }
        if (!afd.completo())
        {
            return "AFD incompleto";
        }
        std::set<std::vector<Estado>> vistos;
        for (Estado estado = 0; estado < afd.quantidadeDeEstados(); ++estado)
        {
            const std::vector<Estado> &conjunto = afd.representa(estado);
            if (!std::is_sorted(conjunto.begin(), conjunto.end()))
            {
                return "D" + std::to_string(estado) + " representa um conjunto fora de ordem";
            }
            if (!vistos.insert(conjunto).second)
            {
                return "dois estados do AFD representam " + pinglu::conjuntoComoTexto(conjunto);
            }
            const bool contem = std::binary_search(conjunto.begin(), conjunto.end(), afn.aceitacao());
            if (contem != afd.ehDeAceitacao(estado))
            {
                return "a aceitacao de D" + std::to_string(estado) + " nao bate com o conjunto";
            }
            for (const int rotulo : afd.alfabeto())
            {
                const std::optional<Estado> destino = afd.transicao(estado, rotulo);
                if (!destino || afd.representa(*destino) != pinglu::moverEFechar(afn, conjunto, rotulo))
                {
                    return "a transicao de D" + std::to_string(estado) + " com '" +
                           pinglu::simboloVisivel(static_cast<char>(rotulo)) + "' nao leva a E(mover(S, a))";
                }
            }
        }
        std::vector<Estado> inicial = afn.fechoVazio({afn.inicial()});
        std::sort(inicial.begin(), inicial.end());
        if (afd.representa(afd.inicial()) != inicial)
        {
            return "o estado inicial nao representa o fecho do inicial do AFN";
        }
        return "";
    }

    // AFN e AFD em todas as cadeias até um comprimento que caiba em ~6000
    // cadeias, sobre o alfabeto do pattern MAIS um símbolo de fora dele. Como o
    // Pinglu não tem coringa, as duas máquinas devem concordar também ali.
    std::string divergenciaEntre(const pinglu::Afn &afn, const pinglu::AfdGerado &afd)
    {
        std::string simbolos = comoTexto(afn.alfabeto());
        for (const char candidato : std::string("z#~Q"))
        {
            if (simbolos.find(candidato) == std::string::npos)
            {
                simbolos += candidato;
                break;
            }
        }
        std::size_t maximo = 0;
        for (std::size_t total = 1, potencia = 1; maximo < 8;)
        {
            potencia *= simbolos.size();
            if (total + potencia > 6000)
            {
                break;
            }
            total += potencia;
            ++maximo;
        }
        std::string divergencia;
        paraCadaCadeia(simbolos, maximo, [&](const std::string &cadeia)
                       {
                           if (divergencia.empty() && afn.aceita(cadeia) != afd.aceita(cadeia))
                           {
                               divergencia = "\"" + cadeia + "\": AFN " + (afn.aceita(cadeia) ? "aceita" : "recusa");
                           } });
        return divergencia;
    }

    void secaoVerificacoes(const std::vector<pinglu::CasoDeTeste> &casos)
    {
        std::cout << "\n=== B as verificacoes da determinizacao (guia, secao 1.8.3) ===\n";
        std::set<std::string> padroes;
        for (const pinglu::CasoDeTeste &caso : casos)
        {
            padroes.insert(caso.expressao);
        }
        padroes.insert({"(a|b)*c", "(a|b)*abb", "(a*)*", "a**", "((a|b)*|c)+d?", "(a|b)*a(a|b)(a|b)"});

        std::size_t conferidos = 0;
        bool tudoCerto = true;
        for (const std::string &padrao : padroes)
        {
            pinglu::Afn afn;
            if (!construir(padrao, afn))
            {
                tudoCerto = false;
                continue;
            }
            ++conferidos;
            const pinglu::AfdGerado afd = pinglu::determinizar(afn);
            std::string problema = problemaNaConstrucao(afn, afd);
            if (problema.empty())
            {
                const std::string divergencia = divergenciaEntre(afn, afd);
                problema = divergencia.empty() ? "" : "AFN e AFD divergem em " + divergencia;
            }
            if (!problema.empty())
            {
                tudoCerto = false;
                confere(false, "/" + padrao + "/ - " + problema);
            }
        }
        confere(tudoCerto && conferidos > 0,
                std::to_string(conferidos) +
                    " patterns: AFD completo, sem transicao vazia, um estado por conjunto, cada transicao "
                    "leva a E(mover(S, a)), aceitacao certa, e AFN e AFD concordam em todas as cadeias curtas");

        // O vazio so aparece quando e alcancavel: em (a|b)*abb nenhuma cadeia
        // mata todos os percursos, e o guia mede 5 estados sem o vazio.
        pinglu::Afn afn;
        if (construir("(a|b)*abb", afn))
        {
            const pinglu::AfdGerado afd = pinglu::determinizar(afn);
            bool temVazio = false;
            for (Estado estado = 0; estado < afd.quantidadeDeEstados(); ++estado)
            {
                temVazio = temVazio || afd.representa(estado).empty();
            }
            std::cout << "  /(a|b)*abb/: AFN " << afn.quantidadeDeEstados() << " estados -> AFD "
                      << afd.quantidadeDeEstados() << " estados\n";
            confere(afn.quantidadeDeEstados() == 14 && afd.quantidadeDeEstados() == 5 && !temVazio,
                    "/(a|b)*abb/: 14 -> 5 estados e nenhum deles e o vazio, que ali e inalcancavel (guia, 1.7.3)");
        }
    }

    // --- C: a bateria conferida a mao, agora no AFD ----------------------------

    void secaoBateria(const pinglu::LeituraDeCasos &leitura)
    {
        std::cout << "\n=== C a bateria conferida a mao, executada no AFD ===\n";
        for (const std::string &erro : leitura.erros)
        {
            confere(false, "arquivo de casos: " + erro);
        }
        if (leitura.casos.empty())
        {
            confere(false, "nenhum caso lido: uma bateria vazia passaria sem verificar nada");
            return;
        }
        std::size_t passaram = 0;
        for (const pinglu::CasoDeTeste &caso : leitura.casos)
        {
            const pinglu::Resultado arvore = pinglu::analisarExpressao(caso.expressao);
            if (!arvore.ok)
            {
                confere(false, caso.origem + ": pattern recusado pela leitura");
                continue;
            }
            const pinglu::AfdGerado afd = pinglu::determinizar(pinglu::construirThompson(arvore.arvore));
            const bool obtido = afd.aceita(caso.cadeia);
            if (obtido == caso.esperado)
            {
                ++passaram;
                continue;
            }
            confere(false, caso.origem + ": /" + caso.expressao + "/ sobre \"" + caso.cadeia + "\" - esperado " +
                               (caso.esperado ? "aceita" : "recusa") + ", obtido " + (obtido ? "aceita" : "recusa"));
        }
        std::cout << "Bateria no AFD: " << passaram << "/" << leitura.casos.size() << " casos passaram.\n";
        if (passaram != leitura.casos.size())
        {
            ++g_falhas;
        }
    }

    // --- D: as classes lexicas, contra as maquinas feitas a mao ----------------

    void secaoClasseLexica(const std::string &nome, const pinglu::Afd &feita, const std::string &padrao,
                           const std::string &simbolos, const std::size_t maximo)
    {
        pinglu::Afn afn;
        if (!construir(padrao, afn))
        {
            return;
        }
        const pinglu::AfdGerado afd = pinglu::determinizar(afn);
        std::cout << "  " << nome << ": AFN " << afn.quantidadeDeEstados() << " estados -> AFD "
                  << afd.quantidadeDeEstados() << " estados (alfabeto de " << afd.alfabeto().size()
                  << " simbolos); a maquina feita a mao tem " << feita.estadosUteis() << " uteis + erro\n";

        std::size_t total = 0;
        std::string divergencia;
        paraCadaCadeia(simbolos, maximo, [&](const std::string &cadeia)
                       {
                           ++total;
                           if (divergencia.empty() && feita.aceita(cadeia) != afd.aceita(cadeia))
                           {
                               divergencia = "\"" + cadeia + "\"";
                           } });
        confere(divergencia.empty() && problemaNaConstrucao(afn, afd).empty(),
                nome + ": " + std::to_string(total) +
                    " cadeias, o AFD gerado e a maquina do modulo 03 nunca divergem" +
                    (divergencia.empty() ? "" : "; primeira divergencia: " + divergencia));
    }

    void secaoClassesLexicas()
    {
        std::cout << "\n=== D as classes lexicas: AFD gerado contra a maquina feita a mao (modulo 03) ===\n";
        secaoClasseLexica("identificador", pinglu::afdIdentificador(), "[a-zA-Z_][a-zA-Z0-9_]*", "aZ_7-", 5);
        secaoClasseLexica("numero", pinglu::afdNumero(), "[0-9]+(\\.[0-9]+)?", "05.-", 6);
        secaoClasseLexica("operadores_e_delimitadores", pinglu::afdOperadoresEDelimitadores(),
                          "\\+|\\*|/|%|\\(|\\)|\\{|\\}|\\[|\\]|,|;|=|<|>|!|-|==|!=|<=|>=|->|&&|\\|\\|",
                          "+*/%(){}[],;=<>!-&|a", 3);
    }

    // --- E: as bordas ----------------------------------------------------------

    void secaoBordas()
    {
        std::cout << "\n=== E as bordas ===\n";

        const pinglu::AfdGerado vazia = pinglu::determinizar(pinglu::construirThompson(pinglu::Arvore{}));
        confere(vazia.quantidadeDeEstados() == 1 && vazia.alfabeto().empty() && vazia.aceita("") &&
                    !vazia.aceita("a"),
                "arvore vazia: um estado, alfabeto vazio, aceita so a cadeia vazia");

        const pinglu::AfdGerado semEstados = pinglu::determinizar(pinglu::Afn{});
        confere(semEstados.quantidadeDeEstados() == 1 && semEstados.representa(0).empty() &&
                    !semEstados.aceita(""),
                "AFN sem estados: o AFD e so o estado de erro (o vazio) e recusa tudo");

        const std::string comZero("a\0b", 3);
        pinglu::Afn afnZero;
        if (construir(comZero, afnZero))
        {
            const pinglu::AfdGerado afd = pinglu::determinizar(afnZero);
            confere(afd.alfabeto() == std::set<int>{0, 'a', 'b'} && afd.aceita(comZero) && !afd.aceita("ab"),
                    "o byte zero no pattern e simbolo do alfabeto, e nao transicao vazia");
        }

        pinglu::Afn afn;
        if (construir("ab", afn))
        {
            pinglu::AfdGerado afd = pinglu::determinizar(afn);
            confere(lanca([&]
                          { (void)afn.mover({0}, pinglu::kEpsilon); }) &&
                        lanca([&]
                              { (void)afn.mover({99}, 'a'); }) &&
                        lanca([&]
                              { afd.adicionarTransicao(0, pinglu::kEpsilon, 0); }) &&
                        lanca([&]
                              { afd.adicionarTransicao(0, 'z', 0); }) &&
                        lanca([&]
                              { afd.adicionarTransicao(0, 'a', 99); }),
                    "mover por kEpsilon, estado inexistente e rotulo fora do alfabeto lancam invalid_argument");
            confere(afn.mover({0}, 'a') == std::vector<Estado>{1} && afn.mover({0}, 'b').empty(),
                    "mover nao aplica o fecho: de {0} lendo a vai so a {1}");
        }
    }

} // namespace

int main()
{
    try
    {
        const pinglu::LeituraDeCasos leitura = pinglu::lerCasos("exemplos/07_casos_afn.txt");

        secaoExemploDoGuia();
        secaoVerificacoes(leitura.casos);
        secaoBateria(leitura);
        secaoClassesLexicas();
        secaoBordas();

        std::cout << '\n'
                  << (g_falhas == 0 ? "todos os casos conferem."
                                    : std::to_string(g_falhas) + " verificacao(oes) divergiram do esperado.")
                  << '\n';
        return g_falhas == 0 ? 0 : 1;
    }
    catch (const std::exception &erro)
    {
        std::cerr << "Erro: " << erro.what() << '\n';
        return 1;
    }
}
