#ifndef MUNDO_HPP
#define MUNDO_HPP

#include <vector>
#include <SFML/Graphics.hpp>

// Usamos un enumerador para identificar de manera limpia el tipo de cada bloque
enum class TipoBloque {
    Aire,
    Pasto,
    Tierra,
    Piedra,
    Agua,
    AguaProfunda, // La barrera invisible del borde de la isla
    Madera,
    MineralHierro,
    MineralOro,
    MineralDiamante,
    Redstone,
    TierraArada,  // Estado para agricultura (raspada con la herramienta)
    CuevaEntrada  // El agujero para bajar al subterraneo
};

// Estructura que representa las propiedades individuales de un solo bloque
struct Bloque {
    TipoBloque tipo;
    bool esSolido;       // Si el jugador choca con él (Piedra, Madera, AguaProfunda)
    float vida;          // <-- CAMBIADO A FLOAT: Para soportar daño decimal por fotograma
    bool estaHidratado;  // Para agricultura: si tiene agua cerca
};

class Mundo {
private:
    int ancho;
    int alto;

    // Matriz bidimensional de bloques para la capa actual
    std::vector<std::vector<Bloque>> cuadricula;

public:
    // Constructor: Define las dimensions del mundo (ej. 1000x1000)
    Mundo(int ancho, int alto);
    
    // Destructor
    ~Mundo();

    // Método para rellenar el mapa de forma aleatoria según el bioma y reglas
    void generarMundo(bool esSubterraneo);

    // Método para dibujar los bloques visibles en la pantalla
    void dibujar(sf::RenderWindow& ventana);

    // ============================================================
    // METODOS DEL SPRINT DE COLISIONES, RESISTENCIA Y MINADO
    // ============================================================
    bool esBloqueSolido(int x, int y) const;
    void romperBloque(int x, int y);
    TipoBloque getTipoBloque(int x, int y) const;
    int getVidaMaximaBloque(TipoBloque tipo) const;
    bool daniarBloque(int x, int y, float cantidadDanio); // <-- CORREGIDO A FLOAT AQUÍ

    // Getters básicos
    int getAncho() const { return ancho; }
    int getAlto() const { return alto; }
};

#endif // MUNDO_HPP