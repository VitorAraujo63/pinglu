#include "02_regex_pinglu.h"

namespace pinglu
{

    bool Arvore::vazia() const { return raiz == kSemFilho; }

    std::size_t tamanho(const Arvore &arvore) { return arvore.nos.size(); }

    namespace
    {

        // O analisador é recursivo-descendente escrito à mão, uma função por produção da
        // gramática do pattern. Ele constrói a árvore já reduzida: as funções `novo*`
        // abaixo são as únicas que criam nós, e nenhuma delas cria nó de `+` ou `?`,
        // porque esses operadores não existem na árvore de saída. Também não existe
        // `novo*` para coringa — o Pinglu não tem esse operador no núcleo.
        class Analisador
        {
        public:
            explicit Analisador(const std::string &texto) : texto_(texto) {}

            Resultado analisar()
            {
                Resultado resultado;
                const std::size_t raiz = alternancia();
                if (falhou_)
                {
                    resultado.ok = false;
                    resultado.erro = erro_;
                    return resultado;
                }
                if (posicao_ != texto_.size())
                {
                    // Sobrou texto: o caso típico é um `)` sem abertura, que a produção
                    // de grupo não consome e ninguém mais reclama.
                    return falhar("simbolo inesperado apos o fim da expressao");
                }
                resultado.ok = true;
                resultado.arvore.nos = nos_;
                resultado.arvore.raiz = raiz;
                return resultado;
            }

        private:
            // --- construção de nós -------------------------------------------------

            std::size_t novoFolha(const TipoDeNo tipo, const char simbolo)
            {
                No no;
                no.tipo = tipo;
                no.simbolo = simbolo;
                nos_.push_back(no);
                return nos_.size() - 1;
            }

            std::size_t novoBinario(const TipoDeNo tipo, const std::size_t esquerda,
                                    const std::size_t direita)
            {
                No no;
                no.tipo = tipo;
                no.esquerda = esquerda;
                no.direita = direita;
                nos_.push_back(no);
                return nos_.size() - 1;
            }

            std::size_t novoFecho(const std::size_t filho)
            {
                No no;
                no.tipo = TipoDeNo::Fecho;
                no.esquerda = filho;
                nos_.push_back(no);
                return nos_.size() - 1;
            }

            // recorte:inicio clonar-em-vez-de-compartilhar
            // Duplica a subárvore enraizada em `origem` e devolve a raiz da cópia.
            // A redução de `+` precisa da subárvore duas vezes — uma vez direta e outra
            // sob o fecho —, e compartilhar o mesmo índice nos dois lugares produziria
            // um grafo, não uma árvore: a construção de Thompson passaria duas vezes
            // pelos mesmos estados e geraria uma máquina errada.
            std::size_t clonar(const std::size_t origem)
            {
                const No &modelo = nos_[origem];
                No copia;
                copia.tipo = modelo.tipo;
                copia.simbolo = modelo.simbolo;
                // Os filhos precisam ser clonados ANTES de o pai entrar no vetor: o
                // `push_back` invalida a referência `modelo`, então lemos tudo dela
                // primeiro e só depois recorremos.
                const std::size_t esquerdaOriginal = modelo.esquerda;
                const std::size_t direitaOriginal = modelo.direita;
                copia.esquerda =
                    esquerdaOriginal == kSemFilho ? kSemFilho : clonar(esquerdaOriginal);
                copia.direita = direitaOriginal == kSemFilho ? kSemFilho : clonar(direitaOriginal);
                nos_.push_back(copia);
                return nos_.size() - 1;
            }
            // recorte:fim clonar-em-vez-de-compartilhar

            // --- leitura do texto --------------------------------------------------

            bool fim() const { return posicao_ >= texto_.size(); }
            char atual() const { return texto_[posicao_]; }

            Resultado falhar(const std::string &mensagem)
            {
                Resultado resultado;
                resultado.ok = false;
                resultado.erro.posicao = posicao_;
                resultado.erro.mensagem = mensagem;
                return resultado;
            }

            std::size_t erroEm(const std::string &mensagem)
            {
                if (!falhou_)
                {
                    falhou_ = true;
                    erro_.posicao = posicao_;
                    erro_.mensagem = mensagem;
                }
                return kSemFilho;
            }

            // --- produções ---------------------------------------------------------

