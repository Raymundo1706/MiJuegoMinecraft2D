#ifndef SISTEMA_CRAFTEO_HPP
#define SISTEMA_CRAFTEO_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <map>
#include "Mundo.hpp" // Para los tipos de bloques

// Estructura que representa la cuadrícula 3x3 de una receta
struct RecetaMatriz {
    TipoBloque matriz[3][3];

    // Operador de comparación para que el sistema busque la receta en un mapa exacto
    bool operator<(const RecetaMatriz& otra) const {
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                if (matriz[i][j] != otra.matriz[i][j]) {
                    return matriz[i][j] < otra.matriz[i][j];
                }
            }
        }
        return false;
    }
};

// Estructura para el resultado del crafteo
struct ResultadoCrafteo {
    TipoBloque tipo = TipoBloque::Aire;
    int cantidad = 0;
};

class SistemaCrafteo {
private:
    bool menuAbierto;
    
    // Las 9 casillas de la matriz de crafteo (0 al 8)
    TipoBloque matrizEntrada[3][3];
    
    // Casilla de salida con el ítem resultante
    ResultadoCrafteo ranuraSalida;

    // Diccionario/Mapa que guarda todas las recetas del juego
    std::map<RecetaMatriz, ResultadoCrafteo> libroRecetas;

    // Dimensiones visuales para el renderizado
    const float TAMANIO_CUADRO = 40.0f;

public:
    SistemaCrafteo();
    ~SistemaCrafteo();

    // Controles del Menú (Tecla R)
    void alternarMenu();
    bool esMenuAbierto() const;

    // Fase 1: Registro de recetas del juego
    void registrarReceta(RecetaMatriz patron, TipoBloque resultado, int cantidad);
    void inicializarRecetasBase();

    // Lógica interna: Revisa la matriz 3x3 actual y calcula qué debe salir
    void verificarCrafteo();
    
    // Interacción con el Mouse (Colocar materiales en la mesa)
    void manejarClicks(sf::Vector2i posicionMouse, bool clicPresionado, TipoBloque itemEnMano);

    // Dibujado del menú 3x3 en pantalla
    void dibujar(sf::RenderWindow& ventana, sf::Font& fuente);
};

#endif // SISTEMA_CRAFTEO_HPP