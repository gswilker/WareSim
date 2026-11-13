#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <map>
#include "sqlite3.h"
#include "Estoque.h"

using namespace std;

struct ItemEstoque {
    std::string lote;
    std::string id;
    std::string nome;
    std::string volume;
    std::string categoria;
    std::string distribuidor;
    std::string preco;
    std::string data_validade;
    std::string reservado;
    int statusValidade = 0; // 0 = OK, 1 = <= 1 mes ([RNF002]), 2 = Vencido ([RF009])
    bool flagReserva = false;
};

// Figura 8: Diagrama de Classes & Quadro 11: Dicionario de informacoes da classe InterfaceGrafica
class InterfaceGrafica {
public:
    int altura;
    int largura;
    std::string titulo;
    sf::RenderWindow janela;
    sf::Font fonte;

    Estoque estoque;

private:
    float largurasColunas[9];
    std::vector<ItemEstoque> listaItensCache;
    std::vector<HistoricodeRemocoes> listaHistoricoCache;
    std::map<std::string, sf::Color> coresPorLote;

    bool modoHistorico = false;
    bool modoTelaCheia = false;
    bool erroBancoDeDados = false;
    std::string mensagemErroBanco;
    std::string mensagemSucessoOperacao;
    int scrollOffset = 0;

    sf::Color obterCorDoLote(const std::string& lote);
    void atualizarDimensoes(unsigned int novaLargura, unsigned int novaAltura);
    void alternarTelaCheia();

public:
    InterfaceGrafica();
    ~InterfaceGrafica();

    void executar();
    void renderizar();
    void renderizarMenu();
    void renderizarLista();
    void renderizarHistorico();
    void selecionar();
    void desenhar_linha(std::string* dados, int indiceVisivel, const ItemEstoque& item);
    float desenhar_linha_responsiva(std::string* dados, float linhaY, float limiteInferiorY, const ItemEstoque& item);
    void atualizar_database();

    // Metodos dos Diagramas de Sequencia (Figuras 3 a 7)
    void verEstoque();
    void verRemocoes();
    void acessarOpcaoInsercao();
    void acessarOpcaoRemocao();
    void acessarOpcaoReserva();
};
