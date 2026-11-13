#pragma once

#include <string>

// Figura 8: Diagrama de Classes & Quadro 10: Dicionario de informacoes da classe Reserva
class Reserva {
private:
    int idReserva;
    std::string cliente;
    std::string dataDaFesta;
    int quantidadeReservada;
    std::string observacao;
    bool confirmada;

public:
    Reserva()
        : idReserva(0),
          cliente(""),
          dataDaFesta(""),
          quantidadeReservada(0),
          observacao(""),
          confirmada(false)
    {
    }

    Reserva(int idReserva, const std::string& cliente, const std::string& dataDaFesta, int quantidadeReservada, const std::string& observacao = "")
        : idReserva(idReserva),
          cliente(cliente),
          dataDaFesta(dataDaFesta),
          quantidadeReservada(quantidadeReservada),
          observacao(observacao),
          confirmada(false)
    {
    }

    void confirmar() { confirmada = true; }
    void cancelar() { confirmada = false; }

    bool isConfirmada() const { return confirmada; }
    int getIdReserva() const { return idReserva; }
    const std::string& getCliente() const { return cliente; }
    const std::string& getDataDaFesta() const { return dataDaFesta; }
    int getQuantidadeReservada() const { return quantidadeReservada; }
    const std::string& getObservacao() const { return observacao; }
};
