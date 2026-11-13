#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include "Estoque.h"
#include "SFMLCompat.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <fstream>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#undef interface
#pragma comment(lib, "winmm.lib")
#endif

namespace WareSimUI {

static std::vector<uint8_t> criarWavEmMemoria(const std::vector<int16_t>& pcm, uint32_t sampleRate = 44100) {
    const uint32_t dataSize = static_cast<uint32_t>(pcm.size() * sizeof(int16_t));
    const uint32_t fileSize = 36 + dataSize;
    std::vector<uint8_t> wav(44 + dataSize);

    auto write32 = [&](size_t offset, uint32_t val) {
        std::memcpy(&wav[offset], &val, 4);
    };
    auto write16 = [&](size_t offset, uint16_t val) {
        std::memcpy(&wav[offset], &val, 2);
    };

    std::memcpy(&wav[0], "RIFF", 4);
    write32(4, fileSize);
    std::memcpy(&wav[8], "WAVEfmt ", 8);
    write32(16, 16);             // Subchunk1Size (PCM)
    write16(20, 1);              // AudioFormat (1 = PCM)
    write16(22, 1);              // NumChannels (1 = Mono)
    write32(24, sampleRate);     // SampleRate
    write32(28, sampleRate * 2); // ByteRate
    write16(32, 2);              // BlockAlign
    write16(34, 16);             // BitsPerSample
    std::memcpy(&wav[36], "data", 4);
    write32(40, dataSize);
    if (dataSize > 0) {
        std::memcpy(&wav[44], pcm.data(), dataSize);
    }
    return wav;
}

static bool carregarSoundBuffer(sf::SoundBuffer& buffer, const std::vector<int16_t>& samples, uint32_t sampleRate = 44100) {
#if SFML_VERSION_MAJOR >= 3
    std::vector<sf::SoundChannel> channels = { sf::SoundChannel::Mono };
    return buffer.loadFromSamples(samples.data(), samples.size(), 1, sampleRate, channels);
#else
    return buffer.loadFromSamples(samples.data(), samples.size(), 1, sampleRate);
#endif
}

// [REU002] Os botoes devem emitir um som ao serem selecionados.
void tocarSomBotao() {
    static bool inicializado = false;
    static bool sfmlAudioOk = false;
    static sf::SoundBuffer bufferClique;
    static std::vector<uint8_t> wavClique;

    if (!inicializado) {
        inicializado = true;
        const uint32_t sampleRate = 44100;
        const uint32_t leadIn = 1100;     // ~25ms para acordar o mixer de audio do Windows
        const uint32_t toneSamples = 4851; // ~110ms de som de clique nitido (dois tons: 700Hz -> 1050Hz)
        std::vector<int16_t> pcm(leadIn + toneSamples, 0);

        for (uint32_t i = 0; i < toneSamples; ++i) {
            double t = static_cast<double>(i) / sampleRate;
            double freq = (i < toneSamples / 3) ? 740.0 : 1108.73; // F#5 -> C#6
            double localT = (i < toneSamples / 3)
                ? t
                : static_cast<double>(i - toneSamples / 3) / sampleRate;
            double env = std::exp(-localT * 38.0);
            double wave = std::sin(2.0 * 3.14159265358979 * freq * t) * 0.75
                        + std::sin(2.0 * 3.14159265358979 * (freq * 2.0) * t) * 0.25;
            pcm[leadIn + i] = static_cast<int16_t>(wave * env * 26000.0);
        }

        sfmlAudioOk = carregarSoundBuffer(bufferClique, pcm, sampleRate);
        wavClique = criarWavEmMemoria(pcm, sampleRate);
    }

    if (sfmlAudioOk) {
#if SFML_VERSION_MAJOR >= 3
        static sf::Sound somClique(bufferClique);
#else
        static sf::Sound somClique;
        somClique.setBuffer(bufferClique);
#endif
        somClique.setVolume(100.f);
        somClique.play();
    }
#ifdef _WIN32
    PlaySoundA(reinterpret_cast<LPCSTR>(wavClique.data()), NULL, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
#endif
}

void tocarSomSucesso() {
    static bool inicializado = false;
    static bool sfmlAudioOk = false;
    static sf::SoundBuffer bufferSucesso;
    static std::vector<uint8_t> wavSucesso;

    if (!inicializado) {
        inicializado = true;
        const uint32_t sampleRate = 44100;
        const uint32_t leadIn = 1100;
        const uint32_t toneSamples = 9702; // ~220ms acorde ascendente de confirmacao
        std::vector<int16_t> pcm(leadIn + toneSamples, 0);

        for (uint32_t i = 0; i < toneSamples; ++i) {
            double t = static_cast<double>(i) / sampleRate;
            bool segundaNota = (i >= toneSamples / 2);
            double freq = segundaNota ? 1046.50 : 783.99; // G5 -> C6
            double localT = segundaNota
                ? static_cast<double>(i - toneSamples / 2) / sampleRate
                : t;
            double env = std::exp(-localT * 22.0);
            double wave = std::sin(2.0 * 3.14159265358979 * freq * t) * 0.8
                        + std::sin(2.0 * 3.14159265358979 * (freq * 1.5) * t) * 0.2;
            pcm[leadIn + i] = static_cast<int16_t>(wave * env * 26000.0);
        }

        sfmlAudioOk = carregarSoundBuffer(bufferSucesso, pcm, sampleRate);
        wavSucesso = criarWavEmMemoria(pcm, sampleRate);
    }

    if (sfmlAudioOk) {
#if SFML_VERSION_MAJOR >= 3
        static sf::Sound somSucesso(bufferSucesso);
#else
        static sf::Sound somSucesso;
        somSucesso.setBuffer(bufferSucesso);
#endif
        somSucesso.setVolume(100.f);
        somSucesso.play();
    }
#ifdef _WIN32
    PlaySoundA(reinterpret_cast<LPCSTR>(wavSucesso.data()), NULL, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
#endif
}

void tocarSomErro() {
    static bool inicializado = false;
    static bool sfmlAudioOk = false;
    static sf::SoundBuffer bufferErro;
    static std::vector<uint8_t> wavErro;

    if (!inicializado) {
        inicializado = true;
        const uint32_t sampleRate = 44100;
        const uint32_t leadIn = 1100;
        const uint32_t toneSamples = 8820; // ~200ms alerta grave duplo
        std::vector<int16_t> pcm(leadIn + toneSamples, 0);

        for (uint32_t i = 0; i < toneSamples; ++i) {
            double t = static_cast<double>(i) / sampleRate;
            bool segundaNota = (i >= toneSamples / 2);
            double freq = segundaNota ? 240.0 : 320.0;
            double localT = segundaNota
                ? static_cast<double>(i - toneSamples / 2) / sampleRate
                : t;
            double env = std::exp(-localT * 24.0);
            double wave = std::sin(2.0 * 3.14159265358979 * freq * t) * 0.7
                        + std::sin(2.0 * 3.14159265358979 * (freq * 3.0) * t) * 0.3;
            pcm[leadIn + i] = static_cast<int16_t>(wave * env * 26000.0);
        }

        sfmlAudioOk = carregarSoundBuffer(bufferErro, pcm, sampleRate);
        wavErro = criarWavEmMemoria(pcm, sampleRate);
    }

    if (sfmlAudioOk) {
#if SFML_VERSION_MAJOR >= 3
        static sf::Sound somErro(bufferErro);
#else
        static sf::Sound somErro;
        somErro.setBuffer(bufferErro);
#endif
        somErro.setVolume(100.f);
        somErro.play();
    }
#ifdef _WIN32
    PlaySoundA(reinterpret_cast<LPCSTR>(wavErro.data()), NULL, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
#endif
}

} // namespace WareSimUI

static std::string trimStr(const std::string& s) {
    size_t ini = 0;
    while (ini < s.size() && (s[ini] == ' ' || s[ini] == '\t' || s[ini] == '\r' || s[ini] == '\n'))
        ++ini;
    size_t fim = s.size();
    while (fim > ini && (s[fim - 1] == ' ' || s[fim - 1] == '\t' || s[fim - 1] == '\r' || s[fim - 1] == '\n'))
        --fim;
    return s.substr(ini, fim - ini);
}

Estoque::Estoque()
    : idEstoque(1),
      localizacao("Armazem Principal"),
      totalItens(0),
      capacidade(10000),
      db(nullptr),
      conectado(false)
{
    conectarBanco("armazem.db");
}

Estoque::~Estoque() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
    conectado = false;
}

bool Estoque::conectarBanco(const std::string& caminho) {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
    conectado = false;

    const std::string candidatos[] = {
        caminho,
        "../" + caminho,
        "../../" + caminho,
        "V2/" + caminho
    };

    std::string caminhoEncontrado = "";
    for (const auto& cand : candidatos) {
        std::ifstream f(cand, std::ios::binary);
        if (f.good()) {
            caminhoEncontrado = cand;
            break;
        }
    }

    // [RN001] O banco de dados deve existir e deve haver conexao entre o WareSim e o banco de dados
    if (caminhoEncontrado.empty()) {
        std::cerr << "Erro: Arquivo de banco de dados '" << caminho << "' nao encontrado!" << std::endl;
        return false;
    }

    if (sqlite3_open_v2(caminhoEncontrado.c_str(), &db, SQLITE_OPEN_READWRITE, nullptr) != SQLITE_OK) {
        std::cerr << "Erro ao abrir o banco de dados: " << (db ? sqlite3_errmsg(db) : "desconhecido") << std::endl;
        if (db) {
            sqlite3_close(db);
            db = nullptr;
        }
        return false;
    }

    conectado = true;
    inicializarEsquema();
    return true;
}

void Estoque::inicializarEsquema() {
    if (!temConexao())
        return;

    const char* sqlItens =
        "CREATE TABLE IF NOT EXISTS \"itens_em_estoque\" ("
        "\"id\" INTEGER PRIMARY KEY AUTOINCREMENT,"
        "\"lote\" TEXT NOT NULL,"
        "\"nome\" TEXT NOT NULL,"
        "\"volume\" REAL,"
        "\"categoria\" TEXT,"
        "\"distribuidor\" TEXT,"
        "\"preco\" REAL,"
        "\"data_validade\" TEXT NOT NULL,"
        "\"reservado\" INTEGER DEFAULT 0"
        ");";
    sqlite3_exec(db, sqlItens, nullptr, nullptr, nullptr);

    // Garante colunas extras para destino de reserva ([RNF003]), data de reserva, data de entrada e flag de validade
    sqlite3_exec(db, "ALTER TABLE itens_em_estoque ADD COLUMN destino_reserva TEXT DEFAULT '';", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "ALTER TABLE itens_em_estoque ADD COLUMN data_reserva TEXT DEFAULT '';", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "ALTER TABLE itens_em_estoque ADD COLUMN data_entrada TEXT DEFAULT '';", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "ALTER TABLE itens_em_estoque ADD COLUMN flag_validade INTEGER DEFAULT 0;", nullptr, nullptr, nullptr);

    // Tabela de Lotes (Quadro 7)
    const char* sqlLotes =
        "CREATE TABLE IF NOT EXISTS \"lotes\" ("
        "\"idLote\" TEXT PRIMARY KEY,"
        "\"quantidadeInicial\" INTEGER NOT NULL,"
        "\"dataValidade\" TEXT NOT NULL,"
        "\"quantidadeAtual\" INTEGER NOT NULL"
        ");";
    sqlite3_exec(db, sqlLotes, nullptr, nullptr, nullptr);

    // Tabela de Historico de Remocoes (Quadro 9, [RF012], [UC002], [UC005])
    const char* sqlHistorico =
        "CREATE TABLE IF NOT EXISTS \"historico_remocoes\" ("
        "\"idRegistro\" INTEGER PRIMARY KEY AUTOINCREMENT,"
        "\"dataRemocao\" TEXT NOT NULL,"
        "\"quantidade\" INTEGER NOT NULL,"
        "\"motivoRemocao\" TEXT NOT NULL,"
        "\"idLote\" TEXT NOT NULL,"
        "\"nomeBebida\" TEXT DEFAULT ''"
        ");";
    sqlite3_exec(db, sqlHistorico, nullptr, nullptr, nullptr);

    // Tabela de Reservas (Quadro 10, [RF007], [UC004])
    const char* sqlReservas =
        "CREATE TABLE IF NOT EXISTS \"reservas\" ("
        "\"idReserva\" INTEGER PRIMARY KEY AUTOINCREMENT,"
        "\"cliente\" TEXT NOT NULL,"
        "\"dataDaFesta\" TEXT NOT NULL,"
        "\"quantidadeReservada\" INTEGER NOT NULL,"
        "\"observacao\" TEXT DEFAULT ''"
        ");";
    sqlite3_exec(db, sqlReservas, nullptr, nullptr, nullptr);
}

std::string Estoque::obterDataAtual() {
    std::time_t t = std::time(nullptr);
    std::tm tmLocal{};
#ifdef _WIN32
    localtime_s(&tmLocal, &t);
#else
    localtime_r(&t, &tmLocal);
#endif
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d",
                  tmLocal.tm_year + 1900, tmLocal.tm_mon + 1, tmLocal.tm_mday);
    return std::string(buf);
}

bool Estoque::converterEValidarData(const std::string& entrada, std::string& dataPadraoISO) {
    std::string s = trimStr(entrada);
    if (s.empty())
        return false;

    int a = 0, b = 0, c = 0;
    char sep1 = 0, sep2 = 0;
    std::istringstream iss(s);
    if (!(iss >> a >> sep1 >> b >> sep2 >> c))
        return false;
    if ((sep1 != '-' && sep1 != '/') || (sep2 != '-' && sep2 != '/'))
        return false;

    int ano = 0, mes = 0, dia = 0;
    if (a >= 1900) {
        // Formato AAAA-MM-DD ou AAAA/MM/DD
        ano = a;
        mes = b;
        dia = c;
    } else {
        // Formato DD/MM/AAAA ou DD/MM/AA
        dia = a;
        mes = b;
        ano = (c < 100) ? (2000 + c) : c;
    }

    if (ano < 1900 || ano > 2200 || mes < 1 || mes > 12 || dia < 1 || dia > 31)
        return false;

    int diasMes[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    bool bissexto = (ano % 4 == 0 && (ano % 100 != 0 || ano % 400 == 0));
    if (bissexto)
        diasMes[2] = 29;
    if (dia > diasMes[mes])
        return false;

    char buf[16];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d", ano, mes, dia);
    dataPadraoISO = buf;
    return true;
}

int Estoque::calcularStatusValidade(const std::string& dataValidade) {
    std::string iso;
    if (!converterEValidarData(dataValidade, iso))
        return 0;

    int ano = 0, mes = 0, dia = 0;
    if (std::sscanf(iso.c_str(), "%d-%d-%d", &ano, &mes, &dia) != 3)
        return 0;

    std::time_t agora = std::time(nullptr);
    std::tm tmHoje{};
#ifdef _WIN32
    localtime_s(&tmHoje, &agora);
#else
    localtime_r(&agora, &tmHoje);
#endif
    tmHoje.tm_hour = 0;
    tmHoje.tm_min = 0;
    tmHoje.tm_sec = 0;
    tmHoje.tm_isdst = -1;

    std::tm tmVal{};
    tmVal.tm_year = ano - 1900;
    tmVal.tm_mon = mes - 1;
    tmVal.tm_mday = dia;
    tmVal.tm_hour = 0;
    tmVal.tm_min = 0;
    tmVal.tm_sec = 0;
    tmVal.tm_isdst = -1;

    std::time_t tHoje = std::mktime(&tmHoje);
    std::time_t tVal = std::mktime(&tmVal);

    double diffDias = std::difftime(tVal, tHoje) / (60.0 * 60.0 * 24.0);

    // [RF009] Sinalizar itens vencidos e proximos da data de vencimento
    // [RNF002] A sinalizacao de vencimento deve ser feita um mes (30 dias) antes do prazo
    if (diffDias < 0.0)
        return 2; // Vencido
    if (diffDias <= 31.0)
        return 1; // Proximo ao vencimento (<= 1 mes)
    return 0;     // Valido (OK)
}

void Estoque::atualizarFlagVencimento() {
    if (!temConexao())
        return;

    const char* sqlSelect = "SELECT id, data_validade FROM itens_em_estoque;";
    sqlite3_stmt* stmt = nullptr;
    std::vector<std::pair<int, int>> atualizacoes;

    if (sqlite3_prepare_v2(db, sqlSelect, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            const char* dt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            int status = calcularStatusValidade(dt ? dt : "");
            atualizacoes.push_back({ id, status });
        }
        sqlite3_finalize(stmt);
    }

    sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
    const char* sqlUpdate = "UPDATE itens_em_estoque SET flag_validade = ? WHERE id = ?;";
    sqlite3_stmt* stmtUpd = nullptr;
    if (sqlite3_prepare_v2(db, sqlUpdate, -1, &stmtUpd, nullptr) == SQLITE_OK) {
        for (const auto& par : atualizacoes) {
            sqlite3_bind_int(stmtUpd, 1, par.second > 0 ? 1 : 0);
            sqlite3_bind_int(stmtUpd, 2, par.first);
            sqlite3_step(stmtUpd);
            sqlite3_reset(stmtUpd);
        }
        sqlite3_finalize(stmtUpd);
    }
    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
}

std::vector<Bebida> Estoque::consultarEstoque(bool& erroAcesso) {
    std::vector<Bebida> resultado;
    if (!temConexao()) {
        erroAcesso = true;
        return resultado;
    }
    erroAcesso = false;

    // Figura 5 ([UC003]): 1.1.1.2 atualizarFlagVencimento()
    atualizarFlagVencimento();

    const char* sql =
        "SELECT id, lote, nome, volume, categoria, distribuidor, preco, data_validade, "
        "reservado, COALESCE(destino_reserva, ''), COALESCE(data_reserva, ''), "
        "COALESCE(data_entrada, ''), COALESCE(flag_validade, 0) "
        "FROM itens_em_estoque ORDER BY lote ASC, id ASC;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        erroAcesso = true;
        return resultado;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char* cLote = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* cNome = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        int volume = static_cast<int>(std::round(sqlite3_column_double(stmt, 3)));
        const char* cCat = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        const char* cDist = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        float preco = static_cast<float>(sqlite3_column_double(stmt, 6));
        const char* cVal = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        int reservadoInt = sqlite3_column_int(stmt, 8);
        const char* cDest = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        const char* cDtRes = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        const char* cDtEnt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));

