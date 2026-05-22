#ifndef JUEGO_HPP
#define JUEGO_HPP

#include <SFML/Graphics.hpp>
#include "Mundo.hpp"
#include <memory>

class Juego {
private:
    sf::RenderWindow ventana;
    bool estaCorriendo;

    // Aquí declaramos la variable que el compilador no encontraba
    std::unique_ptr<Mundo> mapaSuperficie;

public:
    Juego();
    ~Juego();
    void ejecutar();
};

#endif // JUEGO_HPP