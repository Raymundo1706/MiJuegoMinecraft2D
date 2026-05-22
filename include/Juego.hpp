#ifndef JUEGO_HPP
#define JUEGO_HPP

#include <SFML/Graphics.hpp>
#include "Mundo.hpp"
#include "Jugador.hpp" // <-- Incluimos el plano del jugador
#include <memory>

class Juego {
private:
    sf::RenderWindow ventana;
    bool estaCorriendo;

    std::unique_ptr<Mundo> mapaSuperficie;
    std::unique_ptr<Jugador> jugador; // <-- Objeto jugador encapsulado

public:
    Juego();
    ~Juego();
    void ejecutar();
};

#endif // JUEGO_HPP