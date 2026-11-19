// 05_demo.cpp — a demonstração do módulo 05: a construção de subconjuntos e a
// minimização por refinamento de partições sobre a árvore de um pattern,
// conferidas contra o guia do módulo 05, contra o AFN que as originou, contra
// a bateria conferida à mão e contra as máquinas feitas à mão do módulo 03; a
// comparação exaustiva das três máquinas; e o caso construído de propósito
// para a explosão de estados.
//
// Convenção das contagens, declarada antes de qualquer número: as contagens do
// AFD e do mínimo INCLUEM o estado de erro quando ele é alcançável; a do AFN
// não tem estado de erro (lá, a falta de transição já é caminho morto).
//
// Também é o teste do módulo: sai com código 1 se qualquer verificação
// divergir. Precisa rodar da raiz do repositório (o `make` já faz isso): o
// caminho da bateria é relativo.

#include "02_regex_pinglu.h"
#include "03_afd.h"
#include "04_afn.h"
#include "05_determinizacao.h"
#include "05_minimizacao.h"

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
        std::cout << "\n=== C a bateria conferida a mao, executada no AFD e no minimo ===\n";
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
            const pinglu::AfdGerado minimo = pinglu::minimizar(afd).afd;
            const bool porAfd = afd.aceita(caso.cadeia);
            const bool porMinimo = minimo.aceita(caso.cadeia);
            if (porAfd == caso.esperado && porMinimo == caso.esperado)
            {
                ++passaram;
                continue;
            }
            confere(false, caso.origem + ": /" + caso.expressao + "/ sobre \"" + caso.cadeia + "\" - esperado " +
                               (caso.esperado ? "aceita" : "recusa") + ", obtido AFD " +
                               (porAfd ? "aceita" : "recusa") + ", minimo " + (porMinimo ? "aceita" : "recusa"));
        }
        std::cout << "Bateria no AFD e no minimo: " << passaram << "/" << leitura.casos.size()
                  << " casos passaram.\n";
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
        const pinglu::ResultadoDaMinimizacao minimo = pinglu::minimizar(afd);
        std::cout << "  " << nome << ": AFN " << afn.quantidadeDeEstados() << " -> AFD "
                  << afd.quantidadeDeEstados() << " -> minimo " << minimo.estadosDepois << " estados (alfabeto de "
                  << afd.alfabeto().size() << " simbolos); a maquina feita a mao tem " << feita.estadosUteis()
                  << " uteis + erro\n";

        std::size_t total = 0;
        std::string divergencia;
        paraCadaCadeia(simbolos, maximo, [&](const std::string &cadeia)
                       {
                           ++total;
                           const bool porFeita = feita.aceita(cadeia);
                           if (divergencia.empty() &&
                               (porFeita != afd.aceita(cadeia) || porFeita != minimo.afd.aceita(cadeia)))
                           {
                               divergencia = "\"" + cadeia + "\"";
                           } });
        confere(divergencia.empty() && problemaNaConstrucao(afn, afd).empty(),
                nome + ": " + std::to_string(total) +
                    " cadeias, o AFD gerado, o minimo e a maquina do modulo 03 nunca divergem" +
                    (divergencia.empty() ? "" : "; primeira divergencia: " + divergencia));
        // A máquina feita à mão foi projetada no módulo 03 sem algoritmo nenhum;
        // se ela já era mínima, a minimização tem de chegar ao mesmo tamanho.
        confere(minimo.estadosDepois == feita.estadosUteis() + 1,
                nome + ": o minimo tem " + std::to_string(minimo.estadosDepois) +
                    " estados, o mesmo tamanho da maquina feita a mao (uteis + erro)");
    }

    void secaoClassesLexicas()
    {
        std::cout << "\n=== D as classes lexicas: AFD gerado e minimo contra a maquina feita a mao (modulo 03) ===\n";
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

    // --- F: a minimizacao, nos exemplos do guia --------------------------------

    std::string comoTexto(const std::vector<std::size_t> &valores)
    {
        return pinglu::formatarRodadas(valores);
    }

    void secaoMinimizacao()
    {
        std::cout << "\n=== F a minimizacao por refinamento de particoes ===\n"
                  << "(as contagens do AFD e do minimo incluem o estado de erro quando ele e alcancavel; "
                     "a do AFN nao)\n";

        pinglu::Afn afn;
        if (construir("(a|b)*c", afn))
        {
            const pinglu::AfdGerado afd = pinglu::determinizar(afn);
            const pinglu::ResultadoDaMinimizacao minimo = pinglu::minimizar(afd);
            std::cout << "/(a|b)*c/  AFN " << afn.quantidadeDeEstados() << " -> AFD " << minimo.estadosAntes
                      << " -> minimo " << minimo.estadosDepois << "\n"
                      << "  refinamento da particao, rodada a rodada: " << comoTexto(minimo.blocosPorRodada)
                      << " (a ultima repete a anterior: ponto fixo)\n"
                      << "  cada estado do minimo representa um bloco de estados do AFD:\n"
                      << minimo.afd.formatarTabela("M");
            const pinglu::AfdGerado &m = minimo.afd;
            confere(minimo.estadosAntes == 5 && minimo.estadosDepois == 3 && m.completo(),
                    "/(a|b)*c/: 5 estados antes e 3 depois, contado o de erro (numeros da atividade)");
            confere(minimo.blocosPorRodada == std::vector<std::size_t>{2, 3, 3},
                    "particao 2 -> 3 -> 3: os dois ultimos valores iguais provam a parada");
            confere(m.representa(0) == std::vector<Estado>{0, 1, 2} && m.representa(1) == std::vector<Estado>{3} &&
                        m.representa(2) == std::vector<Estado>{4} && m.inicial() == 0 && m.ehDeAceitacao(1) &&
                        !m.ehDeAceitacao(2) && m.transicao(2, 'a') == 2u && m.transicao(2, 'b') == 2u &&
                        m.transicao(2, 'c') == 2u,
                    "M0 = {D0, D1, D2} (inicial), M1 = {D3} (aceita), M2 = {D4} (erro absorvente)");
        }

        // O exemplo da secao 1.6.2 do guia, escrito a mao: A..E sobre {0, 1}.
        {
            pinglu::AfdGerado afd(std::set<int>{'0', '1'});
            for (Estado estado = 0; estado < 5; ++estado)
            {
                afd.novoEstado({estado});
            }
            const Estado A = 0, B = 1, C = 2, D = 3, E = 4;
            afd.definirInicial(A);
            afd.marcarAceitacao(D);
            afd.marcarAceitacao(E);
            const auto ligar = [&](const Estado origem, const Estado com0, const Estado com1)
            {
                afd.adicionarTransicao(origem, '0', com0);
                afd.adicionarTransicao(origem, '1', com1);
            };
            ligar(A, B, C);
            ligar(B, D, E);
            ligar(C, D, E);
            ligar(D, D, D);
            ligar(E, E, E);
            const pinglu::ResultadoDaMinimizacao minimo = pinglu::minimizar(afd);
            std::cout << "exemplo 1.6.2 do guia (A..E sobre {0, 1}): " << minimo.estadosAntes << " -> "
                      << minimo.estadosDepois << ", rodadas " << comoTexto(minimo.blocosPorRodada) << '\n';
            confere(minimo.estadosDepois == 3 && minimo.blocosPorRodada == std::vector<std::size_t>{2, 3, 3} &&
                        minimo.afd.representa(0) == std::vector<Estado>{A} &&
                        minimo.afd.representa(1) == std::vector<Estado>{B, C} &&
                        minimo.afd.representa(2) == std::vector<Estado>{D, E},
                    "exemplo 1.6.2: blocos {A}, {B, C}, {D, E} e contagens 2, 3, 3, como no guia");
        }

        // Um AFD escrito a mao, incompleto e com um estado inacessivel: a
        // minimizacao completa, descarta o inacessivel e nao toca no original.
        {
            pinglu::AfdGerado afd(std::set<int>{'a', 'b'});
            const Estado q0 = afd.novoEstado({});
            const Estado q1 = afd.novoEstado({});
            const Estado ilha = afd.novoEstado({}); // nenhuma cadeia chega aqui
            afd.definirInicial(q0);
            afd.adicionarTransicao(q0, 'a', q1);
            afd.marcarAceitacao(q1);
            afd.marcarAceitacao(ilha);
            afd.adicionarTransicao(ilha, 'a', ilha);
            const pinglu::ResultadoDaMinimizacao minimo = pinglu::minimizar(afd);
            std::cout << "AFD incompleto com um estado inacessivel: " << minimo.estadosAntes << " estados, "
                      << minimo.estadosCompletado << " depois de completar, " << minimo.estadosAlcancaveis
                      << " alcancaveis, " << minimo.estadosDepois << " no minimo\n";
            confere(minimo.estadosAntes == 3 && minimo.estadosCompletado == 4 && minimo.estadosAlcancaveis == 3 &&
                        minimo.estadosDepois == 3 && minimo.afd.completo() && minimo.afd.aceita("a") &&
                        !minimo.afd.aceita("") && !minimo.afd.aceita("aa") && !minimo.afd.aceita("b"),
                    "completar ganha um estado de erro, o inacessivel e descartado, e a linguagem continua {a}");
            confere(afd.quantidadeDeEstados() == 3 && !afd.completo(),
                    "o AFD recebido fica intacto: a minimizacao trabalha sobre uma copia");
        }

        // As bordas: nenhum estado de aceitacao alcancavel, e AFD sem estados.
        {
            pinglu::AfdGerado nada(std::set<int>{'a'});
            const pinglu::ResultadoDaMinimizacao minimo = pinglu::minimizar(nada);
            pinglu::AfdGerado semFinal(std::set<int>{'a'});
            const Estado s0 = semFinal.novoEstado({});
            const Estado s1 = semFinal.novoEstado({});
            semFinal.adicionarTransicao(s0, 'a', s1);
            semFinal.adicionarTransicao(s1, 'a', s0);
            const pinglu::ResultadoDaMinimizacao umBloco = pinglu::minimizar(semFinal);
            confere(minimo.estadosDepois == 1 && !minimo.afd.aceita("") && umBloco.estadosDepois == 1 &&
                        umBloco.blocosPorRodada == std::vector<std::size_t>{1, 1},
                    "AFD sem estados e AFD sem aceitacao: o minimo e um estado so, que recusa tudo");
        }
    }

    // --- G: as tres maquinas, comparadas nos dois saltos ------------------------

    // O alfabeto do pattern mais um símbolo de fora dele, e o maior comprimento
    // que cabe em ~6000 cadeias. Como o Pinglu não tem coringa, as três
    // máquinas devem concordar também no símbolo de fora.
    std::vector<std::string> cadeiasDeTeste(const pinglu::Afn &afn)
    {
        std::string simbolos = pinglu::alfabetoComoTexto(afn.alfabeto());
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
        return pinglu::todasAsCadeias(simbolos, maximo);
    }

    void secaoTresMaquinas(const std::vector<pinglu::CasoDeTeste> &casos)
    {
        std::cout << "\n=== G as tres maquinas (AFN, AFD e minimo), comparadas nos dois saltos ===\n";

        // O exemplo da atividade: (a|b)*c sobre {a, b, c}, ate o comprimento 4.
        pinglu::Afn afn;
        if (construir("(a|b)*c", afn))
        {
            const pinglu::AfdGerado afd = pinglu::determinizar(afn);
            const pinglu::AfdGerado minimo = pinglu::minimizar(afd).afd;
            const pinglu::ComparacaoDasTres comparacao =
                pinglu::compararAsTres(afn, afd, minimo, pinglu::todasAsCadeias("abc", 4));
            std::cout << "/(a|b)*c/  " << comparacao.cadeias << " cadeias ate comprimento 4: AFN aceita "
                      << comparacao.aceitasPeloAfn << ", AFD aceita " << comparacao.aceitasPeloAfd
                      << ", minimo aceita " << comparacao.aceitasPeloMinimo << "; "
                      << pinglu::descreverComparacao(comparacao) << '\n';
            confere(comparacao.cadeias == 121 && comparacao.aceitasPeloAfn == 15 && comparacao.aceitasPeloAfd == 15 &&
                        comparacao.aceitasPeloMinimo == 15 && comparacao.concordam(),
                    "121 cadeias, e as tres maquinas aceitam as mesmas 15 (numeros da atividade)");

            // Entre dois AFDs completos com n1 e n2 estados, concordar em todas
            // as cadeias até n1 + n2 - 2 já é PROVA de equivalência (guia, 1.8.5).
            const std::size_t limite = afd.quantidadeDeEstados() + minimo.quantidadeDeEstados() - 2;
            const std::vector<std::string> todas = pinglu::todasAsCadeias("abc", limite);
            const pinglu::Discordancia d = pinglu::compararSobre(afd, minimo, todas);
            std::cout << "  AFD contra minimo ate o comprimento " << limite << " (= " << afd.quantidadeDeEstados()
                      << " + " << minimo.quantidadeDeEstados() << " - 2): " << todas.size() << " cadeias\n";
            confere(limite == 6 && todas.size() == 1093 && !d.houve,
                    "AFD e minimo concordam nas 1093 cadeias ate o comprimento 6: prova, e nao so evidencia");
        }

        // Todos os patterns da bateria e mais alguns: os dois saltos, a parada
        // certa do refinamento e a idempotencia (minimizar o minimo nao muda nada).
        std::set<std::string> padroes;
        for (const pinglu::CasoDeTeste &caso : casos)
        {
            padroes.insert(caso.expressao);
        }
        padroes.insert({"(a|b)*c", "(a|b)*abb", "a(b|c)*d", "(a*)*", "a**", "((a|b)*|c)+d?", "-?[0-9]+"});

        std::cout << "  estados antes e depois da minimizacao, pattern a pattern:\n";
        std::size_t conferidos = 0;
        bool tudoCerto = true;
        for (const std::string &padrao : padroes)
        {
            pinglu::Afn maquina;
            if (!construir(padrao, maquina))
            {
                tudoCerto = false;
                continue;
            }
            ++conferidos;
            const pinglu::AfdGerado afd = pinglu::determinizar(maquina);
            const pinglu::ResultadoDaMinimizacao minimo = pinglu::minimizar(afd);
            const std::vector<std::size_t> &rodadas = minimo.blocosPorRodada;
            std::string nome = "/" + padrao + "/";
            nome.resize(std::max<std::size_t>(nome.size(), 26), ' ');
            std::cout << "    " << nome << " AFN " << maquina.quantidadeDeEstados() << " -> AFD "
                      << minimo.estadosAntes << " -> minimo " << minimo.estadosDepois << "   (rodadas "
                      << comoTexto(rodadas) << ")\n";

            std::string problema;
            if (rodadas.size() < 2 || rodadas[rodadas.size() - 1] != rodadas[rodadas.size() - 2])
            {
                problema = "o refinamento parou antes do ponto fixo";
            }
            else if (!std::is_sorted(rodadas.begin(), rodadas.end()) || rodadas.back() != minimo.estadosDepois)
            {
                problema = "a contagem de blocos diminuiu ou nao bate com o minimo";
            }
            else if (minimo.estadosDepois > minimo.estadosAlcancaveis || !minimo.afd.completo())
            {
                problema = "o minimo e maior que o AFD ou esta incompleto";
            }
            else if (pinglu::minimizar(minimo.afd).estadosDepois != minimo.estadosDepois)
            {
                problema = "minimizar o minimo mudou o numero de estados";
            }
            else
            {
                const pinglu::ComparacaoDasTres comparacao =
                    pinglu::compararAsTres(maquina, afd, minimo.afd, cadeiasDeTeste(maquina));
                problema = comparacao.concordam() ? "" : pinglu::descreverComparacao(comparacao);
            }
            if (!problema.empty())
            {
                tudoCerto = false;
                confere(false, "/" + padrao + "/ - " + problema);
            }
        }
        confere(tudoCerto && conferidos > 0,
                std::to_string(conferidos) +
                    " patterns: refinamento ate o ponto fixo, minimo completo e idempotente, e AFN, AFD e minimo "
                    "concordam nos dois saltos em todas as cadeias curtas");
    }

    // --- H: a explosao de estados, construida de proposito ----------------------

    // A linguagem das cadeias sobre {a, b} cujo n-esimo simbolo contado do fim
    // e um a. O AFN cresce de 6 em 6; um AFD precisa lembrar os ultimos n
    // simbolos, e isso exige 2^n estados.
    void secaoExplosao()
    {
        std::cout << "\n=== H a explosao de estados: o n-esimo simbolo a partir do fim e um a ===\n"
                  << "pattern: (a|b)*a(a|b)...(a|b), com n - 1 copias de (a|b)\n\n"
                  << "   n |  AFN |  AFD | minimo |  2^n | rodadas | cadeias comparadas\n"
                  << "  ---+------+------+--------+------+---------+-------------------\n";
        bool tudoCerto = true;
        for (std::size_t n = 1; n <= 8; ++n)
        {
            std::string padrao = "(a|b)*a";
            for (std::size_t i = 1; i < n; ++i)
            {
                padrao += "(a|b)";
            }
            pinglu::Afn afn;
            if (!construir(padrao, afn))
            {
                tudoCerto = false;
                continue;
            }
            const pinglu::AfdGerado afd = pinglu::determinizar(afn);
            const pinglu::ResultadoDaMinimizacao minimo = pinglu::minimizar(afd);
            const std::size_t potencia = std::size_t{1} << n;
            const pinglu::ComparacaoDasTres comparacao =
                pinglu::compararAsTres(afn, afd, minimo.afd, pinglu::todasAsCadeias("ab", n + 4));

            const auto coluna = [](const std::size_t valor, const std::size_t largura)
            {
                std::string texto = std::to_string(valor);
                return std::string(largura > texto.size() ? largura - texto.size() : 0, ' ') + texto;
            };
            std::cout << "  " << coluna(n, 2) << " | " << coluna(afn.quantidadeDeEstados(), 4) << " | "
                      << coluna(afd.quantidadeDeEstados(), 4) << " | " << coluna(minimo.estadosDepois, 6) << " | "
                      << coluna(potencia, 4) << " | " << coluna(minimo.blocosPorRodada.size() - 1, 7) << " | "
                      << comparacao.cadeias << (comparacao.concordam() ? ", concordam" : ", DIVERGEM") << '\n';

            tudoCerto = tudoCerto && afn.quantidadeDeEstados() == 6 * n + 4 &&
                        afd.quantidadeDeEstados() == potencia + 1 && minimo.estadosDepois == potencia &&
                        comparacao.concordam();
        }
        std::cout << "\n  O AFD tem 2^n + 1 estados: o inicial (o fecho do inicial do AFN) e equivalente ao\n"
                     "  estado de \"so bs nos ultimos n simbolos\", e a minimizacao o funde. O vazio nunca\n"
                     "  aparece (toda cadeia pode ser prefixo de uma aceita), entao nao ha estado de erro.\n"
                     "  O minimo dobra junto com o AFD: a explosao e exigencia da linguagem, e nao\n"
                     "  desperdicio da construcao de subconjuntos.\n";
        confere(tudoCerto, "n = 1..8: AFN com 6n + 4 estados, AFD com 2^n + 1, minimo com exatamente 2^n, "
                           "e as tres maquinas concordam");
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
        secaoMinimizacao();
        secaoTresMaquinas(leitura.casos);
        secaoExplosao();

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
