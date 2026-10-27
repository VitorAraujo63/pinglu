#include "03_afd.h"

#include <stdexcept>
#include <utility>

namespace pinglu
{

    namespace
    {

        constexpr std::size_t kSemColuna = static_cast<std::size_t>(-1);

        // Alinha a coluna para que a tabela impressa fique legível na projeção.
        std::string preencher(const std::string &texto, const std::size_t largura)
        {
            std::string resultado = texto;
            while (resultado.size() < largura)
            {
                resultado += ' ';
            }
            return resultado;
        }

        // Um símbolo como o leitor consegue ver. Espaço, nova linha e bytes fora
        // do ASCII imprimível são invisíveis ou quebram o terminal, e a mensagem
        // de recusa precisa apontá-los sem ambiguidade.
        std::string visivel(const char simbolo)
        {
            switch (simbolo)
            {
            case ' ':
                return "<esp>";
            case '\n':
                return "\\n";
            case '\t':
                return "\\t";
            case '\r':
                return "\\r";
            default:
                break;
            }
            const unsigned char byte = static_cast<unsigned char>(simbolo);
            if (byte < 0x20 || byte >= 0x7F)
            {
                static const char *const hexa = "0123456789abcdef";
                std::string texto = "\\x";
                texto += hexa[byte >> 4];
                texto += hexa[byte & 0x0F];
                return texto;
            }
            return std::string(1, simbolo);
        }

        // Para ecoar a cadeia: o espaço fica como espaço, o resto passa por visivel.
        std::string visivelDaCadeia(const std::string &cadeia)
        {
            std::string texto;
            for (const char simbolo : cadeia)
            {
                texto += simbolo == ' ' ? std::string(" ") : visivel(simbolo);
            }
            return texto;
        }

        std::string faixa(const char inicio, const char fim)
        {
            std::string simbolos;
            for (int codigo = static_cast<unsigned char>(inicio);
                 codigo <= static_cast<unsigned char>(fim); ++codigo)
            {
                simbolos += static_cast<char>(codigo);
            }
            return simbolos;
        }

        // --- os alfabetos -----------------------------------------------------
        //
        // O Σ de especificacao/alfabeto.md tem 89 símbolos. Dois problemas, ambos
        // registrados em docs/03_representacao_transicao.md como pendência:
        //   1. O `´` (U+00B4) não é ASCII: ocupa dois bytes em UTF-8, e uma
        //      máquina indexada por byte não o enxerga como um símbolo só. Fica
        //      de fora destes alfabetos, que são os 88 símbolos ASCII de Σ.
        //   2. Σ não tem espaço nem nova linha, mas os exemplos os usam o tempo
        //      todo (`print("fim do for")`, todo comentário). As máquinas de
        //      texto acrescentam o espaço; a de comentário acrescenta também a
        //      nova linha, que é o que a faz parar "no fim da linha".

        std::string letras() { return faixa('a', 'z') + faixa('A', 'Z'); }

        std::string digitos() { return faixa('0', '9'); }

        // Os 23 símbolos ASCII de Σ que sobram depois de letras, dígitos e dos
        // três que as máquinas de texto tratam à parte: ' " \ .
        std::string simbolosComuns() { return "/*-+_=&%!;,|<>`(){}[].?"; }

        // Σ ASCII + espaço. A ordem põe ' " \ no fim de propósito: "tudo menos
        // aspas e barra" vira poucas faixas contíguas na tabela impressa.
        std::string alfabetoDeTexto()
        {
            return letras() + digitos() + simbolosComuns() + " " + "'" + "\"" + "\\";
        }

    } // namespace

