#include "05_minimizacao.h"

#include <map>
#include <queue>
#include <utility>

namespace pinglu
{

    // --- completar e alcançar ----------------------------------------------------

    AfdGerado completar(const AfdGerado &afd)
    {
        AfdGerado completo = afd;
        if (completo.completo())
        {
            return completo;
        }
        // O estado de erro representa o conjunto vazio, como na determinização:
        // não aceita, e todas as suas transições voltam a ele mesmo. O laço vai
        // até `erro` inclusive justamente para ligar as dele.
        const Estado erro = completo.novoEstado({});
        for (Estado estado = 0; estado <= erro; ++estado)
        {
            for (const int rotulo : completo.alfabeto())
            {
                if (!completo.transicao(estado, rotulo))
                {
                    completo.adicionarTransicao(estado, rotulo, erro);
                }
            }
        }
        return completo;
    }

    std::set<Estado> estadosAlcancaveis(const AfdGerado &afd)
    {
        std::set<Estado> visitados;
        if (afd.quantidadeDeEstados() == 0)
        {
            return visitados;
        }
        std::queue<Estado> fila;
        visitados.insert(afd.inicial());
        fila.push(afd.inicial());
        while (!fila.empty())
        {
            const Estado atual = fila.front();
            fila.pop();
            for (const int rotulo : afd.alfabeto())
            {
                const std::optional<Estado> destino = afd.transicao(atual, rotulo);
                if (destino && visitados.insert(*destino).second)
                {
                    fila.push(*destino);
                }
            }
        }
        return visitados;
    }

    // --- o refinamento de partições ---------------------------------------------

    namespace
    {

        constexpr std::size_t kForaDaParticao = static_cast<std::size_t>(-1);

        // Agrupa os estados pela chave: estados com a mesma chave caem no mesmo
        // bloco. Os blocos são numerados pela ordem de primeira aparição, e
        // `estados` vem em ordem crescente: o bloco do menor estado é o 0.
        template <typename Chave>
        std::size_t particionar(const std::vector<Estado> &estados, std::vector<std::size_t> &blocoDe,
                                const Chave &chaveDe)
        {
            std::map<std::vector<std::size_t>, std::size_t> numeroDaChave;
            std::vector<std::size_t> novo(blocoDe.size(), kForaDaParticao);
            for (const Estado estado : estados)
            {
                const auto [posicao, inseriu] = numeroDaChave.emplace(chaveDe(estado), numeroDaChave.size());
                (void)inseriu;
                novo[estado] = posicao->second;
            }
            blocoDe = std::move(novo);
            return numeroDaChave.size();
        }

    } // namespace

    // recorte:inicio minimizacao-por-particoes
    ResultadoDaMinimizacao minimizar(const AfdGerado &original)
    {
        const std::size_t estadosAntes = original.quantidadeDeEstados();

        // Um AFD sem estados não aceita nada: o mínimo é só o estado de erro.
        if (estadosAntes == 0)
        {
            AfdGerado soErro(original.alfabeto());
            const Estado erro = soErro.novoEstado({});
            soErro.definirInicial(erro);
            for (const int rotulo : soErro.alfabeto())
            {
                soErro.adicionarTransicao(erro, rotulo, erro);
            }
            return ResultadoDaMinimizacao{std::move(soErro), 0, 1, 1, 1, {1, 1}};
        }

        // 1. completar: o refinamento supõe destino para todo par (estado, símbolo).
        const AfdGerado afd = completar(original);

        // 2. descartar os inacessíveis: o refinamento só olha os alcançáveis.
        const std::set<Estado> alcancaveis = estadosAlcancaveis(afd);
        const std::vector<Estado> estados(alcancaveis.begin(), alcancaveis.end());

        // 3. a partição inicial: a cadeia vazia já separa quem aceita de quem não.
        std::vector<std::size_t> blocoDe(afd.quantidadeDeEstados(), kForaDaParticao);
        std::size_t quantidadeDeBlocos = particionar(estados, blocoDe, [&](const Estado estado)
                                                     { return std::vector<std::size_t>{afd.ehDeAceitacao(estado) ? 1u : 0u}; });
        std::vector<std::size_t> blocosPorRodada{quantidadeDeBlocos};

        // 4. as rodadas. A assinatura é o bloco atual seguido do bloco de destino
        // de cada símbolo, toda calculada contra a MESMA partição (a cópia
        // `anterior`), que não muda durante a rodada.
        while (true)
        {
            const std::vector<std::size_t> anterior = blocoDe;
            const std::size_t novosBlocos = particionar(estados, blocoDe, [&](const Estado estado)
                                                        {
                                                            std::vector<std::size_t> assinatura{anterior[estado]};
                                                            for (const int rotulo : afd.alfabeto())
                                                            {
                                                                assinatura.push_back(anterior[afd.transicao(estado, rotulo).value()]);
                                                            }
                                                            return assinatura; });
            blocosPorRodada.push_back(novosBlocos);
            // Ponto fixo: a rodada não separou ninguém. Parar ao atingir um
            // número "esperado" de blocos, em vez disso, pode deixar estados
            // distinguíveis juntos sem que nenhuma cadeia acuse.
            const bool pontoFixo = novosBlocos == quantidadeDeBlocos;
            quantidadeDeBlocos = novosBlocos;
            if (pontoFixo)
            {
                break;
            }
        }

        // 5. a reconstrução: um estado por bloco, que representa os estados do
        // AFD fundidos nele. Basta um representante por bloco: no ponto fixo,
        // todos os estados de um bloco vão, com cada símbolo, ao mesmo bloco.
        std::vector<std::vector<Estado>> membros(quantidadeDeBlocos);
        for (const Estado estado : estados)
        {
            membros[blocoDe[estado]].push_back(estado);
        }
        AfdGerado minimo(afd.alfabeto());
        for (const std::vector<Estado> &bloco : membros)
        {
            minimo.novoEstado(bloco);
        }
        minimo.definirInicial(blocoDe[afd.inicial()]);
        for (Estado novo = 0; novo < membros.size(); ++novo)
        {
            const Estado representante = membros[novo].front();
            if (afd.ehDeAceitacao(representante))
            {
                minimo.marcarAceitacao(novo);
            }
            for (const int rotulo : afd.alfabeto())
            {
                minimo.adicionarTransicao(novo, rotulo, blocoDe[afd.transicao(representante, rotulo).value()]);
            }
        }

        const std::size_t estadosDepois = minimo.quantidadeDeEstados();
        return ResultadoDaMinimizacao{std::move(minimo), estadosAntes, afd.quantidadeDeEstados(),
                                      alcancaveis.size(), estadosDepois, std::move(blocosPorRodada)};
    }
    // recorte:fim minimizacao-por-particoes

