#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <random>

namespace {
constexpr float TIEMPO_MINA_BASE_SEGUNDOS = 900.0f;

inline TipoBioma calcularBioma(int x, int y, unsigned int semilla) {
    float sx = static_cast<float>(x) + static_cast<float>(semilla % 4096);
    float sy = static_cast<float>(y) + static_cast<float>((semilla / 7) % 4096);
    float valor = std::sin(sx * 0.018f) +
                  std::sin(sy * 0.015f) +
                  std::sin((sx + sy) * 0.008f) +
                  std::sin((sx - sy) * 0.012f);

    if (valor < -1.05f) return TipoBioma::Seco;
    if (valor < 0.35f) return TipoBioma::Pradera;
    if (valor < 1.65f) return TipoBioma::Bosque;
    return TipoBioma::Montana;
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
    sprite.setPosition({bloqueX * 32.0f, bloqueY * 32.0f});
    ventana.draw(sprite);
}

inline void dibujarTextura16Lenta(sf::RenderWindow& ventana, int bloqueX, int bloqueY, bool pasto, bool arada = false) {
    constexpr float pixel = 2.0f;
    sf::RectangleShape cuadro({pixel, pixel});
    for (int py = 0; py < 16; ++py) {
        for (int px = 0; px < 16; ++px) {
            cuadro.setPosition({bloqueX * 32.0f + px * pixel, bloqueY * 32.0f + py * pixel});
            cuadro.setFillColor(pasto ? colorPastoPixel(px, py, TipoBioma::Pradera) : colorTierraPixel(px, py, arada));
            ventana.draw(cuadro);
        }
    }
}

inline void dibujarArbol(sf::RenderWindow& ventana, int bloqueX, int bloqueY, const Bloque& bloque) {
    const float baseX = bloqueX * 32.0f - 20.0f;
    const float baseY = bloqueY * 32.0f - 46.0f;
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

    sf::RectangleShape sombra({44.0f, 11.0f});
    sombra.setPosition({baseX + 14.0f, baseY + 70.0f});
    sombra.setFillColor(sf::Color(18, 35, 22, 95));
    ventana.draw(sombra);

    auto rect = [&](float x, float y, float w, float h, sf::Color color) {
        sf::RectangleShape r({w, h});
        r.setPosition({baseX + x, baseY + y});
        r.setFillColor(color);
        ventana.draw(r);
    };

    rect(30.0f, 43.0f, 12.0f, 35.0f, sf::Color(92, 54, 31));
    rect(34.0f, 44.0f, 4.0f, 30.0f, sf::Color(143, 85, 43));
    rect(28.0f, 62.0f, 5.0f, 9.0f, sf::Color(70, 41, 25));
    rect(39.0f, 58.0f, 5.0f, 12.0f, sf::Color(70, 41, 25));

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

    std::uniform_int_distribution<> disX(MARGEN_OCEANO + 40, ancho - MARGEN_OCEANO - 40);
    std::uniform_int_distribution<> disY(MARGEN_OCEANO + 40, alto - MARGEN_OCEANO - 40);
    std::uniform_int_distribution<> disRadioBase(7, 12);

    for (int i = 0; i < 150; ++i) {
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
                    if (std::sqrt(dx * dx + dy * dy) <= radioBase &&
                        cuadricula[y][x].tipo == TipoBloque::Pasto) {
                        TipoBioma bioma = cuadricula[y][x].bioma;
                        cuadricula[y][x] = {TipoBloque::Agua, false, 50.0f, false, 0.0f, false, 1, 50.0f, bioma, 0};
                    }
                }
            }
        }
    }

    std::uniform_int_distribution<> disRadioPiedra(5, 9);
    for (int i = 0; i < 80; ++i) {
        int centroX = disX(gen);
        int centroY = disY(gen);
        int radio = disRadioPiedra(gen);

        for (int y = centroY - radio; y <= centroY + radio; ++y) {
            for (int x = centroX - radio; x <= centroX + radio; ++x) {
                if (x >= 0 && x < ancho && y >= 0 && y < alto) {
                    float dx = x - centroX;
                    float dy = y - centroY;
                    if (std::sqrt(dx * dx + dy * dy) <= radio &&
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
    for (int i = 0; i < 5200; ++i) {
        int tx = disX(gen);
        int ty = disY(gen);

        if (cuadricula[ty][tx].tipo == TipoBloque::Pasto) {
            TipoBioma bioma = cuadricula[ty][tx].bioma;
            int probabilidad = 86;
            if (bioma == TipoBioma::Bosque) probabilidad = 99;
            if (bioma == TipoBioma::Seco) probabilidad = 55;
            if (bioma == TipoBioma::Montana) probabilidad = 76;
            if (static_cast<int>(gen() % 100) >= probabilidad) {
                continue;
            }

            int troncos = troncosArbol(gen);
            float vida = 20.0f;
            cuadricula[ty][tx] = {TipoBloque::Madera, true, vida, false, 0.0f, false, troncos, vida, bioma, varianteArbol(gen)};
        }
    }
    std::cout << "Superficie generada completamente." << std::endl;
}

inline void Mundo::dibujar(sf::RenderWindow& ventana) {
    const float TAMANIO_BLOQUE = 32.0f;
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

    for (int y = inicioY; y < finY; ++y) {
        for (int x = inicioX; x < finX; ++x) {
            if (cuadricula[y][x].tipo == TipoBloque::Pasto) {
                dibujarTextura16(ventana, x, y, true, false, cuadricula[y][x].bioma);
                continue;
            } else if (cuadricula[y][x].tipo == TipoBloque::Tierra) {
                dibujarTextura16(ventana, x, y, false, false, cuadricula[y][x].bioma);
                continue;
            } else if (cuadricula[y][x].tipo == TipoBloque::Agua) {
                formaBlq.setFillColor(sf::Color(30, 144, 255));
            } else if (cuadricula[y][x].tipo == TipoBloque::AguaProfunda) {
                formaBlq.setFillColor(sf::Color(0, 0, 139));
            } else if (cuadricula[y][x].tipo == TipoBloque::Madera) {
                dibujarTextura16(ventana, x, y, true, false, cuadricula[y][x].bioma);
                continue;
            } else if (cuadricula[y][x].tipo == TipoBloque::MesaCrafteo) {
                formaBlq.setFillColor(sf::Color(64, 38, 20));
                formaBlq.setPosition({x * TAMANIO_BLOQUE, y * TAMANIO_BLOQUE});
                ventana.draw(formaBlq);

                sf::RectangleShape centroMesa({24.0f, 24.0f});
                centroMesa.setPosition({x * TAMANIO_BLOQUE + 4.0f, y * TAMANIO_BLOQUE + 4.0f});
                centroMesa.setFillColor(sf::Color(145, 92, 42));
                ventana.draw(centroMesa);

                sf::RectangleShape lineaVertical({1.0f, 24.0f});
                lineaVertical.setPosition({x * TAMANIO_BLOQUE + 16.0f, y * TAMANIO_BLOQUE + 4.0f});
                lineaVertical.setFillColor(sf::Color::Black);
                ventana.draw(lineaVertical);

                sf::RectangleShape lineaHorizontal({24.0f, 1.0f});
                lineaHorizontal.setPosition({x * TAMANIO_BLOQUE + 4.0f, y * TAMANIO_BLOQUE + 16.0f});
                lineaHorizontal.setFillColor(sf::Color::Black);
                ventana.draw(lineaHorizontal);
                continue;
            } else if (cuadricula[y][x].tipo == TipoBloque::Horno) {
                formaBlq.setFillColor(sf::Color(85, 85, 85));
            } else if (cuadricula[y][x].tipo == TipoBloque::Cristal) {
                formaBlq.setFillColor(sf::Color(180, 235, 255, 180));
            } else if (cuadricula[y][x].tipo == TipoBloque::TierraArada) {
                dibujarTextura16(ventana, x, y, false, true, cuadricula[y][x].bioma);
                continue;
            } else if (cuadricula[y][x].tipo == TipoBloque::CuevaEntrada) {
                formaBlq.setFillColor(cuadricula[y][x].minaAbierta ? sf::Color(45, 25, 65) : sf::Color(18, 18, 22));
            } else if (cuadricula[y][x].tipo == TipoBloque::Piedra) {
                formaBlq.setFillColor(sf::Color(128, 128, 128));
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

            formaBlq.setPosition({x * TAMANIO_BLOQUE, y * TAMANIO_BLOQUE});
            ventana.draw(formaBlq);
        }
    }

    for (int y = inicioY; y < finY; ++y) {
        for (int x = inicioX; x < finX; ++x) {
            if (cuadricula[y][x].tipo == TipoBloque::Madera) {
                dibujarArbol(ventana, x, y, cuadricula[y][x]);
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

