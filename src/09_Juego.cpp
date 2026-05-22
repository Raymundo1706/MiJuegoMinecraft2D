#include "Juego.hpp"
#include <iostream>

// Constructor: Aqui creamos el objeto Mundo usando el constructor de Mundo
Juego::Juego() 
    : ventana(sf::VideoMode({800, 600}), "Minecraft 2D - Cuadrícula Activa"),
      estaCorriendo(true) 
{
    // Instanciamos el mundo en el puntero inteligente
    mapaSuperficie = std::make_unique<Mundo>(1000, 1000);
    
    std::cout << "¡Objeto Juego y Mundo inicializados correctamente!" << std::endl;
}

Juego::~Juego() {}

void Juego::ejecutar() {
    sf::Clock reloj; 

    // Bucle Principal del Juego
    while (ventana.isOpen() && estaCorriendo) {
        
        // 1. Procesar Eventos
        while (const std::optional<sf::Event> evento = ventana.pollEvent()) {
            if (evento->is<sf::Event::Closed>()) {
                ventana.close();
            }
        }

        // 2. Tiempo delta (dt) por si lo necesitamos luego
        float dt = reloj.restart().asSeconds();

        // 3. RENDERIZADO (Dibujo en pantalla)
        ventana.clear(sf::Color::Black); // Limpiamos el rastro del fotograma anterior
        
        // CONEXIÓN CRUCIAL: Si el mapa existe en memoria, le ordenamos pintarse
        if (mapaSuperficie) {
            mapaSuperficie->dibujar(ventana);
        }
        
        ventana.display(); // Mostramos el resultado final en la ventana
    }
}