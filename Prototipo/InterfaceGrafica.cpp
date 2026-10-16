#include "InterfaceGrafica.h"
#include "JanelaInserir.h"
#include "JanelaRemover.h"
#include "JanelaReservar.h"

InterfaceGrafica::InterfaceGrafica() : 
    altura(720), 
    largura(1280), 
    titulo("WareSim"), 
    janela(sf::VideoMode({ static_cast<unsigned int>(largura), static_cast<unsigned int>(altura) }),titulo),
    exemploVisivel(false) // Temporario
{
    if (!fonte.openFromFile("C:/Windows/Fonts/arial.ttf"))
    {
        // A janela ainda abre, mas os textos não serão desenhados.
    }
}


InterfaceGrafica::~InterfaceGrafica() {

}

void InterfaceGrafica::executar()
{
    while (janela.isOpen())
    {
        while (const auto evento = janela.pollEvent())
        {
            if (evento->is<sf::Event::Closed>())
            {
                janela.close();
            }

            if (const auto* clique =
                evento->getIf<sf::Event::MouseButtonPressed>())
            {
                if (clique->button == sf::Mouse::Button::Left)
                    selecionar();
            }
        }

        renderizarMenu();
    }
}

void InterfaceGrafica::renderizarMenu()
{
    janela.clear(sf::Color(255, 227, 163)); // Tela de funco

    // Barra superior com os botões
    const float margem = 16.f;
    const float topo = 12.f;
    const float alturaBotao = 58.f;
    const float espaco = 0.f; // Espaçamento entre os botoes

    const std::string nomes[] = {
        "Listar/Recarregar",
        "Inserir",
        "Remover",
        "Reservar",
        "Historico"
    };

    const float larguras[] = {
        240.f, 250.f, 250.f, 250.f, 250.f
    };

    float x = margem;

    for (int i = 0; i < 5; ++i)
    {
        sf::RectangleShape botao({ // Definição do botão
            larguras[i],
            alturaBotao
        });
        botao.setPosition({ x, topo });

        sf::Color corNormal(255, 222, 179);
        sf::Color corHover(235, 202, 157);
        sf::Color corClique(255, 235, 205);

        sf::Vector2i posMouse = sf::Mouse::getPosition(janela);

        bool mouseSobre =
            posMouse.x >= x &&
            posMouse.x <= x + larguras[i] &&
            posMouse.y >= topo &&
            posMouse.y <= topo + alturaBotao;

        if (mouseSobre &&
            sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
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

        if (fonte.getInfo().family != "")
        {
            sf::Text texto(fonte, nomes[i], 19);
            texto.setFillColor(sf::Color::Black);
            texto.setPosition({ x + 12.f, topo + 18.f });

            janela.draw(texto);
        }

        x += larguras[i] + espaco;
    }

    // Área da tabela
    const float tabelaX = margem;
    const float tabelaY = 86.f;
    const float tabelaLargura = largura - 2.f * margem;
    const float tabelaAltura = altura - tabelaY - margem;

    // Fundo da tabela
    sf::RectangleShape fundoTabela({
        tabelaLargura,
        tabelaAltura
        });

    fundoTabela.setPosition({ tabelaX, tabelaY });
    fundoTabela.setFillColor(sf::Color(255, 222, 179));
    fundoTabela.setOutlineColor(sf::Color(222, 154, 64));
    fundoTabela.setOutlineThickness(2.f);

    janela.draw(fundoTabela);

    // Cabeçalho da tabela
    const std::string colunas[] = {
        "Id do lote",
        "Nome bebida",
        "Id bebida",
        "Volume",
        "Categoria",
        "Distribuidor",
        "Preco",
        "Flag reserva",
        "Flag Validade"
    };

    const float largurasColunas[] = {
        125.f, 170.f, 115.f, 90.f, 140.f,
        140.f, 100.f, 175.f, 193.f
    };

    const float alturaCabecalho = 30.f;
    float colunaX = tabelaX;

    for (int i = 0; i < 9; ++i)
    {
        // Divisória vertical da coluna
        sf::RectangleShape divisoria({
            2.f,
            alturaCabecalho
            });

        divisoria.setPosition({ colunaX, tabelaY });
        divisoria.setFillColor(sf::Color(222, 154, 64));

        janela.draw(divisoria);

        // Texto do cabeçalho
        if (fonte.getInfo().family != "")
        {
            sf::Text textoColuna(fonte, colunas[i], 17);
            textoColuna.setFillColor(sf::Color::Black);
            textoColuna.setStyle(sf::Text::Bold);
            textoColuna.setPosition({
                colunaX + 8.f,
                tabelaY + 8.f
                });

            janela.draw(textoColuna);
        }

        colunaX += largurasColunas[i];
    }

    // Borda inferior do cabeçalho
    sf::RectangleShape linhaCabecalho({
        tabelaLargura,
        2.f
    });

    linhaCabecalho.setPosition({
        tabelaX,
        tabelaY + alturaCabecalho
    });

    linhaCabecalho.setFillColor(sf::Color(222, 154, 64));
    janela.draw(linhaCabecalho);

    // Barra de rolagem, ainda não funcional
    const float barraX = tabelaX + tabelaLargura - 10.f;

    sf::RectangleShape trilho({
        6.f,
        tabelaAltura - 8.f
    });

    trilho.setPosition({ barraX, tabelaY + 4.f });
    trilho.setFillColor(sf::Color(240, 240, 240));

    janela.draw(trilho);

    sf::RectangleShape indicador({ 6.f, 90.f });

    indicador.setPosition({
        barraX,
        tabelaY + 4.f
        });

    indicador.setFillColor(sf::Color(160, 160, 160));
    janela.draw(indicador);

    renderizarLista();

    janela.display();
}

void InterfaceGrafica::selecionar() {
    sf::Vector2i posMouse = sf::Mouse::getPosition(janela);

    const float margem = 16.f;
    const float topo = 12.f;
    const float alturaBotao = 58.f;
    const float espaco = 0.f;

    const float larguras[] = {
        240.f, 250.f, 250.f, 250.f, 250.f
    };

    float x = margem;

    for (int i = 0; i < 5; ++i)
    {
        bool mouseSobre =
            posMouse.x >= x &&
            posMouse.x <= x + larguras[i] &&
            posMouse.y >= topo &&
            posMouse.y <= topo + alturaBotao;

        if (mouseSobre)
        {
            if (i == 0)
                listar();
            else if (i == 1)
            {
                JanelaInserir janelaInserir;
                janelaInserir.executar();
            }
            else if (i == 2)
            {
                JanelaRemover janelaRemover;
                janelaRemover.executar();
            }
            else if (i == 3)
            {
                JanelaReservar janelaReservar;
                janelaReservar.executar();
            }

            break;
        }

        x += larguras[i] + espaco;
    }
}

void InterfaceGrafica::listar() {
    exemploVisivel = true;
}

void InterfaceGrafica::renderizarLista() {

    if (!exemploVisivel)
        return;

    const std::string dados[] = {
        "204",
        "Heineken",
        "1024",
        "300ml",
        "Alcoolico",
        "DistA",
        "13,90",
        "Nao Reservada",
        "17/10/26"
    };

    const float largurasColunas[] = {
        125.f, 170.f, 115.f, 90.f, 140.f,
        140.f, 100.f, 175.f, 193.f
    };

    const float tabelaX = 16.f;
    const float tabelaY = 86.f;
    const float alturaCabecalho = 30.f;
    const float alturaLinha = 52.f;

    float colunaX = tabelaX;
    float linhaY = tabelaY + alturaCabecalho;

    for (int i = 0; i < 9; ++i)
    {
        sf::RectangleShape divisoria({
            2.f,
            alturaLinha
            });

        divisoria.setPosition({ colunaX, linhaY });
        divisoria.setFillColor(sf::Color(222, 154, 64));
        janela.draw(divisoria);

        sf::Text textoCelula(fonte, dados[i], 17);
        textoCelula.setFillColor(sf::Color::Black);
        textoCelula.setPosition({
            colunaX + 8.f,
            linhaY + 15.f
            });

        janela.draw(textoCelula);

        colunaX += largurasColunas[i];
    }

    sf::RectangleShape linhaInferior({
        1248.f,
        2.f
        });

    linhaInferior.setPosition({
        tabelaX,
        linhaY + alturaLinha
        });

    linhaInferior.setFillColor(sf::Color(222, 154, 64));
    janela.draw(linhaInferior);
}