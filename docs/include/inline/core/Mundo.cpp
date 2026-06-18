#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <random>

namespace {
constexpr float TIEMPO_MINA_BASE_SEGUNDOS = 1.5f;

inline float suavizar(float t) {
    return t * t * (3.0f - 2.0f * t);
}

inline float mezclar(float a, float b, float t) {
    return a + (b - a) * t;
}

inline float hash01(int x, int y, unsigned int semilla) {
    std::uint32_t h = static_cast<std::uint32_t>(x) * 374761393u +
                      static_cast<std::uint32_t>(y) * 668265263u +
                      semilla * 1442695041u;
    h = (h ^ (h >> 13u)) * 1274126177u;
    h ^= h >> 16u;
    return static_cast<float>(h & 0x00FFFFFFu) / static_cast<float>(0x01000000u);
}

inline float ruidoValor(float x, float y, float escala, unsigned int semilla) {
    float nx = x * escala;
    float ny = y * escala;
    int x0 = static_cast<int>(std::floor(nx));
    int y0 = static_cast<int>(std::floor(ny));
    float fx = suavizar(nx - static_cast<float>(x0));
    float fy = suavizar(ny - static_cast<float>(y0));

    float a = hash01(x0, y0, semilla);
    float b = hash01(x0 + 1, y0, semilla);
    float c = hash01(x0, y0 + 1, semilla);
    float d = hash01(x0 + 1, y0 + 1, semilla);

    return mezclar(mezclar(a, b, fx), mezclar(c, d, fx), fy);
}

inline float ruidoFractal(float x, float y, float escala, unsigned int semilla) {
    float valor = 0.0f;
    float amplitud = 0.55f;
    float total = 0.0f;

    for (int octava = 0; octava < 4; ++octava) {
        valor += ruidoValor(x, y, escala, semilla + static_cast<unsigned int>(octava * 9176)) * amplitud;
        total += amplitud;
        escala *= 2.0f;
        amplitud *= 0.5f;
    }

    return valor / total;
}

inline TipoBioma calcularBioma(int x, int y, unsigned int semilla) {
    float elevacion = ruidoFractal(static_cast<float>(x), static_cast<float>(y), 0.0045f, semilla + 11u);
    float humedad = ruidoFractal(static_cast<float>(x), static_cast<float>(y), 0.0060f, semilla + 97u);
    float temperatura = ruidoFractal(static_cast<float>(x), static_cast<float>(y), 0.0038f, semilla + 211u);

    if (elevacion > 0.68f) return TipoBioma::Montana;
    if (humedad < 0.34f && temperatura > 0.42f) return TipoBioma::Seco;
    if (humedad > 0.58f) return TipoBioma::Bosque;
    return TipoBioma::Pradera;
}

inline sf::Color ajustarPastoPorBioma(sf::Color base, TipoBioma bioma) {
    switch (bioma) {
        case TipoBioma::Bosque:
            return sf::Color(
                static_cast<std::uint8_t>(base.r * 0.72f),
                static_cast<std::uint8_t>(std::min(210.0f, base.g * 0.92f)),
                static_cast<std::uint8_t>(base.b * 0.78f)
            );
        case TipoBioma::Seco:
            return sf::Color(
                static_cast<std::uint8_t>(std::min(210.0f, base.r * 1.55f)),
                static_cast<std::uint8_t>(std::min(190.0f, base.g * 1.08f)),
                static_cast<std::uint8_t>(base.b * 0.48f)
            );
        case TipoBioma::Montana:
            return sf::Color(
                static_cast<std::uint8_t>(std::min(180.0f, base.r * 1.08f)),
                static_cast<std::uint8_t>(std::min(185.0f, base.g * 0.92f)),
                static_cast<std::uint8_t>(std::min(160.0f, base.b * 1.25f))
            );
        default:
            return base;
    }
}

inline sf::Color colorPastoPixel(int x, int y, TipoBioma bioma) {
    static const int patron[16][16] = {
        {2,2,1,2,2,2,1,2,2,2,2,1,2,2,2,2},
        {2,3,2,2,2,3,2,2,3,2,2,2,3,2,2,2},
        {1,2,2,2,1,2,2,2,2,1,2,2,2,1,2,3},
        {2,2,2,3,2,2,2,2,2,2,3,2,2,2,2,2},
        {2,1,2,2,2,2,1,2,2,2,2,2,2,2,1,2},
        {2,2,3,2,2,3,2,2,2,3,2,2,3,2,2,2},
        {1,2,2,2,2,2,2,1,2,2,2,1,2,2,2,2},
        {2,2,2,2,3,2,2,2,3,2,2,2,2,2,2,3},
        {2,3,1,2,2,2,1,2,2,2,2,3,1,2,2,2},
        {2,2,2,3,2,3,2,2,2,2,2,2,2,3,2,2},
        {1,2,2,2,2,2,2,1,2,1,2,2,2,2,1,2},
        {2,2,3,2,2,2,2,2,3,2,2,3,2,2,2,2},
        {2,2,2,2,3,1,2,2,2,2,2,2,2,2,2,2},
        {1,2,2,2,2,2,3,2,2,1,2,2,3,1,2,3},
        {2,3,2,1,2,2,2,2,3,2,2,2,2,2,2,2},
        {2,2,2,2,2,3,2,2,2,2,3,2,2,2,3,2}
    };

    sf::Color base;
    switch (patron[y % 16][x % 16]) {
        case 1: base = sf::Color(92, 184, 72); break;
        case 3: base = sf::Color(35, 112, 48); break;
        default: base = sf::Color(54, 154, 61); break;
    }
    return ajustarPastoPorBioma(base, bioma);
}

inline sf::Color colorTierraPixel(int x, int y, bool arada) {
    static const char* patron[16] = {
        "bbdBbbdBbbbdBbbb",
        "bDbbblbbDbbdbbLb",
        "bbblbbdbbbLbbdBb",
        "dBbbDbbbdbbbBbbb",
        "bbdbbbLbbDbbblbb",
        "bblbbbdbbbBbbDbb",
        "DbbbdbbbLbbdbbbb",
        "bbBbbbDbbblbbbdb",
        "bbbLbbdbbbDbbbBb",
        "dbbbBbbbdbbbLbbb",
        "bbbdbbbDbbblbbdb",
        "bLbbbdbbbBbbbDbb",
        "bbdbbbLbbbdbbbBb",
        "DbbbblbbDbbbdbbb",
        "bbbBbbbdbbbLbbdb",
        "bbdLbbbBbbdbbbDb"
    };

    if (arada) {
        int ruido = (x * 13 + y * 7 + x * y) % 11;
        if (ruido < 2) return sf::Color(62, 38, 24);
        if (ruido > 8) return sf::Color(122, 75, 42);
        return sf::Color(89, 54, 31);
    }

    switch (patron[y % 16][x % 16]) {
        case 'L': return sf::Color(156, 104, 65);
        case 'l': return sf::Color(139, 91, 55);
        case 'D': return sf::Color(73, 48, 34);
        case 'd': return sf::Color(88, 58, 39);
        case 'B': return sf::Color(101, 66, 43);
        default: return sf::Color(119, 79, 49);
    }
}

inline sf::Color colorPiedraPixel(int x, int y) {
    int ruido = (x * 19 + y * 11 + x * y * 3 + (x - y) * 5) % 19;
    bool grietaPrincipal = (x == y / 2 + 3 && y > 2 && y < 13) ||
                           (x == 12 - y / 3 && y > 5 && y < 15);
    bool grietaSecundaria = (y == 4 && x > 8 && x < 13) ||
                            (y == 11 && x > 2 && x < 7) ||
                            (x == 5 && y > 9 && y < 13);

    if (grietaPrincipal) return sf::Color(54, 56, 57);
    if (grietaSecundaria) return sf::Color(72, 74, 75);
    if (ruido <= 2) return sf::Color(86, 88, 89);
    if (ruido >= 15) return sf::Color(175, 178, 174);
    if ((x + y) % 7 == 0) return sf::Color(145, 147, 145);
    return sf::Color(112, 115, 114);
}

inline void dibujarTexturaPiedra(sf::RenderWindow& ventana, int bloqueX, int bloqueY,
                                 bool vecinoArriba, bool vecinoAbajo,
                                 bool vecinoIzquierda, bool vecinoDerecha) {
    static bool inicializada = false;
    static sf::Texture texturaPiedra;

    if (!inicializada) {
        sf::Image imagen({32, 32}, sf::Color::Transparent);
        for (int py = 0; py < 16; ++py) {
            for (int px = 0; px < 16; ++px) {
                sf::Color color = colorPiedraPixel(px, py);
                for (int sy = 0; sy < 2; ++sy) {
                    for (int sx = 0; sx < 2; ++sx) {
                        imagen.setPixel(
                            {static_cast<unsigned int>(px * 2 + sx), static_cast<unsigned int>(py * 2 + sy)},
                            color
                        );
                    }
                }
            }
        }

        if (!texturaPiedra.loadFromImage(imagen)) {
            return;
        }
        texturaPiedra.setRepeated(false);
        inicializada = true;
    }

    const float x = bloqueX * TAMANIO_BLOQUE_JUEGO;
    const float y = bloqueY * TAMANIO_BLOQUE_JUEGO;
    const float borde = 3.0f;
    const float centro = TAMANIO_BLOQUE_JUEGO - borde * 2.0f;

    auto pieza = [&](int sx, int sy, int sw, int sh, float dx, float dy, float dw, float dh) {
        sf::Sprite sprite(texturaPiedra);
        sprite.setTextureRect(sf::IntRect({sx, sy}, {sw, sh}));
        sprite.setPosition({x + dx, y + dy});
        sprite.setScale({dw / static_cast<float>(sw), dh / static_cast<float>(sh)});
        ventana.draw(sprite);
    };

    pieza(4, 4, 24, 24, borde, borde, centro, centro);

    if (vecinoArriba) pieza(4, 0, 24, 4, borde, 0.0f, centro, borde);
    if (vecinoAbajo) pieza(4, 28, 24, 4, borde, TAMANIO_BLOQUE_JUEGO - borde, centro, borde);
    if (vecinoIzquierda) pieza(0, 4, 4, 24, 0.0f, borde, borde, centro);
    if (vecinoDerecha) pieza(28, 4, 4, 24, TAMANIO_BLOQUE_JUEGO - borde, borde, borde, centro);

    if (vecinoArriba && vecinoIzquierda) pieza(0, 0, 4, 4, 0.0f, 0.0f, borde, borde);
    if (vecinoArriba && vecinoDerecha) pieza(28, 0, 4, 4, TAMANIO_BLOQUE_JUEGO - borde, 0.0f, borde, borde);
    if (vecinoAbajo && vecinoIzquierda) pieza(0, 28, 4, 4, 0.0f, TAMANIO_BLOQUE_JUEGO - borde, borde, borde);
    if (vecinoAbajo && vecinoDerecha) pieza(28, 28, 4, 4, TAMANIO_BLOQUE_JUEGO - borde, TAMANIO_BLOQUE_JUEGO - borde, borde, borde);

    unsigned int h = static_cast<unsigned int>(bloqueX * 73856093u) ^
                     static_cast<unsigned int>(bloqueY * 19349663u);
    h ^= h >> 13u;
    h *= 1274126177u;
    h ^= h >> 16u;

    auto marca = [&](float mx, float my, float mw, float mh, sf::Color color) {
        sf::RectangleShape r({mw, mh});
        r.setPosition({x + mx, y + my});
        r.setFillColor(color);
        ventana.draw(r);
    };

    marca(4.0f + static_cast<float>(h % 4u), 5.0f, 8.0f, 2.0f, sf::Color(70, 72, 73, 135));
    marca(8.0f, 8.0f + static_cast<float>((h >> 3u) % 4u), 2.0f, 7.0f, sf::Color(62, 64, 65, 135));
    marca(13.0f, 14.0f, 7.0f, 2.0f, sf::Color(157, 160, 157, 95));
    if ((h & 3u) == 0u) {
        marca(15.0f, 4.0f, 5.0f, 2.0f, sf::Color(182, 185, 181, 115));
    }

    if (!vecinoArriba) marca(borde, borde - 1.0f, centro, 1.0f, sf::Color(190, 193, 190, 95));
    if (!vecinoIzquierda) marca(borde - 1.0f, borde, 1.0f, centro, sf::Color(185, 188, 185, 75));
    if (!vecinoDerecha) marca(TAMANIO_BLOQUE_JUEGO - borde, borde, 1.0f, centro, sf::Color(55, 57, 58, 95));
    if (!vecinoAbajo) marca(borde, TAMANIO_BLOQUE_JUEGO - borde, centro, 1.0f, sf::Color(48, 50, 51, 105));
}

inline void dibujarRelieveMuro(sf::RenderWindow& ventana, int bloqueX, int bloqueY,
                               bool vecinoArriba, bool vecinoAbajo,
                               bool vecinoIzquierda, bool vecinoDerecha,
                               sf::Color luz, sf::Color sombra) {
    const float x = bloqueX * TAMANIO_BLOQUE_JUEGO;
    const float y = bloqueY * TAMANIO_BLOQUE_JUEGO;

    auto rect = [&](float rx, float ry, float rw, float rh, sf::Color color) {
        sf::RectangleShape r({rw, rh});
        r.setPosition({x + rx, y + ry});
        r.setFillColor(color);
        ventana.draw(r);
    };

    if (!vecinoAbajo) {
        rect(2.0f, TAMANIO_BLOQUE_JUEGO - 1.0f, TAMANIO_BLOQUE_JUEGO + 2.0f, 5.0f, sf::Color(10, 10, 10, 70));
        rect(0.0f, TAMANIO_BLOQUE_JUEGO - 6.0f, TAMANIO_BLOQUE_JUEGO, 6.0f, sombra);
    }
    if (!vecinoDerecha) {
        rect(TAMANIO_BLOQUE_JUEGO - 4.0f, 4.0f, 4.0f, TAMANIO_BLOQUE_JUEGO - 5.0f, sf::Color(sombra.r, sombra.g, sombra.b, 115));
    }
    if (!vecinoIzquierda) {
        rect(0.0f, 3.0f, 2.0f, TAMANIO_BLOQUE_JUEGO - 5.0f, sf::Color(luz.r, luz.g, luz.b, 65));
    }
    if (!vecinoArriba) {
        rect(2.0f, 0.0f, TAMANIO_BLOQUE_JUEGO - 4.0f, 2.0f, luz);
        rect(3.0f, 3.0f, TAMANIO_BLOQUE_JUEGO - 6.0f, 1.0f, sf::Color(255, 255, 255, 35));
    }
}

inline void dibujarBloqueTierraElevado(sf::RenderWindow& ventana, int bloqueX, int bloqueY,
                                       bool vecinoArriba, bool vecinoAbajo,
                                       bool vecinoIzquierda, bool vecinoDerecha,
                                       TipoBioma bioma, bool conPasto = false) {
    const float x = bloqueX * TAMANIO_BLOQUE_JUEGO;
    const float y = bloqueY * TAMANIO_BLOQUE_JUEGO;

    auto rect = [&](float rx, float ry, float rw, float rh, sf::Color color) {
        sf::RectangleShape r({rw, rh});
        r.setPosition({x + rx, y + ry});
        r.setFillColor(color);
        ventana.draw(r);
    };

    if (!vecinoAbajo) rect(3.0f, 22.0f, 27.0f, 7.0f, sf::Color(8, 7, 6, 125));
    if (!vecinoDerecha) rect(22.0f, 1.0f, 6.0f, 23.0f, sf::Color(8, 7, 6, 80));

    if (!vecinoAbajo) rect(0.0f, 15.0f, 24.0f, 9.0f, sf::Color(58, 35, 22));
    if (!vecinoDerecha) rect(19.0f, -2.0f, 5.0f, 23.0f, sf::Color(71, 43, 26));
    if (!vecinoIzquierda) rect(0.0f, -2.0f, 2.0f, 22.0f, sf::Color(162, 101, 57, 180));

    rect(0.0f, -6.0f, 24.0f, 2.0f, sf::Color(35, 24, 17, 170));
    rect(0.0f, -6.0f, 2.0f, 21.0f, sf::Color(35, 24, 17, 130));
    rect(22.0f, -6.0f, 2.0f, 21.0f, sf::Color(35, 24, 17, 150));
    rect(0.0f, 14.0f, 24.0f, 2.0f, sf::Color(35, 24, 17, 170));

    if (conPasto) {
        sf::RectangleShape tapa({24.0f, 20.0f});
        tapa.setPosition({x, y - 6.0f});
        tapa.setFillColor(ajustarPastoPorBioma(sf::Color(54, 154, 61), bioma));
        ventana.draw(tapa);
    } else {
        const float escalaPixelX = 24.0f / 16.0f;
        const float escalaPixelY = 20.0f / 16.0f;
        for (int py = 0; py < 16; ++py) {
            for (int px = 0; px < 16; ++px) {
                sf::RectangleShape pixelTierra({escalaPixelX + 0.15f, escalaPixelY + 0.15f});
                pixelTierra.setPosition({x + static_cast<float>(px) * escalaPixelX, y - 6.0f + static_cast<float>(py) * escalaPixelY});
                pixelTierra.setFillColor(colorTierraPixel(px, py, false));
                ventana.draw(pixelTierra);
            }
        }
    }

    sf::RectangleShape bordeSuperior({24.0f, 2.0f});
    bordeSuperior.setPosition({x, y - 6.0f});
    bordeSuperior.setFillColor(conPasto ? ajustarPastoPorBioma(sf::Color(104, 202, 84), bioma) : sf::Color(157, 101, 62));
    ventana.draw(bordeSuperior);

    if (conPasto) {
        rect(0.0f, 10.0f, 24.0f, 4.0f, sf::Color(75, 126, 45));
        rect(2.0f, 10.0f, 4.0f, 2.0f, sf::Color(105, 173, 58));
        rect(13.0f, 11.0f, 5.0f, 2.0f, sf::Color(41, 100, 40));
    }

    sf::RectangleShape bordeFrontal({24.0f, 3.0f});
    bordeFrontal.setPosition({x, y + 13.0f});
    bordeFrontal.setFillColor(conPasto ? sf::Color(48, 78, 32) : sf::Color(64, 38, 24));
    ventana.draw(bordeFrontal);

    unsigned int h = static_cast<unsigned int>(bloqueX * 73856093u) ^
                     static_cast<unsigned int>(bloqueY * 19349663u);
    h ^= h >> 13u;
    h *= 1274126177u;
    h ^= h >> 16u;

    for (int i = 0; i < 7; ++i) {
        float px = 3.0f + static_cast<float>((h >> (i * 3)) % 18u);
        float py = -3.0f + static_cast<float>((h >> (i * 2)) % 12u);
        sf::RectangleShape mota({2.0f, 2.0f});
        mota.setPosition({x + px, y + py});
        mota.setFillColor(conPasto
            ? (i % 2 == 0 ? ajustarPastoPorBioma(sf::Color(35, 112, 48), bioma) : ajustarPastoPorBioma(sf::Color(92, 184, 72), bioma))
            : (i % 2 == 0 ? sf::Color(86, 52, 31) : sf::Color(151, 94, 53)));
        ventana.draw(mota);
    }

    if (bioma == TipoBioma::Seco) {
        rect(2.0f, -2.0f, 20.0f, 1.0f, sf::Color(196, 156, 74, 90));
    }
}

inline void dibujarTierraRebajada(sf::RenderWindow& ventana, int bloqueX, int bloqueY) {
    const float x = bloqueX * TAMANIO_BLOQUE_JUEGO;
    const float y = bloqueY * TAMANIO_BLOQUE_JUEGO;

    sf::RectangleShape sombraInterior({20.0f, 20.0f});
    sombraInterior.setPosition({x + 2.0f, y + 2.0f});
    sombraInterior.setFillColor(sf::Color(55, 34, 22, 70));
    ventana.draw(sombraInterior);

    sf::RectangleShape luzSup({20.0f, 2.0f});
    luzSup.setPosition({x + 2.0f, y + 2.0f});
    luzSup.setFillColor(sf::Color(172, 108, 59, 90));
    ventana.draw(luzSup);

    sf::RectangleShape sombraInf({20.0f, 3.0f});
    sombraInf.setPosition({x + 2.0f, y + 19.0f});
    sombraInf.setFillColor(sf::Color(44, 27, 18, 95));
    ventana.draw(sombraInf);
}

inline unsigned int ruidoDecoracion(int x, int y) {
    unsigned int h = static_cast<unsigned int>(x * 73856093u) ^
                     static_cast<unsigned int>(y * 19349663u);
    h ^= h >> 13u;
    h *= 1274126177u;
    return h ^ (h >> 16u);
}

inline void dibujarTextura16(sf::RenderWindow& ventana, int bloqueX, int bloqueY, bool pasto, bool arada = false, TipoBioma bioma = TipoBioma::Pradera) {
    static bool inicializadas = false;
    static sf::Texture texturaPastoPradera;
    static sf::Texture texturaPastoBosque;
    static sf::Texture texturaPastoSeco;
    static sf::Texture texturaPastoMontana;
    static sf::Texture texturaTierra;
    static sf::Texture texturaTierraArada;

    if (!inicializadas) {
        sf::Image imagenPastoPradera({32, 32}, sf::Color::Transparent);
        sf::Image imagenPastoBosque({32, 32}, sf::Color::Transparent);
        sf::Image imagenPastoSeco({32, 32}, sf::Color::Transparent);
        sf::Image imagenPastoMontana({32, 32}, sf::Color::Transparent);
        sf::Image imagenTierra({32, 32}, sf::Color::Transparent);
        sf::Image imagenTierraArada({32, 32}, sf::Color::Transparent);

        for (int py = 0; py < 16; ++py) {
            for (int px = 0; px < 16; ++px) {
                sf::Color pastoPradera = colorPastoPixel(px, py, TipoBioma::Pradera);
                sf::Color pastoBosque = colorPastoPixel(px, py, TipoBioma::Bosque);
                sf::Color pastoSeco = colorPastoPixel(px, py, TipoBioma::Seco);
                sf::Color pastoMontana = colorPastoPixel(px, py, TipoBioma::Montana);
                sf::Color tierraColor = colorTierraPixel(px, py, false);
                sf::Color aradaColor = colorTierraPixel(px, py, true);

                for (int sy = 0; sy < 2; ++sy) {
                    for (int sx = 0; sx < 2; ++sx) {
                        sf::Vector2u destino(static_cast<unsigned int>(px * 2 + sx), static_cast<unsigned int>(py * 2 + sy));
                        imagenPastoPradera.setPixel(destino, pastoPradera);
                        imagenPastoBosque.setPixel(destino, pastoBosque);
                        imagenPastoSeco.setPixel(destino, pastoSeco);
                        imagenPastoMontana.setPixel(destino, pastoMontana);
                        imagenTierra.setPixel(destino, tierraColor);
                        imagenTierraArada.setPixel(destino, aradaColor);
                    }
                }
            }
        }

        bool cargoPastoPradera = texturaPastoPradera.loadFromImage(imagenPastoPradera);
        bool cargoPastoBosque = texturaPastoBosque.loadFromImage(imagenPastoBosque);
        bool cargoPastoSeco = texturaPastoSeco.loadFromImage(imagenPastoSeco);
        bool cargoPastoMontana = texturaPastoMontana.loadFromImage(imagenPastoMontana);
        bool cargoTierra = texturaTierra.loadFromImage(imagenTierra);
        bool cargoTierraArada = texturaTierraArada.loadFromImage(imagenTierraArada);
        if (!cargoPastoPradera || !cargoPastoBosque || !cargoPastoSeco || !cargoPastoMontana ||
            !cargoTierra || !cargoTierraArada) {
            return;
        }
        texturaPastoPradera.setRepeated(false);
        texturaPastoBosque.setRepeated(false);
        texturaPastoSeco.setRepeated(false);
        texturaPastoMontana.setRepeated(false);
        texturaTierra.setRepeated(false);
        texturaTierraArada.setRepeated(false);
        inicializadas = true;
    }

    const sf::Texture* textura = arada ? &texturaTierraArada : &texturaTierra;
    if (pasto) {
        switch (bioma) {
            case TipoBioma::Bosque: textura = &texturaPastoBosque; break;
            case TipoBioma::Seco: textura = &texturaPastoSeco; break;
            case TipoBioma::Montana: textura = &texturaPastoMontana; break;
            default: textura = &texturaPastoPradera; break;
        }
    }
    sf::Sprite sprite(*textura);
    sprite.setPosition({bloqueX * TAMANIO_BLOQUE_JUEGO, bloqueY * TAMANIO_BLOQUE_JUEGO});
    sprite.setScale({ESCALA_BLOQUE_JUEGO, ESCALA_BLOQUE_JUEGO});
    ventana.draw(sprite);
}

inline void dibujarPlantasDecorativas(sf::RenderWindow& ventana, int bloqueX, int bloqueY, TipoBioma bioma) {
    unsigned int h = ruidoDecoracion(bloqueX, bloqueY);
    int densidad = 22;
    if (bioma == TipoBioma::Bosque) densidad = 36;
    if (bioma == TipoBioma::Seco) densidad = 10;
    if (bioma == TipoBioma::Montana) densidad = 18;

    if (static_cast<int>(h % 100u) >= densidad) {
        return;
    }

    float baseX = bloqueX * TAMANIO_BLOQUE_JUEGO;
    float baseY = bloqueY * TAMANIO_BLOQUE_JUEGO;
    float ox = static_cast<float>((h >> 4u) % 15u) + 4.0f;
    float oy = static_cast<float>((h >> 9u) % 12u) + 7.0f;

    sf::RectangleShape pixel;
    auto rect = [&](float x, float y, float w, float hgt, sf::Color color) {
        pixel.setSize({w, hgt});
        pixel.setPosition({baseX + x, baseY + y});
        pixel.setFillColor(color);
        ventana.draw(pixel);
    };

    sf::Color tallo = bioma == TipoBioma::Seco ? sf::Color(118, 132, 51) : sf::Color(37, 125, 53);
    sf::Color hoja = bioma == TipoBioma::Seco ? sf::Color(161, 151, 58) : sf::Color(71, 174, 68);

    int tipo = static_cast<int>((h >> 15u) % 5u);
    if (tipo <= 1) {
        rect(ox, oy, 1.5f, 6.0f, tallo);
        rect(ox - 2.0f, oy + 2.0f, 3.0f, 1.5f, hoja);
        rect(ox + 1.0f, oy + 4.0f, 3.0f, 1.5f, hoja);
        return;
    }

    if (tipo == 2 && bioma != TipoBioma::Seco) {
        sf::Color flor = ((h >> 20u) & 1u) ? sf::Color(235, 223, 92) : sf::Color(226, 104, 190);
        rect(ox + 1.0f, oy + 3.0f, 1.5f, 6.0f, tallo);
        rect(ox, oy + 1.0f, 3.0f, 3.0f, flor);
        rect(ox + 1.0f, oy + 2.0f, 1.0f, 1.0f, sf::Color(255, 245, 180));
        return;
    }

    if (tipo == 3) {
        rect(ox - 2.0f, oy + 3.0f, 1.5f, 6.0f, tallo);
        rect(ox + 2.0f, oy + 1.0f, 1.5f, 7.0f, tallo);
        rect(ox + 5.0f, oy + 4.0f, 1.5f, 5.0f, hoja);
        return;
    }

    if (bioma == TipoBioma::Bosque) {
        rect(ox, oy + 5.0f, 6.0f, 3.0f, sf::Color(38, 105, 45));
        rect(ox + 1.0f, oy + 2.0f, 4.0f, 4.0f, sf::Color(55, 146, 62));
    }
}

inline sf::Color colorAguaPixel(int x, int y, int frame, bool profunda) {
    int ondaA = (x + frame * 3 + (y / 3)) % 16;
    int ondaB = (y + frame * 2 + (x / 4)) % 13;
    int brillo = (ondaA == 0 || ondaA == 1 || ondaB == 0) ? 1 : 0;
    int sombra = ((x * 5 + y * 7 + frame * 4) % 19 == 0) ? 1 : 0;

    if (profunda) {
        if (brillo) return sf::Color(28, 82, 171);
        if (sombra) return sf::Color(8, 34, 112);
        return sf::Color(12, 55, 145);
    }

    if (brillo) return sf::Color(88, 184, 245);
    if (sombra) return sf::Color(21, 105, 194);
    return sf::Color(34, 144, 225);
}

inline void dibujarAguaAnimada(sf::RenderWindow& ventana, int bloqueX, int bloqueY, bool profunda) {
    static bool inicializadas = false;
    static sf::Texture agua[4];
    static sf::Texture aguaProfunda[4];
    static sf::Clock relojAgua;

    if (!inicializadas) {
        for (int frame = 0; frame < 4; ++frame) {
            sf::Image imagenAgua({32, 32}, sf::Color::Transparent);
            sf::Image imagenProfunda({32, 32}, sf::Color::Transparent);

            for (int py = 0; py < 16; ++py) {
                for (int px = 0; px < 16; ++px) {
                    sf::Color clara = colorAguaPixel(px, py, frame, false);
                    sf::Color oscura = colorAguaPixel(px, py, frame, true);

                    for (int sy = 0; sy < 2; ++sy) {
                        for (int sx = 0; sx < 2; ++sx) {
                            sf::Vector2u destino(
                                static_cast<unsigned int>(px * 2 + sx),
                                static_cast<unsigned int>(py * 2 + sy)
                            );
                            imagenAgua.setPixel(destino, clara);
                            imagenProfunda.setPixel(destino, oscura);
                        }
                    }
                }
            }

            if (!agua[frame].loadFromImage(imagenAgua) || !aguaProfunda[frame].loadFromImage(imagenProfunda)) {
                return;
            }
            agua[frame].setSmooth(false);
            aguaProfunda[frame].setSmooth(false);
        }
        inicializadas = true;
    }

    int frameBase = static_cast<int>(relojAgua.getElapsedTime().asSeconds() * 4.0f) % 4;
    int frame = (frameBase + ((bloqueX + bloqueY) & 1)) % 4;
    sf::Sprite sprite(profunda ? aguaProfunda[frame] : agua[frame]);
    sprite.setPosition({bloqueX * TAMANIO_BLOQUE_JUEGO, bloqueY * TAMANIO_BLOQUE_JUEGO});
    sprite.setScale({ESCALA_BLOQUE_JUEGO, ESCALA_BLOQUE_JUEGO});
    ventana.draw(sprite);
}

inline void dibujarTextura16Lenta(sf::RenderWindow& ventana, int bloqueX, int bloqueY, bool pasto, bool arada = false) {
    constexpr float pixel = 2.0f;
    sf::RectangleShape cuadro({pixel, pixel});
    for (int py = 0; py < 16; ++py) {
        for (int px = 0; px < 16; ++px) {
            cuadro.setPosition({bloqueX * TAMANIO_BLOQUE_JUEGO + px * pixel, bloqueY * TAMANIO_BLOQUE_JUEGO + py * pixel});
            cuadro.setFillColor(pasto ? colorPastoPixel(px, py, TipoBioma::Pradera) : colorTierraPixel(px, py, arada));
            ventana.draw(cuadro);
        }
    }
}

inline sf::Color conAlpha(sf::Color color, std::uint8_t alpha) {
    color.a = alpha;
    return color;
}

inline void dibujarArbol(sf::RenderWindow& ventana, int bloqueX, int bloqueY, const Bloque& bloque,
                         bool dibujarBase, bool dibujarCopa, std::uint8_t alphaCopa = 255) {
    const float baseX = bloqueX * TAMANIO_BLOQUE_JUEGO - 20.0f;
    const float baseY = bloqueY * TAMANIO_BLOQUE_JUEGO - 46.0f;
    const int variante = bloque.varianteArbol % 3;

    sf::Color hojaOscura(31, 92, 47);
    sf::Color hojaBase(45, 133, 63);
    sf::Color hojaClara(79, 178, 78);

    if (bloque.bioma == TipoBioma::Bosque) {
        hojaOscura = sf::Color(23, 73, 48);
        hojaBase = sf::Color(35, 112, 65);
        hojaClara = sf::Color(62, 151, 83);
    } else if (bloque.bioma == TipoBioma::Seco) {
        hojaOscura = sf::Color(93, 98, 42);
        hojaBase = sf::Color(139, 145, 54);
        hojaClara = sf::Color(184, 178, 65);
    } else if (bloque.bioma == TipoBioma::Montana) {
        hojaOscura = sf::Color(22, 78, 70);
        hojaBase = sf::Color(32, 104, 88);
        hojaClara = sf::Color(58, 139, 112);
    }

    auto rect = [&](float x, float y, float w, float h, sf::Color color) {
        sf::RectangleShape r({w, h});
        r.setPosition({baseX + x, baseY + y});
        r.setFillColor(color);
        ventana.draw(r);
    };

    if (dibujarBase) {
        sf::RectangleShape sombra({44.0f, 11.0f});
        sombra.setPosition({baseX + 14.0f, baseY + 70.0f});
        sombra.setFillColor(sf::Color(18, 35, 22, 95));
        ventana.draw(sombra);

        rect(30.0f, 43.0f, 12.0f, 35.0f, sf::Color(92, 54, 31));
        rect(34.0f, 44.0f, 4.0f, 30.0f, sf::Color(143, 85, 43));
        rect(28.0f, 62.0f, 5.0f, 9.0f, sf::Color(70, 41, 25));
        rect(39.0f, 58.0f, 5.0f, 12.0f, sf::Color(70, 41, 25));
    }

    if (dibujarCopa) {
        hojaOscura = conAlpha(hojaOscura, alphaCopa);
        hojaBase = conAlpha(hojaBase, alphaCopa);
        hojaClara = conAlpha(hojaClara, alphaCopa);
        sf::Color hojaMedia1 = conAlpha(sf::Color(43, 125, 61), alphaCopa);
        sf::Color hojaMedia2 = conAlpha(sf::Color(50, 149, 69), alphaCopa);
        sf::Color hojaSombra1 = conAlpha(sf::Color(38, 111, 55), alphaCopa);
        sf::Color hojaSombra2 = conAlpha(sf::Color(30, 88, 46), alphaCopa);
        sf::Color hojaLuz1 = conAlpha(sf::Color(73, 166, 73), alphaCopa);
        sf::Color hojaLuz2 = conAlpha(sf::Color(75, 169, 75), alphaCopa);
        sf::Color hojaSombra3 = conAlpha(sf::Color(31, 95, 48), alphaCopa);
        sf::Color hojaSombra4 = conAlpha(sf::Color(29, 82, 44), alphaCopa);

        rect(18.0f, variante == 1 ? 2.0f : 7.0f, 36.0f, 7.0f, hojaOscura);
        rect(variante == 2 ? 7.0f : 10.0f, 14.0f, variante == 2 ? 58.0f : 52.0f, 9.0f, hojaMedia1);
        rect(4.0f, 23.0f, 64.0f, 12.0f, hojaMedia2);
        rect(variante == 1 ? 4.0f : 0.0f, 35.0f, variante == 1 ? 64.0f : 72.0f, 12.0f, hojaBase);
        rect(7.0f, 47.0f, 58.0f, 11.0f, hojaSombra1);
        rect(17.0f, 58.0f, 38.0f, 8.0f, hojaSombra2);

        rect(24.0f, variante == 1 ? 5.0f : 10.0f, 12.0f, 6.0f, hojaClara);
        rect(42.0f, 20.0f, 12.0f, 7.0f, hojaLuz1);
        rect(13.0f, 28.0f, 10.0f, 7.0f, hojaLuz2);
        rect(26.0f, 35.0f, 9.0f, 8.0f, hojaSombra2);
        rect(52.0f, 37.0f, 8.0f, 7.0f, hojaSombra3);
        rect(13.0f, 48.0f, 9.0f, 6.0f, hojaSombra4);

        if (variante == 1) {
            rect(27.0f, -2.0f, 18.0f, 6.0f, hojaOscura);
            rect(14.0f, 54.0f, 44.0f, 7.0f, hojaOscura);
        } else if (variante == 2) {
            rect(-4.0f, 39.0f, 10.0f, 8.0f, hojaBase);
            rect(66.0f, 36.0f, 10.0f, 8.0f, hojaBase);
            rect(31.0f, 12.0f, 18.0f, 7.0f, hojaClara);
        }
    }
}

inline void dibujarTecho(sf::RenderWindow& ventana, int bloqueX, int bloqueY, std::uint8_t alpha) {
    const float x = bloqueX * TAMANIO_BLOQUE_JUEGO;
    const float y = bloqueY * TAMANIO_BLOQUE_JUEGO - 7.0f;

    auto color = [&](sf::Color c) {
        c.a = alpha;
        return c;
    };

    sf::RectangleShape sombra({TAMANIO_BLOQUE_JUEGO + 3.0f, 6.0f});
    sombra.setPosition({x + 2.0f, y + TAMANIO_BLOQUE_JUEGO + 2.0f});
    sombra.setFillColor(sf::Color(10, 10, 16, static_cast<std::uint8_t>(alpha * 0.38f)));
    ventana.draw(sombra);

    sf::RectangleShape base({TAMANIO_BLOQUE_JUEGO, TAMANIO_BLOQUE_JUEGO});
    base.setPosition({x, y});
    base.setFillColor(color(sf::Color(70, 70, 88)));
    ventana.draw(base);

    sf::RectangleShape bordeSup({TAMANIO_BLOQUE_JUEGO, 3.0f});
    bordeSup.setPosition({x, y});
    bordeSup.setFillColor(color(sf::Color(132, 132, 154)));
    ventana.draw(bordeSup);

    sf::RectangleShape bordeInf({TAMANIO_BLOQUE_JUEGO, 5.0f});
    bordeInf.setPosition({x, y + TAMANIO_BLOQUE_JUEGO - 5.0f});
    bordeInf.setFillColor(color(sf::Color(40, 40, 56)));
    ventana.draw(bordeInf);

    for (int i = 0; i < 3; ++i) {
        sf::RectangleShape linea({TAMANIO_BLOQUE_JUEGO - 4.0f, 1.0f});
        linea.setPosition({x + 2.0f, y + 7.0f + i * 6.0f});
        linea.setFillColor(color(sf::Color(34, 34, 48)));
        ventana.draw(linea);
    }
}

inline void dibujarPuerta(sf::RenderWindow& ventana, int bloqueX, int bloqueY, bool abierta) {
    const float x = bloqueX * TAMANIO_BLOQUE_JUEGO;
    const float y = bloqueY * TAMANIO_BLOQUE_JUEGO;

    dibujarTextura16(ventana, bloqueX, bloqueY, true, false, TipoBioma::Pradera);

    sf::RectangleShape sombra({TAMANIO_BLOQUE_JUEGO + 2.0f, 5.0f});
    sombra.setPosition({x + 2.0f, y + TAMANIO_BLOQUE_JUEGO - 1.0f});
    sombra.setFillColor(sf::Color(12, 8, 5, 80));
    ventana.draw(sombra);

    sf::RectangleShape marco({TAMANIO_BLOQUE_JUEGO, TAMANIO_BLOQUE_JUEGO});
    marco.setPosition({x, y - 3.0f});
    marco.setFillColor(sf::Color(72, 42, 20));
    ventana.draw(marco);

    sf::RectangleShape hoja({abierta ? 7.0f : 18.0f, 20.0f});
    hoja.setPosition({x + (abierta ? 3.0f : 3.0f), y - 1.0f});
    hoja.setFillColor(sf::Color(145, 88, 38));
    ventana.draw(hoja);

    sf::RectangleShape panel({abierta ? 3.0f : 12.0f, 6.0f});
    panel.setPosition({x + (abierta ? 5.0f : 6.0f), y + 4.0f});
    panel.setFillColor(sf::Color(96, 57, 28));
    ventana.draw(panel);

    sf::CircleShape perilla(1.7f);
    perilla.setPosition({x + (abierta ? 7.0f : 16.0f), y + 11.0f});
    perilla.setFillColor(sf::Color(222, 178, 78));
    ventana.draw(perilla);
}

inline void dibujarCaminoAldea(sf::RenderWindow& ventana, int bloqueX, int bloqueY) {
    const float x = bloqueX * TAMANIO_BLOQUE_JUEGO;
    const float y = bloqueY * TAMANIO_BLOQUE_JUEGO;
    sf::RectangleShape base({TAMANIO_BLOQUE_JUEGO, TAMANIO_BLOQUE_JUEGO});
    base.setPosition({x, y});
    base.setFillColor(sf::Color(134, 101, 62));
    ventana.draw(base);

    unsigned int h = ruidoDecoracion(bloqueX, bloqueY);
    for (int i = 0; i < 7; ++i) {
        sf::RectangleShape mota({2.0f + static_cast<float>((h >> i) & 1u), 2.0f});
        mota.setPosition({
            x + 3.0f + static_cast<float>((h >> (i * 3)) % 18u),
            y + 4.0f + static_cast<float>((h >> (i * 2)) % 16u)
        });
        mota.setFillColor(i % 2 == 0 ? sf::Color(94, 69, 43) : sf::Color(174, 132, 79));
        ventana.draw(mota);
    }
}

inline bool esCultivoBloque(TipoBloque tipo) {
    return tipo == TipoBloque::CultivoTrigo ||
           tipo == TipoBloque::CultivoZanahoria ||
           tipo == TipoBloque::CultivoPatata;
}

inline void dibujarCultivo(sf::RenderWindow& ventana, int bloqueX, int bloqueY, TipoBloque tipo, int fase) {
    dibujarTextura16(ventana, bloqueX, bloqueY, false, true, TipoBioma::Pradera);

    const float x = bloqueX * TAMANIO_BLOQUE_JUEGO;
    const float y = bloqueY * TAMANIO_BLOQUE_JUEGO;
    sf::Color tallo(58, 139, 47);
    sf::Color fruto(220, 188, 64);
    if (tipo == TipoBloque::CultivoZanahoria) fruto = sf::Color(232, 112, 36);
    if (tipo == TipoBloque::CultivoPatata) fruto = sf::Color(180, 136, 72);

    fase = std::clamp(fase, 0, 3);
    int plantas = fase == 0 ? 2 : 4;
    float altoPlanta = 4.0f + static_cast<float>(fase) * 3.0f;

    for (int i = 0; i < plantas; ++i) {
        float ox = 4.0f + static_cast<float>(i % 2) * 10.0f;
        float oy = 6.0f + static_cast<float>(i / 2) * 8.0f;
        sf::RectangleShape hoja({3.0f, altoPlanta});
        hoja.setPosition({x + ox + 2.0f, y + oy + 10.0f - altoPlanta});
        hoja.setFillColor(fase < 2 ? sf::Color(72, 158, 54) : tallo);
        ventana.draw(hoja);
        if (fase >= 2) {
            sf::RectangleShape grano({fase == 2 ? 3.0f : 5.0f, 3.0f});
            grano.setPosition({x + ox, y + oy + (fase == 2 ? 2.0f : 0.0f)});
            grano.setFillColor(fase == 2 ? sf::Color(fruto.r, fruto.g, fruto.b, 170) : fruto);
            ventana.draw(grano);
        }
    }
}

inline void dibujarLava(sf::RenderWindow& ventana, int bloqueX, int bloqueY) {
    const float x = bloqueX * TAMANIO_BLOQUE_JUEGO;
    const float y = bloqueY * TAMANIO_BLOQUE_JUEGO;
    sf::RectangleShape base({TAMANIO_BLOQUE_JUEGO, TAMANIO_BLOQUE_JUEGO});
    base.setPosition({x, y});
    base.setFillColor(sf::Color(218, 58, 18));
    ventana.draw(base);
    for (int i = 0; i < 5; ++i) {
        sf::RectangleShape brillo({8.0f, 2.0f});
        brillo.setPosition({x + 2.0f + static_cast<float>((bloqueX * 5 + i * 7) % 13), y + 4.0f + i * 4.0f});
        brillo.setFillColor(i % 2 == 0 ? sf::Color(255, 198, 55) : sf::Color(255, 114, 30));
        ventana.draw(brillo);
    }
}

inline void dibujarCofre(sf::RenderWindow& ventana, int bloqueX, int bloqueY) {
    const float x = bloqueX * TAMANIO_BLOQUE_JUEGO;
    const float y = bloqueY * TAMANIO_BLOQUE_JUEGO;
    sf::RectangleShape sombra({TAMANIO_BLOQUE_JUEGO + 3.0f, 5.0f});
    sombra.setPosition({x + 2.0f, y + TAMANIO_BLOQUE_JUEGO - 1.0f});
    sombra.setFillColor(sf::Color(12, 9, 5, 88));
    ventana.draw(sombra);

    sf::RectangleShape cuerpo({20.0f, 17.0f});
    cuerpo.setPosition({x + 2.0f, y + 5.0f});
    cuerpo.setFillColor(sf::Color(160, 92, 32));
    cuerpo.setOutlineColor(sf::Color(54, 31, 16));
    cuerpo.setOutlineThickness(2.0f);
    ventana.draw(cuerpo);

    sf::RectangleShape tapa({20.0f, 6.0f});
    tapa.setPosition({x + 2.0f, y + 3.0f});
    tapa.setFillColor(sf::Color(194, 125, 45));
    ventana.draw(tapa);

    sf::RectangleShape chapa({4.0f, 5.0f});
    chapa.setPosition({x + 10.0f, y + 10.0f});
    chapa.setFillColor(sf::Color(226, 186, 88));
    ventana.draw(chapa);
}

inline void dibujarYunque(sf::RenderWindow& ventana, int bloqueX, int bloqueY) {
    const float x = bloqueX * TAMANIO_BLOQUE_JUEGO;
    const float y = bloqueY * TAMANIO_BLOQUE_JUEGO;
    sf::RectangleShape sombra({22.0f, 5.0f});
    sombra.setPosition({x + 3.0f, y + 20.0f});
    sombra.setFillColor(sf::Color(8, 8, 8, 85));
    ventana.draw(sombra);

    auto rect = [&](float rx, float ry, float w, float h, sf::Color color) {
        sf::RectangleShape r({w, h});
        r.setPosition({x + rx, y + ry});
        r.setFillColor(color);
        ventana.draw(r);
    };
    rect(3.0f, 5.0f, 18.0f, 5.0f, sf::Color(98, 102, 106));
    rect(6.0f, 10.0f, 12.0f, 6.0f, sf::Color(66, 69, 73));
    rect(4.0f, 16.0f, 16.0f, 5.0f, sf::Color(88, 91, 95));
    rect(18.0f, 7.0f, 4.0f, 3.0f, sf::Color(128, 132, 136));
}

inline void dibujarCama(sf::RenderWindow& ventana, int bloqueX, int bloqueY) {
    const float x = bloqueX * TAMANIO_BLOQUE_JUEGO;
    const float y = bloqueY * TAMANIO_BLOQUE_JUEGO;

    sf::RectangleShape sombra({22.0f, 5.0f});
    sombra.setPosition({x + 2.0f, y + 19.0f});
    sombra.setFillColor(sf::Color(18, 10, 10, 80));
    ventana.draw(sombra);

    sf::RectangleShape base({20.0f, 16.0f});
    base.setPosition({x + 2.0f, y + 5.0f});
    base.setFillColor(sf::Color(138, 66, 32));
    base.setOutlineColor(sf::Color(55, 30, 20));
    base.setOutlineThickness(1.0f);
    ventana.draw(base);

    sf::RectangleShape sabana({15.0f, 14.0f});
    sabana.setPosition({x + 6.0f, y + 6.0f});
    sabana.setFillColor(sf::Color(188, 36, 42));
    ventana.draw(sabana);

    sf::RectangleShape brillo({15.0f, 2.0f});
    brillo.setPosition({x + 6.0f, y + 6.0f});
    brillo.setFillColor(sf::Color(232, 82, 82, 150));
    ventana.draw(brillo);

    sf::RectangleShape almohada({5.0f, 14.0f});
    almohada.setPosition({x + 2.0f, y + 6.0f});
    almohada.setFillColor(sf::Color(226, 226, 214));
    ventana.draw(almohada);
}

inline TipoBloque revelarSueloRaro() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> suerte(1, 1000);
    int valor = suerte(gen);

