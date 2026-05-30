#include <iostream>
#include <random>
#include <sstream>
#include <cmath>
#include "SistemaHerramientas.hpp"
#include "InventarioGrid.hpp"

inline Juego::Juego()
    : ventana(sf::VideoMode({800, 600}), "TEST DE CAMBIOS REALES"),
      estaCorriendo(true),
      fuenteCargada(false) {
    mapaSuperficie = std::make_unique<Mundo>(1000, 1000);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> spawnBloqueX(30, mapaSuperficie->getAncho() - 31);
    std::uniform_int_distribution<> spawnBloqueY(30, mapaSuperficie->getAlto() - 31);

    auto bloqueSeguroParaSpawn = [&](int bx, int by) {
        if (mapaSuperficie->getTipoBloque(bx, by) != TipoBloque::Pasto) {
            return false;
        }

        for (int y = by - 1; y <= by + 1; ++y) {
            for (int x = bx - 1; x <= bx + 1; ++x) {
                if (mapaSuperficie->esBloqueSolido(x, y)) {
                    return false;
                }
            }
        }
        return true;
    };

    int bloqueSpawnX = 50;
    int bloqueSpawnY = 50;
    bool spawnEncontrado = false;

    for (int intento = 0; intento < 8000 && !spawnEncontrado; ++intento) {
        int bx = spawnBloqueX(gen);
        int by = spawnBloqueY(gen);
        if (bloqueSeguroParaSpawn(bx, by)) {
            bloqueSpawnX = bx;
            bloqueSpawnY = by;
            spawnEncontrado = true;
        }
    }

    if (!spawnEncontrado) {
        for (int y = 30; y < mapaSuperficie->getAlto() - 30 && !spawnEncontrado; ++y) {
            for (int x = 30; x < mapaSuperficie->getAncho() - 30 && !spawnEncontrado; ++x) {
                if (bloqueSeguroParaSpawn(x, y)) {
                    bloqueSpawnX = x;
                    bloqueSpawnY = y;
                    spawnEncontrado = true;
                }
            }
        }
    }

    float posX = static_cast<float>(bloqueSpawnX * 32 + 4);
    float posY = static_cast<float>(bloqueSpawnY * 32 + 4);

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
    inventarioGrid.agregarItem(ItemId::MapaInicial, 1);
    bool clickIzquierdoAnterior = false;
    bool clickDerechoAnterior = false;
    bool mostrarDebug = false;
    float acumuladorFPS = 0.0f;
    int contadorFrames = 0;
    int fpsActuales = 0;
    bool mapaInicialGenerado = false;
    int mapaCentroX = 0;
    int mapaCentroY = 0;
    sf::Texture texturaMapaInicial;
    constexpr int TAMANIO_MAPA_INICIAL = 700;
    constexpr int RADIO_MAPA_INICIAL = TAMANIO_MAPA_INICIAL / 2;

    auto generarMapaInicial = [&]() {
        if (!jugador || !mapaSuperficie) return;

        sf::Vector2f centroJugador = jugador->getPosicion() + sf::Vector2f(12.0f, 12.0f);
        mapaCentroX = static_cast<int>(std::floor(centroJugador.x / 32.0f));
        mapaCentroY = static_cast<int>(std::floor(centroJugador.y / 32.0f));

        sf::Image imagenMapa({TAMANIO_MAPA_INICIAL, TAMANIO_MAPA_INICIAL}, sf::Color(20, 28, 42));
        int inicioX = mapaCentroX - RADIO_MAPA_INICIAL;
        int inicioY = mapaCentroY - RADIO_MAPA_INICIAL;

        for (int y = 0; y < TAMANIO_MAPA_INICIAL; ++y) {
            for (int x = 0; x < TAMANIO_MAPA_INICIAL; ++x) {
                imagenMapa.setPixel(
                    sf::Vector2u(static_cast<unsigned int>(x), static_cast<unsigned int>(y)),
                    mapaSuperficie->getColorMapa(inicioX + x, inicioY + y)
                );
            }
        }

        mapaInicialGenerado = texturaMapaInicial.loadFromImage(imagenMapa);
        texturaMapaInicial.setSmooth(false);
    };

    while (ventana.isOpen() && estaCorriendo) {
        float dt = reloj.restart().asSeconds();
        if (dt > 0.05f) {
            dt = 0.05f;
        }
        acumuladorFPS += dt;
        contadorFrames++;
        if (acumuladorFPS >= 1.0f) {
            fpsActuales = contadorFrames;
            contadorFrames = 0;
            acumuladorFPS = 0.0f;
        }

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
                    if (inventarioGrid.esMesaCrafteoAbierta()) {
                        inventarioGrid.cerrarMesaCrafteo();
                    } else {
                        inventarioGrid.alternarMenu();
                    }
                }

                if (botonTeclado->code == sf::Keyboard::Key::F3) {
                    mostrarDebug = !mostrarDebug;
                }
            }
        }

        sf::Vector2i mousePos = sf::Mouse::getPosition(ventana);
        bool clickIzquierdo = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
        bool clickDerecho = sf::Mouse::isButtonPressed(sf::Mouse::Button::Right);
        inventarioGrid.manejarClicks(mousePos, clickIzquierdo, clickDerecho);

        bool uiAbierta = inventarioGrid.esMenuAbierto() || inventarioGrid.esMesaCrafteoAbierta();

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

        int bloqueJugadorX = -1;
        int bloqueJugadorY = -1;
        bool jugadorSobreEntradaMina = false;
        bool minaAbierta = false;
        float minaRestante = 0.0f;
        float minaProgreso = 0.0f;
        bool mostrandoBarraArbol = false;
        float progresoArbol = 0.0f;
        bool mapaEnSegundaMano = inventarioGrid.getItemSegundaMano() == ItemId::MapaInicial;

        if (mapaEnSegundaMano && !mapaInicialGenerado) {
            generarMapaInicial();
        }

        if (jugador && mapaSuperficie) {
            sf::Vector2f centroJugador = jugador->getPosicion() + sf::Vector2f(12.0f, 12.0f);
            bloqueJugadorX = static_cast<int>(std::floor(centroJugador.x / 32.0f));
            bloqueJugadorY = static_cast<int>(std::floor(centroJugador.y / 32.0f));
            jugadorSobreEntradaMina = mapaSuperficie->getTipoBloque(bloqueJugadorX, bloqueJugadorY) == TipoBloque::CuevaEntrada;

            if (jugadorSobreEntradaMina) {
                ItemId itemEnMano = inventarioGrid.getItemEnHotbar();
                float velocidadPicadoMina = 0.0f;
                if (itemEnMano == ItemId::PicoMadera) velocidadPicadoMina = 1.0f;
                if (itemEnMano == ItemId::PicoPiedra) velocidadPicadoMina = 1.5f;
                if (itemEnMano == ItemId::PicoDiamante) velocidadPicadoMina = 3.0f;

                if (!uiAbierta && clickIzquierdo && velocidadPicadoMina > 0.0f) {
                    mapaSuperficie->picarEntradaMina(bloqueJugadorX, bloqueJugadorY, dt * velocidadPicadoMina);
                }

                minaAbierta = mapaSuperficie->esMinaAbierta(bloqueJugadorX, bloqueJugadorY);
                minaRestante = mapaSuperficie->getTiempoMinaRestante(bloqueJugadorX, bloqueJugadorY);
                minaProgreso = mapaSuperficie->getProgresoMina(bloqueJugadorX, bloqueJugadorY);
            }
        }

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

            if (jugador && mapaSuperficie) {
                sf::Vector2f posJugador = jugador->getPosicion();
                sf::Vector2f centroJugador = posJugador + sf::Vector2f(12.0f, 12.0f);
                sf::Vector2f centroBloque((bloqueMouseX * 32.0f) + 16.0f, (bloqueMouseY * 32.0f) + 16.0f);
                float dx = centroBloque.x - centroJugador.x;
                float dy = centroBloque.y - centroJugador.y;
                float distancia = std::sqrt(dx * dx + dy * dy);

                if (distancia <= 115.0f &&
                           esItemColocable(itemEnMano) &&
                           bloqueAColocar != TipoBloque::Aire &&
                           mapaSuperficie->colocarBloque(bloqueMouseX, bloqueMouseY, bloqueAColocar)) {
                    inventarioGrid.consumirItemHotbar(1);
                }
            }
        }

        if (clickSobreMesa) {
            inventarioGrid.abrirMesaCrafteo();
        }

        bool usandoMina = jugadorSobreEntradaMina && tipoHerramienta(inventarioGrid.getItemEnHotbar()) == TipoHerramienta::Pico;
        if (!uiAbierta && clickIzquierdo && !clickSobreMesa && !usandoMina) {
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
                        if (tipoActual == TipoBloque::Madera) {
                            float velocidadTala = 0.35f;
                            if (itemEnMano == ItemId::HachaMadera) velocidadTala = 1.0f;
                            if (itemEnMano == ItemId::HachaPiedra) velocidadTala = 1.35f;

                            int troncosObtenidos = 0;
                            bool soltoSemilla = false;
                            bool arbolCayo = mapaSuperficie->talarArbol(
                                bloqueX,
                                bloqueY,
                                dt * velocidadTala,
                                troncosObtenidos,
                                soltoSemilla
                            );

                            if (arbolCayo) {
                                inventarioGrid.agregarItem(ItemId::BloqueTronco, troncosObtenidos);
                                if (soltoSemilla) {
                                    inventarioGrid.agregarItem(ItemId::SemillaArbol, 1);
                                }
                            }
                            tipoActual = TipoBloque::Aire;
                        }

                        if (tipoActual == TipoBloque::Piedra &&
                            tipoHerramienta(itemEnMano) == TipoHerramienta::Pico &&
                            mapaSuperficie->crearEntradaMina(bloqueX, bloqueY)) {
                            tipoActual = TipoBloque::Aire;
                        }

                        if (tipoActual != TipoBloque::Aire) {
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
        }

        if (!uiAbierta && jugador && mapaSuperficie) {
            TipoBloque tipoHover = mapaSuperficie->getTipoBloque(bloqueMouseX, bloqueMouseY);
            if (tipoHover == TipoBloque::Madera) {
                sf::Vector2f posJugador = jugador->getPosicion();
                sf::Vector2f centroJugador = posJugador + sf::Vector2f(12.0f, 12.0f);
                sf::Vector2f centroBloque((bloqueMouseX * 32.0f) + 16.0f, (bloqueMouseY * 32.0f) + 16.0f);
                float dx = centroBloque.x - centroJugador.x;
                float dy = centroBloque.y - centroJugador.y;
                if (std::sqrt(dx * dx + dy * dy) <= 115.0f) {
                    mostrandoBarraArbol = true;
                    progresoArbol = mapaSuperficie->getProgresoTala(bloqueMouseX, bloqueMouseY);
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

        if (fuenteCargada && textoCoordenadas && jugador && mostrarDebug) {
            std::stringstream ss;
            sf::Vector2f pos = jugador->getPosicion();

            std::string nombreEnMano = nombreItem(inventarioGrid.getItemEnHotbar());

            ss << "FPS: " << fpsActuales << "\n"
               << "Bloque Coords -> X: " << static_cast<int>(pos.x / 32.0f)
               << " Y: " << static_cast<int>(pos.y / 32.0f) << "\n"
               << "Item en Mano: " << nombreEnMano;

            if (jugadorSobreEntradaMina) {
                int segundosRestantes = static_cast<int>(std::ceil(minaRestante));
                int minutos = segundosRestantes / 60;
                int segundos = segundosRestantes % 60;
                ss << "\nMina: " << (minaAbierta ? "Abierta" : "Cerrada")
                   << "\nRestante: " << minutos << "m " << segundos << "s";
            }

            textoCoordenadas->setString(ss.str());
            textoCoordenadas->setPosition({10.0f, 10.0f});
            ventana.draw(*textoCoordenadas);
        }

        if (fuenteCargada) {
            if (mapaEnSegundaMano && mapaInicialGenerado && jugador) {
                const float tamMapa = 180.0f;
                const float escalaMapa = tamMapa / static_cast<float>(TAMANIO_MAPA_INICIAL);
                const sf::Vector2f posMapa(800.0f - tamMapa - 18.0f, 600.0f - tamMapa - 18.0f);

                sf::RectangleShape marcoMapa({tamMapa + 8.0f, tamMapa + 8.0f});
                marcoMapa.setPosition({posMapa.x - 4.0f, posMapa.y - 4.0f});
                marcoMapa.setFillColor(sf::Color(86, 66, 38, 230));
                marcoMapa.setOutlineColor(sf::Color(30, 22, 14));
                marcoMapa.setOutlineThickness(2.0f);
                ventana.draw(marcoMapa);

                sf::Sprite spriteMapa(texturaMapaInicial);
                spriteMapa.setPosition(posMapa);
                spriteMapa.setScale({escalaMapa, escalaMapa});
                ventana.draw(spriteMapa);

                sf::Vector2f centroJugador = jugador->getPosicion() + sf::Vector2f(12.0f, 12.0f);
                int jugadorMapaX = static_cast<int>(std::floor(centroJugador.x / 32.0f)) - (mapaCentroX - RADIO_MAPA_INICIAL);
                int jugadorMapaY = static_cast<int>(std::floor(centroJugador.y / 32.0f)) - (mapaCentroY - RADIO_MAPA_INICIAL);
                jugadorMapaX = std::clamp(jugadorMapaX, 0, TAMANIO_MAPA_INICIAL - 1);
                jugadorMapaY = std::clamp(jugadorMapaY, 0, TAMANIO_MAPA_INICIAL - 1);

                sf::CircleShape marcador(3.5f);
                marcador.setOrigin({3.5f, 3.5f});
                marcador.setPosition({
                    posMapa.x + static_cast<float>(jugadorMapaX) * escalaMapa,
                    posMapa.y + static_cast<float>(jugadorMapaY) * escalaMapa
                });
                marcador.setFillColor(sf::Color(210, 40, 36));
                marcador.setOutlineColor(sf::Color::White);
                marcador.setOutlineThickness(1.0f);
                ventana.draw(marcador);
            }

            if (mesaHoverCercana && !uiAbierta) {
                sf::Text textoMesa(fuente, "Mesa de Crafteo", 14);
                textoMesa.setPosition({static_cast<float>(mousePos.x + 12), static_cast<float>(mousePos.y - 18)});
                textoMesa.setFillColor(sf::Color::White);
                textoMesa.setOutlineColor(sf::Color::Black);
                textoMesa.setOutlineThickness(2.0f);
                ventana.draw(textoMesa);
            }

            if (jugadorSobreEntradaMina && !uiAbierta) {
                sf::RectangleShape fondoBarra({220.0f, 18.0f});
                fondoBarra.setPosition({290.0f, 78.0f});
                fondoBarra.setFillColor(sf::Color(20, 20, 20, 220));
                fondoBarra.setOutlineColor(sf::Color::White);
                fondoBarra.setOutlineThickness(1.0f);
                ventana.draw(fondoBarra);

                sf::RectangleShape progreso({216.0f * minaProgreso, 14.0f});
                progreso.setPosition({292.0f, 80.0f});
                progreso.setFillColor(minaAbierta ? sf::Color(80, 200, 120) : sf::Color(180, 140, 70));
                ventana.draw(progreso);

                sf::Text textoMina(fuente, minaAbierta ? "Mina abierta" : "Picando entrada de mina", 12);
                textoMina.setPosition({294.0f, 58.0f});
                textoMina.setFillColor(sf::Color::White);
                textoMina.setOutlineColor(sf::Color::Black);
                textoMina.setOutlineThickness(2.0f);
                ventana.draw(textoMina);
            }

            if (mostrandoBarraArbol && !uiAbierta) {
                sf::RectangleShape fondoBarra({220.0f, 16.0f});
                fondoBarra.setPosition({290.0f, 104.0f});
                fondoBarra.setFillColor(sf::Color(28, 20, 12, 220));
                fondoBarra.setOutlineColor(sf::Color(235, 220, 180));
                fondoBarra.setOutlineThickness(1.0f);
                ventana.draw(fondoBarra);

                sf::RectangleShape progreso({216.0f * progresoArbol, 12.0f});
                progreso.setPosition({292.0f, 106.0f});
                progreso.setFillColor(sf::Color(92, 154, 72));
                ventana.draw(progreso);

                sf::Text textoArbol(fuente, "Talando arbol", 12);
                textoArbol.setPosition({294.0f, 84.0f});
                textoArbol.setFillColor(sf::Color::White);
                textoArbol.setOutlineColor(sf::Color::Black);
                textoArbol.setOutlineThickness(2.0f);
                ventana.draw(textoArbol);
            }

            inventarioGrid.dibujar(ventana, fuente);
        }

        ventana.display();
        clickIzquierdoAnterior = clickIzquierdo;
        clickDerechoAnterior = clickDerecho;
    }
}


