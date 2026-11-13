#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include "Estoque.h"

class JanelaReservar
{
private:
    sf::RenderWindow janela;
    sf::Font fonte;
    Estoque* estoque;
    Estoque estoqueLocal;

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

    std::string mensagemStatus;
    bool statusErro = false;
    bool operacaoConcluida = false;

    void processarClique(int x, int y);
    void processarTeclado(char32_t caractere);
    void processarBackspace();
    void renderizar();

public:
    JanelaReservar(Estoque* estoqueRef = nullptr);
    void executar();
    bool teveSucesso() const { return operacaoConcluida; }
    const std::string& getMensagemStatus() const { return mensagemStatus; }
};
