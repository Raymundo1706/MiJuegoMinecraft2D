#include "Mundo.hpp"
#include "Jugador.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>

inline Zombie::Zombie(float x, float y, bool bebe)
    : posicion(x, y),
      velocidad(0.0f, 0.0f),
      vida(bebe ? 10.0f : 20.0f),
      vidaMaxima(bebe ? 10.0f : 20.0f),
      velocidadBase(bebe ? 54.0f : 42.0f),
      tiempoAtaque(0.0f),
      tiempoQuemadura(0.0f),
      tiempoLejos(0.0f),
      bebe(bebe),
      vivo(true),
      temporal(true) {
    float tam = bebe ? 14.0f : 24.0f;
    forma.setSize({tam, tam});
    forma.setFillColor(bebe ? sf::Color(86, 150, 70) : sf::Color(54, 116, 62));
    forma.setOutlineColor(sf::Color(20, 38, 24));
    forma.setOutlineThickness(2.0f);
    forma.setPosition(posicion);
}

inline Zombie::~Zombie() {}

inline bool Zombie::colisionaConMundo(const Mundo& mundo, sf::Vector2f nuevaPosicion) const {
    const float tam = bebe ? 14.0f : 24.0f;
    const float offsetX = bebe ? 3.0f : 6.0f;
    const float offsetY = bebe ? 7.0f : 14.0f;
    const float ancho = bebe ? 8.0f : 12.0f;
    const float alto = bebe ? 7.0f : 10.0f;

    int izq = static_cast<int>(std::floor((nuevaPosicion.x + offsetX) / TAMANIO_BLOQUE_JUEGO));
    int der = static_cast<int>(std::floor((nuevaPosicion.x + offsetX + ancho - 1.0f) / TAMANIO_BLOQUE_JUEGO));
    int arriba = static_cast<int>(std::floor((nuevaPosicion.y + offsetY) / TAMANIO_BLOQUE_JUEGO));
    int abajo = static_cast<int>(std::floor((nuevaPosicion.y + offsetY + alto - 1.0f) / TAMANIO_BLOQUE_JUEGO));

    for (int y = arriba; y <= abajo; ++y) {
        for (int x = izq; x <= der; ++x) {
            if (mundo.esBloqueSolido(x, y)) {
                return true;
            }
        }
    }
    (void)tam;
    return false;
}

inline bool Zombie::estaEnAgua(const Mundo& mundo) const {
    float tam = bebe ? 14.0f : 24.0f;
    float centroX = posicion.x + tam * 0.5f;
    float centroY = posicion.y + tam * 0.72f;
    int bx = static_cast<int>(std::floor(centroX / TAMANIO_BLOQUE_JUEGO));
    int by = static_cast<int>(std::floor(centroY / TAMANIO_BLOQUE_JUEGO));
    TipoBloque tipo = mundo.getTipoBloque(bx, by);
    return tipo == TipoBloque::Agua || tipo == TipoBloque::AguaProfunda;
}

