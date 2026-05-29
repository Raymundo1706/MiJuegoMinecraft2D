#include <iostream>
#include <random>
#include <sstream>
#include <cmath>
#include "SistemaHerramientas.hpp"
#include "InventarioGrid.hpp"
#include "SistemaCrafteo.hpp"

inline Juego::Juego()
    : ventana(sf::VideoMode({800, 600}), "TEST DE CAMBIOS REALES"),
      estaCorriendo(true),
      fuenteCargada(false) {
    mapaSuperficie = std::make_unique<Mundo>(1000, 1000);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> spawnX(100 * 32, 800 * 32);
    std::uniform_int_distribution<> spawnY(100 * 32, 800 * 32);

    float posX = static_cast<float>(spawnX(gen));
    float posY = static_cast<float>(spawnY(gen));

    jugador = std::make_unique<Jugador>(posX, posY);
    camara.setSize({800.0f, 600.0f});

    std::uniform_real_distribution<float> disX(1600.0f, 30400.0f);
    std::uniform_real_distribution<float> disY(1600.0f, 30400.0f);

    for (int i = 0; i < 150; ++i) {
        float animalX = disX(gen);
        float animalY = disY(gen);
        TipoAnimal tipo = (i % 2 == 0) ? TipoAnimal::Cerdo : TipoAnimal::Oveja;
        animales.push_back(new Animal(animalX, animalY, tipo));
    }
    std::cout << "Fauna esparcida con exito por los 32,000 pixeles del mapa." << std::endl;

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

    std::cout << "Spawn aleatorio fijado en X: " << (posX / 32)
              << " Y: " << (posY / 32) << "." << std::endl;
}

inline Juego::~Juego() {
    for (auto* animal : animales) {
        delete animal;
    }
    animales.clear();
}

