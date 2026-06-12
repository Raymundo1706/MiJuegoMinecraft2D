#pragma once

#include <SFML/Graphics.hpp>

class Mundo;

enum class ProfesionAldeano {
    Granjero,
    Herrero,
    Bibliotecario
};

class Aldeano {
private:
    sf::Vector2f posicion;
    sf::Vector2f velocidad;
    sf::Vector2f objetivo;
    ProfesionAldeano profesion;
    float tiempoDecision;
    float tiempoAnimacion;
    bool refugiandose;
    int direccionMirada;

    bool colisionaConMundo(const Mundo& mundo, sf::Vector2f nuevaPosicion) const;
    bool buscarPuertaCercana(const Mundo& mundo, sf::Vector2f& destino) const;
    void elegirObjetivoDia(const Mundo& mundo);
    void dibujarPixelArt(sf::RenderWindow& ventana) const;

public:
    Aldeano(float x, float y, ProfesionAldeano profesion);
    ~Aldeano();

    void actualizar(float dt, const Mundo& mundo, bool esNoche);
    void dibujar(sf::RenderWindow& ventana) const;

    ProfesionAldeano getProfesion() const;
    sf::Vector2f getPosicion() const;
};

#include "../inline/entidades/Aldeano.cpp"
