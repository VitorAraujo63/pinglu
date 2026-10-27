#include "04_afn.h"

#include <fstream>
#include <stdexcept>

namespace pinglu
{

    namespace
    {

        // Um símbolo como o leitor consegue ver no traço: o byte zero, a nova
        // linha e os bytes fora do ASCII imprimível não podem sair crus.
        std::string simboloVisivel(const char simbolo)
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

        std::string conjuntoComoTexto(const std::vector<Estado> &conjunto)
        {
            if (conjunto.empty())
            {
                return "{ }";
            }
            std::string texto = "{ ";
            for (std::size_t i = 0; i < conjunto.size(); ++i)
            {
                if (i > 0)
                {
                    texto += ", ";
                }
                texto += std::to_string(conjunto[i]);
            }
            return texto + " }";
        }

    } // namespace

    // --- a máquina ---------------------------------------------------------------

    Estado Afn::novoEstado()
    {
        transicoes_.emplace_back();
        return transicoes_.size() - 1;
    }

    void Afn::adicionarTransicao(const Estado origem, const int rotulo, const Estado destino)
    {
        if (origem >= transicoes_.size() || destino >= transicoes_.size())
        {
            throw std::invalid_argument("Afn: transicao entre estados que nao existem.");
        }
        if (rotulo < kEpsilon || rotulo > 255)
        {
            throw std::invalid_argument("Afn: o rotulo deve ser um byte (0..255) ou kEpsilon.");
        }
        transicoes_[origem].push_back(TransicaoAfn{rotulo, destino});
    }

    void Afn::definirInicial(const Estado estado)
    {
        if (estado >= transicoes_.size())
        {
            throw std::invalid_argument("Afn: estado inicial inexistente.");
        }
        inicial_ = estado;
    }

    void Afn::definirAceitacao(const Estado estado)
    {
        if (estado >= transicoes_.size())
        {
            throw std::invalid_argument("Afn: estado de aceitacao inexistente.");
        }
        aceitacao_ = estado;
    }

    Estado Afn::inicial() const { return inicial_; }

    Estado Afn::aceitacao() const { return aceitacao_; }

    std::size_t Afn::quantidadeDeEstados() const { return transicoes_.size(); }

    std::size_t Afn::quantidadeDeTransicoes() const
    {
        std::size_t total = 0;
        for (const std::vector<TransicaoAfn> &saidas : transicoes_)
        {
            total += saidas.size();
        }
        return total;
    }

    // recorte:inicio contar-transicoes-vazias
    // Percorre as transições a cada chamada, em vez de ler um contador
    // incrementado em adicionarTransicao(): o contador acertaria só enquanto
    // transição apenas entrasse na máquina, e registraria inserções, não o que
    // a máquina tem, no dia em que alguma operação passasse a removê-las.
    std::size_t Afn::quantidadeDeTransicoesVazias() const
    {
        std::size_t total = 0;
        for (const std::vector<TransicaoAfn> &saidas : transicoes_)
        {
            for (const TransicaoAfn &transicao : saidas)
            {
                if (transicao.rotulo == kEpsilon)
                {
                    ++total;
                }
            }
        }
        return total;
    }
    // recorte:fim contar-transicoes-vazias

    std::vector<Estado> Afn::estadosSemChegada() const
    {
        std::vector<char> recebe(transicoes_.size(), 0);
        for (const std::vector<TransicaoAfn> &saidas : transicoes_)
        {
            for (const TransicaoAfn &transicao : saidas)
            {
                recebe[transicao.destino] = 1;
            }
        }
        std::vector<Estado> semChegada;
        for (Estado estado = 0; estado < transicoes_.size(); ++estado)
        {
            if (recebe[estado] == 0 && estado != inicial_)
            {
                semChegada.push_back(estado);
            }
        }
        return semChegada;
    }

    // --- a simulação -------------------------------------------------------------

