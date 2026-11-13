#include "JanelaRemover.h"
#include "SFMLCompat.h"
#include <cstdio>

JanelaRemover::JanelaRemover(Estoque* estoqueRef)
    : janela(WareSimUI::criarVideoMode(700u, 580u), "Remover Item / Lote"),
      estoque(estoqueRef ? estoqueRef : &estoqueLocal)
{
    if (!WareSimUI::carregarFonte(fonte, "arial.ttf"))
        janela.close();
}

void JanelaRemover::executar()
{
    bool telaCheia = false;
    while (janela.isOpen())
    {
        WareSimUI::EventoUI ev;
        while (WareSimUI::pollEvento(janela, ev))
        {
            if (ev.tipo == WareSimUI::TipoEvento::Fechar)
            {
                WareSimUI::tocarSomBotao();
                janela.close();
            }
            else if (ev.tipo == WareSimUI::TipoEvento::Redimensionar)
            {
                WareSimUI::aplicarViewProporcional(janela, 700.f, 580.f, ev.novaLargura, ev.novaAltura);
            }
            else if (ev.tipo == WareSimUI::TipoEvento::CliqueMouse && ev.botaoEsquerdo)
            {
                processarClique(ev.mouseX, ev.mouseY);
            }
            else if (ev.tipo == WareSimUI::TipoEvento::TextoDigitado)
            {
                processarTeclado(ev.unicode);
            }
            else if (ev.tipo == WareSimUI::TipoEvento::TeclaPressionada)
            {
                if (ev.teclaBackspace)
                    processarBackspace();
                else if (ev.teclaF11) {
                    WareSimUI::tocarSomBotao();
                    telaCheia = !telaCheia;
                    if (telaCheia) {
                        sf::VideoMode dm = sf::VideoMode::getDesktopMode();
                        janela.create(dm, "Remover Item / Lote", sf::Style::Fullscreen);
                        WareSimUI::aplicarViewProporcional(janela, 700.f, 580.f, dm.width, dm.height);
                    } else {
                        janela.create(WareSimUI::criarVideoMode(700u, 580u), "Remover Item / Lote", sf::Style::Default);
                        WareSimUI::aplicarViewProporcional(janela, 700.f, 580.f, 700u, 580u);
                    }
                }
                else if (ev.teclaEsc) {
                    WareSimUI::tocarSomBotao();
                    sf::sleep(sf::milliseconds(90));
                    janela.close();
                }
            }
        }

        if (janela.isOpen())
            renderizar();
    }
}

