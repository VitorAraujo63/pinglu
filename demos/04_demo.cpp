// 04_demo.cpp — a demonstração do marco 04: a construção de Thompson, a
// simulação por conjunto de estados e a bateria conferida à mão.
//
// Também é o teste do módulo: sai com código 1 se qualquer verificação
// divergir, e é isso que `make verificar` usa para acusar regressão. A bateria
// (exemplos/07_casos_afn.txt) é a evidência escrita à mão; as demais seções
// conferem o que a bateria sozinha não alcança.
//
// Precisa rodar da raiz do repositório (o `make` já faz isso): o caminho da
// bateria é relativo.

#include "02_regex_pinglu.h"
#include "03_afd.h"
#include "04_afn.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <set>
#include <sstream>
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

    std::vector<Estado> ordenado(std::vector<Estado> conjunto)
    {
        std::sort(conjunto.begin(), conjunto.end());
        return conjunto;
    }

    // O custo que a árvore prevê, contado DIRETAMENTE nos nós, sem usar a
    // construção: é a tabela de custos do módulo escrita como código. Folha: 2
    // estados e 1 transição (vazia, se for a folha de cadeia vazia);
    // concatenação: 0 estados e 1 transição vazia; alternância e fecho: 2
    // estados e 4 transições vazias.
    struct Custo
    {
        std::size_t estados = 0;
        std::size_t transicoes = 0;
        std::size_t vazias = 0;
        std::size_t nos = 0;
    };

    void somarCusto(const pinglu::Arvore &arvore, const std::size_t indice, Custo &custo)
    {
        const pinglu::No &no = arvore.nos[indice];
        ++custo.nos;
        switch (no.tipo)
        {
        case pinglu::TipoDeNo::Simbolo:
            custo.estados += 2;
            custo.transicoes += 1;
            break;
        case pinglu::TipoDeNo::Vazio:
            custo.estados += 2;
            custo.transicoes += 1;
            custo.vazias += 1;
            break;
        case pinglu::TipoDeNo::Concatenacao:
            custo.transicoes += 1;
            custo.vazias += 1;
            somarCusto(arvore, no.esquerda, custo);
            somarCusto(arvore, no.direita, custo);
            break;
        case pinglu::TipoDeNo::Alternancia:
            custo.estados += 2;
            custo.transicoes += 4;
            custo.vazias += 4;
            somarCusto(arvore, no.esquerda, custo);
            somarCusto(arvore, no.direita, custo);
            break;
        case pinglu::TipoDeNo::Fecho:
            custo.estados += 2;
            custo.transicoes += 4;
            custo.vazias += 4;
            somarCusto(arvore, no.esquerda, custo);
            break;
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

    // --- A: a construção, caso a caso ------------------------------------------

    struct CasoDeConstrucao
    {
        std::string padrao;
        std::string cadeia;
        std::size_t estados;
        std::size_t transicoes;
        std::size_t vazias;
        std::vector<std::vector<Estado>> conjuntos; // um por passo, da partida em diante
        bool aceita;
    };

    void secaoConstrucao()
    {
        // Os números e os conjuntos de cada caso foram calculados NO PAPEL, pela
        // tabela de custos e pela numeração em ordem de visita (filho esquerdo,
        // filho direito, estados do próprio nó): ver o material do módulo 04.
        const std::vector<CasoDeConstrucao> casos = {
            {"ab", "ab", 4, 3, 1, {{0}, {1, 2}, {3}}, true},
            {"a|b", "b", 6, 6, 4, {{4, 0, 2}, {3, 5}}, true},
            {"a*", "aa", 4, 5, 4, {{2, 0, 3}, {1, 0, 3}, {1, 0, 3}}, true},
            {"(a|b)*c", "abc", 10, 12, 9, {{6, 4, 7, 8, 0, 2}, {1, 5, 4, 7, 8, 0, 2}, {3, 5, 4, 7, 8, 0, 2}, {9}}, true},
        };

        std::cout << "\n=== A construcao, caso a caso, com o traco dos estados ativos ===\n";
        for (const CasoDeConstrucao &caso : casos)
        {
            const pinglu::Resultado leitura = pinglu::analisarExpressao(caso.padrao);
            if (!leitura.ok)
            {
                confere(false, "/" + caso.padrao + "/ foi recusado pela leitura");
                continue;
            }
            const pinglu::Afn afn = pinglu::construirThompson(leitura.arvore);
            const pinglu::Simulacao simulacao = afn.simular(caso.cadeia);

            std::cout << "\n/" << caso.padrao << "/ -> " << afn.quantidadeDeEstados() << " estados, "
                      << afn.quantidadeDeTransicoes() << " transicoes ("
                      << afn.quantidadeDeTransicoesVazias() << " vazias)\n"
                      << afn.formatarSimulacao(caso.cadeia, simulacao);

            bool conjuntosBatem = simulacao.passos.size() == caso.conjuntos.size();
            for (std::size_t i = 0; conjuntosBatem && i < caso.conjuntos.size(); ++i)
            {
                conjuntosBatem = ordenado(simulacao.passos[i].ativos) == ordenado(caso.conjuntos[i]);
            }
            const bool contagensBatem = afn.quantidadeDeEstados() == caso.estados &&
                                        afn.quantidadeDeTransicoes() == caso.transicoes &&
                                        afn.quantidadeDeTransicoesVazias() == caso.vazias;
            confere(contagensBatem && conjuntosBatem && simulacao.aceitou == caso.aceita,
                    "contagens e conjuntos ativos conferem com a conta feita a mao");
        }
    }

    // --- B: o custo previsto e os invariantes estruturais ----------------------

    void secaoCusto(const std::vector<pinglu::CasoDeTeste> &casos)
    {
        std::set<std::string> padroes;
        for (const pinglu::CasoDeTeste &caso : casos)
        {
            padroes.insert(caso.expressao);
        }
        padroes.insert({"(a|b)*c", "a|b", "ab|ac", "((a|b)*|c)+d?"});

        std::cout << "\n=== B o custo previsto pela arvore e os invariantes da construcao ===\n";
        std::size_t conferidos = 0;
        bool tudoCerto = true;
        for (const std::string &padrao : padroes)
        {
            const pinglu::Resultado leitura = pinglu::analisarExpressao(padrao);
            if (!leitura.ok)
            {
                continue; // a bateria reporta o pattern recusado
            }
            ++conferidos;
            Custo previsto;
            somarCusto(leitura.arvore, leitura.arvore.raiz, previsto);
            const pinglu::Afn afn = pinglu::construirThompson(leitura.arvore);

            std::string problema;
            if (afn.quantidadeDeEstados() != previsto.estados)
            {
                problema = "estados: previsto " + std::to_string(previsto.estados) + ", construido " +
                           std::to_string(afn.quantidadeDeEstados());
            }
            else if (afn.quantidadeDeTransicoes() != previsto.transicoes)
            {
                problema = "transicoes: previsto " + std::to_string(previsto.transicoes) +
                           ", construido " + std::to_string(afn.quantidadeDeTransicoes());
            }
            else if (afn.quantidadeDeTransicoesVazias() != previsto.vazias)
            {
                problema = "vazias: previsto " + std::to_string(previsto.vazias) + ", construido " +
                           std::to_string(afn.quantidadeDeTransicoesVazias());
            }
            else if (afn.quantidadeDeEstados() > 2 * previsto.nos ||
                     afn.quantidadeDeTransicoes() > 4 * previsto.nos)
            {
                problema = "passa do teto de 2 estados e 4 transicoes por no";
            }
            else if (!afn.estadosSemChegada().empty())
            {
                problema = "ha estado sem nenhuma transicao chegando a ele";
            }
            if (!problema.empty())
            {
                tudoCerto = false;
                confere(false, "/" + padrao + "/ - " + problema);
            }
        }
        confere(tudoCerto && conferidos > 0,
                std::to_string(conferidos) +
                    " patterns distintos: estados, transicoes e vazias batem com a tabela de custos, "
                    "ficam sob o teto linear e nenhum estado fica sem chegada");
    }

    // --- C: as bordas ----------------------------------------------------------

    void secaoBordas()
    {
        std::cout << "\n=== C as bordas ===\n";

        // O byte zero na entrada nao e transicao vazia: as maquinas de texto do
        // Pinglu aceitam qualquer byte, inclusive ele.
        const pinglu::Afn estrela = pinglu::construirThompson(pinglu::analisarExpressao("a*").arvore);
        const std::string zero(1, '\0');
        confere(estrela.aceita("") && !estrela.aceita(zero) && !estrela.aceita(std::string("a\0a", 3)),
                "um byte zero na entrada nao e confundido com transicao vazia (a* recusa \\0)");

        // E um byte zero DENTRO do pattern e um simbolo como outro qualquer.
        const std::string comZero("a\0b", 3);
        const pinglu::Resultado leituraZero = pinglu::analisarExpressao(comZero);
        bool zeroNoPattern = leituraZero.ok;
        if (zeroNoPattern)
        {
            const pinglu::Afn afn = pinglu::construirThompson(leituraZero.arvore);
            zeroNoPattern = afn.aceita(comZero) && !afn.aceita("ab") && !afn.aceita(std::string("a\0", 2));
        }
        confere(zeroNoPattern, "um byte zero no pattern e um simbolo literal (a\\0b aceita so a\\0b)");

        // Bytes acima de 0x7F (UTF-8) sao simbolos comuns. O `char` do C++ e com
        // sinal: sem a conversao para `unsigned char` na construcao e na
        // simulacao, o `ã` viraria um rotulo negativo.
        const std::string acentuado = "n\xc3\xa3o";
        const pinglu::Resultado leituraAcento = pinglu::analisarExpressao(acentuado);
        bool acento = leituraAcento.ok;
        if (acento)
        {
            const pinglu::Afn afn = pinglu::construirThompson(leituraAcento.arvore);
            acento = afn.aceita(acentuado) && !afn.aceita("nao") && !afn.aceita("n\xc3o");
        }
        confere(acento, "bytes acima de 0x7F sao simbolos comuns (n\\xc3\\xa3o aceita so n\\xc3\\xa3o)");

        // Arvore vazia: a linguagem {epsilon}, e nao a linguagem vazia.
        const pinglu::Afn vazia = pinglu::construirThompson(pinglu::Arvore{});
        confere(vazia.quantidadeDeEstados() == 1 && vazia.aceita("") && !vazia.aceita("a"),
                "arvore vazia: uma maquina de um estado que aceita so a cadeia vazia");

        // Montar errado lanca; simular nunca lanca.
        pinglu::Afn pequena;
        const Estado unico = pequena.novoEstado();
        confere(lanca([&]
                      { pequena.adicionarTransicao(unico, 'a', 7); }) &&
                    lanca([&]
                          { pequena.adicionarTransicao(unico, 300, unico); }) &&
                    lanca([&]
                          { pequena.definirInicial(5); }),
                "montar a maquina com estado ou rotulo invalido lanca invalid_argument");

        pinglu::Arvore malformada;
        malformada.nos.push_back(pinglu::No{pinglu::TipoDeNo::Concatenacao, '\0', pinglu::kSemFilho,
                                            pinglu::kSemFilho});
        malformada.raiz = 0;
        confere(lanca([&]
                      { (void)pinglu::construirThompson(malformada); }),
                "arvore malformada (filho que nao existe) lanca em vez de indexar fora do vetor");

        const pinglu::Afn sem_estados;
        confere(!sem_estados.aceita("") && !sem_estados.aceita("a"),
                "maquina sem nenhum estado recusa tudo, sem lancar");

        // O leitor de casos e estrito: linha malformada reprova, nao some.
        std::istringstream fluxo(
            "# comentario\n"
            "\n"
            "a\ta\taceita\r\n"    // fim de linha CRLF e aceito
            "b\tb\taceta\n"       // veredito com erro de digitacao
            "c\tc\n"              // faltou um campo
            "d\td\taceita\tx\n"); // sobrou um campo
        const pinglu::LeituraDeCasos leitura = pinglu::interpretarCasos(fluxo, "teste");
        confere(leitura.casos.size() == 1 && leitura.casos[0].esperado && leitura.erros.size() == 3 &&
                    leitura.erros[0].rfind("teste:4", 0) == 0 && leitura.erros[1].rfind("teste:5", 0) == 0 &&
                    leitura.erros[2].rfind("teste:6", 0) == 0,
                "o leitor aceita CRLF e acusa, com a linha, o veredito invalido, o campo que falta e o que sobra");
        confere(!pinglu::lerCasos("exemplos/nao_existe.txt").erros.empty(),
                "arquivo de casos que nao abre e erro, e nao uma bateria vazia que passa");
    }

    // --- D: equivalencia com as maquinas feitas a mao (modulo 03) --------------

    // Compara, em TODAS as cadeias de ate `maximo` simbolos sobre `alfabeto`, a
    // resposta da maquina feita a mao com a da maquina derivada do pattern.
    // Nao e prova de equivalencia (a prova e a determinizacao e a minimizacao),
    // mas duas descricoes independentes que concordam em milhares de cadeias,
    // inclusive nas recusadas, e evidencia que nenhuma das duas sozinha da.
    void secaoEquivalencia(const std::string &nome, const pinglu::Afd &feita,
                           const std::string &padrao, const std::string &alfabeto,
                           const std::size_t maximo)
    {
        const pinglu::Resultado leitura = pinglu::analisarExpressao(padrao);
        if (!leitura.ok)
        {
            confere(false, nome + ": o pattern foi recusado pela leitura");
            return;
        }
        const pinglu::Afn derivada = pinglu::construirThompson(leitura.arvore);
        std::cout << "  " << nome << ": arvore de " << pinglu::tamanho(leitura.arvore) << " nos -> "
                  << derivada.quantidadeDeEstados() << " estados, " << derivada.quantidadeDeTransicoes()
                  << " transicoes (" << derivada.quantidadeDeTransicoesVazias()
                  << " vazias); a maquina feita a mao tem " << feita.estadosUteis() << " estados uteis\n";

        std::size_t total = 0;
        std::size_t aceitas = 0;
        std::string primeiraDivergencia;
        std::function<void(const std::string &)> gerar = [&](const std::string &cadeia)
        {
            ++total;
            const bool a = feita.aceita(cadeia);
            const bool b = derivada.aceita(cadeia);
            aceitas += a ? 1 : 0;
            if (a != b && primeiraDivergencia.empty())
            {
                primeiraDivergencia = "\"" + cadeia + "\" (feita a mao: " + (a ? "aceita" : "recusa") +
                                      ", derivada do pattern: " + (b ? "aceita" : "recusa") + ")";
            }
            if (cadeia.size() < maximo)
            {
                for (const char simbolo : alfabeto)
                {
                    gerar(cadeia + simbolo);
                }
            }
        };
        gerar("");

        confere(primeiraDivergencia.empty() && aceitas > 0,
                nome + ": " + std::to_string(total) + " cadeias (" + std::to_string(aceitas) +
                    " aceitas), a maquina do modulo 03 e a derivada do pattern nunca divergem" +
                    (primeiraDivergencia.empty() ? "" : "; primeira divergencia: " + primeiraDivergencia));
    }

    void secaoEquivalencias()
    {
        std::cout << "\n=== D a maquina feita a mao (modulo 03) contra a derivada do pattern (modulo 04) ===\n";
        secaoEquivalencia("identificador", pinglu::afdIdentificador(), "[a-zA-Z_][a-zA-Z0-9_]*", "aZ_7-", 5);
        secaoEquivalencia("numero", pinglu::afdNumero(), "[0-9]+(\\.[0-9]+)?", "05.-", 6);
        secaoEquivalencia("operadores_e_delimitadores", pinglu::afdOperadoresEDelimitadores(),
                          "\\+|\\*|/|%|\\(|\\)|\\{|\\}|\\[|\\]|,|;|=|<|>|!|-|==|!=|<=|>=|->|&&|\\|\\|",
                          "+*/%(){}[],;=<>!-&|a", 3);
    }

    // --- E: a bateria conferida a mao ------------------------------------------

    void secaoBateria(const pinglu::LeituraDeCasos &leitura)
    {
        std::cout << "\n=== E a bateria conferida a mao ===\n";
        for (const std::string &erro : leitura.erros)
        {
            confere(false, "arquivo de casos: " + erro);
        }
        if (leitura.casos.empty())
        {
            confere(false, "nenhum caso lido: uma bateria vazia passaria sem verificar nada");
            return;
        }

        std::size_t aceitas = 0;
        for (const pinglu::CasoDeTeste &caso : leitura.casos)
        {
            aceitas += caso.esperado ? 1 : 0;
        }
        const pinglu::ResultadoDaBateria resultado = pinglu::rodarBateria(leitura.casos);
        for (const std::string &falha : resultado.falhas)
        {
            confere(false, falha);
        }
        std::cout << "Bateria conferida a mao: " << resultado.passaram << "/" << resultado.total
                  << " casos passaram (" << aceitas << " de aceitacao, " << resultado.total - aceitas
                  << " de recusa).\n";
        if (!resultado.falhas.empty() || resultado.passaram != resultado.total)
        {
            ++g_falhas;
        }
    }

} // namespace

int main()
{
    try
    {
        const pinglu::LeituraDeCasos leitura = pinglu::lerCasos("exemplos/07_casos_afn.txt");

        secaoConstrucao();
        secaoCusto(leitura.casos);
        secaoBordas();
        secaoEquivalencias();
        secaoBateria(leitura);

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
