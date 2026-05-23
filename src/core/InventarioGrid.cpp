#include "InventarioGrid.hpp"
#include <sstream>

InventarioGrid::InventarioGrid() 
    : menuAbierto(false), slotSeleccionadoHotbar(0), slotArrastrando(-1), manteniendoItem(false) 
{
    // Creamos los 38 slots vacíos (30 inventario general + 8 de la Hotbar)
    slots.resize(38);
}

InventarioGrid::~InventarioGrid() {}

void InventarioGrid::alternarMenu() {
    menuAbierto = !menuAbierto;
}

bool InventarioGrid::esMenuAbierto() const {
    return menuAbierto;
}

void InventarioGrid::seleccionarSlotHotbar(int slot) {
    if (slot >= 0 && slot < 8) {
        slotSeleccionadoHotbar = slot;
    }
}

TipoBloque InventarioGrid::getTipoEnHotbar() const {
    // Los últimos 8 slots (30 al 37) representan la Hotbar interactiva de abajo
    return slots[30 + slotSeleccionadoHotbar].tipo;
}

void InventarioGrid::agregarItem(TipoBloque tipo, int cantidad) {
    if (tipo == TipoBloque::Aire || tipo == TipoBloque::Agua) return;

    // 1. Intentar buscar un stack existente del mismo tipo que no esté lleno (< 64)
    for (int i = 0; i < 38; ++i) {
        if (slots[i].tipo == tipo && slots[i].cantidad < 64) {
            slots[i].cantidad += cantidad;
            return;
        }
    }

    // 2. Si no hay stack existente, buscar el primer cuadrito vacío disponible
    for (int i = 0; i < 38; ++i) {
        if (slots[i].tipo == TipoBloque::Aire) {
            slots[i].tipo = tipo;
            slots[i].cantidad = cantidad;
            return;
        }
    }
}

void InventarioGrid::manejarClicks(sf::Vector2i posicionMouse, bool clicPresionado) {
    if (!menuAbierto) return;

    // Si el jugador suelta el clic y llevaba un ítem flotando, lo soltamos en el nuevo destino
    if (!clicPresionado) {
        if (manteniendoItem && slotArrastrando != -1) {
            // Calcular en qué casilla cayó el mouse al soltarlo
            // (Esta es una simplificación matemática del Grid visual)
            int fila = (posicionMouse.y - 150) / 48;
            int col = (posicionMouse.x - 200) / 48;

            if (col >= 0 && col < 10 && fila >= 0 && fila < 4) {
                int destino = (fila * 10) + col;
                if (destino >= 0 && destino < 38) {
                    // Intercambio clásico estilo Minecraft (Swap)
                    SlotInventario temp = slots[slotArrastrando];
                    slots[slotArrastrando] = slots[destino];
                    slots[destino] = temp;
                }
            }
        }
        manteniendoItem = false;
        slotArrastrando = -1;
        return;
    }

    // Si hace clic primario, detectamos qué casilla está presionando para levantar el ítem
    if (clicPresionado && !manteniendoItem) {
        int fila = (posicionMouse.y - 150) / 48;
        int col = (posicionMouse.x - 200) / 48;

        if (col >= 0 && col < 10 && fila >= 0 && fila < 4) {
            int seleccionado = (fila * 10) + col;
            if (seleccionado >= 0 && seleccionado < 38 && slots[seleccionado].tipo != TipoBloque::Aire) {
                slotArrastrando = seleccionado;
                manteniendoItem = true;
            }
        }
    }
}

void InventarioGrid::dibujar(sf::RenderWindow& ventana, sf::Font& fuente) {
    // --- DIBUJAR SIEMPRE LA HOTBAR (Abajo en la pantalla) ---
    for (int i = 0; i < 8; ++i) {
        sf::RectangleShape cuadro({TAMANIO_CUADRO, TAMANIO_CUADRO});
        cuadro.setPosition({200.0f + (i * 50.0f), 530.0f});
        cuadro.setFillColor(sf::Color(80, 80, 80, 230));
        cuadro.setOutlineThickness(2.0f);
        
        if (i == slotSeleccionadoHotbar) {
            cuadro.setOutlineColor(sf::Color::Yellow); // Resaltado del slot en mano
        } else {
            cuadro.setOutlineColor(sf::Color::White);
        }
        ventana.draw(cuadro);

        // Dibujar el ítem dentro de la Hotbar (Slots del 30 al 37)
        int idx = 30 + i;
        if (slots[idx].tipo != TipoBloque::Aire) {
            sf::Text txt(fuente, std::to_string(slots[idx].cantidad), 12);
            txt.setPosition({202.0f + (i * 50.0f), 550.0f});
            txt.setFillColor(sf::Color::Cyan);
            ventana.draw(txt);
        }
    }

    // --- DIBUJAR EL MENÚ COMPLETO (Solo si presionó la Q) ---
    if (!menuAbierto) return;

    // Fondo oscuro translúcido del menú de fondo
    sf::RectangleShape fondoMenu({500.0f, 250.0f});
    fondoMenu.setPosition({180.0f, 130.0f});
    fondoMenu.setFillColor(sf::Color(30, 30, 30, 240));
    fondoMenu.setOutlineColor(sf::Color::White);
    fondoMenu.setOutlineThickness(3.0f);
    ventana.draw(fondoMenu);

    // Dibujar la cuadrícula de la mochila (3 filas de 10 columnas = 30 slots)
    for (int fila = 0; fila < 3; ++fila) {
        for (int col = 0; col < 10; ++col) {
            int idx = (fila * 10) + col;
            
            sf::RectangleShape celda({TAMANIO_CUADRO, TAMANIO_CUADRO});
            celda.setPosition({200.0f + (col * 48.0f), 150.0f + (fila * 48.0f)});
            
            if (idx == slotArrastrando && manteniendoItem) {
                celda.setFillColor(sf::Color(150, 50, 50, 150)); // Efecto de celda vaciada temporalmente
            } else {
                celda.setFillColor(sf::Color(60, 60, 60, 255));
            }
            celda.setOutlineColor(sf::Color(120, 120, 120));
            celda.setOutlineThickness(1.0f);
            ventana.draw(celda);

            // Pintar texto o cantidad del bloque si hay algo guardado
            if (slots[idx].tipo != TipoBloque::Aire && !(idx == slotArrastrando && manteniendoItem)) {
                std::string inicial = "B";
                if (slots[idx].tipo == TipoBloque::Madera) inicial = "M";
                if (slots[idx].tipo == TipoBloque::Piedra) inicial = "P";
                if (slots[idx].tipo == TipoBloque::MineralHierro) inicial = "H";
                if (slots[idx].tipo == TipoBloque::MineralDiamante) inicial = "D";

                sf::Text txt(fuente, inicial + ":" + std::to_string(slots[idx].cantidad), 11);
                txt.setPosition({202.0f + (col * 48.0f), 155.0f + (fila * 48.0f)});
                txt.setFillColor(sf::Color::White);
                ventana.draw(txt);
            }
        }
    }

    // Si arrastramos un ítem con el mouse, dibujar un cuadrito fantasma que sigue al cursor
    if (manteniendoItem && slotArrastrando != -1) {
        sf::RectangleShape itemFlotante({25.0f, 25.0f});
        itemFlotante.setFillColor(sf::Color::Green);
        // Conversión rápida de la posición del mouse
        sf::Vector2i mousePos = sf::Mouse::getPosition(ventana);
        itemFlotante.setPosition({static_cast<float>(mousePos.x - 12), static_cast<float>(mousePos.y - 12)});
        ventana.draw(itemFlotante);
    }
}