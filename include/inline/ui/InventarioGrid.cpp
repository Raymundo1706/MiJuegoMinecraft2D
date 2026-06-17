#include <algorithm>
#include <cmath>
#include <string>

namespace {
const sf::Vector2f PANEL_POS(32.0f, 24.0f);
const sf::Vector2f PANEL_SIZE(736.0f, 552.0f);
const sf::Vector2f PLAYER_POS(122.0f, 64.0f);
const sf::Vector2f PLAYER_SIZE(170.0f, 206.0f);

struct RecetaCatalogoMesa {
    const char* nombre;
    const char* descripcion;
    int categoria;
    ItemId resultado;
    int cantidadResultado;
    std::vector<SlotInventario> ingredientes;
    ItemId matriz[9];
};

inline SlotInventario ingrediente(ItemId item, int cantidad) {
    return {item, cantidad};
}

inline RecetaCatalogoMesa recetaCatalogo(
    const char* nombre,
    const char* descripcion,
    int categoria,
    ItemId resultado,
    int cantidadResultado,
    std::vector<SlotInventario> ingredientes,
    std::vector<ItemId> matriz
) {
    RecetaCatalogoMesa receta{nombre, descripcion, categoria, resultado, cantidadResultado, ingredientes, {
        ItemId::Ninguno, ItemId::Ninguno, ItemId::Ninguno,
        ItemId::Ninguno, ItemId::Ninguno, ItemId::Ninguno,
        ItemId::Ninguno, ItemId::Ninguno, ItemId::Ninguno
    }};
    for (std::size_t i = 0; i < matriz.size() && i < 9; ++i) {
        receta.matriz[i] = matriz[i];
    }
    return receta;
}

inline const std::vector<RecetaCatalogoMesa>& recetasCatalogoMesa() {
    static const std::vector<RecetaCatalogoMesa> recetas = {
        recetaCatalogo("Tablones de madera", "Refina un tronco en madera lista para construir.", 0,
            ItemId::TablonMadera, 4, {ingrediente(ItemId::BloqueTronco, 1)},
            {ItemId::BloqueTronco, ItemId::Ninguno, ItemId::Ninguno,
             ItemId::Ninguno, ItemId::Ninguno, ItemId::Ninguno,
             ItemId::Ninguno, ItemId::Ninguno, ItemId::Ninguno}),
        recetaCatalogo("Palos", "Componente basico para herramientas.", 0,
            ItemId::PaloMadera, 4, {ingrediente(ItemId::TablonMadera, 2)},
            {ItemId::Ninguno, ItemId::TablonMadera, ItemId::Ninguno,
             ItemId::Ninguno, ItemId::TablonMadera, ItemId::Ninguno,
             ItemId::Ninguno, ItemId::Ninguno, ItemId::Ninguno}),
        recetaCatalogo("Mesa de crafteo", "Permite fabricar objetos avanzados.", 0,
            ItemId::MesaCrafteo, 1, {ingrediente(ItemId::TablonMadera, 4)},
            {ItemId::TablonMadera, ItemId::TablonMadera, ItemId::Ninguno,
             ItemId::TablonMadera, ItemId::TablonMadera, ItemId::Ninguno,
             ItemId::Ninguno, ItemId::Ninguno, ItemId::Ninguno}),
        recetaCatalogo("Horno", "Bloque utilitario para cocinar y fundir.", 0,
            ItemId::Horno, 1, {ingrediente(ItemId::BloquePiedra, 8)},
            {ItemId::BloquePiedra, ItemId::BloquePiedra, ItemId::BloquePiedra,
             ItemId::BloquePiedra, ItemId::Ninguno, ItemId::BloquePiedra,
             ItemId::BloquePiedra, ItemId::BloquePiedra, ItemId::BloquePiedra}),
        recetaCatalogo("Antorchas", "Iluminan cuevas y zonas oscuras.", 0,
            ItemId::Antorcha, 4, {ingrediente(ItemId::Carbon, 1), ingrediente(ItemId::PaloMadera, 1)},
            {ItemId::Ninguno, ItemId::Carbon, ItemId::Ninguno,
             ItemId::Ninguno, ItemId::PaloMadera, ItemId::Ninguno,
             ItemId::Ninguno, ItemId::Ninguno, ItemId::Ninguno}),
        recetaCatalogo("Bloque de techo", "Pieza superior para cubrir habitaciones.", 0,
            ItemId::BloqueTecho, 2, {ingrediente(ItemId::TablonMadera, 4), ingrediente(ItemId::BloquePiedra, 2)},
            {ItemId::TablonMadera, ItemId::TablonMadera, ItemId::TablonMadera,
             ItemId::BloquePiedra, ItemId::Ninguno, ItemId::BloquePiedra,
             ItemId::Ninguno, ItemId::TablonMadera, ItemId::Ninguno}),
        recetaCatalogo("Puerta de madera", "Entrada atravesable para casas cerradas.", 0,
            ItemId::PuertaMadera, 3, {ingrediente(ItemId::TablonMadera, 6)},
            {ItemId::TablonMadera, ItemId::TablonMadera, ItemId::Ninguno,
             ItemId::TablonMadera, ItemId::TablonMadera, ItemId::Ninguno,
             ItemId::TablonMadera, ItemId::TablonMadera, ItemId::Ninguno}),
        recetaCatalogo("Cristal", "Bloque claro para ventanas y vision exterior.", 0,
            ItemId::Cristal, 4, {ingrediente(ItemId::MineralPlata, 1), ingrediente(ItemId::BloquePiedra, 2)},
            {ItemId::Ninguno, ItemId::MineralPlata, ItemId::Ninguno,
             ItemId::BloquePiedra, ItemId::Ninguno, ItemId::BloquePiedra,
             ItemId::Ninguno, ItemId::Ninguno, ItemId::Ninguno}),
        recetaCatalogo("Cama", "Guarda tu punto de reaparicion al dormir.", 0,
            ItemId::Cama, 1, {ingrediente(ItemId::Lana, 3), ingrediente(ItemId::TablonMadera, 3)},
            {ItemId::Lana, ItemId::Lana, ItemId::Lana,
             ItemId::TablonMadera, ItemId::TablonMadera, ItemId::TablonMadera,
             ItemId::Ninguno, ItemId::Ninguno, ItemId::Ninguno}),
        recetaCatalogo("Pico de madera", "Herramienta inicial para minar piedra.", 1,
            ItemId::PicoMadera, 1, {ingrediente(ItemId::TablonMadera, 3), ingrediente(ItemId::PaloMadera, 2)},
            {ItemId::TablonMadera, ItemId::TablonMadera, ItemId::TablonMadera,
             ItemId::Ninguno, ItemId::PaloMadera, ItemId::Ninguno,
             ItemId::Ninguno, ItemId::PaloMadera, ItemId::Ninguno}),
        recetaCatalogo("Pico de piedra", "Mina piedra y minerales basicos mas rapido.", 1,
            ItemId::PicoPiedra, 1, {ingrediente(ItemId::BloquePiedra, 3), ingrediente(ItemId::PaloMadera, 2)},
            {ItemId::BloquePiedra, ItemId::BloquePiedra, ItemId::BloquePiedra,
             ItemId::Ninguno, ItemId::PaloMadera, ItemId::Ninguno,
             ItemId::Ninguno, ItemId::PaloMadera, ItemId::Ninguno}),
        recetaCatalogo("Barreta", "Herramienta pesada para abrir entradas al subsuelo.", 1,
            ItemId::Barreta, 1, {ingrediente(ItemId::BloquePiedra, 9), ingrediente(ItemId::PaloMadera, 6)},
            {ItemId::BloquePiedra, ItemId::BloquePiedra, ItemId::BloquePiedra,
             ItemId::Ninguno, ItemId::PaloMadera, ItemId::Ninguno,
             ItemId::Ninguno, ItemId::PaloMadera, ItemId::Ninguno}),
        recetaCatalogo("Hacha de madera", "Corta madera con mas eficiencia.", 1,
            ItemId::HachaMadera, 1, {ingrediente(ItemId::TablonMadera, 3), ingrediente(ItemId::PaloMadera, 2)},
            {ItemId::TablonMadera, ItemId::TablonMadera, ItemId::Ninguno,
             ItemId::TablonMadera, ItemId::PaloMadera, ItemId::Ninguno,
             ItemId::Ninguno, ItemId::PaloMadera, ItemId::Ninguno}),
        recetaCatalogo("Hacha de piedra", "Corta madera mejor que el hacha de madera.", 1,
            ItemId::HachaPiedra, 1, {ingrediente(ItemId::BloquePiedra, 3), ingrediente(ItemId::PaloMadera, 2)},
            {ItemId::BloquePiedra, ItemId::BloquePiedra, ItemId::Ninguno,
             ItemId::BloquePiedra, ItemId::PaloMadera, ItemId::Ninguno,
             ItemId::Ninguno, ItemId::PaloMadera, ItemId::Ninguno}),
        recetaCatalogo("Pala de madera", "Remueve tierra y arena.", 1,
            ItemId::PalaMadera, 1, {ingrediente(ItemId::TablonMadera, 1), ingrediente(ItemId::PaloMadera, 2)},
            {ItemId::Ninguno, ItemId::TablonMadera, ItemId::Ninguno,
             ItemId::Ninguno, ItemId::PaloMadera, ItemId::Ninguno,
             ItemId::Ninguno, ItemId::PaloMadera, ItemId::Ninguno}),
        recetaCatalogo("Pala de piedra", "Remueve tierra con mayor velocidad.", 1,
            ItemId::PalaPiedra, 1, {ingrediente(ItemId::BloquePiedra, 1), ingrediente(ItemId::PaloMadera, 2)},
            {ItemId::Ninguno, ItemId::BloquePiedra, ItemId::Ninguno,
             ItemId::Ninguno, ItemId::PaloMadera, ItemId::Ninguno,
             ItemId::Ninguno, ItemId::PaloMadera, ItemId::Ninguno}),
        recetaCatalogo("Espada de madera", "Arma basica para defenderte.", 1,
            ItemId::EspadaMadera, 1, {ingrediente(ItemId::TablonMadera, 2), ingrediente(ItemId::PaloMadera, 1)},
            {ItemId::Ninguno, ItemId::TablonMadera, ItemId::Ninguno,
             ItemId::Ninguno, ItemId::TablonMadera, ItemId::Ninguno,
             ItemId::Ninguno, ItemId::PaloMadera, ItemId::Ninguno}),
        recetaCatalogo("Espada de piedra", "Arma de piedra para mayor dano.", 1,
            ItemId::EspadaPiedra, 1, {ingrediente(ItemId::BloquePiedra, 2), ingrediente(ItemId::PaloMadera, 1)},
            {ItemId::Ninguno, ItemId::BloquePiedra, ItemId::Ninguno,
             ItemId::Ninguno, ItemId::BloquePiedra, ItemId::Ninguno,
             ItemId::Ninguno, ItemId::PaloMadera, ItemId::Ninguno}),
        recetaCatalogo("Lana compacta", "Reune lana suelta para usarla en cama.", 2,
            ItemId::Lana, 1, {ingrediente(ItemId::LanaCruda, 4)},
            {ItemId::LanaCruda, ItemId::LanaCruda, ItemId::Ninguno,
             ItemId::LanaCruda, ItemId::LanaCruda, ItemId::Ninguno,
             ItemId::Ninguno, ItemId::Ninguno, ItemId::Ninguno}),
        recetaCatalogo("Chuleta cocinada", "Comida preparada para recuperar mas hambre.", 3,
            ItemId::ChuletaCerdoCocinada, 1, {ingrediente(ItemId::ChuletaCerdoCruda, 1), ingrediente(ItemId::Carbon, 1)},
            {ItemId::Ninguno, ItemId::ChuletaCerdoCruda, ItemId::Ninguno,
             ItemId::Ninguno, ItemId::Carbon, ItemId::Ninguno,
             ItemId::Ninguno, ItemId::Ninguno, ItemId::Ninguno}),
        recetaCatalogo("Racion vegetal", "Combina cultivos en una racion simple.", 3,
            ItemId::Zanahoria, 1, {ingrediente(ItemId::Zanahoria, 1), ingrediente(ItemId::Patata, 1), ingrediente(ItemId::Remolacha, 1)},
            {ItemId::Zanahoria, ItemId::Patata, ItemId::Remolacha,
             ItemId::Ninguno, ItemId::Ninguno, ItemId::Ninguno,
             ItemId::Ninguno, ItemId::Ninguno, ItemId::Ninguno}),
    };
    return recetas;
}

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
        case ItemId::MineralPlata: return sf::Color(205, 210, 215);
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
        case ItemId::CarneResCruda: return sf::Color(178, 63, 70);
        case ItemId::LanaCruda: return sf::Color(220, 220, 220);
        case ItemId::PolloCrudo: return sf::Color(232, 180, 158);
        case ItemId::Pluma: return sf::Color(240, 240, 230);
        case ItemId::CarnePodrida: return sf::Color(92, 108, 42);
        case ItemId::Barreta: return sf::Color(105, 108, 112);
        case ItemId::Carbon: return sf::Color(36, 35, 34);
        case ItemId::Antorcha: return sf::Color(235, 168, 48);
        case ItemId::PuertaMadera: return sf::Color(142, 86, 38);
        case ItemId::CaminoAldea: return sf::Color(142, 105, 66);
        case ItemId::CultivoTrigo: return sf::Color(204, 178, 64);
        case ItemId::CultivoZanahoria: return sf::Color(229, 112, 35);
        case ItemId::CultivoPatata: return sf::Color(178, 136, 73);
        case ItemId::Lava: return sf::Color(255, 88, 24);
        case ItemId::Cofre: return sf::Color(166, 104, 38);
        case ItemId::Yunque: return sf::Color(82, 86, 90);
        case ItemId::Esmeralda: return sf::Color(39, 214, 114);
        case ItemId::Trigo: return sf::Color(219, 190, 83);
        case ItemId::SemillasTrigo: return sf::Color(128, 174, 68);
        case ItemId::Pan: return sf::Color(190, 124, 50);
        case ItemId::LingoteHierro: return sf::Color(206, 198, 184);
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
        case ItemId::MineralPlata: return "Ag";
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
        case ItemId::CarneResCruda: return "Re";
        case ItemId::LanaCruda: return "La";
        case ItemId::PolloCrudo: return "Po";
        case ItemId::Pluma: return "Pl";
        case ItemId::CarnePodrida: return "CP";
        case ItemId::Barreta: return "Br";
        case ItemId::PicoMadera: return "Pk";
        case ItemId::PicoPiedra: return "Pp";
        case ItemId::PicoDiamante: return "Pd";
        case ItemId::PalaMadera: return "Pa";
        case ItemId::PalaPiedra: return "Pp";
        case ItemId::HachaMadera: return "Ha";
        case ItemId::HachaPiedra: return "Hp";
        case ItemId::EspadaMadera: return "Em";
        case ItemId::EspadaPiedra: return "Ep";
        case ItemId::Carbon: return "Co";
        case ItemId::Antorcha: return "An";
        case ItemId::PuertaMadera: return "Pu";
        case ItemId::CaminoAldea: return "Ca";
        case ItemId::CultivoTrigo: return "CT";
        case ItemId::CultivoZanahoria: return "CZ";
        case ItemId::CultivoPatata: return "CP";
        case ItemId::Lava: return "Lv";
        case ItemId::Cofre: return "Cf";
        case ItemId::Yunque: return "Yu";
        case ItemId::Esmeralda: return "Es";
        case ItemId::Trigo: return "Tr";
        case ItemId::SemillasTrigo: return "St";
        case ItemId::Pan: return "Pn";
        case ItemId::LingoteHierro: return "Fe";
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

inline sf::Color colorPixelTierraItem(int x, int y) {
    static const char* patron[16] = {
        "bbcbbdabbcabbdcb",
        "bDaabbccbaadbbab",
        "abbcCbbdabccadbb",
        "cbbadbbcaabbcDab",
        "bbccabbdabbcbaac",
        "adbbcaabDcbbadbb",
        "bbadbbcabbccabba",
        "cabbadbbaacbbdCb",
        "bbccbaadbCbbcaab",
        "adbbcabbccabbdba",
        "bbadCbbcaabbdabb",
        "cabbccadbbaacbba",
        "bbdabbcCbbadbbca",
        "aabbcDabbccbaadb",
        "bbcabbadbbcaabCb",
        "cbaadbbccabbadbb"
    };

    switch (patron[y][x]) {
        case 'a': return sf::Color(104, 67, 38);
        case 'b': return sf::Color(126, 82, 46);
        case 'c': return sf::Color(151, 99, 55);
        case 'd': return sf::Color(82, 52, 31);
        case 'C': return sf::Color(172, 114, 64);
        case 'D': return sf::Color(57, 36, 24);
        default: return sf::Color(126, 82, 46);
    }
}

inline void dibujarBloqueTierraItemSprite(sf::RenderWindow& ventana, sf::Vector2f origen, float escala) {
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            pixel(ventana, origen, x, y, colorPixelTierraItem(x, y), escala);
        }
    }
}

