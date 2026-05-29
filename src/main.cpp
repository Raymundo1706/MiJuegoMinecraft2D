#include "Juego.hpp"
#include <iostream>

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "        INICIANDO DETECTOR DE CAMBIOS   " << std::endl;
    std::cout << "========================================" << std::endl;
    
    Juego juego;
    juego.ejecutar();
    
    return 0;
}