    // recorte:inicio fecho-vazio-com-marcacao
    std::vector<Estado> Afn::fechoVazio(const std::vector<Estado> &conjunto) const
    {
        // Busca em profundidade sobre as transições vazias, com uma marca de
        // visitado por estado. A marca é o que garante o término quando existe
        // ciclo só de transições vazias, o que acontece quando o interior de um
        // fecho pode ser atravessado sem ler símbolo: `a**`, `(a?)*`, `(a*)*`.
        // Em `a*` o retorno passa pela transição que lê `a`, o ciclo não é todo
        // vazio, e a falta da marca não se manifesta.
        std::vector<char> visitado(transicoes_.size(), 0);
        std::vector<Estado> pilha;
        std::vector<Estado> resultado;

        for (const Estado estado : conjunto)
        {
            if (estado >= transicoes_.size())
            {
                throw std::invalid_argument("Afn::fechoVazio: estado inexistente no conjunto.");
            }
            if (visitado[estado] == 0)
            {
                visitado[estado] = 1;
                pilha.push_back(estado);
                resultado.push_back(estado);
            }
        }

        while (!pilha.empty())
        {
            const Estado atual = pilha.back();
            pilha.pop_back();
            for (const TransicaoAfn &transicao : transicoes_[atual])
            {
                if (transicao.rotulo != kEpsilon || visitado[transicao.destino] != 0)
                {
                    continue;
                }
                visitado[transicao.destino] = 1;
                pilha.push_back(transicao.destino);
                resultado.push_back(transicao.destino);
            }
        }
        return resultado;
    }
    // recorte:fim fecho-vazio-com-marcacao

    Simulacao Afn::simular(const std::string &cadeia) const
    {
        Simulacao simulacao;
        if (transicoes_.empty())
        {
            return simulacao; // sem estados não há por onde começar: recusa
        }

        // recorte:inicio conjunto-ativo-comeca-no-fecho
        // O conjunto de partida é o fecho vazio do estado inicial, e NÃO o
        // estado inicial sozinho. Esquecer esse fecho passa em todo teste cujo
        // pattern comece por símbolo e só falha quando a máquina tem transição
        // vazia logo na entrada: nos blocos de alternância e de fecho.
        std::vector<Estado> ativos = fechoVazio({inicial_});
        simulacao.passos.push_back(PassoDaSimulacao{true, '\0', ativos});
        simulacao.maiorConjuntoAtivo = ativos.size();
        // recorte:fim conjunto-ativo-comeca-no-fecho

        // `presente` responde "este estado já entrou em `proximos`?" com uma
        // consulta, sem busca. É alocada uma vez e, a cada símbolo, só se
        // zeram as posições que entraram. O conjunto em si é o vetor
        // `proximos`, que guarda a ordem de inserção e é o que se percorre.
        std::vector<char> presente(transicoes_.size(), 0);

        for (const char simbolo : cadeia)
        {
            const int byte = static_cast<unsigned char>(simbolo);
            std::vector<Estado> proximos;
            for (const Estado estado : ativos)
            {
                for (const TransicaoAfn &transicao : transicoes_[estado])
                {
                    // kEpsilon é -1 e `byte` é 0..255: um byte zero na entrada
                    // jamais casa com uma transição vazia.
                    if (transicao.rotulo == byte && presente[transicao.destino] == 0)
                    {
                        presente[transicao.destino] = 1;
                        proximos.push_back(transicao.destino);
                    }
                }
            }
            for (const Estado estado : proximos)
            {
                presente[estado] = 0;
            }

            ativos = fechoVazio(proximos);
            simulacao.passos.push_back(PassoDaSimulacao{false, simbolo, ativos});
            if (ativos.size() > simulacao.maiorConjuntoAtivo)
            {
                simulacao.maiorConjuntoAtivo = ativos.size();
            }
            if (ativos.empty())
            {
                break; // sem estado ativo, nenhum símbolo seguinte reativa algo
            }
        }

        // Aceitação por EXISTÊNCIA de percurso: basta que o estado de aceitação
        // esteja entre os ativos ao fim. Os outros percursos podem ter morrido.
        for (const Estado estado : ativos)
        {
            if (estado == aceitacao_)
            {
                simulacao.aceitou = true;
                break;
            }
        }
        return simulacao;
    }

    bool Afn::aceita(const std::string &cadeia) const { return simular(cadeia).aceitou; }

