#pragma once

#include <string>
#include "Mundo.hpp"

enum class ItemId {
    Ninguno,
    BloquePasto,
    BloqueTierra,
    BloquePiedra,
    BloqueMadera,
    MineralHierro,
    MineralOro,
    MineralDiamante,
    Redstone,
    Cristal,
    BloqueTecho,
    PicoMadera,
    PicoPiedra,
    PicoDiamante,
    PalaMadera,
    HachaMadera
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
