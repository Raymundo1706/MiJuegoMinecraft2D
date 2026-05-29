#pragma once

#include <SFML/Graphics.hpp>

// Forward declaration para no generar bucles con el mapa
class Mundo;

enum class TipoAnimal {
    Cerdo,
    Oveja
};

class Animal {
private:
    sf::Vector2f posicion;
    sf::Vector2f velocidad;
    sf::RectangleShape forma;
    TipoAnimal tipo;

    // Inteligencia Artificial básica (Cronómetros de movimiento)
    float tiempoCambiandoDireccion;
    float tiempoMaximoDireccion;

    // Dimensiones del animal (Un poco más pequeños que el bloque de 32x32)
    const float ANCHO_ANIMAL = 20.0f;
    const float ALTO_ANIMAL = 20.0f;

    void elegirNuevaDireccion();

public:
    Animal(float x, float y, TipoAnimal tipo);
    ~Animal();

    void actualizar(float dt, const Mundo& mundo);
    void dibujar(sf::RenderWindow& ventana);

    sf::Vector2f getPosicion() const { return posicion; }
};

