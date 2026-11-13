#pragma once

#include <string>
#include "sqlite3.h"

// Figura 8: Diagrama de Classes & Quadro 9: Dicionario de informacoes da classe HistoricodeRemocoes
class HistoricodeRemocoes {
private:
    int idRegistro;
    std::string dataRemocao;
    int quantidade;
    std::string motivoRemocao;
    int idLote;
    std::string codigoLote;
    std::string nomeBebida;

public:
    HistoricodeRemocoes();
    HistoricodeRemocoes(
        int idRegistro,
        const std::string& dataRemocao,
        int quantidade,
        const std::string& motivoRemocao,
        const std::string& codigoLote,
        const std::string& nomeBebida = ""
    );

    // Metodo do Diagrama de Classes (Figura 8)
    void registrarRemocao();
    bool registrarRemocao(sqlite3* db);

    int getIdRegistro() const { return idRegistro; }
    const std::string& getDataRemocao() const { return dataRemocao; }
    int getQuantidade() const { return quantidade; }
    const std::string& getMotivoRemocao() const { return motivoRemocao; }
    int getIdLote() const { return idLote; }
    const std::string& getCodigoLote() const { return codigoLote; }
    const std::string& getNomeBebida() const { return nomeBebida; }
};
