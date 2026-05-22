#include "Juego.hpp"
#include <iostream>

// Constructor: Inicializamos la ventana y creamos los objetos Mundo y Jugador
Juego::Juego() 
    : ventana(sf::VideoMode({800, 600}), "Minecraft 2D - ¡Jugador Activo!"),
      estaCorriendo(true) 
{
    // Instanciamos el mundo
    mapaSuperficie = std::make_unique<Mundo>(1000, 1000);
    
    // Instanciamos al jugador justo en el centro de la ventana (X: 400, Y: 300)
    jugador = std::make_unique<Jugador>(400.0f, 300.0f);

    std::cout << "¡Mundo y Jugador cargados en el motor correctamente!" << std::endl;
}

Juego::~Juego() {}

void Juego::ejecutar() {
    sf::Clock reloj; 

    while (ventana.isOpen() && estaCorriendo) {
        // 1. PROCESAR EVENTOS (Ventana)
        while (const std::optional<sf::Event> evento = ventana.pollEvent()) {
            if (evento->is<sf::Event::Closed>()) {
                ventana.close();
            }
        }

        // 2. ACTUALIZAR LÓGICA (Calculamos el tiempo delta por fotograma)
        float dt = reloj.restart().asSeconds();
        
        // POO en accion: Le ordenamos al jugador que revise el teclado y se mueva
        if (jugador) {
            jugador->controlar(dt);
        }

        // 3. RENDERIZADO (Dibujo en pantalla)
        ventana.clear(sf::Color::Black);
        
        // Primero dibujamos el fondo (el mundo de pasto)
        if (mapaSuperficie) {
            mapaSuperficie->dibujar(ventana);
        }
        
        // Después dibujamos al jugador ENCIMA del mundo para que no quede oculto
        if (jugador) {
            jugador->dibujar(ventana);
        }
        
        ventana.display();
    }
}