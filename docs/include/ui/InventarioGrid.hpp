#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include "../core/Item.hpp"

struct SlotInventario {
    ItemId item = ItemId::Ninguno;
    int cantidad = 0;
};

class InventarioGrid {
private:
    std::vector<SlotInventario> slots;
    bool menuAbierto;
    bool mesaCrafteoAbierta;
    int categoriaMesa;
    int recetaMesaSeleccionada;
    int slotSeleccionadoHotbar;

    SlotInventario itemCursor;
    bool manteniendoItem;
    bool clicIzquierdoAnterior;
    bool clicDerechoAnterior;
    bool enterCatalogoAnterior;
    bool eventoFabricacionCatalogo;
    bool eventoMovimientoItem;

    static constexpr int SLOTS_INVENTARIO_PRINCIPAL = 27;
    static constexpr int SLOTS_HOTBAR = 9;
    static constexpr int INDICE_HOTBAR = 27;
    static constexpr int INDICE_ARMADURA = 36;
    static constexpr int INDICE_SEGUNDA_MANO = 40;
    static constexpr int INDICE_CRAFTEO = 41;
    static constexpr int INDICE_RESULTADO = 45;
    static constexpr int INDICE_MESA_CRAFTEO = 46;
    static constexpr int INDICE_RESULTADO_MESA = 55;
    static constexpr int TOTAL_SLOTS = 56;

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
    void manejarShiftClick(int indice);
    void actualizarResultadoCrafteo();
    void consumirIngredientesCrafteo();
    void devolverCrafteoAlInventario();
    void actualizarResultadoMesa();
    void consumirIngredientesMesa();
    void devolverMesaAlInventario();
    int contarItemBanco(ItemId item) const;
    bool tieneIngredientesCatalogo(const std::vector<SlotInventario>& ingredientes) const;
    bool consumirIngredientesCatalogo(const std::vector<SlotInventario>& ingredientes);
    bool fabricarRecetaCatalogo(int indiceReceta);
    int fabricarRecetaCatalogoMaximo(int indiceReceta);
    void manejarClickCatalogoMesa(sf::Vector2i posicionMouse);

public:
    InventarioGrid();
    ~InventarioGrid();

    void alternarMenu();
    bool esMenuAbierto() const;
    void abrirMesaCrafteo();
    void cerrarMesaCrafteo();
    bool esMesaCrafteoAbierta() const;

    void agregarItem(ItemId item, int cantidad = 1);
    void agregarItem(TipoBloque bloque, int cantidad = 1);
    bool puedeAgregarItem(ItemId item, int cantidad = 1) const;
    bool consumirEventoFabricacion();
    bool consumirEventoMovimientoItem();

    void manejarClicks(sf::Vector2i posicionMouse, bool clicIzquierdo, bool clicDerecho);
    void dibujar(sf::RenderWindow& ventana, sf::Font& fuente);

    ItemId getItemEnHotbar() const;
    ItemId getItemSegundaMano() const;
    TipoBloque getTipoEnHotbar() const;
    void seleccionarSlotHotbar(int slot);
    bool consumirItemHotbar(int cantidad = 1);
    SlotInventario extraerItemHotbar(int cantidad = 1);
    SlotInventario extraerItemCursor(int cantidad = 1);
    SlotInventario extraerItemEnSlot(int indice, int cantidad = 1);
    std::vector<SlotInventario> extraerTodosItemsParaMuerte();
    int getSlotHover(sf::Vector2i posicionMouse) const;
    void intercambiarConSegundaMano();

    bool tieneItemEnMano() const;
    SlotInventario& getSlotArrastrando();
    void soltarItemEnMano();
    std::vector<SlotInventario>& getSlots();
};

#include "../inline/ui/InventarioGrid.cpp"
