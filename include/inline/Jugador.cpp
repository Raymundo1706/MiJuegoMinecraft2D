#include "Mundo.hpp"
#include <cmath>
#include <algorithm>
#include <cstdint>

// Constructor: Recibe la posiciÃ³n aleatoria de spawn
inline Jugador::Jugador(float x, float y) {
    posicion = {x, y};
    velocidad = 250.0f; // PÃ­xeles por segundo
    direccionMirada = DireccionMirada::Abajo;
    caminando = false;
    tiempoAnimacion = 0.0f;
    accionando = false;
    tiempoAccion = 0.0f;
    itemAccion = ItemId::Ninguno;
    enAgua = false;
    hundido = false;
    tiempoEnAgua = 0.0f;
    tiempoHundimiento = 0.0f;
    tiempoMojado = 0.0f;
    vidaMaximaHP = 20;
    vidaHP = vidaMaximaHP;
    tiempoInvulnerable = 0.0f;
    ultimoDanioHP = 0;

    // TamaÃ±o del personaje: 24x24 pÃ­xeles (cabe perfectamente dentro de un bloque de 32x32)
    forma.setSize({24.0f, 24.0f});
    forma.setFillColor(sf::Color::Red); // Cuadro rojo identificador
    forma.setPosition(posicion);
}

// Destructor
inline Jugador::~Jugador() {}

inline bool jugadorTocaAgua(const Mundo& mundo, sf::Vector2f posicionJugador, sf::Vector2f tamanoJugador) {
    const float TAMANIO_BLOQUE = 32.0f;

    auto puntoEsAgua = [&](float px, float py) {
        int bloqueX = static_cast<int>(std::floor(px / TAMANIO_BLOQUE));
        int bloqueY = static_cast<int>(std::floor(py / TAMANIO_BLOQUE));
        TipoBloque tipo = mundo.getTipoBloque(bloqueX, bloqueY);
        return tipo == TipoBloque::Agua || tipo == TipoBloque::AguaProfunda;
    };

    float izquierda = posicionJugador.x + 4.0f;
    float derecha = posicionJugador.x + tamanoJugador.x - 4.0f;
    float centroX = posicionJugador.x + tamanoJugador.x * 0.5f;
    float pechoY = posicionJugador.y + tamanoJugador.y * 0.55f;
    float piesY = posicionJugador.y + tamanoJugador.y - 2.0f;

    if (puntoEsAgua(centroX, pechoY) || puntoEsAgua(centroX, piesY)) {
        return true;
    }

    float muestrasLaterales[2] = {izquierda, derecha};
    for (float x : muestrasLaterales) {
        if (puntoEsAgua(x, piesY) || puntoEsAgua(x, pechoY)) {
            return true;
        }
    }
    return false;
}

