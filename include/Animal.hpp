#pragma once

#include <SFML/Graphics.hpp>

class Mundo;
enum class ItemId;

enum class TipoAnimal {
    Cerdo,
    Oveja,
    Vaca,
    Gallina
};

class Animal {
private:
    sf::Vector2f posicion;
    sf::Vector2f velocidad;
    sf::RectangleShape forma;
    TipoAnimal tipo;
    float tiempoCambiandoDireccion;
    float tiempoAnimacion;
    float tiempoMaximoDireccion;
    float vidaMaxima;
    float vida;
    float tiempoPanico;
    float anchoAnimal;
    float altoAnimal;
    bool mirandoDerecha;
    bool muriendo;
    float tiempoMuerte;
    float tiempoGolpe;
    bool tieneAmenaza;
    sf::Vector2f posicionAmenaza;
    float tiempoParticulas;
    bool enAgua;
    bool hundido;
    float tiempoEnAgua;
    float tiempoHundimiento;

    void elegirNuevaDireccion();
    void dibujarAnimal(sf::RenderWindow& ventana);
    void dibujarParticulasMuerte(sf::RenderWindow& ventana);

public:
    Animal(float x, float y, TipoAnimal tipo);
    ~Animal();

    void actualizar(float dt, const Mundo& mundo);
    void actualizar(float dt, const Mundo& mundo, sf::Vector2f posicionJugador, ItemId itemJugador);
    void dibujar(sf::RenderWindow& ventana);
    void recibirDanio(float danio);
    void recibirDanio(float danio, sf::Vector2f origenDanio);
    bool estaVivo() const;
    bool estaMuriendo() const;
    bool muerteFinalizada() const;
    bool contienePunto(sf::Vector2f punto) const;
    TipoAnimal getTipo() const;

    sf::Vector2f getPosicion() const;
};

#include "inline/Animal.cpp"