    if (valor <= 3) return TipoBloque::MineralOro;
    if (valor <= 10) return TipoBloque::MineralPlata;
    return TipoBloque::Tierra;
}

inline Bloque crearBloqueRevelado(TipoBioma bioma) {
    TipoBloque revelado = revelarSueloRaro();
    if (revelado == TipoBloque::MineralOro) {
        return {TipoBloque::MineralOro, true, 450.0f, false, 0.0f, false, 1, 450.0f, bioma, 0};
    }
    if (revelado == TipoBloque::MineralPlata) {
        return {TipoBloque::MineralPlata, true, 420.0f, false, 0.0f, false, 1, 420.0f, bioma, 0};
    }
    return {TipoBloque::Tierra, false, 30.0f, false, 0.0f, false, 1, 30.0f, bioma, 0};
}

inline unsigned int semillaAleatoriaMundo() {
    std::random_device rd;
    std::mt19937 gen(rd());
    return gen();
}
}

inline Mundo::Mundo(int ancho, int alto) : Mundo(ancho, alto, semillaAleatoriaMundo()) {}

inline Mundo::Mundo(int ancho, int alto, unsigned int semilla) : ancho(ancho), alto(alto), semillaBase(semilla) {
    cuadricula.resize(alto);
    for (int i = 0; i < alto; ++i) {
        cuadricula[i].resize(ancho);
    }

    generarMundo(false);
    std::cout << "Matriz del mundo de " << ancho << "x" << alto << " creada con exito." << std::endl;
}

