#include "InterfaceGrafica.h"
#include "JanelaInserir.h"
#include "JanelaRemover.h"
#include "JanelaReservar.h"
#include "SFMLCompat.h"

#include <iostream>
#include <cstdio>
#include <algorithm>

InterfaceGrafica::InterfaceGrafica()
    : altura(720),
      largura(1280),
      titulo("WareSim - Gerenciamento de Estoque"),
      janela(WareSimUI::criarVideoMode(static_cast<unsigned int>(largura), static_cast<unsigned int>(altura)), titulo, sf::Style::Default),
      largurasColunas{ 75.f, 85.f, 210.f, 85.f, 125.f, 165.f, 90.f, 185.f, 228.f }
{
    if (!WareSimUI::carregarFonte(fonte, "arial.ttf"))
    {
        std::cerr << "Aviso: Nao foi possivel carregar a fonte arial.ttf." << std::endl;
    }

    atualizarDimensoes(static_cast<unsigned int>(largura), static_cast<unsigned int>(altura));

    // [UC003] 1- O ator inicializa o sistema -> verifica itens e sinaliza reservas e validades
    if (!estoque.temConexao())
    {
        erroBancoDeDados = true;
        mensagemErroBanco = "Erro [RN001/RN002]: Sem acesso ao banco de dados (armazem.db)!";
        std::cerr << mensagemErroBanco << std::endl;
    }
    else
    {
        verEstoque();
    }
}

InterfaceGrafica::~InterfaceGrafica()
{
}

void InterfaceGrafica::atualizarDimensoes(unsigned int novaLargura, unsigned int novaAltura)
{
    largura = static_cast<int>(std::max(640u, novaLargura));
    altura = static_cast<int>(std::max(420u, novaAltura));

    sf::View view(sf::FloatRect(0.f, 0.f, static_cast<float>(largura), static_cast<float>(altura)));
    janela.setView(view);

    // Recalcula proporcionalmente as 9 colunas com base na largura atual da tabela
    const float margem = 16.f;
    const float larguraTabelaUtil = std::max(580.f, static_cast<float>(largura) - 2.f * margem - 12.f);

    // Proporcoes ideais das 9 colunas (soma = 1.0)
    static const float proporcoes[9] = {
        0.065f, // Lote
        0.070f, // Id bebida
        0.180f, // Nome
        0.070f, // Volume
        0.105f, // Categoria
        0.135f, // Distribuidor
        0.075f, // Preco
        0.140f, // Validade
        0.160f  // Destino (Reserva)
    };

    for (int i = 0; i < 9; ++i)
    {
        largurasColunas[i] = std::floor(larguraTabelaUtil * proporcoes[i]);
    }
}

void InterfaceGrafica::alternarTelaCheia()
{
    modoTelaCheia = !modoTelaCheia;
    if (modoTelaCheia)
    {
        sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
        janela.create(desktop, titulo, sf::Style::Fullscreen);
        atualizarDimensoes(desktop.width, desktop.height);
    }
    else
    {
        janela.create(WareSimUI::criarVideoMode(1280u, 720u), titulo, sf::Style::Default);
        atualizarDimensoes(1280u, 720u);
    }
}

// [REU003] Cada linha do mesmo lote deve conter a mesma cor.
// [REU004] A interface deve apresentar boa visibilidade.
sf::Color InterfaceGrafica::obterCorDoLote(const std::string& lote)
{
    if (lote.empty() || lote == "-")
        return sf::Color(250, 245, 235); // Cor neutra para itens individuais sem lote (avulsos)

    auto it = coresPorLote.find(lote);
    if (it != coresPorLote.end())
        return it->second;

    static const sf::Color paletaPastel[] = {
        sf::Color(255, 243, 205), // Creme suave
        sf::Color(214, 238, 255), // Azul claro
        sf::Color(220, 247, 225), // Verde menta
        sf::Color(242, 224, 255), // Lavanda
        sf::Color(255, 226, 209), // Pessego
        sf::Color(210, 245, 245), // Ciano pastel
        sf::Color(255, 222, 235), // Rosa claro
        sf::Color(238, 245, 205), // Lima suave
        sf::Color(232, 228, 216), // Areia
        sf::Color(225, 232, 255)  // Pervinca
    };

    const size_t numCores = sizeof(paletaPastel) / sizeof(paletaPastel[0]);
    sf::Color cor = paletaPastel[coresPorLote.size() % numCores];
    coresPorLote[lote] = cor;
    return cor;
}

