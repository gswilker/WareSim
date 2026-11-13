#include "JanelaReservar.h"
#include "SFMLCompat.h"

JanelaReservar::JanelaReservar(Estoque* estoqueRef)
    : janela(WareSimUI::criarVideoMode(700u, 570u), "Reservar Item / Lote"),
      estoque(estoqueRef ? estoqueRef : &estoqueLocal)
{
    if (!WareSimUI::carregarFonte(fonte, "arial.ttf"))
        janela.close();
}

void JanelaReservar::executar()
{
    bool telaCheia = false;
    while (janela.isOpen())
    {
        WareSimUI::EventoUI ev;
        while (WareSimUI::pollEvento(janela, ev))
        {
            if (ev.tipo == WareSimUI::TipoEvento::Fechar)
            {
                WareSimUI::tocarSomBotao();
                janela.close();
            }
            else if (ev.tipo == WareSimUI::TipoEvento::Redimensionar)
            {
                WareSimUI::aplicarViewProporcional(janela, 700.f, 570.f, ev.novaLargura, ev.novaAltura);
            }
            else if (ev.tipo == WareSimUI::TipoEvento::CliqueMouse && ev.botaoEsquerdo)
            {
                processarClique(ev.mouseX, ev.mouseY);
            }
            else if (ev.tipo == WareSimUI::TipoEvento::TextoDigitado)
            {
                processarTeclado(ev.unicode);
            }
            else if (ev.tipo == WareSimUI::TipoEvento::TeclaPressionada)
            {
                if (ev.teclaBackspace)
                    processarBackspace();
                else if (ev.teclaF11) {
                    WareSimUI::tocarSomBotao();
                    telaCheia = !telaCheia;
                    if (telaCheia) {
                        sf::VideoMode dm = sf::VideoMode::getDesktopMode();
                        janela.create(dm, "Reservar Item / Lote", sf::Style::Fullscreen);
                        WareSimUI::aplicarViewProporcional(janela, 700.f, 570.f, dm.width, dm.height);
                    } else {
                        janela.create(WareSimUI::criarVideoMode(700u, 570u), "Reservar Item / Lote", sf::Style::Default);
                        WareSimUI::aplicarViewProporcional(janela, 700.f, 570.f, 700u, 570u);
                    }
                }
                else if (ev.teclaEsc) {
                    WareSimUI::tocarSomBotao();
                    sf::sleep(sf::milliseconds(90));
                    janela.close();
                }
            }
        }

        if (janela.isOpen())
            renderizar();
    }
}