inline bool dibujarTexturaItem(sf::RenderWindow& ventana, const char* ruta, sf::Vector2f centro, float tamanoMaximo) {
    static sf::Texture texturaTronco;
    static bool intentoTronco = false;
    static bool texturaTroncoLista = false;

    if (!intentoTronco) {
        intentoTronco = true;
        texturaTroncoLista = texturaTronco.loadFromFile(ruta);
        if (texturaTroncoLista) {
            texturaTronco.setSmooth(false);
        }
    }

    if (!texturaTroncoLista) {
        return false;
    }

    sf::Vector2u tam = texturaTronco.getSize();
    float escala = tamanoMaximo / static_cast<float>(std::max(tam.x, tam.y));
    sf::Sprite sprite(texturaTronco);
    sprite.setOrigin({static_cast<float>(tam.x) * 0.5f, static_cast<float>(tam.y) * 0.5f});
    sprite.setPosition(centro);
    sprite.setScale({escala, escala});
    ventana.draw(sprite);
    return true;
}

inline bool dibujarRetratoInventario(sf::RenderWindow& ventana) {
    static sf::Texture texturaRetrato;
    static bool intentoCarga = false;
    static bool texturaLista = false;

    if (!intentoCarga) {
        intentoCarga = true;
        texturaLista = texturaRetrato.loadFromFile("assets/textures/inventory_portrait.jpeg");
        if (texturaLista) {
            texturaRetrato.setSmooth(true);
        }
    }

    if (!texturaLista) {
        return false;
    }

    sf::Vector2u tam = texturaRetrato.getSize();
    if (tam.x == 0 || tam.y == 0) {
        return false;
    }

    constexpr float MARGEN_RETRATO = 6.0f;
    float anchoDisponible = PLAYER_SIZE.x - MARGEN_RETRATO * 2.0f;
    float altoDisponible = PLAYER_SIZE.y - MARGEN_RETRATO * 2.0f;
    float escala = std::min(
        anchoDisponible / static_cast<float>(tam.x),
        altoDisponible / static_cast<float>(tam.y)
    );

    sf::Sprite sprite(texturaRetrato);
    sprite.setOrigin({static_cast<float>(tam.x) * 0.5f, static_cast<float>(tam.y) * 0.5f});
    sprite.setPosition({
        PLAYER_POS.x + PLAYER_SIZE.x * 0.5f,
        PLAYER_POS.y + PLAYER_SIZE.y * 0.5f
    });
    sprite.setScale({escala, escala});
    ventana.draw(sprite);
    return true;
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

inline void dibujarTroncoCortadoSprite(sf::RenderWindow& ventana, sf::Vector2f origen, float escala) {
    const sf::Color borde(25, 16, 12);
    const sf::Color corte(181, 126, 72);
    const sf::Color corteLuz(228, 170, 96);
    const sf::Color anillo(111, 73, 45);
    const sf::Color madera(112, 66, 44);
    const sf::Color maderaLuz(158, 96, 62);
    const sf::Color maderaOscura(62, 36, 28);
    const sf::Color cuerda(201, 153, 91);
    const sf::Color cuerdaSombra(116, 79, 45);

    auto pos = [&](float x, float y) {
        return sf::Vector2f(origen.x + x * escala, origen.y + y * escala);
    };

    auto rect = [&](float x, float y, float w, float h, sf::Color fill, sf::Color outline = sf::Color::Transparent, float outlineSize = 0.0f) {
        sf::RectangleShape forma({w * escala, h * escala});
        forma.setPosition(pos(x, y));
        forma.setFillColor(fill);
        forma.setOutlineColor(outline);
        forma.setOutlineThickness(outlineSize * escala);
        ventana.draw(forma);
    };

    auto circulo = [&](float cx, float cy, float radio, sf::Color fill, sf::Color outline = sf::Color::Transparent, float outlineSize = 0.0f) {
        sf::CircleShape forma(radio * escala, 28);
        forma.setOrigin({radio * escala, radio * escala});
        forma.setPosition(pos(cx, cy));
        forma.setFillColor(fill);
        forma.setOutlineColor(outline);
        forma.setOutlineThickness(outlineSize * escala);
        ventana.draw(forma);
    };

    auto veta = [&](float x, float y, float w, sf::Color color) {
        sf::RectangleShape linea({w * escala, 0.55f * escala});
        linea.setPosition(pos(x, y));
        linea.setFillColor(color);
        ventana.draw(linea);
    };

    auto tronco = [&](float x, float y, float largo, float alto) {
        float radio = alto * 0.5f;
        rect(x + radio, y, largo - radio, alto, madera, borde, 0.28f);
        rect(x + largo - 1.3f, y + 0.55f, 1.3f, alto - 1.1f, maderaOscura);
        veta(x + radio + 1.0f, y + 1.05f, largo - radio - 3.0f, maderaLuz);
        veta(x + radio + 2.6f, y + alto * 0.52f, largo - radio - 4.5f, maderaOscura);
        veta(x + radio + 0.7f, y + alto - 1.25f, largo - radio - 3.8f, maderaOscura);

        circulo(x + radio, y + radio, radio, corte, borde, 0.32f);
        circulo(x + radio, y + radio, radio * 0.56f, sf::Color::Transparent, anillo, 0.28f);
        circulo(x + radio, y + radio, radio * 0.22f, sf::Color::Transparent, anillo, 0.22f);
        circulo(x + radio - radio * 0.28f, y + radio - radio * 0.30f, radio * 0.18f, corteLuz);
    };

    sf::CircleShape sombra(7.1f * escala, 30);
    sombra.setScale({1.0f, 0.28f});
    sombra.setOrigin({7.1f * escala, 7.1f * escala});
    sombra.setPosition(pos(8.0f, 13.2f));
    sombra.setFillColor(sf::Color(0, 0, 0, 58));
    ventana.draw(sombra);

    tronco(3.0f, 2.4f, 10.4f, 4.4f);
    tronco(2.0f, 7.7f, 9.2f, 4.7f);
    tronco(6.0f, 8.9f, 7.8f, 4.0f);

    rect(10.4f, 2.6f, 0.85f, 10.6f, cuerdaSombra);
    rect(11.15f, 2.8f, 0.85f, 10.1f, cuerda);
    rect(10.0f, 7.2f, 2.35f, 0.65f, cuerda);
    rect(10.1f, 8.0f, 2.0f, 0.55f, cuerdaSombra);
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
        case ItemId::Carbon:
            for (int y = 5; y < 13; ++y) {
                for (int x = 5; x < 13; ++x) {
                    if (!((x == 5 || x == 12) && (y == 5 || y == 12))) {
                        pixel(ventana, origen, x, y, ((x + y) % 3 == 0) ? sf::Color(62, 61, 58) : sf::Color(28, 27, 26), escala);
                    }
                }
            }
            pixel(ventana, origen, 7, 6, sf::Color(92, 90, 84), escala);
            return;
        case ItemId::Antorcha:
            for (int y = 6; y < 15; ++y) {
                pixel(ventana, origen, 8, y, sf::Color(126, 76, 30), escala);
                pixel(ventana, origen, 9, y, sf::Color(77, 43, 22), escala);
            }
            pixel(ventana, origen, 7, 3, sf::Color(255, 210, 66), escala);
            pixel(ventana, origen, 8, 2, sf::Color(255, 238, 116), escala);
            pixel(ventana, origen, 9, 3, sf::Color(240, 118, 36), escala);
            pixel(ventana, origen, 8, 4, sf::Color(232, 82, 28), escala);
            return;
        case ItemId::BloqueTronco:
            dibujarTroncoCortadoSprite(ventana, origen, escala);
            return;
        case ItemId::BloqueTierra:
            dibujarBloqueTierraItemSprite(ventana, origen, escala);
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
        case ItemId::ChuletaCerdoCocinada:
        case ItemId::CarneResCruda:
        case ItemId::PolloCrudo: {
            sf::Color carne = colorDeItem(item);
            sf::Color borde(carne.r / 2, carne.g / 2, carne.b / 2);
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
        case ItemId::LanaCruda:
            dibujarBloqueSprite(ventana, origen, item, escala);
            return;
        case ItemId::Pluma:
            lineaPixel(ventana, origen, 5, 13, 12, 4, sf::Color(240, 240, 230), escala);
            lineaPixel(ventana, origen, 6, 13, 13, 4, sf::Color(180, 185, 185), escala);
            pixel(ventana, origen, 10, 6, sf::Color::White, escala);
            pixel(ventana, origen, 8, 9, sf::Color::White, escala);
            return;
        default:
            dibujarBloqueSprite(ventana, origen, item, escala);
            return;
    }
}

inline bool contiene(sf::Vector2f pos, float tam, sf::Vector2i mouse) {
    return mouse.x >= pos.x && mouse.x <= pos.x + tam &&
           mouse.y >= pos.y && mouse.y <= pos.y + tam;
}

inline void dibujarTooltipInventario(
    sf::RenderWindow& ventana,
    sf::Font& fuente,
    const std::string& texto,
    sf::Vector2i mouse
) {
    if (texto.empty()) {
        return;
    }

    constexpr float margenX = 8.0f;
    constexpr float margenY = 5.0f;
    sf::Text etiqueta(fuente, texto, 12);
    etiqueta.setFillColor(sf::Color::White);
    etiqueta.setOutlineColor(sf::Color::Black);
    etiqueta.setOutlineThickness(1.0f);

    sf::FloatRect bounds = etiqueta.getLocalBounds();
    sf::Vector2f tamano(bounds.size.x + margenX * 2.0f, bounds.size.y + margenY * 2.0f);
    sf::Vector2f pos(static_cast<float>(mouse.x + 14), static_cast<float>(mouse.y + 18));
    sf::Vector2u ventanaTam = ventana.getSize();

    if (pos.x + tamano.x > static_cast<float>(ventanaTam.x) - 4.0f) {
        pos.x = static_cast<float>(mouse.x) - tamano.x - 12.0f;
    }
    if (pos.y + tamano.y > static_cast<float>(ventanaTam.y) - 4.0f) {
        pos.y = static_cast<float>(mouse.y) - tamano.y - 12.0f;
    }
    pos.x = std::max(4.0f, pos.x);
    pos.y = std::max(4.0f, pos.y);

    sf::RectangleShape fondo(tamano);
    fondo.setPosition(pos);
    fondo.setFillColor(sf::Color(22, 22, 22, 230));
    fondo.setOutlineColor(sf::Color(235, 235, 235, 210));
    fondo.setOutlineThickness(1.0f);
    ventana.draw(fondo);

    etiqueta.setPosition({pos.x + margenX - bounds.position.x, pos.y + margenY - bounds.position.y});
    ventana.draw(etiqueta);
}
}

