#include <algorithm>
#include <string>

namespace {
const sf::Vector2f PANEL_POS(32.0f, 24.0f);
const sf::Vector2f PANEL_SIZE(736.0f, 552.0f);
const sf::Vector2f PLAYER_POS(122.0f, 64.0f);
const sf::Vector2f PLAYER_SIZE(170.0f, 206.0f);

inline sf::Vector2f posicionSlot(int indice) {
    const float paso = 48.0f;

    if (indice >= 0 && indice < 27) {
        int fila = indice / 9;
        int col = indice % 9;
        return {184.0f + col * paso, 334.0f + fila * paso};
    }

    if (indice >= 27 && indice < 36) {
        int col = indice - 27;
        return {184.0f + col * paso, 502.0f};
    }

    if (indice >= 36 && indice < 40) {
        int fila = indice - 36;
        return {70.0f, 64.0f + fila * paso};
    }

    if (indice == 40) {
        return {314.0f, 208.0f};
    }

    if (indice >= 41 && indice < 45) {
        int local = indice - 41;
        int fila = local / 2;
        int col = local % 2;
        return {476.0f + col * paso, 116.0f + fila * paso};
    }

    return {640.0f, 140.0f};
}

inline sf::Color colorDeItem(ItemId item) {
    switch (item) {
        case ItemId::BloquePasto: return sf::Color(55, 150, 65);
        case ItemId::BloqueTierra: return sf::Color(120, 78, 45);
        case ItemId::BloqueTronco: return sf::Color(110, 70, 35);
        case ItemId::TablonMadera: return sf::Color(185, 130, 65);
        case ItemId::BloquePiedra: return sf::Color(120, 120, 120);
        case ItemId::MineralHierro: return sf::Color(202, 172, 120);
        case ItemId::MineralOro: return sf::Color(245, 204, 75);
        case ItemId::MineralDiamante: return sf::Color(70, 220, 235);
        case ItemId::Redstone: return sf::Color(190, 30, 30);
        case ItemId::Cristal: return sf::Color(180, 235, 255, 210);
        case ItemId::Lana: return sf::Color(235, 235, 235);
        case ItemId::PaloMadera: return sf::Color(160, 100, 45);
        case ItemId::MesaCrafteo: return sf::Color(130, 85, 45);
        case ItemId::Horno: return sf::Color(85, 85, 85);
        case ItemId::Cama: return sf::Color(200, 70, 70);
        case ItemId::BloqueTecho: return sf::Color(85, 85, 105);
        case ItemId::PicoMadera:
        case ItemId::PicoPiedra:
        case ItemId::PicoDiamante:
        case ItemId::PalaMadera:
        case ItemId::PalaPiedra:
        case ItemId::HachaMadera:
        case ItemId::HachaPiedra:
        case ItemId::EspadaMadera:
        case ItemId::EspadaPiedra:
            return sf::Color(210, 170, 90);
        default: return sf::Color(90, 150, 90);
    }
}

inline std::string inicialItem(ItemId item) {
    switch (item) {
        case ItemId::BloquePasto: return "Pa";
        case ItemId::BloqueTierra: return "Ti";
        case ItemId::BloqueTronco: return "Tr";
        case ItemId::TablonMadera: return "Ta";
        case ItemId::BloquePiedra: return "Pi";
        case ItemId::MineralHierro: return "Fe";
        case ItemId::MineralOro: return "Au";
        case ItemId::MineralDiamante: return "Di";
        case ItemId::Redstone: return "Rs";
        case ItemId::Cristal: return "Cr";
        case ItemId::Lana: return "La";
        case ItemId::PaloMadera: return "Pl";
        case ItemId::MesaCrafteo: return "MC";
        case ItemId::Horno: return "Ho";
        case ItemId::Cama: return "Ca";
        case ItemId::BloqueTecho: return "Te";
        case ItemId::PicoMadera: return "Pk";
        case ItemId::PicoPiedra: return "Pp";
        case ItemId::PicoDiamante: return "Pd";
        case ItemId::PalaMadera: return "Pa";
        case ItemId::PalaPiedra: return "Pp";
        case ItemId::HachaMadera: return "Ha";
        case ItemId::HachaPiedra: return "Hp";
        case ItemId::EspadaMadera: return "Em";
        case ItemId::EspadaPiedra: return "Ep";
        default: return "?";
    }
}

inline bool contiene(sf::Vector2f pos, float tam, sf::Vector2i mouse) {
    return mouse.x >= pos.x && mouse.x <= pos.x + tam &&
           mouse.y >= pos.y && mouse.y <= pos.y + tam;
}
}