    Afd::Afd(std::string nome, std::string alfabeto, const std::size_t estadosUteis,
             const Estado inicial)
        : nome_(std::move(nome)),
          alfabeto_(std::move(alfabeto)),
          colunaDoByte_(256, kSemColuna),
          aceitacao_(estadosUteis + 1, 0),
          nomes_(estadosUteis + 1),
          inicial_(inicial),
          erro_(estadosUteis)
    {
        if (estadosUteis == 0 || inicial >= estadosUteis)
        {
            throw std::invalid_argument("Afd '" + nome_ + "': o estado inicial deve ser um estado util.");
        }

        // recorte:inicio coluna-nao-e-o-byte
        for (std::size_t coluna = 0; coluna < alfabeto_.size(); ++coluna)
        {
            const std::size_t byte = static_cast<unsigned char>(alfabeto_[coluna]);
            if (colunaDoByte_[byte] != kSemColuna)
            {
                throw std::invalid_argument("Afd '" + nome_ + "': simbolo repetido no alfabeto: " +
                                            visivel(alfabeto_[coluna]));
            }
            colunaDoByte_[byte] = coluna;
        }
        // recorte:fim coluna-nao-e-o-byte

        // recorte:inicio tabela-total-por-construcao
        // Toda posição nasce apontando para o erro: quem monta a máquina declara
        // só as transições que existem, e a função já sai total.
        tabela_.assign((estadosUteis + 1) * alfabeto_.size(), erro_);
        // recorte:fim tabela-total-por-construcao

        for (std::size_t estado = 0; estado < estadosUteis; ++estado)
        {
            nomes_[estado] = "q" + std::to_string(estado);
        }
        nomes_[erro_] = "erro";
    }

    std::size_t Afd::colunaDe(const char simbolo) const
    {
        return colunaDoByte_[static_cast<unsigned char>(simbolo)];
    }

    void Afd::nomearEstado(const Estado estado, std::string nome)
    {
        if (estado >= erro_)
        {
            throw std::invalid_argument("Afd '" + nome_ + "': nomearEstado em estado inexistente.");
        }
        nomes_[estado] = std::move(nome);
    }

    void Afd::marcarAceitacao(const Estado estado)
    {
        if (estado >= erro_)
        {
            throw std::invalid_argument("Afd '" + nome_ +
                                        "': so estados uteis podem ser de aceitacao.");
        }
        aceitacao_[estado] = 1;
    }

    void Afd::ligar(const Estado origem, const std::string &simbolos, const Estado destino)
    {
        if (origem >= erro_ || destino >= erro_)
        {
            throw std::invalid_argument(
                "Afd '" + nome_ +
                "': origem e destino devem ser estados uteis (para ir ao erro, basta nao declarar).");
        }
        for (const char simbolo : simbolos)
        {
            const std::size_t coluna = colunaDe(simbolo);
            if (coluna == kSemColuna)
            {
                throw std::invalid_argument("Afd '" + nome_ + "': simbolo fora do alfabeto na construcao: " +
                                            visivel(simbolo));
            }
            tabela_[origem * alfabeto_.size() + coluna] = destino;
        }
    }

    void Afd::ligarTodosExceto(const Estado origem, const std::string &excecoes,
                               const Estado destino)
    {
        for (const char excecao : excecoes)
        {
            if (colunaDe(excecao) == kSemColuna)
            {
                throw std::invalid_argument("Afd '" + nome_ + "': excecao fora do alfabeto: " +
                                            visivel(excecao));
            }
        }
        std::string restantes;
        for (const char simbolo : alfabeto_)
        {
            if (excecoes.find(simbolo) == std::string::npos)
            {
                restantes += simbolo;
            }
        }
        ligar(origem, restantes, destino);
    }

    // recorte:inicio tabela-densa-indexada
    Estado Afd::transicao(const Estado origem, const char simbolo) const
    {
        const std::size_t coluna = colunaDe(simbolo);
        if (coluna == kSemColuna)
        {
            return erro_;
        }
        return tabela_[origem * alfabeto_.size() + coluna];
    }
    // recorte:fim tabela-densa-indexada

    bool Afd::ehDeAceitacao(const Estado estado) const { return aceitacao_[estado] != 0; }