void JanelaRemover::processarClique(int x, int y)
{
    // Selecao entre Lote e Item.
    if (y >= 25 && y <= 70)
    {
        if (x >= 30 && x <= 180)
        {
            WareSimUI::tocarSomBotao();
            modoLote = true;
            confirmarRemocao = false;
            mensagemStatus.clear();
        }
        else if (x >= 200 && x <= 350)
        {
            WareSimUI::tocarSomBotao();
            modoLote = false;
            confirmarRemocao = false;
            mensagemStatus.clear();
        }

        // Limpa o foco ao trocar de modo.
        idLoteSelecionado = false;
        idItemSelecionado = false;
        quantidadeSelecionada = false;
        motivoSelecionado = false;

        return;
    }

    // Botao "Lote Inteiro" ([RF002]: permite remover lote(s) inteiro(s) de uma vez)
    if (modoLote &&
        x >= 470 && x <= 620 &&
        y >= 120 && y <= 162)
    {
        WareSimUI::tocarSomBotao();
        quantidade = "TODOS";
        confirmarRemocao = false;
        mensagemStatus.clear();
        return;
    }

    // Campos de identificacao e quantidade.
    bool antLote = idLoteSelecionado;
    bool antQtd = quantidadeSelecionada;
    bool antItem = idItemSelecionado;
    bool antMot = motivoSelecionado;

    idLoteSelecionado =
        modoLote &&
        x >= 30 && x <= 230 &&
        y >= 120 && y <= 162;

    quantidadeSelecionada =
        modoLote &&
        x >= 250 && x <= 450 &&
        y >= 120 && y <= 162;

    idItemSelecionado =
        !modoLote &&
        x >= 30 && x <= 330 &&
        y >= 120 && y <= 162;

    // Caixa do motivo.
    motivoSelecionado =
        x >= 30 && x <= 670 &&
        y >= 210 && y <= 400;

    if ((idLoteSelecionado && !antLote) ||
        (quantidadeSelecionada && !antQtd) ||
        (idItemSelecionado && !antItem) ||
        (motivoSelecionado && !antMot))
    {
        WareSimUI::tocarSomBotao();
    }

    // Cancelar ([UC005] Fluxo Alternativo: cancela a operacao de remocao)
    if (x >= 370 && x <= 520 &&
        y >= 505 && y <= 555)
    {
        WareSimUI::tocarSomBotao();
        sf::sleep(sf::milliseconds(90));
        janela.close();
        return;
    }

    // Remover / Confirmar Remocao ([UC005] & [RNF004])
    if (x >= 530 && x <= 680 &&
        y >= 505 && y <= 555)
    {
        auto toUtf8Str = [](const sf::String& s) -> std::string {
            auto u8 = s.toUtf8();
            return std::string(reinterpret_cast<const char*>(u8.data()), u8.size());
        };

        DadosRemocao dados;
        dados.modoLote = modoLote;
        dados.idLote = toUtf8Str(idLote);
        dados.idItem = toUtf8Str(idItem);
        dados.quantidade = toUtf8Str(quantidade);
        dados.motivo = toUtf8Str(motivo);

        if (!confirmarRemocao)
        {
            // Etapa 1: verificarItem(dados) -> se valido, pede confirmacao ([RNF004])
            std::string erro, resumo;
            if (estoque && estoque->verificarItem(dados, erro, resumo))
            {
                WareSimUI::tocarSomBotao();
                confirmarRemocao = true;
                statusErro = false;
                mensagemStatus = resumo + " Clique em 'Confirmar Remocao' para concluir.";
            }
            else
            {
                // [UC005] Fluxo de excecao: sinaliza erro ao tentar remover
                WareSimUI::tocarSomErro();
                confirmarRemocao = false;
                statusErro = true;
                mensagemStatus = erro.empty() ? "Erro ao validar dados de remocao!" : erro;
            }
        }
        else
        {
            // Etapa 2: confirmarRemocao() -> removerItem(dados) e salvar no historico ([RF012])
            std::string erro;
            if (estoque && estoque->executarRemocao(dados, erro))
            {
                WareSimUI::tocarSomSucesso();
                operacaoConcluida = true;
                mensagemStatus = "Mensagem de sucesso: Elemento removido e salvo no historico de remocoes!";
                std::printf("Elemento removido e salvo no historico de remocoes\n");
                sf::sleep(sf::milliseconds(110));
                janela.close();
            }
            else
            {
                WareSimUI::tocarSomErro();
                confirmarRemocao = false;
                statusErro = true;
                mensagemStatus = erro.empty() ? "Erro ao remover item do banco de dados!" : erro;
            }
        }

        return;
    }
}

void JanelaRemover::processarTeclado(char32_t caractere)
{
    if (caractere < 32 || caractere == 127)
    {
        if ((caractere == U'\n' || caractere == U'\r') && motivoSelecionado)
        {
            motivo += U'\n';
            confirmarRemocao = false;
        }
        return;
    }

    // ID do lote (aceita alfanumericos e virgula para multiplos lotes, ex: A, B ou 204).
    if (idLoteSelecionado && modoLote)
    {
        idLote += sf::String(caractere);
        confirmarRemocao = false;
        mensagemStatus.clear();
    }
    // Quantidade de itens do lote (numeros ou "TODOS" / "*" para lote inteiro).
    else if (quantidadeSelecionada && modoLote)
    {
        if ((caractere >= U'0' && caractere <= U'9') ||
            (caractere >= U'a' && caractere <= U'z') ||
            (caractere >= U'A' && caractere <= U'Z') ||
            caractere == U'*')
        {
            quantidade += sf::String(caractere);
            confirmarRemocao = false;
            mensagemStatus.clear();
        }
    }
    // ID do item (somente numeros).
    else if (idItemSelecionado && !modoLote)
    {
        if (caractere >= U'0' && caractere <= U'9')
        {
            idItem += sf::String(caractere);
            confirmarRemocao = false;
            mensagemStatus.clear();
        }
    }
    // Motivo da remocao.
    else if (motivoSelecionado)
    {
        motivo += sf::String(caractere);
        confirmarRemocao = false;
        mensagemStatus.clear();
    }
}