inline InventarioGrid::InventarioGrid()
    : menuAbierto(false),
      mesaCrafteoAbierta(false),
      categoriaMesa(0),
      recetaMesaSeleccionada(0),
      slotSeleccionadoHotbar(0),
      manteniendoItem(false),
      clicIzquierdoAnterior(false),
      clicDerechoAnterior(false),
      enterCatalogoAnterior(false),
      eventoFabricacionCatalogo(false),
      eventoMovimientoItem(false) {
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
    categoriaMesa = 0;
    recetaMesaSeleccionada = 0;
    clicIzquierdoAnterior = false;
    clicDerechoAnterior = false;
    enterCatalogoAnterior = false;
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
    eventoMovimientoItem = true;
    return true;
}

inline SlotInventario InventarioGrid::extraerItemHotbar(int cantidad) {
    int indice = INDICE_HOTBAR + slotSeleccionadoHotbar;
    return extraerItemEnSlot(indice, cantidad);
}

inline SlotInventario InventarioGrid::extraerItemCursor(int cantidad) {
    if (cantidad <= 0 || !manteniendoItem || esItemVacio(itemCursor.item)) {
        return {};
    }

    int extraer = std::min(cantidad, itemCursor.cantidad);
    SlotInventario resultado{itemCursor.item, extraer};
    itemCursor.cantidad -= extraer;
    limpiarSlotSiVacio(itemCursor);
    manteniendoItem = !esItemVacio(itemCursor.item);
    eventoMovimientoItem = true;
    return resultado;
}