inline Mundo::~Mundo() {}

inline void Mundo::generarMundo(bool esSubterraneo) {
    const int MARGEN_OCEANO = 15;
    std::mt19937 gen(semillaBase + (esSubterraneo ? 50021u : 0u));
    unsigned int semillaBiomas = semillaBase;

    for (int y = 0; y < alto; ++y) {
        for (int x = 0; x < ancho; ++x) {
            TipoBioma bioma = calcularBioma(x, y, semillaBiomas);
            if (esSubterraneo) {
                cuadricula[y][x] = {TipoBloque::Piedra, true, 300.0f, false, 0.0f, false, 1, 300.0f, bioma, 0};
            } else if (x < MARGEN_OCEANO || x >= (ancho - MARGEN_OCEANO) ||
                       y < MARGEN_OCEANO || y >= (alto - MARGEN_OCEANO)) {
                cuadricula[y][x] = {TipoBloque::AguaProfunda, true, 9999.0f, false, 0.0f, false, 1, 9999.0f, bioma, 0};
            } else {
                cuadricula[y][x] = {TipoBloque::Pasto, false, 30.0f, false, 0.0f, false, 1, 30.0f, bioma, 0};
            }
        }
    }

    if (esSubterraneo) {
        auto ponerBloque = [&](int bx, int by, TipoBloque tipo, bool solido, float vida) {
            if (bx < 2 || bx >= ancho - 2 || by < 2 || by >= alto - 2) return;
            TipoBioma bioma = cuadricula[by][bx].bioma;
            cuadricula[by][bx] = {tipo, solido, vida, false, 0.0f, false, 1, vida, bioma, 0};
        };

        auto excavarCirculo = [&](int cx, int cy, int radio) {
            for (int dy = -radio; dy <= radio; ++dy) {
                for (int dx = -radio; dx <= radio; ++dx) {
                    int nx = cx + dx;
                    int ny = cy + dy;
                    if (nx < 2 || nx >= ancho - 2 || ny < 2 || ny >= alto - 2) continue;
                    float distancia = std::sqrt(static_cast<float>(dx * dx + dy * dy));
                    float bordeIrregular = 0.35f + hash01(nx, ny, semillaBase + 8803u) * 0.85f;
                    if (distancia <= static_cast<float>(radio) - bordeIrregular) {
                        ponerBloque(nx, ny, TipoBloque::CuevaSuelo, false, 30.0f);
                    }
                }
            }
        };

        int centroX = ancho / 2;
        int centroY = alto / 2;
        excavarCirculo(centroX, centroY, 9);

        std::uniform_int_distribution<> giro(-1, 1);
        std::uniform_int_distribution<> paso(0, 3);
        std::uniform_int_distribution<> radioTunel(2, 3);
        std::uniform_int_distribution<> salaRadio(5, 11);

        for (int ruta = 0; ruta < 26; ++ruta) {
            int x = centroX;
            int y = centroY;
            float angulo = (static_cast<float>(ruta) / 26.0f) * 6.28318f;
            int dx = static_cast<int>(std::round(std::cos(angulo)));
            int dy = static_cast<int>(std::round(std::sin(angulo)));
            if (dx == 0 && dy == 0) dx = 1;

            int largo = 85 + static_cast<int>(gen() % 130);
            for (int i = 0; i < largo; ++i) {
                if (paso(gen) == 0) {
                    int g = giro(gen);
                    if (std::abs(dx) > std::abs(dy)) {
                        dy = std::clamp(dy + g, -1, 1);
                    } else {
                        dx = std::clamp(dx + g, -1, 1);
                    }
                    if (dx == 0 && dy == 0) dx = 1;
                }

                x = std::clamp(x + dx, 12, ancho - 13);
                y = std::clamp(y + dy, 12, alto - 13);
                excavarCirculo(x, y, radioTunel(gen));
                if (i % 28 == 0 || (gen() % 100) < 4) {
                    excavarCirculo(x, y, salaRadio(gen));
                }
            }
        }

        auto esSueloCueva = [&](int bx, int by) {
            if (bx < 0 || bx >= ancho || by < 0 || by >= alto) return false;
            TipoBloque t = cuadricula[by][bx].tipo;
            return t == TipoBloque::CuevaSuelo || t == TipoBloque::CuevaEntrada || t == TipoBloque::Antorcha;
        };
        auto juntoACamino = [&](int bx, int by) {
            return esSueloCueva(bx + 1, by) || esSueloCueva(bx - 1, by) ||
                   esSueloCueva(bx, by + 1) || esSueloCueva(bx, by - 1);
        };

        for (int y = 3; y < alto - 3; ++y) {
            for (int x = 3; x < ancho - 3; ++x) {
                if (cuadricula[y][x].tipo != TipoBloque::Piedra || !juntoACamino(x, y)) continue;
                float r = hash01(x, y, semillaBase + 54021u);
                float profundidad = static_cast<float>(y) / static_cast<float>(alto);
                if (r < 0.070f) ponerBloque(x, y, TipoBloque::MineralCarbon, true, 360.0f);
                else if (r < 0.104f) ponerBloque(x, y, TipoBloque::MineralHierro, true, 450.0f);
                else if (r < 0.116f && profundidad > 0.22f) ponerBloque(x, y, TipoBloque::MineralPlata, true, 420.0f);
                else if (r < 0.124f && profundidad > 0.36f) ponerBloque(x, y, TipoBloque::MineralOro, true, 450.0f);
                else if (r < 0.129f && profundidad > 0.58f) ponerBloque(x, y, TipoBloque::MineralDiamante, true, 600.0f);
            }
        }

        for (int i = 0; i < 18; ++i) {
            float a = static_cast<float>(i) / 18.0f * 6.28318f;
            int tx = centroX + static_cast<int>(std::round(std::cos(a) * 6.0f));
            int ty = centroY + static_cast<int>(std::round(std::sin(a) * 6.0f));
            if (esSueloCueva(tx, ty)) ponerBloque(tx, ty, TipoBloque::Antorcha, false, 20.0f);
        }
        std::cout << "Subterraneo generado con exito." << std::endl;
        return;
    }

    for (int y = MARGEN_OCEANO; y < alto - MARGEN_OCEANO; ++y) {
        for (int x = MARGEN_OCEANO; x < ancho - MARGEN_OCEANO; ++x) {
            if (cuadricula[y][x].tipo != TipoBloque::Pasto) {
                continue;
            }

            float rioA = std::abs(ruidoFractal(static_cast<float>(x), static_cast<float>(y), 0.0075f, semillaBiomas + 701u) - 0.5f);
            float rioB = std::abs(ruidoFractal(static_cast<float>(x + 240), static_cast<float>(y - 130), 0.0060f, semillaBiomas + 1601u) - 0.5f);
            bool esRio = rioA < 0.018f || (rioB < 0.012f && cuadricula[y][x].bioma != TipoBioma::Seco);
            if (esRio) {
                TipoBioma bioma = cuadricula[y][x].bioma;
                cuadricula[y][x] = {TipoBloque::Agua, false, 50.0f, false, 0.0f, false, 1, 50.0f, bioma, 0};
                continue;
            }

            float claroSeco = ruidoFractal(static_cast<float>(x), static_cast<float>(y), 0.025f, semillaBiomas + 911u);
            if (cuadricula[y][x].bioma == TipoBioma::Seco && claroSeco > 0.70f) {
                TipoBioma bioma = cuadricula[y][x].bioma;
                cuadricula[y][x] = {TipoBloque::Tierra, false, 30.0f, false, 0.0f, false, 1, 30.0f, bioma, 0};
            }
        }
    }

    std::uniform_int_distribution<> disX(MARGEN_OCEANO + 40, ancho - MARGEN_OCEANO - 40);
    std::uniform_int_distribution<> disY(MARGEN_OCEANO + 40, alto - MARGEN_OCEANO - 40);
    std::uniform_int_distribution<> disRadioBase(6, 15);

    for (int i = 0; i < 95; ++i) {
        int centroX = disX(gen);
        int centroY = disY(gen);
        int radioBase = disRadioBase(gen);
        float estiramientoX = 1.0f + static_cast<float>(gen() % 100) / 200.0f;
        float estiramientoY = 1.0f + static_cast<float>(gen() % 100) / 200.0f;

        for (int y = centroY - radioBase - 5; y <= centroY + radioBase + 5; ++y) {
            for (int x = centroX - radioBase - 5; x <= centroX + radioBase + 5; ++x) {
                if (x >= 0 && x < ancho && y >= 0 && y < alto) {
                    float dx = (x - centroX) / estiramientoX;
                    float dy = (y - centroY) / estiramientoY;
                    float forma = std::sqrt(dx * dx + dy * dy);
                    float bordeIrregular = ruidoFractal(static_cast<float>(x), static_cast<float>(y), 0.075f, semillaBiomas + 331u) * 2.5f;
                    if (forma <= radioBase + bordeIrregular &&
                        cuadricula[y][x].tipo == TipoBloque::Pasto) {
                        TipoBioma bioma = cuadricula[y][x].bioma;
                        cuadricula[y][x] = {TipoBloque::Agua, false, 50.0f, false, 0.0f, false, 1, 50.0f, bioma, 0};
                    }
                }
            }
        }
    }

    for (int y = MARGEN_OCEANO; y < alto - MARGEN_OCEANO; ++y) {
        for (int x = MARGEN_OCEANO; x < ancho - MARGEN_OCEANO; ++x) {
            if (cuadricula[y][x].tipo != TipoBloque::Pasto) {
                continue;
            }

            float roca = ruidoFractal(static_cast<float>(x), static_cast<float>(y), 0.020f, semillaBiomas + 503u);
            bool rocaMontana = cuadricula[y][x].bioma == TipoBioma::Montana && roca > 0.55f;
            bool rocaSeca = cuadricula[y][x].bioma == TipoBioma::Seco && roca > 0.78f;
            if (rocaMontana || rocaSeca) {
                TipoBioma bioma = cuadricula[y][x].bioma;
                cuadricula[y][x] = {TipoBloque::Piedra, true, 300.0f, false, 0.0f, false, 1, 300.0f, bioma, 0};
            }
        }
    }

    std::uniform_int_distribution<> disRadioPiedra(4, 10);
    for (int i = 0; i < 55; ++i) {
        int centroX = disX(gen);
        int centroY = disY(gen);
        int radio = disRadioPiedra(gen);

        for (int y = centroY - radio; y <= centroY + radio; ++y) {
            for (int x = centroX - radio; x <= centroX + radio; ++x) {
                if (x >= 0 && x < ancho && y >= 0 && y < alto) {
                    float dx = x - centroX;
                    float dy = y - centroY;
                    float irregular = ruidoFractal(static_cast<float>(x), static_cast<float>(y), 0.060f, semillaBiomas + 809u) * 2.0f;
                    if (std::sqrt(dx * dx + dy * dy) <= radio + irregular &&
                        cuadricula[y][x].tipo == TipoBloque::Pasto) {
                        TipoBioma bioma = cuadricula[y][x].bioma;
                        cuadricula[y][x] = {TipoBloque::Piedra, true, 300.0f, false, 0.0f, false, 1, 300.0f, bioma, 0};
                    }
                }
            }
        }
    }

    auto colocarAldeaBloque = [&](int bx, int by, TipoBloque tipo) {
        if (bx < MARGEN_OCEANO + 4 || bx >= ancho - MARGEN_OCEANO - 4 ||
            by < MARGEN_OCEANO + 4 || by >= alto - MARGEN_OCEANO - 4) {
            return;
        }

        TipoBioma bioma = cuadricula[by][bx].bioma;
        bool solido = tipo == TipoBloque::Piedra ||
                      tipo == TipoBloque::Madera ||
                      tipo == TipoBloque::Cristal ||
                      tipo == TipoBloque::Horno ||
                      tipo == TipoBloque::Cofre ||
                      tipo == TipoBloque::Yunque ||
                      tipo == TipoBloque::PuertaCerrada ||
                      tipo == TipoBloque::Lava;
        float vida = static_cast<float>(getVidaMaximaBloque(tipo));
        cuadricula[by][bx] = {tipo, solido, vida, false, 0.0f, false, 1, vida, bioma, 0};
        if (esCultivoBloque(tipo)) {
            cuadricula[by][bx].estaHidratado = true;
            cuadricula[by][bx].faseCultivo = (bx * 31 + by * 17) % 4;
            cuadricula[by][bx].tiempoCrecimiento = 0.0f;
        }
        if (tipo == TipoBloque::TierraArada) {
            cuadricula[by][bx].estaHidratado = true;
        }
    };

    auto limpiarZonaAldea = [&](int cx, int cy, int radio) {
        for (int y = cy - radio; y <= cy + radio; ++y) {
            for (int x = cx - radio; x <= cx + radio; ++x) {
                if (x < MARGEN_OCEANO || x >= ancho - MARGEN_OCEANO ||
                    y < MARGEN_OCEANO || y >= alto - MARGEN_OCEANO) continue;
                TipoBioma bioma = cuadricula[y][x].bioma;
                cuadricula[y][x] = {TipoBloque::Pasto, false, 30.0f, false, 0.0f, false, 1, 30.0f, bioma, 0};
            }
        }
    };

    auto dibujarCaminoAldeaLogico = [&](int x1, int y1, int x2, int y2) {
        if (x1 == x2) {
            int ya = std::min(y1, y2);
            int yb = std::max(y1, y2);
            for (int y = ya; y <= yb; ++y) {
                for (int dx = -1; dx <= 1; ++dx) {
                    colocarAldeaBloque(x1 + dx, y, TipoBloque::CaminoAldea);
                }
            }
        } else {
            int xa = std::min(x1, x2);
            int xb = std::max(x1, x2);
            for (int x = xa; x <= xb; ++x) {
                for (int dy = -1; dy <= 1; ++dy) {
                    colocarAldeaBloque(x, y1 + dy, TipoBloque::CaminoAldea);
                }
            }
        }
    };

    auto construirCasaAldea = [&](int x0, int y0, int w, int h, bool piedra) {
        for (int y = y0; y < y0 + h; ++y) {
            for (int x = x0; x < x0 + w; ++x) {
                bool borde = x == x0 || x == x0 + w - 1 || y == y0 || y == y0 + h - 1;
                if (borde) {
                    colocarAldeaBloque(x, y, piedra ? TipoBloque::Piedra : TipoBloque::Madera);
                } else {
                    colocarAldeaBloque(x, y, TipoBloque::Techo);
                }
            }
        }
        int puertaX = x0 + w / 2;
        int puertaY = y0 + h - 1;
        colocarAldeaBloque(puertaX, puertaY, TipoBloque::PuertaCerrada);
        colocarAldeaBloque(x0 + 1, y0 + h / 2, TipoBloque::Cristal);
        colocarAldeaBloque(x0 + w - 2, y0 + h / 2, TipoBloque::Cristal);
        dibujarCaminoAldeaLogico(puertaX, puertaY + 1, puertaX, puertaY + 5);
    };

    auto construirGranjaAldea = [&](int x0, int y0, TipoBloque cultivo) {
        for (int y = y0; y < y0 + 7; ++y) {
            for (int x = x0; x < x0 + 11; ++x) {
                bool borde = x == x0 || x == x0 + 10 || y == y0 || y == y0 + 6;
                if (borde) {
                    colocarAldeaBloque(x, y, TipoBloque::Madera);
                } else if (x == x0 + 5) {
                    colocarAldeaBloque(x, y, TipoBloque::Agua);
                } else {
                    colocarAldeaBloque(x, y, TipoBloque::TierraArada);
                    colocarAldeaBloque(x, y, cultivo);
                }
            }
        }
    };

    auto construirHerreriaAldea = [&](int x0, int y0) {
        for (int y = y0; y < y0 + 9; ++y) {
            for (int x = x0; x < x0 + 12; ++x) {
                bool borde = x == x0 || x == x0 + 11 || y == y0 || y == y0 + 8;
                if (borde) {
                    colocarAldeaBloque(x, y, TipoBloque::Piedra);
                } else {
                    colocarAldeaBloque(x, y, TipoBloque::Techo);
                }
            }
        }
        colocarAldeaBloque(x0 + 5, y0 + 8, TipoBloque::PuertaCerrada);
        colocarAldeaBloque(x0 + 2, y0 + 2, TipoBloque::Lava);
        colocarAldeaBloque(x0 + 3, y0 + 2, TipoBloque::Piedra);
        colocarAldeaBloque(x0 + 8, y0 + 2, TipoBloque::Horno);
        colocarAldeaBloque(x0 + 9, y0 + 2, TipoBloque::Yunque);
        colocarAldeaBloque(x0 + 8, y0 + 6, TipoBloque::Cofre);
        colocarAldeaBloque(x0 + 1, y0 + 4, TipoBloque::Cristal);
        dibujarCaminoAldeaLogico(x0 + 5, y0 + 9, x0 + 5, y0 + 14);
    };

    std::uniform_int_distribution<> cantidadAldeas(2, 4);
    std::uniform_int_distribution<> aldeaX(MARGEN_OCEANO + 95, ancho - MARGEN_OCEANO - 95);
    std::uniform_int_distribution<> aldeaY(MARGEN_OCEANO + 95, alto - MARGEN_OCEANO - 95);
    int aldeasCreadas = 0;
    int aldeasObjetivo = cantidadAldeas(gen);
    std::vector<sf::Vector2i> centrosAldea;

    for (int intento = 0; intento < 240 && aldeasCreadas < aldeasObjetivo; ++intento) {
        int cx = aldeaX(gen);
        int cy = aldeaY(gen);
        bool lejos = true;
        for (const sf::Vector2i& c : centrosAldea) {
            int dx = c.x - cx;
            int dy = c.y - cy;
            if (dx * dx + dy * dy < 180 * 180) {
                lejos = false;
                break;
            }
        }
        if (!lejos) continue;

        bool zonaValida = true;
        for (int y = cy - 28; y <= cy + 28 && zonaValida; ++y) {
            for (int x = cx - 28; x <= cx + 28; ++x) {
                TipoBloque t = cuadricula[y][x].tipo;
                if (t == TipoBloque::Agua || t == TipoBloque::AguaProfunda) {
                    zonaValida = false;
                    break;
                }
            }
        }
        if (!zonaValida) continue;

        limpiarZonaAldea(cx, cy, 35);
        dibujarCaminoAldeaLogico(cx - 30, cy, cx + 30, cy);
        dibujarCaminoAldeaLogico(cx, cy - 26, cx, cy + 30);
        construirCasaAldea(cx - 24, cy - 18, 9, 8, false);
        construirCasaAldea(cx + 16, cy - 18, 10, 8, true);
        construirCasaAldea(cx - 26, cy + 12, 8, 7, false);
        construirGranjaAldea(cx + 12, cy + 10, (aldeasCreadas % 3 == 0) ? TipoBloque::CultivoTrigo : (aldeasCreadas % 3 == 1 ? TipoBloque::CultivoZanahoria : TipoBloque::CultivoPatata));
        construirHerreriaAldea(cx + 9, cy - 5);

        centrosAldea.push_back({cx, cy});
        ++aldeasCreadas;
    }

    std::uniform_int_distribution<> troncosArbol(2, 5);
    std::uniform_int_distribution<> varianteArbol(0, 2);
    auto hayArbolCerca = [&](int tx, int ty) {
        for (int yy = ty - 2; yy <= ty + 2; ++yy) {
            for (int xx = tx - 2; xx <= tx + 2; ++xx) {
                if (xx < 0 || xx >= ancho || yy < 0 || yy >= alto) {
                    continue;
                }
                if (cuadricula[yy][xx].tipo == TipoBloque::Madera) {
                    return true;
                }
            }
        }
        return false;
    };

    for (int i = 0; i < 16000; ++i) {
        int tx = disX(gen);
        int ty = disY(gen);

        if (cuadricula[ty][tx].tipo != TipoBloque::Pasto || hayArbolCerca(tx, ty)) {
            continue;
        }

        TipoBioma bioma = cuadricula[ty][tx].bioma;
        float bosqueLocal = ruidoFractal(static_cast<float>(tx), static_cast<float>(ty), 0.018f, semillaBiomas + 1201u);
        int probabilidad = 16;
        if (bioma == TipoBioma::Bosque) probabilidad = bosqueLocal > 0.42f ? 82 : 38;
        if (bioma == TipoBioma::Pradera) probabilidad = bosqueLocal > 0.68f ? 42 : 8;
        if (bioma == TipoBioma::Seco) probabilidad = bosqueLocal > 0.76f ? 18 : 3;
        if (bioma == TipoBioma::Montana) probabilidad = bosqueLocal > 0.62f ? 26 : 6;
        if (static_cast<int>(gen() % 100) >= probabilidad) {
            continue;
        }

        int troncos = troncosArbol(gen);
        float vida = 20.0f;
        cuadricula[ty][tx] = {TipoBloque::Madera, true, vida, false, 0.0f, false, troncos, vida, bioma, varianteArbol(gen)};
    }
    std::cout << "Superficie generada completamente." << std::endl;
}

