#include <algorithm>
#include <cmath>
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

inline sf::Vector2f posicionSlotMesa(int indice) {
    const float paso = 46.0f;

    if (indice >= 0 && indice < 27) {
        int fila = indice / 9;
        int col = indice % 9;
        return {184.0f + col * paso, 286.0f + fila * paso};
    }

    if (indice >= 27 && indice < 36) {
        int col = indice - 27;
        return {184.0f + col * paso, 430.0f};
    }

    if (indice >= 46 && indice < 55) {
        int local = indice - 46;
        int fila = local / 3;
        int col = local % 3;
        return {286.0f + col * paso, 86.0f + fila * paso};
    }

    return {500.0f, 132.0f};
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
        case ItemId::SemillaArbol: return sf::Color(58, 150, 55);
        case ItemId::MapaInicial: return sf::Color(216, 198, 134);
        case ItemId::Zanahoria: return sf::Color(230, 110, 35);
        case ItemId::Patata: return sf::Color(176, 132, 70);
        case ItemId::Remolacha: return sf::Color(150, 38, 62);
        case ItemId::ChuletaCerdoCruda: return sf::Color(214, 103, 111);
        case ItemId::ChuletaCerdoCocinada: return sf::Color(134, 75, 42);
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
        case ItemId::SemillaArbol: return "Se";
        case ItemId::MapaInicial: return "Mp";
        case ItemId::Zanahoria: return "Za";
        case ItemId::Patata: return "Pt";
        case ItemId::Remolacha: return "Re";
        case ItemId::ChuletaCerdoCruda: return "Ch";
        case ItemId::ChuletaCerdoCocinada: return "Cc";
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

inline void pixel(sf::RenderWindow& ventana, sf::Vector2f origen, int x, int y, sf::Color color, float escala) {
    if (color.a == 0) return;
    sf::RectangleShape punto({escala, escala});
    punto.setPosition({origen.x + static_cast<float>(x) * escala, origen.y + static_cast<float>(y) * escala});
    punto.setFillColor(color);
    ventana.draw(punto);
}

inline void lineaPixel(sf::RenderWindow& ventana, sf::Vector2f origen, int x1, int y1, int x2, int y2, sf::Color color, float escala) {
    int dx = std::abs(x2 - x1);
    int dy = -std::abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx + dy;
    int x = x1;
    int y = y1;

    while (true) {
        pixel(ventana, origen, x, y, color, escala);
        if (x == x2 && y == y2) break;
        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y += sy;
        }
    }
}

inline sf::Color materialHerramienta(ItemId item) {
    switch (item) {
        case ItemId::PicoPiedra:
        case ItemId::PalaPiedra:
        case ItemId::HachaPiedra:
        case ItemId::EspadaPiedra:
            return sf::Color(158, 158, 158);
        case ItemId::PicoDiamante:
            return sf::Color(75, 225, 235);
        default:
            return sf::Color(184, 124, 60);
    }
}

inline void dibujarBloqueSprite(sf::RenderWindow& ventana, sf::Vector2f origen, ItemId item, float escala) {
    sf::Color base = colorDeItem(item);
    sf::Color luz(std::min(255, base.r + 35), std::min(255, base.g + 35), std::min(255, base.b + 35), base.a);
    sf::Color sombra(base.r / 2, base.g / 2, base.b / 2, base.a);

    for (int y = 2; y < 14; ++y) {
        for (int x = 2; x < 14; ++x) {
            sf::Color c = ((x * 5 + y * 3) % 7 == 0) ? luz : base;
            if ((x * 11 + y * 13) % 17 == 0) c = sombra;
            pixel(ventana, origen, x, y, c, escala);
        }
    }

    for (int i = 2; i < 14; ++i) {
        pixel(ventana, origen, i, 2, luz, escala);
        pixel(ventana, origen, 2, i, luz, escala);
        pixel(ventana, origen, i, 13, sombra, escala);
        pixel(ventana, origen, 13, i, sombra, escala);
    }
}

