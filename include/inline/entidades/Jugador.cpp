#include "../../core/Mundo.hpp"
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace {
constexpr float MULTIPLICADOR_VELOCIDAD_JUGADOR = 1.75f;
}

// Constructor: Recibe la posición aleatoria de spawn
inline Jugador::Jugador(float x, float y) {
    posicion = {x, y};
    velocidad = 4.317f * TAMANIO_BLOQUE_JUEGO * MULTIPLICADOR_VELOCIDAD_JUGADOR;
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
    hambre = 20;
    saturacion = 5.0f;
    agotamiento = 0.0f;
    tiempoRegeneracion = 0.0f;
    tiempoInanicion = 0.0f;
    tiempoDesdeAtaque = 999.0f;
    corriendo = false;
    agachado = false;

    // Tamaño del personaje: 24x24 píxeles (cabe perfectamente dentro de un bloque de 32x32)
    forma.setSize({24.0f, 24.0f});
    forma.setFillColor(sf::Color::Red); // Cuadro rojo identificador
    forma.setPosition(posicion);
}

// Destructor
inline Jugador::~Jugador() {}

inline bool jugadorTocaAgua(const Mundo& mundo, sf::Vector2f posicionJugador, sf::Vector2f tamanoJugador) {
    const float TAMANIO_BLOQUE = TAMANIO_BLOQUE_JUEGO;

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

inline bool jugadorEstaDentroDelAgua(const Mundo& mundo, sf::Vector2f posicionJugador, sf::Vector2f tamanoJugador) {
    const float TAMANIO_BLOQUE = TAMANIO_BLOQUE_JUEGO;

    auto puntoEsAgua = [&](float px, float py) {
        int bloqueX = static_cast<int>(std::floor(px / TAMANIO_BLOQUE));
        int bloqueY = static_cast<int>(std::floor(py / TAMANIO_BLOQUE));
        TipoBloque tipo = mundo.getTipoBloque(bloqueX, bloqueY);
        return tipo == TipoBloque::Agua || tipo == TipoBloque::AguaProfunda;
    };

    float xs[3] = {
        posicionJugador.x + 7.0f,
        posicionJugador.x + tamanoJugador.x * 0.5f,
        posicionJugador.x + tamanoJugador.x - 7.0f
    };
    float yCintura = posicionJugador.y + tamanoJugador.y * 0.68f;
    float yPies = posicionJugador.y + tamanoJugador.y - 3.0f;

    int puntosEnAgua = 0;
    for (float x : xs) {
        if (puntoEsAgua(x, yCintura)) ++puntosEnAgua;
        if (puntoEsAgua(x, yPies)) ++puntosEnAgua;
    }

    return puntoEsAgua(xs[1], yPies) && puntosEnAgua >= 4;
}

inline float tiempoRecargaAtaqueJugador(ItemId item) {
    switch (item) {
        case ItemId::EspadaMadera:
        case ItemId::EspadaPiedra:
            return 0.625f;
        case ItemId::HachaMadera:
            return 1.0f;
        case ItemId::HachaPiedra:
            return 1.25f;
        default:
            return 0.25f;
    }
}

inline bool datosComidaJugador(ItemId item, int& puntosComida, float& modificadorSaturacion) {
    switch (item) {
        case ItemId::Zanahoria:
            puntosComida = 3;
            modificadorSaturacion = 0.6f;
            return true;
        case ItemId::Patata:
            puntosComida = 1;
            modificadorSaturacion = 0.6f;
            return true;
        case ItemId::ChuletaCerdoCruda:
        case ItemId::CarneResCruda:
            puntosComida = 3;
            modificadorSaturacion = 0.3f;
            return true;
        case ItemId::ChuletaCerdoCocinada:
            puntosComida = 8;
            modificadorSaturacion = 0.8f;
            return true;
        case ItemId::PolloCrudo:
            puntosComida = 2;
            modificadorSaturacion = 0.3f;
            return true;
        default:
            puntosComida = 0;
            modificadorSaturacion = 0.0f;
            return false;
    }
}

inline void Jugador::actualizarNutricion(float dt) {
    while (agotamiento >= 4.0f) {
        agotamiento -= 4.0f;
        if (saturacion > 0.0f) {
            saturacion = std::max(0.0f, saturacion - 1.0f);
        } else if (hambre > 0) {
            hambre = std::max(0, hambre - 1);
        }
    }

    if (vidaHP < vidaMaximaHP && hambre == 20 && saturacion > 0.0f) {
        tiempoRegeneracion += dt;
        if (tiempoRegeneracion >= 0.5f) {
            curar(1);
            agregarAgotamiento(6.0f);
            tiempoRegeneracion = 0.0f;
        }
    } else if (vidaHP < vidaMaximaHP && hambre >= 18) {
        tiempoRegeneracion += dt;
        if (tiempoRegeneracion >= 4.0f) {
            curar(1);
            agregarAgotamiento(6.0f);
            tiempoRegeneracion = 0.0f;
        }
    } else {
        tiempoRegeneracion = 0.0f;
    }

    if (hambre == 0) {
        tiempoInanicion += dt;
        if (tiempoInanicion >= 4.0f) {
            if (vidaHP > 1) {
                vidaHP = std::max(1, vidaHP - 1);
            }
            tiempoInanicion = 0.0f;
        }
    } else {
        tiempoInanicion = 0.0f;
    }
}

// Método para mover al personaje detectando colisiones sólidas con el terreno
inline void Jugador::controlar(float dt, const Mundo& mundo) {
    tiempoDesdeAtaque = std::min(10.0f, tiempoDesdeAtaque + dt);
    actualizarNutricion(dt);

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
    bool tocandoAgua = jugadorTocaAgua(mundo, posicion, forma.getSize());
    enAgua = jugadorEstaDentroDelAgua(mundo, posicion, forma.getSize());
    if (enAgua) {
        tiempoEnAgua += dt;
        tiempoMojado = 5.0f;
        if (tiempoEnAgua >= 20.0f) {
            hundido = true;
        }
    } else {
        if (estabaEnAgua || tocandoAgua) {
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
    bool sprintPresionado = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl) ||
                            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RControl);
    bool agachadoPresionado = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) ||
                              sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift);

    // Detección de teclas (WASD y Flechas)
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

    // Si no hay teclas presionadas, no hacemos cálculos
    if (direccion.x == 0.0f && direccion.y == 0.0f) {
        caminando = false;
        corriendo = false;
        agachado = false;
        return;
    }

    caminando = true;
    agachado = agachadoPresionado && !enAgua;
    corriendo = sprintPresionado && !agachado && !enAgua && hambre > 6;
    tiempoAnimacion += dt;

    if (izquierdaPresionado && !derechaPresionado) {
        direccionMirada = DireccionMirada::Izquierda;
    } else if (derechaPresionado && !izquierdaPresionado) {
        direccionMirada = DireccionMirada::Derecha;
    } else {
        direccionMirada = direccion.y > 0.0f ? DireccionMirada::Abajo : DireccionMirada::Arriba;
    }

    // Normalizamos el vector de dirección para evitar que camine más rápido en diagonal
    float longitud = std::sqrt(direccion.x * direccion.x + direccion.y * direccion.y);
    direccion /= longitud;
    float velocidadActual = velocidad;
    if (corriendo) {
        velocidadActual = 5.612f * TAMANIO_BLOQUE_JUEGO * MULTIPLICADOR_VELOCIDAD_JUGADOR;
    }
    if (agachado) {
        velocidadActual = 1.31f * TAMANIO_BLOQUE_JUEGO * MULTIPLICADOR_VELOCIDAD_JUGADOR;
    }
    if (enAgua) {
        velocidadActual = 2.20f * TAMANIO_BLOQUE_JUEGO * MULTIPLICADOR_VELOCIDAD_JUGADOR;
    }

    const float TAMANIO_BLOQUE = TAMANIO_BLOQUE_JUEGO;
    const float hitboxOffsetX = 6.0f;
    const float hitboxOffsetY = 14.0f;
    const float hitboxAncho = 12.0f;
    const float hitboxAlto = 10.0f;

    // --- COMIENZA EL PASO POR EJES INDEPENDIENTES ---
    sf::Vector2f posicionAntes = posicion;
    
    // 1. INTENTO DE MOVIMIENTO EN EJE X
    sf::Vector2f nuevaPosicionX = posicion;
    nuevaPosicionX.x += direccion.x * velocidadActual * dt;

    // En vista cenital solo los pies deben colisionar; el torso/cabeza pueden pasar visualmente frente a objetos.
    int bloqueIzq = static_cast<int>((nuevaPosicionX.x + hitboxOffsetX) / TAMANIO_BLOQUE);
    int bloqueDer = static_cast<int>((nuevaPosicionX.x + hitboxOffsetX + hitboxAncho - 1.0f) / TAMANIO_BLOQUE);
    int bloqueArriba = static_cast<int>((posicion.y + hitboxOffsetY) / TAMANIO_BLOQUE);
    int bloqueAbajo = static_cast<int>((posicion.y + hitboxOffsetY + hitboxAlto - 1.0f) / TAMANIO_BLOQUE);

    bool colisionX = false;
    // Revisamos la solidez de los bloques que toca el cuerpo del jugador en X
    for (int y = bloqueArriba; y <= bloqueAbajo; ++y) {
        if (direccion.x < 0.0f && mundo.esBloqueSolido(bloqueIzq, y)) colisionX = true;
        if (direccion.x > 0.0f && mundo.esBloqueSolido(bloqueDer, y)) colisionX = true;
    }

    // Si no hay colisión, aceptamos el movimiento en X
    if (!colisionX) {
        posicion.x = nuevaPosicionX.x;
    }

    // 2. INTENTO DE MOVIMIENTO EN EJE Y
    sf::Vector2f nuevaPosicionY = posicion;
    nuevaPosicionY.y += direccion.y * velocidadActual * dt;

    // Recalculamos las esquinas ahora con la nueva coordenada de Y
    bloqueIzq = static_cast<int>((posicion.x + hitboxOffsetX) / TAMANIO_BLOQUE);
    bloqueDer = static_cast<int>((posicion.x + hitboxOffsetX + hitboxAncho - 1.0f) / TAMANIO_BLOQUE);
    bloqueArriba = static_cast<int>((nuevaPosicionY.y + hitboxOffsetY) / TAMANIO_BLOQUE);
    bloqueAbajo = static_cast<int>((nuevaPosicionY.y + hitboxOffsetY + hitboxAlto - 1.0f) / TAMANIO_BLOQUE);

    bool colisionY = false;
    // Revisamos la solidez de los bloques que toca el cuerpo del jugador en Y
    for (int x = bloqueIzq; x <= bloqueDer; ++x) {
        if (direccion.y < 0.0f && mundo.esBloqueSolido(x, bloqueArriba)) colisionY = true;
        if (direccion.y > 0.0f && mundo.esBloqueSolido(x, bloqueAbajo)) colisionY = true;
    }

    // Si no hay colisión, aceptamos el movimiento en Y
    if (!colisionY) {
        posicion.y = nuevaPosicionY.y;
    }

    // Aplicamos la posición final validada a la figura del jugador
    forma.setPosition(posicion);

    if (corriendo) {
        sf::Vector2f recorrido = posicion - posicionAntes;
        float metros = std::sqrt(recorrido.x * recorrido.x + recorrido.y * recorrido.y) / TAMANIO_BLOQUE_JUEGO;
        agregarAgotamiento(metros * 0.1f);
    }
}

