#include "JanelaRemover.h"

#include <cstdio>

JanelaRemover::JanelaRemover()
    : janela(sf::VideoMode({ 700u, 570u }), "Remover")
{
    if (!fonte.openFromFile("C:/Windows/Fonts/arial.ttf"))
        janela.close();
}

void JanelaRemover::executar()
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

void JanelaRemover::processarClique(int x, int y)
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

        // Limpa o foco ao trocar de modo.
        idLoteSelecionado = false;
        idItemSelecionado = false;
        quantidadeSelecionada = false;
        motivoSelecionado = false;

        return;
    }

    // Campos de identificação e quantidade.
    idLoteSelecionado =
        modoLote &&
        x >= 30 && x <= 230 &&
        y >= 120 && y <= 160;

    quantidadeSelecionada =
        modoLote &&
        x >= 250 && x <= 450 &&
        y >= 120 && y <= 160;

    idItemSelecionado =
        !modoLote &&
        x >= 30 && x <= 330 &&
        y >= 120 && y <= 160;

    // Caixa do motivo.
    motivoSelecionado =
        x >= 30 && x <= 670 &&
        y >= 220 && y <= 430;

    // Cancelar.
    if (x >= 370 && x <= 520 &&
        y >= 480 && y <= 530)
    {
        janela.close();
        return;
    }

    // Remover / Confirmar Remoção.
    if (x >= 530 && x <= 680 &&
        y >= 480 && y <= 530)
    {
        if (!confirmarRemocao)
        {
            confirmarRemocao = true;
        }
        else
        {
            std::printf(
                "Elemento removido e salvo no histórico de remoções\n"
            );

            janela.close();
        }

        return;
    }
}

void JanelaRemover::processarTeclado(char32_t caractere)
{
    // ID do lote.
    if (idLoteSelecionado && modoLote)
    {
        if (caractere >= U'0' && caractere <= U'9')
            idLote += sf::String(caractere);
    }
    // Quantidade de itens do lote.
    else if (quantidadeSelecionada && modoLote)
    {
        if (caractere >= U'0' && caractere <= U'9')
            quantidade += sf::String(caractere);
    }
    // ID do item.
    else if (idItemSelecionado && !modoLote)
    {
        if (caractere >= U'0' && caractere <= U'9')
            idItem += sf::String(caractere);
    }
    // Motivo da remoção.
    else if (motivoSelecionado)
    {
        if (caractere == U'\n' || caractere == U'\r')
        {
            motivo += U'\n';
        }
        else if (caractere >= 32 && caractere != 127)
        {
            motivo += sf::String(caractere);
        }
    }
}

void JanelaRemover::processarBackspace()
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
    else if (motivoSelecionado &&
        !motivo.isEmpty())
    {
        motivo.erase(motivo.getSize() - 1, 1);
    }
}

void JanelaRemover::renderizar()
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

    // Rótulo do motivo.
    sf::Text rotuloMotivo(fonte, "Motivo da remoção", 17);
    rotuloMotivo.setFillColor(sf::Color::Black);
    rotuloMotivo.setPosition({ 30.f, 185.f });
    janela.draw(rotuloMotivo);

    // Caixa do motivo.
    sf::RectangleShape campoMotivo({ 640.f, 210.f });
    campoMotivo.setPosition({ 30.f, 220.f });
    campoMotivo.setFillColor(sf::Color(255, 245, 225));
    campoMotivo.setOutlineColor(sf::Color(222, 154, 64));
    campoMotivo.setOutlineThickness(2.f);
    janela.draw(campoMotivo);

    sf::Text textoMotivo(fonte, motivo, 17);
    textoMotivo.setFillColor(sf::Color::Black);
    textoMotivo.setPosition({ 40.f, 230.f });
    janela.draw(textoMotivo);

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

    // Botão Remover / Confirmar Remoção.
    sf::RectangleShape botaoRemover({ 150.f, 50.f });
    botaoRemover.setPosition({ 530.f, 480.f });
    botaoRemover.setFillColor(
        confirmarRemocao ? sf::Color(255, 190, 170)
        : sf::Color(255, 222, 179)
    );
    botaoRemover.setOutlineColor(sf::Color(222, 154, 64));
    botaoRemover.setOutlineThickness(2.f);
    janela.draw(botaoRemover);

    sf::Text textoRemover(
        fonte,
        confirmarRemocao ? "Confirmar Remocao" : "Remover",
        15
    );

    textoRemover.setFillColor(sf::Color::Black);
    textoRemover.setPosition(
        confirmarRemocao
        ? sf::Vector2f(545.f, 495.f)
        : sf::Vector2f(570.f, 493.f)
    );
    janela.draw(textoRemover);

    janela.display();
}
