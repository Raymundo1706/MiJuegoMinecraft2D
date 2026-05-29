#include <iostream>

inline bool RecetaMatriz::operator<(const RecetaMatriz& otra) const {
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (matriz[i][j] != otra.matriz[i][j]) {
                return matriz[i][j] < otra.matriz[i][j];
            }
        }
    }
    return false;
}

inline SistemaCrafteo::SistemaCrafteo() : menuAbierto(false) {
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            matrizEntrada[i][j] = ItemId::Ninguno;
        }
    }
    ranuraSalida.item = ItemId::Ninguno;
    ranuraSalida.cantidad = 0;
    inicializarRecetasBase();
}

inline SistemaCrafteo::~SistemaCrafteo() {}

inline void SistemaCrafteo::alternarMenu() {
    menuAbierto = !menuAbierto;
}

inline bool SistemaCrafteo::esMenuAbierto() const {
    return menuAbierto;
}

inline void SistemaCrafteo::registrarReceta(RecetaMatriz patron, ItemId resultado, int cantidad) {
    libroRecetas[patron] = {resultado, cantidad};
}

inline void SistemaCrafteo::inicializarRecetasBase() {
    auto recetaVacia = []() {
        RecetaMatriz receta;
        for (int fila = 0; fila < 3; ++fila) {
            for (int col = 0; col < 3; ++col) {
                receta.matriz[fila][col] = ItemId::Ninguno;
            }
        }
        return receta;
    };

    for (int col = 0; col < 3; ++col) {
        for (int fila = 0; fila < 2; ++fila) {
            RecetaMatriz palos = recetaVacia();
            palos.matriz[fila][col] = ItemId::TablonMadera;
            palos.matriz[fila + 1][col] = ItemId::TablonMadera;
            registrarReceta(palos, ItemId::PaloMadera, 4);
        }
    }

    RecetaMatriz espadaMadera = recetaVacia();
    espadaMadera.matriz[0][1] = ItemId::TablonMadera;
    espadaMadera.matriz[1][1] = ItemId::TablonMadera;
    espadaMadera.matriz[2][1] = ItemId::PaloMadera;
    registrarReceta(espadaMadera, ItemId::EspadaMadera, 1);

    RecetaMatriz espadaPiedra = recetaVacia();
    espadaPiedra.matriz[0][1] = ItemId::BloquePiedra;
    espadaPiedra.matriz[1][1] = ItemId::BloquePiedra;
    espadaPiedra.matriz[2][1] = ItemId::PaloMadera;
    registrarReceta(espadaPiedra, ItemId::EspadaPiedra, 1);

    RecetaMatriz picoMadera = recetaVacia();
    picoMadera.matriz[0][0] = ItemId::TablonMadera;
    picoMadera.matriz[0][1] = ItemId::TablonMadera;
    picoMadera.matriz[0][2] = ItemId::TablonMadera;
    picoMadera.matriz[1][1] = ItemId::PaloMadera;
    picoMadera.matriz[2][1] = ItemId::PaloMadera;
    registrarReceta(picoMadera, ItemId::PicoMadera, 1);

    RecetaMatriz picoPiedra = recetaVacia();
    picoPiedra.matriz[0][0] = ItemId::BloquePiedra;
    picoPiedra.matriz[0][1] = ItemId::BloquePiedra;
    picoPiedra.matriz[0][2] = ItemId::BloquePiedra;
    picoPiedra.matriz[1][1] = ItemId::PaloMadera;
    picoPiedra.matriz[2][1] = ItemId::PaloMadera;
    registrarReceta(picoPiedra, ItemId::PicoPiedra, 1);

    RecetaMatriz hachaMadera = recetaVacia();
    hachaMadera.matriz[0][0] = ItemId::TablonMadera;
    hachaMadera.matriz[0][1] = ItemId::TablonMadera;
    hachaMadera.matriz[1][0] = ItemId::TablonMadera;
    hachaMadera.matriz[1][1] = ItemId::PaloMadera;
    hachaMadera.matriz[2][1] = ItemId::PaloMadera;
    registrarReceta(hachaMadera, ItemId::HachaMadera, 1);

    RecetaMatriz hachaPiedra = recetaVacia();
    hachaPiedra.matriz[0][0] = ItemId::BloquePiedra;
    hachaPiedra.matriz[0][1] = ItemId::BloquePiedra;
    hachaPiedra.matriz[1][0] = ItemId::BloquePiedra;
    hachaPiedra.matriz[1][1] = ItemId::PaloMadera;
    hachaPiedra.matriz[2][1] = ItemId::PaloMadera;
    registrarReceta(hachaPiedra, ItemId::HachaPiedra, 1);

    RecetaMatriz palaMadera = recetaVacia();
    palaMadera.matriz[0][1] = ItemId::TablonMadera;
    palaMadera.matriz[1][1] = ItemId::PaloMadera;
    palaMadera.matriz[2][1] = ItemId::PaloMadera;
    registrarReceta(palaMadera, ItemId::PalaMadera, 1);

    RecetaMatriz palaPiedra = recetaVacia();
    palaPiedra.matriz[0][1] = ItemId::BloquePiedra;
    palaPiedra.matriz[1][1] = ItemId::PaloMadera;
    palaPiedra.matriz[2][1] = ItemId::PaloMadera;
    registrarReceta(palaPiedra, ItemId::PalaPiedra, 1);

    RecetaMatriz horno = recetaVacia();
    for (int fila = 0; fila < 3; ++fila) {
        for (int col = 0; col < 3; ++col) {
            if (!(fila == 1 && col == 1)) {
                horno.matriz[fila][col] = ItemId::BloquePiedra;
            }
        }
    }
    registrarReceta(horno, ItemId::Horno, 1);

    RecetaMatriz cama = recetaVacia();
    cama.matriz[0][0] = ItemId::Lana;
    cama.matriz[0][1] = ItemId::Lana;
    cama.matriz[0][2] = ItemId::Lana;
    cama.matriz[1][0] = ItemId::TablonMadera;
    cama.matriz[1][1] = ItemId::TablonMadera;
    cama.matriz[1][2] = ItemId::TablonMadera;
    registrarReceta(cama, ItemId::Cama, 1);

    std::cout << "[Mesa Crafteo] Recetas 3x3 ocultas inicializadas." << std::endl;
}

