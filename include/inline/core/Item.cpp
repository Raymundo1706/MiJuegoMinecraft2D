inline bool esItemVacio(ItemId item) {
    return item == ItemId::Ninguno;
}

inline bool esHerramienta(ItemId item) {
    return item == ItemId::PicoMadera ||
           item == ItemId::PicoPiedra ||
           item == ItemId::PicoDiamante ||
           item == ItemId::PalaMadera ||
           item == ItemId::PalaPiedra ||
           item == ItemId::HachaMadera ||
           item == ItemId::HachaPiedra ||
           item == ItemId::Barreta ||
           item == ItemId::EspadaMadera ||
           item == ItemId::EspadaPiedra;
}

inline TipoHerramienta tipoHerramienta(ItemId item) {
    switch (item) {
        case ItemId::PicoMadera:
        case ItemId::PicoPiedra:
        case ItemId::PicoDiamante:
            return TipoHerramienta::Pico;
        case ItemId::PalaMadera:
        case ItemId::PalaPiedra:
        case ItemId::Barreta:
            return TipoHerramienta::Pala;
        case ItemId::HachaMadera:
        case ItemId::HachaPiedra:
            return TipoHerramienta::Hacha;
        default:
            return TipoHerramienta::Ninguna;
    }
}

inline int maxStackItem(ItemId item) {
    if (esItemVacio(item)) return 0;
    if (item == ItemId::MapaInicial) return 1;
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
        case TipoBloque::Madera: return ItemId::BloqueTronco;
        case TipoBloque::MesaCrafteo: return ItemId::MesaCrafteo;
        case TipoBloque::Horno: return ItemId::Horno;
        case TipoBloque::Cristal: return ItemId::Cristal;
        case TipoBloque::MineralHierro: return ItemId::MineralHierro;
        case TipoBloque::MineralPlata: return ItemId::MineralPlata;
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
        case ItemId::BloqueTronco: return TipoBloque::Madera;
        case ItemId::TablonMadera: return TipoBloque::Madera;
        case ItemId::MesaCrafteo: return TipoBloque::MesaCrafteo;
        case ItemId::Horno: return TipoBloque::Horno;
        case ItemId::Cristal: return TipoBloque::Cristal;
        case ItemId::MineralHierro: return TipoBloque::MineralHierro;
        case ItemId::MineralPlata: return TipoBloque::MineralPlata;
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
        case ItemId::BloqueTronco: return "Tronco";
        case ItemId::TablonMadera: return "Tablon";
        case ItemId::MineralHierro: return "Hierro";
        case ItemId::MineralPlata: return "Plata";
        case ItemId::MineralOro: return "Oro";
        case ItemId::MineralDiamante: return "Diamante";
        case ItemId::Redstone: return "Redstone";
        case ItemId::Cristal: return "Cristal";
        case ItemId::Lana: return "Lana";
        case ItemId::PaloMadera: return "Palo";
        case ItemId::MesaCrafteo: return "Mesa de Crafteo";
        case ItemId::Horno: return "Horno";
        case ItemId::Cama: return "Cama";
        case ItemId::BloqueTecho: return "Techo";
        case ItemId::SemillaArbol: return "Semilla de arbol";
        case ItemId::MapaInicial: return "Mapa Inicial";
        case ItemId::Zanahoria: return "Zanahoria";
        case ItemId::Patata: return "Patata";
        case ItemId::Remolacha: return "Remolacha";
        case ItemId::ChuletaCerdoCruda: return "Chuleta cruda";
        case ItemId::ChuletaCerdoCocinada: return "Chuleta cocinada";
        case ItemId::CarneResCruda: return "Carne cruda";
        case ItemId::LanaCruda: return "Lana";
        case ItemId::PolloCrudo: return "Pollo crudo";
        case ItemId::Pluma: return "Pluma";
        case ItemId::Barreta: return "Barreta";
        case ItemId::PicoMadera: return "Pico madera";
        case ItemId::PicoPiedra: return "Pico piedra";
        case ItemId::PicoDiamante: return "Pico diamante";
        case ItemId::PalaMadera: return "Pala madera";
        case ItemId::PalaPiedra: return "Pala piedra";
        case ItemId::HachaMadera: return "Hacha madera";
        case ItemId::HachaPiedra: return "Hacha piedra";
        case ItemId::EspadaMadera: return "Espada madera";
        case ItemId::EspadaPiedra: return "Espada piedra";
        default: return "Mano";
    }
}