inline void Juego::ejecutar() {
    sf::Clock reloj;
    SistemaHerramientas herramientas;
    InventarioGrid inventarioGrid;
    SistemaCrafteo sistemaCrafteo;
    bool clickIzquierdoAnterior = false;
    bool clickDerechoAnterior = false;

    while (ventana.isOpen() && estaCorriendo) {
        float dt = reloj.restart().asSeconds();

        while (const std::optional<sf::Event> evento = ventana.pollEvent()) {
            if (evento->is<sf::Event::Closed>()) {
                ventana.close();
            }

            if (const auto* botonTeclado = evento->getIf<sf::Event::KeyPressed>()) {
                if (botonTeclado->code == sf::Keyboard::Key::Num1) {
                    inventarioGrid.seleccionarSlotHotbar(0);
                }
                if (botonTeclado->code == sf::Keyboard::Key::Num2) {
                    inventarioGrid.seleccionarSlotHotbar(1);
                }
                if (botonTeclado->code == sf::Keyboard::Key::Num3) {
                    inventarioGrid.seleccionarSlotHotbar(2);
                }
                if (botonTeclado->code == sf::Keyboard::Key::Num4) {
                    inventarioGrid.seleccionarSlotHotbar(3);
                }
                if (botonTeclado->code == sf::Keyboard::Key::Num5) inventarioGrid.seleccionarSlotHotbar(4);
                if (botonTeclado->code == sf::Keyboard::Key::Num6) inventarioGrid.seleccionarSlotHotbar(5);
                if (botonTeclado->code == sf::Keyboard::Key::Num7) inventarioGrid.seleccionarSlotHotbar(6);
                if (botonTeclado->code == sf::Keyboard::Key::Num8) inventarioGrid.seleccionarSlotHotbar(7);
                if (botonTeclado->code == sf::Keyboard::Key::Num9) inventarioGrid.seleccionarSlotHotbar(8);

                if (botonTeclado->code == sf::Keyboard::Key::Q) {
                    if (sistemaCrafteo.esMenuAbierto()) {
                        sistemaCrafteo.alternarMenu();
                    } else {
                        inventarioGrid.alternarMenu();
                    }
                }
            }
        }

        sf::Vector2i mousePos = sf::Mouse::getPosition(ventana);
        bool clickIzquierdo = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
        bool clickDerecho = sf::Mouse::isButtonPressed(sf::Mouse::Button::Right);
        inventarioGrid.manejarClicks(mousePos, clickIzquierdo, clickDerecho);
        if (sistemaCrafteo.esMenuAbierto()) {
            sistemaCrafteo.manejarClicks(mousePos, clickIzquierdo, inventarioGrid.getItemEnHotbar());
        }

        bool uiAbierta = inventarioGrid.esMenuAbierto() || sistemaCrafteo.esMenuAbierto();

        if (jugador && !uiAbierta) {
            jugador->controlar(dt, *mapaSuperficie);
            camara.setCenter(jugador->getPosicion());
        }

        for (auto* animal : animales) {
            if (animal) animal->actualizar(dt, *mapaSuperficie);
        }

        ventana.setView(camara);
        sf::Vector2f posicionMundoMouse = ventana.mapPixelToCoords(mousePos);
        int bloqueMouseX = static_cast<int>(std::floor(posicionMundoMouse.x / 32.0f));
        int bloqueMouseY = static_cast<int>(std::floor(posicionMundoMouse.y / 32.0f));

        bool mesaHoverCercana = false;
        bool clickSobreMesa = false;
        if (jugador && mapaSuperficie) {
            TipoBloque tipoHover = mapaSuperficie->getTipoBloque(bloqueMouseX, bloqueMouseY);
            if (tipoHover == TipoBloque::MesaCrafteo) {
                sf::Vector2f posJugador = jugador->getPosicion();
                sf::Vector2f centroJugador = posJugador + sf::Vector2f(12.0f, 12.0f);
                sf::Vector2f centroBloque((bloqueMouseX * 32.0f) + 16.0f, (bloqueMouseY * 32.0f) + 16.0f);
                float dx = centroBloque.x - centroJugador.x;
                float dy = centroBloque.y - centroJugador.y;
                mesaHoverCercana = std::sqrt(dx * dx + dy * dy) <= 64.0f;
                clickSobreMesa = mesaHoverCercana && clickIzquierdo && !clickIzquierdoAnterior;
            }
        }

        if (!uiAbierta && clickDerecho && !clickDerechoAnterior) {
            ItemId itemEnMano = inventarioGrid.getItemEnHotbar();
            TipoBloque bloqueAColocar = bloqueDesdeItem(itemEnMano);

            if (esItemColocable(itemEnMano) && bloqueAColocar != TipoBloque::Aire &&
                jugador && mapaSuperficie) {
                sf::Vector2f posJugador = jugador->getPosicion();
                sf::Vector2f centroJugador = posJugador + sf::Vector2f(12.0f, 12.0f);
                sf::Vector2f centroBloque((bloqueMouseX * 32.0f) + 16.0f, (bloqueMouseY * 32.0f) + 16.0f);
                float dx = centroBloque.x - centroJugador.x;
                float dy = centroBloque.y - centroJugador.y;
                float distancia = std::sqrt(dx * dx + dy * dy);

                if (distancia <= 115.0f && mapaSuperficie->colocarBloque(bloqueMouseX, bloqueMouseY, bloqueAColocar)) {
                    inventarioGrid.consumirItemHotbar(1);
                }
            }
        }

        if (clickSobreMesa) {
            sistemaCrafteo.alternarMenu();
        }

        if (!uiAbierta && clickIzquierdo && !clickSobreMesa) {
            ventana.setView(camara);
            int bloqueX = bloqueMouseX;
            int bloqueY = bloqueMouseY;

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
                        ItemId itemEnMano = inventarioGrid.getItemEnHotbar();
                        float danioAplicado = herramientas.calcularDanio(tipoActual, itemEnMano);
                        bool destruido = mapaSuperficie->daniarBloque(bloqueX, bloqueY, danioAplicado);

                        if (destruido) {
                            if (herramientas.puedeRecolectar(tipoActual, itemEnMano)) {
                                inventarioGrid.agregarItem(itemDesdeBloque(tipoActual), 1);
                            }
                        }
                    }
                }
            }
        }

        ventana.clear(sf::Color(135, 206, 235));

        ventana.setView(camara);
        if (mapaSuperficie) mapaSuperficie->dibujar(ventana);

        for (auto* animal : animales) {
            if (animal) animal->dibujar(ventana);
        }

        if (jugador) jugador->dibujar(ventana);

        ventana.setView(ventana.getDefaultView());

        if (fuenteCargada && textoCoordenadas && jugador) {
            std::stringstream ss;
            sf::Vector2f pos = jugador->getPosicion();

            std::string nombreEnMano = nombreItem(inventarioGrid.getItemEnHotbar());

            ss << "FPS: " << static_cast<int>(1.0f / dt) << "\n"
               << "Bloque Coords -> X: " << static_cast<int>(pos.x / 32.0f)
               << " Y: " << static_cast<int>(pos.y / 32.0f) << "\n"
               << "Item en Mano: " << nombreEnMano;

            textoCoordenadas->setString(ss.str());
            textoCoordenadas->setPosition({10.0f, 10.0f});
            ventana.draw(*textoCoordenadas);
        }

        if (fuenteCargada) {
            if (mesaHoverCercana && !uiAbierta) {
                sf::Text textoMesa(fuente, "Mesa de Crafteo", 14);
                textoMesa.setPosition({static_cast<float>(mousePos.x + 12), static_cast<float>(mousePos.y - 18)});
                textoMesa.setFillColor(sf::Color::White);
                textoMesa.setOutlineColor(sf::Color::Black);
                textoMesa.setOutlineThickness(2.0f);
                ventana.draw(textoMesa);
            }

            inventarioGrid.dibujar(ventana, fuente);
            sistemaCrafteo.dibujar(ventana, fuente);
        }

        ventana.display();
        clickIzquierdoAnterior = clickIzquierdo;
        clickDerechoAnterior = clickDerecho;
    }
}


