
#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class JanelaInserir
{
private:
    struct CampoTexto
    {
        std::string rotulo;
        sf::String valor;
        float x;
        float y;
        float largura;
        float altura;
        bool habilitado = true;
    };

    sf::RenderWindow janela;
    sf::Font fonte;

    std::vector<CampoTexto> campos;
    int campoSelecionado = -1;

    bool modoLote = true;

    void processarClique(int x, int y);
    void processarTeclado(char32_t caractere);
    void renderizar();

public:
    JanelaInserir();
    void executar();
};