        std::string strVal = cVal ? cVal : "";
        int statusVal = calcularStatusValidade(strVal);

        std::string strDest = cDest ? cDest : "";
        std::string strDtRes = cDtRes ? cDtRes : "";
        bool isRes = (reservadoInt != 0) || !strDest.empty();
        if (isRes && strDest.empty()) {
            strDest = !strDtRes.empty() ? ("Reservado (" + strDtRes + ")") : "Reservado";
        }

        Bebida b(
            id,
            cLote ? cLote : "",
            cNome ? cNome : "",
            volume,
            cCat ? cCat : "",
            cDist ? cDist : "",
            preco,
            strVal,
            isRes,
            strDest,
            strDtRes,
            cDtEnt ? cDtEnt : "",
            statusVal > 0,
            statusVal
        );
        resultado.push_back(b);
    }

    sqlite3_finalize(stmt);
    totalItens = static_cast<int>(resultado.size());
    return resultado;
}

void Estoque::inserirItem(Bebida item) {
    DadosInsercao d;
    d.lote = item.getLote();
    d.nome = item.getNome();
    d.idBebida = std::to_string(item.getId());
    d.volume = std::to_string(item.getVolume());
    d.categoria = item.getCategoria();
    d.distribuidor = item.getDistribuidor();
    d.preco = std::to_string(item.getPreco());
    d.destinoOuDataReserva = item.getDestinoReserva();
    d.dataValidade = item.getDataValidade();
    d.quantidade = "1";
    d.modoLote = false;
    std::string msgErro;
    salvarItem(d, msgErro);
}

