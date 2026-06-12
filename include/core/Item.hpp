#pragma once

#include <string>
#include "Mundo.hpp"

enum class ItemId {
    Ninguno,
    BloquePasto,
    BloqueTierra,
    BloquePiedra,
    BloqueTronco,
    TablonMadera,
    MineralHierro,
    MineralPlata,
    MineralOro,
    MineralDiamante,
    Redstone,
    Cristal,
    Lana,
    PaloMadera,
    MesaCrafteo,
    Horno,
    Cama,
    BloqueTecho,
    SemillaArbol,
    MapaInicial,
    Zanahoria,
    Patata,
    Remolacha,
    ChuletaCerdoCruda,
    ChuletaCerdoCocinada,
    CarneResCruda,
    LanaCruda,
    PolloCrudo,
    Pluma,
    CarnePodrida,
    Barreta,
    PicoMadera,
    PicoPiedra,
    PicoDiamante,
    PalaMadera,
    PalaPiedra,
    HachaMadera,
    HachaPiedra,
    EspadaMadera,
    EspadaPiedra,
    Carbon,
    Antorcha,
    PuertaMadera,
    CaminoAldea,
    CultivoTrigo,
    CultivoZanahoria,
    CultivoPatata,
    Lava,
    Cofre,
    Yunque,
    Esmeralda,
    Trigo,
    Pan,
    LingoteHierro
};

enum class TipoHerramienta {
    Ninguna,
    Pico,
    Pala,
    Hacha
};

bool esItemVacio(ItemId item);
bool esHerramienta(ItemId item);
TipoHerramienta tipoHerramienta(ItemId item);
int maxStackItem(ItemId item);
ItemId itemDesdeBloque(TipoBloque bloque);
TipoBloque bloqueDesdeItem(ItemId item);
bool esItemColocable(ItemId item);
std::string nombreItem(ItemId item);

#include "../inline/core/Item.cpp"