inline InventarioGrid::InventarioGrid()
    : menuAbierto(false),
      slotSeleccionadoHotbar(0),
      manteniendoItem(false),
      clicIzquierdoAnterior(false),
      clicDerechoAnterior(false) {
    slots.resize(TOTAL_SLOTS);
}

inline InventarioGrid::~InventarioGrid() {}

inline void InventarioGrid::alternarMenu() {
    if (menuAbierto) {
        devolverCrafteoAlInventario();
    }
    menuAbierto = !menuAbierto;
    clicIzquierdoAnterior = false;
    clicDerechoAnterior = false;
}

inline bool InventarioGrid::esMenuAbierto() const {
    return menuAbierto;
}

inline void InventarioGrid::seleccionarSlotHotbar(int slot) {
    if (slot >= 0 && slot < SLOTS_HOTBAR) {
        slotSeleccionadoHotbar = slot;
    }
}

inline ItemId InventarioGrid::getItemEnHotbar() const {
    return slots[INDICE_HOTBAR + slotSeleccionadoHotbar].item;
}

inline TipoBloque InventarioGrid::getTipoEnHotbar() const {
    return bloqueDesdeItem(getItemEnHotbar());
}

inline bool InventarioGrid::consumirItemHotbar(int cantidad) {
    int indice = INDICE_HOTBAR + slotSeleccionadoHotbar;
    if (cantidad <= 0 || esItemVacio(slots[indice].item) || slots[indice].cantidad < cantidad) {
        return false;
    }

    slots[indice].cantidad -= cantidad;
    limpiarSlotSiVacio(slots[indice]);
    return true;
}

inline int InventarioGrid::maxStack(ItemId item) const {
    return maxStackItem(item);
}

inline bool InventarioGrid::esSlotResultado(int indice) const {
    return indice == INDICE_RESULTADO;
}

inline bool InventarioGrid::esSlotPersistente(int indice) const {
    return indice >= 0 && indice < INDICE_CRAFTEO;
}

inline bool InventarioGrid::puedeColocarEnSlot(int indice, ItemId item) const {
    if (indice < 0 || indice >= TOTAL_SLOTS || esSlotResultado(indice)) return false;
    if (esItemVacio(item)) return false;
    return true;
}

inline void InventarioGrid::limpiarSlotSiVacio(SlotInventario& slot) {
    if (slot.cantidad <= 0) {
        slot.item = ItemId::Ninguno;
        slot.cantidad = 0;
    }
}

inline void InventarioGrid::agregarItem(ItemId item, int cantidad) {
    if (esItemVacio(item) || cantidad <= 0) return;

    int restante = cantidad;
    for (int i = 0; i < INDICE_CRAFTEO && restante > 0; ++i) {
        if (slots[i].item == item && slots[i].cantidad < maxStack(item)) {
            int espacio = maxStack(item) - slots[i].cantidad;
            int mover = std::min(espacio, restante);
            slots[i].cantidad += mover;
            restante -= mover;
        }
    }

    for (int i = 0; i < INDICE_CRAFTEO && restante > 0; ++i) {
        if (esItemVacio(slots[i].item)) {
            int mover = std::min(maxStack(item), restante);
            slots[i].item = item;
            slots[i].cantidad = mover;
            restante -= mover;
        }
    }
}

