#pragma once

#include <SFML/Graphics.hpp>
#include <map>
#include "Mundo.hpp"

struct RecetaMatriz {
    TipoBloque matriz[3][3];

    bool operator<(const RecetaMatriz& otra) const;
};

struct ResultadoCrafteo {
    TipoBloque tipo = TipoBloque::Aire;
    int cantidad = 0;
};

class SistemaCrafteo {
private:
    bool menuAbierto;
    TipoBloque matrizEntrada[3][3];
    ResultadoCrafteo ranuraSalida;
    std::map<RecetaMatriz, ResultadoCrafteo> libroRecetas;

    const float TAMANIO_CUADRO = 40.0f;

public:
    SistemaCrafteo();
    ~SistemaCrafteo();

    void alternarMenu();
    bool esMenuAbierto() const;

    void registrarReceta(RecetaMatriz patron, TipoBloque resultado, int cantidad);
    void inicializarRecetasBase();
    void verificarCrafteo();
    void manejarClicks(sf::Vector2i posicionMouse, bool clicPresionado, TipoBloque itemEnMano);
    void dibujar(sf::RenderWindow& ventana, sf::Font& fuente);
};

#include "inline/SistemaCrafteo.cpp"
