
#pragma once

#include <SFML/Graphics.hpp>

class JanelaReservar
{
private:
    sf::RenderWindow janela;
    sf::Font fonte;

    bool modoLote = true;

    bool idLoteSelecionado = false;
    bool idItemSelecionado = false;
    bool quantidadeSelecionada = false;
    bool dataSelecionada = false;
    bool localSelecionado = false;

    sf::String idLote;
    sf::String idItem;
    sf::String quantidade;
    sf::String dataReserva;
    sf::String localReserva;

    void processarClique(int x, int y);
    void processarTeclado(char32_t caractere);
    void processarBackspace();
    void renderizar();

public:
    JanelaReservar();
    void executar();
};