    // recorte:inicio aceita-em-quatro-linhas
    bool Afd::aceita(const std::string &cadeia) const
    {
        Estado atual = inicial_;
        for (const char simbolo : cadeia)
        {
            atual = transicao(atual, simbolo);
        }
        return ehDeAceitacao(atual);
    }
    // recorte:fim aceita-em-quatro-linhas

    // recorte:inicio queda-registrada-sem-parar
    Execucao Afd::executar(const std::string &cadeia) const
    {
        Execucao execucao;
        Estado atual = inicial_;
        Veredito formaDaQueda = Veredito::SemTransicao;
        execucao.passos.push_back(Configuracao{atual, 0, '\0'});

        for (std::size_t i = 0; i < cadeia.size(); ++i)
        {
            const char simbolo = cadeia[i];
            const bool foraDoAlfabeto = colunaDe(simbolo) == kSemColuna;
            const Estado proximo = transicao(atual, simbolo);
            execucao.passos.push_back(Configuracao{proximo, i + 1, simbolo});

            // Registra só a primeira queda e segue lendo: parar daria a mesma
            // resposta, mas o traço terminaria antes da cadeia e não mostraria o
            // erro absorvendo o resto dela.
            if (proximo == erro_ && execucao.posicaoDaQueda == std::string::npos)
            {
                execucao.posicaoDaQueda = i;
                formaDaQueda = foraDoAlfabeto ? Veredito::SimboloForaDoAlfabeto
                                              : Veredito::SemTransicao;
            }
            atual = proximo;
        }

        if (ehDeAceitacao(atual))
        {
            execucao.veredito = Veredito::Aceita;
        }
        else if (execucao.posicaoDaQueda != std::string::npos)
        {
            execucao.veredito = formaDaQueda;
        }
        else
        {
            execucao.veredito = Veredito::ParouEmEstadoNaoFinal;
        }
        return execucao;
    }
    // recorte:fim queda-registrada-sem-parar

    const std::string &Afd::nome() const { return nome_; }

    const std::string &Afd::nomeDoEstado(const Estado estado) const { return nomes_[estado]; }

    std::size_t Afd::estadosUteis() const { return erro_; }

    std::size_t Afd::tamanhoDoAlfabeto() const { return alfabeto_.size(); }

    // recorte:inicio bytes-da-tabela
    std::size_t Afd::bytesDaTabela() const { return tabela_.size() * sizeof(Estado); }

    std::size_t Afd::bytesDoMapeamento() const
    {
        return colunaDoByte_.size() * sizeof(std::size_t);
    }

    std::size_t Afd::bytesSeIndexadaPorByte() const { return (erro_ + 1) * 256 * sizeof(Estado); }

    std::size_t Afd::transicoesDefinidas() const
    {
        std::size_t total = 0;
        for (const Estado destino : tabela_)
        {
            if (destino != erro_)
            {
                ++total;
            }
        }
        return total;
    }
    // recorte:fim bytes-da-tabela

    std::string Afd::formatarTabela() const
    {
        // Por faixa de colunas consecutivas com o mesmo destino, e não coluna a
        // coluna: o alfabeto de texto tem 89 colunas. A faixa é por posição no
        // alfabeto DECLARADO, não por código de caractere; a contagem entre
        // colchetes impede que `a..' [87]` seja lido como intervalo ASCII.
        std::string texto;
        texto += preencher("ESTADO", 20) + preencher("ACEITA", 8) + "TRANSICOES\n";
        for (std::size_t estado = 0; estado <= erro_; ++estado)
        {
            texto += preencher(nomes_[estado], 20);
            texto += preencher(aceitacao_[estado] != 0 ? "sim" : "nao", 8);

            bool primeiro = true;
            for (std::size_t coluna = 0; coluna < alfabeto_.size(); ++coluna)
            {
                const Estado destino = tabela_[estado * alfabeto_.size() + coluna];
                if (destino == erro_)
                {
                    continue;
                }
                std::size_t fim = coluna;
                while (fim + 1 < alfabeto_.size() &&
                       tabela_[estado * alfabeto_.size() + fim + 1] == destino)
                {
                    ++fim;
                }
                if (!primeiro)
                {
                    texto += ", ";
                }
                const std::size_t quantidade = fim - coluna + 1;
                if (quantidade <= 3)
                {
                    // Até três símbolos, um a um: `+ -` escrito como faixa
                    // sugeriria um intervalo que não existe.
                    for (std::size_t i = coluna; i <= fim; ++i)
                    {
                        if (i > coluna)
                        {
                            texto += ' ';
                        }
                        texto += visivel(alfabeto_[i]);
                    }
                }
                else
                {
                    texto += visivel(alfabeto_[coluna]) + ".." + visivel(alfabeto_[fim]) + " [" +
                             std::to_string(quantidade) + "]";
                }
                texto += " -> " + nomes_[destino];
                primeiro = false;
                coluna = fim;
            }
            if (primeiro)
            {
                texto += "(todas para erro)";
            }
            texto += '\n';
        }
        return texto;
    }

