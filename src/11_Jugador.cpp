#include "Jugador.hpp"

// Constructor: Lo ponemos al centro de la pantalla por defecto (400, 300) y de color rojo estilo Steve
Jugador::Jugador(float x, float y) {
    posicion = {x, y};
    velocidad = 250.0f; // Pixeles por segundo

    // Le damos forma de cuadrado/rectangulo (tamaño 24x24 pixeles)
    forma.setSize({24.0f, 24.0f});
    forma.setFillColor(sf::Color::Red); // Cuadro rojo para identificarlo facil
    forma.setPosition(posicion);
}

// Destructor
Jugador::~Jugador() {}

// Metodo para mover al personaje usando DeltaTime (dt) para que vaya fluido
void Jugador::controlar(float dt) {
    sf::Vector2f movimiento(0.0f, 0.0f);

    // Deteccion de teclas (Soporta WASD y Flechas del teclado)
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
        movimiento.y -= 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
        movimiento.y += 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
        movimiento.x -= 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
        movimiento.x += 1.0f;
    }

    // Si hay movimiento, actualizamos la posicion multiplicando por velocidad y dt
    if (movimiento.x != 0.0f || movimiento.y != 0.0f) {
        posicion += movimiento * velocidad * dt;
        forma.setPosition(posicion);
    }
}

// Metodo para pintar al jugador encima del mundo
void Jugador::dibujar(sf::RenderWindow& ventana) {
    ventana.draw(forma);
}