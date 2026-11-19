// 05_minimizacao.h — a minimização do AFD gerado por refinamento de partições
// e a comparação exaustiva das três máquinas (AFN, AFD e mínimo).
//
// Dois estados são EQUIVALENTES quando nenhuma cadeia leva um deles à aceitação
// sem levar o outro. O refinamento parte da partição que a cadeia vazia já
// distingue (aceita / não aceita) e, a cada rodada, separa os estados de um
// mesmo bloco cujos símbolos levam a blocos diferentes. Quando uma rodada não
// separa ninguém, a partição chegou ao ponto fixo, e cada bloco vira um estado.
//
// Decisões deste módulo:
//   - o resultado é um AfdGerado, como o da determinização: o campo
//     `representa` de cada estado novo guarda o BLOCO de estados do AFD que se
//     fundiram nele, e a correspondência pode ser impressa;
//   - antes de refinar, o AFD é COMPLETADO (um estado de erro recebe toda
//     transição que falta) e os estados INACESSÍVEIS são descartados. O
//     resultado é, portanto, o autômato mínimo completo: o estado de erro só
//     aparece quando alguma cadeia leva a ele;
//   - o ponto fixo é detectado pela CONTAGEM de blocos de duas rodadas
//     seguidas, e não por um número esperado. A assinatura de um estado começa
//     pelo bloco em que ele já está, então uma rodada só divide blocos, nunca
//     junta: se a contagem não cresceu, nada se dividiu;
//   - o próprio sistema informa as contagens (antes, alcançáveis, depois e a
//     sequência de blocos por rodada), em vez de alguém contá-las à mão.

#ifndef PINGLU_05_MINIMIZACAO_H
#define PINGLU_05_MINIMIZACAO_H

#include <cstddef>
#include <set>
#include <string>
#include <vector>

#include "04_afn.h"
#include "05_determinizacao.h"

namespace pinglu
{

    // --- a minimização -----------------------------------------------------------

    // Uma cópia do AFD em que toda posição (estado, símbolo) tem destino. Se já
    // era completo, a cópia é idêntica; se não, ganha UM estado de erro (que
    // representa o conjunto vazio e não aceita), para o qual vão todas as
    // transições que faltavam, inclusive as dele mesmo.
    AfdGerado completar(const AfdGerado &afd);

    // Os estados que alguma cadeia alcança a partir do inicial (busca em
    // largura). Os demais não mudam a linguagem, mas impedem o mínimo.
    std::set<Estado> estadosAlcancaveis(const AfdGerado &afd);

    struct ResultadoDaMinimizacao
    {
        AfdGerado afd;
        std::size_t estadosAntes = 0;       // do AFD recebido
        std::size_t estadosCompletado = 0;  // depois de completar
        std::size_t estadosAlcancaveis = 0; // dos quais alcançáveis
        std::size_t estadosDepois = 0;      // do mínimo
        // Blocos da partição inicial e de cada rodada. Os dois últimos valores
        // são iguais: a rodada que não separou ninguém é a condição de parada.
        std::vector<std::size_t> blocosPorRodada;
    };

    // Completa, descarta inacessíveis, monta a partição inicial e refina até o
    // ponto fixo. O AFD recebido não é alterado. Os estados do mínimo são
    // numerados pela ordem do menor estado de cada bloco, e por isso o bloco do
    // estado inicial do AFD de origem (D0, na determinização) vira M0.
    ResultadoDaMinimizacao minimizar(const AfdGerado &afd);

    // "2 -> 3 -> 3", pronto para imprimir.
    std::string formatarRodadas(const std::vector<std::size_t> &blocosPorRodada);

    // --- a comparação exaustiva -------------------------------------------------

    // O alfabeto de rótulos (0..255) como cadeia de símbolos, em ordem.
    std::string alfabetoComoTexto(const std::set<int> &alfabeto);

    // TODAS as cadeias sobre `simbolos`, da vazia até `comprimentoMaximo`, em
    // ordem de comprimento. É enumeração exaustiva, e não amostra: com k
    // símbolos são (k^(L+1) - 1) / (k - 1) cadeias, e por isso L fica pequeno.
    std::vector<std::string> todasAsCadeias(const std::string &simbolos, std::size_t comprimentoMaximo);

    // A primeira cadeia em que duas máquinas discordam. `houve` separa a
    // discordância na cadeia vazia do caso em que as duas concordaram em todas.
    struct Discordancia
    {
        bool houve = false;
        std::string cadeia;
        bool primeiraAceitou = false;
        bool segundaAceitou = false;
    };

    // Param na primeira discordância: uma basta para refutar a equivalência.
    Discordancia compararSobre(const Afn &afn, const AfdGerado &afd, const std::vector<std::string> &cadeias);
    Discordancia compararSobre(const AfdGerado &primeira, const AfdGerado &segunda,
                               const std::vector<std::string> &cadeias);

    // A comparação das TRÊS máquinas, nos dois saltos (AFN -> AFD e AFD ->
    // mínimo), e não só entre as pontas: comparar só AFN com mínimo esconderia
    // dois erros que se cancelam, um em cada etapa.
    struct ComparacaoDasTres
    {
        std::size_t cadeias = 0;
        std::size_t aceitasPeloAfn = 0;
        std::size_t aceitasPeloAfd = 0;
        std::size_t aceitasPeloMinimo = 0;
        Discordancia afnContraAfd;
        Discordancia afdContraMinimo;

        bool concordam() const { return !afnContraAfd.houve && !afdContraMinimo.houve; }
    };

    ComparacaoDasTres compararAsTres(const Afn &afn, const AfdGerado &afd, const AfdGerado &minimo,
                                     const std::vector<std::string> &cadeias);

    // Uma linha de diagnóstico: "as tres maquinas concordam em todas" ou a
    // primeira discordância, com o salto em que ela apareceu.
    std::string descreverComparacao(const ComparacaoDasTres &comparacao);

} // namespace pinglu

#endif // PINGLU_05_MINIMIZACAO_H
