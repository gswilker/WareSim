#pragma once

#include <SFML/Graphics.hpp>
#include <string>

using namespace std;

class InterfaceGrafica {
public:
	int altura;
	int largura;
	string titulo;
	sf::RenderWindow janela; // Alterar nos diagramas
	sf::Font fonte; // Alterar nos diagramas

	bool exemploVisivel; // Temporário

public:
	InterfaceGrafica();
	~InterfaceGrafica();

	void executar();
	void renderizarMenu();
	void renderizarLista();
	void selecionar();
	void listar();
};