void JanelaRemover::processarBackspace()
{
    if (idLoteSelecionado && modoLote && !idLote.isEmpty())
    {
        idLote.erase(idLote.getSize() - 1, 1);
        confirmarRemocao = false;
    }
    else if (quantidadeSelecionada && modoLote && !quantidade.isEmpty())
    {
        quantidade.erase(quantidade.getSize() - 1, 1);
        confirmarRemocao = false;
    }
    else if (idItemSelecionado && !modoLote && !idItem.isEmpty())
    {
        idItem.erase(idItem.getSize() - 1, 1);
        confirmarRemocao = false;
    }
    else if (motivoSelecionado && !motivo.isEmpty())
    {
        motivo.erase(motivo.getSize() - 1, 1);
        confirmarRemocao = false;
    }
}

void JanelaRemover::renderizar()
{
    janela.clear(sf::Color(255, 227, 163));

    // Opcao Lote.
    sf::RectangleShape botaoLote({ 150.f, 45.f });
    botaoLote.setPosition({ 30.f, 25.f });
    botaoLote.setFillColor(
        modoLote ? sf::Color(235, 202, 157)
        : sf::Color(255, 222, 179)
    );
    botaoLote.setOutlineColor(sf::Color(222, 154, 64));
    botaoLote.setOutlineThickness(2.f);
    janela.draw(botaoLote);

    sf::Text textoLote = WareSimUI::criarTexto(fonte, "Lote", 18);
    textoLote.setFillColor(sf::Color::Black);
    textoLote.setPosition({ 78.f, 36.f });
    janela.draw(textoLote);

    // Opcao Item.
    sf::RectangleShape botaoItem({ 150.f, 45.f });
    botaoItem.setPosition({ 200.f, 25.f });
    botaoItem.setFillColor(
        !modoLote ? sf::Color(235, 202, 157)
        : sf::Color(255, 222, 179)
    );
    botaoItem.setOutlineColor(sf::Color(222, 154, 64));
    botaoItem.setOutlineThickness(2.f);
    janela.draw(botaoItem);

    sf::Text textoItem = WareSimUI::criarTexto(fonte, "Item", 18);
    textoItem.setFillColor(sf::Color::Black);
    textoItem.setPosition({ 250.f, 36.f });
    janela.draw(textoItem);

    if (modoLote)
    {
        // Rotulo ID Lote.
        sf::Text rotuloIDLote = WareSimUI::criarTexto(fonte, "ID Lote (ex: A ou A,B)", 16);
        rotuloIDLote.setFillColor(sf::Color::Black);
        rotuloIDLote.setPosition({ 30.f, 88.f });
        janela.draw(rotuloIDLote);

        // Campo ID Lote.
        sf::RectangleShape campoIDLote({ 200.f, 42.f });
        campoIDLote.setPosition({ 30.f, 120.f });
        campoIDLote.setFillColor(sf::Color(255, 245, 225));
        campoIDLote.setOutlineColor(idLoteSelecionado ? sf::Color(80, 120, 200) : sf::Color(222, 154, 64));
        campoIDLote.setOutlineThickness(idLoteSelecionado ? 2.5f : 2.f);
        janela.draw(campoIDLote);

        unsigned int tamFonteLote = 16u;
        sf::String strLoteAjust = WareSimUI::ajustarTextoCaixa(fonte, idLote, 16u, 200.f, 42.f, tamFonteLote);
        sf::Text textoIDLote = WareSimUI::criarTexto(fonte, strLoteAjust, tamFonteLote);
        textoIDLote.setFillColor(sf::Color::Black);
        textoIDLote.setPosition({ 38.f, 125.f });
        janela.draw(textoIDLote);

        // Rotulo da quantidade.
        sf::Text rotuloQuantidade = WareSimUI::criarTexto(fonte, "Qtd Itens", 16);
        rotuloQuantidade.setFillColor(sf::Color::Black);
        rotuloQuantidade.setPosition({ 250.f, 88.f });
        janela.draw(rotuloQuantidade);

        // Campo da quantidade.
        sf::RectangleShape campoQuantidade({ 200.f, 42.f });
        campoQuantidade.setPosition({ 250.f, 120.f });
        campoQuantidade.setFillColor(sf::Color(255, 245, 225));
        campoQuantidade.setOutlineColor(quantidadeSelecionada ? sf::Color(80, 120, 200) : sf::Color(222, 154, 64));
        campoQuantidade.setOutlineThickness(quantidadeSelecionada ? 2.5f : 2.f);
        janela.draw(campoQuantidade);

        unsigned int tamFonteQtd = 16u;
        sf::String strQtdAjust = WareSimUI::ajustarTextoCaixa(fonte, quantidade, 16u, 200.f, 42.f, tamFonteQtd);
        sf::Text textoQuantidade = WareSimUI::criarTexto(fonte, strQtdAjust, tamFonteQtd);
        textoQuantidade.setFillColor(sf::Color::Black);
        textoQuantidade.setPosition({ 258.f, 125.f });
        janela.draw(textoQuantidade);

        // Botao atalho "Lote Inteiro" ([RF002])
        sf::RectangleShape botaoLoteInteiro({ 150.f, 42.f });
        botaoLoteInteiro.setPosition({ 470.f, 120.f });
        botaoLoteInteiro.setFillColor(sf::Color(245, 210, 160));
        botaoLoteInteiro.setOutlineColor(sf::Color(222, 154, 64));
        botaoLoteInteiro.setOutlineThickness(2.f);
        janela.draw(botaoLoteInteiro);

        sf::Text txtLoteInteiro = WareSimUI::criarTexto(fonte, "Lote Inteiro", 16);
        txtLoteInteiro.setFillColor(sf::Color::Black);
        txtLoteInteiro.setPosition({ 498.f, 130.f });
        janela.draw(txtLoteInteiro);
    }
    else
    {
        // Rotulo ID Item.
        sf::Text rotuloIDItem = WareSimUI::criarTexto(fonte, "ID Item (Id bebida)", 16);
        rotuloIDItem.setFillColor(sf::Color::Black);
        rotuloIDItem.setPosition({ 30.f, 88.f });
        janela.draw(rotuloIDItem);

        // Campo ID Item.
        sf::RectangleShape campoIDItem({ 300.f, 42.f });
        campoIDItem.setPosition({ 30.f, 120.f });
        campoIDItem.setFillColor(sf::Color(255, 245, 225));
        campoIDItem.setOutlineColor(idItemSelecionado ? sf::Color(80, 120, 200) : sf::Color(222, 154, 64));
        campoIDItem.setOutlineThickness(idItemSelecionado ? 2.5f : 2.f);
        janela.draw(campoIDItem);

        unsigned int tamFonteItem = 16u;
        sf::String strItemAjust = WareSimUI::ajustarTextoCaixa(fonte, idItem, 16u, 300.f, 42.f, tamFonteItem);
        sf::Text textoIDItem = WareSimUI::criarTexto(fonte, strItemAjust, tamFonteItem);
        textoIDItem.setFillColor(sf::Color::Black);
        textoIDItem.setPosition({ 38.f, 125.f });
        janela.draw(textoIDItem);
    }

    // Rotulo do motivo.
    sf::Text rotuloMotivo = WareSimUI::criarTexto(fonte, "Motivo da remocao", 16);
    rotuloMotivo.setFillColor(sf::Color::Black);
    rotuloMotivo.setPosition({ 30.f, 178.f });
    janela.draw(rotuloMotivo);

    // Caixa do motivo com quebra de linha automatica quando exceder o limite horizontal
    sf::RectangleShape campoMotivo({ 640.f, 190.f });
    campoMotivo.setPosition({ 30.f, 210.f });
    campoMotivo.setFillColor(sf::Color(255, 245, 225));
    campoMotivo.setOutlineColor(motivoSelecionado ? sf::Color(80, 120, 200) : sf::Color(222, 154, 64));
    campoMotivo.setOutlineThickness(motivoSelecionado ? 2.5f : 2.f);
    janela.draw(campoMotivo);

    unsigned int tamFonteMotivo = 16u;
    sf::String motivoQuebrado = WareSimUI::ajustarTextoCaixa(fonte, motivo, 16u, 640.f, 190.f, tamFonteMotivo);
    sf::Text textoMotivo = WareSimUI::criarTexto(fonte, motivoQuebrado, tamFonteMotivo);
    textoMotivo.setFillColor(sf::Color::Black);
    textoMotivo.setPosition({ 38.f, 218.f });
    janela.draw(textoMotivo);

    // Caixa de confirmacao ([RNF004]) ou mensagem de erro ([UC005] Fluxo de Excecao) com quebra de linha
    if (!mensagemStatus.empty())
    {
        sf::String msgUtf8 = sf::String::fromUtf8(mensagemStatus.begin(), mensagemStatus.end());
        int numLinhas = 1;
        sf::String msgQuebrada = WareSimUI::quebrarTexto(fonte, msgUtf8, 13u, 620.f, &numLinhas);
        float alturaBarra = std::max(68.f, 28.f + numLinhas * 17.f + 10.f);

        sf::RectangleShape barraMsg({ 640.f, alturaBarra });
        barraMsg.setPosition({ 30.f, 412.f });
        if (confirmarRemocao)
        {
            barraMsg.setFillColor(sf::Color(255, 238, 190));
            barraMsg.setOutlineColor(sf::Color(210, 110, 20));
        }
        else if (statusErro)
        {
            barraMsg.setFillColor(sf::Color(255, 210, 210));
            barraMsg.setOutlineColor(sf::Color(190, 40, 40));
        }
        else
        {
            barraMsg.setFillColor(sf::Color(210, 245, 210));
            barraMsg.setOutlineColor(sf::Color(40, 150, 40));
        }
        barraMsg.setOutlineThickness(2.f);
        janela.draw(barraMsg);

        sf::Text tituloMsg = WareSimUI::criarTexto(
            fonte,
            confirmarRemocao ? "TELA DE CONFIRMACAO DE REMOCAO [RNF004]:" : "AVISO DO SISTEMA:",
            14
        );
        tituloMsg.setStyle(sf::Text::Bold);
        tituloMsg.setFillColor(statusErro ? sf::Color(160, 0, 0) : sf::Color(140, 60, 0));
        tituloMsg.setPosition({ 40.f, 418.f });
        janela.draw(tituloMsg);

        sf::Text txtMsg = WareSimUI::criarTexto(fonte, msgQuebrada, 13);
        txtMsg.setFillColor(statusErro ? sf::Color(160, 0, 0) : sf::Color::Black);
        txtMsg.setPosition({ 40.f, 440.f });
        janela.draw(txtMsg);
    }

    // Botao Cancelar.
    sf::RectangleShape botaoCancelar({ 150.f, 50.f });
    botaoCancelar.setPosition({ 370.f, 505.f });
    botaoCancelar.setFillColor(sf::Color(255, 222, 179));
    botaoCancelar.setOutlineColor(sf::Color(222, 154, 64));
    botaoCancelar.setOutlineThickness(2.f);
    janela.draw(botaoCancelar);

    sf::Text textoCancelar = WareSimUI::criarTexto(fonte, "Cancelar", 18);
    textoCancelar.setFillColor(sf::Color::Black);
    textoCancelar.setPosition({ 407.f, 518.f });
    janela.draw(textoCancelar);

    // Botao Remover / Confirmar Remocao.
    sf::RectangleShape botaoRemover({ 150.f, 50.f });
    botaoRemover.setPosition({ 530.f, 505.f });
    botaoRemover.setFillColor(
        confirmarRemocao ? sf::Color(255, 190, 170)
        : sf::Color(255, 222, 179)
    );
    botaoRemover.setOutlineColor(sf::Color(222, 154, 64));
    botaoRemover.setOutlineThickness(2.f);
    janela.draw(botaoRemover);

    sf::Text textoRemover = WareSimUI::criarTexto(
        fonte,
        confirmarRemocao ? "Confirmar Remocao" : "Remover",
        15
    );

    textoRemover.setFillColor(sf::Color::Black);
    textoRemover.setPosition(
        confirmarRemocao
        ? sf::Vector2f(538.f, 520.f)
        : sf::Vector2f(570.f, 518.f)
    );
    janela.draw(textoRemover);

    janela.display();
}
