#include "Juego.hpp"
#include <iostream>
#include <random>
#include <sstream>
#include <cmath>
#include "SistemaHerramientas.hpp"
#include "InventarioGrid.hpp" // <-- CORREGIDO: Inclusión necesaria para la rejilla

Juego::Juego() 
    : ventana(sf::VideoMode({800, 600}), "TEST DE CAMBIOS REALES"),
      estaCorriendo(true),
      fuenteCargada(false) 
{
    mapaSuperficie = std::make_unique<Mundo>(1000, 1000);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> spawnX(100 * 32, 800 * 32); 
    std::uniform_int_distribution<> spawnY(100 * 32, 800 * 32);

    float posX = static_cast<float>(spawnX(gen));
    float posY = static_cast<float>(spawnY(gen));
    
    jugador = std::make_unique<Jugador>(posX, posY);
    camara.setSize({800.0f, 600.0f});

    // ===================================================================
    // FAUNA ESPARCIDA POR TODO EL MEGA MAPA DE 1000x1000
    // ===================================================================
    std::uniform_real_distribution<float> disX(1600.0f, 30400.0f);
    std::uniform_real_distribution<float> disY(1600.0f, 30400.0f);

    for (int i = 0; i < 150; ++i) {
        float animalX = disX(gen);
        float animalY = disY(gen);
        
        TipoAnimal tipo = (i % 2 == 0) ? TipoAnimal::Cerdo : TipoAnimal::Oveja;
        animales.push_back(new Animal(animalX, animalY, tipo));
    }
    std::cout << "¡Fauna esparcida con exito por los 32,000 pixeles del mapa!" << std::endl;

    // CARGA DE FUENTE Y TEXTO
    if (fuente.openFromFile("assets/fonts/Minecraft.ttf")) {
        fuenteCargada = true;
        textoCoordenadas.emplace(fuente);
        textoCoordenadas->setCharacterSize(16);
        textoCoordenadas->setFillColor(sf::Color::White);
        textoCoordenadas->setOutlineColor(sf::Color::Black);
        textoCoordenadas->setOutlineThickness(2.0f);
    } else {
        std::cout << "[Error] No se pudo cargar assets/fonts/Minecraft.ttf" << std::endl;
    }

    std::cout << "¡Spawn aleatorio fijado en X: " << (posX/32) << " Y: " << (posY/32) << "!" << std::endl;
}

Juego::~Juego() {
    for (auto* animal : animales) {
        delete animal;
    }
    animales.clear();
}