// MÃ©todo para mover al personaje detectando colisiones sÃ³lidas con el terreno
inline void Jugador::controlar(float dt, const Mundo& mundo) {
    if (tiempoInvulnerable > 0.0f) {
        tiempoInvulnerable = std::max(0.0f, tiempoInvulnerable - dt);
        if (tiempoInvulnerable <= 0.0f) {
            ultimoDanioHP = 0;
        }
    }

    if (accionando) {
        tiempoAccion += dt;
        if (tiempoAccion >= 0.32f) {
            accionando = false;
            tiempoAccion = 0.0f;
            itemAccion = ItemId::Ninguno;
        }
    }

    bool estabaEnAgua = enAgua;
    enAgua = jugadorTocaAgua(mundo, posicion, forma.getSize());
    if (enAgua) {
        tiempoEnAgua += dt;
        tiempoMojado = 5.0f;
        if (tiempoEnAgua >= 20.0f) {
            hundido = true;
        }
    } else {
        if (estabaEnAgua) {
            tiempoMojado = 5.0f;
        } else if (tiempoMojado > 0.0f) {
            tiempoMojado = std::max(0.0f, tiempoMojado - dt);
        }
        tiempoEnAgua = 0.0f;
        tiempoHundimiento = 0.0f;
        hundido = false;
    }

    if (hundido) {
        tiempoHundimiento += dt;
        caminando = false;
        forma.setPosition(posicion);
        return;
    }

    sf::Vector2f direccion(0.0f, 0.0f);
    bool arribaPresionado = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) ||
                            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up);
    bool abajoPresionado = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) ||
                           sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down);
    bool izquierdaPresionado = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
                               sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left);
    bool derechaPresionado = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
                             sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right);

    // DetecciÃ³n de teclas (WASD y Flechas)
    if (arribaPresionado) {
        direccion.y -= 1.0f;
    }
    if (abajoPresionado) {
        direccion.y += 1.0f;
    }
    if (izquierdaPresionado) {
        direccion.x -= 1.0f;
    }
    if (derechaPresionado) {
        direccion.x += 1.0f;
    }

    // Si no hay teclas presionadas, no hacemos cÃ¡lculos
    if (direccion.x == 0.0f && direccion.y == 0.0f) {
        caminando = false;
        return;
    }

    caminando = true;
    tiempoAnimacion += dt;

    if (izquierdaPresionado && !derechaPresionado) {
        direccionMirada = DireccionMirada::Izquierda;
    } else if (derechaPresionado && !izquierdaPresionado) {
        direccionMirada = DireccionMirada::Derecha;
    } else {
        direccionMirada = direccion.y > 0.0f ? DireccionMirada::Abajo : DireccionMirada::Arriba;
    }

    // Normalizamos el vector de direcciÃ³n para evitar que camine mÃ¡s rÃ¡pido en diagonal
    float longitud = std::sqrt(direccion.x * direccion.x + direccion.y * direccion.y);
    direccion /= longitud;
    float velocidadActual = enAgua ? velocidad / 3.0f : velocidad;

    const float TAMANIO_BLOQUE = 32.0f;
    float anchoJugador = forma.getSize().x;
    float altoJugador = forma.getSize().y;

    // --- COMIENZA EL PASO POR EJES INDEPENDIENTES ---
    
    // 1. INTENTO DE MOVIMIENTO EN EJE X
    sf::Vector2f nuevaPosicionX = posicion;
    nuevaPosicionX.x += direccion.x * velocidadActual * dt;

    // Calculamos las esquinas de la caja del jugador en el eje X para ver con quÃ© bloques interseca
    int bloqueIzq = static_cast<int>(nuevaPosicionX.x / TAMANIO_BLOQUE);
    int bloqueDer = static_cast<int>((nuevaPosicionX.x + anchoJugador - 1.0f) / TAMANIO_BLOQUE);
    int bloqueArriba = static_cast<int>(posicion.y / TAMANIO_BLOQUE);
    int bloqueAbajo = static_cast<int>((posicion.y + altoJugador - 1.0f) / TAMANIO_BLOQUE);

    bool colisionX = false;
    // Revisamos la solidez de los bloques que toca el cuerpo del jugador en X
    for (int y = bloqueArriba; y <= bloqueAbajo; ++y) {
        if (direccion.x < 0.0f && mundo.esBloqueSolido(bloqueIzq, y)) colisionX = true;
        if (direccion.x > 0.0f && mundo.esBloqueSolido(bloqueDer, y)) colisionX = true;
    }

    // Si no hay colisiÃ³n, aceptamos el movimiento en X
    if (!colisionX) {
        posicion.x = nuevaPosicionX.x;
    }

    // 2. INTENTO DE MOVIMIENTO EN EJE Y
    sf::Vector2f nuevaPosicionY = posicion;
    nuevaPosicionY.y += direccion.y * velocidadActual * dt;

    // Recalculamos las esquinas ahora con la nueva coordenada de Y
    bloqueIzq = static_cast<int>(posicion.x / TAMANIO_BLOQUE);
    bloqueDer = static_cast<int>((posicion.x + anchoJugador - 1.0f) / TAMANIO_BLOQUE);
    bloqueArriba = static_cast<int>(nuevaPosicionY.y / TAMANIO_BLOQUE);
    bloqueAbajo = static_cast<int>((nuevaPosicionY.y + altoJugador - 1.0f) / TAMANIO_BLOQUE);

    bool colisionY = false;
    // Revisamos la solidez de los bloques que toca el cuerpo del jugador en Y
    for (int x = bloqueIzq; x <= bloqueDer; ++x) {
        if (direccion.y < 0.0f && mundo.esBloqueSolido(x, bloqueArriba)) colisionY = true;
        if (direccion.y > 0.0f && mundo.esBloqueSolido(x, bloqueAbajo)) colisionY = true;
    }

    // Si no hay colisiÃ³n, aceptamos el movimiento en Y
    if (!colisionY) {
        posicion.y = nuevaPosicionY.y;
    }

    // Aplicamos la posiciÃ³n final validada a la figura del jugador
    forma.setPosition(posicion);
}

