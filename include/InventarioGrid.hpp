#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include "Item.hpp"

struct SlotInventario {
    ItemId item = ItemId::Ninguno;
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
    int maxStack(ItemId item) const;
    bool esSlotResultado(int indice) const;
    bool esSlotPersistente(int indice) const;
    bool puedeColocarEnSlot(int indice, ItemId item) const;
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

    void agregarItem(ItemId item, int cantidad = 1);
    void agregarItem(TipoBloque bloque, int cantidad = 1);

    void manejarClicks(sf::Vector2i posicionMouse, bool clicIzquierdo, bool clicDerecho);
    void dibujar(sf::RenderWindow& ventana, sf::Font& fuente);

    ItemId getItemEnHotbar() const;
    TipoBloque getTipoEnHotbar() const;
    void seleccionarSlotHotbar(int slot);
    bool consumirItemHotbar(int cantidad = 1);

    bool tieneItemEnMano() const;
    SlotInventario& getSlotArrastrando();
    void soltarItemEnMano();
    std::vector<SlotInventario>& getSlots();
};

#include "inline/InventarioGrid.cpp"
