#ifndef JUGADOR_HPP
#define JUGADOR_HPP

#include <SFML/Graphics.hpp>

class Jugador {
private:
    // Propiedades del jugador encapsuladas
    sf::Vector2f posicion;
    float velocidad;
    sf::RectangleShape forma;

public:
    // Constructor: Define posicion inicial y apariencia
    Jugador(float x, float y);
    
    // Destructor
    ~Jugador();

    // Metodo para leer el teclado y mover al personaje
    void controlar(float dt);

    // Metodo para dibujar al jugador en la ventana
    void dibujar(sf::RenderWindow& ventana);

    // Getters por si el Mundo o el Juego necesitan saber donde esta el jugador
    sf::Vector2f getPosicion() const { return posicion; }
};

#endif // JUGADOR_HPP