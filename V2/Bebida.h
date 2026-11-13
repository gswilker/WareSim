#pragma once

#include <string>

// Figura 8: Diagrama de Classes & Quadro 6: Dicionario de informacoes da classe Bebida
class Bebida {
private:
    int id;
    std::string nome;
    int volume;
    float preco;
    std::string distribuidor;
    bool flagValidade;
    bool flagReserva;
    std::string dataEntrada;
    std::string categoria;

    // Informacoes complementares de lote, validade e destino de reserva ([RNF002], [RNF003])
    std::string lote;
    std::string dataValidade;
    std::string destinoReserva;
    std::string dataReserva;
    int statusValidade; // 0 = OK, 1 = Vence em <= 1 mes ([RNF002]), 2 = Vencido ([RF009])

public:
    Bebida();
    Bebida(
        int id,
        const std::string& lote,
        const std::string& nome,
        int volume,
        const std::string& categoria,
        const std::string& distribuidor,
        float preco,
        const std::string& dataValidade,
        bool flagReserva = false,
        const std::string& destinoReserva = "",
        const std::string& dataReserva = "",
        const std::string& dataEntrada = "",
        bool flagValidade = false,
        int statusValidade = 0
    );

    // Metodos do Diagrama de Classes (Figura 8)
    std::string getInfo() const;
    bool isValido() const;
    bool isReservado() const;

    // Getters e Setters
    int getId() const { return id; }
    void setId(int valor) { id = valor; }

    const std::string& getLote() const { return lote; }
    void setLote(const std::string& valor) { lote = valor; }

    const std::string& getNome() const { return nome; }
    void setNome(const std::string& valor) { nome = valor; }

    int getVolume() const { return volume; }
    void setVolume(int valor) { volume = valor; }

    float getPreco() const { return preco; }
    void setPreco(float valor) { preco = valor; }

    const std::string& getDistribuidor() const { return distribuidor; }
    void setDistribuidor(const std::string& valor) { distribuidor = valor; }

    bool getFlagValidade() const { return flagValidade; }
    void setFlagValidade(bool valor) { flagValidade = valor; }

    bool getFlagReserva() const { return flagReserva; }
    void setFlagReserva(bool valor) { flagReserva = valor; }

    const std::string& getDataEntrada() const { return dataEntrada; }
    void setDataEntrada(const std::string& valor) { dataEntrada = valor; }

    const std::string& getCategoria() const { return categoria; }
    void setCategoria(const std::string& valor) { categoria = valor; }

    const std::string& getDataValidade() const { return dataValidade; }
    void setDataValidade(const std::string& valor) { dataValidade = valor; }

    const std::string& getDestinoReserva() const { return destinoReserva; }
    void setDestinoReserva(const std::string& valor) {
        destinoReserva = valor;
        flagReserva = !destinoReserva.empty();
    }

    const std::string& getDataReserva() const { return dataReserva; }
    void setDataReserva(const std::string& valor) { dataReserva = valor; }

    int getStatusValidade() const { return statusValidade; }
    void setStatusValidade(int valor) {
        statusValidade = valor;
        flagValidade = (valor > 0);
    }
};
