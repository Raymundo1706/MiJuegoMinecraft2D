#pragma once

#include <map>
#include "Item.hpp"

class SistemaHerramientas {
private:
    int herramientaActiva;
    std::map<TipoBloque, int> inventario;

public:
    SistemaHerramientas();
    ~SistemaHerramientas();

    void cambiarHerramienta(int slot);
    int getHerramientaActiva() const;

    float calcularDanio(TipoBloque tipo) const;
    float calcularDanio(TipoBloque tipo, ItemId itemEnMano) const;
    float calcularTiempoMinado(TipoBloque tipo, ItemId itemEnMano) const;
    bool puedeRecolectar(TipoBloque tipo, ItemId itemEnMano) const;

    void agregarAlInventario(TipoBloque tipo, int herramientaUsada);
    int getCantidad(TipoBloque tipo) const;
};

#include "inline/SistemaHerramientas.cpp"
