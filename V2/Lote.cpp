#include "Lote.h"
#include <cstdlib>

Lote::Lote()
    : idLote(0),
      codigoLote(""),
      quantidadeInicial(0),
      dataValidade(""),
      quantidadeAtual(0)
{
}

Lote::Lote(int idLote, int quantidadeInicial, const std::string& dataValidade, int quantidadeAtual)
    : idLote(idLote),
      codigoLote(std::to_string(idLote)),
      quantidadeInicial(quantidadeInicial),
      dataValidade(dataValidade),
      quantidadeAtual(quantidadeAtual)
{
}

Lote::Lote(const std::string& codigoLote, int quantidadeInicial, const std::string& dataValidade, int quantidadeAtual)
    : idLote(std::atoi(codigoLote.c_str())),
      codigoLote(codigoLote),
      quantidadeInicial(quantidadeInicial),
      dataValidade(dataValidade),
      quantidadeAtual(quantidadeAtual)
{
}

int Lote::getQuantidade() const {
    return quantidadeAtual;
}