    std::string Afd::formatarResumo() const
    {
        return "  estados uteis: " + std::to_string(erro_) + " (+1 de erro) | alfabeto: " +
               std::to_string(alfabeto_.size()) + " | posicoes: " + std::to_string(tabela_.size()) +
               " | bytes: " + std::to_string(bytesDaTabela()) +
               " | transicoes nao-erro: " + std::to_string(transicoesDefinidas()) + "\n";
    }

    std::string Afd::descreverResultado(const std::string &cadeia, const Execucao &execucao) const
    {
        switch (execucao.veredito)
        {
        case Veredito::Aceita:
            return "ACEITA, parou em " + nomes_[execucao.passos.back().estado];
        case Veredito::SimboloForaDoAlfabeto:
        {
            const std::size_t i = execucao.posicaoDaQueda;
            return "RECUSA: o simbolo '" + visivel(cadeia[i]) + "' (posicao " + std::to_string(i) +
                   ") esta fora do alfabeto desta maquina";
        }
        case Veredito::SemTransicao:
        {
            const std::size_t i = execucao.posicaoDaQueda;
            return "RECUSA: o simbolo '" + visivel(cadeia[i]) + "' (posicao " + std::to_string(i) +
                   ") e do alfabeto, mas nao ha transicao a partir de " +
                   nomes_[execucao.passos[i].estado];
        }
        case Veredito::ParouEmEstadoNaoFinal:
            return "RECUSA: leu a cadeia inteira e parou em " +
                   nomes_[execucao.passos.back().estado] + ", que nao e de aceitacao";
        }
        return "";
    }

    std::string Afd::formatarExecucao(const std::string &cadeia, const Execucao &execucao) const
    {
        std::string texto = "  cadeia: \"" + visivelDaCadeia(cadeia) + "\"\n";
        texto += "  configuracoes: ";
        for (std::size_t i = 0; i < execucao.passos.size(); ++i)
        {
            const Configuracao &passo = execucao.passos[i];
            if (i > 0)
            {
                texto += " -" + visivel(passo.simboloLido) + "-> ";
            }
            texto += nomes_[passo.estado];
        }
        texto += "\n  resultado: " + descreverResultado(cadeia, execucao) + "\n";
        return texto;
    }

    // ==========================================================================
    // As máquinas das classes léxicas do Pinglu.
    //
    // Método de cada uma: "qual é a menor distinção que a especificação obriga a
    // lembrar?". Um estado é uma memória, não um lugar no desenho: dois estados
    // só se justificam se existir uma continuação aceita a partir de um e
    // recusada a partir do outro.
    // ==========================================================================

