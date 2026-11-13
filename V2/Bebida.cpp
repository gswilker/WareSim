#include "Bebida.h"
#include <sstream>
#include <iomanip>

Bebida::Bebida()
    : id(0),
      nome(""),
      volume(0),
      preco(0.f),
      distribuidor(""),
      flagValidade(false),
      flagReserva(false),
      dataEntrada(""),
      categoria(""),
      lote(""),
      dataValidade(""),
      destinoReserva(""),
      dataReserva(""),
      statusValidade(0)
{
}

Bebida::Bebida(
    int id,
    const std::string& lote,
    const std::string& nome,
    int volume,
    const std::string& categoria,
    const std::string& distribuidor,
    float preco,
    const std::string& dataValidade,
    bool flagReserva,
    const std::string& destinoReserva,
    const std::string& dataReserva,
    const std::string& dataEntrada,
    bool flagValidade,
    int statusValidade
)
    : id(id),
      nome(nome),
      volume(volume),
      preco(preco),
      distribuidor(distribuidor),
      flagValidade(flagValidade),
      flagReserva(flagReserva || !destinoReserva.empty()),
      dataEntrada(dataEntrada),
      categoria(categoria),
      lote(lote),
      dataValidade(dataValidade),
      destinoReserva(destinoReserva),
      dataReserva(dataReserva),
      statusValidade(statusValidade)
{
}

std::string Bebida::getInfo() const {
    std::ostringstream oss;
    oss << "Lote: " << lote
        << " | ID: " << id
        << " | Nome: " << nome
        << " | Volume: " << volume << "ml"
        << " | Categoria: " << categoria
        << " | Distribuidor: " << distribuidor
        << " | Preco: " << std::fixed << std::setprecision(2) << preco
        << " | Validade: " << dataValidade
        << " | Reserva: " << (isReservado() ? destinoReserva : "Nao Reservado");
    return oss.str();
}

bool Bebida::isValido() const {
    // Retorna true se o item nao esta vencido (statusValidade != 2)
    return statusValidade != 2;
}

bool Bebida::isReservado() const {
    // [RNF003] Se a coluna de destino estiver preenchida (ou flagReserva ativa), esta reservada
    return flagReserva || !destinoReserva.empty();
}
