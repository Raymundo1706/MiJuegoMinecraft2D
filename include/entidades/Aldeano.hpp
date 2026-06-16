#pragma once

#include <SFML/Graphics.hpp>
#include <array>

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
    std::array<int, 6> usosOferta;
    std::array<float, 6> bloqueoOferta;

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
    bool contienePunto(sf::Vector2f punto) const;
    bool ofertaBloqueada(int indice) const;
    void registrarTrade(int indice);
};

#include "../inline/entidades/Aldeano.cpp"
