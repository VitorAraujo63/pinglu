// 05_determinizacao.h — o AFD gerado a partir de um AFN e a construção de
// subconjuntos que o produz.
//
// Simular um AFN é manter, durante a leitura, o conjunto dos estados em que ele
// pode estar. Determinizar é fazer desses conjuntos os estados de uma máquina
// nova: cada estado do AFD representa um conjunto de estados do AFN, e a
// transição de um conjunto S com o símbolo a é E(mover(S, a)).
//
// Decisões deste módulo:
//   - o AFD gerado é uma estrutura NOVA, e não o Afd do módulo 03: aquele
//     precisa saber o número de estados antes de começar e já traz um estado
//     de erro implícito. Aqui os estados nascem conforme os conjuntos são
//     descobertos, e o estado de erro é o conjunto vazio, que recebe número
//     como qualquer outro na primeira vez que aparece;
//   - o rótulo é `int` (0..255), como no AFN: o byte zero é um símbolo
//     possível, e kEpsilon (-1) nunca entra no alfabeto;
//   - o alfabeto é o do AFN, FECHADO: só os símbolos que aparecem nas
//     transições. Como o Pinglu não tem coringa, um símbolo fora dele é
//     recusado tanto pelo AFN (nenhuma transição o lê) quanto pelo AFD (não há
//     coluna para ele), e as duas máquinas concordam em QUALQUER cadeia, não só
//     nas cadeias sobre o alfabeto.
//
// Montar uma máquina com estado ou rótulo inexistente lança
// std::invalid_argument; reconhecer uma cadeia nunca lança.

#ifndef PINGLU_05_DETERMINIZACAO_H
#define PINGLU_05_DETERMINIZACAO_H

#include <cstddef>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "04_afn.h"

namespace pinglu
{

    class AfdGerado
    {
    public:
        explicit AfdGerado(std::set<int> alfabeto);

        // --- construção ------------------------------------------------------
        // `representa` é o que o estado novo significa na máquina de origem:
        // na determinização, o conjunto (ordenado) de estados do AFN; na
        // minimização, o bloco de estados do AFD que se fundiram nele.
        Estado novoEstado(std::vector<Estado> representa);
        // Uma segunda transição com o mesmo par (origem, rótulo) SUBSTITUI a
        // primeira: é o que faz da função de transição uma função.
        void adicionarTransicao(Estado origem, int rotulo, Estado destino);
        void definirInicial(Estado estado);
        void marcarAceitacao(Estado estado);

        // --- consulta --------------------------------------------------------
        Estado inicial() const;
        const std::set<int> &alfabeto() const;
        const std::set<Estado> &finais() const;
        bool ehDeAceitacao(Estado estado) const;
        const std::vector<Estado> &representa(Estado estado) const;
        std::optional<Estado> transicao(Estado origem, int rotulo) const;

        std::size_t quantidadeDeEstados() const;
        std::size_t quantidadeDeTransicoes() const;
        // Completo: toda posição (estado, símbolo do alfabeto) tem destino.
        bool completo() const;

        // --- reconhecimento --------------------------------------------------
        bool aceita(const std::string &cadeia) const;

        // A tabela de transições, uma linha por estado, com o conjunto que cada
        // um representa e a marca de inicial (->) e de aceitação (*). `prefixo`
        // nomeia os estados: "D" no AFD da determinização, "M" no mínimo.
        std::string formatarTabela(const std::string &prefixo = "D") const;

    private:
        void exigirEstado(Estado estado, const char *quem) const;

        std::vector<std::map<int, Estado>> transicoes_;
        std::vector<std::vector<Estado>> representa_;
        std::set<int> alfabeto_;
        std::set<Estado> finais_;
        Estado inicial_ = 0;
    };

    // E(mover(S, a)), devolvido ORDENADO. A ordem importa: o conjunto é chave
    // do catálogo, e {0, 2, 4} e {4, 0, 2} precisam cair na mesma chave.
    std::vector<Estado> moverEFechar(const Afn &afn, const std::vector<Estado> &conjunto, int rotulo);

    // A construção de subconjuntos. Os estados do AFD são numerados na ordem em
    // que os conjuntos entram no catálogo, com os pendentes numa pilha (o
    // último que entrou é o próximo processado) e o alfabeto em ordem
    // crescente — a mesma numeração das tabelas do guia do módulo 05.
    // O AFD sai completo: o conjunto vazio, quando alcançável, é o estado de
    // erro, e suas transições voltam a ele mesmo.
    AfdGerado determinizar(const Afn &afn);

} // namespace pinglu

#endif // PINGLU_05_DETERMINIZACAO_H
