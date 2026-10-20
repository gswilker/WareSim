
#include "JanelaInserir.h"

JanelaInserir::JanelaInserir()
    : janela(sf::VideoMode({ 760u, 700u }), "Inserir")
{
    if (!fonte.openFromFile("arial.ttf"))
    {
        janela.close();
        return;
    }

    // Coluna esquerda
    const float xEsquerda = 30.f;
    const float larguraCampo = 330.f;

    // Coluna direita
    const float xDireita = 400.f;

    const float yInicial = 140.f;
    const float distancia = 88.f;
    const float alturaCampo = 34.f;

    campos = {
        { "Id do lote",       "", xEsquerda, yInicial, larguraCampo, alturaCampo },
        { "Nome bebida",      "", xEsquerda, yInicial + distancia, larguraCampo, alturaCampo },
        { "Id bebida",        "", xEsquerda, yInicial + distancia * 2, larguraCampo, alturaCampo },
        { "Volume",           "", xEsquerda, yInicial + distancia * 3, larguraCampo, alturaCampo },
        { "Categoria",        "", xEsquerda, yInicial + distancia * 4, larguraCampo, alturaCampo },

        { "Distribuidor",     "", xDireita, yInicial, larguraCampo, alturaCampo },
        { "Preco",            "", xDireita, yInicial + distancia, larguraCampo, alturaCampo },
        { "Data Reserva",     "", xDireita, yInicial + distancia * 2, larguraCampo, alturaCampo },
        { "Data Validade",    "", xDireita, yInicial + distancia * 3, larguraCampo, alturaCampo },
        { "Qtd itens no lote","", xDireita, yInicial + distancia * 4, larguraCampo, alturaCampo }
    };

    // Inicialmente, a quantidade só pode ser preenchida no modo Lote.
    campos[9].habilitado = true;
}

void JanelaInserir::executar()
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
                    processarClique(clique->position.x, clique->position.y);
                }
            }
            else if (const auto* entrada =
                evento->getIf<sf::Event::TextEntered>())
            {
                processarTeclado(entrada->unicode);
            }
        }

        if (janela.isOpen())
            renderizar();
    }
}

void JanelaInserir::processarClique(int x, int y)
{
    // Seleção entre Lote e Item.
    if (y >= 48 && y <= 88)
    {
        if (x >= 30 && x <= 180)
        {
            modoLote = true;
            campos[9].habilitado = true;
        }
        else if (x >= 200 && x <= 350)
        {
            modoLote = false;
            campos[9].habilitado = false;
        }
    }

    // Botões Inserir e Cancelar: ambos fecham a janela.
    if (y >= 620 && y <= 670)
    {
        if ((x >= 400 && x <= 530) ||
            (x >= 550 && x <= 680))
        {
            janela.close();
            return;
        }
    }

    // Seleção de um campo de texto.
    campoSelecionado = -1;

    for (int i = 0; i < static_cast<int>(campos.size()); ++i)
    {
        const auto& campo = campos[i];

        if (campo.habilitado &&
            x >= campo.x &&
            x <= campo.x + campo.largura &&
            y >= campo.y &&
            y <= campo.y + campo.altura)
        {
            campoSelecionado = i;
            break;
        }
    }
}

void JanelaInserir::processarTeclado(char32_t caractere)
{
    if (campoSelecionado < 0 ||
        campoSelecionado >= static_cast<int>(campos.size()))
    {
        return;
    }

    auto& campo = campos[campoSelecionado];

    if (!campo.habilitado)
        return;

    // Backspace apaga o último caractere.
    if (caractere == 8)
    {
        if (!campo.valor.isEmpty())
            campo.valor.erase(campo.valor.getSize() - 1, 1);
    }
    // Ignora caracteres de controle, como Enter e Tab.
    else if (caractere >= 32 && caractere != 127)
    {
        campo.valor += sf::String(caractere);
    }
}

void JanelaInserir::renderizar()
{
    janela.clear(sf::Color(255, 227, 163));

    // Opções Lote e Item.
    sf::RectangleShape botaoLote({ 150.f, 40.f });
    botaoLote.setPosition({ 30.f, 48.f });
    botaoLote.setFillColor(
        modoLote ? sf::Color(235, 202, 157)
        : sf::Color(255, 222, 179));
    botaoLote.setOutlineColor(sf::Color(222, 154, 64));
    botaoLote.setOutlineThickness(2.f);
    janela.draw(botaoLote);

    sf::Text textoLote(fonte, "Lote", 18);
    textoLote.setFillColor(sf::Color::Black);
    textoLote.setPosition({ 80.f, 57.f });
    janela.draw(textoLote);

    sf::RectangleShape botaoItem({ 150.f, 40.f });
    botaoItem.setPosition({ 200.f, 48.f });
    botaoItem.setFillColor(
        !modoLote ? sf::Color(235, 202, 157)
        : sf::Color(255, 222, 179));
    botaoItem.setOutlineColor(sf::Color(222, 154, 64));
    botaoItem.setOutlineThickness(2.f);
    janela.draw(botaoItem);

    sf::Text textoItem(fonte, "Item", 18);
    textoItem.setFillColor(sf::Color::Black);
    textoItem.setPosition({ 250.f, 57.f });
    janela.draw(textoItem);

    // Campos de preenchimento.
    for (int i = 0; i < static_cast<int>(campos.size()); ++i)
    {
        const auto& campo = campos[i];

        sf::Text rotulo(fonte, campo.rotulo, 16);
        rotulo.setFillColor(sf::Color::Black);
        rotulo.setPosition({ campo.x, campo.y - 25.f });
        janela.draw(rotulo);

        sf::RectangleShape caixa({
            campo.largura,
            campo.altura
            });

        caixa.setPosition({ campo.x, campo.y });

        if (!campo.habilitado)
            caixa.setFillColor(sf::Color(190, 190, 190));
        else
            caixa.setFillColor(sf::Color(255, 245, 225));

        caixa.setOutlineColor(
            campoSelecionado == i
            ? sf::Color(80, 120, 200)
            : sf::Color(222, 154, 64));

        caixa.setOutlineThickness(2.f);
        janela.draw(caixa);

        sf::Text valor(fonte, campo.valor, 16);
        valor.setFillColor(sf::Color::Black);
        valor.setPosition({ campo.x + 8.f, campo.y + 7.f });
        janela.draw(valor);
    }

    // Botão Inserir.
    sf::RectangleShape botaoInserir({ 130.f, 45.f });
    botaoInserir.setPosition({ 400.f, 620.f });
    botaoInserir.setFillColor(sf::Color(255, 222, 179));
    botaoInserir.setOutlineColor(sf::Color(222, 154, 64));
    botaoInserir.setOutlineThickness(2.f);
    janela.draw(botaoInserir);

    sf::Text textoInserir(fonte, "Inserir", 18);
    textoInserir.setFillColor(sf::Color::Black);
    textoInserir.setPosition({ 430.f, 631.f });
    janela.draw(textoInserir);

    // Botão Cancelar.
    sf::RectangleShape botaoCancelar({ 130.f, 45.f });
    botaoCancelar.setPosition({ 550.f, 620.f });
    botaoCancelar.setFillColor(sf::Color(255, 222, 179));
    botaoCancelar.setOutlineColor(sf::Color(222, 154, 64));
    botaoCancelar.setOutlineThickness(2.f);
    janela.draw(botaoCancelar);

    sf::Text textoCancelar(fonte, "Cancelar", 18);
    textoCancelar.setFillColor(sf::Color::Black);
    textoCancelar.setPosition({ 568.f, 631.f });
    janela.draw(textoCancelar);

    janela.display();
}
