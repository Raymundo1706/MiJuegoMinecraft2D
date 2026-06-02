#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <random>

namespace {
constexpr float TIEMPO_MINA_BASE_SEGUNDOS = 900.0f;

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
    int ruido = (x * 13 + y * 7 + x * y) % 11;
    if (arada) {
        if (ruido < 2) return sf::Color(62, 38, 24);
        if (ruido > 8) return sf::Color(122, 75, 42);
        return sf::Color(89, 54, 31);
    }
    if (ruido < 2) return sf::Color(82, 52, 32);
    if (ruido > 8) return sf::Color(151, 94, 53);
    return sf::Color(118, 75, 43);
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

inline void dibujarArbol(sf::RenderWindow& ventana, int bloqueX, int bloqueY, const Bloque& bloque,
                         bool dibujarBase, bool dibujarCopa) {
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
        rect(18.0f, variante == 1 ? 2.0f : 7.0f, 36.0f, 7.0f, hojaOscura);
        rect(variante == 2 ? 7.0f : 10.0f, 14.0f, variante == 2 ? 58.0f : 52.0f, 9.0f, sf::Color(43, 125, 61));
        rect(4.0f, 23.0f, 64.0f, 12.0f, sf::Color(50, 149, 69));
        rect(variante == 1 ? 4.0f : 0.0f, 35.0f, variante == 1 ? 64.0f : 72.0f, 12.0f, hojaBase);
        rect(7.0f, 47.0f, 58.0f, 11.0f, sf::Color(38, 111, 55));
        rect(17.0f, 58.0f, 38.0f, 8.0f, sf::Color(30, 88, 46));

        rect(24.0f, variante == 1 ? 5.0f : 10.0f, 12.0f, 6.0f, hojaClara);
        rect(42.0f, 20.0f, 12.0f, 7.0f, sf::Color(73, 166, 73));
        rect(13.0f, 28.0f, 10.0f, 7.0f, sf::Color(75, 169, 75));
        rect(26.0f, 35.0f, 9.0f, 8.0f, sf::Color(30, 88, 46));
        rect(52.0f, 37.0f, 8.0f, 7.0f, sf::Color(31, 95, 48));
        rect(13.0f, 48.0f, 9.0f, 6.0f, sf::Color(29, 82, 44));

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
}

inline Mundo::Mundo(int ancho, int alto) : ancho(ancho), alto(alto) {
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
    std::random_device rd;
    std::mt19937 gen(rd());
    unsigned int semillaBiomas = gen();

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
        std::uniform_int_distribution<> disX(10, ancho - 10);

        for (int i = 0; i < 800; ++i) {
            int centroX = disX(gen);
            std::uniform_int_distribution<> disY(10, alto - 10);
            int centroY = disY(gen);

            for (int dy = 0; dy <= 1; ++dy) {
                for (int dx = 0; dx <= 1; ++dx) {
                    int nx = centroX + dx;
                    int ny = centroY + dy;
                    if (nx >= 0 && nx < ancho && ny >= 0 && ny < alto) {
                        cuadricula[ny][nx] = {TipoBloque::MineralHierro, true, 450.0f, false};
                    }
                }
            }
        }

        for (int i = 0; i < 250; ++i) {
            int centroX = disX(gen);
            std::uniform_int_distribution<> disYProfundo(alto / 2, alto - 10);
            int centroY = disYProfundo(gen);

            if (centroX >= 0 && centroX < ancho && centroY >= 0 && centroY < alto) {
                cuadricula[centroY][centroX] = {TipoBloque::MineralDiamante, true, 600.0f, false};
                if (centroX + 1 < ancho && (gen() % 2 == 0)) {
                    cuadricula[centroY][centroX + 1] = {TipoBloque::MineralDiamante, true, 600.0f, false};
                }
            }
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
               tipo == TipoBloque::MineralPlata ||
               tipo == TipoBloque::MineralOro ||
               tipo == TipoBloque::MineralDiamante;
    };

    for (int y = inicioY; y < finY; ++y) {
        for (int x = inicioX; x < finX; ++x) {
            bool bloqueConAltura = false;
            if (cuadricula[y][x].tipo == TipoBloque::Pasto) {
                dibujarTextura16(ventana, x, y, true, false, cuadricula[y][x].bioma);
                dibujarPlantasDecorativas(ventana, x, y, cuadricula[y][x].bioma);
                continue;
            } else if (cuadricula[y][x].tipo == TipoBloque::Tierra) {
                dibujarTextura16(ventana, x, y, false, false, cuadricula[y][x].bioma);
                continue;
            } else if (cuadricula[y][x].tipo == TipoBloque::Agua) {
                dibujarAguaAnimada(ventana, x, y, false);
                continue;
            } else if (cuadricula[y][x].tipo == TipoBloque::AguaProfunda) {
                dibujarAguaAnimada(ventana, x, y, true);
                continue;
            } else if (cuadricula[y][x].tipo == TipoBloque::Madera) {
                dibujarTextura16(ventana, x, y, true, false, cuadricula[y][x].bioma);
                dibujarPlantasDecorativas(ventana, x, y, cuadricula[y][x].bioma);
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
                continue;
            } else if (cuadricula[y][x].tipo == TipoBloque::Horno) {
                formaBlq.setFillColor(sf::Color(85, 85, 85));
                bloqueConAltura = true;
            } else if (cuadricula[y][x].tipo == TipoBloque::Cristal) {
                formaBlq.setFillColor(sf::Color(180, 235, 255, 180));
                bloqueConAltura = true;
            } else if (cuadricula[y][x].tipo == TipoBloque::TierraArada) {
                dibujarTextura16(ventana, x, y, false, true, cuadricula[y][x].bioma);
                continue;
            } else if (cuadricula[y][x].tipo == TipoBloque::CuevaEntrada) {
                formaBlq.setFillColor(cuadricula[y][x].minaAbierta ? sf::Color(45, 25, 65) : sf::Color(18, 18, 22));
            } else if (cuadricula[y][x].tipo == TipoBloque::Piedra) {
                dibujarTextura16(ventana, x, y, true, false, cuadricula[y][x].bioma);
                dibujarTexturaPiedra(
                    ventana, x, y,
                    esPiedraVisual(x, y - 1),
                    esPiedraVisual(x, y + 1),
                    esPiedraVisual(x - 1, y),
                    esPiedraVisual(x + 1, y)
                );
                continue;
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
                sf::RectangleShape sombraObjeto({TAMANIO_BLOQUE + 2.0f, 5.0f});
                sombraObjeto.setPosition({x * TAMANIO_BLOQUE + 2.0f, y * TAMANIO_BLOQUE + TAMANIO_BLOQUE - 1.0f});
                sombraObjeto.setFillColor(sf::Color(18, 18, 18, 75));
                ventana.draw(sombraObjeto);
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
        }
    }

    for (int y = inicioY; y < finY; ++y) {
        for (int x = inicioX; x < finX; ++x) {
            if (cuadricula[y][x].tipo == TipoBloque::Madera) {
                dibujarArbol(ventana, x, y, cuadricula[y][x], true, false);
            }
        }
    }

    for (int y = inicioY; y < finY; ++y) {
        for (int x = inicioX; x < finX; ++x) {
            if (cuadricula[y][x].tipo == TipoBloque::Madera) {
                dibujarArbol(ventana, x, y, cuadricula[y][x], false, true);
            }
        }
    }
}

inline void Mundo::dibujarArbolesSobreJugador(sf::RenderWindow& ventana, float piesJugadorY) {
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

    for (int y = inicioY; y < finY; ++y) {
        for (int x = inicioX; x < finX; ++x) {
            if (cuadricula[y][x].tipo != TipoBloque::Madera) {
                continue;
            }

            float profundidadArbol = y * TAMANIO_BLOQUE_JUEGO + TAMANIO_BLOQUE_JUEGO * 0.8f;
            if (piesJugadorY < profundidadArbol) {
                dibujarArbol(ventana, x, y, cuadricula[y][x], false, true);
            }
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
           tipo == TipoBloque::TierraArada;
}

inline bool Mundo::colocarBloque(int x, int y, TipoBloque tipo) {
    if (!puedeColocarBloque(x, y) || tipo == TipoBloque::Aire) {
        return false;
    }

    bool solido = true;
    float vida = static_cast<float>(getVidaMaximaBloque(tipo));
    TipoBioma bioma = cuadricula[y][x].bioma;
    cuadricula[y][x] = {tipo, solido, vida, false, 0.0f, false, 1, vida, bioma, 0};
    return true;
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
    cuadricula[y][x] = {TipoBloque::TierraArada, false, 30.0f, false, 0.0f, false, 1, 30.0f, bioma, 0};
    return true;
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

inline bool Mundo::picarEntradaMina(int x, int y, float segundosTrabajo) {
    if (x < 0 || x >= ancho || y < 0 || y >= alto || segundosTrabajo <= 0.0f) {
        return false;
    }

    Bloque& bloque = cuadricula[y][x];
    if (bloque.tipo != TipoBloque::CuevaEntrada || bloque.minaAbierta) {
        return false;
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
    return 1.0f - (cuadricula[y][x].tiempoMinaRestante / TIEMPO_MINA_BASE_SEGUNDOS);
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
        case TipoBloque::Piedra:
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

