// 04_afn.h — o autômato finito não determinístico com transições vazias, a
// construção de Thompson que o produz a partir da árvore de um pattern, e a
// bateria de casos conferidos à mão.
//
// A construção tem um caso por tipo de nó da árvore, e a recursão faz a
// composição. Todo caso devolve um bloco com exatamente uma entrada e uma
// saída: essa interface uniforme é o que deixa compor blocos sem olhar dentro
// deles. As transições vazias mantêm cada caso local, e em troca a máquina se
// executa com um CONJUNTO de estados ativos, não com um estado corrente.
//
// Duas decisões deste módulo seguem das anteriores:
//   - o Pinglu não tem coringa (docs/02_nucleo_minimo.md), então a árvore tem
//     cinco tipos de nó e a construção, cinco casos, sem rótulo especial;
//   - as máquinas de texto aceitam QUALQUER byte (especificacao/alfabeto.md),
//     inclusive o zero. Por isso o rótulo da transição vazia não é um byte: é um
//     `int` fora do intervalo 0..255, e uma entrada com bytes zero nunca é
//     confundida com transição vazia.
//
// Montar uma máquina é trabalho do programador, e um erro ali (estado
// inexistente, árvore malformada) lança std::invalid_argument. Simular nunca
// lança: a cadeia é do usuário, e a resposta a qualquer cadeia é aceita ou
// recusa.

#ifndef PINGLU_04_AFN_H
#define PINGLU_04_AFN_H

#include <cstddef>
#include <istream>
#include <string>
#include <vector>

#include "02_regex_pinglu.h"
#include "03_afd.h" // só o tipo Estado

namespace pinglu
{

    // Rótulo da transição vazia. Os rótulos de símbolo são os bytes 0..255.
    inline constexpr int kEpsilon = -1;

    struct TransicaoAfn
    {
        int rotulo = kEpsilon;
        Estado destino = 0;
    };

    // Um bloco de máquina com uma entrada e uma saída — o invariante da
    // construção. Quando um bloco nasce, nenhuma seta chega à entrada e
    // nenhuma sai da saída: as ligações vêm depois, de fora.
    struct Bloco
    {
        Estado entrada = 0;
        Estado saida = 0;
    };

    // Um passo do traço da simulação: o conjunto ativo antes do primeiro
    // símbolo (`ehPartida`) ou logo depois de ler um símbolo. A partida tem
    // marca própria, e não um símbolo-sentinela, pelo mesmo motivo do rótulo
    // vazio: o byte zero é uma entrada possível.
    struct PassoDaSimulacao
    {
        bool ehPartida = false;
        char simboloLido = '\0';
        std::vector<Estado> ativos;
    };

    struct Simulacao
    {
        bool aceitou = false;
        std::vector<PassoDaSimulacao> passos;
        std::size_t maiorConjuntoAtivo = 0;
    };

    class Afn
    {
    public:
        Estado novoEstado();
        void adicionarTransicao(Estado origem, int rotulo, Estado destino);

        void definirInicial(Estado estado);
        void definirAceitacao(Estado estado);

        Estado inicial() const;
        Estado aceitacao() const;
        std::size_t quantidadeDeEstados() const;
        std::size_t quantidadeDeTransicoes() const;
        std::size_t quantidadeDeTransicoesVazias() const;

        // Os estados, fora o inicial, aos quais nenhuma transição chega. Numa
        // máquina de Thompson a lista é sempre vazia, e uma ligação esquecida
        // pode deixá-la com um estado — verificação que não simula nada.
        std::vector<Estado> estadosSemChegada() const;

        // O fecho vazio de um conjunto: todos os estados alcançáveis a partir
        // dele sem consumir símbolo. A simulação o aplica depois de cada
        // símbolo e também ao conjunto INICIAL, que é a aplicação fácil de
        // esquecer.
        std::vector<Estado> fechoVazio(const std::vector<Estado> &conjunto) const;

        bool aceita(const std::string &cadeia) const;
        Simulacao simular(const std::string &cadeia) const;

        std::string formatarSimulacao(const std::string &cadeia, const Simulacao &simulacao) const;

    private:
        std::vector<std::vector<TransicaoAfn>> transicoes_;
        Estado inicial_ = 0;
        Estado aceitacao_ = 0;
    };

    // Constrói a máquina a partir da árvore devolvida por analisarExpressao().
    // Uma árvore vazia dá a máquina de um estado só, que aceita apenas a cadeia
    // vazia.
    Afn construirThompson(const Arvore &arvore);

    // --- a bateria de casos conferidos à mão -----------------------------------

    // Um caso: o pattern, a cadeia e o veredito determinado sem rodar o programa.
    struct CasoDeTeste
    {
        std::string expressao;
        std::string cadeia;
        bool esperado = false;
        std::string origem; // arquivo:linha, citado na mensagem de falha
    };

    // Os casos lidos mais os problemas do arquivo. Uma linha malformada, um
    // veredito que não é "aceita" nem "recusa" e um arquivo que não abre são
    // ERROS: ignorá-los faria o caso sumir da bateria sem que nada acusasse.
    struct LeituraDeCasos
    {
        std::vector<CasoDeTeste> casos;
        std::vector<std::string> erros;
    };

    struct ResultadoDaBateria
    {
        std::size_t total = 0;
        std::size_t passaram = 0;
        std::vector<std::string> falhas;
    };

    // Lê os casos de um fluxo (`origem` é o nome citado nas mensagens) e de um
    // arquivo. A primeira existe para poder testar o leitor sem tocar no disco.
    LeituraDeCasos interpretarCasos(std::istream &fluxo, const std::string &origem);
    LeituraDeCasos lerCasos(const std::string &caminho);

    // Executa cada caso contra a simulação e acumula as falhas, sem parar na
    // primeira: quando uma mudança quebra vários casos, a lista completa vem de
    // uma execução só.
    ResultadoDaBateria rodarBateria(const std::vector<CasoDeTeste> &casos);

} // namespace pinglu

#endif // PINGLU_04_AFN_H
