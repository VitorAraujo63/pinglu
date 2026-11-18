#include "05_determinizacao.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace pinglu
{

    // --- o AFD gerado ------------------------------------------------------------

    AfdGerado::AfdGerado(std::set<int> alfabeto) : alfabeto_(std::move(alfabeto))
    {
        for (const int rotulo : alfabeto_)
        {
            if (rotulo < 0 || rotulo > 255)
            {
                throw std::invalid_argument("AfdGerado: o alfabeto so tem bytes (0..255).");
            }
        }
    }

    void AfdGerado::exigirEstado(const Estado estado, const char *quem) const
    {
        if (estado >= transicoes_.size())
        {
            throw std::invalid_argument(std::string("AfdGerado::") + quem + ": estado inexistente.");
        }
    }

    Estado AfdGerado::novoEstado(std::vector<Estado> representa)
    {
        transicoes_.emplace_back();
        representa_.push_back(std::move(representa));
        return transicoes_.size() - 1;
    }

    void AfdGerado::adicionarTransicao(const Estado origem, const int rotulo, const Estado destino)
    {
        exigirEstado(origem, "adicionarTransicao");
        exigirEstado(destino, "adicionarTransicao");
        if (!alfabeto_.contains(rotulo))
        {
            // Inclui kEpsilon: um AFD não tem transição vazia.
            throw std::invalid_argument("AfdGerado::adicionarTransicao: rotulo fora do alfabeto.");
        }
        transicoes_[origem][rotulo] = destino;
    }

    void AfdGerado::definirInicial(const Estado estado)
    {
        exigirEstado(estado, "definirInicial");
        inicial_ = estado;
    }

    void AfdGerado::marcarAceitacao(const Estado estado)
    {
        exigirEstado(estado, "marcarAceitacao");
        finais_.insert(estado);
    }

    Estado AfdGerado::inicial() const { return inicial_; }

    const std::set<int> &AfdGerado::alfabeto() const { return alfabeto_; }

    const std::set<Estado> &AfdGerado::finais() const { return finais_; }

    bool AfdGerado::ehDeAceitacao(const Estado estado) const { return finais_.contains(estado); }

    const std::vector<Estado> &AfdGerado::representa(const Estado estado) const
    {
        exigirEstado(estado, "representa");
        return representa_[estado];
    }

    std::optional<Estado> AfdGerado::transicao(const Estado origem, const int rotulo) const
    {
        exigirEstado(origem, "transicao");
        const auto encontrado = transicoes_[origem].find(rotulo);
        if (encontrado == transicoes_[origem].end())
        {
            return std::nullopt;
        }
        return encontrado->second;
    }

    std::size_t AfdGerado::quantidadeDeEstados() const { return transicoes_.size(); }

    std::size_t AfdGerado::quantidadeDeTransicoes() const
    {
        std::size_t total = 0;
        for (const std::map<int, Estado> &saidas : transicoes_)
        {
            total += saidas.size();
        }
        return total;
    }

    bool AfdGerado::completo() const
    {
        for (const std::map<int, Estado> &saidas : transicoes_)
        {
            if (saidas.size() != alfabeto_.size())
            {
                return false; // adicionarTransicao só aceita rótulos do alfabeto
            }
        }
        return true;
    }

    bool AfdGerado::aceita(const std::string &cadeia) const
    {
        if (transicoes_.empty())
        {
            return false;
        }
        Estado atual = inicial_;
        for (const char simbolo : cadeia)
        {
            const auto encontrado = transicoes_[atual].find(static_cast<unsigned char>(simbolo));
            if (encontrado == transicoes_[atual].end())
            {
                // Num AFD completo, só um símbolo fora do alfabeto chega aqui.
                return false;
            }
            atual = encontrado->second;
        }
        return finais_.contains(atual);
    }

    std::string AfdGerado::formatarTabela() const
    {
        std::string texto;
        for (Estado estado = 0; estado < transicoes_.size(); ++estado)
        {
            texto += estado == inicial_ ? "  ->" : "    ";
            texto += finais_.contains(estado) ? "* " : "  ";
            std::string nome = "D" + std::to_string(estado);
            nome.resize(std::max<std::size_t>(nome.size(), 5), ' ');
            std::string conjunto = conjuntoComoTexto(representa_[estado]);
            conjunto.resize(std::max<std::size_t>(conjunto.size(), 28), ' ');
            texto += nome + conjunto;
            for (const auto &[rotulo, destino] : transicoes_[estado])
            {
                texto += " " + simboloVisivel(static_cast<char>(rotulo)) + "->D" + std::to_string(destino);
            }
            texto += "\n";
        }
        return texto;
    }

    // --- a construção de subconjuntos -------------------------------------------

    std::vector<Estado> moverEFechar(const Afn &afn, const std::vector<Estado> &conjunto, const int rotulo)
    {
        std::vector<Estado> resultado = afn.fechoVazio(afn.mover(conjunto, rotulo));
        std::sort(resultado.begin(), resultado.end());
        return resultado;
    }

    // recorte:inicio construcao-de-subconjuntos
    AfdGerado determinizar(const Afn &afn)
    {
        using Conjunto = std::vector<Estado>; // sempre ordenado: é chave do catálogo

        AfdGerado afd(afn.alfabeto());

        // Um AFN sem estados não aceita nada: o AFD equivalente é só o estado
        // de erro, que representa o conjunto vazio.
        if (afn.quantidadeDeEstados() == 0)
        {
            const Estado erro = afd.novoEstado({});
            afd.definirInicial(erro);
            for (const int rotulo : afd.alfabeto())
            {
                afd.adicionarTransicao(erro, rotulo, erro);
            }
            return afd;
        }

        const auto contemFinal = [&afn](const Conjunto &conjunto)
        { return std::binary_search(conjunto.begin(), conjunto.end(), afn.aceitacao()); };

        // O catálogo associa cada conjunto que já recebeu número ao estado do
        // AFD; os pendentes são os conjuntos do catálogo cujas transições ainda
        // não foram calculadas.
        std::map<Conjunto, Estado> catalogo;
        std::vector<Conjunto> pendentes;

        const auto registrar = [&](const Conjunto &conjunto)
        {
            const Estado id = afd.novoEstado(conjunto);
            catalogo.emplace(conjunto, id);
            pendentes.push_back(conjunto);
            if (contemFinal(conjunto))
            {
                afd.marcarAceitacao(id);
            }
            return id;
        };

        Conjunto inicial = afn.fechoVazio({afn.inicial()});
        std::sort(inicial.begin(), inicial.end());
        afd.definirInicial(registrar(inicial));

        while (!pendentes.empty())
        {
            const Conjunto atual = pendentes.back();
            pendentes.pop_back();
            const Estado origem = catalogo.at(atual);

            for (const int rotulo : afd.alfabeto())
            {
                // O conjunto vazio não tem caso especial: mover(∅, a) = ∅ e
                // E(∅) = ∅, então ele entra no catálogo como qualquer outro na
                // primeira vez que aparece, e vira o estado de erro.
                const Conjunto destino = moverEFechar(afn, atual, rotulo);
                const auto encontrado = catalogo.find(destino);
                const Estado idDestino = encontrado == catalogo.end() ? registrar(destino) : encontrado->second;
                afd.adicionarTransicao(origem, rotulo, idDestino);
            }
        }
        return afd;
    }
    // recorte:fim construcao-de-subconjuntos

} // namespace pinglu