// MÃ©todo para pintar al jugador encima del mundo
inline void Jugador::dibujar(sf::RenderWindow& ventana) {
    dibujarSpriteJugador(ventana);
}

inline void Jugador::iniciarAccion(ItemId item) {
    accionando = true;
    tiempoAccion = 0.0f;
    itemAccion = item;
}

inline void Jugador::dibujarPixel(sf::RenderWindow& ventana, sf::Vector2f origen, int x, int y, sf::Color color, float escala) {
    sf::RectangleShape pixel({escala, escala});
    pixel.setPosition({origen.x + static_cast<float>(x) * escala, origen.y + static_cast<float>(y) * escala});
    pixel.setFillColor(color);
    ventana.draw(pixel);
}

inline void Jugador::dibujarRectPixel(sf::RenderWindow& ventana, sf::Vector2f origen, int x, int y, int ancho, int alto, sf::Color color, float escala) {
    for (int py = y; py < y + alto; ++py) {
        for (int px = x; px < x + ancho; ++px) {
            dibujarPixel(ventana, origen, px, py, color, escala);
        }
    }
}

inline void Jugador::dibujarSpriteJugador(sf::RenderWindow& ventana) {
    static bool texturaLista = false;
    static sf::Texture texturaJugador;
    static sf::Texture texturaJugadorEspejada;
    static bool accionesLista = false;
    static sf::Texture texturaAcciones;
    static sf::Texture texturaAccionesEspejada;

    if (!texturaLista) {
        sf::Image imagen;
        if (imagen.loadFromFile("assets/player_walk.png")) {
            sf::Image espejo(imagen.getSize(), sf::Color::Transparent);
            sf::Vector2u tam = imagen.getSize();
            for (unsigned int y = 0; y < tam.y; ++y) {
                for (unsigned int x = 0; x < tam.x; ++x) {
                    espejo.setPixel({tam.x - 1 - x, y}, imagen.getPixel({x, y}));
                }
            }
            texturaLista = texturaJugador.loadFromImage(imagen) &&
                           texturaJugadorEspejada.loadFromImage(espejo);
            texturaJugador.setSmooth(false);
            texturaJugadorEspejada.setSmooth(false);
        }
    }
    if (!accionesLista) {
        sf::Image imagen;
        if (imagen.loadFromFile("assets/player_actions.png")) {
            sf::Image espejo(imagen.getSize(), sf::Color::Transparent);
            sf::Vector2u tam = imagen.getSize();
            for (unsigned int y = 0; y < tam.y; ++y) {
                for (unsigned int x = 0; x < tam.x; ++x) {
                    espejo.setPixel({tam.x - 1 - x, y}, imagen.getPixel({x, y}));
                }
            }
            accionesLista = texturaAcciones.loadFromImage(imagen) &&
                            texturaAccionesEspejada.loadFromImage(espejo);
            texturaAcciones.setSmooth(false);
            texturaAccionesEspejada.setSmooth(false);
        }
    }

    float hundimientoVisual = enAgua ? 8.0f : 0.0f;
    if (hundido) {
        hundimientoVisual = std::min(24.0f, 8.0f + tiempoHundimiento * 10.0f);
    }

    if (!enAgua || !hundido) {
        sf::RectangleShape sombra({24.0f, 7.0f});
        sombra.setPosition({posicion.x, posicion.y + 20.0f});
        sombra.setFillColor(sf::Color(8, 18, 12, enAgua ? 35 : 80));
        ventana.draw(sombra);
    }

    if (!texturaLista) {
        forma.setPosition({posicion.x, posicion.y + hundimientoVisual});
        ventana.draw(forma);
        return;
    }

    bool usarAccion = accionando && accionesLista;
    int fila = 0;
    bool espejarHorizontal = false;
    int columna = 0;
    sf::Texture* texturaActiva = &texturaJugador;
    int altoFrame = 32;

    if (usarAccion) {
        texturaActiva = &texturaAcciones;
        columna = std::min(2, static_cast<int>(tiempoAccion / 0.11f));

        bool esHerramientaTrabajo = itemAccion == ItemId::PicoMadera ||
                                    itemAccion == ItemId::PicoPiedra ||
                                    itemAccion == ItemId::PicoDiamante ||
                                    itemAccion == ItemId::HachaMadera ||
                                    itemAccion == ItemId::HachaPiedra ||
                                    itemAccion == ItemId::PalaMadera ||
                                    itemAccion == ItemId::PalaPiedra ||
                                    itemAccion == ItemId::Barreta;
        int baseAccion = esHerramientaTrabajo ? 0 : 6;

        if (direccionMirada == DireccionMirada::Abajo) fila = baseAccion + 0;
        if (direccionMirada == DireccionMirada::Izquierda) {
            fila = baseAccion + 1;
            espejarHorizontal = true;
        }
        if (direccionMirada == DireccionMirada::Derecha) {
            fila = baseAccion + 1;
        }
        if (direccionMirada == DireccionMirada::Arriba) fila = baseAccion + 2;
    } else {
        if (direccionMirada == DireccionMirada::Abajo) fila = caminando ? 1 : 0;
        if (direccionMirada == DireccionMirada::Arriba) fila = caminando ? 5 : 2;
        if (direccionMirada == DireccionMirada::Derecha) fila = caminando ? 4 : 3;
        if (direccionMirada == DireccionMirada::Izquierda) {
            fila = caminando ? 4 : 3;
            espejarHorizontal = true;
        }

        columna = caminando ? static_cast<int>(tiempoAnimacion * 8.0f) % 6 : 0;
    }

    if (espejarHorizontal) {
        texturaActiva = usarAccion ? &texturaAccionesEspejada : &texturaJugadorEspejada;
        int columnasTotales = usarAccion ? 3 : 6;
        columna = columnasTotales - 1 - columna;
    }

    sf::Sprite sprite(*texturaActiva);
    sprite.setTextureRect(sf::IntRect({columna * 32, fila * 32}, {32, altoFrame}));
    sprite.setOrigin({16.0f, 30.0f});
    sprite.setPosition({posicion.x + 12.0f, posicion.y + 26.0f + hundimientoVisual});
    sprite.setScale({1.2f, 1.2f});
    if (hundido) {
        float alpha = std::max(45.0f, 255.0f - tiempoHundimiento * 90.0f);
        sprite.setColor(sf::Color(185, 220, 255, static_cast<std::uint8_t>(alpha)));
    } else if (enAgua) {
        sprite.setColor(sf::Color(215, 238, 255, 245));
    } else if (tiempoMojado > 0.0f) {
        std::uint8_t azul = static_cast<std::uint8_t>(std::min(255.0f, 235.0f + tiempoMojado * 4.0f));
        sprite.setColor(sf::Color(230, 242, azul, 255));
    }
    ventana.draw(sprite);

    if (!enAgua && tiempoMojado > 0.0f) {
        sf::RectangleShape gota;
        gota.setFillColor(sf::Color(92, 190, 245, static_cast<std::uint8_t>(45 + tiempoMojado * 35.0f)));
        float fase = (5.0f - tiempoMojado) * 5.0f;
        for (int i = 0; i < 4; ++i) {
            float x = posicion.x + 4.0f + static_cast<float>((i * 6 + static_cast<int>(fase)) % 22);
            float y = posicion.y + 10.0f + std::fmod(fase * 3.0f + static_cast<float>(i * 7), 22.0f);
            gota.setSize({2.0f, i % 2 == 0 ? 4.0f : 3.0f});
            gota.setPosition({x, y});
            ventana.draw(gota);
        }
    }
}

