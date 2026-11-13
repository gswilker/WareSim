#include "JanelaInserir.h"
#include "SFMLCompat.h"

JanelaInserir::JanelaInserir(Estoque* estoqueRef)
    : janela(WareSimUI::criarVideoMode(760u, 700u), "Inserir Item / Lote"),
      estoque(estoqueRef ? estoqueRef : &estoqueLocal)
{
    if (!WareSimUI::carregarFonte(fonte, "arial.ttf"))
    {
        janela.close();
        return;
    }

    // Coluna esquerda
    const float xEsquerda = 30.f;
    const float larguraCampo = 330.f;

    // Coluna direita
    const float xDireita = 400.f;

    const float yInicial = 136.f;
    const float distancia = 84.f;
    const float alturaCampo = 42.f;

    campos = {
        { "Id do lote",                         "", xEsquerda, yInicial,                 larguraCampo, alturaCampo, true },
        { "Nome bebida",                        "", xEsquerda, yInicial + distancia,     larguraCampo, alturaCampo, true },
        { "Id bebida (opcional - auto)",        "", xEsquerda, yInicial + distancia * 2, larguraCampo, alturaCampo, true },
        { "Volume (ml)",                        "", xEsquerda, yInicial + distancia * 3, larguraCampo, alturaCampo, true },
        { "Categoria",                          "", xEsquerda, yInicial + distancia * 4, larguraCampo, alturaCampo, true },

        { "Distribuidor",                       "", xDireita,  yInicial,                 larguraCampo, alturaCampo, true },
        { "Preco",                              "", xDireita,  yInicial + distancia,     larguraCampo, alturaCampo, true },
        { "Destino Reserva (opcional)",         "", xDireita,  yInicial + distancia * 2, larguraCampo, alturaCampo, true },
        { "Data Validade (AAAA-MM-DD)",         "", xDireita,  yInicial + distancia * 3, larguraCampo, alturaCampo, true },
        { "Qtd itens no lote",                  "", xDireita,  yInicial + distancia * 4, larguraCampo, alturaCampo, true }
    };

    campos[9].habilitado = true;
}

