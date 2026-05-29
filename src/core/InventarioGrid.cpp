#include "InventarioGrid.hpp"
#include <algorithm>
#include <string>

namespace {
const sf::Vector2f PANEL_POS(32.0f, 24.0f);
const sf::Vector2f PANEL_SIZE(736.0f, 552.0f);
const sf::Vector2f PLAYER_POS(122.0f, 64.0f);
const sf::Vector2f PLAYER_SIZE(170.0f, 206.0f);

sf::Vector2f posicionSlot(int indice) {
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

sf::Color colorDeItem(TipoBloque tipo) {
    switch (tipo) {
        case TipoBloque::Pasto: return sf::Color(55, 150, 65);
        case TipoBloque::Tierra: return sf::Color(120, 78, 45);
        case TipoBloque::Madera: return sf::Color(145, 92, 42);
        case TipoBloque::Piedra: return sf::Color(120, 120, 120);
        case TipoBloque::MineralHierro: return sf::Color(202, 172, 120);
        case TipoBloque::MineralOro: return sf::Color(245, 204, 75);
        case TipoBloque::MineralDiamante: return sf::Color(70, 220, 235);
        case TipoBloque::Redstone: return sf::Color(190, 30, 30);
        default: return sf::Color(90, 150, 90);
    }
}

std::string inicialItem(TipoBloque tipo) {
    switch (tipo) {
        case TipoBloque::Pasto: return "Pa";
        case TipoBloque::Tierra: return "Ti";
        case TipoBloque::Madera: return "Ma";
        case TipoBloque::Piedra: return "Pi";
        case TipoBloque::MineralHierro: return "Fe";
        case TipoBloque::MineralOro: return "Au";
        case TipoBloque::MineralDiamante: return "Di";
        case TipoBloque::Redstone: return "Rs";
        default: return "?";
    }
}

bool contiene(sf::Vector2f pos, float tam, sf::Vector2i mouse) {
    return mouse.x >= pos.x && mouse.x <= pos.x + tam &&
           mouse.y >= pos.y && mouse.y <= pos.y + tam;
}
}

InventarioGrid::InventarioGrid()
    : menuAbierto(false),
      slotSeleccionadoHotbar(0),
      manteniendoItem(false),
      clicIzquierdoAnterior(false),
      clicDerechoAnterior(false) {
    slots.resize(TOTAL_SLOTS);
}

InventarioGrid::~InventarioGrid() {}

void InventarioGrid::alternarMenu() {
    if (menuAbierto) {
        devolverCrafteoAlInventario();
    }
    menuAbierto = !menuAbierto;
    clicIzquierdoAnterior = false;
    clicDerechoAnterior = false;
}

bool InventarioGrid::esMenuAbierto() const {
    return menuAbierto;
}

void InventarioGrid::seleccionarSlotHotbar(int slot) {
    if (slot >= 0 && slot < SLOTS_HOTBAR) {
        slotSeleccionadoHotbar = slot;
    }
}

TipoBloque InventarioGrid::getTipoEnHotbar() const {
    return slots[INDICE_HOTBAR + slotSeleccionadoHotbar].tipo;
}

int InventarioGrid::maxStack(TipoBloque tipo) const {
    if (tipo == TipoBloque::Aire) return 0;
    return 64;
}

bool InventarioGrid::esSlotResultado(int indice) const {
    return indice == INDICE_RESULTADO;
}

bool InventarioGrid::esSlotPersistente(int indice) const {
    return indice >= 0 && indice < INDICE_CRAFTEO;
}

bool InventarioGrid::puedeColocarEnSlot(int indice, TipoBloque tipo) const {
    if (indice < 0 || indice >= TOTAL_SLOTS || esSlotResultado(indice)) return false;
    if (tipo == TipoBloque::Aire) return false;
    return true;
}

void InventarioGrid::limpiarSlotSiVacio(SlotInventario& slot) {
    if (slot.cantidad <= 0) {
        slot.tipo = TipoBloque::Aire;
        slot.cantidad = 0;
    }
}

void InventarioGrid::agregarItem(TipoBloque tipo, int cantidad) {
    if (tipo == TipoBloque::Aire || tipo == TipoBloque::Agua || cantidad <= 0) return;

    int restante = cantidad;
    for (int i = 0; i < INDICE_CRAFTEO && restante > 0; ++i) {
        if (slots[i].tipo == tipo && slots[i].cantidad < maxStack(tipo)) {
            int espacio = maxStack(tipo) - slots[i].cantidad;
            int mover = std::min(espacio, restante);
            slots[i].cantidad += mover;
            restante -= mover;
        }
    }

    for (int i = 0; i < INDICE_CRAFTEO && restante > 0; ++i) {
        if (slots[i].tipo == TipoBloque::Aire) {
            int mover = std::min(maxStack(tipo), restante);
            slots[i].tipo = tipo;
            slots[i].cantidad = mover;
            restante -= mover;
        }
    }
}

int InventarioGrid::obtenerSlotEnPosicion(sf::Vector2i posicionMouse) const {
    if (!menuAbierto) {
        return -1;
    }

    for (int i = 0; i < TOTAL_SLOTS; ++i) {
        if (contiene(posicionSlot(i), TAMANIO_CUADRO, posicionMouse)) return i;
    }
    return -1;
}

void InventarioGrid::actualizarResultadoCrafteo() {
    slots[INDICE_RESULTADO] = {};

    int cantidadMadera = 0;
    int materiales = 0;
    for (int i = INDICE_CRAFTEO; i < INDICE_RESULTADO; ++i) {
        if (slots[i].tipo != TipoBloque::Aire) {
            ++materiales;
            if (slots[i].tipo == TipoBloque::Madera) {
                ++cantidadMadera;
            }
        }
    }

    if (materiales == 1 && cantidadMadera == 1) {
        slots[INDICE_RESULTADO] = {TipoBloque::Madera, 4};
    }
}

void InventarioGrid::consumirIngredientesCrafteo() {
    for (int i = INDICE_CRAFTEO; i < INDICE_RESULTADO; ++i) {
        if (slots[i].tipo != TipoBloque::Aire) {
            slots[i].cantidad -= 1;
            limpiarSlotSiVacio(slots[i]);
        }
    }
    actualizarResultadoCrafteo();
}

void InventarioGrid::devolverCrafteoAlInventario() {
    for (int i = INDICE_CRAFTEO; i < INDICE_RESULTADO; ++i) {
        if (slots[i].tipo != TipoBloque::Aire) {
            agregarItem(slots[i].tipo, slots[i].cantidad);
            slots[i] = {};
        }
    }
    slots[INDICE_RESULTADO] = {};
}

void InventarioGrid::manejarClickIzquierdo(int indice) {
    if (indice < 0 || indice >= TOTAL_SLOTS) return;

    if (esSlotResultado(indice)) {
        actualizarResultadoCrafteo();
        if (slots[indice].tipo == TipoBloque::Aire) return;

        if (!manteniendoItem) {
            itemCursor = slots[indice];
            manteniendoItem = true;
            consumirIngredientesCrafteo();
        } else if (itemCursor.tipo == slots[indice].tipo && itemCursor.cantidad < maxStack(itemCursor.tipo)) {
            int espacio = maxStack(itemCursor.tipo) - itemCursor.cantidad;
            int mover = std::min(espacio, slots[indice].cantidad);
            itemCursor.cantidad += mover;
            if (mover > 0) consumirIngredientesCrafteo();
        }
        return;
    }

    SlotInventario& slot = slots[indice];

    if (!manteniendoItem) {
        if (slot.tipo != TipoBloque::Aire) {
            itemCursor = slot;
            slot = {};
            manteniendoItem = true;
        }
        actualizarResultadoCrafteo();
        return;
    }

    if (!puedeColocarEnSlot(indice, itemCursor.tipo)) return;

    if (slot.tipo == TipoBloque::Aire) {
        slot = itemCursor;
        itemCursor = {};
        manteniendoItem = false;
    } else if (slot.tipo == itemCursor.tipo) {
        int espacio = maxStack(slot.tipo) - slot.cantidad;
        int mover = std::min(espacio, itemCursor.cantidad);
        slot.cantidad += mover;
        itemCursor.cantidad -= mover;
        limpiarSlotSiVacio(itemCursor);
        manteniendoItem = itemCursor.tipo != TipoBloque::Aire;
    } else {
        std::swap(slot, itemCursor);
    }

    actualizarResultadoCrafteo();
}

void InventarioGrid::manejarClickDerecho(int indice) {
    if (indice < 0 || indice >= TOTAL_SLOTS || esSlotResultado(indice)) return;

    SlotInventario& slot = slots[indice];

    if (!manteniendoItem) {
        if (slot.tipo != TipoBloque::Aire) {
            int levantar = (slot.cantidad + 1) / 2;
            itemCursor = {slot.tipo, levantar};
            slot.cantidad -= levantar;
            limpiarSlotSiVacio(slot);
            manteniendoItem = true;
        }
        actualizarResultadoCrafteo();
        return;
    }

    if (!puedeColocarEnSlot(indice, itemCursor.tipo)) return;

    if (slot.tipo == TipoBloque::Aire) {
        slot = {itemCursor.tipo, 1};
        itemCursor.cantidad -= 1;
    } else if (slot.tipo == itemCursor.tipo && slot.cantidad < maxStack(slot.tipo)) {
        slot.cantidad += 1;
        itemCursor.cantidad -= 1;
    }

    limpiarSlotSiVacio(itemCursor);
    manteniendoItem = itemCursor.tipo != TipoBloque::Aire;
    actualizarResultadoCrafteo();
}

void InventarioGrid::manejarClicks(sf::Vector2i posicionMouse, bool clicIzquierdo, bool clicDerecho) {
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

void InventarioGrid::dibujar(sf::RenderWindow& ventana, sf::Font& fuente) {
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

        if (slots[indice].tipo != TipoBloque::Aire) {
            sf::RectangleShape icono({26.0f, 26.0f});
            icono.setPosition({pos.x + 7.0f, pos.y + 6.0f});
            icono.setFillColor(colorDeItem(slots[indice].tipo));
            ventana.draw(icono);

            sf::Text inicial(fuente, inicialItem(slots[indice].tipo), 10);
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

    if (manteniendoItem && itemCursor.tipo != TipoBloque::Aire) {
        sf::RectangleShape icono({28.0f, 28.0f});
        icono.setPosition({static_cast<float>(mouse.x - 14), static_cast<float>(mouse.y - 14)});
        icono.setFillColor(colorDeItem(itemCursor.tipo));
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
