#include <iostream>
#include <random>
#include <sstream>
#include <cmath>
#include <cstdint>
#include "SistemaHerramientas.hpp"
#include "InventarioGrid.hpp"

namespace {
constexpr float TICKS_POR_SEGUNDO_MUNDO = 100.0f;
constexpr float TICKS_DIA_COMPLETO = 24000.0f;
constexpr float TICK_FIN_DIA = 12000.0f;
constexpr float TICK_INICIO_NOCHE = 13000.0f;
constexpr float TICK_FIN_NOCHE = 22000.0f;
constexpr float TICK_PUEDE_DORMIR = 12542.0f;

inline float calcularSkyLightFloat(float worldTime) {
    if (worldTime <= TICK_FIN_DIA) {
        return 15.0f;
    }
    if (worldTime <= TICK_INICIO_NOCHE) {
        float t = (worldTime - TICK_FIN_DIA) / (TICK_INICIO_NOCHE - TICK_FIN_DIA);
        return 15.0f + (4.0f - 15.0f) * t;
    }
    if (worldTime <= TICK_FIN_NOCHE) {
        return 4.0f;
    }

    float t = (worldTime - TICK_FIN_NOCHE) / (TICKS_DIA_COMPLETO - TICK_FIN_NOCHE);
    return 4.0f + (15.0f - 4.0f) * t;
}

inline int calcularSkyLight(float worldTime) {
    return std::clamp(static_cast<int>(std::round(calcularSkyLightFloat(worldTime))), 0, 15);
}

inline const char* nombreMomentoDia(float worldTime) {
    if (worldTime <= TICK_FIN_DIA) return "Dia";
    if (worldTime <= TICK_INICIO_NOCHE) return "Atardecer";
    if (worldTime <= TICK_FIN_NOCHE) return "Noche";
    return "Amanecer";
}

inline void dibujarFiltroDiaNoche(sf::RenderWindow& ventana, const sf::View& camara, float worldTime, int skyLight) {
    float oscuridad = 1.0f - (static_cast<float>(skyLight) / 15.0f);
    if (oscuridad <= 0.01f) {
        return;
    }

    std::uint8_t alphaAzul = static_cast<std::uint8_t>(std::clamp(oscuridad * 170.0f, 0.0f, 170.0f));
    sf::Color colorFiltro(8, 18, 54, alphaAzul);

    if (worldTime > TICK_FIN_DIA && worldTime <= TICK_INICIO_NOCHE) {
        float t = (worldTime - TICK_FIN_DIA) / (TICK_INICIO_NOCHE - TICK_FIN_DIA);
        std::uint8_t alphaCalido = static_cast<std::uint8_t>(std::clamp(45.0f * (1.0f - std::abs(t - 0.45f) * 1.8f), 0.0f, 45.0f));
        sf::RectangleShape atardecer(camara.getSize());
        atardecer.setOrigin(camara.getSize() * 0.5f);
        atardecer.setPosition(camara.getCenter());
        atardecer.setFillColor(sf::Color(255, 112, 42, alphaCalido));
        ventana.draw(atardecer);
    }

    sf::RectangleShape filtro(camara.getSize());
    filtro.setOrigin(camara.getSize() * 0.5f);
    filtro.setPosition(camara.getCenter());
    filtro.setFillColor(colorFiltro);
    ventana.draw(filtro);
}

inline void dibujarPixelHUD(sf::RenderWindow& ventana, sf::Vector2f origen, int x, int y, sf::Color color, float escala) {
    sf::RectangleShape pixel({escala, escala});
    pixel.setPosition({origen.x + static_cast<float>(x) * escala, origen.y + static_cast<float>(y) * escala});
    pixel.setFillColor(color);
    ventana.draw(pixel);
}

inline void dibujarCorazon(sf::RenderWindow& ventana, sf::Vector2f origen, int estadoHP, float escala) {
    static const char* vacio[9] = {
        ".BB...BB.",
        "BggB.BggB",
        "BgggBgggB",
        "BgggggggB",
        ".BgggggB.",
        "..BgggB..",
        "...BgB...",
        "....B....",
        "........."
    };
    static const char* lleno[9] = {
        ".BB...BB.",
        "BrrB.BrrB",
        "BrhRBrhRB",
        "BRRRRRRRB",
        ".BRRRRRB.",
        "..BRRRB..",
        "...BRB...",
        "....B....",
        "........."
    };

    sf::Color borde(18, 10, 12);
    sf::Color gris(96, 88, 104);
    sf::Color grisOscuro(42, 36, 48);
    sf::Color rojo(234, 28, 39);
    sf::Color rojoMedio(205, 22, 36);
    sf::Color rojoOscuro(126, 12, 24);
    sf::Color brillo(255, 222, 226);

    for (int y = 0; y < 9; ++y) {
        for (int x = 0; x < 9; ++x) {
            char base = vacio[y][x];
            if (base == '.') {
                continue;
            }

            bool pintarRelleno = estadoHP == 2 || (estadoHP == 1 && x <= 4);
            char pixel = pintarRelleno ? lleno[y][x] : base;

            sf::Color color = gris;
            if (pixel == 'B') color = borde;
            if (pixel == 'g') color = (x >= 5 || y >= 4) ? grisOscuro : gris;
            if (pixel == 'r') color = rojo;
            if (pixel == 'R') color = (y >= 5 || x >= 6) ? rojoOscuro : rojoMedio;
            if (pixel == 'h') color = brillo;

            if (estadoHP == 1 && x == 5 && base != 'B') {
                color = borde;
            }
            dibujarPixelHUD(ventana, origen, x, y, color, escala);
        }
    }
}

inline void dibujarBarraVida(sf::RenderWindow& ventana, const Jugador& jugador) {
    int vida = std::clamp(jugador.getVidaHP(), 0, jugador.getVidaMaximaHP());
    int corazones = jugador.getVidaMaximaHP() / 2;
    constexpr float escala = 2.2f;
    sf::Vector2f origen(184.0f, 476.0f);

    for (int i = 0; i < corazones; ++i) {
        int hpCorazon = std::clamp(vida - i * 2, 0, 2);
        dibujarCorazon(ventana, {origen.x + static_cast<float>(i) * 24.0f, origen.y}, hpCorazon, escala);
    }
}

inline void dibujarMusloHambre(sf::RenderWindow& ventana, sf::Vector2f origen, int estado, float escala) {
    static const char* silueta[9] = {
        "...BBB...",
        "..BggB...",
        ".BgggBB..",
        ".BggggB..",
        "..BggB...",
        "...BB....",
        "...B.B...",
        "..B...B..",
        "..B...B.."
    };
    static const char* lleno[9] = {
        "...BBB...",
        "..BooB...",
        ".BoooBB..",
        ".BooooB..",
        "..BooB...",
        "...BB....",
        "...BcB...",
        "..B...B..",
        "..B...B.."
    };

    sf::Color borde(28, 14, 8);
    sf::Color gris(82, 70, 64);
    sf::Color naranja(205, 92, 34);
    sf::Color naranjaClaro(238, 133, 54);
    sf::Color hueso(230, 210, 172);

    for (int y = 0; y < 9; ++y) {
        for (int x = 0; x < 9; ++x) {
            char base = silueta[y][x];
            if (base == '.') {
                continue;
            }

            bool pintarRelleno = estado == 2 || (estado == 1 && x <= 4);
            char pixel = pintarRelleno ? lleno[y][x] : base;

            sf::Color color = gris;
            if (pixel == 'B') color = borde;
            if (pixel == 'g') color = gris;
            if (pixel == 'o') color = (y <= 2 || x <= 3) ? naranjaClaro : naranja;
            if (pixel == 'c') color = hueso;

            if (estado == 1 && x == 5 && base != 'B') {
                color = borde;
            }
            dibujarPixelHUD(ventana, origen, x, y, color, escala);
        }
    }
}

inline void dibujarBarraHambre(sf::RenderWindow& ventana, const Jugador& jugador) {
    int hambre = std::clamp(jugador.getHambre(), 0, 20);
    constexpr float escala = 2.2f;
    sf::Vector2f origen(430.0f, 476.0f);

    for (int i = 0; i < 10; ++i) {
        int estado = std::clamp(hambre - i * 2, 0, 2);
        dibujarMusloHambre(ventana, {origen.x + static_cast<float>(i) * 22.0f, origen.y}, estado, escala);
    }
}
}