inline void InventarioGrid::agregarItem(TipoBloque bloque, int cantidad) {
    agregarItem(itemDesdeBloque(bloque), cantidad);
}

inline int InventarioGrid::obtenerSlotEnPosicion(sf::Vector2i posicionMouse) const {
    if (!menuAbierto) {
        return -1;
    }

    for (int i = 0; i < TOTAL_SLOTS; ++i) {
        if (contiene(posicionSlot(i), TAMANIO_CUADRO, posicionMouse)) return i;
    }
    return -1;
}

inline void InventarioGrid::actualizarResultadoCrafteo() {
    slots[INDICE_RESULTADO] = {};

    int cantidadTroncos = 0;
    int cantidadTablones = 0;
    int materiales = 0;
    for (int i = INDICE_CRAFTEO; i < INDICE_RESULTADO; ++i) {
        if (!esItemVacio(slots[i].item)) {
            ++materiales;
            if (slots[i].item == ItemId::BloqueTronco) ++cantidadTroncos;
            if (slots[i].item == ItemId::TablonMadera) ++cantidadTablones;
        }
    }

    if (materiales == 1 && cantidadTroncos == 1) {
        slots[INDICE_RESULTADO] = {ItemId::TablonMadera, 4};
        return;
    }

    if (materiales == 4 && cantidadTablones == 4) {
        slots[INDICE_RESULTADO] = {ItemId::MesaCrafteo, 1};
        return;
    }

    bool palosColumnaIzquierda = slots[INDICE_CRAFTEO].item == ItemId::TablonMadera &&
                                 slots[INDICE_CRAFTEO + 2].item == ItemId::TablonMadera;
    bool palosColumnaDerecha = slots[INDICE_CRAFTEO + 1].item == ItemId::TablonMadera &&
                               slots[INDICE_CRAFTEO + 3].item == ItemId::TablonMadera;
    if (materiales == 2 && cantidadTablones == 2 && (palosColumnaIzquierda || palosColumnaDerecha)) {
        slots[INDICE_RESULTADO] = {ItemId::PaloMadera, 4};
    }
}

inline void InventarioGrid::consumirIngredientesCrafteo() {
    for (int i = INDICE_CRAFTEO; i < INDICE_RESULTADO; ++i) {
        if (!esItemVacio(slots[i].item)) {
            slots[i].cantidad -= 1;
            limpiarSlotSiVacio(slots[i]);
        }
    }
    actualizarResultadoCrafteo();
}

inline void InventarioGrid::devolverCrafteoAlInventario() {
    for (int i = INDICE_CRAFTEO; i < INDICE_RESULTADO; ++i) {
        if (!esItemVacio(slots[i].item)) {
            agregarItem(slots[i].item, slots[i].cantidad);
            slots[i] = {};
        }
    }
    slots[INDICE_RESULTADO] = {};
}

inline void InventarioGrid::manejarClickIzquierdo(int indice) {
    if (indice < 0 || indice >= TOTAL_SLOTS) return;

    if (esSlotResultado(indice)) {
        actualizarResultadoCrafteo();
        if (esItemVacio(slots[indice].item)) return;

        if (!manteniendoItem) {
            itemCursor = slots[indice];
            manteniendoItem = true;
            consumirIngredientesCrafteo();
        } else if (itemCursor.item == slots[indice].item && itemCursor.cantidad < maxStack(itemCursor.item)) {
            int espacio = maxStack(itemCursor.item) - itemCursor.cantidad;
            int mover = std::min(espacio, slots[indice].cantidad);
            itemCursor.cantidad += mover;
            if (mover > 0) consumirIngredientesCrafteo();
        }
        return;
    }

    SlotInventario& slot = slots[indice];

    if (!manteniendoItem) {
        if (!esItemVacio(slot.item)) {
            itemCursor = slot;
            slot = {};
            manteniendoItem = true;
        }
        actualizarResultadoCrafteo();
        return;
    }

    if (!puedeColocarEnSlot(indice, itemCursor.item)) return;

    if (esItemVacio(slot.item)) {
        slot = itemCursor;
        itemCursor = {};
        manteniendoItem = false;
    } else if (slot.item == itemCursor.item) {
        int espacio = maxStack(slot.item) - slot.cantidad;
        int mover = std::min(espacio, itemCursor.cantidad);
        slot.cantidad += mover;
        itemCursor.cantidad -= mover;
        limpiarSlotSiVacio(itemCursor);
        manteniendoItem = !esItemVacio(itemCursor.item);
    } else {
        std::swap(slot, itemCursor);
    }

    actualizarResultadoCrafteo();
}

