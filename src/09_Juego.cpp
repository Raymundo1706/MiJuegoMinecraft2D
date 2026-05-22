#include "Juego.hpp"
#include <iostream>
#include <random>
#include <sstream>

Juego::Juego() 
    : ventana(sf::VideoMode({800, 600}), "Minecraft 2D - Coordenadas Activas"),
      estaCorriendo(true),
      fuenteCargada(false) 
{
    mapaSuperficie = std::make_unique<Mundo>(1000, 1000);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> spawnX(50 * 32, 900 * 32);
    std::uniform_int_distribution<> spawnY(50 * 32, 900 * 32);

    float posX = static_cast<float>(spawnX(gen));
    float posY = static_cast<float>(spawnY(gen));
    
    jugador = std::make_unique<Jugador>(posX, posY);
    camara.setSize({800.0f, 600.0f});

    // ============================================================
    // MEJORA VISUAL: CARGAR FUENTE OFICIAL DE MINECRAFT
    // ============================================================
    // Usamos la ruta exacta donde guardaste el archivo: assets/fonts/
    // ¡Ojo con la 'M' mayúscula!
    if (fuente.openFromFile("assets/fonts/Minecraft.ttf")) {
        fuenteCargada = true;
        // Creamos el objeto de texto dinámicamente usando la fuente pixelada
        // Un tamaño de 16px es perfecto para que se vea nítida y retro
        textoCoordenadas = std::make_unique<sf::Text>(fuente, "", 16); 
        textoCoordenadas->setFillColor(sf::Color::White); // Letras blancas
        // Le añadimos un contorno negro para que sea legible sobre el pasto o el agua
        textoCoordenadas->setOutlineColor(sf::Color::Black);
        textoCoordenadas->setOutlineThickness(2.0f);
    } else {
        // Si sale este error, revisa que el archivo esté bien copiado en la carpeta assets/fonts/
        std::cout << "[Error Crítico] No se pudo cargar assets/fonts/Minecraft.ttf." << std::endl;
        std::cout << "Asegurate de que el archivo exista y el nombre sea exacto." << std::endl;
    }

    std::cout << "¡Spawn aleatorio fijado en X: " << (posX/32) << " Y: " << (posY/32) << "!" << std::endl;
}

Juego::~Juego() {}

void Juego::ejecutar() {
    sf::Clock reloj; 

    while (ventana.isOpen() && estaCorriendo) {
        // --- MANEJO DE EVENTOS ---
        while (const std::optional<sf::Event> evento = ventana.pollEvent()) {
            if (evento->is<sf::Event::Closed>()) {
                ventana.close();
            }

            // DETECCIÓN DE CLIC IZQUIERDO PARA MINAR (SFML 3)
            if (const auto* botonMouse = evento->getIf<sf::Event::MouseButtonPressed>()) {
                if (botonMouse->button == sf::Mouse::Button::Left) {
                    
                    // 1. Convertimos la posición del mouse de píxeles de pantalla a coordenadas del Mundo real
                    sf::Vector2i posicionMousePantalla = { botonMouse->position.x, botonMouse->position.y };
                    sf::Vector2f posicionMundo = ventana.mapPixelToCoords(posicionMousePantalla, camara);

                    // 2. Traducimos la posición del mundo flotante a índices enteros de nuestra cuadrícula (bloques de 32x32)
                    int bloqueX = static_cast<int>(posicionMundo.x / 32.0f);
                    int bloqueY = static_cast<int>(posicionMundo.y / 32.0f);

                    // 3. Verificamos la distancia entre el jugador y el bloque (Rango de minado)
                    if (jugador) {
                        sf::Vector2f posJugador = jugador->getPosicion();
                        // Centro aproximado del bloque clickeado en píxeles del mundo
                        sf::Vector2f centroBloque((bloqueX * 32.0f) + 16.0f, (bloqueY * 32.0f) + 16.0f);
                        
                        // Vector de distancia
                        float dx = centroBloque.x - (posJugador.x + 12.0f); // 12 es la mitad del tamaño 24 del jugador
                        float dy = centroBloque.y - (posJugador.y + 12.0f);
                        float distancia = std::sqrt(dx * dx + dy * dy);

                        // Límite de alcance: 120 píxeles (aproximadamente 3.5 bloques de distancia máxima)
                        if (distancia <= 120.0f) {
                            if (mapaSuperficie) {
                                mapaSuperficie->romperBloque(bloqueX, bloqueY);
                            }
                        }
                    }
                }
            }
        }

        // --- ACTUALIZACIÓN DE LÓGICA ---
        float dt = reloj.restart().asSeconds();
        
        if (jugador) {
            jugador->controlar(dt, *mapaSuperficie);
            camara.setCenter(jugador->getPosicion());
            
            int bloqueX = static_cast<int>(jugador->getPosicion().x / 32.0f);
            int bloqueY = static_cast<int>(jugador->getPosicion().y / 32.0f);

            if (fuenteCargada && textoCoordenadas) {
                std::stringstream ss;
                ss << "Bloque X: " << bloqueX << "\nBloque Y: " << bloqueY;
                textoCoordenadas->setString(ss.str());
            }
        }

        // --- RENDERIZADO / DIBUJO ---
        ventana.clear(sf::Color::Black);
        ventana.setView(camara);
        
        if (mapaSuperficie) { mapaSuperficie->dibujar(ventana); }
        if (jugador) { jugador->dibujar(ventana); }
        
        if (fuenteCargada && textoCoordenadas) {
            sf::Vector2f centroCamara = camara.getCenter();
            sf::Vector2f tamanioCamara = camara.getSize();
            float UI_X = centroCamara.x - (tamanioCamara.x / 2.0f) + 15.0f;
            float UI_Y = centroCamara.y - (tamanioCamara.y / 2.0f) + 15.0f;
            
            textoCoordenadas->setPosition({UI_X, UI_Y});
            ventana.draw(*textoCoordenadas);
        }
        
        ventana.display();
    }
}