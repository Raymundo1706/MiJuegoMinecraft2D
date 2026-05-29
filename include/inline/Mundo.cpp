#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>

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

    for (int y = 0; y < alto; ++y) {
        for (int x = 0; x < ancho; ++x) {
            if (esSubterraneo) {
                cuadricula[y][x] = {TipoBloque::Piedra, true, 300.0f, false};
            } else if (x < MARGEN_OCEANO || x >= (ancho - MARGEN_OCEANO) ||
                       y < MARGEN_OCEANO || y >= (alto - MARGEN_OCEANO)) {
                cuadricula[y][x] = {TipoBloque::AguaProfunda, true, 9999.0f, false};
            } else {
                cuadricula[y][x] = {TipoBloque::Pasto, false, 30.0f, false};
            }
        }
    }

    std::random_device rd;
    std::mt19937 gen(rd());

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
                        cuadricula[y][x] = {TipoBloque::Agua, false, 50.0f, false};
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
                        int suerte = gen() % 100;
                        if (suerte < 5) {
                            cuadricula[y][x] = {TipoBloque::MineralHierro, true, 450.0f, false};
                        } else if (suerte == 99) {
                            cuadricula[y][x] = {TipoBloque::MineralDiamante, true, 600.0f, false};
                        } else {
                            cuadricula[y][x] = {TipoBloque::Piedra, true, 300.0f, false};
                        }
                    }
                }
            }
        }
    }

    for (int i = 0; i < 600; ++i) {
        int tx = disX(gen);
        int ty = disY(gen);

        if (cuadricula[ty][tx].tipo == TipoBloque::Pasto) {
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int nx = tx + dx;
                    int ny = ty + dy;
                    if (std::abs(dx) + std::abs(dy) <= 1 &&
                        nx >= 0 && nx < ancho && ny >= 0 && ny < alto &&
                        cuadricula[ny][nx].tipo == TipoBloque::Pasto) {
                        cuadricula[ny][nx] = {TipoBloque::Madera, true, 90.0f, false};
                    }
                }
            }
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
                formaBlq.setFillColor((x + y) % 2 == 0 ? sf::Color(34, 139, 34) : sf::Color(46, 139, 87));
            } else if (cuadricula[y][x].tipo == TipoBloque::Agua) {
                formaBlq.setFillColor(sf::Color(30, 144, 255));
            } else if (cuadricula[y][x].tipo == TipoBloque::AguaProfunda) {
                formaBlq.setFillColor(sf::Color(0, 0, 139));
            } else if (cuadricula[y][x].tipo == TipoBloque::Madera) {
                formaBlq.setFillColor(sf::Color(139, 69, 19));
            } else if (cuadricula[y][x].tipo == TipoBloque::MesaCrafteo) {
                formaBlq.setFillColor(sf::Color(130, 85, 45));
            } else if (cuadricula[y][x].tipo == TipoBloque::Horno) {
                formaBlq.setFillColor(sf::Color(85, 85, 85));
            } else if (cuadricula[y][x].tipo == TipoBloque::Cristal) {
                formaBlq.setFillColor(sf::Color(180, 235, 255, 180));
            } else if (cuadricula[y][x].tipo == TipoBloque::Piedra) {
                formaBlq.setFillColor(sf::Color(128, 128, 128));
            } else if (cuadricula[y][x].tipo == TipoBloque::MineralHierro) {
                formaBlq.setFillColor(sf::Color(210, 180, 140));
            } else if (cuadricula[y][x].tipo == TipoBloque::MineralDiamante) {
                formaBlq.setFillColor(sf::Color(0, 255, 255));
            } else {
                formaBlq.setFillColor(sf::Color(0, 100, 0));
            }

            formaBlq.setPosition({x * TAMANIO_BLOQUE, y * TAMANIO_BLOQUE});
            ventana.draw(formaBlq);
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
        case TipoBloque::Piedra: return 300;
        case TipoBloque::MineralHierro: return 450;
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
    cuadricula[y][x] = {tipo, solido, vida, false};
    return true;
}

inline TipoBloque Mundo::getTipoBloque(int x, int y) const {
    if (x < 0 || x >= ancho || y < 0 || y >= alto) return TipoBloque::Piedra;
    return cuadricula[y][x].tipo;
}

inline void Mundo::romperBloque(int x, int y) {
    if (x >= 0 && x < ancho && y >= 0 && y < alto) {
        cuadricula[y][x] = {TipoBloque::Aire, false, 0.0f, false};
    }
}

inline bool Mundo::daniarBloque(int x, int y, float cantidadDanio) {
    if (x < 0 || x >= ancho || y < 0 || y >= alto) return false;
    if (cuadricula[y][x].tipo == TipoBloque::Aire) return false;

    cuadricula[y][x].vida -= cantidadDanio;
    if (cuadricula[y][x].vida <= 0.0f) {
        cuadricula[y][x] = {TipoBloque::Aire, false, 0.0f, false};
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

