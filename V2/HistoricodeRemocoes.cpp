#include "HistoricodeRemocoes.h"
#include <cstdlib>
#include <iostream>

HistoricodeRemocoes::HistoricodeRemocoes()
    : idRegistro(0),
      dataRemocao(""),
      quantidade(0),
      motivoRemocao(""),
      idLote(0),
      codigoLote(""),
      nomeBebida("")
{
}

HistoricodeRemocoes::HistoricodeRemocoes(
    int idRegistro,
    const std::string& dataRemocao,
    int quantidade,
    const std::string& motivoRemocao,
    const std::string& codigoLote,
    const std::string& nomeBebida
)
    : idRegistro(idRegistro),
      dataRemocao(dataRemocao),
      quantidade(quantidade),
      motivoRemocao(motivoRemocao),
      idLote(std::atoi(codigoLote.c_str())),
      codigoLote(codigoLote),
      nomeBebida(nomeBebida)
{
}

void HistoricodeRemocoes::registrarRemocao() {
    std::cout << "[Historico] Remocao registrada - Lote: " << codigoLote
              << " | Qtd: " << quantidade
              << " | Data: " << dataRemocao
              << " | Motivo: " << motivoRemocao << std::endl;
}

bool HistoricodeRemocoes::registrarRemocao(sqlite3* db) {
    if (!db)
        return false;

    const char* sql =
        "INSERT INTO historico_remocoes (dataRemocao, quantidade, motivoRemocao, idLote, nomeBebida) "
        "VALUES (?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_text(stmt, 1, dataRemocao.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, quantidade);
    sqlite3_bind_text(stmt, 3, motivoRemocao.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, codigoLote.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, nomeBebida.c_str(), -1, SQLITE_TRANSIENT);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);

    if (ok) {
        idRegistro = static_cast<int>(sqlite3_last_insert_rowid(db));
        registrarRemocao();
    }
    return ok;
}