    // identificador := (letra | '_') (letra | digito | '_')*
    //
    // Duas situações: nada lido ainda, e primeiro caractere válido já lido. A
    // segunda posição e a quinta aceitam o mesmo conjunto, e nas duas a cadeia
    // até ali já é válida: nada as distingue. Palavra reservada NÃO tem máquina
    // própria: lê-se pelo identificador e consulta-se uma tabela depois.
    Afd afdIdentificador()
    {
        Afd afd("identificador", letras() + digitos() + "_", 2, 0);
        afd.nomearEstado(0, "inicio");
        afd.nomearEstado(1, "corpo");
        afd.ligar(0, letras() + "_", 1);
        afd.ligar(1, letras() + digitos() + "_", 1);
        afd.marcarAceitacao(1);
        return afd;
    }

    // numero := digito+ ('.' digito+)?
    //
    // SEM sinal: o `-` unário usa o mesmo token da subtração, e quem decide o
    // papel dele é o analisador sintático (ver "Observacoes abertas" em
    // classes_lexicas.md). Pôr o sinal aqui resolveria no autômato o que é do
    // parser, e `a-1` viraria `a` seguido do número `-1`.
    //
    // Dois estados de aceitação, porque são duas classes léxicas: `inteiro`
    // (numero_inteiro) e `fracao` (numero_float). O estado em que a máquina
    // pára diz qual foi reconhecida. `apos_ponto` não aceita: "3." não é número,
    // mas ainda pode vir a ser, e essa situação difere das outras duas.
    Afd afdNumero()
    {
        Afd afd("numero", digitos() + ".", 4, 0);
        afd.nomearEstado(0, "inicio");
        afd.nomearEstado(1, "inteiro");
        afd.nomearEstado(2, "apos_ponto");
        afd.nomearEstado(3, "fracao");
        afd.ligar(0, digitos(), 1);
        afd.ligar(1, digitos(), 1);
        afd.ligar(1, ".", 2);
        afd.ligar(2, digitos(), 3);
        afd.ligar(3, digitos(), 3);
        afd.marcarAceitacao(1);
        afd.marcarAceitacao(3);
        return afd;
    }

    // cadeia := '"' ( qualquer-menos-aspas-e-barra | '\' ('"' | '\' | 'n' | 't') )* '"'
    //
    // `escape` é um estado próprio: logo depois da barra a máquina espera um dos
    // quatro símbolos de escape, e nenhum outro cabe, nem a aspa que fecharia a
    // cadeia em `dentro`. `fechada` aceita e não tem saída: `"a""b"` são duas
    // cadeias, não uma. A nova linha não pertence ao alfabeto de texto, então
    // uma cadeia não atravessa linhas.
    Afd afdCadeiaDeCaracteres()
    {
        Afd afd("cadeia_de_caracteres", alfabetoDeTexto(), 4, 0);
        afd.nomearEstado(0, "inicio");
        afd.nomearEstado(1, "dentro");
        afd.nomearEstado(2, "escape");
        afd.nomearEstado(3, "fechada");
        afd.ligar(0, "\"", 1);
        afd.ligarTodosExceto(1, "\"\\", 1);
        afd.ligar(1, "\"", 3);
        afd.ligar(1, "\\", 2);
        afd.ligar(2, "\"\\nt", 1);
        afd.marcarAceitacao(3);
        return afd;
    }

    // caractere := '\'' ( qualquer-menos-aspa-simples-e-barra | '\' ('\'' | '\' | 'n') ) '\''
    //
    // Exatamente um caractere entre as aspas, e é isso que pede o estado
    // `um_simbolo`: ele já leu o conteúdo inteiro e só aceita o fecho. Sem ele a
    // máquina aceitaria `'ab'`. O escape conta como um caractere só, e `''` é
    // recusado: de `aberta` a aspa simples não tem transição.
    Afd afdCaractere()
    {
        Afd afd("caractere", alfabetoDeTexto(), 5, 0);
        afd.nomearEstado(0, "inicio");
        afd.nomearEstado(1, "aberta");
        afd.nomearEstado(2, "escape");
        afd.nomearEstado(3, "um_simbolo");
        afd.nomearEstado(4, "fechada");
        afd.ligar(0, "'", 1);
        afd.ligarTodosExceto(1, "'\\", 3);
        afd.ligar(1, "\\", 2);
        afd.ligar(2, "'\\n", 3);
        afd.ligar(3, "'", 4);
        afd.marcarAceitacao(4);
        return afd;
    }

