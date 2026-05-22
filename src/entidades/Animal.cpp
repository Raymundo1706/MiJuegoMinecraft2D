#include "entidades/Animal.hpp"
#include "core/Mundo.hpp"
#include <random>
#include <cmath>

Animal::Animal(float x, float y, TipoAnimal tipo) 
    : posicion(x, y), tipo(tipo), tiempoCambiandoDireccion(0.0f) {
    
    forma.setSize({ANCHO_ANIMAL, ALTO_ANIMAL});
    
    // Color según la criatura estilo Minecraft
    if (tipo == TipoAnimal::Cerdo) {
        forma.setFillColor(sf::Color(255, 105, 180)); // Rosa Cerdito
    } else {
        forma.setFillColor(sf::Color(240, 240, 240)); // Blanco Oveja
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(1.5f, 4.0f);
    tiempoMaximoDireccion = dis(gen);

    elegirNuevaDireccion();
}

Animal::~Animal() {}

void Animal::elegirNuevaDireccion() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> disDir(0, 4); // 0=Quieto, 1=Izquierda, 2=Derecha, 3=Arriba, 4=Abajo
    std::uniform_real_distribution<> disTiempo(2.0f, 5.0f);

    int dir = disDir(gen);
    float velocidadCaminado = 40.0f; // Velocidad lenta de paseo

    if (dir == 0) velocidad = {0.0f, 0.0f};
    else if (dir == 1) velocidad = {-velocidadCaminado, 0.0f};
    else if (dir == 2) velocidad = {velocidadCaminado, 0.0f};
    else if (dir == 3) velocidad = {0.0f, -velocidadCaminado};
    else if (dir == 4) velocidad = {0.0f, velocidadCaminado};

    tiempoCambiandoDireccion = 0.0f;
    tiempoMaximoDireccion = disTiempo(gen);
}

void Animal::actualizar(float dt, const Mundo& mundo) {
    tiempoCambiandoDireccion += dt;
    if (tiempoCambiandoDireccion >= tiempoMaximoDireccion) {
        elegirNuevaDireccion();
    }

    const float TAMANIO_BLOQUE = 32.0f;

    // --- COLISIÓN EN EJE X ---
    posicion.x += velocidad.x * dt;
    
    int blqIzq = static_cast<int>(posicion.x / TAMANIO_BLOQUE);
    int blqDer = static_cast<int>((posicion.x + ANCHO_ANIMAL) / TAMANIO_BLOQUE);
    int blqArribaY = static_cast<int>(posicion.y / TAMANIO_BLOQUE);
    int blqAbajoY = static_cast<int>((posicion.y + ALTO_ANIMAL) / TAMANIO_BLOQUE);

    if (velocidad.x > 0) { // Moviéndose a la derecha
        if (mundo.esBloqueSolido(blqDer, blqArribaY) || mundo.esBloqueSolido(blqDer, blqAbajoY)) {
            posicion.x = blqDer * TAMANIO_BLOQUE - ANCHO_ANIMAL - 0.1f;
            elegirNuevaDireccion(); // Cambia de rumbo si choca
        }
    } else if (velocidad.x < 0) { // Moviéndose a la izquierda
        if (mundo.esBloqueSolido(blqIzq, blqArribaY) || mundo.esBloqueSolido(blqIzq, blqAbajoY)) {
            posicion.x = (blqIzq + 1) * TAMANIO_BLOQUE + 0.1f;
            elegirNuevaDireccion();
        }
    }

    // --- COLISIÓN EN EJE Y ---
    posicion.y += velocidad.y * dt;
    
    blqIzq = static_cast<int>(posicion.x / TAMANIO_BLOQUE);
    blqDer = static_cast<int>((posicion.x + ANCHO_ANIMAL) / TAMANIO_BLOQUE);
    int blqArriba = static_cast<int>(posicion.y / TAMANIO_BLOQUE);
    int blqAbajo = static_cast<int>((posicion.y + ALTO_ANIMAL) / TAMANIO_BLOQUE);

    if (velocidad.y > 0) { // Moviéndose hacia abajo
        if (mundo.esBloqueSolido(blqIzq, blqAbajo) || mundo.esBloqueSolido(blqDer, blqAbajo)) {
            posicion.y = blqAbajo * TAMANIO_BLOQUE - ALTO_ANIMAL - 0.1f;
            elegirNuevaDireccion();
        }
    } else if (velocidad.y < 0) { // Moviéndose hacia arriba
        if (mundo.esBloqueSolido(blqIzq, blqArriba) || mundo.esBloqueSolido(blqDer, blqArriba)) {
            posicion.y = (blqArriba + 1) * TAMANIO_BLOQUE + 0.1f;
            elegirNuevaDireccion();
        }
    }
}

void Animal::dibujar(sf::RenderWindow& ventana) {
    forma.setPosition(posicion);
    ventana.draw(forma);
}