#pragma once

#include <SFML/Graphics.hpp>

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
    float tiempoCambiandoDireccion;
    float tiempoMaximoDireccion;

    const float ANCHO_ANIMAL = 20.0f;
    const float ALTO_ANIMAL = 20.0f;

    void elegirNuevaDireccion();

public:
    Animal(float x, float y, TipoAnimal tipo);
    ~Animal();

    void actualizar(float dt, const Mundo& mundo);
    void dibujar(sf::RenderWindow& ventana);

    sf::Vector2f getPosicion() const;
};

#include "inline/Animal.cpp"