void Estoque::removerItem(int id) {
    DadosRemocao d;
    d.modoLote = false;
    d.idItem = std::to_string(id);
    d.motivo = "Remocao direta";
    std::string msgErro;
    executarRemocao(d, msgErro);
}

Bebida Estoque::buscarItem(int id) {
    bool erro = false;
    auto lista = consultarEstoque(erro);
    for (const auto& b : lista) {
        if (b.getId() == id)
            return b;
    }
    return Bebida();
}

std::list<Bebida> Estoque::listarEstoque() {
    bool erro = false;
    auto vec = consultarEstoque(erro);
    return std::list<Bebida>(vec.begin(), vec.end());
}

// [UC001] Validar dados de insercao (Quadro 1 & Figura 3: 1.1.1.1 validarDados())
bool Estoque::validarDados(const DadosInsercao& dados, std::string& msgErro) const {
    if (!temConexao()) {
        msgErro = "Erro: Sem conexao com o banco de dados!";
        return false;
    }

    std::string lote = trimStr(dados.lote);
    std::string nome = trimStr(dados.nome);
    std::string idBeb = trimStr(dados.idBebida);
    std::string vol = trimStr(dados.volume);
    std::string cat = trimStr(dados.categoria);
    std::string dist = trimStr(dados.distribuidor);
    std::string precoStr = trimStr(dados.preco);
    std::string valStr = trimStr(dados.dataValidade);
    std::string qtdStr = trimStr(dados.quantidade);

    // [RN001] Todos os campos obrigatorios devem ser preenchidos.
    // No modo Item (!dados.modoLote), o item pode ser avulso (sem Id do lote) e Id da bebida pode ser automatico se vazio.
    if ((dados.modoLote && lote.empty()) || nome.empty() || vol.empty() ||
        cat.empty() || dist.empty() || precoStr.empty() || valStr.empty() ||
        (dados.modoLote && qtdStr.empty())) {
        msgErro = "Erro [RN001]: Preencha todos os campos obrigatorios!";
        return false;
    }

    // Valida formato numerico de ID (se informado), Volume, Preco e Quantidade
    if (!idBeb.empty()) {
        try {
            int idVal = std::stoi(idBeb);
            if (idVal < 0) {
                msgErro = "Erro: Id da bebida deve ser um numero positivo!";
                return false;
            }
        } catch (...) {
            msgErro = "Erro: Id da bebida invalido (use apenas numeros)!";
            return false;
        }
    }

    std::string volNorm = vol;
    std::replace(volNorm.begin(), volNorm.end(), ',', '.');
    try {
        double v = std::stod(volNorm);
        if (v <= 0.0) {
            msgErro = "Erro: Volume deve ser maior que zero!";
            return false;
        }
    } catch (...) {
        msgErro = "Erro: Volume invalido!";
        return false;
    }

    std::string precoNorm = precoStr;
    std::replace(precoNorm.begin(), precoNorm.end(), ',', '.');
    try {
        double p = std::stod(precoNorm);
        if (p < 0.0) {
            msgErro = "Erro: Preco nao pode ser negativo!";
            return false;
        }
    } catch (...) {
        msgErro = "Erro: Preco invalido!";
        return false;
    }

    std::string dataISO;
    if (!converterEValidarData(valStr, dataISO)) {
        msgErro = "Erro: Data de validade invalida (use DD/MM/AAAA ou AAAA-MM-DD)!";
        return false;
    }

    if (dados.modoLote) {
        try {
            int q = std::stoi(qtdStr);
            if (q <= 0 || q > 5000) {
                msgErro = "Erro: Quantidade no lote deve ser entre 1 e 5000!";
                return false;
            }
        } catch (...) {
            msgErro = "Erro: Quantidade de itens no lote invalida!";
            return false;
        }
    }

    return true;
}