void JanelaInserir::executar()
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
                WareSimUI::aplicarViewProporcional(janela, 760.f, 700.f, ev.novaLargura, ev.novaAltura);
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
                else if (ev.teclaTab) {
                    WareSimUI::tocarSomBotao();
                    avancarCampo();
                }
                else if (ev.teclaF11) {
                    WareSimUI::tocarSomBotao();
                    telaCheia = !telaCheia;
                    if (telaCheia) {
                        sf::VideoMode dm = sf::VideoMode::getDesktopMode();
                        janela.create(dm, "Inserir Item / Lote", sf::Style::Fullscreen);
                        WareSimUI::aplicarViewProporcional(janela, 760.f, 700.f, dm.width, dm.height);
                    } else {
                        janela.create(WareSimUI::criarVideoMode(760u, 700u), "Inserir Item / Lote", sf::Style::Default);
                        WareSimUI::aplicarViewProporcional(janela, 760.f, 700.f, 760u, 700u);
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

void JanelaInserir::avancarCampo()
{
    if (campos.empty())
        return;

    int prox = (campoSelecionado + 1) % static_cast<int>(campos.size());
    for (size_t i = 0; i < campos.size(); ++i)
    {
        if (campos[prox].habilitado)
        {
            campoSelecionado = prox;
            return;
        }
        prox = (prox + 1) % static_cast<int>(campos.size());
    }
}

void JanelaInserir::processarClique(int x, int y)
{
    // Selecao entre Lote e Item.
    if (y >= 48 && y <= 88)
    {
        if (x >= 30 && x <= 180)
        {
            WareSimUI::tocarSomBotao();
            modoLote = true;
            campos[0].rotulo = "Id do lote";
            campos[9].habilitado = true;
            mensagemStatus.clear();
            return;
        }
        else if (x >= 200 && x <= 350)
        {
            WareSimUI::tocarSomBotao();
            modoLote = false;
            campos[0].rotulo = "Id do lote (opcional se avulso)";
            campos[9].habilitado = false;
            if (campoSelecionado == 9)
                campoSelecionado = -1;
            mensagemStatus.clear();
            return;
        }
    }

    // Botao Inserir ([UC001] Fluxo Basico e Fluxo de Excecao)
    if (y >= 630 && y <= 675 && x >= 400 && x <= 530)
    {
        auto toUtf8Str = [](const sf::String& s) -> std::string {
            auto u8 = s.toUtf8();
            return std::string(reinterpret_cast<const char*>(u8.data()), u8.size());
        };

        DadosInsercao dados;
        dados.lote = toUtf8Str(campos[0].valor);
        dados.nome = toUtf8Str(campos[1].valor);
        dados.idBebida = toUtf8Str(campos[2].valor);
        dados.volume = toUtf8Str(campos[3].valor);
        dados.categoria = toUtf8Str(campos[4].valor);
        dados.distribuidor = toUtf8Str(campos[5].valor);
        dados.preco = toUtf8Str(campos[6].valor);
        dados.destinoOuDataReserva = toUtf8Str(campos[7].valor);
        dados.dataValidade = toUtf8Str(campos[8].valor);
        dados.quantidade = modoLote ? toUtf8Str(campos[9].valor) : "1";
        dados.modoLote = modoLote;

        std::string erro;
        if (estoque && estoque->salvarItem(dados, erro))
        {
            WareSimUI::tocarSomSucesso();
            statusErro = false;
            operacaoConcluida = true;
            mensagemStatus = "Mensagem de sucesso: Item/Lote inserido no estoque com sucesso!";
            sf::sleep(sf::milliseconds(110));
            janela.close();
        }
        else
        {
            // [UC001] Fluxo de excecao: sinaliza a falta de informacoes necessarias
            WareSimUI::tocarSomErro();
            statusErro = true;
            mensagemStatus = erro.empty() ? "Erro: Dados invalidos ou incompletos!" : erro;
        }
        return;
    }

    // Botao Cancelar ([UC001] Fluxo Alternativo: O sistema nao faz nada)
    if (y >= 630 && y <= 675 && x >= 550 && x <= 680)
    {
        WareSimUI::tocarSomBotao();
        sf::sleep(sf::milliseconds(90));
        janela.close();
        return;
    }

    // Selecao de um campo de texto.
    campoSelecionado = -1;

    for (int i = 0; i < static_cast<int>(campos.size()); ++i)
    {
        const auto& campo = campos[i];

        if (campo.habilitado &&
            x >= campo.x &&
            x <= campo.x + campo.largura &&
            y >= campo.y &&
            y <= campo.y + campo.altura)
        {
            WareSimUI::tocarSomBotao();
            campoSelecionado = i;
            break;
        }
    }
}

void JanelaInserir::processarBackspace()
{
    if (campoSelecionado < 0 ||
        campoSelecionado >= static_cast<int>(campos.size()))
    {
        return;
    }

    auto& campo = campos[campoSelecionado];
    if (campo.habilitado && !campo.valor.isEmpty())
    {
        campo.valor.erase(campo.valor.getSize() - 1, 1);
    }
}

void JanelaInserir::processarTeclado(char32_t caractere)
{
    if (campoSelecionado < 0 ||
        campoSelecionado >= static_cast<int>(campos.size()))
    {
        return;
    }

    auto& campo = campos[campoSelecionado];

    if (!campo.habilitado)
        return;

    // Ignora caracteres de controle (Backspace ja tratado em processarBackspace)
    if (caractere >= 32 && caractere != 127)
    {
        campo.valor += sf::String(caractere);
        mensagemStatus.clear();
    }
}

void JanelaInserir::renderizar()
{
    janela.clear(sf::Color(255, 227, 163));

    // Opcoes Lote e Item.
    sf::RectangleShape botaoLote({ 150.f, 40.f });
    botaoLote.setPosition({ 30.f, 48.f });
    botaoLote.setFillColor(
        modoLote ? sf::Color(235, 202, 157)
        : sf::Color(255, 222, 179));
    botaoLote.setOutlineColor(sf::Color(222, 154, 64));
    botaoLote.setOutlineThickness(2.f);
    janela.draw(botaoLote);

    sf::Text textoLote = WareSimUI::criarTexto(fonte, "Lote", 18);
    textoLote.setFillColor(sf::Color::Black);
    textoLote.setPosition({ 80.f, 57.f });
    janela.draw(textoLote);

    sf::RectangleShape botaoItem({ 150.f, 40.f });
    botaoItem.setPosition({ 200.f, 48.f });
    botaoItem.setFillColor(
        !modoLote ? sf::Color(235, 202, 157)
        : sf::Color(255, 222, 179));
    botaoItem.setOutlineColor(sf::Color(222, 154, 64));
    botaoItem.setOutlineThickness(2.f);
    janela.draw(botaoItem);

    sf::Text textoItem = WareSimUI::criarTexto(fonte, "Item", 18);
    textoItem.setFillColor(sf::Color::Black);
    textoItem.setPosition({ 250.f, 57.f });
    janela.draw(textoItem);

    sf::Text dicaModo = WareSimUI::criarTexto(
        fonte,
        modoLote
            ? "Modo Lote: insere multiplos itens em um lote."
            : "Modo Item: insere 1 item individual (Id do lote opcional).",
        14
    );
    dicaModo.setFillColor(sf::Color(80, 50, 10));
    dicaModo.setPosition({ 370.f, 60.f });
    janela.draw(dicaModo);

    // Campos de preenchimento.
    for (int i = 0; i < static_cast<int>(campos.size()); ++i)
    {
        const auto& campo = campos[i];

        sf::Text rotulo = WareSimUI::criarTexto(fonte, campo.rotulo, 15);
        rotulo.setFillColor(sf::Color::Black);
        rotulo.setPosition({ campo.x, campo.y - 23.f });
        janela.draw(rotulo);

        sf::RectangleShape caixa({
            campo.largura,
            campo.altura
            });

        caixa.setPosition({ campo.x, campo.y });

        if (!campo.habilitado)
            caixa.setFillColor(sf::Color(190, 190, 190));
        else
            caixa.setFillColor(sf::Color(255, 245, 225));

        caixa.setOutlineColor(
            campoSelecionado == i
            ? sf::Color(80, 120, 200)
            : sf::Color(222, 154, 64));

        caixa.setOutlineThickness(campoSelecionado == i ? 2.5f : 2.f);
        janela.draw(caixa);

        // Quebra de linha automatica quando o texto exceder o limite horizontal da caixa
        unsigned int tamFonteCampo = 15u;
        sf::String textoAjustado = WareSimUI::ajustarTextoCaixa(
            fonte, campo.valor, 15u, campo.largura, campo.altura, tamFonteCampo
        );
        sf::Text valor = WareSimUI::criarTexto(fonte, textoAjustado, tamFonteCampo);
        valor.setFillColor(sf::Color::Black);
        valor.setPosition({ campo.x + 8.f, campo.y + 4.f });
        janela.draw(valor);
    }

    // Mensagem de erro / validacao ([UC001] Fluxo de Excecao) com quebra de linha automatica
    if (!mensagemStatus.empty())
    {
        int numLinhasMsg = 1;
        sf::String msgUtf8 = sf::String::fromUtf8(mensagemStatus.begin(), mensagemStatus.end());
        sf::String msgQuebrada = WareSimUI::quebrarTexto(fonte, msgUtf8, 14u, 680.f, &numLinhasMsg);
        float alturaBarra = std::max(40.f, numLinhasMsg * 18.f + 16.f);

        sf::RectangleShape barraMsg({ 700.f, alturaBarra });
        barraMsg.setPosition({ 30.f, 570.f });
        barraMsg.setFillColor(statusErro ? sf::Color(255, 210, 210) : sf::Color(210, 245, 210));
        barraMsg.setOutlineColor(statusErro ? sf::Color(190, 40, 40) : sf::Color(40, 150, 40));
        barraMsg.setOutlineThickness(2.f);
        janela.draw(barraMsg);

        sf::Text txtMsg = WareSimUI::criarTexto(fonte, msgQuebrada, 14);
        txtMsg.setFillColor(statusErro ? sf::Color(160, 0, 0) : sf::Color(0, 110, 0));
        txtMsg.setPosition({ 40.f, 578.f });
        janela.draw(txtMsg);
    }

    // Botao Inserir.
    sf::RectangleShape botaoInserir({ 130.f, 45.f });
    botaoInserir.setPosition({ 400.f, 630.f });
    botaoInserir.setFillColor(sf::Color(255, 222, 179));
    botaoInserir.setOutlineColor(sf::Color(222, 154, 64));
    botaoInserir.setOutlineThickness(2.f);
    janela.draw(botaoInserir);

    sf::Text textoInserir = WareSimUI::criarTexto(fonte, "Inserir", 18);
    textoInserir.setFillColor(sf::Color::Black);
    textoInserir.setPosition({ 435.f, 641.f });
    janela.draw(textoInserir);

    // Botao Cancelar.
    sf::RectangleShape botaoCancelar({ 130.f, 45.f });
    botaoCancelar.setPosition({ 550.f, 630.f });
    botaoCancelar.setFillColor(sf::Color(255, 222, 179));
    botaoCancelar.setOutlineColor(sf::Color(222, 154, 64));
    botaoCancelar.setOutlineThickness(2.f);
    janela.draw(botaoCancelar);

    sf::Text textoCancelar = WareSimUI::criarTexto(fonte, "Cancelar", 18);
    textoCancelar.setFillColor(sf::Color::Black);
    textoCancelar.setPosition({ 575.f, 641.f });
    janela.draw(textoCancelar);

    janela.display();
}