inline void dibujarHerramientaSprite(sf::RenderWindow& ventana, sf::Vector2f origen, ItemId item, float escala) {
    sf::Color mango(120, 72, 32);
    sf::Color material = materialHerramienta(item);
    sf::Color borde(material.r / 2, material.g / 2, material.b / 2);

    if (item == ItemId::EspadaMadera || item == ItemId::EspadaPiedra) {
        for (int y = 2; y < 11; ++y) {
            pixel(ventana, origen, 7, y, material, escala);
            pixel(ventana, origen, 8, y, material, escala);
            if (y > 3) pixel(ventana, origen, 6, y, borde, escala);
        }
        pixel(ventana, origen, 7, 1, material, escala);
        pixel(ventana, origen, 8, 1, material, escala);
        for (int x = 5; x <= 10; ++x) pixel(ventana, origen, x, 11, mango, escala);
        for (int y = 12; y <= 14; ++y) {
            pixel(ventana, origen, 7, y, mango, escala);
            pixel(ventana, origen, 8, y, mango, escala);
        }
        return;
    }

    if (item == ItemId::PalaMadera || item == ItemId::PalaPiedra) {
        for (int y = 6; y <= 14; ++y) {
            pixel(ventana, origen, 7, y, mango, escala);
            pixel(ventana, origen, 8, y, mango, escala);
        }
        for (int y = 1; y <= 5; ++y) {
            for (int x = 6 - (y > 2 ? 1 : 0); x <= 9 + (y > 2 ? 1 : 0); ++x) {
                pixel(ventana, origen, x, y, material, escala);
            }
        }
        pixel(ventana, origen, 6, 5, borde, escala);
        pixel(ventana, origen, 10, 5, borde, escala);
        return;
    }

    if (item == ItemId::HachaMadera || item == ItemId::HachaPiedra) {
        for (int y = 5; y <= 14; ++y) {
            pixel(ventana, origen, 8, y, mango, escala);
            if (y > 8) pixel(ventana, origen, 7, y, mango, escala);
        }
        for (int y = 2; y <= 7; ++y) {
            for (int x = 4; x <= 9; ++x) {
                if (!(x == 4 && y == 7)) pixel(ventana, origen, x, y, material, escala);
            }
        }
        pixel(ventana, origen, 3, 4, borde, escala);
        pixel(ventana, origen, 3, 5, borde, escala);
        return;
    }

    if (item == ItemId::PicoMadera || item == ItemId::PicoPiedra || item == ItemId::PicoDiamante) {
        lineaPixel(ventana, origen, 5, 14, 9, 7, mango, escala);
        lineaPixel(ventana, origen, 6, 14, 10, 7, mango, escala);
        for (int x = 3; x <= 12; ++x) {
            pixel(ventana, origen, x, 3, material, escala);
            if (x >= 4 && x <= 11) pixel(ventana, origen, x, 4, material, escala);
        }
        pixel(ventana, origen, 2, 4, borde, escala);
        pixel(ventana, origen, 13, 4, borde, escala);
        return;
    }
}

