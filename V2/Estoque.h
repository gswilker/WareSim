#pragma once

#include <string>
#include <list>
#include <vector>
#include "Bebida.h"
#include "Lote.h"
#include "Reserva.h"
#include "HistoricodeRemocoes.h"
#include "sqlite3.h"

using namespace std;

// Dados de formulario para [UC001] Inserir item/lote
struct DadosInsercao {
    std::string lote;
    std::string nome;
    std::string idBebida;
    std::string volume;
    std::string categoria;
    std::string distribuidor;
    std::string preco;
    std::string destinoOuDataReserva;
    std::string dataValidade;
    std::string quantidade;
    bool modoLote = true;
};

// Dados de formulario para [UC004] Reservar item/lote
struct DadosReserva {
    bool modoLote = true;
    std::string idLote;
    std::string idItem;
    std::string quantidade;
    std::string dataReserva;
    std::string localReserva;
};

// Dados de formulario para [UC005] Remover item/lote
struct DadosRemocao {
    bool modoLote = true;
    std::string idLote;
    std::string idItem;
    std::string quantidade;
    std::string motivo;
};

// Figura 8: Diagrama de Classes & Quadro 8: Dicionario de informacoes da classe Estoque
class Estoque {
private:
    int idEstoque;
    std::string localizacao;
    int totalItens;
    int capacidade;
    sqlite3* db;
    bool conectado;

    void inicializarEsquema();

public:
    Estoque();
    ~Estoque();

    bool conectarBanco(const std::string& caminho = "armazem.db");
    bool temConexao() const { return conectado && db != nullptr; }
    sqlite3* getDb() const { return db; }

    // Metodos do Diagrama de Classes (Figura 8)
    void inserirItem(Bebida item);
    void removerItem(int id);
    void RemoverItem(int id) { removerItem(id); }
    Bebida buscarItem(int id);
    Bebida BuscarItem(int id) { return buscarItem(id); }
    std::list<Bebida> listarEstoque();

    // [UC001] Inserir item/lote (Figura 3 & Quadro 1)
    bool validarDados(const DadosInsercao& dados, std::string& msgErro) const;
    bool salvarItem(const DadosInsercao& dados, std::string& msgErro);

    // [UC002] Visualizar historico de remocao (Figura 4 & Quadro 2)
    std::vector<HistoricodeRemocoes> consultarHistorico(bool& erroAcesso);

    // [UC003] Visualizar estoque (Figura 5 & Quadro 3)
    void atualizarFlagVencimento();
    std::vector<Bebida> consultarEstoque(bool& erroAcesso);

    // [UC004] Reservar item/lote (Figura 6 & Quadro 4)
    bool reservarItem(const DadosReserva& dados, std::string& msgErro);

    // [UC005] Remover item/lote (Figura 7 & Quadro 5)
    bool verificarItem(const DadosRemocao& dados, std::string& msgErro, std::string& resumoConfirmacao);
    bool executarRemocao(const DadosRemocao& dados, std::string& msgErro);

    // Utilitarios de data e validade ([RF009], [RNF002])
    static std::string obterDataAtual();
    static bool converterEValidarData(const std::string& entrada, std::string& dataPadraoISO);
    static int calcularStatusValidade(const std::string& dataValidade);

    int getIdEstoque() const { return idEstoque; }
    const std::string& getLocalizacao() const { return localizacao; }
    int getTotalItens() const { return totalItens; }
    int getCapacidade() const { return capacidade; }
};
