#pragma once

#include <SFML/Graphics.hpp>

class Mundo;
class Jugador;

class Zombie {
private:
    sf::Vector2f posicion;
    sf::Vector2f velocidad;
    sf::RectangleShape forma;
    float vida;
    float vidaMaxima;
    float velocidadBase;
    float tiempoAtaque;
    float tiempoQuemadura;
    float tiempoLejos;
    float tiempoAnimacion;
    float tiempoSprite;
    float tiempoGolpe;
    int direccionMirada;
    bool bebe;
    bool vivo;
    bool temporal;

    bool colisionaConMundo(const Mundo& mundo, sf::Vector2f nuevaPosicion) const;
    bool estaEnAgua(const Mundo& mundo) const;

public:
    Zombie(float x, float y, bool bebe = false);
    ~Zombie();

    void actualizar(float dt, const Mundo& mundo, Jugador& jugador, int skyLight);
    void dibujar(sf::RenderWindow& ventana);
    void recibirDanio(float danio);

    bool estaVivo() const;
    bool debeEliminarse() const;
    bool contienePunto(sf::Vector2f punto) const;
    sf::Vector2f getPosicion() const;
};

#include "../inline/entidades/Zombie.cpp"
