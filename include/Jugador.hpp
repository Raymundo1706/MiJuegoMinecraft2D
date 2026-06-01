#pragma once

#include <SFML/Graphics.hpp>
#include "Item.hpp"

class Mundo;

class Jugador {
private:
    enum class DireccionMirada {
        Abajo,
        Arriba,
        Izquierda,
        Derecha
    };

    sf::Vector2f posicion;
    float velocidad;
    sf::RectangleShape forma;
    DireccionMirada direccionMirada;
    bool caminando;
    float tiempoAnimacion;
    bool accionando;
    float tiempoAccion;
    ItemId itemAccion;
    bool enAgua;
    bool hundido;
    float tiempoEnAgua;
    float tiempoHundimiento;
    float tiempoMojado;
    int vidaMaximaHP;
    int vidaHP;
    float tiempoInvulnerable;
    int ultimoDanioHP;

    void dibujarPixel(sf::RenderWindow& ventana, sf::Vector2f origen, int x, int y, sf::Color color, float escala);
    void dibujarRectPixel(sf::RenderWindow& ventana, sf::Vector2f origen, int x, int y, int ancho, int alto, sf::Color color, float escala);
    void dibujarSpriteJugador(sf::RenderWindow& ventana);

public:
    Jugador(float x, float y);
    ~Jugador();

    void controlar(float dt, const Mundo& mundo);
    void dibujar(sf::RenderWindow& ventana);
    void iniciarAccion(ItemId item);

    sf::Vector2f getPosicion() const;
    bool estaEnAgua() const;
    bool estaHundido() const;
    float getTiempoEnAgua() const;
    int getVidaHP() const;
    int getVidaMaximaHP() const;
    bool estaMuerto() const;
    void recibirDanio(int danioHP);
    void curar(int puntosHP);
};

#include "inline/Jugador.cpp"