inline void dibujarItemSprite(sf::RenderWindow& ventana, ItemId item, sf::Vector2f origen, float escala) {
    switch (item) {
        case ItemId::PicoMadera:
        case ItemId::PicoPiedra:
        case ItemId::PicoDiamante:
        case ItemId::PalaMadera:
        case ItemId::PalaPiedra:
        case ItemId::HachaMadera:
        case ItemId::HachaPiedra:
        case ItemId::EspadaMadera:
        case ItemId::EspadaPiedra:
            dibujarHerramientaSprite(ventana, origen, item, escala);
            return;
        case ItemId::PaloMadera:
            lineaPixel(ventana, origen, 5, 13, 11, 3, sf::Color(154, 93, 39), escala);
            lineaPixel(ventana, origen, 6, 13, 12, 3, sf::Color(100, 62, 28), escala);
            return;
        case ItemId::MapaInicial:
            for (int y = 2; y < 14; ++y) {
                for (int x = 2; x < 14; ++x) {
                    pixel(ventana, origen, x, y, sf::Color(219, 198, 132), escala);
                }
            }
            for (int i = 2; i < 14; ++i) {
                pixel(ventana, origen, i, 2, sf::Color(96, 73, 44), escala);
                pixel(ventana, origen, i, 13, sf::Color(96, 73, 44), escala);
                pixel(ventana, origen, 2, i, sf::Color(96, 73, 44), escala);
                pixel(ventana, origen, 13, i, sf::Color(96, 73, 44), escala);
            }
            pixel(ventana, origen, 5, 5, sf::Color(61, 151, 74), escala);
            pixel(ventana, origen, 6, 5, sf::Color(61, 151, 74), escala);
            pixel(ventana, origen, 5, 6, sf::Color(61, 151, 74), escala);
            pixel(ventana, origen, 9, 8, sf::Color(45, 116, 205), escala);
            pixel(ventana, origen, 10, 8, sf::Color(45, 116, 205), escala);
            pixel(ventana, origen, 9, 9, sf::Color(45, 116, 205), escala);
            pixel(ventana, origen, 7, 10, sf::Color(180, 60, 52), escala);
            return;
        case ItemId::Zanahoria:
            for (int y = 5; y < 14; ++y) {
                int ancho = (y < 8) ? 5 : (y < 11 ? 4 : 2);
                for (int x = 7; x < 7 + ancho; ++x) pixel(ventana, origen, x, y, sf::Color(232, 112, 35), escala);
            }
            pixel(ventana, origen, 7, 4, sf::Color(57, 155, 64), escala);
            pixel(ventana, origen, 8, 3, sf::Color(57, 155, 64), escala);
            pixel(ventana, origen, 10, 4, sf::Color(57, 155, 64), escala);
            return;
        case ItemId::Patata:
            for (int y = 4; y < 13; ++y) {
                for (int x = 5; x < 12; ++x) {
                    if (!((x == 5 || x == 11) && (y < 6 || y > 11))) {
                        pixel(ventana, origen, x, y, sf::Color(178, 132, 71), escala);
                    }
                }
            }
            pixel(ventana, origen, 7, 7, sf::Color(103, 78, 45), escala);
            pixel(ventana, origen, 10, 10, sf::Color(103, 78, 45), escala);
            return;
        case ItemId::Remolacha:
            for (int y = 6; y < 13; ++y) {
                for (int x = 6; x < 12; ++x) {
                    pixel(ventana, origen, x, y, sf::Color(151, 38, 63), escala);
                }
            }
            pixel(ventana, origen, 7, 4, sf::Color(54, 146, 63), escala);
            pixel(ventana, origen, 9, 3, sf::Color(54, 146, 63), escala);
            pixel(ventana, origen, 10, 4, sf::Color(54, 146, 63), escala);
            return;
        case ItemId::ChuletaCerdoCruda:
        case ItemId::ChuletaCerdoCocinada: {
            sf::Color carne = item == ItemId::ChuletaCerdoCruda ? sf::Color(217, 103, 111) : sf::Color(139, 79, 45);
            sf::Color borde = item == ItemId::ChuletaCerdoCruda ? sf::Color(156, 58, 72) : sf::Color(89, 48, 28);
            for (int y = 5; y < 13; ++y) {
                for (int x = 4; x < 13; ++x) {
                    if (!((x == 4 || x == 12) && (y == 5 || y == 12))) {
                        pixel(ventana, origen, x, y, carne, escala);
                    }
                }
            }
            pixel(ventana, origen, 4, 8, borde, escala);
            pixel(ventana, origen, 5, 12, borde, escala);
            pixel(ventana, origen, 11, 6, sf::Color(240, 178, 172), escala);
            return;
        }
        default:
            dibujarBloqueSprite(ventana, origen, item, escala);
            return;
    }
}

inline bool contiene(sf::Vector2f pos, float tam, sf::Vector2i mouse) {
    return mouse.x >= pos.x && mouse.x <= pos.x + tam &&
           mouse.y >= pos.y && mouse.y <= pos.y + tam;
}
}

inline InventarioGrid::InventarioGrid()
    : menuAbierto(false),
      mesaCrafteoAbierta(false),
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
    if (mesaCrafteoAbierta) {
        cerrarMesaCrafteo();
    }
    menuAbierto = !menuAbierto;
    clicIzquierdoAnterior = false;
    clicDerechoAnterior = false;
}

inline bool InventarioGrid::esMenuAbierto() const {
    return menuAbierto;
}

inline void InventarioGrid::abrirMesaCrafteo() {
    if (menuAbierto) {
        devolverCrafteoAlInventario();
    }
    menuAbierto = false;
    mesaCrafteoAbierta = true;
    clicIzquierdoAnterior = false;
    clicDerechoAnterior = false;
    actualizarResultadoMesa();
}

inline void InventarioGrid::cerrarMesaCrafteo() {
    if (mesaCrafteoAbierta) {
        devolverMesaAlInventario();
    }
    mesaCrafteoAbierta = false;
    clicIzquierdoAnterior = false;
    clicDerechoAnterior = false;
}

inline bool InventarioGrid::esMesaCrafteoAbierta() const {
    return mesaCrafteoAbierta;
}

inline void InventarioGrid::seleccionarSlotHotbar(int slot) {
    if (slot >= 0 && slot < SLOTS_HOTBAR) {
        slotSeleccionadoHotbar = slot;
    }
}