inline SlotInventario InventarioGrid::extraerItemEnSlot(int indice, int cantidad) {
    if (indice < 0 || indice >= TOTAL_SLOTS || esSlotResultado(indice) || cantidad <= 0 || esItemVacio(slots[indice].item)) {
        return {};
    }

    int extraer = std::min(cantidad, slots[indice].cantidad);
    SlotInventario resultado{slots[indice].item, extraer};
    slots[indice].cantidad -= extraer;
    limpiarSlotSiVacio(slots[indice]);
    actualizarResultadoCrafteo();
    actualizarResultadoMesa();
    eventoMovimientoItem = true;
    return resultado;
}

inline int InventarioGrid::getSlotHover(sf::Vector2i posicionMouse) const {
    return obtenerSlotEnPosicion(posicionMouse);
}

inline void InventarioGrid::intercambiarConSegundaMano() {
    int indiceHotbar = INDICE_HOTBAR + slotSeleccionadoHotbar;
    std::swap(slots[indiceHotbar], slots[INDICE_SEGUNDA_MANO]);
    eventoMovimientoItem = true;
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

    auto apilarEnRango = [&](int inicio, int fin) {
        for (int i = inicio; i < fin && restante > 0; ++i) {
            if (slots[i].item == item && slots[i].cantidad < maxStack(item)) {
                int espacio = maxStack(item) - slots[i].cantidad;
                int mover = std::min(espacio, restante);
                slots[i].cantidad += mover;
                restante -= mover;
            }
        }
    };

    auto ocuparVaciosEnRango = [&](int inicio, int fin) {
        for (int i = inicio; i < fin && restante > 0; ++i) {
            if (esItemVacio(slots[i].item)) {
                int mover = std::min(maxStack(item), restante);
                slots[i].item = item;
                slots[i].cantidad = mover;
                restante -= mover;
            }
        }
    };

    apilarEnRango(INDICE_HOTBAR, INDICE_HOTBAR + SLOTS_HOTBAR);
    apilarEnRango(0, SLOTS_INVENTARIO_PRINCIPAL);
    ocuparVaciosEnRango(INDICE_HOTBAR, INDICE_HOTBAR + SLOTS_HOTBAR);
    ocuparVaciosEnRango(0, SLOTS_INVENTARIO_PRINCIPAL);
    if (restante < cantidad) {
        eventoMovimientoItem = true;
    }
}