void JanelaReservar::processarClique(int x, int y)
{
    // Selecao entre Lote e Item.
    if (y >= 25 && y <= 70)
    {
        if (x >= 30 && x <= 180)
        {
            WareSimUI::tocarSomBotao();
            modoLote = true;
            mensagemStatus.clear();
        }
        else if (x >= 200 && x <= 350)
        {
            WareSimUI::tocarSomBotao();
            modoLote = false;
            mensagemStatus.clear();
        }

        // Remove o foco dos campos ao trocar de modo.
        idLoteSelecionado = false;
        idItemSelecionado = false;
        quantidadeSelecionada = false;
        dataSelecionada = false;
        localSelecionado = false;

        return;
    }

    bool antLote = idLoteSelecionado;
    bool antQtd = quantidadeSelecionada;
    bool antItem = idItemSelecionado;
    bool antData = dataSelecionada;
    bool antLocal = localSelecionado;

    // ID Lote.
    idLoteSelecionado =
        modoLote &&
        x >= 30 && x <= 230 &&
        y >= 120 && y <= 162;

    // Quantidade de itens do lote.
    quantidadeSelecionada =
        modoLote &&
        x >= 250 && x <= 450 &&
        y >= 120 && y <= 162;

    // ID Item.
    idItemSelecionado =
        !modoLote &&
        x >= 30 && x <= 330 &&
        y >= 120 && y <= 162;

    // Data da reserva.
    dataSelecionada =
        x >= 30 && x <= 330 &&
        y >= 210 && y <= 252;

    // Local da reserva.
    localSelecionado =
        x >= 30 && x <= 670 &&
        y >= 300 && y <= 375;

    if ((idLoteSelecionado && !antLote) ||
        (quantidadeSelecionada && !antQtd) ||
        (idItemSelecionado && !antItem) ||
        (dataSelecionada && !antData) ||
        (localSelecionado && !antLocal))
    {
        WareSimUI::tocarSomBotao();
    }

    // Cancelar ([UC004] Fluxo Alternativo: O sistema nao faz nada)
    if (x >= 370 && x <= 520 &&
        y >= 480 && y <= 530)
    {
        WareSimUI::tocarSomBotao();
        sf::sleep(sf::milliseconds(90));
        janela.close();
        return;
    }

    // Reservar ([UC004] Fluxo Basico e Fluxo de Excecao)
    if (x >= 530 && x <= 680 &&
        y >= 480 && y <= 530)
    {
        auto toUtf8Str = [](const sf::String& s) -> std::string {
            auto u8 = s.toUtf8();
            return std::string(reinterpret_cast<const char*>(u8.data()), u8.size());
        };

        DadosReserva dados;
        dados.modoLote = modoLote;
        dados.idLote = toUtf8Str(idLote);
        dados.idItem = toUtf8Str(idItem);
        dados.quantidade = toUtf8Str(quantidade);
        dados.dataReserva = toUtf8Str(dataReserva);
        dados.localReserva = toUtf8Str(localReserva);

        std::string erro;
        if (estoque && estoque->reservarItem(dados, erro))
        {
            WareSimUI::tocarSomSucesso();
            statusErro = false;
            operacaoConcluida = true;
            mensagemStatus = "Mensagem de sucesso: Reserva registrada no estoque com sucesso!";
            sf::sleep(sf::milliseconds(110));
            janela.close();
        }
        else
        {
            // [UC004] Fluxo de excecao: sinaliza o erro ao tentar reservar
            WareSimUI::tocarSomErro();
            statusErro = true;
            mensagemStatus = erro.empty() ? "Erro ao realizar reserva!" : erro;
        }
        return;
    }
}

void JanelaReservar::processarTeclado(char32_t caractere)
{
    if (caractere < 32 || caractere == 127)
        return;

    // ID Lote: aceita letras e numeros (ex: Lote A, B, 204).
    if (idLoteSelecionado && modoLote)
    {
        idLote += sf::String(caractere);
        mensagemStatus.clear();
    }
    // Quantidade: somente numeros.
    else if (quantidadeSelecionada && modoLote)
    {
        if (caractere >= U'0' && caractere <= U'9')
        {
            quantidade += sf::String(caractere);
            mensagemStatus.clear();
        }
    }
    // ID Item: somente numeros.
    else if (idItemSelecionado && !modoLote)
    {
        if (caractere >= U'0' && caractere <= U'9')
        {
            idItem += sf::String(caractere);
            mensagemStatus.clear();
        }
    }
    // Data: numeros, barras e hifens (DD/MM/AAAA ou AAAA-MM-DD).
    else if (dataSelecionada)
    {
        if ((caractere >= U'0' && caractere <= U'9') ||
            caractere == U'/' || caractere == U'-')
        {
            dataReserva += sf::String(caractere);
            mensagemStatus.clear();
        }
    }
    // Local / Destino da reserva: texto livre ([RNF003]).
    else if (localSelecionado)
    {
        if (caractere != U'\n' && caractere != U'\r')
        {
            localReserva += sf::String(caractere);
            mensagemStatus.clear();
        }
    }
}

