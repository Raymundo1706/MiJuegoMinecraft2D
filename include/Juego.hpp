#ifndef JUEGO_HPP
#define JUEGO_HPP

#include <SFML/Graphics.hpp>
#include "Mundo.hpp"
#include "Jugador.hpp"
#include <memory>

class Juego {
private:
    sf::RenderWindow ventana;
    bool estaCorriendo;

    std::unique_ptr<Mundo> mapaSuperficie;
    std::unique_ptr<Jugador> jugador;
    
    sf::View camara;

    sf::Font fuente;
    // En SFML 3 usamos un puntero inteligente para inicializarlo en cuanto cargue la fuente
    std::unique_ptr<sf::Text> textoCoordenadas; 
    bool fuenteCargada;

public:
    Juego();
    ~Juego();
    void ejecutar();
};

#endif // JUEGO_HPP