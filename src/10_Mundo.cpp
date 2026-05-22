#include "Mundo.hpp"
#include <iostream>

// Constructor: Reservamos el tamaño de la matriz de bloques
Mundo::Mundo(int ancho, int alto) : ancho(ancho), alto(alto) {
    // Redimensionamos la matriz exterior (filas)
    cuadricula.resize(alto);
    
    // Redimensionamos cada fila para tener las columnas correspondientes
    for (int i = 0; i < alto; ++i) {
        cuadricula[i].resize(ancho);
    }
    
    // ¡CONEXIÓN POO!: Forzamos al objeto a rellenar su propia matriz con Pasto al nacer
    generarMundo(false);
    
    std::cout << "¡Matriz del mundo de " << ancho << "x" << alto << " creada con exito!" << std::endl;
}

// Destructor
Mundo::~Mundo() {}

// Rellenamos el mundo con bloques por defecto por ahora
void Mundo::generarMundo(bool esSubterraneo) {
    // Definimos el grosor del océano en los bordes (ej. 15 bloques de ancho)
    const int MARGEN_OCEANO = 15;

    for (int y = 0; y < alto; ++y) {
        for (int x = 0; x < ancho; ++x) {
            if (esSubterraneo) {
                // Si es el subterráneo, todo empieza lleno de Piedra
                cuadricula[y][x] = { TipoBloque::Piedra, true, 5, false };
            } else {
                // REGLA DE LA ISLA: Si está en los bordes del mapa, es Agua Profunda
                if (x < MARGEN_OCEANO || x >= (ancho - MARGEN_OCEANO) ||
                    y < MARGEN_OCEANO || y >= (alto - MARGEN_OCEANO)) {
                    
                    cuadricula[y][x] = { TipoBloque::AguaProfunda, true, 999, false };
                } else {
                    // El interior de la isla inicia con Pasto base
                    cuadricula[y][x] = { TipoBloque::Pasto, false, 3, false };
                }
            }
        }
    }
    std::cout << "¡Bordes de la isla generados correctamente!" << std::endl;
}

// Metodo para dibujar los bloques en la ventana
void Mundo::dibujar(sf::RenderWindow& ventana) {
    // Definimos el tamaño visual en pixeles de cada bloque (ej. 32x32 pixeles)
    const float TAMANIO_BLOQUE = 32.0f;
    
    // Usamos una figura simple de SFML para representar el bloque en pantalla
    sf::RectangleShape formaBlq({TAMANIO_BLOQUE, TAMANIO_BLOQUE});

    // Por ahora, para probar el rendimiento, solo dibujaremos lo que quepa en una zona pequeña (ej. 25x20 bloques)
    // Mas adelante usaremos una "Camara" (sf::View) para que solo se dibuje lo que el jugador ve
    int bloquesVisiblesX = 25;
    int bloquesVisiblesY = 20;

    for (int y = 0; y < bloquesVisiblesY && y < alto; ++y) {
        for (int x = 0; x < bloquesVisiblesX && x < ancho; ++x) {
            
            // Elegimos el color dependiendo del tipo de bloque
            // Catálogo de colores según el tipo de bloque
            if (cuadricula[y][x].tipo == TipoBloque::Pasto) {
                formaBlq.setFillColor(sf::Color(34, 139, 34)); // Verde bosque
            } else if (cuadricula[y][x].tipo == TipoBloque::Agua) {
                formaBlq.setFillColor(sf::Color(30, 144, 255)); // Azul brillante (Lagos)
            } else if (cuadricula[y][x].tipo == TipoBloque::AguaProfunda) {
                formaBlq.setFillColor(sf::Color(0, 0, 139)); // Azul oscuro (Bordes del mapa)
            } else if (cuadricula[y][x].tipo == TipoBloque::Madera) {
                formaBlq.setFillColor(sf::Color(139, 69, 19)); // Marrón (Árboles)
            } else if (cuadricula[y][x].tipo == TipoBloque::Piedra) {
                formaBlq.setFillColor(sf::Color(128, 128, 128)); // Gris oscuro
            } else if (cuadricula[y][x].tipo == TipoBloque::MineralHierro) {
                formaBlq.setFillColor(sf::Color(210, 180, 140)); // Beige/Café claro
            } else if (cuadricula[y][x].tipo == TipoBloque::MineralDiamante) {
                formaBlq.setFillColor(sf::Color(0, 255, 255)); // Cyan brillante
            } else {
                formaBlq.setFillColor(sf::Color(0, 100, 0)); // Verde oscuro por defecto
            }

            // Posicionamos el bloque en la cuadricula de la pantalla
            formaBlq.setPosition({x * TAMANIO_BLOQUE, y * TAMANIO_BLOQUE});
            
            // Dibujamos el bloque en la ventana de SFML
            ventana.draw(formaBlq);
        }
    }
}