inline void InventarioGrid::manejarClickDerecho(int indice) {
    if (indice < 0 || indice >= TOTAL_SLOTS || esSlotResultado(indice)) return;

    SlotInventario& slot = slots[indice];

    if (!manteniendoItem) {
        if (!esItemVacio(slot.item)) {
            int levantar = (slot.cantidad + 1) / 2;
            itemCursor = {slot.item, levantar};
            slot.cantidad -= levantar;
            limpiarSlotSiVacio(slot);
            manteniendoItem = true;
        }
        actualizarResultadoCrafteo();
        return;
    }

    if (!puedeColocarEnSlot(indice, itemCursor.item)) return;

    if (esItemVacio(slot.item)) {
        slot = {itemCursor.item, 1};
        itemCursor.cantidad -= 1;
    } else if (slot.item == itemCursor.item && slot.cantidad < maxStack(slot.item)) {
        slot.cantidad += 1;
        itemCursor.cantidad -= 1;
    }

    limpiarSlotSiVacio(itemCursor);
    manteniendoItem = !esItemVacio(itemCursor.item);
    actualizarResultadoCrafteo();
}

inline void InventarioGrid::manejarClicks(sf::Vector2i posicionMouse, bool clicIzquierdo, bool clicDerecho) {
    if (!menuAbierto) {
        clicIzquierdoAnterior = clicIzquierdo;
        clicDerechoAnterior = clicDerecho;
        return;
    }

    int indice = obtenerSlotEnPosicion(posicionMouse);

    if (clicIzquierdo && !clicIzquierdoAnterior) {
        manejarClickIzquierdo(indice);
    }

    if (clicDerecho && !clicDerechoAnterior) {
        manejarClickDerecho(indice);
    }

    clicIzquierdoAnterior = clicIzquierdo;
    clicDerechoAnterior = clicDerecho;
}

