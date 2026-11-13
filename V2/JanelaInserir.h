#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "Estoque.h"

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
    Estoque* estoque;
    Estoque estoqueLocal;

    std::vector<CampoTexto> campos;
    int campoSelecionado = -1;

    bool modoLote = true;
    std::string mensagemStatus;
    bool statusErro = false;
    bool operacaoConcluida = false;

    void processarClique(int x, int y);
    void processarTeclado(char32_t caractere);
    void processarBackspace();
    void avancarCampo();
    void renderizar();

public:
    JanelaInserir(Estoque* estoqueRef = nullptr);
    void executar();
    bool teveSucesso() const { return operacaoConcluida; }
    const std::string& getMensagemStatus() const { return mensagemStatus; }
};