inline void Mundo::dibujar(sf::RenderWindow& ventana) {
    const float TAMANIO_BLOQUE = TAMANIO_BLOQUE_JUEGO;
    sf::RectangleShape formaBlq({TAMANIO_BLOQUE, TAMANIO_BLOQUE});

    sf::View vistaActual = ventana.getView();
    sf::Vector2f centro = vistaActual.getCenter();
    sf::Vector2f tamanio = vistaActual.getSize();

    float izq = centro.x - (tamanio.x / 2.0f);
    float der = centro.x + (tamanio.x / 2.0f);
    float arriba = centro.y - (tamanio.y / 2.0f);
    float abajo = centro.y + (tamanio.y / 2.0f);

    int inicioX = std::max(0, static_cast<int>(izq / TAMANIO_BLOQUE) - 1);
    int finX = std::min(ancho, static_cast<int>(der / TAMANIO_BLOQUE) + 2);
    int inicioY = std::max(0, static_cast<int>(arriba / TAMANIO_BLOQUE) - 1);
    int finY = std::min(alto, static_cast<int>(abajo / TAMANIO_BLOQUE) + 2);

    auto esPiedraVisual = [&](int bx, int by) {
        if (bx < 0 || bx >= ancho || by < 0 || by >= alto) {
            return false;
        }
        TipoBloque tipo = cuadricula[by][bx].tipo;
        return tipo == TipoBloque::Piedra ||
               tipo == TipoBloque::MineralHierro ||
               tipo == TipoBloque::MineralCarbon ||
               tipo == TipoBloque::MineralPlata ||
               tipo == TipoBloque::MineralOro ||
               tipo == TipoBloque::MineralDiamante;
    };
    auto esMuroVisual = [&](int bx, int by) {
        if (bx < 0 || bx >= ancho || by < 0 || by >= alto) {
            return false;
        }
        const Bloque& bloque = cuadricula[by][bx];
        if (!bloque.esSolido) {
            return false;
        }
        return bloque.tipo == TipoBloque::Pasto ||
               bloque.tipo == TipoBloque::Tierra ||
               bloque.tipo == TipoBloque::Piedra ||
               bloque.tipo == TipoBloque::MineralHierro ||
               bloque.tipo == TipoBloque::MineralCarbon ||
               bloque.tipo == TipoBloque::MineralPlata ||
               bloque.tipo == TipoBloque::MineralOro ||
               bloque.tipo == TipoBloque::MineralDiamante ||
               bloque.tipo == TipoBloque::Horno ||
               bloque.tipo == TipoBloque::Cristal ||
               bloque.tipo == TipoBloque::MesaCrafteo ||
               bloque.tipo == TipoBloque::PuertaCerrada;
    };

    for (int y = inicioY; y < finY; ++y) {
        for (int x = inicioX; x < finX; ++x) {
            bool bloqueConAltura = false;
            if (cuadricula[y][x].tipo == TipoBloque::Pasto) {
                dibujarTextura16(ventana, x, y, true, false, cuadricula[y][x].bioma);
                if (cuadricula[y][x].esSolido) {
                    dibujarBloqueTierraElevado(
                        ventana, x, y,
                        esMuroVisual(x, y - 1), esMuroVisual(x, y + 1),
                        esMuroVisual(x - 1, y), esMuroVisual(x + 1, y),
                        cuadricula[y][x].bioma, true
                    );
                } else {
                    dibujarPlantasDecorativas(ventana, x, y, cuadricula[y][x].bioma);
                }
                continue;
            } else if (cuadricula[y][x].tipo == TipoBloque::Tierra) {
                dibujarTextura16(ventana, x, y, false, false, cuadricula[y][x].bioma);
                if (cuadricula[y][x].esSolido) {
                    dibujarBloqueTierraElevado(
                        ventana, x, y,
                        esMuroVisual(x, y - 1), esMuroVisual(x, y + 1),
                        esMuroVisual(x - 1, y), esMuroVisual(x + 1, y),
                        cuadricula[y][x].bioma
                    );
                } else {
                    dibujarTierraRebajada(ventana, x, y);
                }
                continue;
            } else if (cuadricula[y][x].tipo == TipoBloque::Agua) {
                dibujarAguaAnimada(ventana, x, y, false);
                continue;
            } else if (cuadricula[y][x].tipo == TipoBloque::AguaProfunda) {
                dibujarAguaAnimada(ventana, x, y, true);
                continue;
            } else if (cuadricula[y][x].tipo == TipoBloque::CuevaSuelo) {
                sf::RectangleShape suelo({TAMANIO_BLOQUE_JUEGO, TAMANIO_BLOQUE_JUEGO});
                suelo.setPosition({x * TAMANIO_BLOQUE_JUEGO, y * TAMANIO_BLOQUE_JUEGO});
                int ruido = static_cast<int>((x * 17 + y * 31 + x * y) % 5);
                suelo.setFillColor(ruido == 0 ? sf::Color(58, 55, 52) : (ruido == 1 ? sf::Color(74, 70, 64) : sf::Color(47, 45, 43)));
                ventana.draw(suelo);
                continue;
            } else if (cuadricula[y][x].tipo == TipoBloque::CaminoAldea) {
                dibujarCaminoAldea(ventana, x, y);
                continue;
            } else if (cuadricula[y][x].tipo == TipoBloque::CultivoTrigo ||
                       cuadricula[y][x].tipo == TipoBloque::CultivoZanahoria ||
                       cuadricula[y][x].tipo == TipoBloque::CultivoPatata) {
                dibujarCultivo(ventana, x, y, cuadricula[y][x].tipo, cuadricula[y][x].faseCultivo);
                continue;
            } else if (cuadricula[y][x].tipo == TipoBloque::Lava) {
                dibujarLava(ventana, x, y);
                continue;
            } else if (cuadricula[y][x].tipo == TipoBloque::Madera) {
                dibujarTextura16(ventana, x, y, true, false, cuadricula[y][x].bioma);
                dibujarPlantasDecorativas(ventana, x, y, cuadricula[y][x].bioma);
                continue;
            } else if (cuadricula[y][x].tipo == TipoBloque::Antorcha) {
                sf::RectangleShape suelo({TAMANIO_BLOQUE_JUEGO, TAMANIO_BLOQUE_JUEGO});
                suelo.setPosition({x * TAMANIO_BLOQUE_JUEGO, y * TAMANIO_BLOQUE_JUEGO});
                suelo.setFillColor(sf::Color(54, 51, 47));
                ventana.draw(suelo);
                sf::CircleShape luz(16.0f);
                luz.setOrigin({16.0f, 16.0f});
                luz.setPosition({x * TAMANIO_BLOQUE_JUEGO + 12.0f, y * TAMANIO_BLOQUE_JUEGO + 12.0f});
                luz.setFillColor(sf::Color(255, 172, 54, 55));
                ventana.draw(luz);
                sf::RectangleShape palo({4.0f, 16.0f});
                palo.setPosition({x * TAMANIO_BLOQUE_JUEGO + 10.0f, y * TAMANIO_BLOQUE_JUEGO + 6.0f});
                palo.setFillColor(sf::Color(126, 76, 30));
                ventana.draw(palo);
                sf::CircleShape llama(4.0f);
                llama.setOrigin({4.0f, 4.0f});
                llama.setPosition({x * TAMANIO_BLOQUE_JUEGO + 12.0f, y * TAMANIO_BLOQUE_JUEGO + 5.0f});
                llama.setFillColor(sf::Color(255, 224, 74));
                ventana.draw(llama);
                continue;
            } else if (cuadricula[y][x].tipo == TipoBloque::Techo) {
                sf::RectangleShape pisoInterior({TAMANIO_BLOQUE_JUEGO, TAMANIO_BLOQUE_JUEGO});
                pisoInterior.setPosition({x * TAMANIO_BLOQUE_JUEGO, y * TAMANIO_BLOQUE_JUEGO});
                pisoInterior.setFillColor(sf::Color(96, 72, 46));
                ventana.draw(pisoInterior);

                sf::RectangleShape veta({TAMANIO_BLOQUE_JUEGO, 1.0f});
                for (int i = 0; i < 4; ++i) {
                    veta.setPosition({x * TAMANIO_BLOQUE_JUEGO, y * TAMANIO_BLOQUE_JUEGO + 4.0f + i * 6.0f});
                    veta.setFillColor(i % 2 == 0 ? sf::Color(68, 48, 32) : sf::Color(132, 94, 56));
                    ventana.draw(veta);
                }
                continue;
            } else if (cuadricula[y][x].tipo == TipoBloque::PuertaCerrada ||
                       cuadricula[y][x].tipo == TipoBloque::PuertaAbierta) {
                dibujarPuerta(ventana, x, y, cuadricula[y][x].tipo == TipoBloque::PuertaAbierta);
                continue;
            } else if (cuadricula[y][x].tipo == TipoBloque::MesaCrafteo) {
                sf::RectangleShape sombraObjeto({TAMANIO_BLOQUE + 3.0f, 6.0f});
                sombraObjeto.setPosition({x * TAMANIO_BLOQUE + 2.0f, y * TAMANIO_BLOQUE + TAMANIO_BLOQUE - 2.0f});
                sombraObjeto.setFillColor(sf::Color(18, 16, 12, 85));
                ventana.draw(sombraObjeto);

                formaBlq.setFillColor(sf::Color(64, 38, 20));
                formaBlq.setPosition({x * TAMANIO_BLOQUE, y * TAMANIO_BLOQUE - 2.0f});
                ventana.draw(formaBlq);

                sf::RectangleShape centroMesa({TAMANIO_BLOQUE - 6.0f, TAMANIO_BLOQUE - 6.0f});
                centroMesa.setPosition({x * TAMANIO_BLOQUE + 3.0f, y * TAMANIO_BLOQUE + 1.0f});
                centroMesa.setFillColor(sf::Color(145, 92, 42));
                ventana.draw(centroMesa);

                sf::RectangleShape lineaVertical({1.0f, TAMANIO_BLOQUE - 6.0f});
                lineaVertical.setPosition({x * TAMANIO_BLOQUE + TAMANIO_BLOQUE * 0.5f, y * TAMANIO_BLOQUE + 1.0f});
                lineaVertical.setFillColor(sf::Color::Black);
                ventana.draw(lineaVertical);

                sf::RectangleShape lineaHorizontal({TAMANIO_BLOQUE - 6.0f, 1.0f});
                lineaHorizontal.setPosition({x * TAMANIO_BLOQUE + 3.0f, y * TAMANIO_BLOQUE + TAMANIO_BLOQUE * 0.5f - 2.0f});
                lineaHorizontal.setFillColor(sf::Color::Black);
                ventana.draw(lineaHorizontal);
                dibujarRelieveMuro(
                    ventana, x, y,
                    esMuroVisual(x, y - 1), esMuroVisual(x, y + 1),
                    esMuroVisual(x - 1, y), esMuroVisual(x + 1, y),
                    sf::Color(224, 170, 94, 95), sf::Color(58, 34, 19, 115)
                );
                continue;
            } else if (cuadricula[y][x].tipo == TipoBloque::Horno) {
                formaBlq.setFillColor(sf::Color(85, 85, 85));
                bloqueConAltura = true;
            } else if (cuadricula[y][x].tipo == TipoBloque::Cofre) {
                dibujarCofre(ventana, x, y);
                continue;
            } else if (cuadricula[y][x].tipo == TipoBloque::Yunque) {
                dibujarYunque(ventana, x, y);
                continue;
            } else if (cuadricula[y][x].tipo == TipoBloque::Cama) {
                dibujarCama(ventana, x, y);
                continue;
            } else if (cuadricula[y][x].tipo == TipoBloque::Cristal) {
                formaBlq.setFillColor(sf::Color(180, 235, 255, 180));
                bloqueConAltura = true;
            } else if (cuadricula[y][x].tipo == TipoBloque::TierraArada) {
                dibujarTextura16(ventana, x, y, false, true, cuadricula[y][x].bioma);
                if (cuadricula[y][x].estaHidratado) {
                    sf::RectangleShape humedad({TAMANIO_BLOQUE, TAMANIO_BLOQUE});
                    humedad.setPosition({x * TAMANIO_BLOQUE, y * TAMANIO_BLOQUE});
                    humedad.setFillColor(sf::Color(26, 42, 68, 55));
                    ventana.draw(humedad);
                }
                continue;
            } else if (cuadricula[y][x].tipo == TipoBloque::CuevaEntrada) {
                sf::RectangleShape baseEntrada({TAMANIO_BLOQUE, TAMANIO_BLOQUE});
                baseEntrada.setPosition({x * TAMANIO_BLOQUE, y * TAMANIO_BLOQUE});
                baseEntrada.setFillColor(cuadricula[y][x].minaAbierta ? sf::Color(38, 28, 48) : sf::Color(18, 18, 22));
                ventana.draw(baseEntrada);

                sf::RectangleShape sombraInterior({TAMANIO_BLOQUE - 6.0f, TAMANIO_BLOQUE - 6.0f});
                sombraInterior.setPosition({x * TAMANIO_BLOQUE + 3.0f, y * TAMANIO_BLOQUE + 3.0f});
                sombraInterior.setFillColor(sf::Color(8, 7, 12, 210));
                ventana.draw(sombraInterior);

                sf::RectangleShape bordeLuz({TAMANIO_BLOQUE - 4.0f, 2.0f});
                bordeLuz.setPosition({x * TAMANIO_BLOQUE + 2.0f, y * TAMANIO_BLOQUE + 2.0f});
                bordeLuz.setFillColor(sf::Color(96, 76, 104, 150));
                ventana.draw(bordeLuz);

                if (cuadricula[y][x].minaAbierta) {
                    sf::RectangleShape poste({2.0f, 16.0f});
                    poste.setFillColor(sf::Color(142, 90, 43));
                    poste.setPosition({x * TAMANIO_BLOQUE + 7.0f, y * TAMANIO_BLOQUE + 4.0f});
                    ventana.draw(poste);
                    poste.setPosition({x * TAMANIO_BLOQUE + 15.0f, y * TAMANIO_BLOQUE + 4.0f});
                    ventana.draw(poste);

                    sf::RectangleShape peldaño({12.0f, 2.0f});
                    peldaño.setFillColor(sf::Color(181, 121, 58));
                    for (int i = 0; i < 4; ++i) {
                        peldaño.setPosition({x * TAMANIO_BLOQUE + 6.0f, y * TAMANIO_BLOQUE + 6.0f + i * 4.0f});
                        ventana.draw(peldaño);
                    }
                }
                continue;
            } else if (cuadricula[y][x].tipo == TipoBloque::Piedra) {
                dibujarTextura16(ventana, x, y, true, false, cuadricula[y][x].bioma);
                dibujarTexturaPiedra(
                    ventana, x, y,
                    esPiedraVisual(x, y - 1),
                    esPiedraVisual(x, y + 1),
                    esPiedraVisual(x - 1, y),
                    esPiedraVisual(x + 1, y)
                );
                dibujarRelieveMuro(
                    ventana, x, y,
                    esMuroVisual(x, y - 1), esMuroVisual(x, y + 1),
                    esMuroVisual(x - 1, y), esMuroVisual(x + 1, y),
                    sf::Color(210, 214, 210, 90), sf::Color(50, 52, 53, 115)
                );
                continue;
            } else if (cuadricula[y][x].tipo == TipoBloque::MineralCarbon) {
                formaBlq.setFillColor(sf::Color(38, 38, 36));
            } else if (cuadricula[y][x].tipo == TipoBloque::MineralHierro) {
                formaBlq.setFillColor(sf::Color(210, 180, 140));
            } else if (cuadricula[y][x].tipo == TipoBloque::MineralPlata) {
                formaBlq.setFillColor(sf::Color(188, 196, 202));
            } else if (cuadricula[y][x].tipo == TipoBloque::MineralOro) {
                formaBlq.setFillColor(sf::Color(224, 180, 58));
            } else if (cuadricula[y][x].tipo == TipoBloque::MineralDiamante) {
                formaBlq.setFillColor(sf::Color(0, 255, 255));
            } else {
                formaBlq.setFillColor(sf::Color(0, 100, 0));
            }

            if (bloqueConAltura) {
                const float baseX = x * TAMANIO_BLOQUE;
                const float baseY = y * TAMANIO_BLOQUE;

                sf::RectangleShape sombraObjeto({TAMANIO_BLOQUE + 4.0f, 6.0f});
                sombraObjeto.setPosition({baseX + 2.0f, baseY + TAMANIO_BLOQUE - 1.0f});
                sombraObjeto.setFillColor(sf::Color(10, 10, 10, 85));
                ventana.draw(sombraObjeto);

                sf::RectangleShape caraFrontal({TAMANIO_BLOQUE, 7.0f});
                caraFrontal.setPosition({baseX, baseY + TAMANIO_BLOQUE - 6.0f});
                caraFrontal.setFillColor(sf::Color(0, 0, 0, 65));
                ventana.draw(caraFrontal);

                sf::RectangleShape caraDerecha({5.0f, TAMANIO_BLOQUE - 4.0f});
                caraDerecha.setPosition({baseX + TAMANIO_BLOQUE - 5.0f, baseY + 2.0f});
                caraDerecha.setFillColor(sf::Color(0, 0, 0, 42));
                ventana.draw(caraDerecha);
            }

            formaBlq.setPosition({x * TAMANIO_BLOQUE, y * TAMANIO_BLOQUE});
            ventana.draw(formaBlq);

            if (bloqueConAltura) {
                sf::RectangleShape brilloSup({TAMANIO_BLOQUE, 2.0f});
                brilloSup.setPosition({x * TAMANIO_BLOQUE, y * TAMANIO_BLOQUE});
                brilloSup.setFillColor(sf::Color(255, 255, 255, 45));
                ventana.draw(brilloSup);

                sf::RectangleShape sombraInf({TAMANIO_BLOQUE, 2.0f});
                sombraInf.setPosition({x * TAMANIO_BLOQUE, y * TAMANIO_BLOQUE + TAMANIO_BLOQUE - 2.0f});
                sombraInf.setFillColor(sf::Color(0, 0, 0, 65));
                ventana.draw(sombraInf);
            }

            if (esMuroVisual(x, y)) {
                sf::Color luz(230, 232, 230, 70);
                sf::Color sombra(42, 42, 42, 105);
                if (cuadricula[y][x].tipo == TipoBloque::Cristal) {
                    luz = sf::Color(235, 255, 255, 105);
                    sombra = sf::Color(42, 86, 100, 85);
                } else if (cuadricula[y][x].tipo == TipoBloque::MineralOro) {
                    luz = sf::Color(255, 230, 120, 90);
                    sombra = sf::Color(120, 82, 22, 115);
                } else if (cuadricula[y][x].tipo == TipoBloque::MineralPlata) {
                    luz = sf::Color(245, 250, 250, 90);
                    sombra = sf::Color(76, 84, 88, 105);
                } else if (cuadricula[y][x].tipo == TipoBloque::MineralCarbon) {
                    luz = sf::Color(150, 150, 140, 70);
                    sombra = sf::Color(10, 10, 10, 130);
                }
                dibujarRelieveMuro(
                    ventana, x, y,
                    esMuroVisual(x, y - 1), esMuroVisual(x, y + 1),
                    esMuroVisual(x - 1, y), esMuroVisual(x + 1, y),
                    luz, sombra
                );
            }
        }
    }

    for (int y = inicioY; y < finY; ++y) {
        for (int x = inicioX; x < finX; ++x) {
            if (cuadricula[y][x].tipo == TipoBloque::Madera) {
                dibujarArbol(ventana, x, y, cuadricula[y][x], true, false);
            }
        }
    }

}

