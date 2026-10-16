
#pragma once

#include <SFML/Graphics.hpp>

class JanelaRemover
{
private:
    sf::RenderWindow janela;
    sf::Font fonte;

    bool modoLote = true;
    bool confirmarRemocao = false;

    bool idLoteSelecionado = false;
    bool idItemSelecionado = false;
    bool quantidadeSelecionada = false;
    bool motivoSelecionado = false;

    sf::String idLote;
    sf::String idItem;
    sf::String quantidade;
    sf::String motivo;

    void processarClique(int x, int y);
    void processarTeclado(char32_t caractere);
    void processarBackspace();
    void renderizar();

public:
    JanelaRemover();
    void executar();
};
