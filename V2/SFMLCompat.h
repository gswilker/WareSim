#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include <vector>
#include <cmath>
#include <cstdint>
#include <algorithm>

namespace WareSimUI {

inline sf::VideoMode criarVideoMode(unsigned int largura, unsigned int altura) {
#if SFML_VERSION_MAJOR >= 3
    return sf::VideoMode({ largura, altura });
#else
    return sf::VideoMode(largura, altura);
#endif
}

inline bool carregarFonte(sf::Font& fonte, const std::string& arquivo = "arial.ttf") {
    const std::string caminhos[] = {
        arquivo,
        "../" + arquivo,
        "../../" + arquivo,
        "C:/Windows/Fonts/arial.ttf",
        "/usr/share/fonts/truetype/msttcorefonts/Arial.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
    };

    for (const auto& caminho : caminhos) {
#if SFML_VERSION_MAJOR >= 3
        if (fonte.openFromFile(caminho))
            return true;
#else
        if (fonte.loadFromFile(caminho))
            return true;
#endif
    }
    return false;
}

inline bool fonteCarregada(const sf::Font& fonte) {
    return !fonte.getInfo().family.empty();
}

inline sf::Text criarTexto(const sf::Font& fonte, const sf::String& conteudo, unsigned int tamanho = 17) {
#if SFML_VERSION_MAJOR >= 3
    return sf::Text(fonte, conteudo, tamanho);
#else
    return sf::Text(conteudo, fonte, tamanho);
#endif
}

inline float medirLarguraTexto(const sf::Font& fonte, const sf::String& str, unsigned int tamanhoFonte, bool negrito = false) {
    if (str.isEmpty())
        return 0.f;
    sf::Text txt = criarTexto(fonte, str, tamanhoFonte);
    if (negrito)
        txt.setStyle(sf::Text::Bold);
    sf::FloatRect bounds = txt.getLocalBounds();
#if SFML_VERSION_MAJOR >= 3
    return bounds.position.x + bounds.size.x;
#else
    return bounds.left + bounds.width;
#endif
}

// Quebra automaticamente o texto para a proxima linha quando exceder larguraMax
inline sf::String quebrarTexto(
    const sf::Font& fonte,
    const sf::String& textoOriginal,
    unsigned int tamanhoFonte,
    float larguraMax,
    int* outNumLinhas = nullptr,
    bool negrito = false
) {
    if (larguraMax <= 10.f || textoOriginal.isEmpty() || !fonteCarregada(fonte)) {
        if (outNumLinhas)
            *outNumLinhas = 1;
        return textoOriginal;
    }

    std::vector<sf::String> linhas;
    sf::String linhaAtual;
    int posUltimoEspaco = -1;

    for (std::size_t i = 0; i < textoOriginal.getSize(); ++i) {
        char32_t c = textoOriginal[i];
        if (c == U'\r')
            continue;
        if (c == U'\n') {
            linhas.push_back(linhaAtual);
            linhaAtual.clear();
            posUltimoEspaco = -1;
            continue;
        }

        sf::String tentativa = linhaAtual + c;
        float larg = medirLarguraTexto(fonte, tentativa, tamanhoFonte, negrito);

        if (larg > larguraMax && !linhaAtual.isEmpty()) {
            if (posUltimoEspaco > 0) {
                sf::String parte1 = linhaAtual.substring(0, static_cast<std::size_t>(posUltimoEspaco));
                sf::String resto = linhaAtual.substring(static_cast<std::size_t>(posUltimoEspaco + 1));
                linhas.push_back(parte1);
                linhaAtual = resto;
                if (c != U' ')
                    linhaAtual += c;
                posUltimoEspaco = -1;
                for (std::size_t k = 0; k < linhaAtual.getSize(); ++k) {
                    if (linhaAtual[k] == U' ')
                        posUltimoEspaco = static_cast<int>(k);
                }
            } else {
                linhas.push_back(linhaAtual);
                linhaAtual.clear();
                if (c != U' ')
                    linhaAtual += c;
                posUltimoEspaco = -1;
            }
        } else {
            if (c == U' ')
                posUltimoEspaco = static_cast<int>(linhaAtual.getSize());
            linhaAtual += c;
        }
    }

    linhas.push_back(linhaAtual);

    if (outNumLinhas)
        *outNumLinhas = std::max(1, static_cast<int>(linhas.size()));

    sf::String resultado;
    for (std::size_t i = 0; i < linhas.size(); ++i) {
        if (i > 0)
            resultado += U'\n';
        resultado += linhas[i];
    }
    return resultado;
}

// Formata texto para caber dentro de uma caixa de entrada (com quebra de linha e rolagem vertical se exceder a altura)
inline sf::String ajustarTextoCaixa(
    const sf::Font& fonte,
    const sf::String& textoOriginal,
    unsigned int tamanhoBase,
    float larguraCaixa,
    float alturaCaixa,
    unsigned int& outTamanhoFonte
) {
    outTamanhoFonte = tamanhoBase;
    if (textoOriginal.isEmpty() || !fonteCarregada(fonte))
        return textoOriginal;

    float largUtil = std::max(20.f, larguraCaixa - 16.f);
    int numLinhas = 1;
    sf::String quebrado = quebrarTexto(fonte, textoOriginal, outTamanhoFonte, largUtil, &numLinhas);

    // Se quebrou em mais de 1 linha numa caixa baixa, reduz levemente a fonte para caber melhor
    if (numLinhas > 1 && alturaCaixa < 60.f && tamanhoBase > 13u) {
        outTamanhoFonte = 13u;
        quebrado = quebrarTexto(fonte, textoOriginal, outTamanhoFonte, largUtil, &numLinhas);
    }

    float alturaLinha = static_cast<float>(outTamanhoFonte) + 4.f;
    int maxLinhasVisiveis = std::max(1, static_cast<int>((alturaCaixa - 8.f) / alturaLinha));

    if (numLinhas <= maxLinhasVisiveis)
        return quebrado;

    // Mantem as ultimas maxLinhasVisiveis linhas visiveis enquanto o usuario digita
    std::vector<sf::String> todasLinhas;
    sf::String atual;
    for (std::size_t i = 0; i < quebrado.getSize(); ++i) {
        if (quebrado[i] == U'\n') {
            todasLinhas.push_back(atual);
            atual.clear();
        } else {
            atual += quebrado[i];
        }
    }
    todasLinhas.push_back(atual);

    sf::String visivel;
    std::size_t inicio = todasLinhas.size() > static_cast<std::size_t>(maxLinhasVisiveis)
        ? todasLinhas.size() - static_cast<std::size_t>(maxLinhasVisiveis)
        : 0;
    for (std::size_t i = inicio; i < todasLinhas.size(); ++i) {
        if (i > inicio)
            visivel += U'\n';
        visivel += todasLinhas[i];
    }
    return visivel;
}

// Aplica uma View proporcional centralizada (para janelas modais quando redimensionadas/maximizada/fullscreen)
inline void aplicarViewProporcional(sf::RenderWindow& janela, float larguraLogica, float alturaLogica, unsigned int janelaW, unsigned int janelaH) {
    if (janelaW == 0 || janelaH == 0)
        return;
    float razaoJanela = static_cast<float>(janelaW) / static_cast<float>(janelaH);
    float razaoLogica = larguraLogica / alturaLogica;

    sf::View view(sf::FloatRect(0.f, 0.f, larguraLogica, alturaLogica));
    float vpX = 0.f, vpY = 0.f, vpW = 1.f, vpH = 1.f;

    if (razaoJanela > razaoLogica) {
        vpW = razaoLogica / razaoJanela;
        vpX = (1.f - vpW) * 0.5f;
    } else if (razaoJanela < razaoLogica) {
        vpH = razaoJanela / razaoLogica;
        vpY = (1.f - vpH) * 0.5f;
    }

    view.setViewport(sf::FloatRect(vpX, vpY, vpW, vpH));
    janela.setView(view);
}

inline sf::Vector2f obterPosicaoMouse(const sf::RenderWindow& janela) {
    sf::Vector2i pixelPos = sf::Mouse::getPosition(janela);
    return janela.mapPixelToCoords(pixelPos);
}

enum class TipoEvento {
    Outro,
    Fechar,
    Redimensionar,
    CliqueMouse,
    RodaMouse,
    TextoDigitado,
    TeclaPressionada
};

struct EventoUI {
    TipoEvento tipo = TipoEvento::Outro;
    int mouseX = 0;
    int mouseY = 0;
    unsigned int novaLargura = 0;
    unsigned int novaAltura = 0;
    bool botaoEsquerdo = false;
    float deltaRoda = 0.f;
    char32_t unicode = 0;
    bool teclaBackspace = false;
    bool teclaEnter = false;
    bool teclaEsc = false;
    bool teclaTab = false;
    bool teclaF11 = false;
};

inline bool pollEvento(sf::RenderWindow& janela, EventoUI& out) {
    out = EventoUI{};
#if SFML_VERSION_MAJOR >= 3
    if (const auto evento = janela.pollEvent()) {
        if (evento->is<sf::Event::Closed>()) {
            out.tipo = TipoEvento::Fechar;
            return true;
        }
        if (const auto* redim = evento->getIf<sf::Event::Resized>()) {
            out.tipo = TipoEvento::Redimensionar;
            out.novaLargura = redim->size.x;
            out.novaAltura = redim->size.y;
            return true;
        }
        if (const auto* clique = evento->getIf<sf::Event::MouseButtonPressed>()) {
            out.tipo = TipoEvento::CliqueMouse;
            out.botaoEsquerdo = (clique->button == sf::Mouse::Button::Left);
            sf::Vector2f worldPos = janela.mapPixelToCoords(clique->position);
            out.mouseX = static_cast<int>(std::round(worldPos.x));
            out.mouseY = static_cast<int>(std::round(worldPos.y));
            return true;
        }
        if (const auto* roda = evento->getIf<sf::Event::MouseWheelScrolled>()) {
            out.tipo = TipoEvento::RodaMouse;
            out.deltaRoda = roda->delta;
            sf::Vector2f worldPos = janela.mapPixelToCoords(roda->position);
            out.mouseX = static_cast<int>(std::round(worldPos.x));
            out.mouseY = static_cast<int>(std::round(worldPos.y));
            return true;
        }
        if (const auto* entrada = evento->getIf<sf::Event::TextEntered>()) {
            out.tipo = TipoEvento::TextoDigitado;
            out.unicode = entrada->unicode;
            return true;
        }
        if (const auto* tecla = evento->getIf<sf::Event::KeyPressed>()) {
            out.tipo = TipoEvento::TeclaPressionada;
            out.teclaBackspace = (tecla->code == sf::Keyboard::Key::Backspace);
            out.teclaEnter = (tecla->code == sf::Keyboard::Key::Enter);
            out.teclaEsc = (tecla->code == sf::Keyboard::Key::Escape);
            out.teclaTab = (tecla->code == sf::Keyboard::Key::Tab);
            out.teclaF11 = (tecla->code == sf::Keyboard::Key::F11);
            return true;
        }
        out.tipo = TipoEvento::Outro;
        return true;
    }
    return false;
#else
    sf::Event evento;
    if (janela.pollEvent(evento)) {
        if (evento.type == sf::Event::Closed) {
            out.tipo = TipoEvento::Fechar;
            return true;
        }
        if (evento.type == sf::Event::Resized) {
            out.tipo = TipoEvento::Redimensionar;
            out.novaLargura = evento.size.width;
            out.novaAltura = evento.size.height;
            return true;
        }
        if (evento.type == sf::Event::MouseButtonPressed) {
            out.tipo = TipoEvento::CliqueMouse;
            out.botaoEsquerdo = (evento.mouseButton.button == sf::Mouse::Left);
            sf::Vector2f worldPos = janela.mapPixelToCoords(sf::Vector2i(evento.mouseButton.x, evento.mouseButton.y));
            out.mouseX = static_cast<int>(std::round(worldPos.x));
            out.mouseY = static_cast<int>(std::round(worldPos.y));
            return true;
        }
        if (evento.type == sf::Event::MouseWheelScrolled) {
            out.tipo = TipoEvento::RodaMouse;
            out.deltaRoda = evento.mouseWheelScroll.delta;
            sf::Vector2f worldPos = janela.mapPixelToCoords(sf::Vector2i(evento.mouseWheelScroll.x, evento.mouseWheelScroll.y));
            out.mouseX = static_cast<int>(std::round(worldPos.x));
            out.mouseY = static_cast<int>(std::round(worldPos.y));
            return true;
        }
        if (evento.type == sf::Event::TextEntered) {
            out.tipo = TipoEvento::TextoDigitado;
            out.unicode = static_cast<char32_t>(evento.text.unicode);
            return true;
        }
        if (evento.type == sf::Event::KeyPressed) {
            out.tipo = TipoEvento::TeclaPressionada;
            out.teclaBackspace = (evento.key.code == sf::Keyboard::BackSpace);
            out.teclaEnter = (evento.key.code == sf::Keyboard::Enter);
            out.teclaEsc = (evento.key.code == sf::Keyboard::Escape);
            out.teclaTab = (evento.key.code == sf::Keyboard::Tab);
            out.teclaF11 = (evento.key.code == sf::Keyboard::F11);
            return true;
        }
        out.tipo = TipoEvento::Outro;
        return true;
    }
    return false;
#endif
}

// [REU002] Os botoes devem emitir um som ao serem selecionados.
void tocarSomBotao();
void tocarSomSucesso();
void tocarSomErro();

} // namespace WareSimUI