// [UC001] Salvar item/lote no banco de dados (Figura 3: 1.1.1 salvarItem / 1.1.1.2 salvar)
bool Estoque::salvarItem(const DadosInsercao& dados, std::string& msgErro) {
    if (!validarDados(dados, msgErro))
        return false;

    std::string lote = trimStr(dados.lote);
    if (lote.empty()) {
        lote = "-"; // Item individual avulso (sem lote)
    }
    std::string nome = trimStr(dados.nome);
    std::string idBebTrim = trimStr(dados.idBebida);
    int idInicial = idBebTrim.empty() ? 0 : std::stoi(idBebTrim);

    std::string volNorm = trimStr(dados.volume);
    std::replace(volNorm.begin(), volNorm.end(), ',', '.');
    double volume = std::stod(volNorm);

    std::string categoria = trimStr(dados.categoria);
    std::string distribuidor = trimStr(dados.distribuidor);

    std::string precoNorm = trimStr(dados.preco);
    std::replace(precoNorm.begin(), precoNorm.end(), ',', '.');
    double preco = std::stod(precoNorm);

    std::string dataValISO;
    converterEValidarData(dados.dataValidade, dataValISO);

    int qtd = dados.modoLote ? std::stoi(trimStr(dados.quantidade)) : 1;

    std::string reservaInput = trimStr(dados.destinoOuDataReserva);
    bool temReserva = !reservaInput.empty() && reservaInput != "0" &&
                      reservaInput != "-" && reservaInput != "N/A" &&
                      reservaInput != "Nao" && reservaInput != "nao";
    std::string destinoReserva = temReserva ? reservaInput : "";
    int statusVal = calcularStatusValidade(dataValISO);
    std::string dataEntrada = obterDataAtual();

    sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

    // Verifica se o idInicial solicitado esta livre
    bool usarIdExplicito = (idInicial > 0);
    if (usarIdExplicito) {
        sqlite3_stmt* stCheck = nullptr;
        if (sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM itens_em_estoque WHERE id >= ? AND id < ?;", -1, &stCheck, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stCheck, 1, idInicial);
            sqlite3_bind_int(stCheck, 2, idInicial + qtd);
            if (sqlite3_step(stCheck) == SQLITE_ROW && sqlite3_column_int(stCheck, 0) > 0) {
                usarIdExplicito = false; // Auto-incrementa se ja existir conflito de ID
            }
            sqlite3_finalize(stCheck);
        }
    }

    const char* sqlComId =
        "INSERT INTO itens_em_estoque "
        "(id, lote, nome, volume, categoria, distribuidor, preco, data_validade, reservado, destino_reserva, data_reserva, data_entrada, flag_validade) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

    const char* sqlSemId =
        "INSERT INTO itens_em_estoque "
        "(lote, nome, volume, categoria, distribuidor, preco, data_validade, reservado, destino_reserva, data_reserva, data_entrada, flag_validade) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    const char* sqlUsado = usarIdExplicito ? sqlComId : sqlSemId;
    if (sqlite3_prepare_v2(db, sqlUsado, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        msgErro = "Erro ao preparar insercao no banco de dados!";
        return false;
    }

    for (int i = 0; i < qtd; ++i) {
        int col = 1;
        if (usarIdExplicito) {
            sqlite3_bind_int(stmt, col++, idInicial + i);
        }
        sqlite3_bind_text(stmt, col++, lote.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, col++, nome.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, col++, volume);
        sqlite3_bind_text(stmt, col++, categoria.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, col++, distribuidor.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, col++, preco);
        sqlite3_bind_text(stmt, col++, dataValISO.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, col++, temReserva ? 1 : 0);
        sqlite3_bind_text(stmt, col++, destinoReserva.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, col++, destinoReserva.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, col++, dataEntrada.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, col++, statusVal > 0 ? 1 : 0);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
            msgErro = "Erro ao inserir item no banco de dados!";
            return false;
        }
        sqlite3_reset(stmt);
    }
    sqlite3_finalize(stmt);

    // Atualiza ou insere registro do Lote (Quadro 7), apenas quando o item pertence a um lote
    if (!lote.empty() && lote != "-") {
        const char* sqlLote =
            "INSERT INTO lotes (idLote, quantidadeInicial, dataValidade, quantidadeAtual) "
            "VALUES (?, ?, ?, ?) "
            "ON CONFLICT(idLote) DO UPDATE SET "
            "quantidadeInicial = quantidadeInicial + excluded.quantidadeInicial, "
            "quantidadeAtual = quantidadeAtual + excluded.quantidadeAtual, "
            "dataValidade = excluded.dataValidade;";
        sqlite3_stmt* stmtLote = nullptr;
        if (sqlite3_prepare_v2(db, sqlLote, -1, &stmtLote, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmtLote, 1, lote.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmtLote, 2, qtd);
            sqlite3_bind_text(stmtLote, 3, dataValISO.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmtLote, 4, qtd);
            sqlite3_step(stmtLote);
            sqlite3_finalize(stmtLote);
        }
    }

    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
    return true;
}