inline void SistemaCrafteo::verificarCrafteo() {
    RecetaMatriz intentoActual;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            intentoActual.matriz[i][j] = matrizEntrada[i][j];
        }
    }

    auto it = libroRecetas.find(intentoActual);
    if (it != libroRecetas.end()) {
        ranuraSalida = it->second;
    } else {
        ranuraSalida.item = ItemId::Ninguno;
        ranuraSalida.cantidad = 0;
    }
}

inline void SistemaCrafteo::manejarClicks(sf::Vector2i posicionMouse, bool clicPresionado, ItemId itemEnMano) {
    if (!menuAbierto || !clicPresionado) return;
    (void)posicionMouse;
    (void)itemEnMano;
    verificarCrafteo();
}

inline void SistemaCrafteo::dibujar(sf::RenderWindow& ventana, sf::Font& fuente) {
    if (!menuAbierto) return;

    sf::RectangleShape fondoCrafteo({230.0f, 160.0f});
    fondoCrafteo.setPosition({400.0f, 150.0f});
    fondoCrafteo.setFillColor(sf::Color(30, 30, 30, 240));
    fondoCrafteo.setOutlineColor(sf::Color::White);
    fondoCrafteo.setOutlineThickness(1.0f);
    ventana.draw(fondoCrafteo);

    for (int fila = 0; fila < 3; ++fila) {
        for (int col = 0; col < 3; ++col) {
            sf::RectangleShape celda({TAMANIO_CUADRO, TAMANIO_CUADRO});
            celda.setPosition({410.0f + (col * 45.0f), 158.0f + (fila * 45.0f)});
            celda.setFillColor(sf::Color(60, 60, 60));
            celda.setOutlineColor(sf::Color(120, 120, 120));
            celda.setOutlineThickness(1.0f);
            ventana.draw(celda);
        }
    }

    sf::Text flecha(fuente, "->", 18);
    flecha.setPosition({555.0f, 215.0f});
    flecha.setFillColor(sf::Color::White);
    ventana.draw(flecha);

    sf::RectangleShape celdaSalida({TAMANIO_CUADRO, TAMANIO_CUADRO});
    celdaSalida.setPosition({585.0f, 210.0f});
    celdaSalida.setFillColor(sf::Color(50, 50, 50));
    celdaSalida.setOutlineColor(sf::Color::Green);
    celdaSalida.setOutlineThickness(1.0f);
    ventana.draw(celdaSalida);
}