            // recorte:inicio precedencia-por-descida
            // alternancia := concatenacao ( '|' concatenacao )*
            std::size_t alternancia()
            {
                std::size_t esquerda = concatenacao();
                if (falhou_)
                {
                    return kSemFilho;
                }
                while (!fim() && atual() == '|')
                {
                    ++posicao_;
                    const std::size_t direita = concatenacao();
                    if (falhou_)
                    {
                        return kSemFilho;
                    }
                    esquerda = novoBinario(TipoDeNo::Alternancia, esquerda, direita);
                }
                return esquerda;
            }
            // recorte:fim precedencia-por-descida

            // recorte:inicio associatividade-na-arvore
            // concatenacao := repeticao+
            // A associatividade à esquerda está na FORMA da árvore, e não numa nota
            // escrita à parte: `abc` vira Concat(Concat(a,b),c).
            std::size_t concatenacao()
            {
                if (fim() || atual() == '|' || atual() == ')')
                {
                    return erroEm("esperava uma expressao aqui");
                }
                std::size_t esquerda = repeticao();
                if (falhou_)
                {
                    return kSemFilho;
                }
                while (!fim() && atual() != '|' && atual() != ')')
                {
                    const std::size_t direita = repeticao();
                    if (falhou_)
                    {
                        return kSemFilho;
                    }
                    esquerda = novoBinario(TipoDeNo::Concatenacao, esquerda, direita);
                }
                return esquerda;
            }
            // recorte:fim associatividade-na-arvore

            // recorte:inicio reducao-ao-nucleo
            // repeticao := atomo ( '*' | '+' | '?' )*
            // Aqui moram as duas reduções ao núcleo. Aceitar sufixos repetidos custa um
            // laço e evita recusar `a**`, que é redundante mas não é malformado.
            std::size_t repeticao()
            {
                std::size_t no = atomo();
                if (falhou_)
                {
                    return kSemFilho;
                }
                while (!fim() && (atual() == '*' || atual() == '+' || atual() == '?'))
                {
                    const char sufixo = atual();
                    ++posicao_;
                    if (sufixo == '*')
                    {
                        no = novoFecho(no);
                    }
                    else if (sufixo == '+')
                    {
                        // x+ reduz a x x*  — uma ocorrência obrigatória seguida do fecho.
                        const std::size_t copia = clonar(no);
                        no = novoBinario(TipoDeNo::Concatenacao, no, novoFecho(copia));
                    }
                    else
                    {
                        // x? reduz a (x|ε).
                        no = novoBinario(TipoDeNo::Alternancia, no, novoFolha(TipoDeNo::Vazio, '\0'));
                    }
                }
                return no;
            }
            // recorte:fim reducao-ao-nucleo

            // atomo := SIMBOLO | '\' SIMBOLO | '[' classe ']' | '(' alternancia ')'
            // Sem produção para `.`: o Pinglu não tem coringa (02_nucleo_minimo.md), e
            // um `.` no texto do padrão cai direto no ramo final da função, como
            // qualquer outro símbolo literal — é assim que ele passa a significar
            // "ponto", e não "qualquer símbolo".
            std::size_t atomo()
            {
                if (fim())
                {
                    return erroEm("expressao terminou antes do esperado");
                }
                const char simbolo = atual();
                if (simbolo == '(')
                {
                    ++posicao_;
                    const std::size_t interno = alternancia();
                    if (falhou_)
                    {
                        return kSemFilho;
                    }
                    if (fim() || atual() != ')')
                    {
                        return erroEm("falta o fecha-parenteses do grupo");
                    }
                    ++posicao_;
                    return interno;
                }
                if (simbolo == '[')
                {
                    return classe();
                }
                if (simbolo == '\\')
                {
                    // A barra invertida tira o significado especial do símbolo seguinte.
                    // Sem ela não há como escrever, por exemplo, um colchete ou um
                    // asterisco literal.
                    ++posicao_;
                    if (fim())
                    {
                        return erroEm("barra invertida no fim da expressao, sem o simbolo que ela escapa");
                    }
                    const char escapado = atual();
                    ++posicao_;
                    return novoFolha(TipoDeNo::Simbolo, escapado);
                }
                if (simbolo == '*' || simbolo == '+' || simbolo == '?')
                {
                    return erroEm("operador de repeticao sem expressao a que se aplicar");
                }
                if (simbolo == ')')
                {
                    return erroEm("fecha-parenteses sem abertura correspondente");
                }
                ++posicao_;
                return novoFolha(TipoDeNo::Simbolo, simbolo);
            }