// [UC002] Visualizar historico de remocao (Figura 4 & Quadro 2)
std::vector<HistoricodeRemocoes> Estoque::consultarHistorico(bool& erroAcesso) {
    std::vector<HistoricodeRemocoes> lista;
    if (!temConexao()) {
        erroAcesso = true;
        return lista;
    }
    erroAcesso = false;

    const char* sql =
        "SELECT idRegistro, dataRemocao, quantidade, motivoRemocao, idLote, COALESCE(nomeBebida, '') "
        "FROM historico_remocoes ORDER BY idRegistro DESC;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        erroAcesso = true;
        return lista;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int idReg = sqlite3_column_int(stmt, 0);
        const char* cData = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        int qtd = sqlite3_column_int(stmt, 2);
        const char* cMotivo = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        const char* cLote = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        const char* cNome = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

        lista.emplace_back(
            idReg,
            cData ? cData : "",
            qtd,
            cMotivo ? cMotivo : "",
            cLote ? cLote : "",
            cNome ? cNome : ""
        );
    }

    sqlite3_finalize(stmt);
    return lista;
}

// [UC004] Reservar item/lote (Figura 6 & Quadro 4, [RF007], [RF010], [RNF003])
bool Estoque::reservarItem(const DadosReserva& dados, std::string& msgErro) {
    if (!temConexao()) {
        msgErro = "Erro: Sem conexao com o banco de dados!";
        return false;
    }

    std::string idLote = trimStr(dados.idLote);
    std::string idItem = trimStr(dados.idItem);
    std::string qtdStr = trimStr(dados.quantidade);
    std::string dataRes = trimStr(dados.dataReserva);
    std::string localRes = trimStr(dados.localReserva);

    // [RN002] Todos os dados da reserva devem ser preenchidos
    if (dataRes.empty() || localRes.empty() ||
        (dados.modoLote && (idLote.empty() || qtdStr.empty())) ||
        (!dados.modoLote && idItem.empty())) {
        msgErro = "Erro [RN002]: Todos os dados da reserva devem ser preenchidos!";
        return false;
    }

    std::string dataISO;
    if (!converterEValidarData(dataRes, dataISO)) {
        msgErro = "Erro: Data da reserva invalida (use DD/MM/AAAA ou AAAA-MM-DD)!";
        return false;
    }

    std::string destinoFormatado = localRes + " (" + dataRes + ")";
    std::vector<int> idsParaReservar;

    if (dados.modoLote) {
        int qtd = 0;
        try {
            qtd = std::stoi(qtdStr);
        } catch (...) {
            qtd = 0;
        }
        if (qtd <= 0) {
            msgErro = "Erro: A quantidade para reserva deve ser maior que zero!";
            return false;
        }

        // Busca itens disponiveis (nao reservados) no lote especificado
        const char* sqlBusca =
            "SELECT id FROM itens_em_estoque "
            "WHERE lote = ? AND (reservado = 0 OR reservado IS NULL) AND (destino_reserva = '' OR destino_reserva IS NULL) "
            "ORDER BY id ASC LIMIT ?;";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sqlBusca, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, idLote.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 2, qtd);
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                idsParaReservar.push_back(sqlite3_column_int(stmt, 0));
            }
            sqlite3_finalize(stmt);
        }

        // [RN001] Deve existir pelo menos um item (ou a quantidade solicitada) para reservar
        if (static_cast<int>(idsParaReservar.size()) < qtd) {
            msgErro = "Erro [RN001]: Lote inexistente ou qtd de itens livres insuficiente (" +
                      std::to_string(idsParaReservar.size()) + " disponiveis)!";
            return false;
        }
    } else {
        int idNum = 0;
        try {
            idNum = std::stoi(idItem);
        } catch (...) {
            msgErro = "Erro: ID do item invalido!";
            return false;
        }

        const char* sqlBusca =
            "SELECT id, reservado, COALESCE(destino_reserva, '') FROM itens_em_estoque WHERE id = ?;";
        sqlite3_stmt* stmt = nullptr;
        bool encontrado = false;
        bool jaReservado = false;
        if (sqlite3_prepare_v2(db, sqlBusca, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, idNum);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                encontrado = true;
                int res = sqlite3_column_int(stmt, 1);
                const char* dest = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                jaReservado = (res != 0) || (dest && std::strlen(dest) > 0);
            }
            sqlite3_finalize(stmt);
        }

        if (!encontrado) {
            msgErro = "Erro [RN001]: Item com ID " + idItem + " nao encontrado no estoque!";
            return false;
        }
        if (jaReservado) {
            msgErro = "Erro: O item com ID " + idItem + " ja se encontra reservado!";
            return false;
        }
        idsParaReservar.push_back(idNum);
    }

    sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

    const char* sqlUpd =
        "UPDATE itens_em_estoque SET reservado = 1, destino_reserva = ?, data_reserva = ? WHERE id = ?;";
    sqlite3_stmt* stmtUpd = nullptr;
    if (sqlite3_prepare_v2(db, sqlUpd, -1, &stmtUpd, nullptr) != SQLITE_OK) {
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        msgErro = "Erro ao atualizar reserva no banco de dados!";
        return false;
    }

    for (int id : idsParaReservar) {
        sqlite3_bind_text(stmtUpd, 1, destinoFormatado.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmtUpd, 2, dataISO.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmtUpd, 3, id);
        sqlite3_step(stmtUpd);
        sqlite3_reset(stmtUpd);
    }
    sqlite3_finalize(stmtUpd);

    // Salva na tabela reservas (Classe Reserva - Quadro 10)
    Reserva novaReserva(0, localRes, dataISO, static_cast<int>(idsParaReservar.size()),
                        dados.modoLote ? ("Lote " + idLote) : ("Item " + idItem));
    novaReserva.confirmar();

    const char* sqlInsRes =
        "INSERT INTO reservas (cliente, dataDaFesta, quantidadeReservada, observacao) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmtRes = nullptr;
    if (sqlite3_prepare_v2(db, sqlInsRes, -1, &stmtRes, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmtRes, 1, novaReserva.getCliente().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmtRes, 2, novaReserva.getDataDaFesta().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmtRes, 3, novaReserva.getQuantidadeReservada());
        sqlite3_bind_text(stmtRes, 4, novaReserva.getObservacao().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(stmtRes);
        sqlite3_finalize(stmtRes);
    }

    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
    return true;
}

