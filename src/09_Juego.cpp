#include "Juego.hpp"
#include <iostream>

Juego::Juego() 
    : ventana(sf::VideoMode({800, 600}), "Minecraft 2D - Exploracion Activa"),
      estaCorriendo(true) 
{
    mapaSuperficie = std::make_unique<Mundo>(1000, 1000);
    
    // Hacemos que el jugador aparezca un poco más adentro de la isla (coordenada de pixeles)
    jugador = std::make_unique<Jugador>(600.0f, 600.0f);

    // Inicializamos la cámara para que tenga el tamaño de la ventana (800x600)
    camara.setSize({800.0f, 600.0f});

    std::cout << "¡Sistema de camara listo para el desplazamiento!" << std::endl;
}

Juego::~Juego() {}

void Juego::ejecutar() {
    sf::Clock reloj; 

    while (ventana.isOpen() && estaCorriendo) {
        while (const std::optional<sf::Event> evento = ventana.pollEvent()) {
            if (evento->is<sf::Event::Closed>()) {
                ventana.close();
            }
        }

        float dt = reloj.restart().asSeconds();
        
        if (jugador) {
            jugador->controlar(dt);
            
            // LA MAGIA: Centramos la cámara en la posición actual del jugador
            camara.setCenter(jugador->getPosicion());
        }

        // RENDERIZADO
        ventana.clear(sf::Color::Black);
        
        // Aplicamos la cámara a la ventana ANTES de dibujar
        ventana.setView(camara);
        
        if (mapaSuperficie) {
            mapaSuperficie->dibujar(ventana);
        }
        
        if (jugador) {
            jugador->dibujar(ventana);
        }
        
        ventana.display();
    }
}