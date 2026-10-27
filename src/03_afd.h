// 03_afd.h — o autômato finito determinístico do Pinglu.
//
// A quíntupla M = (Q, Σ, δ, q0, F) escrita como classe, mais as máquinas das
// classes léxicas de especificacao/classes_lexicas.md projetadas a partir da
// especificação em prosa. O gerador de autômatos só chega no módulo 04; até lá,
// cada máquina é escrita à mão, e um executor testado contra máquinas
// conhecidas separa "a máquina está errada" de "quem a executa está errado".
//
// Decisões de representação (a conta está em docs/03_representacao_transicao.md):
//   - δ é uma matriz densa, indexada por estado e por COLUNA de símbolo, e a
//     coluna sai de um mapeamento byte -> coluna calculado no construtor;
//   - δ é total por construção: existe um estado de erro absorvente, e toda
//     posição da tabela nasce apontando para ele. Quem monta a máquina declara
//     só as transições que NÃO vão para o erro.
//
// Montar uma máquina é trabalho do programador, e um erro ali é defeito, não
// entrada inválida: por isso a construção lança std::invalid_argument. Já
// reconhecer texto NUNCA lança: texto arbitrário do usuário é recusado, não
// abortado.

#ifndef PINGLU_03_AFD_H
#define PINGLU_03_AFD_H

#include <cstddef>
#include <string>
#include <vector>

namespace pinglu
{

    using Estado = std::size_t;

    // Uma configuração é o par (estado corrente, posição na cadeia). Guardar a
    // posição em vez do sufixo ainda não lido carrega a mesma informação sem
    // alocar uma cadeia nova a cada passo.
    struct Configuracao
    {
        Estado estado = 0;
        std::size_t posicao = 0;
        char simboloLido = '\0'; // o símbolo que levou a esta configuração
    };

    // As três maneiras de recusar, mais a aceitação. Distingui-las no próprio
    // resultado evita que quem recebe a execução precise da cadeia e da máquina
    // juntas para descobrir o que aconteceu.
    enum class Veredito
    {
        Aceita,
        SimboloForaDoAlfabeto, // o símbolo nem tem coluna na tabela
        SemTransicao,          // o símbolo é do alfabeto, mas δ manda para o erro
        ParouEmEstadoNaoFinal, // leu tudo, sem cair, e parou fora de F
    };

    struct Execucao
    {
        Veredito veredito = Veredito::ParouEmEstadoNaoFinal;
        std::vector<Configuracao> passos;
        // Posição do primeiro símbolo que levou ao erro; npos se nunca caiu.
        std::size_t posicaoDaQueda = std::string::npos;

        bool aceitou() const { return veredito == Veredito::Aceita; }
    };

    class Afd
    {
    public:
        // `estadosUteis` não conta o estado de erro: ele recebe o índice seguinte.
        Afd(std::string nome, std::string alfabeto, std::size_t estadosUteis, Estado inicial);

        // --- construção ------------------------------------------------------
        void nomearEstado(Estado estado, std::string nome);
        void marcarAceitacao(Estado estado);
        // Liga cada símbolo de `simbolos` a `destino`, a partir de `origem`.
        void ligar(Estado origem, const std::string &simbolos, Estado destino);
        // Liga TODO o alfabeto, menos `excecoes`, a `destino`. Existe porque as
        // máquinas de texto (string, char, comentário) dizem "qualquer coisa,
        // menos X" sobre um alfabeto de dezenas de símbolos.
        void ligarTodosExceto(Estado origem, const std::string &excecoes, Estado destino);

        // --- reconhecimento --------------------------------------------------
        Estado transicao(Estado origem, char simbolo) const;
        bool ehDeAceitacao(Estado estado) const;
        bool aceita(const std::string &cadeia) const;
        Execucao executar(const std::string &cadeia) const;

        // --- o que a máquina informa sobre si --------------------------------
        const std::string &nome() const;
        const std::string &nomeDoEstado(Estado estado) const;
        std::size_t estadosUteis() const;
        std::size_t tamanhoDoAlfabeto() const;

        // (estadosUteis + 1) x |alfabeto| x sizeof(Estado).
        std::size_t bytesDaTabela() const;
        // Custo do vetor byte -> coluna: uma vez por máquina, não por estado.
        std::size_t bytesDoMapeamento() const;
        // O que a tabela custaria se a coluna fosse o próprio código do byte.
        std::size_t bytesSeIndexadaPorByte() const;
        // Posições cujo destino não é o erro. O que falta para o total é o que
        // a matriz densa guarda a mais do que um mapa esparso guardaria.
        std::size_t transicoesDefinidas() const;

        std::string formatarTabela() const;
        std::string formatarResumo() const;
        std::string descreverResultado(const std::string &cadeia, const Execucao &execucao) const;
        std::string formatarExecucao(const std::string &cadeia, const Execucao &execucao) const;

    private:
        std::size_t colunaDe(char simbolo) const;

        std::string nome_;
        std::string alfabeto_;
        std::vector<std::size_t> colunaDoByte_; // 256 entradas: byte -> coluna
        std::vector<Estado> tabela_;            // (estadosUteis + 1) x |alfabeto|
        // `char` e não `bool`: vector<bool> empacota bits e não devolve
        // referência de verdade.
        std::vector<char> aceitacao_;
        std::vector<std::string> nomes_;
        Estado inicial_ = 0;
        Estado erro_ = 0;
    };

    // As máquinas das classes léxicas do Pinglu, cada uma projetada a partir da
    // especificação em prosa (ver os comentários de cada uma em 03_afd.cpp).
    Afd afdIdentificador();
    Afd afdNumero();
    Afd afdCadeiaDeCaracteres();
    Afd afdCaractere();
    Afd afdComentarioDeLinha();
    Afd afdOperadoresEDelimitadores();

    std::vector<Afd> automatosDoPinglu();

} // namespace pinglu

#endif // PINGLU_03_AFD_H
