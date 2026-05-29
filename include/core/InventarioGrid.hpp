#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include "Mundo.hpp"

struct SlotInventario {
    TipoBloque tipo = TipoBloque::Aire;
    int cantidad = 0;
};

class InventarioGrid {
private:
    std::vector<SlotInventario> slots;
    bool menuAbierto;
    int slotSeleccionadoHotbar;

    SlotInventario itemCursor;
    bool manteniendoItem;
    bool clicIzquierdoAnterior;
    bool clicDerechoAnterior;

    static constexpr int SLOTS_INVENTARIO_PRINCIPAL = 27;
    static constexpr int SLOTS_HOTBAR = 9;
    static constexpr int INDICE_HOTBAR = 27;
    static constexpr int INDICE_ARMADURA = 36;
    static constexpr int INDICE_SEGUNDA_MANO = 40;
    static constexpr int INDICE_CRAFTEO = 41;
    static constexpr int INDICE_RESULTADO = 45;
    static constexpr int TOTAL_SLOTS = 46;

    const float TAMANIO_CUADRO = 40.0f;
    const float ESPACIADO = 48.0f;

    int obtenerSlotEnPosicion(sf::Vector2i posicionMouse) const;
    int maxStack(TipoBloque tipo) const;
    bool esSlotResultado(int indice) const;
    bool esSlotPersistente(int indice) const;
    bool puedeColocarEnSlot(int indice, TipoBloque tipo) const;
    void limpiarSlotSiVacio(SlotInventario& slot);
    void manejarClickIzquierdo(int indice);
    void manejarClickDerecho(int indice);
    void actualizarResultadoCrafteo();
    void consumirIngredientesCrafteo();
    void devolverCrafteoAlInventario();

public:
    InventarioGrid();
    ~InventarioGrid();

    void alternarMenu();
    bool esMenuAbierto() const;

    void agregarItem(TipoBloque tipo, int cantidad = 1);

    void manejarClicks(sf::Vector2i posicionMouse, bool clicIzquierdo, bool clicDerecho);
    void dibujar(sf::RenderWindow& ventana, sf::Font& fuente);

    TipoBloque getTipoEnHotbar() const;
    void seleccionarSlotHotbar(int slot);

    bool tieneItemEnMano() const { return manteniendoItem; }
    SlotInventario& getSlotArrastrando() { return itemCursor; }
    void soltarItemEnMano() { itemCursor = {}; manteniendoItem = false; }
    std::vector<SlotInventario>& getSlots() { return slots; }
};