inline bool InventarioGrid::puedeAgregarItem(ItemId item, int cantidad) const {
    if (esItemVacio(item) || cantidad <= 0) return false;

    int restante = cantidad;
    auto revisarRango = [&](int inicio, int fin, bool soloStacks) {
        for (int i = inicio; i < fin && restante > 0; ++i) {
            if (soloStacks) {
                if (slots[i].item == item && slots[i].cantidad < maxStack(item)) {
                    restante -= std::min(maxStack(item) - slots[i].cantidad, restante);
                }
            } else if (esItemVacio(slots[i].item)) {
                restante -= std::min(maxStack(item), restante);
            }
        }
    };

    revisarRango(INDICE_HOTBAR, INDICE_HOTBAR + SLOTS_HOTBAR, true);
    revisarRango(0, SLOTS_INVENTARIO_PRINCIPAL, true);
    revisarRango(INDICE_HOTBAR, INDICE_HOTBAR + SLOTS_HOTBAR, false);
    revisarRango(0, SLOTS_INVENTARIO_PRINCIPAL, false);
    return restante <= 0;
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

inline int InventarioGrid::contarItemBanco(ItemId item) const {
    if (esItemVacio(item)) return 0;
    int total = 0;
    for (int i = INDICE_HOTBAR; i < INDICE_HOTBAR + SLOTS_HOTBAR; ++i) {
        if (slots[i].item == item) total += slots[i].cantidad;
    }
    for (int i = 0; i < SLOTS_INVENTARIO_PRINCIPAL; ++i) {
        if (slots[i].item == item) total += slots[i].cantidad;
    }
    return total;
}

inline bool InventarioGrid::tieneIngredientesCatalogo(const std::vector<SlotInventario>& ingredientes) const {
    for (const SlotInventario& ingredienteActual : ingredientes) {
        if (contarItemBanco(ingredienteActual.item) < ingredienteActual.cantidad) {
            return false;
        }
    }
    return true;
}

inline bool InventarioGrid::consumirIngredientesCatalogo(const std::vector<SlotInventario>& ingredientes) {
    if (!tieneIngredientesCatalogo(ingredientes)) return false;

    auto consumirEnRango = [&](ItemId item, int& restante, int inicio, int fin) {
        for (int i = inicio; i < fin && restante > 0; ++i) {
            if (slots[i].item != item) continue;
            int quitar = std::min(slots[i].cantidad, restante);
            slots[i].cantidad -= quitar;
            restante -= quitar;
            limpiarSlotSiVacio(slots[i]);
        }
    };

    for (const SlotInventario& ingredienteActual : ingredientes) {
        int restante = ingredienteActual.cantidad;
        consumirEnRango(ingredienteActual.item, restante, INDICE_HOTBAR, INDICE_HOTBAR + SLOTS_HOTBAR);
        consumirEnRango(ingredienteActual.item, restante, 0, SLOTS_INVENTARIO_PRINCIPAL);
    }
    return true;
}

inline bool InventarioGrid::fabricarRecetaCatalogo(int indiceReceta) {
    const auto& recetas = recetasCatalogoMesa();
    if (indiceReceta < 0 || indiceReceta >= static_cast<int>(recetas.size())) return false;

    const RecetaCatalogoMesa& receta = recetas[indiceReceta];
    if (receta.categoria != categoriaMesa) return false;
    if (!tieneIngredientesCatalogo(receta.ingredientes)) return false;
    if (!puedeAgregarItem(receta.resultado, receta.cantidadResultado)) return false;

    if (!consumirIngredientesCatalogo(receta.ingredientes)) return false;
    agregarItem(receta.resultado, receta.cantidadResultado);
    eventoFabricacionCatalogo = true;
    return true;
}

inline int InventarioGrid::fabricarRecetaCatalogoMaximo(int indiceReceta) {
    int fabricados = 0;
    while (fabricados < 64 && fabricarRecetaCatalogo(indiceReceta)) {
        ++fabricados;
    }
    return fabricados;
}

inline bool InventarioGrid::consumirEventoFabricacion() {
    bool evento = eventoFabricacionCatalogo;
    eventoFabricacionCatalogo = false;
    return evento;
}

inline bool InventarioGrid::consumirEventoMovimientoItem() {
    bool evento = eventoMovimientoItem;
    eventoMovimientoItem = false;
    return evento;
}

inline void InventarioGrid::manejarClickCatalogoMesa(sf::Vector2i posicionMouse) {
    sf::Vector2f mouse(static_cast<float>(posicionMouse.x), static_cast<float>(posicionMouse.y));

    for (int i = 0; i < 4; ++i) {
        sf::FloatRect tab({34.0f + static_cast<float>(i) * 86.0f, 34.0f}, {72.0f, 42.0f});
        if (tab.contains(mouse)) {
            categoriaMesa = i;
            recetaMesaSeleccionada = 0;
            return;
        }
    }

    const auto& recetas = recetasCatalogoMesa();
    sf::FloatRect botonFabricar({512.0f, 506.0f}, {108.0f, 38.0f});
    sf::FloatRect botonMaximo({632.0f, 506.0f}, {108.0f, 38.0f});
    if (botonFabricar.contains(mouse) || botonMaximo.contains(mouse)) {
        int visible = 0;
        for (int i = 0; i < static_cast<int>(recetas.size()); ++i) {
            if (recetas[i].categoria != categoriaMesa) continue;
            if (visible == recetaMesaSeleccionada) {
                if (botonMaximo.contains(mouse)) {
                    fabricarRecetaCatalogoMaximo(i);
                } else {
                    fabricarRecetaCatalogo(i);
                }
                return;
            }
            ++visible;
        }
        return;
    }

    int visible = 0;
    for (int i = 0; i < static_cast<int>(recetas.size()); ++i) {
        if (recetas[i].categoria != categoriaMesa) continue;

        int col = visible % 4;
        int fila = visible / 4;
        sf::FloatRect celda({56.0f + static_cast<float>(col) * 74.0f, 128.0f + static_cast<float>(fila) * 72.0f}, {56.0f, 56.0f});
        if (celda.contains(mouse)) {
            recetaMesaSeleccionada = visible;
            return;
        }
        ++visible;
    }
}

inline void InventarioGrid::manejarShiftClick(int indice) {
    if (indice < 0 || indice >= TOTAL_SLOTS || esSlotResultado(indice) || esItemVacio(slots[indice].item)) {
        return;
    }

    SlotInventario& origen = slots[indice];
    ItemId item = origen.item;
    int restante = origen.cantidad;

    auto moverARango = [&](int inicio, int fin) {
        for (int i = inicio; i < fin && restante > 0; ++i) {
            if (i == indice) continue;
            if (slots[i].item == item && slots[i].cantidad < maxStack(item)) {
                int mover = std::min(maxStack(item) - slots[i].cantidad, restante);
                slots[i].cantidad += mover;
                restante -= mover;
            }
        }
        for (int i = inicio; i < fin && restante > 0; ++i) {
            if (i == indice) continue;
            if (esItemVacio(slots[i].item)) {
                int mover = std::min(maxStack(item), restante);
                slots[i] = {item, mover};
                restante -= mover;
            }
        }
    };

    if (indice >= INDICE_HOTBAR && indice < INDICE_HOTBAR + SLOTS_HOTBAR) {
        moverARango(0, SLOTS_INVENTARIO_PRINCIPAL);
    } else if (indice >= 0 && indice < SLOTS_INVENTARIO_PRINCIPAL) {
        moverARango(INDICE_HOTBAR, INDICE_HOTBAR + SLOTS_HOTBAR);
    } else if (indice >= INDICE_CRAFTEO && indice < INDICE_RESULTADO) {
        moverARango(INDICE_HOTBAR, INDICE_HOTBAR + SLOTS_HOTBAR);
        moverARango(0, SLOTS_INVENTARIO_PRINCIPAL);
    } else if (indice == INDICE_SEGUNDA_MANO || (indice >= INDICE_ARMADURA && indice < INDICE_ARMADURA + 4)) {
        moverARango(INDICE_HOTBAR, INDICE_HOTBAR + SLOTS_HOTBAR);
        moverARango(0, SLOTS_INVENTARIO_PRINCIPAL);
    }

    if (restante != origen.cantidad) {
        origen.cantidad = restante;
        limpiarSlotSiVacio(origen);
        actualizarResultadoCrafteo();
        actualizarResultadoMesa();
        eventoMovimientoItem = true;
    }
}

inline void InventarioGrid::manejarClickIzquierdo(int indice) {
    if (indice < 0 || indice >= TOTAL_SLOTS) return;

    bool shiftPresionado = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) ||
                           sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift);
    if (shiftPresionado && !manteniendoItem) {
        manejarShiftClick(indice);
        return;
    }

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
        eventoMovimientoItem = true;
        return;
    }

    SlotInventario& slot = slots[indice];

    if (!manteniendoItem) {
        if (!esItemVacio(slot.item)) {
            itemCursor = slot;
            slot = {};
            manteniendoItem = true;
            eventoMovimientoItem = true;
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
        eventoMovimientoItem = true;
    } else if (slot.item == itemCursor.item) {
        int espacio = maxStack(slot.item) - slot.cantidad;
        int mover = std::min(espacio, itemCursor.cantidad);
        slot.cantidad += mover;
        itemCursor.cantidad -= mover;
        limpiarSlotSiVacio(itemCursor);
        manteniendoItem = !esItemVacio(itemCursor.item);
        if (mover > 0) eventoMovimientoItem = true;
    } else {
        std::swap(slot, itemCursor);
        eventoMovimientoItem = true;
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
            eventoMovimientoItem = true;
        }
        actualizarResultadoCrafteo();
        actualizarResultadoMesa();
        return;
    }

    if (!puedeColocarEnSlot(indice, itemCursor.item)) return;

    if (esItemVacio(slot.item)) {
        slot = {itemCursor.item, 1};
        itemCursor.cantidad -= 1;
        eventoMovimientoItem = true;
    } else if (slot.item == itemCursor.item && slot.cantidad < maxStack(slot.item)) {
        slot.cantidad += 1;
        itemCursor.cantidad -= 1;
        eventoMovimientoItem = true;
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

    if (mesaCrafteoAbierta) {
        if (clicIzquierdo && !clicIzquierdoAnterior) {
            manejarClickCatalogoMesa(posicionMouse);
        }
        bool enterCatalogo = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter);
        if (enterCatalogo && !enterCatalogoAnterior) {
            const auto& recetas = recetasCatalogoMesa();
            int visible = 0;
            for (int i = 0; i < static_cast<int>(recetas.size()); ++i) {
                if (recetas[i].categoria != categoriaMesa) continue;
                if (visible == recetaMesaSeleccionada) {
                    fabricarRecetaCatalogo(i);
                    break;
                }
                ++visible;
            }
        }
        clicIzquierdoAnterior = clicIzquierdo;
        clicDerechoAnterior = clicDerecho;
        enterCatalogoAnterior = enterCatalogo;
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
        ventana.setView(ventana.getDefaultView());

        sf::RectangleShape fondo({800.0f, 600.0f});
        fondo.setFillColor(sf::Color(116, 116, 116, 246));
        ventana.draw(fondo);

        sf::RectangleShape panel({748.0f, 520.0f});
        panel.setPosition({26.0f, 44.0f});
        panel.setFillColor(sf::Color(198, 198, 198, 250));
        panel.setOutlineColor(sf::Color(36, 36, 36));
        panel.setOutlineThickness(4.0f);
        ventana.draw(panel);

        sf::Text titulo(fuente, "Mesa de Crafteo", 22);
        titulo.setFillColor(sf::Color(62, 62, 62));
        titulo.setOutlineColor(sf::Color(238, 238, 238));
        titulo.setOutlineThickness(1.0f);
        titulo.setPosition({40.0f, 10.0f});
        ventana.draw(titulo);

        const char* categorias[4] = {"Bloques", "Herr.", "Armad.", "Comida"};
        ItemId iconosCategoria[4] = {ItemId::TablonMadera, ItemId::PicoMadera, ItemId::Lana, ItemId::Zanahoria};
        for (int i = 0; i < 4; ++i) {
            sf::Vector2f posTab(34.0f + static_cast<float>(i) * 86.0f, 34.0f);
            bool activo = i == categoriaMesa;
            sf::RectangleShape tab({72.0f, 42.0f});
            tab.setPosition(posTab);
            tab.setFillColor(activo ? sf::Color(232, 232, 232) : sf::Color(136, 136, 136));
            tab.setOutlineColor(activo ? sf::Color(76, 190, 70) : sf::Color(35, 35, 35));
            tab.setOutlineThickness(activo ? 3.0f : 2.0f);
            ventana.draw(tab);
            dibujarItemSprite(ventana, iconosCategoria[i], {posTab.x + 8.0f, posTab.y + 6.0f}, 1.2f);
            sf::Text etiqueta(fuente, categorias[i], 9);
            etiqueta.setFillColor(sf::Color(42, 42, 42));
            etiqueta.setPosition({posTab.x + 30.0f, posTab.y + 16.0f});
            ventana.draw(etiqueta);
        }

        sf::RectangleShape lista({350.0f, 422.0f});
        lista.setPosition({40.0f, 104.0f});
        lista.setFillColor(sf::Color(154, 154, 154));
        lista.setOutlineColor(sf::Color(52, 52, 52));
        lista.setOutlineThickness(3.0f);
        ventana.draw(lista);

        sf::RectangleShape panelInfo({330.0f, 422.0f});
        panelInfo.setPosition({420.0f, 104.0f});
        panelInfo.setFillColor(sf::Color(176, 176, 176));
        panelInfo.setOutlineColor(sf::Color(52, 52, 52));
        panelInfo.setOutlineThickness(3.0f);
        ventana.draw(panelInfo);

        std::vector<int> recetasVisibles;
        const auto& recetas = recetasCatalogoMesa();
        for (int i = 0; i < static_cast<int>(recetas.size()); ++i) {
            if (recetas[i].categoria == categoriaMesa) recetasVisibles.push_back(i);
        }
        if (recetaMesaSeleccionada >= static_cast<int>(recetasVisibles.size())) {
            recetaMesaSeleccionada = std::max(0, static_cast<int>(recetasVisibles.size()) - 1);
        }

        sf::Vector2f mouseF(static_cast<float>(mouse.x), static_cast<float>(mouse.y));
        for (int visible = 0; visible < static_cast<int>(recetasVisibles.size()); ++visible) {
            int recetaIndice = recetasVisibles[visible];
            const RecetaCatalogoMesa& receta = recetas[recetaIndice];
            int col = visible % 4;
            int fila = visible / 4;
            sf::Vector2f posCelda(56.0f + static_cast<float>(col) * 74.0f, 128.0f + static_cast<float>(fila) * 72.0f);
            bool disponible = tieneIngredientesCatalogo(receta.ingredientes) && puedeAgregarItem(receta.resultado, receta.cantidadResultado);
            bool seleccionado = visible == recetaMesaSeleccionada;
            bool sobre = sf::FloatRect(posCelda, {56.0f, 56.0f}).contains(mouseF);

            sf::RectangleShape celda({56.0f, 56.0f});
            celda.setPosition(posCelda);
            celda.setFillColor(seleccionado ? sf::Color(218, 218, 218) : sf::Color(96, 96, 96));
            celda.setOutlineColor(seleccionado ? sf::Color(255, 255, 120) : (sobre ? sf::Color(235, 235, 235) : sf::Color(28, 28, 28)));
            celda.setOutlineThickness(seleccionado ? 3.0f : 2.0f);
            ventana.draw(celda);

            dibujarItemSprite(ventana, receta.resultado, {posCelda.x + 10.0f, posCelda.y + 8.0f}, 1.9f);
            if (!disponible) {
                sf::RectangleShape bloqueo({56.0f, 56.0f});
                bloqueo.setPosition(posCelda);
                bloqueo.setFillColor(sf::Color(30, 30, 30, 138));
                ventana.draw(bloqueo);
            }
            if (receta.cantidadResultado > 1) {
                sf::Text cantidad(fuente, std::to_string(receta.cantidadResultado), 12);
                cantidad.setFillColor(sf::Color::White);
                cantidad.setOutlineColor(sf::Color::Black);
                cantidad.setOutlineThickness(2.0f);
                cantidad.setPosition({posCelda.x + 39.0f, posCelda.y + 38.0f});
                ventana.draw(cantidad);
            }
        }

        if (!recetasVisibles.empty()) {
            const RecetaCatalogoMesa& seleccionada = recetas[recetasVisibles[recetaMesaSeleccionada]];
            bool puedeFabricar = tieneIngredientesCatalogo(seleccionada.ingredientes) &&
                                 puedeAgregarItem(seleccionada.resultado, seleccionada.cantidadResultado);

            sf::Text nombre(fuente, seleccionada.nombre, 18);
            nombre.setFillColor(sf::Color(45, 45, 45));
            nombre.setOutlineColor(sf::Color(235, 235, 235));
            nombre.setOutlineThickness(1.0f);
            nombre.setPosition({442.0f, 124.0f});
            ventana.draw(nombre);

            sf::Text descripcion(fuente, seleccionada.descripcion, 11);
            descripcion.setFillColor(sf::Color(70, 70, 70));
            descripcion.setPosition({442.0f, 154.0f});
            ventana.draw(descripcion);

            sf::Text recetaTxt(fuente, "Receta", 14);
            recetaTxt.setFillColor(sf::Color(55, 55, 55));
            recetaTxt.setPosition({442.0f, 190.0f});
            ventana.draw(recetaTxt);

            for (int i = 0; i < 9; ++i) {
                int col = i % 3;
                int fila = i / 3;
                sf::Vector2f posSlot(444.0f + static_cast<float>(col) * 38.0f, 218.0f + static_cast<float>(fila) * 38.0f);
                sf::RectangleShape slot({32.0f, 32.0f});
                slot.setPosition(posSlot);
                slot.setFillColor(sf::Color(92, 92, 92));
                slot.setOutlineColor(sf::Color(30, 30, 30));
                slot.setOutlineThickness(2.0f);
                ventana.draw(slot);
                if (!esItemVacio(seleccionada.matriz[i])) {
                    dibujarItemSprite(ventana, seleccionada.matriz[i], {posSlot.x + 2.0f, posSlot.y + 2.0f}, 1.25f);
                }
            }

            dibujarItemSprite(ventana, seleccionada.resultado, {616.0f, 232.0f}, 2.0f);
            sf::Text resultado(fuente, "Resultado", 12);
            resultado.setFillColor(sf::Color(60, 60, 60));
            resultado.setPosition({602.0f, 310.0f});
            ventana.draw(resultado);

            sf::Text materiales(fuente, "Materiales", 14);
            materiales.setFillColor(sf::Color(55, 55, 55));
            materiales.setPosition({442.0f, 348.0f});
            ventana.draw(materiales);

            for (int i = 0; i < static_cast<int>(seleccionada.ingredientes.size()); ++i) {
                const SlotInventario& ing = seleccionada.ingredientes[i];
                int tienes = contarItemBanco(ing.item);
                sf::Vector2f posIng(442.0f, 376.0f + static_cast<float>(i) * 28.0f);
                dibujarItemSprite(ventana, ing.item, posIng, 0.95f);
                std::string texto = nombreItem(ing.item) + ": " + std::to_string(tienes) + " / " + std::to_string(ing.cantidad);
                sf::Text linea(fuente, texto, 12);
                linea.setFillColor(tienes >= ing.cantidad ? sf::Color(42, 105, 35) : sf::Color(155, 45, 45));
                linea.setPosition({posIng.x + 28.0f, posIng.y + 5.0f});
                ventana.draw(linea);
            }

            auto dibujarBotonFabricacion = [&](sf::FloatRect rect, const std::string& texto, unsigned int tamanoTexto) {
                sf::RectangleShape boton(rect.size);
                boton.setPosition(rect.position);
                boton.setFillColor(puedeFabricar ? sf::Color(172, 172, 172) : sf::Color(88, 88, 88));
                boton.setOutlineColor(rect.contains(mouseF) && puedeFabricar ? sf::Color(255, 255, 120) : sf::Color(28, 28, 28));
                boton.setOutlineThickness(3.0f);
                ventana.draw(boton);

                sf::Text etiqueta(fuente, texto, tamanoTexto);
                etiqueta.setFillColor(puedeFabricar ? sf::Color::White : sf::Color(160, 160, 160));
                etiqueta.setOutlineColor(sf::Color::Black);
                etiqueta.setOutlineThickness(2.0f);
                sf::FloatRect bounds = etiqueta.getLocalBounds();
                etiqueta.setOrigin({bounds.position.x + bounds.size.x * 0.5f, bounds.position.y + bounds.size.y * 0.5f});
                etiqueta.setPosition({rect.position.x + rect.size.x * 0.5f, rect.position.y + rect.size.y * 0.5f});
                ventana.draw(etiqueta);
            };

            dibujarBotonFabricacion({{512.0f, 506.0f}, {108.0f, 38.0f}}, "Fabricar", 14);
            dibujarBotonFabricacion({{632.0f, 506.0f}, {108.0f, 38.0f}}, "Max", 16);
        }

        sf::Text ayuda(fuente, "Click: seleccionar   Enter: fabricar   Max: repetir   ESC/E: cerrar", 12);
        ayuda.setFillColor(sf::Color(38, 38, 38));
        ayuda.setPosition({42.0f, 540.0f});
        ventana.draw(ayuda);

        if (manteniendoItem && !esItemVacio(itemCursor.item)) {
            dibujarItemSprite(
                ventana,
                itemCursor.item,
                {static_cast<float>(mouse.x - 14), static_cast<float>(mouse.y - 14)},
                1.8f
            );
        }
        return;
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

        if (!dibujarRetratoInventario(ventana)) {
            sf::RectangleShape cabeza({34.0f, 34.0f});
            cabeza.setPosition({PLAYER_POS.x + 68.0f, PLAYER_POS.y + 34.0f});
            cabeza.setFillColor(sf::Color(185, 55, 55));
            ventana.draw(cabeza);
        }

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

        if (esItemVacio(slots[indice].item) && !mesaCrafteoAbierta) {
            sf::Color guia(185, 185, 185, 72);
            auto rectGuia = [&](float x, float y, float w, float h) {
                sf::RectangleShape r({w, h});
                r.setPosition({pos.x + x, pos.y + y});
                r.setFillColor(guia);
                ventana.draw(r);
            };

            if (indice >= INDICE_ARMADURA && indice < INDICE_ARMADURA + 4) {
                int pieza = indice - INDICE_ARMADURA;
                if (pieza == 0) {
                    rectGuia(12.0f, 9.0f, 16.0f, 6.0f);
                    rectGuia(9.0f, 15.0f, 22.0f, 10.0f);
                } else if (pieza == 1) {
                    rectGuia(10.0f, 9.0f, 20.0f, 22.0f);
                    rectGuia(7.0f, 14.0f, 5.0f, 12.0f);
                    rectGuia(28.0f, 14.0f, 5.0f, 12.0f);
                } else if (pieza == 2) {
                    rectGuia(10.0f, 8.0f, 20.0f, 10.0f);
                    rectGuia(10.0f, 18.0f, 8.0f, 15.0f);
                    rectGuia(22.0f, 18.0f, 8.0f, 15.0f);
                } else {
                    rectGuia(9.0f, 24.0f, 10.0f, 8.0f);
                    rectGuia(21.0f, 24.0f, 10.0f, 8.0f);
                }
            } else if (indice == INDICE_SEGUNDA_MANO) {
                rectGuia(12.0f, 8.0f, 16.0f, 24.0f);
                rectGuia(8.0f, 13.0f, 24.0f, 13.0f);
            }
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

    if (!manteniendoItem &&
        hover >= 0 &&
        hover < static_cast<int>(slots.size()) &&
        !esItemVacio(slots[hover].item)) {
        std::string textoTooltip = nombreItem(slots[hover].item);
        if (slots[hover].cantidad > 1) {
            textoTooltip += " x" + std::to_string(slots[hover].cantidad);
        }
        dibujarTooltipInventario(ventana, fuente, textoTooltip, mouse);
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