inline void Zombie::actualizar(float dt, const Mundo& mundo, Jugador& jugador, int skyLight) {
    if (!vivo) {
        return;
    }

    tiempoAtaque = std::max(0.0f, tiempoAtaque - dt);
    sf::Vector2f centroJugador = jugador.getPosicion() + sf::Vector2f(12.0f, 12.0f);
    float tam = bebe ? 14.0f : 24.0f;
    sf::Vector2f centroZombie = posicion + sf::Vector2f(tam * 0.5f, tam * 0.5f);
    sf::Vector2f delta = centroJugador - centroZombie;
    float distancia = std::sqrt(delta.x * delta.x + delta.y * delta.y);

    if (distancia > 128.0f * TAMANIO_BLOQUE_JUEGO && temporal) {
        vivo = false;
        return;
    }

    if (distancia > 32.0f * TAMANIO_BLOQUE_JUEGO) {
        tiempoLejos += dt;
        if (tiempoLejos > 30.0f) {
            unsigned int azar = static_cast<unsigned int>(posicion.x * 13.0f + posicion.y * 17.0f + tiempoLejos * 31.0f);
            if ((azar % 40u) == 0u) {
                vivo = false;
                return;
            }
        }
    } else {
        tiempoLejos = 0.0f;
    }

    if (!bebe && skyLight > 10 && !estaEnAgua(mundo)) {
        tiempoQuemadura += dt;
        if (tiempoQuemadura >= 1.0f) {
            vida -= 1.0f;
            tiempoQuemadura = 0.0f;
            if (vida <= 0.0f) {
                vivo = false;
                return;
            }
        }
    } else {
        tiempoQuemadura = 0.0f;
    }

    if (distancia < 35.0f * TAMANIO_BLOQUE_JUEGO && distancia > 1.0f) {
        sf::Vector2f direccion = delta / distancia;
        velocidad = direccion * velocidadBase;
    } else {
        velocidad = {0.0f, 0.0f};
    }

    if (distancia <= 1.5f * TAMANIO_BLOQUE_JUEGO && tiempoAtaque <= 0.0f) {
        jugador.recibirDanio(3);
        tiempoAtaque = 1.0f;
    }

    sf::Vector2f nuevaX = posicion + sf::Vector2f(velocidad.x * dt, 0.0f);
    if (!colisionaConMundo(mundo, nuevaX)) {
        posicion.x = nuevaX.x;
    }

    sf::Vector2f nuevaY = posicion + sf::Vector2f(0.0f, velocidad.y * dt);
    if (!colisionaConMundo(mundo, nuevaY)) {
        posicion.y = nuevaY.y;
    }

    forma.setPosition(posicion);
}

inline void Zombie::dibujar(sf::RenderWindow& ventana) {
    if (!vivo) {
        return;
    }

    float tam = bebe ? 14.0f : 24.0f;
    sf::RectangleShape sombra({tam, 5.0f});
    sombra.setPosition({posicion.x, posicion.y + tam - 2.0f});
    sombra.setFillColor(sf::Color(0, 0, 0, 85));
    ventana.draw(sombra);

    sf::RectangleShape cuerpo = forma;
    if (tiempoQuemadura > 0.0f) {
        cuerpo.setFillColor(sf::Color(130, 88, 44));
    }
    ventana.draw(cuerpo);

    sf::RectangleShape ojos({3.0f, 3.0f});
    ojos.setFillColor(sf::Color(20, 18, 18));
    ojos.setPosition({posicion.x + tam * 0.25f, posicion.y + tam * 0.28f});
    ventana.draw(ojos);
    ojos.setPosition({posicion.x + tam * 0.62f, posicion.y + tam * 0.28f});
    ventana.draw(ojos);

    if (vida < vidaMaxima) {
        sf::RectangleShape fondo({tam, 3.0f});
        fondo.setPosition({posicion.x, posicion.y - 6.0f});
        fondo.setFillColor(sf::Color(40, 8, 8, 180));
        ventana.draw(fondo);

        sf::RectangleShape barra({tam * std::max(0.0f, vida / vidaMaxima), 3.0f});
        barra.setPosition({posicion.x, posicion.y - 6.0f});
        barra.setFillColor(sf::Color(170, 32, 32));
        ventana.draw(barra);
    }
}

inline void Zombie::recibirDanio(float danio) {
    if (danio <= 0.0f || !vivo) {
        return;
    }
    float danioFinal = danio * 0.92f;
    vida -= danioFinal;
    if (vida <= 0.0f) {
        vivo = false;
    }
}

inline bool Zombie::estaVivo() const {
    return vivo;
}

inline bool Zombie::debeEliminarse() const {
    return !vivo;
}

inline bool Zombie::contienePunto(sf::Vector2f punto) const {
    return forma.getGlobalBounds().contains(punto);
}

inline sf::Vector2f Zombie::getPosicion() const {
    return posicion;
}
