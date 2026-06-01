#pragma once

#include <SFML/Graphics.hpp>

class Mundo;

class Jugador {
private:
    enum class DireccionMirada {
        Abajo,
        Arriba,
        Izquierda,
        Derecha
    };

    sf::Vector2f posicion;
    float velocidad;
    sf::RectangleShape forma;
    DireccionMirada direccionMirada;
    bool caminando;
    float tiempoAnimacion;

    void dibujarPixel(sf::RenderWindow& ventana, sf::Vector2f origen, int x, int y, sf::Color color, float escala);
    void dibujarRectPixel(sf::RenderWindow& ventana, sf::Vector2f origen, int x, int y, int ancho, int alto, sf::Color color, float escala);
    void dibujarSteve(sf::RenderWindow& ventana);

public:
    Jugador(float x, float y);
    ~Jugador();

    void controlar(float dt, const Mundo& mundo);
    void dibujar(sf::RenderWindow& ventana);

    sf::Vector2f getPosicion() const;
};

#include "inline/Jugador.cpp"
