#include "Juego.hpp"

int main() {
    // Instanciamos (creamos) el objeto principal de nuestro juego
    Juego miJuego;

    // Le ordenamos al objeto que ejecute su ciclo de vida encapsulado
    miJuego.ejecutar();

    return 0;
}