void JanelaReservar::processarBackspace()
{
    if (idLoteSelecionado && modoLote && !idLote.isEmpty())
    {
        idLote.erase(idLote.getSize() - 1, 1);
    }
    else if (quantidadeSelecionada && modoLote && !quantidade.isEmpty())
    {
        quantidade.erase(quantidade.getSize() - 1, 1);
    }
    else if (idItemSelecionado && !modoLote && !idItem.isEmpty())
    {
        idItem.erase(idItem.getSize() - 1, 1);
    }
    else if (dataSelecionada && !dataReserva.isEmpty())
    {
        dataReserva.erase(dataReserva.getSize() - 1, 1);
    }
    else if (localSelecionado && !localReserva.isEmpty())
    {
        localReserva.erase(localReserva.getSize() - 1, 1);
    }
}

void JanelaReservar::renderizar()
{
    janela.clear(sf::Color(255, 227, 163));

    // Opcao Lote.
    sf::RectangleShape botaoLote({ 150.f, 45.f });
    botaoLote.setPosition({ 30.f, 25.f });
    botaoLote.setFillColor(
        modoLote ? sf::Color(235, 202, 157)
        : sf::Color(255, 222, 179)
    );
    botaoLote.setOutlineColor(sf::Color(222, 154, 64));
    botaoLote.setOutlineThickness(2.f);
    janela.draw(botaoLote);

    sf::Text textoLote = WareSimUI::criarTexto(fonte, "Lote", 18);
    textoLote.setFillColor(sf::Color::Black);
    textoLote.setPosition({ 78.f, 36.f });
    janela.draw(textoLote);

    // Opcao Item.
    sf::RectangleShape botaoItem({ 150.f, 45.f });
    botaoItem.setPosition({ 200.f, 25.f });
    botaoItem.setFillColor(
        !modoLote ? sf::Color(235, 202, 157)
        : sf::Color(255, 222, 179)
    );
    botaoItem.setOutlineColor(sf::Color(222, 154, 64));
    botaoItem.setOutlineThickness(2.f);
    janela.draw(botaoItem);

    sf::Text textoItem = WareSimUI::criarTexto(fonte, "Item", 18);
    textoItem.setFillColor(sf::Color::Black);
    textoItem.setPosition({ 250.f, 36.f });
    janela.draw(textoItem);

    if (modoLote)
    {
        // Rotulo ID Lote.
        sf::Text rotuloIDLote = WareSimUI::criarTexto(fonte, "ID Lote", 17);
        rotuloIDLote.setFillColor(sf::Color::Black);
        rotuloIDLote.setPosition({ 30.f, 88.f });
        janela.draw(rotuloIDLote);

        // Campo ID Lote.
        sf::RectangleShape campoIDLote({ 200.f, 42.f });
        campoIDLote.setPosition({ 30.f, 120.f });
        campoIDLote.setFillColor(sf::Color(255, 245, 225));
        campoIDLote.setOutlineColor(idLoteSelecionado ? sf::Color(80, 120, 200) : sf::Color(222, 154, 64));
        campoIDLote.setOutlineThickness(idLoteSelecionado ? 2.5f : 2.f);
        janela.draw(campoIDLote);

        unsigned int tamFonteLote = 16u;
        sf::String strLoteAjust = WareSimUI::ajustarTextoCaixa(fonte, idLote, 16u, 200.f, 42.f, tamFonteLote);
        sf::Text textoIDLote = WareSimUI::criarTexto(fonte, strLoteAjust, tamFonteLote);
        textoIDLote.setFillColor(sf::Color::Black);
        textoIDLote.setPosition({ 38.f, 125.f });
        janela.draw(textoIDLote);

        // Rotulo da quantidade.
        sf::Text rotuloQuantidade = WareSimUI::criarTexto(fonte, "Qtd Itens", 17);
        rotuloQuantidade.setFillColor(sf::Color::Black);
        rotuloQuantidade.setPosition({ 250.f, 88.f });
        janela.draw(rotuloQuantidade);

        // Campo da quantidade.
        sf::RectangleShape campoQuantidade({ 200.f, 42.f });
        campoQuantidade.setPosition({ 250.f, 120.f });
        campoQuantidade.setFillColor(sf::Color(255, 245, 225));
        campoQuantidade.setOutlineColor(quantidadeSelecionada ? sf::Color(80, 120, 200) : sf::Color(222, 154, 64));
        campoQuantidade.setOutlineThickness(quantidadeSelecionada ? 2.5f : 2.f);
        janela.draw(campoQuantidade);

        unsigned int tamFonteQtd = 16u;
        sf::String strQtdAjust = WareSimUI::ajustarTextoCaixa(fonte, quantidade, 16u, 200.f, 42.f, tamFonteQtd);
        sf::Text textoQuantidade = WareSimUI::criarTexto(fonte, strQtdAjust, tamFonteQtd);
        textoQuantidade.setFillColor(sf::Color::Black);
        textoQuantidade.setPosition({ 258.f, 125.f });
        janela.draw(textoQuantidade);
    }
    else
    {
        // Rotulo ID Item.
        sf::Text rotuloIDItem = WareSimUI::criarTexto(fonte, "ID Item (Id bebida)", 17);
        rotuloIDItem.setFillColor(sf::Color::Black);
        rotuloIDItem.setPosition({ 30.f, 88.f });
        janela.draw(rotuloIDItem);

        // Campo ID Item.
        sf::RectangleShape campoIDItem({ 300.f, 42.f });
        campoIDItem.setPosition({ 30.f, 120.f });
        campoIDItem.setFillColor(sf::Color(255, 245, 225));
        campoIDItem.setOutlineColor(idItemSelecionado ? sf::Color(80, 120, 200) : sf::Color(222, 154, 64));
        campoIDItem.setOutlineThickness(idItemSelecionado ? 2.5f : 2.f);
        janela.draw(campoIDItem);

        unsigned int tamFonteItem = 16u;
        sf::String strItemAjust = WareSimUI::ajustarTextoCaixa(fonte, idItem, 16u, 300.f, 42.f, tamFonteItem);
        sf::Text textoIDItem = WareSimUI::criarTexto(fonte, strItemAjust, tamFonteItem);
        textoIDItem.setFillColor(sf::Color::Black);
        textoIDItem.setPosition({ 38.f, 125.f });
        janela.draw(textoIDItem);
    }

    // Rotulo da data.
    sf::Text rotuloData = WareSimUI::criarTexto(fonte, "Data da reserva (DD/MM/AAAA ou AAAA-MM-DD)", 17);
    rotuloData.setFillColor(sf::Color::Black);
    rotuloData.setPosition({ 30.f, 178.f });
    janela.draw(rotuloData);

    // Campo da data.
    sf::RectangleShape campoData({ 300.f, 42.f });
    campoData.setPosition({ 30.f, 210.f });
    campoData.setFillColor(sf::Color(255, 245, 225));
    campoData.setOutlineColor(dataSelecionada ? sf::Color(80, 120, 200) : sf::Color(222, 154, 64));
    campoData.setOutlineThickness(dataSelecionada ? 2.5f : 2.f);
    janela.draw(campoData);

    unsigned int tamFonteData = 16u;
    sf::String strDataAjust = WareSimUI::ajustarTextoCaixa(fonte, dataReserva, 16u, 300.f, 42.f, tamFonteData);
    sf::Text textoData = WareSimUI::criarTexto(fonte, strDataAjust, tamFonteData);
    textoData.setFillColor(sf::Color::Black);
    textoData.setPosition({ 38.f, 215.f });
    janela.draw(textoData);

    // Rotulo do local / destino ([RNF003]).
    sf::Text rotuloLocal = WareSimUI::criarTexto(fonte, "Local / Destino da reserva (Cliente / Evento)", 17);
    rotuloLocal.setFillColor(sf::Color::Black);
    rotuloLocal.setPosition({ 30.f, 268.f });
    janela.draw(rotuloLocal);

    // Campo do local com quebra de linha automatica quando exceder o limite horizontal
    sf::RectangleShape campoLocal({ 640.f, 75.f });
    campoLocal.setPosition({ 30.f, 300.f });
    campoLocal.setFillColor(sf::Color(255, 245, 225));
    campoLocal.setOutlineColor(localSelecionado ? sf::Color(80, 120, 200) : sf::Color(222, 154, 64));
    campoLocal.setOutlineThickness(localSelecionado ? 2.5f : 2.f);
    janela.draw(campoLocal);

    unsigned int tamFonteLocal = 16u;
    sf::String strLocalAjust = WareSimUI::ajustarTextoCaixa(fonte, localReserva, 16u, 640.f, 75.f, tamFonteLocal);
    sf::Text textoLocal = WareSimUI::criarTexto(fonte, strLocalAjust, tamFonteLocal);
    textoLocal.setFillColor(sf::Color::Black);
    textoLocal.setPosition({ 38.f, 308.f });
    janela.draw(textoLocal);

    // Mensagem de erro ([UC004] Fluxo de Excecao) com quebra de linha automatica
    if (!mensagemStatus.empty())
    {
        sf::String msgUtf8 = sf::String::fromUtf8(mensagemStatus.begin(), mensagemStatus.end());
        int numLinhas = 1;
        sf::String msgQuebrada = WareSimUI::quebrarTexto(fonte, msgUtf8, 14u, 620.f, &numLinhas);
        float alturaBarra = std::max(46.f, numLinhas * 18.f + 18.f);

        sf::RectangleShape barraMsg({ 640.f, alturaBarra });
        barraMsg.setPosition({ 30.f, 395.f });
        barraMsg.setFillColor(statusErro ? sf::Color(255, 210, 210) : sf::Color(210, 245, 210));
        barraMsg.setOutlineColor(statusErro ? sf::Color(190, 40, 40) : sf::Color(40, 150, 40));
        barraMsg.setOutlineThickness(2.f);
        janela.draw(barraMsg);

        sf::Text txtMsg = WareSimUI::criarTexto(fonte, msgQuebrada, 14);
        txtMsg.setFillColor(statusErro ? sf::Color(160, 0, 0) : sf::Color(0, 110, 0));
        txtMsg.setPosition({ 40.f, 404.f });
        janela.draw(txtMsg);
    }

    // Botao Cancelar.
    sf::RectangleShape botaoCancelar({ 150.f, 50.f });
    botaoCancelar.setPosition({ 370.f, 480.f });
    botaoCancelar.setFillColor(sf::Color(255, 222, 179));
    botaoCancelar.setOutlineColor(sf::Color(222, 154, 64));
    botaoCancelar.setOutlineThickness(2.f);
    janela.draw(botaoCancelar);

    sf::Text textoCancelar = WareSimUI::criarTexto(fonte, "Cancelar", 18);
    textoCancelar.setFillColor(sf::Color::Black);
    textoCancelar.setPosition({ 407.f, 493.f });
    janela.draw(textoCancelar);

    // Botao Reservar.
    sf::RectangleShape botaoReservar({ 150.f, 50.f });
    botaoReservar.setPosition({ 530.f, 480.f });
    botaoReservar.setFillColor(sf::Color(255, 222, 179));
    botaoReservar.setOutlineColor(sf::Color(222, 154, 64));
    botaoReservar.setOutlineThickness(2.f);
    janela.draw(botaoReservar);

    sf::Text textoReservar = WareSimUI::criarTexto(fonte, "Reservar", 18);
    textoReservar.setFillColor(sf::Color::Black);
    textoReservar.setPosition({ 565.f, 493.f });
    janela.draw(textoReservar);

    janela.display();
}
