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
    PicoMadera,
    PicoPiedra,
    PicoDiamante,
    PalaMadera,
    PalaPiedra,
    HachaMadera,
    HachaPiedra,
    EspadaMadera,
    EspadaPiedra
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

#include "inline/Item.cpp"
