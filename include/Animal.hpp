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
    float vidaMaxima;
    float vida;
    float tiempoPanico;
    float anchoAnimal;
    float altoAnimal;
    bool mirandoDerecha;

    void elegirNuevaDireccion();
    void dibujarCerdo(sf::RenderWindow& ventana);

public:
    Animal(float x, float y, TipoAnimal tipo);
    ~Animal();

    void actualizar(float dt, const Mundo& mundo);
    void dibujar(sf::RenderWindow& ventana);
    void recibirDanio(float danio);
    bool estaVivo() const;

    sf::Vector2f getPosicion() const;
};

#include "inline/Animal.cpp"
