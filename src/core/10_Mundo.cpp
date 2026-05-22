#include "Mundo.hpp"
#include <iostream>
#include <cmath>
#include <random>
#include <algorithm>

// Constructor del Mundo
Mundo::Mundo(int ancho, int alto) : ancho(ancho), alto(alto) {
    cuadricula.resize(alto);
    for (int i = 0; i < alto; ++i) {
        cuadricula[i].resize(ancho);
    }
    
    // Proporciones iniciales
    generarMundo(false);
    
    std::cout << "¡Matriz del mundo de " << ancho << "x" << alto << " creada con exito!" << std::endl;
}

// Destructor
Mundo::~Mundo() {}

// Lógica de Generación del Mundo por Capas/Sprints
void Mundo::generarMundo(bool esSubterraneo) {
    const int MARGEN_OCEANO = 15;

    // 1. Inicialización base del Terreno con vidas máximas reales desde el inicio
    for (int y = 0; y < alto; ++y) {
        for (int x = 0; x < ancho; ++x) {
            if (esSubterraneo) {
                cuadricula[y][x] = { TipoBloque::Piedra, true, 300.0f, false };
            } else {
                if (x < MARGEN_OCEANO || x >= (ancho - MARGEN_OCEANO) ||
                    y < MARGEN_OCEANO || y >= (alto - MARGEN_OCEANO)) {
                    cuadricula[y][x] = { TipoBloque::AguaProfunda, true, 9999.0f, false };
                } else {
                    cuadricula[y][x] = { TipoBloque::Pasto, false, 30.0f, false };
                }
            }
        }
    }

    std::random_device rd;
    std::mt19937 gen(rd());

    // --- CAPA SUBTERRÁNEA: VENAS DE MINERALES ---
    if (esSubterraneo) {
        std::uniform_int_distribution<> disX(10, ancho - 10);
        
        // Venas de Hierro Subterráneo
        int cantidadVenasHierro = 800;
        for (int i = 0; i < cantidadVenasHierro; ++i) {
            int centroX = disX(gen);
            std::uniform_int_distribution<> disY(10, alto - 10);
            int centroY = disY(gen);
            
            for (int dy = 0; dy <= 1; ++dy) {
                for (int dx = 0; dx <= 1; ++dx) {
                    int nx = centroX + dx;
                    int ny = centroY + dy;
                    if (nx >= 0 && nx < ancho && ny >= 0 && ny < alto) {
                        cuadricula[ny][nx] = { TipoBloque::MineralHierro, true, 450.0f, false };
                    }
                }
            }
        }

        // Venas de Diamante Subterráneo
        int cantidadVenasDiamante = 250;
        for (int i = 0; i < cantidadVenasDiamante; ++i) {
            int centroX = disX(gen);
            std::uniform_int_distribution<> disYProfundo(alto / 2, alto - 10);
            int centroY = disYProfundo(gen);

            if (centroX >= 0 && centroX < ancho && centroY >= 0 && centroY < alto) {
                cuadricula[centroY][centroX] = { TipoBloque::MineralDiamante, true, 600.0f, false };
                if (centroX + 1 < ancho && (gen() % 2 == 0)) {
                    cuadricula[centroY][centroX + 1] = { TipoBloque::MineralDiamante, true, 600.0f, false };
                }
            }
        }
        std::cout << "¡Subterraneo generado con exito!" << std::endl;
        return;
    }

    // --- CAPA SUPERFICIE: LAGOS, CANTERAS DE PIEDRA Y ÁRBOLES ---
    std::uniform_int_distribution<> disX(MARGEN_OCEANO + 40, ancho - MARGEN_OCEANO - 40);
    std::uniform_int_distribution<> disY(MARGEN_OCEANO + 40, alto - MARGEN_OCEANO - 40);
    std::uniform_int_distribution<> disRadioBase(7, 12);

    // Generar Lagos de Agua
    int cantidadLagos = 150;
    for (int i = 0; i < cantidadLagos; ++i) {
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
                    if (std::sqrt(dx * dx + dy * dy) <= radioBase) {
                        if (cuadricula[y][x].tipo == TipoBloque::Pasto) {
                            cuadricula[y][x] = { TipoBloque::Agua, false, 50.0f, false };
                        }
                    }
                }
            }
        }
    }

    // NUEVO: Generar Canteras de Piedra en la Superficie
    int canterasPiedra = 80;
    std::uniform_int_distribution<> disRadioPiedra(5, 9);
    for (int i = 0; i < canterasPiedra; ++i) {
        int centroX = disX(gen);
        int centroY = disY(gen);
        int radio = disRadioPiedra(gen);

        for (int y = centroY - radio; y <= centroY + radio; ++y) {
            for (int x = centroX - radio; x <= centroX + radio; ++x) {
                if (x >= 0 && x < ancho && y >= 0 && y < alto) {
                    float dx = x - centroX;
                    float dy = y - centroY;
                    if (std::sqrt(dx * dx + dy * dy) <= radio) {
                        if (cuadricula[y][x].tipo == TipoBloque::Pasto) {
                            // De manera aleatoria, ponemos piedra común o algún mineral expuesto
                            int suerte = gen() % 100;
                            if (suerte < 5) {
                                cuadricula[y][x] = { TipoBloque::MineralHierro, true, 450.0f, false };
                            } else if (suerte == 99) {
                                cuadricula[y][x] = { TipoBloque::MineralDiamante, true, 600.0f, false };
                            } else {
                                cuadricula[y][x] = { TipoBloque::Piedra, true, 300.0f, false };
                            }
                        }
                    }
                }
            }
        }
    }

    // Generar Árboles de Madera
    int cantidadArboles = 600; 
    for (int i = 0; i < cantidadArboles; ++i) {
        int tx = disX(gen);
        int ty = disY(gen);

        if (cuadricula[ty][tx].tipo == TipoBloque::Pasto) {
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int nx = tx + dx;
                    int ny = ty + dy;
                    if (std::abs(dx) + std::abs(dy) <= 1) {
                        if (nx >= 0 && nx < ancho && ny >= 0 && ny < alto) {
                            if (cuadricula[ny][nx].tipo == TipoBloque::Pasto) {
                                // Nace directamente con los 90.0f de vida reglamentarios
                                cuadricula[ny][nx] = { TipoBloque::Madera, true, 90.0f, false };
                            }
                        }
                    }
                }
            }
        }
    }
    std::cout << "¡Superficie generada completamente!" << std::endl;
}

