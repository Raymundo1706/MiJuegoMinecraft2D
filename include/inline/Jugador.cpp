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
    const float escala = 2.0f;
    int fase = caminando ? (static_cast<int>(tiempoAnimacion * 8.0f) % 4) : 0;
    int paso = (fase == 0 || fase == 3) ? 1 : (fase == 1 ? 0 : -1);
    float bob = caminando ? (fase == 1 || fase == 3 ? -1.0f : 0.0f) : 0.0f;
    sf::Vector2f origen(posicion.x - 6.0f, posicion.y - 8.0f + bob);

    sf::Color piel(239, 171, 122);
    sf::Color pielLuz(255, 194, 146);
    sf::Color pielSombra(188, 101, 62);
    sf::Color pielOscura(139, 72, 45);
    sf::Color pelo(77, 47, 25);
    sf::Color peloOscuro(53, 31, 18);
    sf::Color peloClaro(114, 82, 52);
    sf::Color camisa(21, 151, 174);
    sf::Color camisaLuz(42, 207, 211);
    sf::Color camisaSombra(12, 101, 128);
    sf::Color pantalon(56, 82, 88);
    sf::Color pantalonLuz(95, 126, 129);
    sf::Color zapato(35, 54, 54);
    sf::Color ojo(64, 55, 210);

    auto sombraSuelo = [&]() {
        sf::RectangleShape sombra({28.0f, 8.0f});
        sombra.setPosition({posicion.x - 2.0f, posicion.y + 21.0f});
        sombra.setFillColor(sf::Color(8, 18, 12, 80));
        ventana.draw(sombra);
    };

    auto dibujarCabezaFrente = [&]() {
        dibujarRectPixel(ventana, origen, 5, 0, 12, 10, piel, escala);
        dibujarRectPixel(ventana, origen, 5, 0, 12, 2, peloOscuro, escala);
        dibujarRectPixel(ventana, origen, 5, 2, 2, 5, pelo, escala);
        dibujarRectPixel(ventana, origen, 7, 2, 3, 2, peloClaro, escala);
        dibujarRectPixel(ventana, origen, 14, 2, 3, 6, pelo, escala);
        dibujarRectPixel(ventana, origen, 6, 3, 3, 3, pielLuz, escala);
        dibujarRectPixel(ventana, origen, 9, 5, 2, 1, sf::Color::White, escala);
        dibujarPixel(ventana, origen, 10, 5, ojo, escala);
        dibujarRectPixel(ventana, origen, 13, 5, 2, 1, sf::Color::White, escala);
        dibujarPixel(ventana, origen, 14, 5, ojo, escala);
        dibujarRectPixel(ventana, origen, 11, 7, 5, 2, pelo, escala);
        dibujarRectPixel(ventana, origen, 8, 8, 3, 1, pielSombra, escala);
        dibujarRectPixel(ventana, origen, 5, 8, 12, 2, pielOscura, escala);
        dibujarRectPixel(ventana, origen, 8, 8, 3, 1, piel, escala);
    };

    auto dibujarCabezaEspalda = [&]() {
        dibujarRectPixel(ventana, origen, 5, 0, 12, 10, pelo, escala);
        dibujarRectPixel(ventana, origen, 5, 0, 12, 2, peloOscuro, escala);
        dibujarRectPixel(ventana, origen, 5, 2, 2, 8, peloOscuro, escala);
        dibujarRectPixel(ventana, origen, 14, 2, 3, 8, peloOscuro, escala);
        dibujarRectPixel(ventana, origen, 8, 3, 6, 3, peloClaro, escala);
        dibujarRectPixel(ventana, origen, 7, 7, 8, 3, peloOscuro, escala);
    };

    auto dibujarCabezaLado = [&](bool derecha) {
        dibujarRectPixel(ventana, origen, 6, 1, 10, 9, piel, escala);
        dibujarRectPixel(ventana, origen, 6, 1, 10, 2, peloOscuro, escala);
        dibujarRectPixel(ventana, origen, derecha ? 6 : 14, 3, 2, 5, pelo, escala);
        dibujarRectPixel(ventana, origen, derecha ? 8 : 7, 3, 4, 2, peloClaro, escala);
        dibujarRectPixel(ventana, origen, derecha ? 13 : 8, 5, 2, 1, sf::Color::White, escala);
        dibujarPixel(ventana, origen, derecha ? 14 : 8, 5, ojo, escala);
        dibujarRectPixel(ventana, origen, derecha ? 12 : 8, 7, 4, 2, pelo, escala);
        dibujarRectPixel(ventana, origen, derecha ? 15 : 6, 4, 1, 4, pielLuz, escala);
        dibujarRectPixel(ventana, origen, derecha ? 6 : 15, 8, 3, 2, pielOscura, escala);
    };

    auto dibujarCuerpo = [&]() {
        int brazoFrente = 10 + paso;
        int brazoAtras = 10 - paso;
        int piernaFrente = 15 - paso;
        int piernaAtras = 15 + paso;

        if (direccionMirada == DireccionMirada::Arriba) {
            dibujarRectPixel(ventana, origen, 5, brazoAtras, 3, 6, pielSombra, escala);
            dibujarRectPixel(ventana, origen, 15, brazoFrente, 3, 6, pielSombra, escala);
            dibujarRectPixel(ventana, origen, 7, 10, 9, 6, camisaSombra, escala);
            dibujarRectPixel(ventana, origen, 8, 10, 8, 2, camisa, escala);
        } else if (direccionMirada == DireccionMirada::Izquierda) {
            dibujarRectPixel(ventana, origen, 5, brazoFrente, 3, 6, piel, escala);
            dibujarRectPixel(ventana, origen, 14, brazoAtras, 3, 6, pielSombra, escala);
            dibujarRectPixel(ventana, origen, 7, 10, 9, 6, camisa, escala);
            dibujarRectPixel(ventana, origen, 7, 11, 5, 2, camisaLuz, escala);
        } else if (direccionMirada == DireccionMirada::Derecha) {
            dibujarRectPixel(ventana, origen, 5, brazoAtras, 3, 6, pielSombra, escala);
            dibujarRectPixel(ventana, origen, 14, brazoFrente, 3, 6, piel, escala);
            dibujarRectPixel(ventana, origen, 7, 10, 9, 6, camisa, escala);
            dibujarRectPixel(ventana, origen, 10, 11, 6, 2, camisaLuz, escala);
        } else {
            dibujarRectPixel(ventana, origen, 5, brazoFrente, 3, 6, piel, escala);
            dibujarRectPixel(ventana, origen, 15, brazoAtras, 3, 6, piel, escala);
            dibujarRectPixel(ventana, origen, 7, 10, 9, 6, camisa, escala);
            dibujarRectPixel(ventana, origen, 8, 11, 8, 2, camisaLuz, escala);
        }

        dibujarRectPixel(ventana, origen, 5, brazoFrente, 3, 2, camisaSombra, escala);
        dibujarRectPixel(ventana, origen, 15, brazoAtras, 3, 2, camisaSombra, escala);

        dibujarRectPixel(ventana, origen, 7, piernaFrente, 4, 6, pantalon, escala);
        dibujarRectPixel(ventana, origen, 12, piernaAtras, 4, 6, pantalon, escala);
        dibujarRectPixel(ventana, origen, 8, piernaFrente, 2, 2, pantalonLuz, escala);
        dibujarRectPixel(ventana, origen, 13, piernaAtras, 2, 2, pantalonLuz, escala);
        dibujarRectPixel(ventana, origen, 7, piernaFrente + 5, 4, 1, zapato, escala);
        dibujarRectPixel(ventana, origen, 12, piernaAtras + 5, 4, 1, zapato, escala);
    };

    sombraSuelo();

    if (direccionMirada == DireccionMirada::Arriba) {
        dibujarCuerpo();
        dibujarCabezaEspalda();
    } else if (direccionMirada == DireccionMirada::Izquierda) {
        dibujarCuerpo();
        dibujarCabezaLado(false);
    } else if (direccionMirada == DireccionMirada::Derecha) {
        dibujarCuerpo();
        dibujarCabezaLado(true);
    } else {
        dibujarCuerpo();
        dibujarCabezaFrente();
    }
}

inline sf::Vector2f Jugador::getPosicion() const {
    return posicion;
}

