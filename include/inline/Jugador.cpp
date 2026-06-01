#include "Mundo.hpp"
#include <cmath>
#include <algorithm>

// Constructor: Recibe la posiciÃ³n aleatoria de spawn
inline Jugador::Jugador(float x, float y) {
    posicion = {x, y};
    velocidad = 250.0f; // PÃ­xeles por segundo
    direccionMirada = DireccionMirada::Abajo;
    caminando = false;
    tiempoAnimacion = 0.0f;

    // TamaÃ±o del personaje: 24x24 pÃ­xeles (cabe perfectamente dentro de un bloque de 32x32)
    forma.setSize({24.0f, 24.0f});
    forma.setFillColor(sf::Color::Red); // Cuadro rojo identificador
    forma.setPosition(posicion);
}

// Destructor
inline Jugador::~Jugador() {}

// MÃ©todo para mover al personaje detectando colisiones sÃ³lidas con el terreno
inline void Jugador::controlar(float dt, const Mundo& mundo) {
    sf::Vector2f direccion(0.0f, 0.0f);

    // DetecciÃ³n de teclas (WASD y Flechas)
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
        direccion.y -= 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
        direccion.y += 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
        direccion.x -= 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
        direccion.x += 1.0f;
    }

    // Si no hay teclas presionadas, no hacemos cÃ¡lculos
    if (direccion.x == 0.0f && direccion.y == 0.0f) {
        caminando = false;
        return;
    }

    caminando = true;
    tiempoAnimacion += dt;

    if (std::abs(direccion.x) > std::abs(direccion.y)) {
        direccionMirada = direccion.x > 0.0f ? DireccionMirada::Derecha : DireccionMirada::Izquierda;
    } else {
        direccionMirada = direccion.y > 0.0f ? DireccionMirada::Abajo : DireccionMirada::Arriba;
    }

    // Normalizamos el vector de direcciÃ³n para evitar que camine mÃ¡s rÃ¡pido en diagonal
    float longitud = std::sqrt(direccion.x * direccion.x + direccion.y * direccion.y);
    direccion /= longitud;

    const float TAMANIO_BLOQUE = 32.0f;
    float anchoJugador = forma.getSize().x;
    float altoJugador = forma.getSize().y;

    // --- COMIENZA EL PASO POR EJES INDEPENDIENTES ---
    
    // 1. INTENTO DE MOVIMIENTO EN EJE X
    sf::Vector2f nuevaPosicionX = posicion;
    nuevaPosicionX.x += direccion.x * velocidad * dt;

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
    nuevaPosicionY.y += direccion.y * velocidad * dt;

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
    dibujarSteve(ventana);
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

inline void Jugador::dibujarSteve(sf::RenderWindow& ventana) {
    static bool texturaLista = false;
    static sf::Texture texturaJugador;

    if (!texturaLista) {
        texturaLista = texturaJugador.loadFromFile("assets/player_sprite.png");
        texturaJugador.setSmooth(false);
    }

    sf::RectangleShape sombra({24.0f, 7.0f});
    sombra.setPosition({posicion.x, posicion.y + 20.0f});
    sombra.setFillColor(sf::Color(8, 18, 12, 80));
    ventana.draw(sombra);

    if (!texturaLista) {
        ventana.draw(forma);
        return;
    }

    int fila = 0;
    bool espejarHorizontal = false;
    if (direccionMirada == DireccionMirada::Izquierda) {
        fila = 3;
        espejarHorizontal = true;
    }
    if (direccionMirada == DireccionMirada::Arriba) fila = 2;
    if (direccionMirada == DireccionMirada::Derecha) fila = 3;

    int columna = caminando ? static_cast<int>(tiempoAnimacion * 8.0f) % 4 : 0;
    static const int xs[4] = {1, 23, 45, 67};
    static const int ys[4] = {1, 27, 52, 78};
    static const int hs[4] = {25, 24, 24, 24};

    sf::Sprite sprite(texturaJugador);
    sprite.setTextureRect(sf::IntRect({xs[columna], ys[fila]}, {17, hs[fila]}));
    sprite.setOrigin({8.5f, static_cast<float>(hs[fila])});
    sprite.setPosition({posicion.x + 12.0f, posicion.y + 26.0f});
    sprite.setScale({espejarHorizontal ? -1.35f : 1.35f, 1.35f});
    ventana.draw(sprite);
}

inline sf::Vector2f Jugador::getPosicion() const {
    return posicion;
}

