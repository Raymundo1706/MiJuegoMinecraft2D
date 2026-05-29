#pragma once

#include <SFML/Graphics.hpp>
#include <map>
#include "Item.hpp"

struct RecetaMatriz {
    ItemId matriz[3][3];

    bool operator<(const RecetaMatriz& otra) const;
};

struct ResultadoCrafteo {
    ItemId item = ItemId::Ninguno;
    int cantidad = 0;
};

class SistemaCrafteo {
private:
    bool menuAbierto;
    ItemId matrizEntrada[3][3];
    ResultadoCrafteo ranuraSalida;
    std::map<RecetaMatriz, ResultadoCrafteo> libroRecetas;

    const float TAMANIO_CUADRO = 40.0f;

public:
    SistemaCrafteo();
    ~SistemaCrafteo();

    void alternarMenu();
    bool esMenuAbierto() const;

    void registrarReceta(RecetaMatriz patron, ItemId resultado, int cantidad);
    void inicializarRecetasBase();
    void verificarCrafteo();
    void manejarClicks(sf::Vector2i posicionMouse, bool clicPresionado, ItemId itemEnMano);
    void dibujar(sf::RenderWindow& ventana, sf::Font& fuente);
};

#include "inline/SistemaCrafteo.cpp"
