
#include "JanelaReservar.h"

JanelaReservar::JanelaReservar()
    : janela(sf::VideoMode({ 700u, 570u }), "Reservar")
{
    if (!fonte.openFromFile("arial.ttf"))
        janela.close();
}

void JanelaReservar::executar()
{
    while (janela.isOpen())
    {
        while (const auto evento = janela.pollEvent())
        {
            if (evento->is<sf::Event::Closed>())
            {
                janela.close();
            }
            else if (const auto* clique =
                evento->getIf<sf::Event::MouseButtonPressed>())
            {
                if (clique->button == sf::Mouse::Button::Left)
                {
                    processarClique(
                        clique->position.x,
                        clique->position.y
                    );
                }
            }
            else if (const auto* entrada =
                evento->getIf<sf::Event::TextEntered>())
            {
                processarTeclado(entrada->unicode);
            }
            else if (const auto* tecla =
                evento->getIf<sf::Event::KeyPressed>())
            {
                if (tecla->code == sf::Keyboard::Key::Backspace)
                {
                    processarBackspace();
                }
            }
        }

        if (janela.isOpen())
            renderizar();
    }
}

void JanelaReservar::processarClique(int x, int y)
{
    // Seleção entre Lote e Item.
    if (y >= 25 && y <= 70)
    {
        if (x >= 30 && x <= 180)
        {
            modoLote = true;
        }
        else if (x >= 200 && x <= 350)
        {
            modoLote = false;
        }

        // Remove o foco dos campos ao trocar de modo.
        idLoteSelecionado = false;
        idItemSelecionado = false;
        quantidadeSelecionada = false;
        dataSelecionada = false;
        localSelecionado = false;

        return;
    }

    // ID Lote.
    idLoteSelecionado =
        modoLote &&
        x >= 30 && x <= 230 &&
        y >= 120 && y <= 160;

    // Quantidade de itens do lote.
    quantidadeSelecionada =
        modoLote &&
        x >= 250 && x <= 450 &&
        y >= 120 && y <= 160;

    // ID Item.
    idItemSelecionado =
        !modoLote &&
        x >= 30 && x <= 330 &&
        y >= 120 && y <= 160;

    // Data da reserva.
    dataSelecionada =
        x >= 30 && x <= 330 &&
        y >= 210 && y <= 250;

    // Local da reserva.
    localSelecionado =
        x >= 30 && x <= 670 &&
        y >= 300 && y <= 350;

    // Cancelar: fecha a janela.
    if (x >= 370 && x <= 520 &&
        y >= 480 && y <= 530)
    {
        janela.close();
        return;
    }

    // Reservar: fecha a janela.
    if (x >= 530 && x <= 680 &&
        y >= 480 && y <= 530)
    {
        janela.close();
        return;
    }
}

void JanelaReservar::processarTeclado(char32_t caractere)
{
    // ID Lote: somente números.
    if (idLoteSelecionado && modoLote)
    {
        if (caractere >= U'0' && caractere <= U'9')
            idLote += sf::String(caractere);
    }
    // Quantidade: somente números.
    else if (quantidadeSelecionada && modoLote)
    {
        if (caractere >= U'0' && caractere <= U'9')
            quantidade += sf::String(caractere);
    }
    // ID Item: somente números.
    else if (idItemSelecionado && !modoLote)
    {
        if (caractere >= U'0' && caractere <= U'9')
            idItem += sf::String(caractere);
    }
    // Data: números e barras.
    else if (dataSelecionada)
    {
        if ((caractere >= U'0' && caractere <= U'9') ||
            caractere == U'/')
        {
            dataReserva += sf::String(caractere);
        }
    }
    // Local: texto livre em uma linha.
    else if (localSelecionado)
    {
        if (caractere >= 32 &&
            caractere != 127 &&
            caractere != U'\n' &&
            caractere != U'\r')
        {
            localReserva += sf::String(caractere);
        }
    }
}

