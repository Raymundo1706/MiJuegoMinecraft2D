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
    bool esSolido;       // Si el jugador choca con el (Piedra, Madera, AguaProfunda)
    int vida;            // Cuantos golpes le quedan antes de romperse
    bool estaHidratado;  // Para agricultura: si tiene agua cerca
};

class Mundo {
private:
    int ancho;
    int alto;

    // Matriz bidimensional de bloques para la capa actual
    // std::vector de std::vectores es la forma mas segura y dinamica en C++
    std::vector<std::vector<Bloque>> cuadricula;

public:
    // Constructor: Define las dimensiones del mundo (ej. 1000x1000)
    Mundo(int ancho, int alto);
    
    // Destructor
    ~Mundo();

    // Metodo para rellenar el mapa de forma aleatoria segun el bioma y reglas
    void generarMundo(bool esSubterraneo);

    // Metodo para dibujar los bloques visibles en la pantalla
    void dibujar(sf::RenderWindow& ventana);

    // Getters basicos
    int getAncho() const { return ancho; }
    int getAlto() const { return alto; }
};

#endif // MUNDO_HPP