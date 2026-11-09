#include "InterfaceGrafica.h"
#include "JanelaInserir.h"
#include "JanelaRemover.h"
#include "JanelaReservar.h"

#include <iostream>

InterfaceGrafica::InterfaceGrafica() :
    db(NULL),
    stmt(NULL),
    altura(720),
    largurasColunas{80.f, 100.f, 230.f, 90.f, 140.f, 230.f, 100.f, 130.f, 100.f },
    largura(1280),
    titulo("WareSim"),
    janela(sf::VideoMode({ static_cast<unsigned int>(largura), static_cast<unsigned int>(altura) }),titulo)
{
    if (!fonte.openFromFile("arial.ttf"))
    {
        // A janela ainda abre, mas os textos não serão desenhados.
    }

    if (sqlite3_open("armazem.db", &db) != SQLITE_OK) {
        std::cerr << "Erro ao abrir o banco de dados!" << std::endl;
        //return 1;
    }
}


InterfaceGrafica::~InterfaceGrafica() {
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void InterfaceGrafica::atualizar_database(){

    listaItensCache.clear();
    std::string sql = "SELECT lote,id,nome,volume,categoria,distribuidor,preco,data_validade,reservado FROM itens_em_estoque;";
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            ItemEstoque item;

            //LOTE:
            item.lote = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            //ID:
            item.id = std::to_string(sqlite3_column_int(stmt, 1));
            //NOME:

            //dados[2] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            std::string texto = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            item.nome = sf::String::fromUtf8(texto.begin(), texto.end());
            //VOLUME:
            char buffer[20];
            std::snprintf(buffer, sizeof(buffer), "%.2f", sqlite3_column_double(stmt, 3));
            item.volume = buffer;
            //CATEGORIA:
            texto = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            item.categoria = sf::String::fromUtf8(texto.begin(), texto.end());
            //DISTRIBUIDOR:
            texto = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            item.distribuidor = sf::String::fromUtf8(texto.begin(), texto.end());
            //PRECO:
            std::snprintf(buffer, sizeof(buffer), "%.2f", sqlite3_column_double(stmt, 6));
            item.preco = buffer;
            //DATA DE VALIDADE:
            item.data_validade = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
            //RESERVADO:
            item.reservado = std::to_string(sqlite3_column_int(stmt, 8));

            listaItensCache.push_back(item);
        }
    } else {
        std::cerr << "Erro na sintaxe do comando SQL ou a tabela nao existe." << std::endl;
    }

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
        "Lote",
        "Id bebida",
        "Nome",
        "Volume",
        "Categoria",
        "Distribuidor",
        "Preço",
        "Validade",
        "Flag reserva"
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
                atualizar_database();
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

void InterfaceGrafica::renderizarLista() {

    std::string dados[9];
    int i=0;

    for (const auto& item : listaItensCache) {

        dados[0] = item.lote;
        dados[1] = item.id;
        dados[2] = item.nome;
        dados[3] = item.volume;
        dados[4] = item.categoria;
        dados[5] = item.distribuidor;
        dados[6] = item.preco;
        dados[7] = item.data_validade;
        dados[8] = item.reservado;
        desenhar_linha(dados,i);
        i++;
    }
}

void InterfaceGrafica::desenhar_linha(string* dados, int i){

    const float tabelaX = 16.f;
    const float tabelaY = 86.f;
    const float alturaCabecalho = 30.f;
    const float alturaLinha = 52.f;

    float colunaX = tabelaX;
    float linhaY = tabelaY + alturaLinha*i + alturaCabecalho;

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
