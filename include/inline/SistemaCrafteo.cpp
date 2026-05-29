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
    // Inicializar la mesa de crafteo completamente vacÃ­a (con Aire)
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            matrizEntrada[i][j] = TipoBloque::Aire;
        }
    }
    ranuraSalida.tipo = TipoBloque::Aire;
    ranuraSalida.cantidad = 0;

    // Cargar las recetas del juego en la Fase 1
    inicializarRecetasBase();
}

inline SistemaCrafteo::~SistemaCrafteo() {}

inline void SistemaCrafteo::alternarMenu() {
    menuAbierto = !menuAbierto;
}

inline bool SistemaCrafteo::esMenuAbierto() const {
    return menuAbierto;
}

inline void SistemaCrafteo::registrarReceta(RecetaMatriz patron, TipoBloque resultado, int cantidad) {
    libroRecetas[patron] = {resultado, cantidad};
}

inline void SistemaCrafteo::inicializarRecetasBase() {
    // -------------------------------------------------------------------
    // RECETA BASE 1: 1 Tronco de Madera -> 4 Tablas de Madera Procesada
    // Ponemos el Tronco en la esquina superior izquierda [0][0], lo demÃ¡s Aire
    // -------------------------------------------------------------------
    RecetaMatriz recetaMadera;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            recetaMadera.matriz[i][j] = TipoBloque::Aire;
        }
    }
    
    // NOTA: AsegÃºrate de que en tu 'Mundo.hpp' tu bloque de madera reciÃ©n talada 
    // se llame 'Madera' o similar. Si tienes 'TipoBloque::Madera', aquÃ­ lo usamos:
    recetaMadera.matriz[0][0] = TipoBloque::Madera; 

    // Registramos: Ese patrÃ³n da como resultado 4 bloques de Madera (Tablas)
    registrarReceta(recetaMadera, TipoBloque::Madera, 4);
    
    std::cout << "[Mesa Crafteo] Fase 1: Recetas base inicializadas con exito." << std::endl;
}

inline void SistemaCrafteo::verificarCrafteo() {
    // CORREGIDO: Convertimos explÃ­citamente la matriz nativa actual en un objeto RecetaMatriz
    RecetaMatriz intentoActual;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            intentoActual.matriz[i][j] = matrizEntrada[i][j];
        }
    }

    // Ahora buscamos el objeto struct directamente dentro del mapa de recetas
    auto it = libroRecetas.find(intentoActual);
    
    if (it != libroRecetas.end()) {
        ranuraSalida = it->second; // Se activa el Ã­tem de salida
    } else {
        // Si no coincide con ninguna receta, la salida se queda vacÃ­a
        ranuraSalida.tipo = TipoBloque::Aire;
        ranuraSalida.cantidad = 0;
    }
}

inline void SistemaCrafteo::manejarClicks(sf::Vector2i posicionMouse, bool clicPresionado, TipoBloque itemEnMano) {
    if (!menuAbierto || !clicPresionado) return;

    // La lÃ³gica de interacciÃ³n por clicks y arrastres la desarrollaremos en la Fase 2.
    // De momento, dejamos la verificaciÃ³n en escucha.
    verificarCrafteo();
}

inline void SistemaCrafteo::dibujar(sf::RenderWindow& ventana, sf::Font& fuente) {
    if (!menuAbierto) return;

    // Fondo de la Mesa - EMPUJADO AÃšN MÃS A LA DERECHA (X increased dramatically)
    sf::RectangleShape fondoCrafteo({230.0f, 160.0f});
    fondoCrafteo.setPosition({400.0f, 150.0f}); // X increased from 540 to 600
    fondoCrafteo.setFillColor(sf::Color(30, 30, 30, 240));
    fondoCrafteo.setOutlineColor(sf::Color::White);
    fondoCrafteo.setOutlineThickness(1.0f);
    ventana.draw(fondoCrafteo);

    // Dibujar la cuadrÃ­cula 3x3 movida a su nueva posiciÃ³n (X increased dramatically)
    for (int fila = 0; fila < 3; ++fila) {
        for (int col = 0; col < 3; ++col) {
            sf::RectangleShape celda({TAMANIO_CUADRO, TAMANIO_CUADRO});
            // X increased from 550 to 610 to stay centered in the new background
            celda.setPosition({610.0f + (col * 45.0f), 158.0f + (fila * 45.0f)});
            celda.setFillColor(sf::Color(60, 60, 60));
            celda.setOutlineColor(sf::Color(120, 120, 120));
            celda.setOutlineThickness(1.0f);
            ventana.draw(celda);
        }
    }

    // Flecha indicadora de crafteo (->) empujada a la derecha (X increased dramatically)
    sf::Text flecha(fuente, "->", 18);
    flecha.setPosition({755.0f, 215.0f}); // X increased from 695 to 755
    flecha.setFillColor(sf::Color::White);
    ventana.draw(flecha);

    // Casilla de Salida del crafteo empujada a la derecha (X increased dramatically)
    sf::RectangleShape celdaSalida({TAMANIO_CUADRO, TAMANIO_CUADRO});
    celdaSalida.setPosition({778.0f, 210.0f}); // X increased from 718 to 778
    celdaSalida.setFillColor(sf::Color(50, 50, 50));
    celdaSalida.setOutlineColor(sf::Color::Green);
    celdaSalida.setOutlineThickness(1.0f);
    ventana.draw(celdaSalida);
}

