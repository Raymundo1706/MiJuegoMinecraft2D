inline bool esItemVacio(ItemId item) {
    return item == ItemId::Ninguno;
}

inline bool esHerramienta(ItemId item) {
    return item == ItemId::PicoMadera ||
           item == ItemId::PicoPiedra ||
           item == ItemId::PicoDiamante ||
           item == ItemId::PalaMadera ||
           item == ItemId::HachaMadera;
}

inline TipoHerramienta tipoHerramienta(ItemId item) {
    switch (item) {
        case ItemId::PicoMadera:
        case ItemId::PicoPiedra:
        case ItemId::PicoDiamante:
            return TipoHerramienta::Pico;
        case ItemId::PalaMadera:
            return TipoHerramienta::Pala;
        case ItemId::HachaMadera:
            return TipoHerramienta::Hacha;
        default:
            return TipoHerramienta::Ninguna;
    }
}

inline int maxStackItem(ItemId item) {
    if (esItemVacio(item)) return 0;
    if (esHerramienta(item)) return 1;
    return 64;
}

inline ItemId itemDesdeBloque(TipoBloque bloque) {
    switch (bloque) {
        case TipoBloque::Pasto: return ItemId::BloquePasto;
        case TipoBloque::Tierra:
        case TipoBloque::TierraArada:
            return ItemId::BloqueTierra;
        case TipoBloque::Piedra: return ItemId::BloquePiedra;
        case TipoBloque::Madera: return ItemId::BloqueMadera;
        case TipoBloque::MineralHierro: return ItemId::MineralHierro;
        case TipoBloque::MineralOro: return ItemId::MineralOro;
        case TipoBloque::MineralDiamante: return ItemId::MineralDiamante;
        case TipoBloque::Redstone: return ItemId::Redstone;
        default: return ItemId::Ninguno;
    }
}

inline TipoBloque bloqueDesdeItem(ItemId item) {
    switch (item) {
        case ItemId::BloquePasto: return TipoBloque::Pasto;
        case ItemId::BloqueTierra: return TipoBloque::Tierra;
        case ItemId::BloquePiedra: return TipoBloque::Piedra;
        case ItemId::BloqueMadera: return TipoBloque::Madera;
        case ItemId::MineralHierro: return TipoBloque::MineralHierro;
        case ItemId::MineralOro: return TipoBloque::MineralOro;
        case ItemId::MineralDiamante: return TipoBloque::MineralDiamante;
        case ItemId::Redstone: return TipoBloque::Redstone;
        default: return TipoBloque::Aire;
    }
}

inline bool esItemColocable(ItemId item) {
    return bloqueDesdeItem(item) != TipoBloque::Aire ||
           item == ItemId::Cristal ||
           item == ItemId::BloqueTecho;
}

inline std::string nombreItem(ItemId item) {
    switch (item) {
        case ItemId::BloquePasto: return "Pasto";
        case ItemId::BloqueTierra: return "Tierra";
        case ItemId::BloquePiedra: return "Piedra";
        case ItemId::BloqueMadera: return "Madera";
        case ItemId::MineralHierro: return "Hierro";
        case ItemId::MineralOro: return "Oro";
        case ItemId::MineralDiamante: return "Diamante";
        case ItemId::Redstone: return "Redstone";
        case ItemId::Cristal: return "Cristal";
        case ItemId::BloqueTecho: return "Techo";
        case ItemId::PicoMadera: return "Pico madera";
        case ItemId::PicoPiedra: return "Pico piedra";
        case ItemId::PicoDiamante: return "Pico diamante";
        case ItemId::PalaMadera: return "Pala madera";
        case ItemId::HachaMadera: return "Hacha madera";
        default: return "Mano";
    }
}