// [UC005] Verificar item/lote antes da confirmacao de remocao (Figura 7: 1.1.1 verificarItem)
bool Estoque::verificarItem(const DadosRemocao& dados, std::string& msgErro, std::string& resumoConfirmacao) {
    if (!temConexao()) {
        msgErro = "Erro [RN004]: Sem conexao com o banco de dados!";
        return false;
    }

    std::string idLote = trimStr(dados.idLote);
    std::string idItem = trimStr(dados.idItem);
    std::string qtdStr = trimStr(dados.quantidade);
    std::string motivo = trimStr(dados.motivo);

    // [RN002] Todos os dados da remocao devem ser preenchidos
    if (motivo.empty() ||
        (dados.modoLote && (idLote.empty() || qtdStr.empty())) ||
        (!dados.modoLote && idItem.empty())) {
        msgErro = "Erro [RN002]: Preencha todos os campos (ID/Quantidade e Motivo)!";
        return false;
    }

    if (dados.modoLote) {
        bool removerLoteInteiro = (qtdStr == "TODOS" || qtdStr == "todos" || qtdStr == "TUDO" || qtdStr == "tudo" || qtdStr == "*");
        int qtd = 0;
        if (!removerLoteInteiro) {
            try {
                qtd = std::stoi(qtdStr);
            } catch (...) {
                qtd = 0;
            }
            if (qtd <= 0) {
                msgErro = "Erro: Informe uma quantidade maior que zero ou 'TODOS'!";
                return false;
            }
        }

        // Suporta um ou mais lotes separados por virgula ([RF002] e [RF004])
        std::vector<std::string> lotes;
        std::stringstream ss(idLote);
        std::string itemLote;
        while (std::getline(ss, itemLote, ',')) {
            std::string t = trimStr(itemLote);
            if (!t.empty())
                lotes.push_back(t);
        }
        if (lotes.empty()) {
            msgErro = "Erro [RN002]: Informe pelo menos um ID de lote valido!";
            return false;
        }

        int totalDisponivel = 0;
        for (const auto& l : lotes) {
            const char* sqlCount = "SELECT COUNT(*) FROM itens_em_estoque WHERE lote = ?;";
            sqlite3_stmt* stmt = nullptr;
            int disponiveis = 0;
            if (sqlite3_prepare_v2(db, sqlCount, -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_text(stmt, 1, l.c_str(), -1, SQLITE_TRANSIENT);
                if (sqlite3_step(stmt) == SQLITE_ROW) {
                    disponiveis = sqlite3_column_int(stmt, 0);
                }
                sqlite3_finalize(stmt);
            }

            if (disponiveis == 0) {
                msgErro = "Erro [RN001]: Lote '" + l + "' nao encontrado no estoque!";
                return false;
            }
            if (!removerLoteInteiro && qtd > disponiveis) {
                msgErro = "Erro [RN001]: O lote '" + l + "' possui apenas " +
                          std::to_string(disponiveis) + " item(ns) no estoque!";
                return false;
            }
            totalDisponivel += disponiveis;
        }

        if (removerLoteInteiro) {
            resumoConfirmacao = "Confirmar remocao INTEGRAL do(s) lote(s) (" + idLote +
                                ") - Total: " + std::to_string(totalDisponivel) + " un.?";
        } else {
            resumoConfirmacao = "Confirmar remocao de " + std::to_string(qtd) +
                                " un. por lote (" + idLote + ")?";
        }
        return true;
    } else {
        int idNum = 0;
        try {
            idNum = std::stoi(idItem);
        } catch (...) {
            msgErro = "Erro: ID do item invalido!";
            return false;
        }

        const char* sqlBusca = "SELECT lote, nome FROM itens_em_estoque WHERE id = ?;";
        sqlite3_stmt* stmt = nullptr;
        bool achou = false;
        std::string loteItem, nomeItem;
        if (sqlite3_prepare_v2(db, sqlBusca, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, idNum);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                achou = true;
                const char* cl = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                const char* cn = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                if (cl) loteItem = cl;
                if (cn) nomeItem = cn;
            }
            sqlite3_finalize(stmt);
        }

        if (!achou) {
            msgErro = "Erro [RN001]: Item com ID " + idItem + " nao encontrado no estoque!";
            return false;
        }

        resumoConfirmacao = "Confirmar remocao do Item ID " + idItem +
                            " (" + nomeItem + ", Lote " + loteItem + ")?";
        return true;
    }
}