            // recorte:inicio classe-custa-caro
            // classe := '[' ( SIMBOLO | SIMBOLO '-' SIMBOLO )+ ']'
            // Reduz a uma cadeia de alternâncias. É a redução mais cara do conjunto —
            // uma faixa de dez símbolos vira dez folhas e nove nós de alternância —, e a
            // demonstração mede esse custo de propósito. É também a redução que cobre o
            // caso do coringa quando ele aparece por composição (por exemplo "qualquer
            // símbolo que não seja aspas" dentro de uma string): uma alternância grande,
            // não um operador novo.
            std::size_t classe()
            {
                ++posicao_; // consome '['
                std::size_t acumulado = kSemFilho;
                bool algumSimbolo = false;
                while (!fim() && atual() != ']')
                {
                    const char inicio = atual();
                    ++posicao_;
                    char fimDaFaixa = inicio;
                    if (!fim() && atual() == '-' && posicao_ + 1 < texto_.size() &&
                        texto_[posicao_ + 1] != ']')
                    {
                        ++posicao_; // consome '-'
                        fimDaFaixa = atual();
                        ++posicao_;
                        if (static_cast<unsigned char>(fimDaFaixa) < static_cast<unsigned char>(inicio))
                        {
                            return erroEm("faixa invertida na classe de simbolos");
                        }
                    }
                    for (int codigo = static_cast<unsigned char>(inicio);
                         codigo <= static_cast<unsigned char>(fimDaFaixa); ++codigo)
                    {
                        const std::size_t folha =
                            novoFolha(TipoDeNo::Simbolo, static_cast<char>(codigo));
                        acumulado = acumulado == kSemFilho
                                        ? folha
                                        : novoBinario(TipoDeNo::Alternancia, acumulado, folha);
                    }
                    algumSimbolo = true;
                }
                if (fim())
                {
                    return erroEm("falta o fecha-colchetes da classe de simbolos");
                }
                ++posicao_; // consome ']'
                if (!algumSimbolo)
                {
                    return erroEm("classe de simbolos vazia");
                }
                return acumulado;
            }
            // recorte:fim classe-custa-caro

            const std::string &texto_;
            std::size_t posicao_ = 0;
            std::vector<No> nos_;
            bool falhou_ = false;
            ErroDeSintaxe erro_;
        };

        // recorte:inicio forma-prefixa-comparavel
        void escreverPrefixa(const Arvore &arvore, const std::size_t indice, std::string &saida)
        {
            if (indice == kSemFilho)
            {
                return;
            }
            const No &no = arvore.nos[indice];
            switch (no.tipo)
            {
            case TipoDeNo::Simbolo:
                saida += '\'';
                saida += no.simbolo;
                saida += '\'';
                return;
            case TipoDeNo::Vazio:
                saida += "vazio";
                return;
            case TipoDeNo::Concatenacao:
                saida += "concat(";
                break;
            case TipoDeNo::Alternancia:
                saida += "alt(";
                break;
            case TipoDeNo::Fecho:
                saida += "fecho(";
                break;
            }
            escreverPrefixa(arvore, no.esquerda, saida);
            if (no.direita != kSemFilho)
            {
                saida += ", ";
                escreverPrefixa(arvore, no.direita, saida);
            }
            saida += ')';
        }
        // recorte:fim forma-prefixa-comparavel

    } // namespace

    Resultado analisarExpressao(const std::string &expressao)
    {
        if (expressao.empty())
        {
            Resultado resultado;
            resultado.ok = false;
            resultado.erro.posicao = 0;
            resultado.erro.mensagem = "expressao vazia";
            return resultado;
        }
        Analisador analisador(expressao);
        return analisador.analisar();
    }

    std::string formatarArvore(const Arvore &arvore)
    {
        if (arvore.vazia())
        {
            return "(arvore vazia)";
        }
        std::string saida;
        escreverPrefixa(arvore, arvore.raiz, saida);
        return saida;
    }

    std::string formatarErro(const std::string &expressao, const ErroDeSintaxe &erro)
    {
        std::string saida = "  " + expressao + '\n';
        saida += "  ";
        // A posição é contada em símbolos desde zero; o cursor vai exatamente sob o
        // símbolo recusado. Uma mensagem sem esta linha obriga quem escreveu o
        // pattern a procurar o defeito, que é justamente o trabalho que ela deveria
        // poupar.
        for (std::size_t i = 0; i < erro.posicao && i < expressao.size(); ++i)
        {
            saida += ' ';
        }
        saida += "^ ";
        saida += erro.mensagem;
        saida += " (posicao " + std::to_string(erro.posicao) + ")";
        return saida;
    }

} // namespace pinglu