inline void InventarioGrid::dibujar(sf::RenderWindow& ventana, sf::Font& fuente) {
    sf::Vector2i mouse = sf::Mouse::getPosition(ventana);
    int hover = obtenerSlotEnPosicion(mouse);

    if (menuAbierto) {
        sf::RectangleShape fondo(PANEL_SIZE);
        fondo.setPosition(PANEL_POS);
        fondo.setFillColor(sf::Color(198, 198, 198, 245));
        fondo.setOutlineColor(sf::Color(45, 45, 45));
        fondo.setOutlineThickness(4.0f);
        ventana.draw(fondo);

        sf::RectangleShape jugador(PLAYER_SIZE);
        jugador.setPosition(PLAYER_POS);
        jugador.setFillColor(sf::Color(38, 38, 38));
        jugador.setOutlineColor(sf::Color::White);
        jugador.setOutlineThickness(2.0f);
        ventana.draw(jugador);

        sf::RectangleShape cabeza({34.0f, 34.0f});
        cabeza.setPosition({PLAYER_POS.x + 68.0f, PLAYER_POS.y + 34.0f});
        cabeza.setFillColor(sf::Color(185, 55, 55));
        ventana.draw(cabeza);

        sf::Text titulo(fuente, "Crafting", 24);
        titulo.setPosition({474.0f, 70.0f});
        titulo.setFillColor(sf::Color(70, 70, 70));
        ventana.draw(titulo);

        sf::Text flecha(fuente, "->", 34);
        flecha.setPosition({586.0f, 138.0f});
        flecha.setFillColor(sf::Color(110, 110, 110));
        ventana.draw(flecha);
    }

    auto dibujarSlot = [&](int indice) {
        sf::Vector2f pos = posicionSlot(indice);
        sf::RectangleShape marco({TAMANIO_CUADRO, TAMANIO_CUADRO});
        marco.setPosition(pos);
        marco.setFillColor(sf::Color(138, 138, 138));
        marco.setOutlineThickness(2.0f);
        marco.setOutlineColor(indice == INDICE_HOTBAR + slotSeleccionadoHotbar ? sf::Color::Yellow : sf::Color(238, 238, 238));
        ventana.draw(marco);

        if (indice == hover) {
            sf::RectangleShape luz({TAMANIO_CUADRO, TAMANIO_CUADRO});
            luz.setPosition(pos);
            luz.setFillColor(sf::Color(255, 255, 255, 70));
            ventana.draw(luz);
        }

        if (!esItemVacio(slots[indice].item)) {
            sf::RectangleShape icono({26.0f, 26.0f});
            icono.setPosition({pos.x + 7.0f, pos.y + 6.0f});
            icono.setFillColor(colorDeItem(slots[indice].item));
            ventana.draw(icono);

            sf::Text inicial(fuente, inicialItem(slots[indice].item), 10);
            inicial.setPosition({pos.x + 9.0f, pos.y + 9.0f});
            inicial.setFillColor(sf::Color::White);
            ventana.draw(inicial);

            if (slots[indice].cantidad > 1) {
                sf::Text sombra(fuente, std::to_string(slots[indice].cantidad), 12);
                sombra.setPosition({pos.x + 25.0f, pos.y + 25.0f});
                sombra.setFillColor(sf::Color::Black);
                ventana.draw(sombra);

                sf::Text cantidad(fuente, std::to_string(slots[indice].cantidad), 12);
                cantidad.setPosition({pos.x + 24.0f, pos.y + 24.0f});
                cantidad.setFillColor(sf::Color::White);
                ventana.draw(cantidad);
            }
        }
    };

    if (menuAbierto) {
        for (int i = 0; i < TOTAL_SLOTS; ++i) {
            dibujarSlot(i);
        }
    } else {
        for (int i = INDICE_HOTBAR; i < INDICE_HOTBAR + SLOTS_HOTBAR; ++i) {
            dibujarSlot(i);
        }
    }

    if (manteniendoItem && !esItemVacio(itemCursor.item)) {
        sf::RectangleShape icono({28.0f, 28.0f});
        icono.setPosition({static_cast<float>(mouse.x - 14), static_cast<float>(mouse.y - 14)});
        icono.setFillColor(colorDeItem(itemCursor.item));
        icono.setOutlineColor(sf::Color::White);
        icono.setOutlineThickness(1.0f);
        ventana.draw(icono);

        if (itemCursor.cantidad > 1) {
            sf::Text cantidad(fuente, std::to_string(itemCursor.cantidad), 12);
            cantidad.setPosition({static_cast<float>(mouse.x + 4), static_cast<float>(mouse.y + 4)});
            cantidad.setFillColor(sf::Color::White);
            ventana.draw(cantidad);
        }
    }
}

inline bool InventarioGrid::tieneItemEnMano() const {
    return manteniendoItem;
}

inline SlotInventario& InventarioGrid::getSlotArrastrando() {
    return itemCursor;
}

inline void InventarioGrid::soltarItemEnMano() {
    itemCursor = {};
    manteniendoItem = false;
}

inline std::vector<SlotInventario>& InventarioGrid::getSlots() {
    return slots;
}


