#pragma once

#include <map>
#include "Mundo.hpp" // Para que conozca el TipoBloque

class SistemaHerramientas {
private:
    int herramientaActiva; // 0 = Mano, 1 = Pico, 2 = Pala, 3 = Hacha
    std::map<TipoBloque, int> inventario;

public:
    SistemaHerramientas();
    ~SistemaHerramientas();

    // Gestión de la Hotbar
    void cambiarHerramienta(int slot);
    int getHerramientaActiva() const;

    // Calcula el daño que hace la herramienta actual al bloque seleccionado
    float calcularDanio(TipoBloque tipo) const;

    // Gestión del Inventario
    void agregarAlInventario(TipoBloque tipo, int herramientaUsada);
    int getCantidad(TipoBloque tipo) const;
};

