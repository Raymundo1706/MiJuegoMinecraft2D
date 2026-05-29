#include "SistemaHerramientas.hpp"

SistemaHerramientas::SistemaHerramientas() : herramientaActiva(0) {
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
    ItemId itemLegacy = ItemId::Ninguno;
    if (herramientaActiva == 1) itemLegacy = ItemId::PicoMadera;
    if (herramientaActiva == 2) itemLegacy = ItemId::PalaMadera;
    if (herramientaActiva == 3) itemLegacy = ItemId::HachaMadera;
    return calcularDanio(tipo, itemLegacy);
}

float SistemaHerramientas::calcularDanio(TipoBloque tipo, ItemId itemEnMano) const {
    float danio = 0.5f;

    if (tipo == TipoBloque::Pasto || tipo == TipoBloque::Tierra) {
        danio = 0.16f;
    } else if (tipo == TipoBloque::Madera) {
        danio = 0.5f;
    } else if (tipo == TipoBloque::Piedra ||
               tipo == TipoBloque::MineralHierro ||
               tipo == TipoBloque::MineralOro ||
               tipo == TipoBloque::MineralDiamante ||
               tipo == TipoBloque::Redstone) {
        danio = 0.33f;
    }

    TipoHerramienta herramienta = tipoHerramienta(itemEnMano);

    if (herramienta == TipoHerramienta::Pico) {
        if (tipo == TipoBloque::Piedra ||
            tipo == TipoBloque::MineralHierro ||
            tipo == TipoBloque::MineralOro ||
            tipo == TipoBloque::MineralDiamante ||
            tipo == TipoBloque::Redstone) {
            danio = 2.5f;
            if (itemEnMano == ItemId::PicoPiedra) danio = 3.25f;
            if (itemEnMano == ItemId::PicoDiamante) danio = 5.0f;
        } else if (tipo == TipoBloque::Madera) {
            danio = 0.38f;
        }
    } else if (herramienta == TipoHerramienta::Pala) {
        if (tipo == TipoBloque::Pasto || tipo == TipoBloque::Tierra) {
            danio = 0.25f;
        }
    } else if (herramienta == TipoHerramienta::Hacha) {
        if (tipo == TipoBloque::Madera) {
            danio = 0.75f;
        }
    }

    return danio;
}

bool SistemaHerramientas::puedeRecolectar(TipoBloque tipo, ItemId itemEnMano) const {
    bool requierePico = (tipo == TipoBloque::Piedra ||
                         tipo == TipoBloque::MineralHierro ||
                         tipo == TipoBloque::MineralOro ||
                         tipo == TipoBloque::MineralDiamante ||
                         tipo == TipoBloque::Redstone);

    if (!requierePico) {
        return itemDesdeBloque(tipo) != ItemId::Ninguno;
    }

    return tipoHerramienta(itemEnMano) == TipoHerramienta::Pico;
}

void SistemaHerramientas::agregarAlInventario(TipoBloque tipo, int herramientaUsada) {
    bool esMineral = (tipo == TipoBloque::Piedra ||
                      tipo == TipoBloque::MineralHierro ||
                      tipo == TipoBloque::MineralDiamante);

    if (!esMineral || (esMineral && herramientaUsada == 1)) {
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
