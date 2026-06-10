#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <optional>
#include <vector>
#include "Mundo.hpp"
#include "../entidades/Jugador.hpp"
#include "../entidades/Animal.hpp"
#include "../entidades/Zombie.hpp"
#include "Item.hpp"

struct ItemSuelo {
    ItemId item = ItemId::Ninguno;
    int cantidad = 1;
    sf::Vector2f posicion;
    sf::Vector2f posicionBase;
    sf::Vector2f velocidad;
    float altura = 0.0f;
    float velocidadAltura = 0.0f;
    float tiempo = 0.0f;
    float escala = 1.0f;
    float giroY = 1.0f;
    int rebotesRestantes = 2;
    bool fisicaInicializada = false;
    bool absorbiendo = false;

    ItemSuelo() = default;
    ItemSuelo(ItemId item_, int cantidad_, sf::Vector2f posicion_)
        : item(item_), cantidad(cantidad_), posicion(posicion_), posicionBase(posicion_) {}
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
    std::vector<Zombie*> zombis;
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

#include "../inline/core/Juego.cpp"