inline Juego::Juego()
    : ventana(sf::VideoMode({800, 600}), "TEST DE CAMBIOS REALES"),
      estaCorriendo(true),
      fuenteCargada(false),
      worldTime(0.0f),
      skyLight(15),
      spawnHostilesHabilitado(false) {
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

    float posX = static_cast<float>(bloqueSpawnX) * TAMANIO_BLOQUE_JUEGO;
    float posY = static_cast<float>(bloqueSpawnY) * TAMANIO_BLOQUE_JUEGO;

    jugador = std::make_unique<Jugador>(posX, posY);
    camara.setSize({560.0f, 420.0f});

    std::uniform_int_distribution<> animalBloqueX(30, mapaSuperficie->getAncho() - 31);
    std::uniform_int_distribution<> animalBloqueY(30, mapaSuperficie->getAlto() - 31);

    auto bloqueSeguroParaAnimal = [&](int bx, int by) {
        return mapaSuperficie->getTipoBloque(bx, by) == TipoBloque::Pasto &&
               !mapaSuperficie->esBloqueSolido(bx, by);
    };

    constexpr int TOTAL_ANIMALES = 420;
    int animalesCreados = 0;
    for (int intento = 0; intento < TOTAL_ANIMALES * 40 && animalesCreados < TOTAL_ANIMALES; ++intento) {
        int bx = animalBloqueX(gen);
        int by = animalBloqueY(gen);
        if (!bloqueSeguroParaAnimal(bx, by)) {
            continue;
        }

        float animalX = static_cast<float>(bx) * TAMANIO_BLOQUE_JUEGO;
        float animalY = static_cast<float>(by) * TAMANIO_BLOQUE_JUEGO;
        TipoAnimal tipo = TipoAnimal::Cerdo;
        int especie = animalesCreados % 4;
        if (especie == 1) {
            tipo = TipoAnimal::Oveja;
        } else if (especie == 2) {
            tipo = TipoAnimal::Vaca;
        } else if (especie == 3) {
            tipo = TipoAnimal::Gallina;
        }
        animales.push_back(new Animal(animalX, animalY, tipo));
        animalesCreados++;
    }
    std::cout << "Fauna esparcida con exito: " << animalesCreados << " animales." << std::endl;

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

    std::cout << "Spawn aleatorio fijado en X: " << (posX / TAMANIO_BLOQUE_JUEGO)
              << " Y: " << (posY / TAMANIO_BLOQUE_JUEGO) << "." << std::endl;
}