    std::string Afn::formatarSimulacao(const std::string &cadeia, const Simulacao &simulacao) const
    {
        std::string eco;
        for (const char simbolo : cadeia)
        {
            eco += simbolo == ' ' ? std::string(" ") : simboloVisivel(simbolo);
        }
        std::string texto = "  cadeia: \"" + eco + "\"\n";
        for (const PassoDaSimulacao &passo : simulacao.passos)
        {
            texto += "    ";
            texto += passo.ehPartida ? "inicio " : "apos '" + simboloVisivel(passo.simboloLido) + "' ";
            texto += "-> " + conjuntoComoTexto(passo.ativos) + "\n";
        }
        texto += std::string("  resultado: ") + (simulacao.aceitou ? "ACEITA" : "RECUSA") +
                 " | maior conjunto ativo: " + std::to_string(simulacao.maiorConjuntoAtivo) + "\n";
        return texto;
    }

    // --- a construção de Thompson ------------------------------------------------

    namespace
    {

        // Um caso por tipo de nó, e a recursão faz a composição. Nenhum caso olha
        // dentro dos blocos que recebe: usa só a entrada e a saída de cada um.
        // A ordem de criação dos estados (filho esquerdo, filho direito, depois
        // os estados do próprio nó) é a que a construção feita à mão no material
        // do módulo segue, e é por ela que a numeração do traço pode ser
        // conferida contra a conta no papel.
        Bloco construir(const Arvore &arvore, const std::size_t indice, Afn &afn)
        {
            if (indice >= arvore.nos.size())
            {
                throw std::invalid_argument("construirThompson: arvore malformada (no inexistente).");
            }
            const No &no = arvore.nos[indice];

            switch (no.tipo)
            {
            // recorte:inicio thompson-caso-base
            case TipoDeNo::Simbolo:
            case TipoDeNo::Vazio:
            {
                // As duas folhas têm a mesma forma — dois estados e uma
                // transição —, e diferem só no rótulo dela.
                const Estado entrada = afn.novoEstado();
                const Estado saida = afn.novoEstado();
                const int rotulo =
                    no.tipo == TipoDeNo::Simbolo ? static_cast<unsigned char>(no.simbolo) : kEpsilon;
                afn.adicionarTransicao(entrada, rotulo, saida);
                return Bloco{entrada, saida};
            }
            // recorte:fim thompson-caso-base
            // recorte:inicio thompson-concatenacao
            case TipoDeNo::Concatenacao:
            {
                // A saída do primeiro passa a alimentar a entrada do segundo por
                // uma transição vazia, e os estados NÃO se fundem: fundir
                // funcionaria aqui e destruiria a propriedade das pontas, que é o
                // que mantém a construção mecânica.
                const Bloco esquerdo = construir(arvore, no.esquerda, afn);
                const Bloco direito = construir(arvore, no.direita, afn);
                afn.adicionarTransicao(esquerdo.saida, kEpsilon, direito.entrada);
                return Bloco{esquerdo.entrada, direito.saida};
            }
            // recorte:fim thompson-concatenacao
            // recorte:inicio thompson-alternancia
            case TipoDeNo::Alternancia:
            {
                const Bloco esquerdo = construir(arvore, no.esquerda, afn);
                const Bloco direito = construir(arvore, no.direita, afn);
                const Estado entrada = afn.novoEstado();
                const Estado saida = afn.novoEstado();
                afn.adicionarTransicao(entrada, kEpsilon, esquerdo.entrada);
                afn.adicionarTransicao(entrada, kEpsilon, direito.entrada);
                afn.adicionarTransicao(esquerdo.saida, kEpsilon, saida);
                afn.adicionarTransicao(direito.saida, kEpsilon, saida);
                return Bloco{entrada, saida};
            }
            // recorte:fim thompson-alternancia
            // recorte:inicio thompson-fecho
            case TipoDeNo::Fecho:
            {
                const Bloco interno = construir(arvore, no.esquerda, afn);
                const Estado entrada = afn.novoEstado();
                const Estado saida = afn.novoEstado();
                afn.adicionarTransicao(entrada, kEpsilon, interno.entrada);       // uma vez ou mais
                afn.adicionarTransicao(entrada, kEpsilon, saida);                  // zero vezes
                afn.adicionarTransicao(interno.saida, kEpsilon, interno.entrada); // de novo
                afn.adicionarTransicao(interno.saida, kEpsilon, saida);            // basta
                return Bloco{entrada, saida};
            }
                // recorte:fim thompson-fecho
            }

            // O switch cobre os cinco tipos de nó. O -Wall do projeto acusa um
            // `case` que faltar (em GCC e Clang; o MSVC não acusa por padrão), e
            // este throw é a rede para um tipo de nó que um dia entre na árvore
            // sem regra aqui: falha alta, em vez de um bloco inventado.
            throw std::logic_error("construirThompson: tipo de no sem regra de construcao.");
        }

    } // namespace

