#pragma once

#include <vector>
#include <SFML/Graphics.hpp>

enum class TipoBloque {
    Aire,
    Pasto,
    Tierra,
    Piedra,
    Agua,
    AguaProfunda,
    Madera,
    MineralHierro,
    MineralOro,
    MineralDiamante,
    Redstone,
    TierraArada,
    CuevaEntrada
};

struct Bloque {
    TipoBloque tipo;
    bool esSolido;
    float vida;
    bool estaHidratado;
};

class Mundo {
private:
    int ancho;
    int alto;
    std::vector<std::vector<Bloque>> cuadricula;

public:
    Mundo(int ancho, int alto);
    ~Mundo();

    void generarMundo(bool esSubterraneo);
    void dibujar(sf::RenderWindow& ventana);

    bool esBloqueSolido(int x, int y) const;
    void romperBloque(int x, int y);
    TipoBloque getTipoBloque(int x, int y) const;
    int getVidaMaximaBloque(TipoBloque tipo) const;
    bool daniarBloque(int x, int y, float cantidadDanio);

    int getAncho() const;
    int getAlto() const;
};

#include "inline/Mundo.cpp"