// Método para pintar al jugador encima del mundo
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

    float hundimientoVisual = 0.0f;
    if (hundido) {
        hundimientoVisual = std::min(24.0f, 8.0f + tiempoHundimiento * 10.0f);
    }

    if (!enAgua) {
        sf::RectangleShape sombra({24.0f, 7.0f});
        sombra.setPosition({posicion.x, posicion.y + 20.0f});
        sombra.setFillColor(sf::Color(8, 18, 12, 80));
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

    int altoVisible = altoFrame;
    if (enAgua) {
        altoVisible = hundido
            ? std::max(6, 18 - static_cast<int>(tiempoHundimiento * 7.0f))
            : 18;
    }

    sf::Sprite sprite(*texturaActiva);
    sprite.setTextureRect(sf::IntRect({columna * 32, fila * 32}, {32, altoVisible}));
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

inline int Jugador::getHambre() const {
    return hambre;
}

inline float Jugador::getSaturacion() const {
    return saturacion;
}

inline float Jugador::getAgotamiento() const {
    return agotamiento;
}

inline float Jugador::getMultiplicadorAtaque(ItemId item) const {
    float recarga = tiempoRecargaAtaqueJugador(item);
    float carga = std::clamp(tiempoDesdeAtaque / recarga, 0.0f, 1.0f);
    return 0.2f + 0.8f * carga;
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
    agregarAgotamiento(0.1f);
    ultimoDanioHP = std::max(ultimoDanioHP, danioHP);
    tiempoInvulnerable = 0.5f;
}

inline void Jugador::curar(int puntosHP) {
    if (puntosHP <= 0 || vidaHP <= 0) {
        return;
    }
    vidaHP = std::min(vidaMaximaHP, vidaHP + puntosHP);
}

inline void Jugador::agregarAgotamiento(float puntos) {
    if (puntos <= 0.0f) {
        return;
    }
    agotamiento += puntos;
}

inline void Jugador::registrarAtaque(ItemId item) {
    (void)item;
    tiempoDesdeAtaque = 0.0f;
    agregarAgotamiento(0.1f);
}

inline bool Jugador::consumirComida(ItemId item) {
    int puntosComida = 0;
    float modificadorSaturacion = 0.0f;
    if (!datosComidaJugador(item, puntosComida, modificadorSaturacion)) {
        return false;
    }
    if (hambre >= 20 && vidaHP >= vidaMaximaHP) {
        return false;
    }

    hambre = std::min(20, hambre + puntosComida);
    saturacion = std::min(
        static_cast<float>(hambre),
        saturacion + static_cast<float>(puntosComida) * modificadorSaturacion * 2.0f
    );
    return true;
}