inline ItemId InventarioGrid::getItemEnHotbar() const {
    return slots[INDICE_HOTBAR + slotSeleccionadoHotbar].item;
}

inline ItemId InventarioGrid::getItemSegundaMano() const {
    return slots[INDICE_SEGUNDA_MANO].item;
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
    return indice == INDICE_RESULTADO || indice == INDICE_RESULTADO_MESA;
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
    if (!menuAbierto && !mesaCrafteoAbierta) {
        return -1;
    }

    if (mesaCrafteoAbierta) {
        for (int i = 0; i < 36; ++i) {
            if (contiene(posicionSlotMesa(i), TAMANIO_CUADRO, posicionMouse)) return i;
        }
        for (int i = INDICE_MESA_CRAFTEO; i <= INDICE_RESULTADO_MESA; ++i) {
            if (contiene(posicionSlotMesa(i), TAMANIO_CUADRO, posicionMouse)) return i;
        }
        return -1;
    }

    for (int i = 0; i <= INDICE_RESULTADO; ++i) {
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

inline void InventarioGrid::actualizarResultadoMesa() {
    slots[INDICE_RESULTADO_MESA] = {};

    auto itemEnMesa = [&](int fila, int col) {
        return slots[INDICE_MESA_CRAFTEO + fila * 3 + col].item;
    };

    auto estaVacioExcepto = [&](std::vector<int> usados) {
        for (int i = INDICE_MESA_CRAFTEO; i < INDICE_RESULTADO_MESA; ++i) {
            bool usado = std::find(usados.begin(), usados.end(), i) != usados.end();
            if (!usado && !esItemVacio(slots[i].item)) {
                return false;
            }
        }
        return true;
    };

    for (int col = 0; col < 3; ++col) {
        for (int fila = 0; fila < 2; ++fila) {
            int a = INDICE_MESA_CRAFTEO + fila * 3 + col;
            int b = INDICE_MESA_CRAFTEO + (fila + 1) * 3 + col;
            if (slots[a].item == ItemId::TablonMadera &&
                slots[b].item == ItemId::TablonMadera &&
                estaVacioExcepto({a, b})) {
                slots[INDICE_RESULTADO_MESA] = {ItemId::PaloMadera, 4};
                return;
            }
        }
    }

    ItemId arriba = itemEnMesa(0, 1);
    ItemId centro = itemEnMesa(1, 1);
    ItemId abajo = itemEnMesa(2, 1);
    if (centro == ItemId::PaloMadera && abajo == ItemId::PaloMadera) {
        if (arriba == ItemId::TablonMadera && estaVacioExcepto({47, 50, 53})) {
            slots[INDICE_RESULTADO_MESA] = {ItemId::PalaMadera, 1};
            return;
        }
        if (arriba == ItemId::BloquePiedra && estaVacioExcepto({47, 50, 53})) {
            slots[INDICE_RESULTADO_MESA] = {ItemId::PalaPiedra, 1};
            return;
        }
    }

    if (arriba == ItemId::TablonMadera && centro == ItemId::TablonMadera &&
        abajo == ItemId::PaloMadera && estaVacioExcepto({47, 50, 53})) {
        slots[INDICE_RESULTADO_MESA] = {ItemId::EspadaMadera, 1};
        return;
    }
    if (arriba == ItemId::BloquePiedra && centro == ItemId::BloquePiedra &&
        abajo == ItemId::PaloMadera && estaVacioExcepto({47, 50, 53})) {
        slots[INDICE_RESULTADO_MESA] = {ItemId::EspadaPiedra, 1};
        return;
    }

    auto recetaPico = [&](ItemId material, ItemId resultado) {
        bool coincide = itemEnMesa(0, 0) == material &&
                        itemEnMesa(0, 1) == material &&
                        itemEnMesa(0, 2) == material &&
                        itemEnMesa(1, 1) == ItemId::PaloMadera &&
                        itemEnMesa(2, 1) == ItemId::PaloMadera &&
                        estaVacioExcepto({46, 47, 48, 50, 53});
        if (coincide) {
            slots[INDICE_RESULTADO_MESA] = {resultado, 1};
            return true;
        }
        return false;
    };
    if (recetaPico(ItemId::TablonMadera, ItemId::PicoMadera)) return;
    if (recetaPico(ItemId::BloquePiedra, ItemId::PicoPiedra)) return;

    auto recetaHacha = [&](ItemId material, ItemId resultado) {
        bool coincide = itemEnMesa(0, 0) == material &&
                        itemEnMesa(0, 1) == material &&
                        itemEnMesa(1, 0) == material &&
                        itemEnMesa(1, 1) == ItemId::PaloMadera &&
                        itemEnMesa(2, 1) == ItemId::PaloMadera &&
                        estaVacioExcepto({46, 47, 49, 50, 53});
        if (coincide) {
            slots[INDICE_RESULTADO_MESA] = {resultado, 1};
            return true;
        }
        return false;
    };
    if (recetaHacha(ItemId::TablonMadera, ItemId::HachaMadera)) return;
    if (recetaHacha(ItemId::BloquePiedra, ItemId::HachaPiedra)) return;

    bool horno = true;
    for (int fila = 0; fila < 3; ++fila) {
        for (int col = 0; col < 3; ++col) {
            ItemId esperado = (fila == 1 && col == 1) ? ItemId::Ninguno : ItemId::BloquePiedra;
            if (itemEnMesa(fila, col) != esperado) {
                horno = false;
            }
        }
    }
    if (horno) {
        slots[INDICE_RESULTADO_MESA] = {ItemId::Horno, 1};
        return;
    }

    bool cama = itemEnMesa(0, 0) == ItemId::Lana &&
                itemEnMesa(0, 1) == ItemId::Lana &&
                itemEnMesa(0, 2) == ItemId::Lana &&
                itemEnMesa(1, 0) == ItemId::TablonMadera &&
                itemEnMesa(1, 1) == ItemId::TablonMadera &&
                itemEnMesa(1, 2) == ItemId::TablonMadera &&
                estaVacioExcepto({46, 47, 48, 49, 50, 51});
    if (cama) {
        slots[INDICE_RESULTADO_MESA] = {ItemId::Cama, 1};
    }
}

inline void InventarioGrid::consumirIngredientesMesa() {
    for (int i = INDICE_MESA_CRAFTEO; i < INDICE_RESULTADO_MESA; ++i) {
        if (!esItemVacio(slots[i].item)) {
            slots[i].cantidad -= 1;
            limpiarSlotSiVacio(slots[i]);
        }
    }
    actualizarResultadoMesa();
}

inline void InventarioGrid::devolverMesaAlInventario() {
    for (int i = INDICE_MESA_CRAFTEO; i < INDICE_RESULTADO_MESA; ++i) {
        if (!esItemVacio(slots[i].item)) {
            agregarItem(slots[i].item, slots[i].cantidad);
            slots[i] = {};
        }
    }
    slots[INDICE_RESULTADO_MESA] = {};
}

inline void InventarioGrid::manejarClickIzquierdo(int indice) {
    if (indice < 0 || indice >= TOTAL_SLOTS) return;

    if (esSlotResultado(indice)) {
        if (indice == INDICE_RESULTADO_MESA) {
            actualizarResultadoMesa();
        } else {
            actualizarResultadoCrafteo();
        }
        if (esItemVacio(slots[indice].item)) return;

        if (!manteniendoItem) {
            itemCursor = slots[indice];
            manteniendoItem = true;
            if (indice == INDICE_RESULTADO_MESA) {
                consumirIngredientesMesa();
            } else {
                consumirIngredientesCrafteo();
            }
        } else if (itemCursor.item == slots[indice].item && itemCursor.cantidad < maxStack(itemCursor.item)) {
            int espacio = maxStack(itemCursor.item) - itemCursor.cantidad;
            int mover = std::min(espacio, slots[indice].cantidad);
            itemCursor.cantidad += mover;
            if (mover > 0) {
                if (indice == INDICE_RESULTADO_MESA) {
                    consumirIngredientesMesa();
                } else {
                    consumirIngredientesCrafteo();
                }
            }
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
        actualizarResultadoMesa();
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
    actualizarResultadoMesa();
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
        actualizarResultadoMesa();
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
    actualizarResultadoMesa();
}

inline void InventarioGrid::manejarClicks(sf::Vector2i posicionMouse, bool clicIzquierdo, bool clicDerecho) {
    if (!menuAbierto && !mesaCrafteoAbierta) {
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

    if (mesaCrafteoAbierta) {
        sf::RectangleShape fondo({500.0f, 430.0f});
        fondo.setPosition({150.0f, 46.0f});
        fondo.setFillColor(sf::Color(198, 198, 198, 248));
        fondo.setOutlineColor(sf::Color(45, 45, 45));
        fondo.setOutlineThickness(4.0f);
        ventana.draw(fondo);

        sf::Text titulo(fuente, "Crafting", 18);
        titulo.setPosition({184.0f, 56.0f});
        titulo.setFillColor(sf::Color(70, 70, 70));
        ventana.draw(titulo);

        sf::Text inventarioTxt(fuente, "Inventory", 16);
        inventarioTxt.setPosition({184.0f, 258.0f});
        inventarioTxt.setFillColor(sf::Color(70, 70, 70));
        ventana.draw(inventarioTxt);

        sf::Text libro(fuente, "?", 22);
        libro.setPosition({228.0f, 138.0f});
        libro.setFillColor(sf::Color(45, 120, 55));
        ventana.draw(libro);

        sf::Text flecha(fuente, "->", 34);
        flecha.setPosition({438.0f, 136.0f});
        flecha.setFillColor(sf::Color(110, 110, 110));
        ventana.draw(flecha);
    } else if (menuAbierto) {
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
        sf::Vector2f pos = mesaCrafteoAbierta ? posicionSlotMesa(indice) : posicionSlot(indice);
        sf::RectangleShape marco({TAMANIO_CUADRO, TAMANIO_CUADRO});
        marco.setPosition(pos);
        marco.setFillColor(sf::Color(88, 88, 88));
        ventana.draw(marco);

        sf::RectangleShape bordeSuperior({TAMANIO_CUADRO, 2.0f});
        bordeSuperior.setPosition(pos);
        bordeSuperior.setFillColor(sf::Color(220, 220, 220));
        ventana.draw(bordeSuperior);

        sf::RectangleShape bordeIzquierdo({2.0f, TAMANIO_CUADRO});
        bordeIzquierdo.setPosition(pos);
        bordeIzquierdo.setFillColor(sf::Color(220, 220, 220));
        ventana.draw(bordeIzquierdo);

        sf::RectangleShape bordeInferior({TAMANIO_CUADRO, 2.0f});
        bordeInferior.setPosition({pos.x, pos.y + TAMANIO_CUADRO - 2.0f});
        bordeInferior.setFillColor(sf::Color(18, 18, 18));
        ventana.draw(bordeInferior);

        sf::RectangleShape bordeDerecho({2.0f, TAMANIO_CUADRO});
        bordeDerecho.setPosition({pos.x + TAMANIO_CUADRO - 2.0f, pos.y});
        bordeDerecho.setFillColor(sf::Color(18, 18, 18));
        ventana.draw(bordeDerecho);

        if (indice == INDICE_HOTBAR + slotSeleccionadoHotbar) {
            sf::RectangleShape seleccion({TAMANIO_CUADRO, TAMANIO_CUADRO});
            seleccion.setPosition(pos);
            seleccion.setFillColor(sf::Color::Transparent);
            seleccion.setOutlineThickness(2.0f);
            seleccion.setOutlineColor(sf::Color::Yellow);
            ventana.draw(seleccion);
        }

        if (indice == hover) {
            sf::RectangleShape luz({TAMANIO_CUADRO, TAMANIO_CUADRO});
            luz.setPosition(pos);
            luz.setFillColor(sf::Color(255, 255, 255, 70));
            ventana.draw(luz);
        }

        if (!esItemVacio(slots[indice].item)) {
            dibujarItemSprite(ventana, slots[indice].item, {pos.x + 6.0f, pos.y + 5.0f}, 1.8f);

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

    if (mesaCrafteoAbierta) {
        for (int i = 0; i < 36; ++i) {
            dibujarSlot(i);
        }
        for (int i = INDICE_MESA_CRAFTEO; i <= INDICE_RESULTADO_MESA; ++i) {
            dibujarSlot(i);
        }
    } else if (menuAbierto) {
        for (int i = 0; i < TOTAL_SLOTS; ++i) {
            if (i >= INDICE_MESA_CRAFTEO) continue;
            dibujarSlot(i);
        }
    } else {
        for (int i = INDICE_HOTBAR; i < INDICE_HOTBAR + SLOTS_HOTBAR; ++i) {
            dibujarSlot(i);
        }
    }

    if (manteniendoItem && !esItemVacio(itemCursor.item)) {
        dibujarItemSprite(
            ventana,
            itemCursor.item,
            {static_cast<float>(mouse.x - 14), static_cast<float>(mouse.y - 14)},
            1.8f
        );

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


