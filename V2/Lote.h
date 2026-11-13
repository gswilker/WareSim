#pragma once

#include <string>

// Figura 8: Diagrama de Classes & Quadro 7: Dicionario de informacoes da classe Lote
class Lote {
private:
    int idLote;
    std::string codigoLote;
    int quantidadeInicial;
    std::string dataValidade;
    int quantidadeAtual;

public:
    Lote();
    Lote(int idLote, int quantidadeInicial, const std::string& dataValidade, int quantidadeAtual);
    Lote(const std::string& codigoLote, int quantidadeInicial, const std::string& dataValidade, int quantidadeAtual);

    int getQuantidade() const;

    int getIdLote() const { return idLote; }
    void setIdLote(int valor) { idLote = valor; }

    const std::string& getCodigoLote() const { return codigoLote; }
    void setCodigoLote(const std::string& valor) { codigoLote = valor; }

    int getQuantidadeInicial() const { return quantidadeInicial; }
    void setQuantidadeInicial(int valor) { quantidadeInicial = valor; }

    const std::string& getDataValidade() const { return dataValidade; }
    void setDataValidade(const std::string& valor) { dataValidade = valor; }

    void setQuantidadeAtual(int valor) { quantidadeAtual = valor; }
};