void InterfaceGrafica::verEstoque()
{
    modoHistorico = false;
    scrollOffset = 0;
    atualizar_database();
}

void InterfaceGrafica::verRemocoes()
{
    // [UC002] Visualizar historico de remocao (Figura 4 & Quadro 2)
    modoHistorico = true;
    scrollOffset = 0;
    bool erro = false;
    listaHistoricoCache = estoque.consultarHistorico(erro);
    if (erro)
    {
        erroBancoDeDados = true;
        mensagemErroBanco = "Erro [UC002/RN001]: Sem acesso ao banco de dados de remocoes!";
    }
    else
    {
        erroBancoDeDados = false;
        mensagemErroBanco.clear();
    }
}

void InterfaceGrafica::acessarOpcaoInsercao()
{
    JanelaInserir janelaInserir(&estoque);
    janelaInserir.executar();
    if (janelaInserir.teveSucesso())
        mensagemSucessoOperacao = janelaInserir.getMensagemStatus();
    verEstoque();
}

void InterfaceGrafica::acessarOpcaoRemocao()
{
    JanelaRemover janelaRemover(&estoque);
    janelaRemover.executar();
    if (janelaRemover.teveSucesso())
        mensagemSucessoOperacao = janelaRemover.getMensagemStatus();
    if (modoHistorico)
        verRemocoes();
    else
        verEstoque();
}

void InterfaceGrafica::acessarOpcaoReserva()
{
    JanelaReservar janelaReservar(&estoque);
    janelaReservar.executar();
    if (janelaReservar.teveSucesso())
        mensagemSucessoOperacao = janelaReservar.getMensagemStatus();
    verEstoque();
}

void InterfaceGrafica::atualizar_database()
{
    listaItensCache.clear();
    bool erro = false;
    std::vector<Bebida> bebidas = estoque.consultarEstoque(erro);
    if (erro)
    {
        erroBancoDeDados = true;
        mensagemErroBanco = "Erro [UC003/RN001]: Nao foi possivel acessar o banco de dados!";
        return;
    }

    erroBancoDeDados = false;
    mensagemErroBanco.clear();

    for (const auto& b : bebidas)
    {
        ItemEstoque item;
        item.lote = b.getLote().empty() ? "-" : b.getLote();
        item.id = std::to_string(b.getId());
        item.nome = b.getNome();

        char bufVol[32];
        std::snprintf(bufVol, sizeof(bufVol), "%d ml", b.getVolume());
        item.volume = bufVol;

        item.categoria = b.getCategoria();
        item.distribuidor = b.getDistribuidor();

        char bufPreco[32];
        std::snprintf(bufPreco, sizeof(bufPreco), "R$ %.2f", b.getPreco());
        item.preco = bufPreco;

        // [RF009] e [RNF002] Sinalizacao de itens vencidos e a <= 1 mes do vencimento
        item.statusValidade = b.getStatusValidade();
        if (item.statusValidade == 2)
            item.data_validade = "[VENCIDO] " + b.getDataValidade();
        else if (item.statusValidade == 1)
            item.data_validade = "[! <=1m] " + b.getDataValidade();
        else
            item.data_validade = b.getDataValidade();

        // [RF010] e [RNF003] Sinalizacao das reservas por item com uma coluna de destino
        item.flagReserva = b.isReservado();
        item.reservado = b.isReservado() ? b.getDestinoReserva() : "";

        // Registra a cor do lote na ordem de aparicao
        obterCorDoLote(item.lote);

        listaItensCache.push_back(item);
    }
}

