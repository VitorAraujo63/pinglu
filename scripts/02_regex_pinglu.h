// 02_regex_pinglu.h — Leitura de uma expressão de padrão e conversão em árvore.
//
// Adaptado de 02_regex.h (Peneira) para o núcleo mínimo do Pinglu
// A única mudança estrutural é a remoção do coringa `.`,
// que o Pinglu não usa — o alfabeto do Pinglu é finito e fechado, então
// "qualquer símbolo" continua redutível a uma alternância comum, sem precisar
// de um operador de núcleo à parte.
//
// Recebe o texto de um pattern e devolve a árvore que a construção de Thompson
// consumirá na etapa seguinte — ou o primeiro erro encontrado, com a posição
// exata no texto.
//
// A árvore usa apenas o NÚCLEO MÍNIMO: concatenação, alternância e fecho, mais
// as duas folhas (símbolo e cadeia vazia). Toda notação de conveniência — `+`,
// `?`, classe de símbolos — é reduzida a esse núcleo durante a leitura, e não
// depois: o que sai daqui já não conhece os operadores reduzidos, e nenhuma
// peça posterior precisa aprendê-los.
//
// Os nós vivem num vetor e se referenciam por índice, nunca por ponteiro. A
// árvore é copiável, serializável e não vaza; e a duplicação de subárvore que a
// redução de `+` exige vira uma cópia de faixa de vetor, não um passeio
// recursivo de alocação.

#ifndef PINGLU_02_REGEX_H
#define PINGLU_02_REGEX_H

#include <cstddef>
#include <string>
#include <vector>

namespace pinglu
{

    // Índice ausente. Uma folha não tem filhos; o fecho tem só o esquerdo.
    inline constexpr std::size_t kSemFilho = static_cast<std::size_t>(-1);

    // recorte:inicio nucleo-minimo-como-tipo
    enum class TipoDeNo
    {
        Simbolo,      // um símbolo literal do alfabeto
        Vazio,        // a cadeia vazia, produzida pela redução de `?`
        Concatenacao, // núcleo
        Alternancia,  // núcleo
        Fecho,        // núcleo
    };

    struct No
    {
        TipoDeNo tipo = TipoDeNo::Vazio;
        char simbolo = '\0'; // significativo apenas em Simbolo
        std::size_t esquerda = kSemFilho;
        std::size_t direita = kSemFilho;
    };
    // recorte:fim nucleo-minimo-como-tipo

    struct Arvore
    {
        std::vector<No> nos;
        std::size_t raiz = kSemFilho;

        bool vazia() const;
    };

    // Erro de sintaxe com a posição em que foi detectado, contada em símbolos a
    // partir de zero. Carregar a posição desde a leitura é bem mais barato do que
    // acrescentá-la depois, quando a análise já está espalhada por vários pontos.
    struct ErroDeSintaxe
    {
        std::size_t posicao = 0;
        std::string mensagem;
    };

    struct Resultado
    {
        bool ok = false;
        Arvore arvore;
        ErroDeSintaxe erro;
    };

    // Lê a expressão e devolve a árvore reduzida ao núcleo, ou o primeiro erro.
    Resultado analisarExpressao(const std::string &expressao);

    // Forma prefixa canônica da árvore, em uma linha. É o que permite verificar que
    // duas notações diferentes do mesmo padrão convergiram para a mesma estrutura —
    // comparação de texto, e não inspeção visual de duas figuras.
    std::string formatarArvore(const Arvore &arvore);

    // A mensagem de erro pronta para exibição, com o cursor sob a posição.
    std::string formatarErro(const std::string &expressao, const ErroDeSintaxe &erro);

    // Número de nós da árvore: a medida do custo de uma redução, usada na
    // demonstração para mostrar o que uma classe de símbolos larga produz.
    std::size_t tamanho(const Arvore &arvore);

} // namespace pinglu

#endif // PINGLU_02_REGEX_H