// [UC005] Executar remocao confirmada e registrar no historico (Figura 7: 1.2.1 removerItem)
bool Estoque::executarRemocao(const DadosRemocao& dados, std::string& msgErro) {
    std::string resumo;
    if (!verificarItem(dados, msgErro, resumo))
        return false;

    std::string idLote = trimStr(dados.idLote);
    std::string idItem = trimStr(dados.idItem);
    std::string motivo = trimStr(dados.motivo);

    struct GrupoRemocao {
        std::string lote;
        std::string nomeBebida;
        std::vector<int> ids;
    };
    std::vector<GrupoRemocao> grupos;

    if (dados.modoLote) {
        std::string qTrim = trimStr(dados.quantidade);
        bool removerLoteInteiro = (qTrim == "TODOS" || qTrim == "todos" || qTrim == "TUDO" || qTrim == "tudo" || qTrim == "*");
        int qtdRemover = removerLoteInteiro ? 1000000 : std::stoi(qTrim);
        std::stringstream ss(idLote);
        std::string itemLote;
        while (std::getline(ss, itemLote, ',')) {
            std::string l = trimStr(itemLote);
            if (l.empty()) continue;

            GrupoRemocao g;
            g.lote = l;
            const char* sqlSel = "SELECT id, nome FROM itens_em_estoque WHERE lote = ? ORDER BY id ASC LIMIT ?;";
            sqlite3_stmt* stmt = nullptr;
            if (sqlite3_prepare_v2(db, sqlSel, -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_text(stmt, 1, l.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_int(stmt, 2, qtdRemover);
                while (sqlite3_step(stmt) == SQLITE_ROW) {
                    g.ids.push_back(sqlite3_column_int(stmt, 0));
                    if (g.nomeBebida.empty()) {
                        const char* cn = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                        if (cn) g.nomeBebida = cn;
                    }
                }
                sqlite3_finalize(stmt);
            }
            if (!g.ids.empty())
                grupos.push_back(g);
        }
    } else {
        int idNum = std::stoi(idItem);
        const char* sqlSel = "SELECT lote, nome FROM itens_em_estoque WHERE id = ?;";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, sqlSel, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, idNum);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                GrupoRemocao g;
                const char* cl = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                const char* cn = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                if (cl) g.lote = cl;
                if (cn) g.nomeBebida = cn;
                g.ids.push_back(idNum);
                grupos.push_back(g);
            }
            sqlite3_finalize(stmt);
        }
    }

    if (grupos.empty()) {
        msgErro = "Erro: Nenhum item encontrado para remover!";
        return false;
    }

    sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

    const char* sqlDel = "DELETE FROM itens_em_estoque WHERE id = ?;";
    sqlite3_stmt* stmtDel = nullptr;
    if (sqlite3_prepare_v2(db, sqlDel, -1, &stmtDel, nullptr) != SQLITE_OK) {
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        msgErro = "Erro ao remover item do banco de dados!";
        return false;
    }

    const char* sqlUpdLote =
        "UPDATE lotes SET quantidadeAtual = MAX(0, quantidadeAtual - ?) WHERE idLote = ?;";

    for (const auto& g : grupos) {
        for (int id : g.ids) {
            sqlite3_bind_int(stmtDel, 1, id);
            sqlite3_step(stmtDel);
            sqlite3_reset(stmtDel);
        }

        sqlite3_stmt* stmtLote = nullptr;
        if (sqlite3_prepare_v2(db, sqlUpdLote, -1, &stmtLote, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stmtLote, 1, static_cast<int>(g.ids.size()));
            sqlite3_bind_text(stmtLote, 2, g.lote.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_step(stmtLote);
            sqlite3_finalize(stmtLote);
        }

        // [RF012] Registra no historico de remocoes (Quadro 9)
        HistoricodeRemocoes reg(
            0,
            obterDataAtual(),
            static_cast<int>(g.ids.size()),
            motivo,
            g.lote,
            g.nomeBebida
        );
        if (!reg.registrarRemocao(db)) {
            sqlite3_finalize(stmtDel);
            sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
            msgErro = "Erro [RN003]: Falha ao salvar no historico de remocoes!";
            return false;
        }
    }

    sqlite3_finalize(stmtDel);
    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
    return true;
}
