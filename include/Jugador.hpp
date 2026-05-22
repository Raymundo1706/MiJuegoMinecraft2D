#ifndef JUGADOR_HPP
#define JUGADOR_HPP

#include <SFML/Graphics.hpp>

// FORWARD DECLARATION: Le avisamos al compilador que la clase Mundo existe
// sin necesidad de meter el include completo aquí, evitando bucles.
class Mundo; 

class Jugador {
private:
    sf::Vector2f posicion;
    float velocidad;
    sf::RectangleShape forma;

public:
    Jugador(float x, float y);
    ~Jugador();

    // Ahora el compilador ya acepta la referencia de Mundo perfectamente
    void controlar(float dt, const Mundo& mundo);
    void dibujar(sf::RenderWindow& ventana);

    // Getters necesarios para la cámara y la interfaz
    sf::Vector2f getPosicion() const { return posicion; }
};

#endif // JUGADOR_HPP