inline sf::Vector2f Jugador::getPosicion() const {
    return posicion;
}

inline bool Jugador::estaEnAgua() const {
    return enAgua;
}

inline bool Jugador::estaHundido() const {
    return hundido;
}

inline float Jugador::getTiempoEnAgua() const {
    return tiempoEnAgua;
}

inline int Jugador::getVidaHP() const {
    return vidaHP;
}

inline int Jugador::getVidaMaximaHP() const {
    return vidaMaximaHP;
}

inline bool Jugador::estaMuerto() const {
    return vidaHP <= 0;
}

inline void Jugador::recibirDanio(int danioHP) {
    if (danioHP <= 0 || vidaHP <= 0) {
        return;
    }

    int danioAplicado = danioHP;
    if (tiempoInvulnerable > 0.0f) {
        if (danioHP <= ultimoDanioHP) {
            return;
        }
        danioAplicado = danioHP - ultimoDanioHP;
    }

    vidaHP = std::max(0, vidaHP - danioAplicado);
    ultimoDanioHP = std::max(ultimoDanioHP, danioHP);
    tiempoInvulnerable = 0.5f;
}

inline void Jugador::curar(int puntosHP) {
    if (puntosHP <= 0 || vidaHP <= 0) {
        return;
    }
    vidaHP = std::min(vidaMaximaHP, vidaHP + puntosHP);
}

