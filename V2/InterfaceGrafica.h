#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include "sqlite3.h"

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
};

class InterfaceGrafica {
public:
	int altura;
	int largura;
	string titulo;
	sf::RenderWindow janela; // Alterar nos diagramas
	sf::Font fonte; // Alterar nos diagramas

	sqlite3* db;
    sqlite3_stmt* stmt;

private:
    float largurasColunas[9];
    std::vector<ItemEstoque> listaItensCache;

public:
	InterfaceGrafica();
	~InterfaceGrafica();

	void executar();
	void renderizarMenu();
	void renderizarLista();
	//int callback(void* naoUsado, int numColunas, char** dadosColuna, char** nomeColunas);
	void conectarEBuscarEstoque();
	void selecionar();
	void desenhar_linha(string* dados,int i);
	void atualizar_database();
};