void Juego::ejecutar() {
    sf::Clock reloj; 

    // === SISTEMA DE INVENTARIO Y HERRAMIENTAS MODULAR ===
    SistemaHerramientas herramientas;
    InventarioGrid inventarioGrid; // <-- Instanciado correctamente

    bool estaMinando = false;
    sf::Vector2i bloqueSiendoMinado = {-1, -1};
    
    while (ventana.isOpen() && estaCorriendo) {
        float dt = reloj.restart().asSeconds(); // <-- 'dt' inicializado al principio del bucle

        // --- MANEJO DE EVENTOS ---
        while (const std::optional<sf::Event> evento = ventana.pollEvent()) {
            if (evento->is<sf::Event::Closed>()) {
                ventana.close();
            }

            // Cambiar de herramienta usando el nuevo sistema modular
            if (const auto* botonTeclado = evento->getIf<sf::Event::KeyPressed>()) {
                if (botonTeclado->code == sf::Keyboard::Key::Num1) herramientas.cambiarHerramienta(0); // Mano
                if (botonTeclado->code == sf::Keyboard::Key::Num2) herramientas.cambiarHerramienta(1); // Pico
                if (botonTeclado->code == sf::Keyboard::Key::Num3) herramientas.cambiarHerramienta(2); // Pala
                if (botonTeclado->code == sf::Keyboard::Key::Num4) herramientas.cambiarHerramienta(3); // Hacha
                if (botonTeclado->code == sf::Keyboard::Key::Num5) herramientas.cambiarHerramienta(4); // Slot vacio

                // Alternar el menú con la tecla Q
                if (botonTeclado->code == sf::Keyboard::Key::Q) {
                    inventarioGrid.alternarMenu();
                }
            }
        }

        // Actualizar arrastre de ítems si el menú interactivo está abierto
        if (inventarioGrid.esMenuAbierto()) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(ventana);
            bool clickPresionado = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
            inventarioGrid.manejarClicks(mousePos, clickPresionado);
        }

        // --- ACTUALIZACIÓN DE LÓGICA (MOVIMIENTO Y FÍSICAS) ---
        if (jugador) {
            jugador->controlar(dt, *mapaSuperficie); // Usando 'controlar' con 'dt' visible
            camara.setCenter(jugador->getPosicion());
        }

        for (auto* animal : animales) {
            if (animal) animal->actualizar(dt, *mapaSuperficie);
        }

        // --- DETECCIÓN CONTINUA DE MINADO ---
        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            sf::Vector2i posicionMousePantalla = sf::Mouse::getPosition(ventana);
            
            ventana.setView(camara); 
            sf::Vector2f posicionMundo = ventana.mapPixelToCoords(posicionMousePantalla);

            int bloqueX = static_cast<int>(std::floor(posicionMundo.x / 32.0f));
            int bloqueY = static_cast<int>(std::floor(posicionMundo.y / 32.0f));

            if (jugador && mapaSuperficie) {
                sf::Vector2f posJugador = jugador->getPosicion();
                sf::Vector2f centroJugador = posJugador + sf::Vector2f(12.0f, 12.0f);
                sf::Vector2f centroBloque((bloqueX * 32.0f) + 16.0f, (bloqueY * 32.0f) + 16.0f);
                
                float dx = centroBloque.x - centroJugador.x;
                float dy = centroBloque.y - centroJugador.y;
                float distancia = std::sqrt(dx * dx + dy * dy);

                if (distancia <= 115.0f) {
                    TipoBloque tipoActual = mapaSuperficie->getTipoBloque(bloqueX, bloqueY);
                    
                    if (tipoActual != TipoBloque::Aire && tipoActual != TipoBloque::Agua) {
                        float danioAplicado = herramientas.calcularDanio(tipoActual);
                        bool destruido = mapaSuperficie->daniarBloque(bloqueX, bloqueY, danioAplicado);
                        
                        if (destruido) {
                            herramientas.agregarAlInventario(tipoActual, herramientas.getHerramientaActiva());
                            // Almacenamos el bloque recolectado en las celdas estilo Minecraft
                            inventarioGrid.agregarItem(tipoActual, 1);
                        }
                    }
                }
            }
        }

        // --- RENDERIZADO DEL MUNDO ---
        ventana.clear(sf::Color(135, 206, 235)); // Cielo Azul

        ventana.setView(camara);
        if (mapaSuperficie) mapaSuperficie->dibujar(ventana);

        for (auto* animal : animales) {
            if (animal) animal->dibujar(ventana);
        }

        if (jugador) jugador->dibujar(ventana);

        // Capa de Interfaz de Usuario (UI Fija)
        ventana.setView(ventana.getDefaultView());

        if (fuenteCargada && textoCoordenadas && jugador) {
            std::stringstream ss;
            sf::Vector2f pos = jugador->getPosicion();
            
            std::string nombreItem = "Mano";
            int activa = herramientas.getHerramientaActiva();
            if (activa == 1) nombreItem = "Pico";
            if (activa == 2) nombreItem = "Pala";
            if (activa == 3) nombreItem = "Hacha";
            if (activa == 4) nombreItem = "Item Especial";

            ss << "FPS: " << static_cast<int>(1.0f / dt) << "\n"
               << "Bloque Coords -> X: " << static_cast<int>(pos.x / 32.0f) 
               << " Y: " << static_cast<int>(pos.y / 32.0f) << "\n"
               << "Item en Mano: " << nombreItem;
            
            textoCoordenadas->setString(ss.str());
            textoCoordenadas->setPosition({10.0f, 10.0f});
            ventana.draw(*textoCoordenadas);

            std::stringstream ssInv;
            ssInv << "PASTO: " << herramientas.getCantidad(TipoBloque::Pasto)
                  << " | MADERA: " << herramientas.getCantidad(TipoBloque::Madera)
                  << " | PIEDRA: " << herramientas.getCantidad(TipoBloque::Piedra)
                  << " | HIERRO: " << herramientas.getCantidad(TipoBloque::MineralHierro)
                  << " | DIAMANTE: " << herramientas.getCantidad(TipoBloque::MineralDiamante);

            sf::Text textoInventario(fuente, ssInv.str(), 16);
            textoInventario.setFillColor(sf::Color::Yellow);
            textoInventario.setOutlineColor(sf::Color::Black);
            textoInventario.setOutlineThickness(2.0f);
            textoInventario.setPosition({10.0f, 570.0f});
            ventana.draw(textoInventario);
        }

        // Dibujar ranuras de la Hotbar de herramientas
        for (int i = 0; i < 5; ++i) {
            sf::RectangleShape cuadroHotbar({40.0f, 40.0f});
            cuadroHotbar.setPosition({250.0f + (i * 50.0f), 10.0f});
            cuadroHotbar.setFillColor(sf::Color(100, 100, 100, 200));
            cuadroHotbar.setOutlineThickness(2.0f);

            if (i == herramientas.getHerramientaActiva()) {
                cuadroHotbar.setOutlineColor(sf::Color::Green);
            } else {
                cuadroHotbar.setOutlineColor(sf::Color::White);
            }
            ventana.draw(cuadroHotbar);
        }

        // Dibujar el sistema de rejilla por slots (Mochila completa + Hotbar inferior)
        if (fuenteCargada) {
            inventarioGrid.dibujar(ventana, fuente);
        }

        ventana.display();
    }
}