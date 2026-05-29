#pragma once

#include <SFML/Graphics.hpp>

class Mundo;

class Jugador {
private:
    sf::Vector2f posicion;
    float velocidad;
    sf::RectangleShape forma;

public:
    Jugador(float x, float y);
    ~Jugador();

    void controlar(float dt, const Mundo& mundo);
    void dibujar(sf::RenderWindow& ventana);

    sf::Vector2f getPosicion() const;
};

#include "inline/Jugador.cpp"