    Afn construirThompson(const Arvore &arvore)
    {
        Afn afn;
        if (arvore.vazia())
        {
            const Estado unico = afn.novoEstado();
            afn.definirInicial(unico);
            afn.definirAceitacao(unico);
            return afn;
        }
        const Bloco raiz = construir(arvore, arvore.raiz, afn);
        afn.definirInicial(raiz.entrada);
        afn.definirAceitacao(raiz.saida);
        return afn;
    }

    // --- a bateria de casos conferidos à mão -------------------------------------

    LeituraDeCasos interpretarCasos(std::istream &fluxo, const std::string &origem)
    {
        LeituraDeCasos leitura;
        std::string linha;
        std::size_t numero = 0;
        while (std::getline(fluxo, linha))
        {
            ++numero;
            if (!linha.empty() && linha.back() == '\r')
            {
                linha.pop_back(); // fim de linha CRLF
            }
            if (linha.empty() || linha[0] == '#')
            {
                continue;
            }

            const std::string local = origem + ":" + std::to_string(numero);
            const std::size_t primeiro = linha.find('\t');
            const std::size_t segundo =
                primeiro == std::string::npos ? std::string::npos : linha.find('\t', primeiro + 1);
            if (segundo == std::string::npos)
            {
                leitura.erros.push_back(
                    local + ": esperava tres campos separados por tabulacao (expressao, cadeia, veredito)");
                continue;
            }
            if (linha.find('\t', segundo + 1) != std::string::npos)
            {
                leitura.erros.push_back(local + ": campos demais; sao tres, separados por tabulacao");
                continue;
            }

            CasoDeTeste caso;
            caso.expressao = linha.substr(0, primeiro);
            caso.cadeia = linha.substr(primeiro + 1, segundo - primeiro - 1);
            const std::string veredito = linha.substr(segundo + 1);
            if (veredito == "aceita")
            {
                caso.esperado = true;
            }
            else if (veredito == "recusa")
            {
                caso.esperado = false;
            }
            else
            {
                // Tratar o que não é "aceita" como recusa faria um erro de digitação
                // virar um caso de recusa, e o caso passaria sem verificar o que
                // se queria.
                leitura.erros.push_back(local + ": veredito \"" + veredito +
                                        "\" invalido; so 'aceita' ou 'recusa'");
                continue;
            }
            caso.origem = local;
            leitura.casos.push_back(caso);
        }
        return leitura;
    }

    LeituraDeCasos lerCasos(const std::string &caminho)
    {
        std::ifstream arquivo(caminho);
        if (!arquivo)
        {
            LeituraDeCasos leitura;
            leitura.erros.push_back(caminho + ": nao foi possivel abrir o arquivo de casos");
            return leitura;
        }
        return interpretarCasos(arquivo, caminho);
    }

    ResultadoDaBateria rodarBateria(const std::vector<CasoDeTeste> &casos)
    {
        ResultadoDaBateria resultado;
        for (const CasoDeTeste &caso : casos)
        {
            ++resultado.total;
            const Resultado leitura = analisarExpressao(caso.expressao);
            if (!leitura.ok)
            {
                resultado.falhas.push_back(caso.origem + ": pattern recusado pela leitura - " +
                                           leitura.erro.mensagem);
                continue;
            }
            const Afn afn = construirThompson(leitura.arvore);
            const bool obtido = afn.aceita(caso.cadeia);
            if (obtido == caso.esperado)
            {
                ++resultado.passaram;
                continue;
            }
            resultado.falhas.push_back(caso.origem + ": /" + caso.expressao + "/ sobre \"" +
                                       caso.cadeia + "\" - esperado " +
                                       (caso.esperado ? "aceita" : "recusa") + ", obtido " +
                                       (obtido ? "aceita" : "recusa"));
        }
        return resultado;
    }

} // namespace pinglu