// Método Dibujar
void Mundo::dibujar(sf::RenderWindow& ventana) {
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
                if ((x + y) % 2 == 0) {
                    formaBlq.setFillColor(sf::Color(34, 139, 34));  // Verde Bosque
                } else {
                    formaBlq.setFillColor(sf::Color(46, 139, 87));  // Verde Mar Oscuro
                }
            } else if (cuadricula[y][x].tipo == TipoBloque::Agua) {
                formaBlq.setFillColor(sf::Color(30, 144, 255));
            } else if (cuadricula[y][x].tipo == TipoBloque::AguaProfunda) {
                formaBlq.setFillColor(sf::Color(0, 0, 139));
            } else if (cuadricula[y][x].tipo == TipoBloque::Madera) {
                formaBlq.setFillColor(sf::Color(139, 69, 19));
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

int Mundo::getVidaMaximaBloque(TipoBloque tipo) const {
    switch (tipo) {
        case TipoBloque::Pasto:           return 30;   
        case TipoBloque::Madera:          return 90;   
        case TipoBloque::Piedra:          return 300;  
        case TipoBloque::MineralHierro:   return 450;  
        case TipoBloque::MineralDiamante: return 600;  
        default: return 50;
    }
}

bool Mundo::esBloqueSolido(int x, int y) const {
    if (x < 0 || x >= ancho || y < 0 || y >= alto) {
        return true; 
    }
    return cuadricula[y][x].esSolido;
}

TipoBloque Mundo::getTipoBloque(int x, int y) const {
    if (x < 0 || x >= ancho || y < 0 || y >= alto) return TipoBloque::Piedra;
    return cuadricula[y][x].tipo;
}

void Mundo::romperBloque(int x, int y) {
    if (x >= 0 && x < ancho && y >= 0 && y < alto) {
        cuadricula[y][x] = { TipoBloque::Aire, false, 0.0f, false };
    }
}

bool Mundo::daniarBloque(int x, int y, float cantidadDanio) {
    if (x < 0 || x >= ancho || y < 0 || y >= alto) return false;
    if (cuadricula[y][x].tipo == TipoBloque::Aire) return false;

    // Quitamos el reinicio forzado molesto; ahora confiamos plenamente en la vida que el bloque trae
    cuadricula[y][x].vida -= cantidadDanio;

    if (cuadricula[y][x].vida <= 0.0f) {
        cuadricula[y][x] = { TipoBloque::Aire, false, 0.0f, false };
        return true; // ¡Bloque destruido!
    }
    return false; 
}