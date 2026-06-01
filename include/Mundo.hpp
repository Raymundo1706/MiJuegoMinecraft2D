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
    MesaCrafteo,
    Horno,
    Cristal,
    MineralHierro,
    MineralPlata,
    MineralOro,
    MineralDiamante,
    Redstone,
    TierraArada,
    CuevaEntrada
};

enum class TipoBioma {
    Pradera,
    Bosque,
    Seco,
    Montana
};

struct Bloque {
    TipoBloque tipo = TipoBloque::Aire;
    bool esSolido = false;
    float vida = 0.0f;
    bool estaHidratado = false;
    float tiempoMinaRestante = 0.0f;
    bool minaAbierta = false;
    int troncosAlTalar = 1;
    float vidaMaxima = 0.0f;
    TipoBioma bioma = TipoBioma::Pradera;
    int varianteArbol = 0;
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
    bool puedeColocarBloque(int x, int y) const;
    bool colocarBloque(int x, int y, TipoBloque tipo);
    bool ararTierra(int x, int y);
    bool crearEntradaMina(int x, int y);
    bool talarArbol(int x, int y, float segundosTrabajo, int& troncosObtenidos, bool& soltoSemilla);
    float getProgresoTala(int x, int y) const;
    bool picarEntradaMina(int x, int y, float segundosTrabajo);
    bool esMinaAbierta(int x, int y) const;
    float getTiempoMinaRestante(int x, int y) const;
    float getProgresoMina(int x, int y) const;
    void romperBloque(int x, int y);
    TipoBloque getTipoBloque(int x, int y) const;
    sf::Color getColorMapa(int x, int y) const;
    int getVidaMaximaBloque(TipoBloque tipo) const;
    bool daniarBloque(int x, int y, float cantidadDanio);

    int getAncho() const;
    int getAlto() const;
};

#include "inline/Mundo.cpp"