void InterfaceGrafica::executar()
{
    while (janela.isOpen())
    {
        WareSimUI::EventoUI ev;
        while (WareSimUI::pollEvento(janela, ev))
        {
            if (ev.tipo == WareSimUI::TipoEvento::Fechar)
            {
                WareSimUI::tocarSomBotao();
                sf::sleep(sf::milliseconds(90));
                janela.close();
            }
            else if (ev.tipo == WareSimUI::TipoEvento::Redimensionar)
            {
                atualizarDimensoes(ev.novaLargura, ev.novaAltura);
            }
            else if (ev.tipo == WareSimUI::TipoEvento::TeclaPressionada)
            {
                if (ev.teclaEsc)
                {
                    // Requisito 4: permitir 'ESC' fechar o programa
                    WareSimUI::tocarSomBotao();
                    sf::sleep(sf::milliseconds(100));
                    janela.close();
                }
                else if (ev.teclaF11)
                {
                    // Requisito 3: alternar entre janela e fullscreen
                    WareSimUI::tocarSomBotao();
                    alternarTelaCheia();
                }
            }
            else if (ev.tipo == WareSimUI::TipoEvento::CliqueMouse && ev.botaoEsquerdo)
            {
                selecionar();
            }
            else if (ev.tipo == WareSimUI::TipoEvento::RodaMouse)
            {
                int totalLinhas = modoHistorico
                    ? static_cast<int>(listaHistoricoCache.size())
                    : static_cast<int>(listaItensCache.size());
                int maxScroll = std::max(0, totalLinhas - 1);
                if (ev.deltaRoda < 0)
                    scrollOffset = std::min(maxScroll, scrollOffset + 1);
                else if (ev.deltaRoda > 0)
                    scrollOffset = std::max(0, scrollOffset - 1);
            }
        }

        if (janela.isOpen())
            renderizarMenu();
    }
}

void InterfaceGrafica::renderizar()
{
    renderizarMenu();
}