inline void Mundo::dibujarCapaSuperior(sf::RenderWindow& ventana, sf::Vector2f posicionJugador) {
    sf::View vistaActual = ventana.getView();
    sf::Vector2f centro = vistaActual.getCenter();
    sf::Vector2f tamanio = vistaActual.getSize();

    float izq = centro.x - (tamanio.x / 2.0f);
    float der = centro.x + (tamanio.x / 2.0f);
    float arriba = centro.y - (tamanio.y / 2.0f);
    float abajo = centro.y + (tamanio.y / 2.0f);

    int inicioX = std::max(0, static_cast<int>(izq / TAMANIO_BLOQUE_JUEGO) - 4);
    int finX = std::min(ancho, static_cast<int>(der / TAMANIO_BLOQUE_JUEGO) + 5);
    int inicioY = std::max(0, static_cast<int>(arriba / TAMANIO_BLOQUE_JUEGO) - 4);
    int finY = std::min(alto, static_cast<int>(abajo / TAMANIO_BLOQUE_JUEGO) + 5);
    int anchoLocal = std::max(0, finX - inicioX);
    int altoLocal = std::max(0, finY - inicioY);
    std::vector<unsigned char> techoInterior(static_cast<std::size_t>(anchoLocal * altoLocal), 0);

    auto indiceLocal = [&](int bx, int by) {
        return (by - inicioY) * anchoLocal + (bx - inicioX);
    };

    if (posicionJugador.x > -90000.0f && anchoLocal > 0 && altoLocal > 0) {
        sf::Vector2f centroJugador = posicionJugador + sf::Vector2f(12.0f, 12.0f);
        int jugadorX = static_cast<int>(std::floor(centroJugador.x / TAMANIO_BLOQUE_JUEGO));
        int jugadorY = static_cast<int>(std::floor(centroJugador.y / TAMANIO_BLOQUE_JUEGO));

        if (jugadorX >= inicioX && jugadorX < finX &&
            jugadorY >= inicioY && jugadorY < finY &&
            esInteriorTechadoEn(jugadorX, jugadorY)) {
            std::vector<sf::Vector2i> pila;
            pila.push_back({jugadorX, jugadorY});
            techoInterior[static_cast<std::size_t>(indiceLocal(jugadorX, jugadorY))] = 1;

            while (!pila.empty()) {
                sf::Vector2i actual = pila.back();
                pila.pop_back();

                const sf::Vector2i vecinos[4] = {
                    {actual.x + 1, actual.y},
                    {actual.x - 1, actual.y},
                    {actual.x, actual.y + 1},
                    {actual.x, actual.y - 1}
                };

                for (const sf::Vector2i& v : vecinos) {
                    if (v.x < inicioX || v.x >= finX || v.y < inicioY || v.y >= finY) {
                        continue;
                    }
                    if (cuadricula[v.y][v.x].tipo != TipoBloque::Techo) {
                        continue;
                    }

                    std::size_t idx = static_cast<std::size_t>(indiceLocal(v.x, v.y));
                    if (techoInterior[idx]) {
                        continue;
                    }
                    techoInterior[idx] = 1;
                    pila.push_back(v);
                }
            }
        }
    }

    for (int y = inicioY; y < finY; ++y) {
        for (int x = inicioX; x < finX; ++x) {
            if (cuadricula[y][x].tipo == TipoBloque::Techo) {
                std::uint8_t alpha = 238;
                if (anchoLocal > 0 && techoInterior[static_cast<std::size_t>(indiceLocal(x, y))]) {
                    alpha = 38;
                }
                dibujarTecho(ventana, x, y, alpha);
            }
        }
    }

    for (int y = inicioY; y < finY; ++y) {
        for (int x = inicioX; x < finX; ++x) {
            if (cuadricula[y][x].tipo != TipoBloque::Madera) {
                continue;
            }
            const float baseX = x * TAMANIO_BLOQUE_JUEGO - 20.0f;
            const float baseY = y * TAMANIO_BLOQUE_JUEGO - 46.0f;
            const sf::Vector2f centroJugador = posicionJugador + sf::Vector2f(12.0f, 14.0f);
            const sf::FloatRect zonaCopa({baseX - 4.0f, baseY - 4.0f}, {80.0f, 78.0f});
            const bool cercaJugador = posicionJugador.x > -90000.0f && zonaCopa.contains(centroJugador);

            dibujarArbol(ventana, x, y, cuadricula[y][x], false, true, cercaJugador ? 118 : 255);
        }
    }
}

