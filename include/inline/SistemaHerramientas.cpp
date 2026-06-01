
inline SistemaHerramientas::SistemaHerramientas() : herramientaActiva(0) {
    inventario[TipoBloque::Pasto] = 0;
    inventario[TipoBloque::Tierra] = 0;
    inventario[TipoBloque::Madera] = 0;
    inventario[TipoBloque::Piedra] = 0;
    inventario[TipoBloque::MineralHierro] = 0;
    inventario[TipoBloque::MineralDiamante] = 0;
}

inline SistemaHerramientas::~SistemaHerramientas() {}

inline void SistemaHerramientas::cambiarHerramienta(int slot) {
    if (slot >= 0 && slot <= 3) {
        herramientaActiva = slot;
    }
}

inline int SistemaHerramientas::getHerramientaActiva() const {
    return herramientaActiva;
}

inline float SistemaHerramientas::calcularDanio(TipoBloque tipo) const {
    ItemId itemLegacy = ItemId::Ninguno;
    if (herramientaActiva == 1) itemLegacy = ItemId::PicoMadera;
    if (herramientaActiva == 2) itemLegacy = ItemId::PalaMadera;
    if (herramientaActiva == 3) itemLegacy = ItemId::HachaMadera;
    return calcularDanio(tipo, itemLegacy);
}

inline float SistemaHerramientas::calcularDanio(TipoBloque tipo, ItemId itemEnMano) const {
    float vidaReferencia = 50.0f;
    switch (tipo) {
        case TipoBloque::Pasto:
        case TipoBloque::Tierra:
        case TipoBloque::TierraArada:
            vidaReferencia = 30.0f;
            break;
        case TipoBloque::Madera:
        case TipoBloque::MesaCrafteo:
            vidaReferencia = 90.0f;
            break;
        case TipoBloque::Piedra:
            vidaReferencia = 300.0f;
            break;
        case TipoBloque::MineralHierro:
        case TipoBloque::MineralOro:
            vidaReferencia = 450.0f;
            break;
        case TipoBloque::MineralDiamante:
        case TipoBloque::Redstone:
            vidaReferencia = 600.0f;
            break;
        default:
            break;
    }

    return vidaReferencia / calcularTiempoMinado(tipo, itemEnMano);
}

inline float SistemaHerramientas::calcularTiempoMinado(TipoBloque tipo, ItemId itemEnMano) const {
    TipoHerramienta herramienta = tipoHerramienta(itemEnMano);

    if (tipo == TipoBloque::Pasto || tipo == TipoBloque::Tierra || tipo == TipoBloque::TierraArada) {
        if (itemEnMano == ItemId::PalaMadera) return 0.4f;
        if (itemEnMano == ItemId::PalaPiedra) return 0.2f;
        return 0.75f;
    }

    if (tipo == TipoBloque::Madera || tipo == TipoBloque::MesaCrafteo) {
        if (itemEnMano == ItemId::HachaMadera) return 13.0f;
        if (itemEnMano == ItemId::HachaPiedra) return 10.0f;
        return 20.0f;
    }

    if (tipo == TipoBloque::Piedra) {
        if (itemEnMano == ItemId::PicoMadera) return 1.15f;
        if (itemEnMano == ItemId::PicoPiedra) return 0.6f;
        if (itemEnMano == ItemId::PicoDiamante) return 0.3f;
        return 7.5f;
    }

    if (tipo == TipoBloque::MineralHierro || tipo == TipoBloque::MineralOro || tipo == TipoBloque::Redstone) {
        if (itemEnMano == ItemId::PicoMadera) return 2.25f;
        if (itemEnMano == ItemId::PicoPiedra) return 1.15f;
        if (itemEnMano == ItemId::PicoDiamante) return 0.6f;
        return 15.0f;
    }

    if (tipo == TipoBloque::MineralDiamante) {
        if (itemEnMano == ItemId::PicoMadera || itemEnMano == ItemId::PicoPiedra) return 2.25f;
        if (itemEnMano == ItemId::PicoDiamante) return 0.6f;
        return 15.0f;
    }

    if (herramienta == TipoHerramienta::Pico ||
        herramienta == TipoHerramienta::Pala ||
        herramienta == TipoHerramienta::Hacha) {
        return 2.0f;
    }

    return 1.0f;
}

inline bool SistemaHerramientas::puedeRecolectar(TipoBloque tipo, ItemId itemEnMano) const {
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

inline void SistemaHerramientas::agregarAlInventario(TipoBloque tipo, int herramientaUsada) {
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

inline int SistemaHerramientas::getCantidad(TipoBloque tipo) const {
    auto it = inventario.find(tipo);
    if (it != inventario.end()) {
        return it->second;
    }
    return 0;
}