void InterfaceGrafica::renderizarMenu()
{
    janela.clear(sf::Color(255, 227, 163));

    // Barra superior responsiva com 6 botoes (incluindo Tela Cheia / Janela F11)
    const float margem = 16.f;
    const float topo = 12.f;
    const float alturaBotao = 56.f;
    const int numBotoes = 6;
    const float larguraTotalBotoes = std::max(600.f, static_cast<float>(largura) - 2.f * margem);
    const float larguraCadaBotao = larguraTotalBotoes / static_cast<float>(numBotoes);

    const std::string nomes[] = {
        "Listar/Recarregar",
        "Inserir",
        "Remover",
        "Reservar",
        "Historico",
        modoTelaCheia ? "Janela (F11)" : "Tela Cheia (F11)"
    };

    sf::Vector2f posMouse = WareSimUI::obterPosicaoMouse(janela);
    float x = margem;
    unsigned int tamFonteBotao = (largura < 1050) ? 14u : 17u;

    for (int i = 0; i < numBotoes; ++i)
    {
        sf::RectangleShape botao({ larguraCadaBotao, alturaBotao });
        botao.setPosition({ x, topo });

        bool abaAtiva = (i == 0 && !modoHistorico) || (i == 4 && modoHistorico) || (i == 5 && modoTelaCheia);
        sf::Color corNormal = abaAtiva ? sf::Color(245, 205, 145) : sf::Color(255, 222, 179);
        sf::Color corHover(235, 202, 157);
        sf::Color corClique(255, 235, 205);

        bool mouseSobre =
            posMouse.x >= x &&
            posMouse.x <= x + larguraCadaBotao &&
            posMouse.y >= topo &&
            posMouse.y <= topo + alturaBotao;

        if (mouseSobre && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
        {
            botao.setFillColor(corClique);
        }
        else if (mouseSobre)
        {
            botao.setFillColor(corHover);
        }
        else
        {
            botao.setFillColor(corNormal);
        }

        botao.setOutlineColor(sf::Color(222, 154, 64));
        botao.setOutlineThickness(2.f);

        janela.draw(botao);

        if (WareSimUI::fonteCarregada(fonte))
        {
            sf::String txtBotaoQuebrado = WareSimUI::quebrarTexto(
                fonte, nomes[i], tamFonteBotao, larguraCadaBotao - 14.f, nullptr, abaAtiva
            );
            sf::Text texto = WareSimUI::criarTexto(fonte, txtBotaoQuebrado, tamFonteBotao);
            texto.setFillColor(sf::Color::Black);
            if (abaAtiva)
                texto.setStyle(sf::Text::Bold);
            texto.setPosition({ x + 10.f, topo + 16.f });

            janela.draw(texto);
        }

        x += larguraCadaBotao;
    }

    // Area da tabela responsiva
    const float tabelaX = margem;
    const float tabelaY = 82.f;
    const float tabelaLargura = std::max(600.f, static_cast<float>(largura) - 2.f * margem);
    const float tabelaAltura = std::max(260.f, static_cast<float>(altura) - tabelaY - 46.f);

    // Fundo da tabela
    sf::RectangleShape fundoTabela({ tabelaLargura, tabelaAltura });
    fundoTabela.setPosition({ tabelaX, tabelaY });
    fundoTabela.setFillColor(sf::Color(255, 222, 179));
    fundoTabela.setOutlineColor(sf::Color(222, 154, 64));
    fundoTabela.setOutlineThickness(2.f);
    janela.draw(fundoTabela);

    if (erroBancoDeDados)
    {
        sf::RectangleShape caixaErro({ tabelaLargura - 80.f, 80.f });
        caixaErro.setPosition({ tabelaX + 40.f, tabelaY + 80.f });
        caixaErro.setFillColor(sf::Color(255, 210, 210));
        caixaErro.setOutlineColor(sf::Color(190, 40, 40));
        caixaErro.setOutlineThickness(2.f);
        janela.draw(caixaErro);

        if (WareSimUI::fonteCarregada(fonte))
        {
            sf::String msgUtf8 = sf::String::fromUtf8(mensagemErroBanco.begin(), mensagemErroBanco.end());
            sf::String msgQuebrada = WareSimUI::quebrarTexto(fonte, msgUtf8, 17u, tabelaLargura - 120.f);
            sf::Text txtErro = WareSimUI::criarTexto(fonte, msgQuebrada, 17);
            txtErro.setFillColor(sf::Color(160, 0, 0));
            txtErro.setStyle(sf::Text::Bold);
            txtErro.setPosition({ tabelaX + 60.f, tabelaY + 100.f });
            janela.draw(txtErro);
        }
        janela.display();
        return;
    }

    if (!modoHistorico)
    {
        // Cabecalho da tabela de Estoque ([UC003], [RNF003])
        const std::string colunas[] = {
            "Lote",
            "Id bebida",
            "Nome",
            "Volume",
            "Categoria",
            "Distribuidor",
            "Preco",
            "Validade",
            "Destino (Reserva)"
        };

        const float alturaCabecalho = 36.f;
        float colunaX = tabelaX;
        unsigned int tamFonteCab = (largura < 1050) ? 13u : 15u;

        for (int i = 0; i < 9; ++i)
        {
            sf::RectangleShape divisoria({ 2.f, alturaCabecalho });
            divisoria.setPosition({ colunaX, tabelaY });
            divisoria.setFillColor(sf::Color(222, 154, 64));
            janela.draw(divisoria);

            if (WareSimUI::fonteCarregada(fonte))
            {
                sf::String colQuebrada = WareSimUI::quebrarTexto(
                    fonte, colunas[i], tamFonteCab, largurasColunas[i] - 10.f, nullptr, true
                );
                sf::Text textoColuna = WareSimUI::criarTexto(fonte, colQuebrada, tamFonteCab);
                textoColuna.setFillColor(sf::Color::Black);
                textoColuna.setStyle(sf::Text::Bold);
                textoColuna.setPosition({ colunaX + 5.f, tabelaY + 4.f });
                janela.draw(textoColuna);
            }

            colunaX += largurasColunas[i];
        }

        sf::RectangleShape linhaCabecalho({ tabelaLargura, 2.f });
        linhaCabecalho.setPosition({ tabelaX, tabelaY + alturaCabecalho });
        linhaCabecalho.setFillColor(sf::Color(222, 154, 64));
        janela.draw(linhaCabecalho);

        renderizarLista();
    }
    else
    {
        // Cabecalho e linhas da tabela de Historico de Remocoes ([UC002], [RF012], Quadro 9)
        renderizarHistorico();
    }

    // Barra de rolagem funcional
    const float barraX = tabelaX + tabelaLargura - 10.f;
    const float trilhoAltura = tabelaAltura - 8.f;

    sf::RectangleShape trilho({ 6.f, trilhoAltura });
    trilho.setPosition({ barraX, tabelaY + 4.f });
    trilho.setFillColor(sf::Color(240, 240, 240));
    janela.draw(trilho);

    int totalItens = modoHistorico
        ? static_cast<int>(listaHistoricoCache.size())
        : static_cast<int>(listaItensCache.size());
    int linhasEstimadas = std::max(1, static_cast<int>((tabelaAltura - 36.f) / 55.f));
    float proporcao = (totalItens > linhasEstimadas)
        ? static_cast<float>(linhasEstimadas) / static_cast<float>(totalItens)
        : 1.f;
    float indicadorAltura = std::max(30.f, trilhoAltura * proporcao);
    int maxScroll = std::max(1, totalItens - 1);
    float deslocamentoY = (totalItens > 1)
        ? (trilhoAltura - indicadorAltura) * (static_cast<float>(scrollOffset) / static_cast<float>(maxScroll))
        : 0.f;

    sf::RectangleShape indicador({ 6.f, indicadorAltura });
    indicador.setPosition({ barraX, tabelaY + 4.f + deslocamentoY });
    indicador.setFillColor(sf::Color(150, 150, 150));
    janela.draw(indicador);

    // Rodape / Barra de status e legenda de indicadores ([RF009], [RF010], [REU003])
    if (WareSimUI::fonteCarregada(fonte))
    {
        int qtdVencidos = 0;
        int qtdProximos = 0;
        int qtdReservados = 0;
        for (const auto& it : listaItensCache)
        {
            if (it.statusValidade == 2) ++qtdVencidos;
            else if (it.statusValidade == 1) ++qtdProximos;
            if (it.flagReserva) ++qtdReservados;
        }

        std::string textoRodape;
        if (!modoHistorico)
        {
            textoRodape = "Estoque: " + std::to_string(listaItensCache.size()) +
                          " | Reservados: " + std::to_string(qtdReservados) +
                          " | <=1 mes: " + std::to_string(qtdProximos) +
                          " | Vencidos: " + std::to_string(qtdVencidos) +
                          " | [Roda: Rolar | F11: Tela Cheia | ESC: Sair]";
        }
        else
        {
            textoRodape = "Historico de Remocoes - Registros: " +
                          std::to_string(listaHistoricoCache.size()) +
                          " | [F11: Tela Cheia | ESC: Sair]";
        }

        sf::Text rodape = WareSimUI::criarTexto(fonte, textoRodape, 13);
        rodape.setFillColor(sf::Color(60, 40, 10));
        rodape.setPosition({ margem + 4.f, static_cast<float>(altura) - 30.f });
        janela.draw(rodape);

        if (!mensagemSucessoOperacao.empty())
        {
            float largBanner = std::min(480.f, static_cast<float>(largura) * 0.42f);
            sf::RectangleShape barraSucesso({ largBanner, 26.f });
            barraSucesso.setPosition({ static_cast<float>(largura) - margem - largBanner, static_cast<float>(altura) - 34.f });
            barraSucesso.setFillColor(sf::Color(210, 245, 210));
            barraSucesso.setOutlineColor(sf::Color(40, 150, 40));
            barraSucesso.setOutlineThickness(1.5f);
            janela.draw(barraSucesso);

            sf::String sucUtf8 = sf::String::fromUtf8(mensagemSucessoOperacao.begin(), mensagemSucessoOperacao.end());
            sf::String sucQuebrado = WareSimUI::quebrarTexto(fonte, sucUtf8, 12u, largBanner - 14.f);
            sf::Text txtSuc = WareSimUI::criarTexto(fonte, sucQuebrado, 12);
            txtSuc.setFillColor(sf::Color(0, 105, 0));
            txtSuc.setStyle(sf::Text::Bold);
            txtSuc.setPosition({ static_cast<float>(largura) - margem - largBanner + 8.f, static_cast<float>(altura) - 29.f });
            janela.draw(txtSuc);
        }
    }

    janela.display();
}

void InterfaceGrafica::selecionar()
{
    sf::Vector2f posMouse = WareSimUI::obterPosicaoMouse(janela);

    const float margem = 16.f;
    const float topo = 12.f;
    const float alturaBotao = 56.f;
    const int numBotoes = 6;
    const float larguraTotalBotoes = std::max(600.f, static_cast<float>(largura) - 2.f * margem);
    const float larguraCadaBotao = larguraTotalBotoes / static_cast<float>(numBotoes);

    float x = margem;

    for (int i = 0; i < numBotoes; ++i)
    {
        bool mouseSobre =
            posMouse.x >= x &&
            posMouse.x <= x + larguraCadaBotao &&
            posMouse.y >= topo &&
            posMouse.y <= topo + alturaBotao;

        if (mouseSobre)
        {
            // [REU002] Os botoes devem emitir um som ao serem selecionados.
            WareSimUI::tocarSomBotao();

            if (i == 0)
            {
                mensagemSucessoOperacao.clear();
                verEstoque();
            }
            else if (i == 1)
            {
                acessarOpcaoInsercao();
            }
            else if (i == 2)
            {
                acessarOpcaoRemocao();
            }
            else if (i == 3)
            {
                acessarOpcaoReserva();
            }
            else if (i == 4)
            {
                mensagemSucessoOperacao.clear();
                verRemocoes();
            }
            else if (i == 5)
            {
                alternarTelaCheia();
            }

            break;
        }

        x += larguraCadaBotao;
    }
}

void InterfaceGrafica::renderizarLista()
{
    std::string dados[9];
    int total = static_cast<int>(listaItensCache.size());

    const float tabelaY = 82.f;
    const float alturaCabecalho = 36.f;
    const float tabelaAltura = std::max(260.f, static_cast<float>(altura) - tabelaY - 46.f);
    const float limiteInferiorY = tabelaY + tabelaAltura;

    float linhaY = tabelaY + alturaCabecalho;

    for (int idx = scrollOffset; idx < total; ++idx)
    {
        const auto& item = listaItensCache[idx];
        dados[0] = item.lote;
        dados[1] = item.id;
        dados[2] = item.nome;
        dados[3] = item.volume;
        dados[4] = item.categoria;
        dados[5] = item.distribuidor;
        dados[6] = item.preco;
        dados[7] = item.data_validade;
        dados[8] = item.reservado;

        float alturaDesenhada = desenhar_linha_responsiva(dados, linhaY, limiteInferiorY, item);
        if (alturaDesenhada <= 0.f)
            break;
        linhaY += alturaDesenhada;
    }
}

void InterfaceGrafica::desenhar_linha(std::string* dados, int indiceVisivel, const ItemEstoque& item)
{
    const float tabelaY = 82.f;
    const float alturaCabecalho = 36.f;
    const float tabelaAltura = std::max(260.f, static_cast<float>(altura) - tabelaY - 46.f);
    float linhaY = tabelaY + alturaCabecalho + 55.f * indiceVisivel;
    desenhar_linha_responsiva(dados, linhaY, tabelaY + tabelaAltura, item);
}

// Renderiza uma linha da tabela quebrando o texto automaticamente quando exceder a largura da coluna
float InterfaceGrafica::desenhar_linha_responsiva(std::string* dados, float linhaY, float limiteInferiorY, const ItemEstoque& item)
{
    const float tabelaX = 16.f;
    const float tabelaLargura = std::max(600.f, static_cast<float>(largura) - 32.f);
    const float larguraUtil = tabelaLargura - 12.f;

    unsigned int tamFonteCelula = (largura < 1050) ? 13u : 14u;
    sf::String textosQuebrados[9];
    int maxLinhasNaLinha = 1;

    for (int col = 0; col < 9; ++col)
    {
        sf::String strUtf8 = sf::String::fromUtf8(dados[col].begin(), dados[col].end());
        bool emNegrito = (col == 7 && item.statusValidade > 0) || (col == 8 && item.flagReserva);
        int numLinhasCol = 1;
        textosQuebrados[col] = WareSimUI::quebrarTexto(
            fonte, strUtf8, tamFonteCelula, largurasColunas[col] - 12.f, &numLinhasCol, emNegrito
        );
        if (numLinhasCol > maxLinhasNaLinha)
            maxLinhasNaLinha = numLinhasCol;
    }

    float alturaLinha = std::max(52.f, maxLinhasNaLinha * (static_cast<float>(tamFonteCelula) + 5.f) + 16.f);
    if (linhaY + alturaLinha > limiteInferiorY + 1.f)
        return 0.f;

    float colunaX = tabelaX;

    // [REU003] Cada linha do mesmo lote deve conter a mesma cor.
    sf::RectangleShape fundoLinha({ larguraUtil, alturaLinha });
    fundoLinha.setPosition({ tabelaX, linhaY });
    fundoLinha.setFillColor(obterCorDoLote(item.lote));
    janela.draw(fundoLinha);

    for (int col = 0; col < 9; ++col)
    {
        // Destaque visual para coluna de Validade ([RF009], [RNF002]) e Reserva ([RF010], [RNF003])
        if (col == 7 && item.statusValidade > 0)
        {
            sf::RectangleShape destaqueVal({ largurasColunas[col] - 2.f, alturaLinha - 2.f });
            destaqueVal.setPosition({ colunaX + 2.f, linhaY + 1.f });
            destaqueVal.setFillColor(
                item.statusValidade == 2
                ? sf::Color(255, 185, 185)  // Vermelho claro para VENCIDO
                : sf::Color(255, 225, 145)  // Amarelo/Laranja claro para <= 1 mes
            );
            janela.draw(destaqueVal);
        }
        else if (col == 8 && item.flagReserva)
        {
            sf::RectangleShape destaqueRes({ largurasColunas[col] - 4.f, alturaLinha - 2.f });
            destaqueRes.setPosition({ colunaX + 2.f, linhaY + 1.f });
            destaqueRes.setFillColor(sf::Color(195, 240, 205)); // Verde suave para item reservado
            janela.draw(destaqueRes);
        }

        sf::RectangleShape divisoria({ 2.f, alturaLinha });
        divisoria.setPosition({ colunaX, linhaY });
        divisoria.setFillColor(sf::Color(222, 154, 64));
        janela.draw(divisoria);

        if (WareSimUI::fonteCarregada(fonte))
        {
            sf::Text textoCelula = WareSimUI::criarTexto(fonte, textosQuebrados[col], tamFonteCelula);

            if (col == 7 && item.statusValidade == 2)
            {
                textoCelula.setFillColor(sf::Color(165, 0, 0));
                textoCelula.setStyle(sf::Text::Bold);
            }
            else if (col == 7 && item.statusValidade == 1)
            {
                textoCelula.setFillColor(sf::Color(160, 75, 0));
                textoCelula.setStyle(sf::Text::Bold);
            }
            else if (col == 8 && item.flagReserva)
            {
                textoCelula.setFillColor(sf::Color(10, 100, 30));
                textoCelula.setStyle(sf::Text::Bold);
            }
            else
            {
                textoCelula.setFillColor(sf::Color::Black);
            }

            textoCelula.setPosition({ colunaX + 6.f, linhaY + 8.f });
            janela.draw(textoCelula);
        }

        colunaX += largurasColunas[col];
    }

    sf::RectangleShape linhaInferior({ tabelaLargura, 2.f });
    linhaInferior.setPosition({ tabelaX, linhaY + alturaLinha });
    linhaInferior.setFillColor(sf::Color(222, 154, 64));
    janela.draw(linhaInferior);

    return alturaLinha;
}

void InterfaceGrafica::renderizarHistorico()
{
    const float tabelaX = 16.f;
    const float tabelaY = 82.f;
    const float tabelaLargura = std::max(600.f, static_cast<float>(largura) - 32.f);
    const float larguraUtil = tabelaLargura - 12.f;
    const float tabelaAltura = std::max(260.f, static_cast<float>(altura) - tabelaY - 46.f);
    const float alturaCabecalho = 36.f;
    const float limiteInferiorY = tabelaY + tabelaAltura;

    const std::string colunasHist[] = {
        "Id Registro",
        "Id do Lote",
        "Bebida Removida",
        "Qtd Removida",
        "Data da Remocao",
        "Motivo da Remocao"
    };

    static const float propHist[6] = {
        0.095f, 0.095f, 0.195f, 0.115f, 0.135f, 0.365f
    };
    float largurasHist[6];
    for (int i = 0; i < 6; ++i)
        largurasHist[i] = std::floor(larguraUtil * propHist[i]);

    unsigned int tamFonteCab = (largura < 1050) ? 13u : 15u;
    unsigned int tamFonteCelula = (largura < 1050) ? 13u : 14u;

    float colX = tabelaX;
    for (int i = 0; i < 6; ++i)
    {
        sf::RectangleShape divisoria({ 2.f, alturaCabecalho });
        divisoria.setPosition({ colX, tabelaY });
        divisoria.setFillColor(sf::Color(222, 154, 64));
        janela.draw(divisoria);

        if (WareSimUI::fonteCarregada(fonte))
        {
            sf::String cabQuebrado = WareSimUI::quebrarTexto(
                fonte, colunasHist[i], tamFonteCab, largurasHist[i] - 12.f, nullptr, true
            );
            sf::Text textoCol = WareSimUI::criarTexto(fonte, cabQuebrado, tamFonteCab);
            textoCol.setFillColor(sf::Color::Black);
            textoCol.setStyle(sf::Text::Bold);
            textoCol.setPosition({ colX + 6.f, tabelaY + 4.f });
            janela.draw(textoCol);
        }
        colX += largurasHist[i];
    }

    sf::RectangleShape linhaCab({ tabelaLargura, 2.f });
    linhaCab.setPosition({ tabelaX, tabelaY + alturaCabecalho });
    linhaCab.setFillColor(sf::Color(222, 154, 64));
    janela.draw(linhaCab);

    int total = static_cast<int>(listaHistoricoCache.size());
    float linhaY = tabelaY + alturaCabecalho;

    for (int idx = scrollOffset; idx < total; ++idx)
    {
        const auto& reg = listaHistoricoCache[idx];

        std::string camposLinha[6] = {
            std::to_string(reg.getIdRegistro()),
            reg.getCodigoLote().empty() ? "-" : reg.getCodigoLote(),
            reg.getNomeBebida().empty() ? "-" : reg.getNomeBebida(),
            std::to_string(reg.getQuantidade()),
            reg.getDataRemocao(),
            reg.getMotivoRemocao()
        };

        sf::String textosQuebrados[6];
        int maxLinhas = 1;
        for (int c = 0; c < 6; ++c)
        {
            sf::String strUtf8 = sf::String::fromUtf8(camposLinha[c].begin(), camposLinha[c].end());
            int nLinhas = 1;
            textosQuebrados[c] = WareSimUI::quebrarTexto(
                fonte, strUtf8, tamFonteCelula, largurasHist[c] - 14.f, &nLinhas
            );
            if (nLinhas > maxLinhas)
                maxLinhas = nLinhas;
        }

        float alturaLinha = std::max(52.f, maxLinhas * (static_cast<float>(tamFonteCelula) + 5.f) + 16.f);
        if (linhaY + alturaLinha > limiteInferiorY + 1.f)
            break;

        sf::RectangleShape fundoLinha({ larguraUtil, alturaLinha });
        fundoLinha.setPosition({ tabelaX, linhaY });
        fundoLinha.setFillColor(obterCorDoLote(reg.getCodigoLote()));
        janela.draw(fundoLinha);

        float cx = tabelaX;
        for (int c = 0; c < 6; ++c)
        {
            sf::RectangleShape div({ 2.f, alturaLinha });
            div.setPosition({ cx, linhaY });
            div.setFillColor(sf::Color(222, 154, 64));
            janela.draw(div);

            if (WareSimUI::fonteCarregada(fonte))
            {
                sf::Text txt = WareSimUI::criarTexto(fonte, textosQuebrados[c], tamFonteCelula);
                txt.setFillColor(sf::Color::Black);
                txt.setPosition({ cx + 6.f, linhaY + 8.f });
                janela.draw(txt);
            }
            cx += largurasHist[c];
        }

        sf::RectangleShape linhaInf({ tabelaLargura, 2.f });
        linhaInf.setPosition({ tabelaX, linhaY + alturaLinha });
        linhaInf.setFillColor(sf::Color(222, 154, 64));
        janela.draw(linhaInf);

        linhaY += alturaLinha;
    }
}
