#pragma once

#include <SFML/Graphics.hpp>
#include "../core/Item.hpp"

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
    int hambre;
    float saturacion;
    float agotamiento;
    float tiempoRegeneracion;
    float tiempoInanicion;
    float tiempoDesdeAtaque;
    int nivelExperiencia;
    float progresoExperiencia;
    bool corriendo;
    bool agachado;

    void dibujarPixel(sf::RenderWindow& ventana, sf::Vector2f origen, int x, int y, sf::Color color, float escala);
    void dibujarRectPixel(sf::RenderWindow& ventana, sf::Vector2f origen, int x, int y, int ancho, int alto, sf::Color color, float escala);
    void dibujarSpriteJugador(sf::RenderWindow& ventana);
    void actualizarNutricion(float dt);

public:
    Jugador(float x, float y);
    ~Jugador();

    void controlar(float dt, const Mundo& mundo);
    void dibujar(sf::RenderWindow& ventana);
    void iniciarAccion(ItemId item);

    sf::Vector2f getPosicion() const;
    void setPosicion(sf::Vector2f nuevaPosicion);
    bool estaEnAgua() const;
    bool estaHundido() const;
    float getTiempoEnAgua() const;
    float getOxigenoNormalizado() const;
    float getFlashDanioNormalizado() const;
    int getVidaHP() const;
    int getVidaMaximaHP() const;
    int getHambre() const;
    float getSaturacion() const;
    float getAgotamiento() const;
    int getNivelExperiencia() const;
    float getProgresoExperiencia() const;
    float getMultiplicadorAtaque(ItemId item) const;
    bool estaMuerto() const;
    void recibirDanio(int danioHP);
    void aplicarEmpuje(sf::Vector2f direccion, float fuerza, const Mundo& mundo);
    void curar(int puntosHP);
    void agregarAgotamiento(float puntos);
    void agregarExperiencia(float puntos);
    void registrarAtaque(ItemId item);
    bool consumirComida(ItemId item);
};

#include "../inline/entidades/Jugador.cpp"