inline int Mundo::getVidaMaximaBloque(TipoBloque tipo) const {
    switch (tipo) {
        case TipoBloque::Pasto: return 30;
        case TipoBloque::Madera: return 90;
        case TipoBloque::MesaCrafteo: return 90;
        case TipoBloque::Horno: return 300;
        case TipoBloque::Cristal: return 30;
        case TipoBloque::TierraArada: return 30;
        case TipoBloque::CuevaEntrada: return 9999;
        case TipoBloque::Antorcha: return 20;
        case TipoBloque::CuevaSuelo: return 30;
        case TipoBloque::Techo: return 70;
        case TipoBloque::CaminoAldea: return 25;
        case TipoBloque::CultivoTrigo:
        case TipoBloque::CultivoZanahoria:
        case TipoBloque::CultivoPatata:
            return 15;
        case TipoBloque::Lava: return 9999;
        case TipoBloque::Cofre: return 80;
        case TipoBloque::Yunque: return 320;
        case TipoBloque::Cama: return 60;
        case TipoBloque::PuertaCerrada:
        case TipoBloque::PuertaAbierta:
            return 80;
        case TipoBloque::MineralCarbon: return 360;
        case TipoBloque::Piedra: return 300;
        case TipoBloque::MineralHierro: return 450;
        case TipoBloque::MineralPlata: return 420;
        case TipoBloque::MineralDiamante: return 600;
        default: return 50;
    }
}

