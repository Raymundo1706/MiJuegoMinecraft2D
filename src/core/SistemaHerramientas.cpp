#include "SistemaHerramientas.hpp"

SistemaHerramientas::SistemaHerramientas() : herramientaActiva(0) {
    // Inicializamos todos los materiales recolectables en 0
    inventario[TipoBloque::Pasto] = 0;
    inventario[TipoBloque::Tierra] = 0;
    inventario[TipoBloque::Madera] = 0;
    inventario[TipoBloque::Piedra] = 0;
    inventario[TipoBloque::MineralHierro] = 0;
    inventario[TipoBloque::MineralDiamante] = 0;
}

SistemaHerramientas::~SistemaHerramientas() {}

void SistemaHerramientas::cambiarHerramienta(int slot) {
    if (slot >= 0 && slot <= 3) {
        herramientaActiva = slot;
    }
}

int SistemaHerramientas::getHerramientaActiva() const {
    return herramientaActiva;
}

float SistemaHerramientas::calcularDanio(TipoBloque tipo) const {
    // DAÑO POR DEFECTO: Mano desnuda u otra herramienta no apta
    float danio = 0.5f;

    if (tipo == TipoBloque::Pasto || tipo == TipoBloque::Tierra) {
        danio = 0.16f; // Tarda ~3 segundos con la mano
    }
    else if (tipo == TipoBloque::Madera) {
        danio = 0.5f;  // Tarda ~3 segundos con la mano
    }
    else if (tipo == TipoBloque::Piedra || 
             tipo == TipoBloque::MineralHierro || 
             tipo == TipoBloque::MineralDiamante) {
        danio = 0.33f; // Tarda ~15 segundos con la mano o herramienta incorrecta
    }

    // MEJORAS SI USA LA HERRAMIENTA CORRECTA
    if (herramientaActiva == 1) { // === PICO ===
        if (tipo == TipoBloque::Piedra || tipo == TipoBloque::MineralHierro || tipo == TipoBloque::MineralDiamante) {
            danio = 2.5f; 
        } else if (tipo == TipoBloque::Madera) {
            danio = 0.38f; // Tarda 4 segundos con pico
        }
    } 
    else if (herramientaActiva == 2) { // === PALA ===
        if (tipo == TipoBloque::Pasto || tipo == TipoBloque::Tierra) {
            danio = 0.25f; // 2 segundos
        }
    }
    else if (herramientaActiva == 3) { // === HACHA ===
        if (tipo == TipoBloque::Madera) {
            danio = 0.75f; // 2 segundos
        }
    }

    return danio;
}

void SistemaHerramientas::agregarAlInventario(TipoBloque tipo, int herramientaUsada) {
    // REGLA: Piedra y minerales requieren obligatoriamente el Pico (1)
    bool esMineral = (tipo == TipoBloque::Piedra || 
                      tipo == TipoBloque::MineralHierro || 
                      tipo == TipoBloque::MineralDiamante);

    if (!esMineral || (esMineral && herramientaUsada == 1)) {
        // Corrección menor: Si es pasto o tierra, ambos acumulan en "Pasto" (o el que uses en tu UI)
        if (tipo == TipoBloque::Tierra) {
            inventario[TipoBloque::Pasto]++;
        } else {
            inventario[tipo]++;
        }
    }
}

int SistemaHerramientas::getCantidad(TipoBloque tipo) const {
    auto it = inventario.find(tipo);
    if (it != inventario.end()) {
        return it->second;
    }
    return 0;
}