void JanelaReservar::processarBackspace()
{
    if (idLoteSelecionado &&
        modoLote &&
        !idLote.isEmpty())
    {
        idLote.erase(idLote.getSize() - 1, 1);
    }
    else if (quantidadeSelecionada &&
        modoLote &&
        !quantidade.isEmpty())
    {
        quantidade.erase(quantidade.getSize() - 1, 1);
    }
    else if (idItemSelecionado &&
        !modoLote &&
        !idItem.isEmpty())
    {
        idItem.erase(idItem.getSize() - 1, 1);
    }
    else if (dataSelecionada &&
        !dataReserva.isEmpty())
    {
        dataReserva.erase(dataReserva.getSize() - 1, 1);
    }
    else if (localSelecionado &&
        !localReserva.isEmpty())
    {
        localReserva.erase(localReserva.getSize() - 1, 1);
    }
}

void JanelaReservar::renderizar()
{
    janela.clear(sf::Color(255, 227, 163));

    // Opção Lote.
    sf::RectangleShape botaoLote({ 150.f, 45.f });
    botaoLote.setPosition({ 30.f, 25.f });
    botaoLote.setFillColor(
        modoLote ? sf::Color(235, 202, 157)
        : sf::Color(255, 222, 179)
    );
    botaoLote.setOutlineColor(sf::Color(222, 154, 64));
    botaoLote.setOutlineThickness(2.f);
    janela.draw(botaoLote);

    sf::Text textoLote(fonte, "Lote", 18);
    textoLote.setFillColor(sf::Color::Black);
    textoLote.setPosition({ 78.f, 36.f });
    janela.draw(textoLote);

    // Opção Item.
    sf::RectangleShape botaoItem({ 150.f, 45.f });
    botaoItem.setPosition({ 200.f, 25.f });
    botaoItem.setFillColor(
        !modoLote ? sf::Color(235, 202, 157)
        : sf::Color(255, 222, 179)
    );
    botaoItem.setOutlineColor(sf::Color(222, 154, 64));
    botaoItem.setOutlineThickness(2.f);
    janela.draw(botaoItem);

    sf::Text textoItem(fonte, "Item", 18);
    textoItem.setFillColor(sf::Color::Black);
    textoItem.setPosition({ 250.f, 36.f });
    janela.draw(textoItem);

    if (modoLote)
    {
        // Rótulo ID Lote.
        sf::Text rotuloIDLote(fonte, "ID Lote", 17);
        rotuloIDLote.setFillColor(sf::Color::Black);
        rotuloIDLote.setPosition({ 30.f, 88.f });
        janela.draw(rotuloIDLote);

        // Campo ID Lote.
        sf::RectangleShape campoIDLote({ 200.f, 40.f });
        campoIDLote.setPosition({ 30.f, 120.f });
        campoIDLote.setFillColor(sf::Color(255, 245, 225));
        campoIDLote.setOutlineColor(sf::Color(222, 154, 64));
        campoIDLote.setOutlineThickness(2.f);
        janela.draw(campoIDLote);

        sf::Text textoIDLote(fonte, idLote, 17);
        textoIDLote.setFillColor(sf::Color::Black);
        textoIDLote.setPosition({ 40.f, 128.f });
        janela.draw(textoIDLote);

        // Rótulo da quantidade.
        sf::Text rotuloQuantidade(fonte, "Qtd Itens", 17);
        rotuloQuantidade.setFillColor(sf::Color::Black);
        rotuloQuantidade.setPosition({ 250.f, 88.f });
        janela.draw(rotuloQuantidade);

        // Campo da quantidade.
        sf::RectangleShape campoQuantidade({ 200.f, 40.f });
        campoQuantidade.setPosition({ 250.f, 120.f });
        campoQuantidade.setFillColor(sf::Color(255, 245, 225));
        campoQuantidade.setOutlineColor(sf::Color(222, 154, 64));
        campoQuantidade.setOutlineThickness(2.f);
        janela.draw(campoQuantidade);

        sf::Text textoQuantidade(fonte, quantidade, 17);
        textoQuantidade.setFillColor(sf::Color::Black);
        textoQuantidade.setPosition({ 260.f, 128.f });
        janela.draw(textoQuantidade);
    }
    else
    {
        // Rótulo ID Item.
        sf::Text rotuloIDItem(fonte, "ID Item", 17);
        rotuloIDItem.setFillColor(sf::Color::Black);
        rotuloIDItem.setPosition({ 30.f, 88.f });
        janela.draw(rotuloIDItem);

        // Campo ID Item.
        sf::RectangleShape campoIDItem({ 300.f, 40.f });
        campoIDItem.setPosition({ 30.f, 120.f });
        campoIDItem.setFillColor(sf::Color(255, 245, 225));
        campoIDItem.setOutlineColor(sf::Color(222, 154, 64));
        campoIDItem.setOutlineThickness(2.f);
        janela.draw(campoIDItem);

        sf::Text textoIDItem(fonte, idItem, 17);
        textoIDItem.setFillColor(sf::Color::Black);
        textoIDItem.setPosition({ 40.f, 128.f });
        janela.draw(textoIDItem);
    }

    // Rótulo da data.
    sf::Text rotuloData(fonte, "Data da reserva", 17);
    rotuloData.setFillColor(sf::Color::Black);
    rotuloData.setPosition({ 30.f, 178.f });
    janela.draw(rotuloData);

    // Campo da data.
    sf::RectangleShape campoData({ 300.f, 40.f });
    campoData.setPosition({ 30.f, 210.f });
    campoData.setFillColor(sf::Color(255, 245, 225));
    campoData.setOutlineColor(sf::Color(222, 154, 64));
    campoData.setOutlineThickness(2.f);
    janela.draw(campoData);

    sf::Text textoData(fonte, dataReserva, 17);
    textoData.setFillColor(sf::Color::Black);
    textoData.setPosition({ 40.f, 218.f });
    janela.draw(textoData);

    // Rótulo do local.
    sf::Text rotuloLocal(fonte, "Local da reserva", 17);
    rotuloLocal.setFillColor(sf::Color::Black);
    rotuloLocal.setPosition({ 30.f, 268.f });
    janela.draw(rotuloLocal);

    // Campo do local.
    sf::RectangleShape campoLocal({ 640.f, 50.f });
    campoLocal.setPosition({ 30.f, 300.f });
    campoLocal.setFillColor(sf::Color(255, 245, 225));
    campoLocal.setOutlineColor(sf::Color(222, 154, 64));
    campoLocal.setOutlineThickness(2.f);
    janela.draw(campoLocal);

    sf::Text textoLocal(fonte, localReserva, 17);
    textoLocal.setFillColor(sf::Color::Black);
    textoLocal.setPosition({ 40.f, 312.f });
    janela.draw(textoLocal);

    // Botão Cancelar.
    sf::RectangleShape botaoCancelar({ 150.f, 50.f });
    botaoCancelar.setPosition({ 370.f, 480.f });
    botaoCancelar.setFillColor(sf::Color(255, 222, 179));
    botaoCancelar.setOutlineColor(sf::Color(222, 154, 64));
    botaoCancelar.setOutlineThickness(2.f);
    janela.draw(botaoCancelar);

    sf::Text textoCancelar(fonte, "Cancelar", 18);
    textoCancelar.setFillColor(sf::Color::Black);
    textoCancelar.setPosition({ 407.f, 493.f });
    janela.draw(textoCancelar);

    // Botão Reservar.
    sf::RectangleShape botaoReservar({ 150.f, 50.f });
    botaoReservar.setPosition({ 530.f, 480.f });
    botaoReservar.setFillColor(sf::Color(255, 222, 179));
    botaoReservar.setOutlineColor(sf::Color(222, 154, 64));
    botaoReservar.setOutlineThickness(2.f);
    janela.draw(botaoReservar);

    sf::Text textoReservar(fonte, "Reservar", 18);
    textoReservar.setFillColor(sf::Color::Black);
    textoReservar.setPosition({ 565.f, 493.f });
    janela.draw(textoReservar);

    janela.display();
}
