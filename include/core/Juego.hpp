#ifndef JUEGO_HPP
#define JUEGO_HPP

#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include "core/Mundo.hpp"
#include "core/Jugador.hpp"
#include "entidades/Animal.hpp" // <-- IMPORTANTE: Para que conozca TipoAnimal y Animal

class Juego {
private:
    sf::RenderWindow ventana;
    sf::View camara;
    bool estaCorriendo;

    std::unique_ptr<Mundo> mapaSuperficie;
    std::unique_ptr<Jugador> jugador;

    // Elementos de Interfaz (UI)
    sf::Font fuente;
    std::optional<sf::Text> textoCoordenadas;
    bool fuenteCargada;

    // LISTA DE ANIMALES AUTÓNOMOS
    std::vector<Animal*> animales; // <-- IMPORTANTE: Declaración de la lista

public:
    Juego();
    ~Juego();
    void ejecutar();
};

#endif // JUEGO_HPP