    std::string formatarRodadas(const std::vector<std::size_t> &blocosPorRodada)
    {
        std::string texto;
        for (std::size_t i = 0; i < blocosPorRodada.size(); ++i)
        {
            texto += (i > 0 ? " -> " : "") + std::to_string(blocosPorRodada[i]);
        }
        return texto;
    }

    // --- a comparação exaustiva -------------------------------------------------

    std::string alfabetoComoTexto(const std::set<int> &alfabeto)
    {
        std::string texto;
        for (const int rotulo : alfabeto)
        {
            texto += static_cast<char>(rotulo);
        }
        return texto;
    }

    std::vector<std::string> todasAsCadeias(const std::string &simbolos, const std::size_t comprimentoMaximo)
    {
        std::vector<std::string> cadeias{""};
        std::size_t inicioDoNivel = 0;
        for (std::size_t comprimento = 1; comprimento <= comprimentoMaximo && !simbolos.empty(); ++comprimento)
        {
            const std::size_t fimDoNivel = cadeias.size();
            for (std::size_t i = inicioDoNivel; i < fimDoNivel; ++i)
            {
                for (const char simbolo : simbolos)
                {
                    cadeias.push_back(cadeias[i] + simbolo);
                }
            }
            inicioDoNivel = fimDoNivel;
        }
        return cadeias;
    }

    namespace
    {

        template <typename Primeira, typename Segunda>
        Discordancia primeiraDiscordancia(const Primeira &primeira, const Segunda &segunda,
                                          const std::vector<std::string> &cadeias)
        {
            for (const std::string &cadeia : cadeias)
            {
                const bool porPrimeira = primeira.aceita(cadeia);
                const bool porSegunda = segunda.aceita(cadeia);
                if (porPrimeira != porSegunda)
                {
                    return Discordancia{true, cadeia, porPrimeira, porSegunda};
                }
            }
            return Discordancia{};
        }

    } // namespace

    Discordancia compararSobre(const Afn &afn, const AfdGerado &afd, const std::vector<std::string> &cadeias)
    {
        return primeiraDiscordancia(afn, afd, cadeias);
    }

    Discordancia compararSobre(const AfdGerado &primeira, const AfdGerado &segunda,
                               const std::vector<std::string> &cadeias)
    {
        return primeiraDiscordancia(primeira, segunda, cadeias);
    }

    ComparacaoDasTres compararAsTres(const Afn &afn, const AfdGerado &afd, const AfdGerado &minimo,
                                     const std::vector<std::string> &cadeias)
    {
        ComparacaoDasTres resultado;
        resultado.cadeias = cadeias.size();
        for (const std::string &cadeia : cadeias)
        {
            const bool porAfn = afn.aceita(cadeia);
            const bool porAfd = afd.aceita(cadeia);
            const bool porMinimo = minimo.aceita(cadeia);
            resultado.aceitasPeloAfn += porAfn ? 1 : 0;
            resultado.aceitasPeloAfd += porAfd ? 1 : 0;
            resultado.aceitasPeloMinimo += porMinimo ? 1 : 0;
            // Os dois saltos são conferidos sempre, cada um com a sua primeira
            // discordância: um não esconde o outro.
            if (!resultado.afnContraAfd.houve && porAfn != porAfd)
            {
                resultado.afnContraAfd = Discordancia{true, cadeia, porAfn, porAfd};
            }
            if (!resultado.afdContraMinimo.houve && porAfd != porMinimo)
            {
                resultado.afdContraMinimo = Discordancia{true, cadeia, porAfd, porMinimo};
            }
        }
        return resultado;
    }

    std::string descreverComparacao(const ComparacaoDasTres &comparacao)
    {
        const auto veredito = [](const bool aceitou)
        { return aceitou ? "aceita" : "recusa"; };
        if (comparacao.afnContraAfd.houve)
        {
            const Discordancia &d = comparacao.afnContraAfd;
            return "AFN != AFD em \"" + d.cadeia + "\" (AFN " + veredito(d.primeiraAceitou) + ", AFD " +
                   veredito(d.segundaAceitou) + ")";
        }
        if (comparacao.afdContraMinimo.houve)
        {
            const Discordancia &d = comparacao.afdContraMinimo;
            return "AFD != minimo em \"" + d.cadeia + "\" (AFD " + veredito(d.primeiraAceitou) + ", minimo " +
                   veredito(d.segundaAceitou) + ")";
        }
        return "as tres maquinas concordam em todas";
    }

} // namespace pinglu
