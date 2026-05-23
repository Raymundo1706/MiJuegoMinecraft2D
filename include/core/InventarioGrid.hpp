#ifndef INVENTARIO_GRID_HPP
#define INVENTARIO_GRID_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include "Mundo.hpp" // Para heredar TipoBloque

// Estructura para cada uno de los cuadritos (Slots)
struct SlotInventario {
    TipoBloque tipo = TipoBloque::Aire; // Aire significa ranura vacía
    int cantidad = 0;
};

class InventarioGrid {
private:
    std::vector<SlotInventario> slots;
    bool menuAbierto;
    int slotSeleccionadoHotbar; // Del 0 al 7 (los 8 espacios de acción)

    // Variables para el arrastre de ítems con el mouse (Mover de lugar)
    int slotArrastrando; // Índice del slot que estamos moviendo (-1 si ninguno)
    bool manteniendoItem;

    // Dimensiones visuales
    const float TAMANIO_CUADRO = 40.0f;
    const float MARGEN = 8.0f;

public:
    InventarioGrid();
    ~InventarioGrid();

    // Controles del Menú
    void alternarMenu();
    bool esMenuAbierto() const;

    // Lógica de recolección libre (Busca el primer slot libre o un stack existente)
    void agregarItem(TipoBloque tipo, int cantidad = 1);

    // Interacción del Mouse (Mover stacks de lugar con clic)
    void manejarClicks(sf::Vector2i posicionMouse, bool clicPresionado);

    // Dibujado del Inventario Completo y de la Hotbar
    void dibujar(sf::RenderWindow& ventana, sf::Font& fuente);

    // Utilidades para el juego
    TipoBloque getTipoEnHotbar() const;
    void seleccionarSlotHotbar(int slot);
};

#endif // INVENTARIO_GRID_HPP