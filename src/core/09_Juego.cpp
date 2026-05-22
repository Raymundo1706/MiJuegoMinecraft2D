#include "Juego.hpp"
#include <iostream>
#include <random>
#include <sstream>
#include "SistemaHerramientas.hpp"

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

    // SPAWN DE ANIMALES ANCLADOS
    std::uniform_real_distribution<float> disX(posX - 400.0f, posX + 400.0f);
    std::uniform_real_distribution<float> disY(posY - 400.0f, posY + 400.0f);

    for (int i = 0; i < 150; ++i) {
        float animalX = disX(gen);
        float animalY = disY(gen);
        
        if (animalX < 32.0f) animalX = 100.0f;
        if (animalY < 32.0f) animalY = 100.0f;

        TipoAnimal tipo = (i % 2 == 0) ? TipoAnimal::Cerdo : TipoAnimal::Oveja;
        animales.push_back(new Animal(animalX, animalY, tipo));
    }

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

Juego::~Juego() {}

void Juego::ejecutar() {
    sf::Clock reloj; 

    // === SISTEMA DE INVENTARIO Y HERRAMIENTAS MODULAR ===
    SistemaHerramientas herramientas;

    // Conservamos estas dos por si las usas para animaciones de minado futuras
    bool estaMinando = false;
    sf::Vector2i bloqueSiendoMinado = {-1, -1};
    
    // ... aquí continúa el resto de tu código (el bucle while(ventana.isOpen()), etc.)
    while (ventana.isOpen() && estaCorriendo) {
        // --- MANEJO DE EVENTOS ---
        while (const std::optional<sf::Event> evento = ventana.pollEvent()) {
            if (evento->is<sf::Event::Closed>()) {
                ventana.close();
            }

            // Detectar cambio de herramientas con las teclas 1, 2, 3, 4, 5
            if (const auto* botonTeclado = evento->getIf<sf::Event::KeyPressed>()) {
                if (botonTeclado->code == sf::Keyboard::Key::Num1) herramientaEquipada = 0;
                if (botonTeclado->code == sf::Keyboard::Key::Num2) herramientaEquipada = 1;
                if (botonTeclado->code == sf::Keyboard::Key::Num3) herramientaEquipada = 2;
                if (botonTeclado->code == sf::Keyboard::Key::Num4) herramientaEquipada = 3;
                if (botonTeclado->code == sf::Keyboard::Key::Num5) herramientaEquipada = 4;
            }
        }

        // --- ACTUALIZACIÓN DE TIEMPO (DeltaTime) ---
        float dt = reloj.restart().asSeconds();

        // --- DETECCIÓN CONTINUA DE MINADO (TIEMPOS CALIBRADOS EN SEGUNDOS) ---
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

                // Rango estricto de 3 bloques a la redonda
                if (distancia <= 115.0f) {
                    TipoBloque tipoActual = mapaSuperficie->getTipoBloque(bloqueX, bloqueY);
                    
                    if (tipoActual != TipoBloque::Aire && tipoActual != TipoBloque::Agua) {
                        
                        // DAÑO POR DEFECTO: Mano desnuda (herramientaEquipada == 0 o slots vacíos)
                        float danioAplicado = 0.5f; 
                        
                        // Ajustamos el daño de la mano según el bloque
                        if (tipoActual == TipoBloque::Pasto || tipoActual == TipoBloque::Tierra) {
                            danioAplicado = 0.16f; // Tarda ~3 segundos con la mano
                        }
                        else if (tipoActual == TipoBloque::Madera) {
                            danioAplicado = 0.5f;  // Tarda ~3 segundos con la mano
                        }
                        else if (tipoActual == TipoBloque::Piedra || 
                                 tipoActual == TipoBloque::MineralHierro || 
                                 tipoActual == TipoBloque::MineralDiamante) {
                            danioAplicado = 0.33f; // Tarda ~15 segundos con la mano o herramienta incorrecta
                        }

                        // CALIBRACIÓN ESPECÍFICA POR HERRAMIENTA
                        if (herramientaEquipada == 1) { // === PICO ===
                            if (tipoActual == TipoBloque::Piedra || 
                                tipoActual == TipoBloque::MineralHierro || 
                                tipoActual == TipoBloque::MineralDiamante) {
                                danioAplicado = 2.5f; // Lo pica rápido (un par de segundos)
                            } else if (tipoActual == TipoBloque::Madera) {
                                danioAplicado = 0.38f; // Tarda 4 segundos en romper madera con pico
                            }
                        } 
                        else if (herramientaEquipada == 2) { // === PALA ===
                            if (tipoActual == TipoBloque::Pasto || tipoActual == TipoBloque::Tierra) {
                                danioAplicado = 0.25f; // Rompe la tierra en exactamente 2 segundos
                            }
                        }
                        else if (herramientaEquipada == 3) { // === HACHA ===
                            if (tipoActual == TipoBloque::Madera) {
                                danioAplicado = 0.75f; // Tala el árbol en exactamente 2 segundos
                            }
                        }

                        TipoBloque bloqueCosechado = tipoActual;

                        // Aplicamos el golpe al bloque
                        bool destruido = mapaSuperficie->daniarBloque(bloqueX, bloqueY, danioAplicado);
                        
                        if (destruido) {
                            // VALIDACIÓN DE HERRAMIENTA REQUERIDA (Piedra y minerales solo dan ítem con el Pico)
                            bool esMineral = (bloqueCosechado == TipoBloque::Piedra || 
                                              bloqueCosechado == TipoBloque::MineralHierro || 
                                              bloqueCosechado == TipoBloque::MineralDiamante);
                            
                            if (!esMineral || (esMineral && herramientaEquipada == 1)) {
                                if (bloqueCosechado == TipoBloque::Pasto)           inventarioPasto++;
                                if (bloqueCosechado == TipoBloque::Tierra)          inventarioPasto++;
                                if (bloqueCosechado == TipoBloque::Madera)          inventarioMadera++;
                                if (bloqueCosechado == TipoBloque::Piedra)          inventarioPiedra++;
                                if (bloqueCosechado == TipoBloque::MineralHierro)   inventarioHierro++;
                                if (bloqueCosechado == TipoBloque::MineralDiamante) inventarioDiamante++;
                            }
                        }
                    }
                }
            }
        }
        // --- ACTUALIZACIÓN DE LÓGICA (JUGADOR Y ANIMALES) ---
        if (jugador) {
            jugador->controlar(dt, *mapaSuperficie);
            camara.setCenter(jugador->getPosicion());
            
            int bloqueX = static_cast<int>(jugador->getPosicion().x / 32.0f);
            int bloqueY = static_cast<int>(jugador->getPosicion().y / 32.0f);

            if (fuenteCargada && textoCoordenadas) {
                std::stringstream ss;
                ss << "Bloque X: " << bloqueX << "\nBloque Y: " << bloqueY;
                
                std::string nombreItem = "Mano";
                if (herramientaEquipada == 1) nombreItem = "Pico";
                if (herramientaEquipada == 2) nombreItem = "Pala";
                if (herramientaEquipada == 3) nombreItem = "Hacha";
                if (herramientaEquipada == 4) nombreItem = "Item Especial";
                
                ss << "\nItem: " << nombreItem;
                textoCoordenadas->setString(ss.str());
            }
        }

        for (auto* animal : animales) {
            if (animal && mapaSuperficie) {
                animal->actualizar(dt, *mapaSuperficie);
            }
        }

        // --- RENDERIZADO GENERAL ---
        ventana.clear(sf::Color::Black);
        
        // 1. DIBUJAR CAPA DEL MUNDO
        ventana.setView(camara);
        if (mapaSuperficie) { 
            mapaSuperficie->dibujar(ventana); 
        }
        
        for (auto* animal : animales) {
            if (animal) { animal->dibujar(ventana); }
        }
        
        if (jugador) { jugador->dibujar(ventana); }
        
        // 2. DIBUJAR CAPA DE INTERFAZ (UI Fija)
        ventana.setView(ventana.getDefaultView());
        
        int totalSlots = 5;
        float tamanioSlot = 45.0f;
        float espacioEntreSlots = 5.0f;
        float anchoTotalHotbar = (totalSlots * tamanioSlot) + ((totalSlots - 1) * espacioEntreSlots);
        float inicioX = (800.0f - anchoTotalHotbar) / 2.0f; 
        float posYHotbar = 535.0f;

        for (int i = 0; i < totalSlots; ++i) {
            float slotX = inicioX + i * (tamanioSlot + espacioEntreSlots);

            sf::RectangleShape slot({tamanioSlot, tamanioSlot});
            slot.setPosition({slotX, posYHotbar});
            slot.setFillColor(sf::Color(140, 140, 140, 200)); 
            slot.setOutlineThickness(3.0f);
            
            if (i == herramientaEquipada) {
                slot.setOutlineColor(sf::Color::White); 
                slot.setFillColor(sf::Color(180, 180, 180, 230)); 
            } else {
                slot.setOutlineColor(sf::Color(60, 60, 60)); 
            }
            
            ventana.draw(slot);

            sf::RectangleShape icono({20.0f, 20.0f});
            icono.setPosition({slotX + 12.5f, posYHotbar + 12.5f});
            
            if (i == 0) icono.setFillColor(sf::Color(150, 75, 0));    // Mano
            if (i == 1) icono.setFillColor(sf::Color(128, 128, 128)); // Pico
            if (i == 2) icono.setFillColor(sf::Color(210, 180, 140)); // Pala
            if (i == 3) icono.setFillColor(sf::Color(34, 139, 34));   // Hacha
            if (i == 4) icono.setFillColor(sf::Color(0, 191, 255));   // Especial

            ventana.draw(icono);
        }

        if (fuenteCargada && textoCoordenadas) {
            textoCoordenadas->setPosition({15.0f, 15.0f});
            ventana.draw(*textoCoordenadas);

            sf::Text textoInventario(fuente);
            textoInventario.setCharacterSize(14);
            textoInventario.setFillColor(sf::Color::White);
            textoInventario.setOutlineColor(sf::Color::Black);
            textoInventario.setOutlineThickness(1.5f);
            
            std::stringstream ssInv;
            ssInv << "PASTO: " << inventarioPasto 
                  << " | MADERA: " << inventarioMadera 
                  << " | PIEDRA: " << inventarioPiedra 
                  << " | HIERRO: " << inventarioHierro 
                  << " | DIAMANTE: " << inventarioDiamante;
                  
            textoInventario.setString(ssInv.str());
            textoInventario.setPosition({120.0f, 500.0f}); 
            ventana.draw(textoInventario);
        }
        
        ventana.display();
    }
}