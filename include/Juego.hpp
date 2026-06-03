#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <optional>
#include <vector>
#include "Mundo.hpp"
#include "Jugador.hpp"
#include "Animal.hpp"
#include "Item.hpp"

struct ItemSuelo {
    ItemId item = ItemId::Ninguno;
    int cantidad = 1;
    sf::Vector2f posicion;
};

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
    std::vector<ItemSuelo> itemsSuelo;

    float worldTime;
    int skyLight;
    bool spawnHostilesHabilitado;

    void actualizarTiempo(float dt);
    bool puedeDormir() const;
    void saltarAlAmanecer();

public:
    Juego();
    ~Juego();
    void ejecutar();
};

#include "inline/Juego.cpp"
