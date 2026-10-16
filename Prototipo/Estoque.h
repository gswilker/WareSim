#pragma once

#include <string>
#include <list>
#include "Bebida.h"

using namespace std;
class Estoque {
private:
	int idEstoque;
	string localizacao;
	int totalItens;
	int capacidade;

public:
	Estoque();
	~Estoque();

	void inserirItem(Bebida item);
	void removerItem(int Id);
	Bebida buscarItem(int id);
	list<Bebida> listarEstoque();
};