    // comentario := '//' (qualquer-simbolo-menos-nova-linha)*
    //
    // A nova linha É do alfabeto desta máquina, e de `no_comentario` ela não tem
    // transição: é isso que faz o comentário terminar "no fim da linha". A
    // cadeia `// a\nb` é recusada, e o motivo é o certo (símbolo válido, sem
    // transição), porque o lexer lerá o comentário até a nova linha e começará
    // outro token dali. `/` sozinho é a divisão: quem decide entre as duas é o
    // casamento mais longo, no módulo de análise léxica.
    Afd afdComentarioDeLinha()
    {
        Afd afd("comentario_de_linha", alfabetoDeTexto() + "\n", 3, 0);
        afd.nomearEstado(0, "inicio");
        afd.nomearEstado(1, "uma_barra");
        afd.nomearEstado(2, "no_comentario");
        afd.ligar(0, "/", 1);
        afd.ligar(1, "/", 2);
        afd.ligarTodosExceto(2, "\n", 2);
        afd.marcarAceitacao(2);
        return afd;
    }

    // Todos os símbolos e operadores de forma fixa numa máquina só:
    //   + * / % ( ) { } [ ] , ;   = < > !   -   ->   ==  !=  <=  >=   &&  ||
    //
    // Seis estados úteis (o inicial mais cinco), e cada um é uma resposta
    // diferente a "o que ainda pode vir?":
    //   `pode_ganhar_igual`: já é um token (`=`, `<`, `>`, `!`) e ainda aceita um `=`.
    //                        Os quatro se comportam igual, e por isso dividem o estado.
    //   `menos`:             já é um token (`-`) e ainda aceita um `>`.
    //   `e_comercial`:       `&` sozinho NÃO é token (promessa pendente: falta o
    //   `barra_vertical`:    segundo); o mesmo para `|`. Dois estados porque o
    //                        símbolo que cumpre a promessa difere.
    //   `completo`:          aceita e não tem saída. Cobre os operadores de um
    //                        símbolo sem extensão e os de dois símbolos: ambos
    //                        aceitam e não admitem mais nada, então são o mesmo estado.
    //
    // O estado em que pára não diz QUAL token foi lido; isso sai do lexema, por
    // tabela, como nas palavras reservadas.
    Afd afdOperadoresEDelimitadores()
    {
        Afd afd("operadores_e_delimitadores", "+*/%(){}[],;=<>!-&|", 6, 0);
        afd.nomearEstado(0, "inicio");
        afd.nomearEstado(1, "pode_ganhar_igual");
        afd.nomearEstado(2, "menos");
        afd.nomearEstado(3, "e_comercial");
        afd.nomearEstado(4, "barra_vertical");
        afd.nomearEstado(5, "completo");
        afd.ligar(0, "+*/%(){}[],;", 5);
        afd.ligar(0, "=<>!", 1);
        afd.ligar(0, "-", 2);
        afd.ligar(0, "&", 3);
        afd.ligar(0, "|", 4);
        afd.ligar(1, "=", 5);
        afd.ligar(2, ">", 5);
        afd.ligar(3, "&", 5);
        afd.ligar(4, "|", 5);
        afd.marcarAceitacao(1);
        afd.marcarAceitacao(2);
        afd.marcarAceitacao(5);
        return afd;
    }

    std::vector<Afd> automatosDoPinglu()
    {
        std::vector<Afd> automatos;
        automatos.push_back(afdIdentificador());
        automatos.push_back(afdNumero());
        automatos.push_back(afdCadeiaDeCaracteres());
        automatos.push_back(afdCaractere());
        automatos.push_back(afdComentarioDeLinha());
        automatos.push_back(afdOperadoresEDelimitadores());
        return automatos;
    }

} // namespace pinglu
