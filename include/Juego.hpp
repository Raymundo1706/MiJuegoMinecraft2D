#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <optional>
#include <vector>
#include "Mundo.hpp"
#include "Jugador.hpp"
#include "Animal.hpp"

class Juego {
private:
    sf::RenderWindow ventana;
    sf::View camara;
    bool estaCorriendo;

    std::unique_ptr<Mundo> mapaSuperficie;
    std::unique_ptr<Jugador> jugador;

    sf::Font fuente;
    std::optional<sf::Text> textoCoordenadas;
    bool fuenteCargada;

    std::vector<Animal*> animales;

public:
    Juego();
    ~Juego();
    void ejecutar();
};

#include "inline/Juego.cpp"