inline bool Mundo::esBloqueSolido(int x, int y) const {
    if (x < 0 || x >= ancho || y < 0 || y >= alto) {
        return true;
    }
    return cuadricula[y][x].esSolido;
}

inline bool Mundo::puedeColocarBloque(int x, int y) const {
    if (x < 0 || x >= ancho || y < 0 || y >= alto) {
        return false;
    }

    TipoBloque tipo = cuadricula[y][x].tipo;
    return tipo == TipoBloque::Aire ||
           tipo == TipoBloque::Pasto ||
           tipo == TipoBloque::Tierra ||
           tipo == TipoBloque::TierraArada ||
           tipo == TipoBloque::CuevaSuelo;
}

inline bool Mundo::colocarBloque(int x, int y, TipoBloque tipo) {
    if (!puedeColocarBloque(x, y) || tipo == TipoBloque::Aire) {
        return false;
    }

    if (esCultivoBloque(tipo)) {
        return sembrarCultivo(x, y, tipo);
    }

    bool solido = tipo != TipoBloque::Antorcha &&
                  tipo != TipoBloque::Techo &&
                  tipo != TipoBloque::PuertaAbierta &&
                  tipo != TipoBloque::CaminoAldea &&
                  tipo != TipoBloque::CultivoTrigo &&
                  tipo != TipoBloque::CultivoZanahoria &&
                  tipo != TipoBloque::CultivoPatata &&
                  tipo != TipoBloque::Cama;
    float vida = static_cast<float>(getVidaMaximaBloque(tipo));
    TipoBioma bioma = cuadricula[y][x].bioma;
    cuadricula[y][x] = {tipo, solido, vida, false, 0.0f, false, 1, vida, bioma, 0};
    if (tipo == TipoBloque::TierraArada) {
        cuadricula[y][x].estaHidratado = false;
    }
    return true;
}

inline bool Mundo::alternarPuerta(int x, int y) {
    if (x < 0 || x >= ancho || y < 0 || y >= alto) {
        return false;
    }

    Bloque& bloque = cuadricula[y][x];
    if (bloque.tipo == TipoBloque::PuertaCerrada) {
        bloque.tipo = TipoBloque::PuertaAbierta;
        bloque.esSolido = false;
        return true;
    }

    if (bloque.tipo == TipoBloque::PuertaAbierta) {
        bloque.tipo = TipoBloque::PuertaCerrada;
        bloque.esSolido = true;
        return true;
    }

    return false;
}

inline bool Mundo::esInteriorTechadoEn(int x, int y) const {
    if (x < 0 || x >= ancho || y < 0 || y >= alto) {
        return false;
    }
    if (cuadricula[y][x].tipo != TipoBloque::Techo) {
        return false;
    }

    auto esMuroPerimetral = [&](TipoBloque tipo) {
        return tipo == TipoBloque::Pasto ||
               tipo == TipoBloque::Tierra ||
               tipo == TipoBloque::Piedra ||
               tipo == TipoBloque::Madera ||
               tipo == TipoBloque::MesaCrafteo ||
               tipo == TipoBloque::Horno ||
               tipo == TipoBloque::Cristal ||
               tipo == TipoBloque::PuertaCerrada ||
               tipo == TipoBloque::PuertaAbierta ||
               tipo == TipoBloque::MineralCarbon ||
               tipo == TipoBloque::MineralHierro ||
               tipo == TipoBloque::MineralPlata ||
               tipo == TipoBloque::MineralOro ||
               tipo == TipoBloque::MineralDiamante;
    };

    constexpr int RADIO_REVISION = 18;
    int minX = std::max(0, x - RADIO_REVISION);
    int maxX = std::min(ancho - 1, x + RADIO_REVISION);
    int minY = std::max(0, y - RADIO_REVISION);
    int maxY = std::min(alto - 1, y + RADIO_REVISION);
    int anchoLocal = maxX - minX + 1;
    int altoLocal = maxY - minY + 1;

    std::vector<unsigned char> visitado(static_cast<std::size_t>(anchoLocal * altoLocal), 0);
    std::vector<sf::Vector2i> pila;
    auto indice = [&](int bx, int by) {
        return (by - minY) * anchoLocal + (bx - minX);
    };

    pila.push_back({x, y});
    visitado[static_cast<std::size_t>(indice(x, y))] = 1;

    while (!pila.empty()) {
        sf::Vector2i actual = pila.back();
        pila.pop_back();

        const sf::Vector2i vecinos[4] = {
            {actual.x + 1, actual.y},
            {actual.x - 1, actual.y},
            {actual.x, actual.y + 1},
            {actual.x, actual.y - 1}
        };

        for (const sf::Vector2i& v : vecinos) {
            if (v.x < 0 || v.x >= ancho || v.y < 0 || v.y >= alto) {
                return false;
            }

            TipoBloque vecino = cuadricula[v.y][v.x].tipo;
            if (vecino == TipoBloque::Techo) {
                if (v.x < minX || v.x > maxX || v.y < minY || v.y > maxY) {
                    return false;
                }

                std::size_t idx = static_cast<std::size_t>(indice(v.x, v.y));
                if (!visitado[idx]) {
                    visitado[idx] = 1;
                    pila.push_back(v);
                }
                continue;
            }

            if (!esMuroPerimetral(vecino)) {
                return false;
            }
        }
    }

    return true;
}

inline void Mundo::actualizarCultivos(float dt) {
    if (ancho <= 0 || alto <= 0) {
        return;
    }

    acumuladorCultivos += dt;
    if (acumuladorCultivos < 1.0f) {
        return;
    }

    float pasoTiempo = acumuladorCultivos;
    acumuladorCultivos = 0.0f;
    int filasPorPaso = std::max(1, alto / 20);

    auto hayAguaCerca = [&](int cx, int cy) {
        for (int yy = cy - 4; yy <= cy + 4; ++yy) {
            for (int xx = cx - 4; xx <= cx + 4; ++xx) {
                if (xx < 0 || xx >= ancho || yy < 0 || yy >= alto) continue;
                TipoBloque tipo = cuadricula[yy][xx].tipo;
                if (tipo == TipoBloque::Agua || tipo == TipoBloque::AguaProfunda) {
                    return true;
                }
            }
        }
        return false;
    };

    for (int i = 0; i < filasPorPaso; ++i) {
        int y = (filaActualizacionCultivos + i) % alto;
        for (int x = 0; x < ancho; ++x) {
            Bloque& bloque = cuadricula[y][x];
            if (bloque.tipo != TipoBloque::TierraArada && !esCultivoBloque(bloque.tipo)) {
                continue;
            }

            bloque.estaHidratado = hayAguaCerca(x, y);
            if (esCultivoBloque(bloque.tipo) && bloque.faseCultivo < 3) {
                float tiempoNecesario = bloque.estaHidratado ? 45.0f : 90.0f;
                bloque.tiempoCrecimiento += pasoTiempo * static_cast<float>(20) / static_cast<float>(filasPorPaso);
                while (bloque.faseCultivo < 3 && bloque.tiempoCrecimiento >= tiempoNecesario) {
                    bloque.tiempoCrecimiento -= tiempoNecesario;
                    ++bloque.faseCultivo;
                }
            }
        }
    }

    filaActualizacionCultivos = (filaActualizacionCultivos + filasPorPaso) % alto;
}

inline bool Mundo::ararTierra(int x, int y) {
    if (x < 0 || x >= ancho || y < 0 || y >= alto) {
        return false;
    }

    TipoBloque tipo = cuadricula[y][x].tipo;
    if (tipo != TipoBloque::Pasto && tipo != TipoBloque::Tierra) {
        return false;
    }

    TipoBioma bioma = cuadricula[y][x].bioma;
    bool hidratado = false;
    for (int yy = y - 4; yy <= y + 4 && !hidratado; ++yy) {
        for (int xx = x - 4; xx <= x + 4; ++xx) {
            if (xx < 0 || xx >= ancho || yy < 0 || yy >= alto) continue;
            TipoBloque vecino = cuadricula[yy][xx].tipo;
            if (vecino == TipoBloque::Agua || vecino == TipoBloque::AguaProfunda) {
                hidratado = true;
                break;
            }
        }
    }
    cuadricula[y][x] = {TipoBloque::TierraArada, false, 30.0f, hidratado, 0.0f, false, 1, 30.0f, bioma, 0};
    return true;
}

inline bool Mundo::sembrarCultivo(int x, int y, TipoBloque cultivo) {
    if (x < 0 || x >= ancho || y < 0 || y >= alto || !esCultivoBloque(cultivo)) {
        return false;
    }

    if (cuadricula[y][x].tipo != TipoBloque::TierraArada) {
        return false;
    }

    TipoBioma bioma = cuadricula[y][x].bioma;
    bool hidratado = cuadricula[y][x].estaHidratado;
    float vida = static_cast<float>(getVidaMaximaBloque(cultivo));
    cuadricula[y][x] = {cultivo, false, vida, hidratado, 0.0f, false, 1, vida, bioma, 0};
    cuadricula[y][x].faseCultivo = 0;
    cuadricula[y][x].tiempoCrecimiento = 0.0f;
    return true;
}

inline int Mundo::getFaseCultivo(int x, int y) const {
    if (x < 0 || x >= ancho || y < 0 || y >= alto || !esCultivoBloque(cuadricula[y][x].tipo)) {
        return 0;
    }
    return cuadricula[y][x].faseCultivo;
}

inline bool Mundo::estaCultivoMaduro(int x, int y) const {
    if (x < 0 || x >= ancho || y < 0 || y >= alto || !esCultivoBloque(cuadricula[y][x].tipo)) {
        return false;
    }
    return cuadricula[y][x].faseCultivo >= 3;
}

inline bool Mundo::crearEntradaMina(int x, int y) {
    if (x < 0 || x >= ancho || y < 0 || y >= alto) {
        return false;
    }

    TipoBloque tipo = cuadricula[y][x].tipo;
    if (tipo != TipoBloque::Tierra) {
        return false;
    }

    TipoBioma bioma = cuadricula[y][x].bioma;
    cuadricula[y][x] = {TipoBloque::CuevaEntrada, false, 9999.0f, false, TIEMPO_MINA_BASE_SEGUNDOS, false, 1, 9999.0f, bioma, 0};
    return true;
}

inline void Mundo::crearZonaEntradaSubterranea(int x, int y) {
    if (x < 2 || x >= ancho - 2 || y < 2 || y >= alto - 2) {
        return;
    }

    for (int by = y - 7; by <= y + 7; ++by) {
        for (int bx = x - 7; bx <= x + 7; ++bx) {
            if (bx < 0 || bx >= ancho || by < 0 || by >= alto) continue;

            int dx = bx - x;
            int dy = by - y;
            float distancia = std::sqrt(static_cast<float>(dx * dx + dy * dy));
            TipoBioma bioma = cuadricula[by][bx].bioma;

            if (distancia <= 5.7f) {
                cuadricula[by][bx] = {TipoBloque::CuevaSuelo, false, 30.0f, false, 0.0f, false, 1, 30.0f, bioma, 0};
            } else if (distancia <= 7.2f) {
                cuadricula[by][bx] = {TipoBloque::Piedra, true, 300.0f, false, 0.0f, false, 1, 300.0f, bioma, 0};
            }
        }
    }

    TipoBioma bioma = cuadricula[y][x].bioma;
    cuadricula[y][x] = {TipoBloque::CuevaEntrada, false, 9999.0f, false, 0.0f, true, 1, 9999.0f, bioma, 0};
    if (x + 2 < ancho) cuadricula[y][x + 2] = {TipoBloque::Antorcha, false, 20.0f, false, 0.0f, false, 1, 20.0f, bioma, 0};
    if (x - 2 >= 0) cuadricula[y][x - 2] = {TipoBloque::Antorcha, false, 20.0f, false, 0.0f, false, 1, 20.0f, bioma, 0};
}