inline Juego::~Juego() {
    for (auto* animal : animales) {
        delete animal;
    }
    animales.clear();

    for (auto* zombie : zombis) {
        delete zombie;
    }
    zombis.clear();
}

inline void Juego::actualizarTiempo(float dt) {
    worldTime += dt * TICKS_POR_SEGUNDO_MUNDO;
    while (worldTime >= TICKS_DIA_COMPLETO) {
        worldTime -= TICKS_DIA_COMPLETO;
    }

    skyLight = calcularSkyLight(worldTime);
    spawnHostilesHabilitado = skyLight < 7;
}

inline bool Juego::puedeDormir() const {
    return worldTime >= TICK_PUEDE_DORMIR && worldTime < TICKS_DIA_COMPLETO;
}

inline void Juego::saltarAlAmanecer() {
    worldTime = 0.0f;
    skyLight = 15;
    spawnHostilesHabilitado = false;
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
    float acumuladorSpawnZombies = 0.0f;
    sf::Texture texturaMapaInicial;
    constexpr int TAMANIO_MAPA_INICIAL = 700;
    constexpr int RADIO_MAPA_INICIAL = TAMANIO_MAPA_INICIAL / 2;
    std::random_device rdLoot;
    std::mt19937 genLoot(rdLoot());

    auto generarMapaInicial = [&]() {
        if (!jugador || !mapaSuperficie) return;

        sf::Vector2f centroJugador = jugador->getPosicion() + sf::Vector2f(12.0f, 12.0f);
        mapaCentroX = static_cast<int>(std::floor(centroJugador.x / TAMANIO_BLOQUE_JUEGO));
        mapaCentroY = static_cast<int>(std::floor(centroJugador.y / TAMANIO_BLOQUE_JUEGO));

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

    auto danioContraAnimal = [](ItemId item) {
        switch (item) {
            case ItemId::EspadaMadera: return 4.0f;
            case ItemId::EspadaPiedra: return 5.0f;
            case ItemId::HachaMadera:
            case ItemId::HachaPiedra:
                return 9.0f;
            default:
                return 1.0f;
        }
    };

    auto colorItemSuelo = [](ItemId item) {
        switch (item) {
            case ItemId::ChuletaCerdoCruda: return sf::Color(217, 103, 111);
            case ItemId::ChuletaCerdoCocinada: return sf::Color(139, 79, 45);
            case ItemId::CarneResCruda: return sf::Color(178, 63, 70);
            case ItemId::LanaCruda: return sf::Color(220, 220, 220);
            case ItemId::PolloCrudo: return sf::Color(232, 180, 158);
            case ItemId::Pluma: return sf::Color(240, 240, 230);
            case ItemId::BloqueTronco: return sf::Color(120, 72, 35);
            default: return sf::Color(230, 210, 120);
        }
    };

    auto intentarSpawnearZombie = [&]() {
        if (!jugador || !mapaSuperficie || !spawnHostilesHabilitado || zombis.size() >= 35) {
            return;
        }

        static std::random_device rdZombie;
        static std::mt19937 genZombie(rdZombie());
        std::uniform_real_distribution<float> angulo(0.0f, 6.2831853f);
        std::uniform_real_distribution<float> distanciaSpawn(24.0f, 38.0f);
        std::uniform_int_distribution<int> chanceBebe(1, 100);

        sf::Vector2f centroJugador = jugador->getPosicion() + sf::Vector2f(12.0f, 12.0f);
        for (int intento = 0; intento < 20; ++intento) {
            float a = angulo(genZombie);
            float d = distanciaSpawn(genZombie) * TAMANIO_BLOQUE_JUEGO;
            int bx = static_cast<int>(std::floor((centroJugador.x + std::cos(a) * d) / TAMANIO_BLOQUE_JUEGO));
            int by = static_cast<int>(std::floor((centroJugador.y + std::sin(a) * d) / TAMANIO_BLOQUE_JUEGO));

            TipoBloque tipo = mapaSuperficie->getTipoBloque(bx, by);
            if (mapaSuperficie->esBloqueSolido(bx, by) ||
                tipo == TipoBloque::Agua ||
                tipo == TipoBloque::AguaProfunda ||
                tipo == TipoBloque::Madera) {
                continue;
            }

            bool cercaDeOtroZombie = false;
            sf::Vector2f posSpawn(static_cast<float>(bx) * TAMANIO_BLOQUE_JUEGO, static_cast<float>(by) * TAMANIO_BLOQUE_JUEGO);
            for (auto* zombie : zombis) {
                if (!zombie) continue;
                sf::Vector2f delta = zombie->getPosicion() - posSpawn;
                if (std::sqrt(delta.x * delta.x + delta.y * delta.y) < 5.0f * TAMANIO_BLOQUE_JUEGO) {
                    cercaDeOtroZombie = true;
                    break;
                }
            }
            if (cercaDeOtroZombie) {
                continue;
            }

            bool bebe = chanceBebe(genZombie) <= 5;
            zombis.push_back(new Zombie(posSpawn.x, posSpawn.y, bebe));
            return;
        }
    };

    while (ventana.isOpen() && estaCorriendo) {
        float dt = reloj.restart().asSeconds();
        if (dt > 0.05f) {
            dt = 0.05f;
        }
        actualizarTiempo(dt);
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

                if (botonTeclado->code == sf::Keyboard::Key::B && puedeDormir()) {
                    saltarAlAmanecer();
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

        sf::Vector2f centroCamara = camara.getCenter();
        sf::Vector2f tamanoCamara = camara.getSize();
        float margenActivo = 256.0f;
        float activoIzq = centroCamara.x - tamanoCamara.x / 2.0f - margenActivo;
        float activoDer = centroCamara.x + tamanoCamara.x / 2.0f + margenActivo;
        float activoArriba = centroCamara.y - tamanoCamara.y / 2.0f - margenActivo;
        float activoAbajo = centroCamara.y + tamanoCamara.y / 2.0f + margenActivo;

        for (auto* animal : animales) {
            if (!animal) continue;
            sf::Vector2f posAnimal = animal->getPosicion();
            if (posAnimal.x >= activoIzq && posAnimal.x <= activoDer &&
                posAnimal.y >= activoArriba && posAnimal.y <= activoAbajo) {
                sf::Vector2f objetivoAnimal(-99999.0f, -99999.0f);
                ItemId itemAtraccion = ItemId::Ninguno;
                if (jugador) {
                    objetivoAnimal = jugador->getPosicion() + sf::Vector2f(12.0f, 12.0f);
                    itemAtraccion = inventarioGrid.getItemEnHotbar();
                }
                animal->actualizar(dt, *mapaSuperficie, objetivoAnimal, itemAtraccion);
            }
        }

        if (spawnHostilesHabilitado && !uiAbierta) {
            acumuladorSpawnZombies += dt;
            if (acumuladorSpawnZombies >= 1.0f) {
                acumuladorSpawnZombies = 0.0f;
                intentarSpawnearZombie();
            }
        } else {
            acumuladorSpawnZombies = 0.0f;
        }

        if (jugador && mapaSuperficie) {
            for (auto* zombie : zombis) {
                if (zombie) {
                    zombie->actualizar(dt, *mapaSuperficie, *jugador, skyLight);
                }
            }
        }

        if (jugador) {
            sf::Vector2f centroJugador = jugador->getPosicion() + sf::Vector2f(12.0f, 12.0f);
            for (auto it = itemsSuelo.begin(); it != itemsSuelo.end();) {
                sf::Vector2f delta = it->posicion - centroJugador;
                if (std::sqrt(delta.x * delta.x + delta.y * delta.y) <= 24.0f) {
                    inventarioGrid.agregarItem(it->item, it->cantidad);
                    it = itemsSuelo.erase(it);
                } else {
                    ++it;
                }
            }
        }

        ventana.setView(camara);
        sf::Vector2f posicionMundoMouse = ventana.mapPixelToCoords(mousePos);
        int bloqueMouseX = static_cast<int>(std::floor(posicionMundoMouse.x / TAMANIO_BLOQUE_JUEGO));
        int bloqueMouseY = static_cast<int>(std::floor(posicionMundoMouse.y / TAMANIO_BLOQUE_JUEGO));

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
            bloqueJugadorX = static_cast<int>(std::floor(centroJugador.x / TAMANIO_BLOQUE_JUEGO));
            bloqueJugadorY = static_cast<int>(std::floor(centroJugador.y / TAMANIO_BLOQUE_JUEGO));
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
                sf::Vector2f centroBloque((bloqueMouseX + 0.5f) * TAMANIO_BLOQUE_JUEGO, (bloqueMouseY + 0.5f) * TAMANIO_BLOQUE_JUEGO);
                float dx = centroBloque.x - centroJugador.x;
                float dy = centroBloque.y - centroJugador.y;
                mesaHoverCercana = std::sqrt(dx * dx + dy * dy) <= TAMANIO_BLOQUE_JUEGO * 2.0f;
                clickSobreMesa = mesaHoverCercana && clickIzquierdo && !clickIzquierdoAnterior;
            }
        }

        if (!uiAbierta && clickDerecho && !clickDerechoAnterior) {
            ItemId itemEnMano = inventarioGrid.getItemEnHotbar();
            TipoBloque bloqueAColocar = bloqueDesdeItem(itemEnMano);

            if (jugador && mapaSuperficie) {
                bool consumioComida = jugador->consumirComida(itemEnMano);
                if (consumioComida) {
                    inventarioGrid.consumirItemHotbar(1);
                }

                if (!consumioComida) {
                    sf::Vector2f posJugador = jugador->getPosicion();
                    sf::Vector2f centroJugador = posJugador + sf::Vector2f(12.0f, 12.0f);
                    sf::Vector2f centroBloque((bloqueMouseX + 0.5f) * TAMANIO_BLOQUE_JUEGO, (bloqueMouseY + 0.5f) * TAMANIO_BLOQUE_JUEGO);
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
        }

        if (clickSobreMesa) {
            inventarioGrid.abrirMesaCrafteo();
        }

        bool usandoMina = jugadorSobreEntradaMina && tipoHerramienta(inventarioGrid.getItemEnHotbar()) == TipoHerramienta::Pico;
        if (!uiAbierta && clickIzquierdo && !clickIzquierdoAnterior && jugador) {
            jugador->iniciarAccion(inventarioGrid.getItemEnHotbar());
        }

        bool golpeoAnimal = false;
        bool golpeoZombie = false;
        if (!uiAbierta && clickIzquierdo && !clickIzquierdoAnterior && jugador) {
            sf::Vector2f centroJugador = jugador->getPosicion() + sf::Vector2f(12.0f, 12.0f);
            for (auto* zombie : zombis) {
                if (!zombie || !zombie->estaVivo() || !zombie->contienePunto(posicionMundoMouse)) {
                    continue;
                }

                sf::Vector2f centroZombie = zombie->getPosicion() + sf::Vector2f(12.0f, 12.0f);
                sf::Vector2f delta = centroZombie - centroJugador;
                if (std::sqrt(delta.x * delta.x + delta.y * delta.y) > 95.0f) {
                    continue;
                }

                ItemId arma = inventarioGrid.getItemEnHotbar();
                zombie->recibirDanio(danioContraAnimal(arma) * jugador->getMultiplicadorAtaque(arma));
                jugador->registrarAtaque(arma);
                golpeoZombie = true;
                break;
            }
        }

        if (!uiAbierta && clickIzquierdo && !clickIzquierdoAnterior && jugador) {
            sf::Vector2f centroJugador = jugador->getPosicion() + sf::Vector2f(12.0f, 12.0f);
            for (auto it = animales.begin(); it != animales.end(); ++it) {
                Animal* animal = *it;
                if (!animal || !animal->estaVivo() || !animal->contienePunto(posicionMundoMouse)) {
                    continue;
                }

                sf::Vector2f centroAnimal = animal->getPosicion() + sf::Vector2f(14.0f, 14.0f);
                float dx = centroAnimal.x - centroJugador.x;
                float dy = centroAnimal.y - centroJugador.y;
                if (std::sqrt(dx * dx + dy * dy) > 95.0f) {
                    continue;
                }

                ItemId arma = inventarioGrid.getItemEnHotbar();
                animal->recibirDanio(danioContraAnimal(arma) * jugador->getMultiplicadorAtaque(arma), centroJugador);
                jugador->registrarAtaque(arma);
                golpeoAnimal = true;

                if (animal->estaMuriendo()) {
                    sf::Vector2f posDrop = animal->getPosicion() + sf::Vector2f(12.0f, 12.0f);
                    switch (animal->getTipo()) {
                        case TipoAnimal::Cerdo: {
                            std::uniform_int_distribution<> loot(1, 3);
                            itemsSuelo.push_back({ItemId::ChuletaCerdoCruda, loot(genLoot), posDrop});
                            break;
                        }
                        case TipoAnimal::Vaca: {
                            std::uniform_int_distribution<> loot(1, 3);
                            itemsSuelo.push_back({ItemId::CarneResCruda, loot(genLoot), posDrop});
                            break;
                        }
                        case TipoAnimal::Oveja: {
                            std::uniform_int_distribution<> loot(1, 2);
                            itemsSuelo.push_back({ItemId::LanaCruda, loot(genLoot), posDrop});
                            break;
                        }
                        case TipoAnimal::Gallina: {
                            std::uniform_int_distribution<> plumas(0, 2);
                            itemsSuelo.push_back({ItemId::PolloCrudo, 1, posDrop});
                            int cantidadPlumas = plumas(genLoot);
                            if (cantidadPlumas > 0) {
                                itemsSuelo.push_back({ItemId::Pluma, cantidadPlumas, posDrop + sf::Vector2f(10.0f, -6.0f)});
                            }
                            break;
                        }
                    }
                }
                break;
            }
        }

        for (auto it = animales.begin(); it != animales.end();) {
            Animal* animal = *it;
            if (animal && animal->muerteFinalizada()) {
                delete animal;
                it = animales.erase(it);
            } else {
                ++it;
            }
        }

        for (auto it = zombis.begin(); it != zombis.end();) {
            Zombie* zombie = *it;
            if (zombie && zombie->debeEliminarse()) {
                delete zombie;
                it = zombis.erase(it);
            } else {
                ++it;
            }
        }

        if (!uiAbierta && clickIzquierdo && !clickSobreMesa && !usandoMina && !golpeoAnimal && !golpeoZombie) {
            ventana.setView(camara);
            int bloqueX = bloqueMouseX;
            int bloqueY = bloqueMouseY;

            if (jugador && mapaSuperficie) {
                sf::Vector2f posJugador = jugador->getPosicion();
                sf::Vector2f centroJugador = posJugador + sf::Vector2f(12.0f, 12.0f);
                sf::Vector2f centroBloque((bloqueX + 0.5f) * TAMANIO_BLOQUE_JUEGO, (bloqueY + 0.5f) * TAMANIO_BLOQUE_JUEGO);

                float dx = centroBloque.x - centroJugador.x;
                float dy = centroBloque.y - centroJugador.y;
                float distancia = std::sqrt(dx * dx + dy * dy);

                if (distancia <= 115.0f) {
                    TipoBloque tipoActual = mapaSuperficie->getTipoBloque(bloqueX, bloqueY);

                    if (tipoActual != TipoBloque::Aire && tipoActual != TipoBloque::Agua) {
                        ItemId itemEnMano = inventarioGrid.getItemEnHotbar();
                        if (tipoActual == TipoBloque::Madera) {
                            float velocidadTala = 1.0f;
                            if (itemEnMano == ItemId::HachaMadera) velocidadTala = 20.0f / 13.0f;
                            if (itemEnMano == ItemId::HachaPiedra) velocidadTala = 2.0f;

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

                        if (tipoActual == TipoBloque::Tierra &&
                            itemEnMano == ItemId::Barreta &&
                            mapaSuperficie->crearEntradaMina(bloqueX, bloqueY)) {
                            tipoActual = TipoBloque::Aire;
                        }

                        if (tipoActual != TipoBloque::Aire) {
                            float danioPorSegundo = herramientas.calcularDanio(tipoActual, itemEnMano);
                            float danioAplicado = danioPorSegundo * dt;
                            bool destruido = mapaSuperficie->daniarBloque(bloqueX, bloqueY, danioAplicado);

                            if (destruido) {
                                jugador->agregarAgotamiento(0.005f);
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
                sf::Vector2f centroBloque((bloqueMouseX + 0.5f) * TAMANIO_BLOQUE_JUEGO, (bloqueMouseY + 0.5f) * TAMANIO_BLOQUE_JUEGO);
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

        sf::Vector2f centroVista = camara.getCenter();
        sf::Vector2f tamanoVista = camara.getSize();
        float margenDibujo = 96.0f;
        float dibujoIzq = centroVista.x - tamanoVista.x / 2.0f - margenDibujo;
        float dibujoDer = centroVista.x + tamanoVista.x / 2.0f + margenDibujo;
        float dibujoArriba = centroVista.y - tamanoVista.y / 2.0f - margenDibujo;
        float dibujoAbajo = centroVista.y + tamanoVista.y / 2.0f + margenDibujo;

        for (auto* animal : animales) {
            if (!animal) continue;
            sf::Vector2f posAnimal = animal->getPosicion();
            if (posAnimal.x >= dibujoIzq && posAnimal.x <= dibujoDer &&
                posAnimal.y >= dibujoArriba && posAnimal.y <= dibujoAbajo) {
                animal->dibujar(ventana);
            }
        }

        for (auto* zombie : zombis) {
            if (!zombie) continue;
            sf::Vector2f posZombie = zombie->getPosicion();
            if (posZombie.x >= dibujoIzq && posZombie.x <= dibujoDer &&
                posZombie.y >= dibujoArriba && posZombie.y <= dibujoAbajo) {
                zombie->dibujar(ventana);
            }
        }

        for (const auto& item : itemsSuelo) {
            if (item.posicion.x < dibujoIzq || item.posicion.x > dibujoDer ||
                item.posicion.y < dibujoArriba || item.posicion.y > dibujoAbajo) {
                continue;
            }

            sf::RectangleShape sombra({16.0f, 5.0f});
            sombra.setOrigin({8.0f, 2.5f});
            sombra.setPosition({item.posicion.x, item.posicion.y + 9.0f});
            sombra.setFillColor(sf::Color(20, 20, 20, 80));
            ventana.draw(sombra);

            sf::RectangleShape icono({12.0f, 9.0f});
            icono.setOrigin({6.0f, 4.5f});
            icono.setPosition(item.posicion);
            icono.setFillColor(colorItemSuelo(item.item));
            icono.setOutlineColor(sf::Color(70, 35, 35));
            icono.setOutlineThickness(1.0f);
            ventana.draw(icono);

            if (item.cantidad > 1) {
                sf::CircleShape brillo(2.0f);
                brillo.setPosition({item.posicion.x + 3.0f, item.posicion.y - 4.0f});
                brillo.setFillColor(sf::Color::White);
                ventana.draw(brillo);
            }
        }

        if (jugador) {
            jugador->dibujar(ventana);
            if (mapaSuperficie) {
                mapaSuperficie->dibujarArbolesSobreJugador(ventana, jugador->getPosicion().y + 24.0f);
            }
        }

        dibujarFiltroDiaNoche(ventana, camara, worldTime, skyLight);

        ventana.setView(ventana.getDefaultView());

        if (jugador) {
            dibujarBarraVida(ventana, *jugador);
            dibujarBarraHambre(ventana, *jugador);
        }

        if (fuenteCargada && textoCoordenadas && jugador && mostrarDebug) {
            std::stringstream ss;
            sf::Vector2f pos = jugador->getPosicion();

            std::string nombreEnMano = nombreItem(inventarioGrid.getItemEnHotbar());

            ss << "FPS: " << fpsActuales << "\n"
               << "Bloque Coords -> X: " << static_cast<int>(pos.x / TAMANIO_BLOQUE_JUEGO)
               << " Y: " << static_cast<int>(pos.y / TAMANIO_BLOQUE_JUEGO) << "\n"
               << "Item en Mano: " << nombreEnMano << "\n"
               << "Tiempo: " << static_cast<int>(worldTime)
               << " (" << nombreMomentoDia(worldTime) << ")"
               << "\nLuz cielo: " << skyLight
               << "\nSpawn hostil: " << (spawnHostilesHabilitado ? "Activo" : "Inactivo")
               << "\nZombies: " << zombis.size()
               << "\nHambre: " << jugador->getHambre()
               << " Sat: " << static_cast<int>(jugador->getSaturacion() * 10.0f) / 10.0f
               << " Agot: " << static_cast<int>(jugador->getAgotamiento() * 10.0f) / 10.0f
               << "\nCarga ataque: " << static_cast<int>(jugador->getMultiplicadorAtaque(inventarioGrid.getItemEnHotbar()) * 100.0f) << "%"
               << "\nDormir: " << (puedeDormir() ? "Disponible (B)" : "No");

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
                int jugadorMapaX = static_cast<int>(std::floor(centroJugador.x / TAMANIO_BLOQUE_JUEGO)) - (mapaCentroX - RADIO_MAPA_INICIAL);
                int jugadorMapaY = static_cast<int>(std::floor(centroJugador.y / TAMANIO_BLOQUE_JUEGO)) - (mapaCentroY - RADIO_MAPA_INICIAL);
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