inline bool Mundo::talarArbol(int x, int y, float segundosTrabajo, int& troncosObtenidos, bool& soltoSemilla) {
    troncosObtenidos = 0;
    soltoSemilla = false;

    if (x < 0 || x >= ancho || y < 0 || y >= alto || segundosTrabajo <= 0.0f) {
        return false;
    }

    Bloque& bloque = cuadricula[y][x];
    if (bloque.tipo != TipoBloque::Madera) {
        return false;
    }

    bloque.vida = std::max(0.0f, bloque.vida - segundosTrabajo);
    if (bloque.vida > 0.0f) {
        return false;
    }

    troncosObtenidos = std::max(2, bloque.troncosAlTalar);

    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> semilla(1, 100);
    soltoSemilla = semilla(gen) <= 35;

    TipoBioma bioma = bloque.bioma;
    cuadricula[y][x] = {TipoBloque::Pasto, false, 30.0f, false, 0.0f, false, 1, 30.0f, bioma, 0};
    return true;
}

inline float Mundo::getProgresoTala(int x, int y) const {
    if (x < 0 || x >= ancho || y < 0 || y >= alto) {
        return 0.0f;
    }

    const Bloque& bloque = cuadricula[y][x];
    if (bloque.tipo != TipoBloque::Madera || bloque.vidaMaxima <= 0.0f) {
        return 0.0f;
    }

    return 1.0f - (bloque.vida / bloque.vidaMaxima);
}

inline float Mundo::getProgresoBloque(int x, int y) const {
    if (x < 0 || x >= ancho || y < 0 || y >= alto) {
        return 0.0f;
    }

    const Bloque& bloque = cuadricula[y][x];
    if (bloque.tipo == TipoBloque::Aire || bloque.vidaMaxima <= 0.0f || bloque.vida >= bloque.vidaMaxima) {
        return 0.0f;
    }

    return std::clamp(1.0f - (bloque.vida / bloque.vidaMaxima), 0.0f, 1.0f);
}

inline bool Mundo::picarEntradaMina(int x, int y, float segundosTrabajo) {
    if (x < 0 || x >= ancho || y < 0 || y >= alto || segundosTrabajo <= 0.0f) {
        return false;
    }

    Bloque& bloque = cuadricula[y][x];
    if (bloque.tipo != TipoBloque::CuevaEntrada || bloque.minaAbierta) {
        return false;
    }

    if (bloque.tiempoMinaRestante > TIEMPO_MINA_BASE_SEGUNDOS) {
        bloque.tiempoMinaRestante = TIEMPO_MINA_BASE_SEGUNDOS;
    }
    bloque.tiempoMinaRestante = std::max(0.0f, bloque.tiempoMinaRestante - segundosTrabajo);
    if (bloque.tiempoMinaRestante <= 0.0f) {
        bloque.minaAbierta = true;
        bloque.tiempoMinaRestante = 0.0f;
    }
    return true;
}

inline bool Mundo::esMinaAbierta(int x, int y) const {
    if (x < 0 || x >= ancho || y < 0 || y >= alto) {
        return false;
    }
    return cuadricula[y][x].tipo == TipoBloque::CuevaEntrada && cuadricula[y][x].minaAbierta;
}

inline float Mundo::getTiempoMinaRestante(int x, int y) const {
    if (x < 0 || x >= ancho || y < 0 || y >= alto) {
        return 0.0f;
    }
    if (cuadricula[y][x].tipo != TipoBloque::CuevaEntrada) {
        return 0.0f;
    }
    return cuadricula[y][x].tiempoMinaRestante;
}

inline float Mundo::getProgresoMina(int x, int y) const {
    if (x < 0 || x >= ancho || y < 0 || y >= alto) {
        return 0.0f;
    }
    if (cuadricula[y][x].tipo != TipoBloque::CuevaEntrada) {
        return 0.0f;
    }
    float restante = std::min(cuadricula[y][x].tiempoMinaRestante, TIEMPO_MINA_BASE_SEGUNDOS);
    return std::clamp(1.0f - (restante / TIEMPO_MINA_BASE_SEGUNDOS), 0.0f, 1.0f);
}

inline TipoBloque Mundo::getTipoBloque(int x, int y) const {
    if (x < 0 || x >= ancho || y < 0 || y >= alto) return TipoBloque::Piedra;
    return cuadricula[y][x].tipo;
}

inline sf::Color Mundo::getColorMapa(int x, int y) const {
    if (x < 0 || x >= ancho || y < 0 || y >= alto) {
        return sf::Color(18, 38, 92);
    }

    const Bloque& bloque = cuadricula[y][x];
    switch (bloque.tipo) {
        case TipoBloque::Agua:
            return sf::Color(36, 124, 210);
        case TipoBloque::AguaProfunda:
            return sf::Color(18, 54, 145);
        case TipoBloque::CuevaSuelo:
            return sf::Color(58, 55, 52);
        case TipoBloque::Antorcha:
            return sf::Color(235, 170, 50);
        case TipoBloque::Piedra:
        case TipoBloque::MineralCarbon:
        case TipoBloque::MineralHierro:
        case TipoBloque::MineralPlata:
        case TipoBloque::MineralOro:
        case TipoBloque::MineralDiamante:
        case TipoBloque::Redstone:
            return sf::Color(128, 128, 118);
        case TipoBloque::Madera:
            return sf::Color(24, 92, 42);
        case TipoBloque::Tierra:
        case TipoBloque::TierraArada:
            return sf::Color(122, 82, 45);
        case TipoBloque::MesaCrafteo:
        case TipoBloque::Horno:
            return sf::Color(116, 78, 52);
        case TipoBloque::Cristal:
            return sf::Color(165, 226, 230);
        case TipoBloque::Techo:
            return sf::Color(82, 82, 104);
        case TipoBloque::PuertaCerrada:
        case TipoBloque::PuertaAbierta:
            return sf::Color(140, 86, 38);
        case TipoBloque::CaminoAldea:
            return sf::Color(145, 110, 68);
        case TipoBloque::CultivoTrigo:
            return sf::Color(205, 178, 65);
        case TipoBloque::CultivoZanahoria:
            return sf::Color(212, 110, 45);
        case TipoBloque::CultivoPatata:
            return sf::Color(176, 132, 72);
        case TipoBloque::Lava:
            return sf::Color(238, 72, 22);
        case TipoBloque::Cofre:
            return sf::Color(166, 104, 38);
        case TipoBloque::Yunque:
            return sf::Color(82, 86, 90);
        default:
            switch (bloque.bioma) {
                case TipoBioma::Bosque: return sf::Color(43, 118, 57);
                case TipoBioma::Seco: return sf::Color(174, 166, 80);
                case TipoBioma::Montana: return sf::Color(106, 139, 118);
                default: return sf::Color(77, 166, 71);
            }
    }
}

inline void Mundo::romperBloque(int x, int y) {
    if (x >= 0 && x < ancho && y >= 0 && y < alto) {
        cuadricula[y][x] = {TipoBloque::Aire, false, 0.0f, false};
    }
}

inline bool Mundo::daniarBloque(int x, int y, float cantidadDanio) {
    if (x < 0 || x >= ancho || y < 0 || y >= alto) return false;
    if (cuadricula[y][x].tipo == TipoBloque::Aire) return false;
    if (cuadricula[y][x].tipo == TipoBloque::CuevaEntrada) return false;

    Bloque& bloque = cuadricula[y][x];
    TipoBloque tipoOriginal = bloque.tipo;
    TipoBioma bioma = bloque.bioma;

    bloque.vida -= cantidadDanio;
    if (bloque.vida <= 0.0f) {
        if (tipoOriginal == TipoBloque::Pasto || tipoOriginal == TipoBloque::Piedra) {
            cuadricula[y][x] = crearBloqueRevelado(bioma);
        } else if (tipoOriginal == TipoBloque::Tierra || tipoOriginal == TipoBloque::TierraArada) {
            cuadricula[y][x] = {TipoBloque::Tierra, false, 30.0f, false, 0.0f, false, 1, 30.0f, bioma, 0};
        } else if (tipoOriginal == TipoBloque::MineralPlata || tipoOriginal == TipoBloque::MineralOro) {
            cuadricula[y][x] = {TipoBloque::Tierra, false, 30.0f, false, 0.0f, false, 1, 30.0f, bioma, 0};
        } else if (tipoOriginal == TipoBloque::MineralCarbon ||
                   tipoOriginal == TipoBloque::MineralHierro ||
                   tipoOriginal == TipoBloque::MineralDiamante ||
                   tipoOriginal == TipoBloque::Antorcha) {
            cuadricula[y][x] = {TipoBloque::CuevaSuelo, false, 30.0f, false, 0.0f, false, 1, 30.0f, bioma, 0};
        } else if (tipoOriginal == TipoBloque::Techo ||
                   tipoOriginal == TipoBloque::PuertaCerrada ||
                   tipoOriginal == TipoBloque::PuertaAbierta ||
                   tipoOriginal == TipoBloque::Cofre ||
                   tipoOriginal == TipoBloque::Yunque ||
                   tipoOriginal == TipoBloque::Cama) {
            cuadricula[y][x] = {TipoBloque::Pasto, false, 30.0f, false, 0.0f, false, 1, 30.0f, bioma, 0};
        } else if (tipoOriginal == TipoBloque::CaminoAldea) {
            cuadricula[y][x] = {TipoBloque::Tierra, false, 30.0f, false, 0.0f, false, 1, 30.0f, bioma, 0};
        } else if (tipoOriginal == TipoBloque::CultivoTrigo ||
                   tipoOriginal == TipoBloque::CultivoZanahoria ||
                   tipoOriginal == TipoBloque::CultivoPatata) {
            bool hidratado = bloque.estaHidratado;
            cuadricula[y][x] = {TipoBloque::TierraArada, false, 30.0f, hidratado, 0.0f, false, 1, 30.0f, bioma, 0};
        } else if (tipoOriginal == TipoBloque::Lava) {
            cuadricula[y][x] = {TipoBloque::Piedra, true, 300.0f, false, 0.0f, false, 1, 300.0f, bioma, 0};
        } else {
            cuadricula[y][x] = {TipoBloque::Aire, false, 0.0f, false, 0.0f, false, 1, 0.0f, bioma, 0};
        }
        return true;
    }
    return false;
}

inline int Mundo::getAncho() const {
    return ancho;
}

inline int Mundo::getAlto() const {
    return alto;
}

inline unsigned int Mundo::getSemilla() const {
    return semillaBase;
}

inline bool Mundo::guardarEstado(const std::string& ruta) const {
    std::ofstream out(ruta, std::ios::binary);
    if (!out) {
        return false;
    }

    const char firma[4] = {'M', '2', 'D', 'W'};
    int version = 2;
    out.write(firma, sizeof(firma));
    out.write(reinterpret_cast<const char*>(&version), sizeof(version));
    out.write(reinterpret_cast<const char*>(&ancho), sizeof(ancho));
    out.write(reinterpret_cast<const char*>(&alto), sizeof(alto));
    out.write(reinterpret_cast<const char*>(&semillaBase), sizeof(semillaBase));

    for (int y = 0; y < alto; ++y) {
        for (int x = 0; x < ancho; ++x) {
            const Bloque& b = cuadricula[y][x];
            int tipo = static_cast<int>(b.tipo);
            int bioma = static_cast<int>(b.bioma);
            out.write(reinterpret_cast<const char*>(&tipo), sizeof(tipo));
            out.write(reinterpret_cast<const char*>(&b.esSolido), sizeof(b.esSolido));
            out.write(reinterpret_cast<const char*>(&b.vida), sizeof(b.vida));
            out.write(reinterpret_cast<const char*>(&b.estaHidratado), sizeof(b.estaHidratado));
            out.write(reinterpret_cast<const char*>(&b.tiempoMinaRestante), sizeof(b.tiempoMinaRestante));
            out.write(reinterpret_cast<const char*>(&b.minaAbierta), sizeof(b.minaAbierta));
            out.write(reinterpret_cast<const char*>(&b.troncosAlTalar), sizeof(b.troncosAlTalar));
            out.write(reinterpret_cast<const char*>(&b.vidaMaxima), sizeof(b.vidaMaxima));
            out.write(reinterpret_cast<const char*>(&bioma), sizeof(bioma));
            out.write(reinterpret_cast<const char*>(&b.varianteArbol), sizeof(b.varianteArbol));
            out.write(reinterpret_cast<const char*>(&b.faseCultivo), sizeof(b.faseCultivo));
            out.write(reinterpret_cast<const char*>(&b.tiempoCrecimiento), sizeof(b.tiempoCrecimiento));
        }
    }

    return static_cast<bool>(out);
}

inline bool Mundo::cargarEstado(const std::string& ruta) {
    std::ifstream in(ruta, std::ios::binary);
    if (!in) {
        return false;
    }

    char firma[4] = {};
    int version = 0;
    int anchoArchivo = 0;
    int altoArchivo = 0;
    unsigned int semillaArchivo = 0;
    in.read(firma, sizeof(firma));
    in.read(reinterpret_cast<char*>(&version), sizeof(version));
    in.read(reinterpret_cast<char*>(&anchoArchivo), sizeof(anchoArchivo));
    in.read(reinterpret_cast<char*>(&altoArchivo), sizeof(altoArchivo));
    in.read(reinterpret_cast<char*>(&semillaArchivo), sizeof(semillaArchivo));

    if (!in || firma[0] != 'M' || firma[1] != '2' || firma[2] != 'D' || firma[3] != 'W' ||
        (version != 1 && version != 2) || anchoArchivo != ancho || altoArchivo != alto) {
        return false;
    }

    semillaBase = semillaArchivo;
    for (int y = 0; y < alto; ++y) {
        for (int x = 0; x < ancho; ++x) {
            Bloque b;
            int tipo = 0;
            int bioma = 0;
            in.read(reinterpret_cast<char*>(&tipo), sizeof(tipo));
            in.read(reinterpret_cast<char*>(&b.esSolido), sizeof(b.esSolido));
            in.read(reinterpret_cast<char*>(&b.vida), sizeof(b.vida));
            in.read(reinterpret_cast<char*>(&b.estaHidratado), sizeof(b.estaHidratado));
            in.read(reinterpret_cast<char*>(&b.tiempoMinaRestante), sizeof(b.tiempoMinaRestante));
            in.read(reinterpret_cast<char*>(&b.minaAbierta), sizeof(b.minaAbierta));
            in.read(reinterpret_cast<char*>(&b.troncosAlTalar), sizeof(b.troncosAlTalar));
            in.read(reinterpret_cast<char*>(&b.vidaMaxima), sizeof(b.vidaMaxima));
            in.read(reinterpret_cast<char*>(&bioma), sizeof(bioma));
            in.read(reinterpret_cast<char*>(&b.varianteArbol), sizeof(b.varianteArbol));
            if (version >= 2) {
                in.read(reinterpret_cast<char*>(&b.faseCultivo), sizeof(b.faseCultivo));
                in.read(reinterpret_cast<char*>(&b.tiempoCrecimiento), sizeof(b.tiempoCrecimiento));
            }
            if (!in) {
                return false;
            }
            b.tipo = static_cast<TipoBloque>(tipo);
            b.bioma = static_cast<TipoBioma>(bioma);
            if (esCultivoBloque(b.tipo)) {
                b.esSolido = false;
                b.faseCultivo = std::clamp(b.faseCultivo, 0, 3);
                b.vidaMaxima = static_cast<float>(getVidaMaximaBloque(b.tipo));
                if (b.vida <= 0.0f) b.vida = b.vidaMaxima;
            }
            cuadricula[y][x] = b;
        }
    }

    return true;
}

