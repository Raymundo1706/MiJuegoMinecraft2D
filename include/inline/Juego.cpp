#include <iostream>
#include <random>
#include <sstream>
#include <cmath>
#include <cstdint>
#include <vector>
#include <string>
#include <SFML/Audio.hpp>
#include "SistemaHerramientas.hpp"
#include "InventarioGrid.hpp"

namespace {
constexpr float TICKS_POR_SEGUNDO_MUNDO = 216.67f;
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

inline void dibujarPixelMundo(sf::RenderWindow& ventana, sf::Vector2f origen, int x, int y, sf::Color color, float escala) {
    sf::RectangleShape pixel({escala, escala});
    pixel.setPosition({origen.x + static_cast<float>(x) * escala, origen.y + static_cast<float>(y) * escala});
    pixel.setFillColor(color);
    ventana.draw(pixel);
}

inline void dibujarTroncoSuelo(sf::RenderWindow& ventana, sf::Vector2f centro) {
    static sf::Texture texturaTroncoSuelo;
    static bool intentoTextura = false;
    static bool texturaLista = false;

    if (!intentoTextura) {
        intentoTextura = true;
        texturaLista = texturaTroncoSuelo.loadFromFile("assets/items/log_bundle.png");
        if (texturaLista) {
            texturaTroncoSuelo.setSmooth(false);
        }
    }

    if (texturaLista) {
        sf::Vector2u tam = texturaTroncoSuelo.getSize();
        float escala = 20.0f / static_cast<float>(std::max(tam.x, tam.y));
        sf::Sprite sprite(texturaTroncoSuelo);
        sprite.setOrigin({static_cast<float>(tam.x) * 0.5f, static_cast<float>(tam.y) * 0.5f});
        sprite.setPosition(centro);
        sprite.setScale({escala, escala});
        ventana.draw(sprite);
        return;
    }

    const float escala = 1.0f;
    sf::Vector2f origen(centro.x - 8.0f, centro.y - 8.0f);
    sf::Color borde(20, 13, 11);
    sf::Color madera(112, 68, 48);
    sf::Color maderaLuz(150, 95, 66);
    sf::Color maderaOscura(64, 38, 32);
    sf::Color corte(178, 128, 77);
    sf::Color corteOscuro(105, 70, 44);
    sf::Color cuerda(190, 144, 86);

    auto pixel = [&](int x, int y, sf::Color color) {
        dibujarPixelMundo(ventana, origen, x, y, color, escala);
    };

    auto barra = [&](int x, int y, int largo) {
        for (int py = y; py < y + 5; ++py) {
            for (int px = x + 2; px <= x + largo; ++px) {
                pixel(px, py, madera);
            }
        }
        for (int px = x + 2; px <= x + largo; ++px) {
            pixel(px, y, borde);
            pixel(px, y + 4, borde);
        }
        for (int py = y; py < y + 5; ++py) {
            pixel(x + largo, py, borde);
        }
        pixel(x + 5, y + 1, maderaLuz);
        pixel(x + 7, y + 2, maderaOscura);
        pixel(x + 9, y + 3, maderaOscura);
    };

    auto corteFrente = [&](int cx, int cy) {
        pixel(cx - 2, cy - 1, borde);
        pixel(cx - 1, cy - 2, borde);
        pixel(cx, cy - 2, borde);
        pixel(cx + 1, cy - 1, borde);
        pixel(cx + 1, cy, borde);
        pixel(cx, cy + 1, borde);
        pixel(cx - 1, cy + 1, borde);
        pixel(cx - 2, cy, borde);
        pixel(cx - 1, cy - 1, corte);
        pixel(cx, cy - 1, corte);
        pixel(cx - 1, cy, corteOscuro);
        pixel(cx, cy, corte);
    };

    barra(2, 4, 11);
    barra(4, 9, 10);
    corteFrente(3, 6);
    corteFrente(5, 11);
    corteFrente(9, 12);

    for (int y = 4; y <= 12; ++y) {
        pixel(11, y, cuerda);
        if (y % 2 == 0) pixel(12, y, sf::Color(122, 84, 50));
    }
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
    constexpr float escala = 1.9f;
    sf::Vector2f origen(184.0f, 480.0f);

    for (int i = 0; i < corazones; ++i) {
        int hpCorazon = std::clamp(vida - i * 2, 0, 2);
        dibujarCorazon(ventana, {origen.x + static_cast<float>(i) * 17.0f, origen.y}, hpCorazon, escala);
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
    constexpr float escala = 1.9f;
    sf::Vector2f origen(445.0f, 480.0f);

    for (int i = 0; i < 10; ++i) {
        int estado = std::clamp(hambre - i * 2, 0, 2);
        dibujarMusloHambre(ventana, {origen.x + static_cast<float>(i) * 17.0f, origen.y}, estado, escala);
    }
}

inline sf::FloatRect rectBotonMenu(int indice) {
    return sf::FloatRect({232.0f, 274.0f + static_cast<float>(indice) * 48.0f}, {336.0f, 36.0f});
}

inline int indiceBotonMenu(sf::Vector2i mouse) {
    sf::Vector2f pos(static_cast<float>(mouse.x), static_cast<float>(mouse.y));
    for (int i = 0; i < 4; ++i) {
        if (rectBotonMenu(i).contains(pos)) {
            return i;
        }
    }
    return -1;
}

inline void centrarTexto(sf::Text& texto, sf::Vector2f posicion) {
    sf::FloatRect bounds = texto.getLocalBounds();
    texto.setOrigin({bounds.position.x + bounds.size.x * 0.5f, bounds.position.y + bounds.size.y * 0.5f});
    texto.setPosition(posicion);
}

inline void dibujarPanoramaMenu(sf::RenderWindow& ventana, float tiempo) {
    sf::RectangleShape cielo({800.0f, 600.0f});
    cielo.setFillColor(sf::Color(74, 137, 204));
    ventana.draw(cielo);

    float desplazamiento = std::fmod(tiempo * 18.0f, 800.0f);
    for (int capa = 0; capa < 2; ++capa) {
        float baseX = -desplazamiento + capa * 800.0f;

        for (int i = 0; i < 9; ++i) {
            float x = baseX + static_cast<float>(i) * 96.0f;
            float y = 64.0f + std::sin(tiempo * 0.45f + static_cast<float>(i)) * 12.0f;
            sf::RectangleShape nube({54.0f, 18.0f});
            nube.setPosition({x, y});
            nube.setFillColor(sf::Color(218, 232, 244, 150));
            ventana.draw(nube);
            nube.setSize({36.0f, 18.0f});
            nube.setPosition({x + 26.0f, y - 12.0f});
            ventana.draw(nube);
        }

        for (int i = 0; i < 18; ++i) {
            float x = baseX + static_cast<float>(i) * 48.0f;
            float altura = 70.0f + std::sin(tiempo * 0.25f + static_cast<float>(i) * 0.8f) * 34.0f;
            sf::RectangleShape monte({54.0f, altura});
            monte.setPosition({x, 310.0f - altura});
            monte.setFillColor(i % 3 == 0 ? sf::Color(61, 104, 75) : sf::Color(50, 88, 68));
            ventana.draw(monte);
        }
    }

    for (int y = 330; y < 600; y += 24) {
        for (int x = -24; x < 824; x += 24) {
            int ruido = (x / 24 + y / 24) % 4;
            sf::RectangleShape bloque({24.0f, 24.0f});
            bloque.setPosition({static_cast<float>(x), static_cast<float>(y)});
            bloque.setFillColor(ruido == 0 ? sf::Color(62, 139, 64) : sf::Color(48, 118, 54));
            ventana.draw(bloque);
        }
    }

    sf::RectangleShape filtro({800.0f, 600.0f});
    filtro.setFillColor(sf::Color(0, 0, 0, 92));
    ventana.draw(filtro);
}

inline void dibujarBotonMenu(sf::RenderWindow& ventana, sf::Font& fuente, int indice, const char* texto, bool seleccionado) {
    sf::FloatRect rect = rectBotonMenu(indice);
    sf::RectangleShape sombra(rect.size);
    sombra.setPosition({rect.position.x + 4.0f, rect.position.y + 4.0f});
    sombra.setFillColor(sf::Color(0, 0, 0, 135));
    ventana.draw(sombra);

    sf::RectangleShape boton(rect.size);
    boton.setPosition(rect.position);
    boton.setFillColor(seleccionado ? sf::Color(128, 142, 124, 238) : sf::Color(92, 92, 92, 226));
    boton.setOutlineColor(seleccionado ? sf::Color(180, 255, 116) : sf::Color(38, 38, 38));
    boton.setOutlineThickness(seleccionado ? 3.0f : 2.0f);
    ventana.draw(boton);

    sf::RectangleShape brillo({rect.size.x - 8.0f, 3.0f});
    brillo.setPosition({rect.position.x + 4.0f, rect.position.y + 4.0f});
    brillo.setFillColor(seleccionado ? sf::Color(230, 255, 196, 110) : sf::Color(210, 210, 210, 65));
    ventana.draw(brillo);

    sf::Text etiqueta(fuente, texto, 18);
    etiqueta.setFillColor(sf::Color::White);
    etiqueta.setOutlineColor(sf::Color::Black);
    etiqueta.setOutlineThickness(2.0f);
    centrarTexto(etiqueta, {rect.position.x + rect.size.x * 0.5f, rect.position.y + rect.size.y * 0.48f});
    ventana.draw(etiqueta);
}

inline void dibujarMenuInicio(sf::RenderWindow& ventana, sf::Font& fuente, bool fuenteCargada, float tiempo, int opcionSeleccionada, sf::Vector2i mouse) {
    ventana.setView(ventana.getDefaultView());
    dibujarPanoramaMenu(ventana, tiempo);

    int hover = indiceBotonMenu(mouse);
    if (hover >= 0) {
        opcionSeleccionada = hover;
    }

    if (fuenteCargada) {
        float logoY = 112.0f + std::sin(tiempo * 1.25f) * 4.0f;
        sf::Text sombraLogo(fuente, "MINECRAFT 2D", 54);
        sombraLogo.setFillColor(sf::Color(0, 0, 0, 170));
        sombraLogo.setOutlineColor(sf::Color(0, 0, 0));
        sombraLogo.setOutlineThickness(4.0f);
        centrarTexto(sombraLogo, {404.0f, logoY + 8.0f});
        ventana.draw(sombraLogo);

        sf::Text logo(fuente, "MINECRAFT 2D", 54);
        logo.setFillColor(sf::Color(196, 196, 196));
        logo.setOutlineColor(sf::Color(48, 48, 48));
        logo.setOutlineThickness(4.0f);
        centrarTexto(logo, {400.0f, logoY});
        ventana.draw(logo);

        float pulso = 1.0f + std::sin(tiempo * 5.8f) * 0.08f;
        sf::Text splash(fuente, "Top-down survival!", 18);
        splash.setFillColor(sf::Color(255, 234, 54));
        splash.setOutlineColor(sf::Color(70, 48, 0));
        splash.setOutlineThickness(2.0f);
        centrarTexto(splash, {536.0f, logoY + 46.0f});
        splash.setRotation(sf::degrees(-20.0f));
        splash.setScale({pulso, pulso});
        ventana.draw(splash);

        dibujarBotonMenu(ventana, fuente, 0, "Jugar", opcionSeleccionada == 0);
        dibujarBotonMenu(ventana, fuente, 1, "Tabla de Clasificacion / Logros", opcionSeleccionada == 1);
        dibujarBotonMenu(ventana, fuente, 2, "Ayuda y Opciones", opcionSeleccionada == 2);
        dibujarBotonMenu(ventana, fuente, 3, "Salir del Juego", opcionSeleccionada == 3);

        sf::Text version(fuente, "v1.0.0", 13);
        version.setFillColor(sf::Color(220, 220, 220));
        version.setOutlineColor(sf::Color::Black);
        version.setOutlineThickness(2.0f);
        version.setPosition({12.0f, 572.0f});
        ventana.draw(version);

        sf::Text copy(fuente, "(c) 2026 Raymu Studio", 13);
        copy.setFillColor(sf::Color(220, 220, 220));
        copy.setOutlineColor(sf::Color::Black);
        copy.setOutlineThickness(2.0f);
        sf::FloatRect copyBounds = copy.getLocalBounds();
        copy.setPosition({788.0f - copyBounds.size.x, 572.0f});
        ventana.draw(copy);
    }
}

inline sf::FloatRect rectBotonLista(float y, float ancho = 360.0f) {
    return sf::FloatRect({400.0f - ancho * 0.5f, y}, {ancho, 36.0f});
}

inline int indiceListaMenu(sf::Vector2i mouse, int cantidad, float yInicial, float paso = 48.0f) {
    sf::Vector2f pos(static_cast<float>(mouse.x), static_cast<float>(mouse.y));
    for (int i = 0; i < cantidad; ++i) {
        if (rectBotonLista(yInicial + static_cast<float>(i) * paso).contains(pos)) {
            return i;
        }
    }
    return -1;
}

inline void dibujarBotonRect(sf::RenderWindow& ventana, sf::Font& fuente, sf::FloatRect rect, const char* texto, bool seleccionado, int tamTexto = 18) {
    sf::RectangleShape sombra(rect.size);
    sombra.setPosition({rect.position.x + 4.0f, rect.position.y + 4.0f});
    sombra.setFillColor(sf::Color(0, 0, 0, 135));
    ventana.draw(sombra);

    sf::RectangleShape boton(rect.size);
    boton.setPosition(rect.position);
    boton.setFillColor(seleccionado ? sf::Color(126, 142, 122, 238) : sf::Color(86, 86, 86, 232));
    boton.setOutlineColor(seleccionado ? sf::Color(180, 255, 116) : sf::Color(36, 36, 36));
    boton.setOutlineThickness(seleccionado ? 3.0f : 2.0f);
    ventana.draw(boton);

    sf::RectangleShape brillo({rect.size.x - 8.0f, 3.0f});
    brillo.setPosition({rect.position.x + 4.0f, rect.position.y + 4.0f});
    brillo.setFillColor(seleccionado ? sf::Color(230, 255, 196, 110) : sf::Color(210, 210, 210, 60));
    ventana.draw(brillo);

    sf::Text etiqueta(fuente, texto, tamTexto);
    etiqueta.setFillColor(sf::Color::White);
    etiqueta.setOutlineColor(sf::Color::Black);
    etiqueta.setOutlineThickness(2.0f);
    centrarTexto(etiqueta, {rect.position.x + rect.size.x * 0.5f, rect.position.y + rect.size.y * 0.48f});
    ventana.draw(etiqueta);
}

inline void dibujarTituloSubmenu(sf::RenderWindow& ventana, sf::Font& fuente, const char* titulo) {
    sf::RectangleShape velo({800.0f, 600.0f});
    velo.setFillColor(sf::Color(0, 0, 0, 132));
    ventana.draw(velo);

    sf::Text texto(fuente, titulo, 32);
    texto.setFillColor(sf::Color(232, 232, 232));
    texto.setOutlineColor(sf::Color::Black);
    texto.setOutlineThickness(3.0f);
    centrarTexto(texto, {400.0f, 82.0f});
    ventana.draw(texto);
}

inline void dibujarAyudaOpciones(sf::RenderWindow& ventana, sf::Font& fuente, float tiempo, int opcion, sf::Vector2i mouse) {
    dibujarPanoramaMenu(ventana, tiempo);
    dibujarTituloSubmenu(ventana, fuente, "Ayuda y Opciones");

    const char* opciones[5] = {
        "Como Jugar",
        "Controles",
        "Configuracion",
        "Creditos",
        "Atras"
    };

    int hover = indiceListaMenu(mouse, 5, 190.0f);
    if (hover >= 0) {
        opcion = hover;
    }

    for (int i = 0; i < 5; ++i) {
        dibujarBotonRect(ventana, fuente, rectBotonLista(190.0f + static_cast<float>(i) * 48.0f), opciones[i], opcion == i);
    }
}

inline void dibujarGuiaControlesMenu(sf::RenderWindow& ventana, sf::Font& fuente) {
    sf::RectangleShape barra({800.0f, 34.0f});
    barra.setPosition({0.0f, 566.0f});
    barra.setFillColor(sf::Color(0, 0, 0, 150));
    ventana.draw(barra);

    sf::CircleShape a(8.0f);
    a.setPosition({30.0f, 575.0f});
    a.setFillColor(sf::Color(54, 185, 66));
    ventana.draw(a);
    sf::CircleShape b(8.0f);
    b.setPosition({168.0f, 575.0f});
    b.setFillColor(sf::Color(204, 52, 48));
    ventana.draw(b);

    sf::Text ta(fuente, "A Seleccionar", 13);
    ta.setFillColor(sf::Color::White);
    ta.setOutlineColor(sf::Color::Black);
    ta.setOutlineThickness(2.0f);
    ta.setPosition({52.0f, 573.0f});
    ventana.draw(ta);

    sf::Text tb(fuente, "B Atras", 13);
    tb.setFillColor(sf::Color::White);
    tb.setOutlineColor(sf::Color::Black);
    tb.setOutlineThickness(2.0f);
    tb.setPosition({190.0f, 573.0f});
    ventana.draw(tb);
}

inline void dibujarCheckboxMenu(sf::RenderWindow& ventana, sf::Font& fuente, sf::Vector2f pos, const char* texto, bool activo) {
    sf::RectangleShape caja({20.0f, 20.0f});
    caja.setPosition(pos);
    caja.setFillColor(sf::Color(36, 36, 36, 230));
    caja.setOutlineColor(sf::Color(180, 180, 180));
    caja.setOutlineThickness(2.0f);
    ventana.draw(caja);

    if (activo) {
        sf::RectangleShape marca1({14.0f, 4.0f});
        marca1.setPosition({pos.x + 4.0f, pos.y + 9.0f});
        marca1.setFillColor(sf::Color(118, 220, 75));
        marca1.setRotation(sf::degrees(-35.0f));
        ventana.draw(marca1);
    }

    sf::Text etiqueta(fuente, texto, 15);
    etiqueta.setFillColor(sf::Color::White);
    etiqueta.setOutlineColor(sf::Color::Black);
    etiqueta.setOutlineThickness(2.0f);
    etiqueta.setPosition({pos.x + 30.0f, pos.y - 1.0f});
    ventana.draw(etiqueta);
}

inline void dibujarEntradaTextoMenu(sf::RenderWindow& ventana, sf::Font& fuente, sf::FloatRect rect, const char* etiqueta, const std::string& valor, bool activa, const char* ayuda = nullptr) {
    sf::Text titulo(fuente, etiqueta, 15);
    titulo.setFillColor(sf::Color::White);
    titulo.setOutlineColor(sf::Color::Black);
    titulo.setOutlineThickness(2.0f);
    titulo.setPosition({rect.position.x, rect.position.y - 24.0f});
    ventana.draw(titulo);

    sf::RectangleShape campo(rect.size);
    campo.setPosition(rect.position);
    campo.setFillColor(sf::Color(32, 32, 32, 235));
    campo.setOutlineColor(activa ? sf::Color(180, 255, 116) : sf::Color(130, 130, 130));
    campo.setOutlineThickness(activa ? 3.0f : 2.0f);
    ventana.draw(campo);

    sf::Text texto(fuente, valor.empty() ? " " : valor, 16);
    texto.setFillColor(sf::Color::White);
    texto.setOutlineColor(sf::Color::Black);
    texto.setOutlineThickness(2.0f);
    texto.setPosition({rect.position.x + 10.0f, rect.position.y + 8.0f});
    ventana.draw(texto);

    if (ayuda) {
        sf::Text h(fuente, ayuda, 12);
        h.setFillColor(sf::Color(184, 184, 184));
        h.setOutlineColor(sf::Color::Black);
        h.setOutlineThickness(1.0f);
        h.setPosition({rect.position.x, rect.position.y + rect.size.y + 6.0f});
        ventana.draw(h);
    }
}

inline void dibujarMenuJugar(sf::RenderWindow& ventana, sf::Font& fuente, float tiempo, bool online, sf::Vector2i mouse) {
    dibujarPanoramaMenu(ventana, tiempo);
    dibujarTituloSubmenu(ventana, fuente, "Jugar");

    sf::RectangleShape panel({700.0f, 414.0f});
    panel.setPosition({50.0f, 112.0f});
    panel.setFillColor(sf::Color(34, 34, 34, 226));
    panel.setOutlineColor(sf::Color(142, 142, 142));
    panel.setOutlineThickness(3.0f);
    ventana.draw(panel);

    dibujarCheckboxMenu(ventana, fuente, {82.0f, 134.0f}, "Partida en linea", online);

    sf::Text iniciar(fuente, "Iniciar Partida", 18);
    iniciar.setFillColor(sf::Color(255, 236, 120));
    iniciar.setPosition({82.0f, 178.0f});
    ventana.draw(iniciar);

    sf::Text unir(fuente, "Unirse a Partida", 18);
    unir.setFillColor(sf::Color(255, 236, 120));
    unir.setPosition({446.0f, 178.0f});
    ventana.draw(unir);

    dibujarBotonRect(ventana, fuente, sf::FloatRect({82.0f, 212.0f}, {284.0f, 34.0f}), "Crear nuevo mundo", sf::FloatRect({82.0f, 212.0f}, {284.0f, 34.0f}).contains(sf::Vector2f(mouse)), 15);
    dibujarBotonRect(ventana, fuente, sf::FloatRect({82.0f, 254.0f}, {284.0f, 34.0f}), "Jugar Tutorial", sf::FloatRect({82.0f, 254.0f}, {284.0f, 34.0f}).contains(sf::Vector2f(mouse)), 15);

    sf::Text guardados(fuente, "Mundos guardados", 14);
    guardados.setFillColor(sf::Color(210, 210, 210));
    guardados.setPosition({84.0f, 304.0f});
    ventana.draw(guardados);

    const char* nombres[3] = {"Mundo de Raymu", "Bosque inicial", "Prueba de zombies"};
    const char* semillas[3] = {"Seed: 20260609", "Seed: random", "Seed: night-test"};
    for (int i = 0; i < 3; ++i) {
        float y = 332.0f + static_cast<float>(i) * 48.0f;
        sf::FloatRect r({82.0f, y}, {284.0f, 40.0f});
        bool hover = r.contains(sf::Vector2f(mouse));
        sf::RectangleShape fila(r.size);
        fila.setPosition(r.position);
        fila.setFillColor(hover ? sf::Color(92, 108, 82, 230) : sf::Color(48, 48, 48, 220));
        fila.setOutlineColor(hover ? sf::Color(180, 255, 116) : sf::Color(24, 24, 24));
        fila.setOutlineThickness(2.0f);
        ventana.draw(fila);

        sf::RectangleShape icono({28.0f, 28.0f});
        icono.setPosition({r.position.x + 8.0f, r.position.y + 6.0f});
        icono.setFillColor(sf::Color(72, 152, 64));
        icono.setOutlineColor(sf::Color(64, 42, 24));
        icono.setOutlineThickness(2.0f);
        ventana.draw(icono);

        sf::Text nombre(fuente, nombres[i], 13);
        nombre.setFillColor(sf::Color::White);
        nombre.setOutlineColor(sf::Color::Black);
        nombre.setOutlineThickness(1.0f);
        nombre.setPosition({r.position.x + 46.0f, r.position.y + 5.0f});
        ventana.draw(nombre);

        sf::Text seed(fuente, semillas[i], 11);
        seed.setFillColor(sf::Color(190, 190, 190));
        seed.setOutlineColor(sf::Color::Black);
        seed.setOutlineThickness(1.0f);
        seed.setPosition({r.position.x + 46.0f, r.position.y + 23.0f});
        ventana.draw(seed);
    }

    sf::FloatRect partida({446.0f, 216.0f}, {222.0f, 54.0f});
    bool hoverPartida = partida.contains(sf::Vector2f(mouse));
    dibujarBotonRect(ventana, fuente, partida, "Sesion de amigo", hoverPartida, 15);

    sf::Text detalle(fuente, "2 jugadores en linea\nSupervivencia - Normal", 12);
    detalle.setFillColor(sf::Color(210, 210, 210));
    detalle.setOutlineColor(sf::Color::Black);
    detalle.setOutlineThickness(1.0f);
    detalle.setPosition({456.0f, 286.0f});
    ventana.draw(detalle);

    dibujarBotonRect(ventana, fuente, sf::FloatRect({576.0f, 482.0f}, {124.0f, 34.0f}), "Atras", sf::FloatRect({576.0f, 482.0f}, {124.0f, 34.0f}).contains(sf::Vector2f(mouse)), 15);
    dibujarGuiaControlesMenu(ventana, fuente);
}

inline void dibujarCrearMundo(sf::RenderWindow& ventana, sf::Font& fuente, float tiempo, bool online, bool soloInvitados,
                              const std::string& nombreMundo, const std::string& semilla, int dificultad, int inputActivo, sf::Vector2i mouse) {
    dibujarPanoramaMenu(ventana, tiempo);
    dibujarTituloSubmenu(ventana, fuente, "Crear Nuevo Mundo");

    sf::RectangleShape panel({520.0f, 430.0f});
    panel.setPosition({140.0f, 106.0f});
    panel.setFillColor(sf::Color(34, 34, 34, 232));
    panel.setOutlineColor(sf::Color(142, 142, 142));
    panel.setOutlineThickness(3.0f);
    ventana.draw(panel);

    dibujarCheckboxMenu(ventana, fuente, {176.0f, 132.0f}, "Online game", online);
    dibujarCheckboxMenu(ventana, fuente, {410.0f, 132.0f}, "Solo invitados", online && soloInvitados);

    dibujarEntradaTextoMenu(ventana, fuente, sf::FloatRect({176.0f, 202.0f}, {448.0f, 40.0f}), "Nombre del Mundo", nombreMundo, inputActivo == 1);
    dibujarEntradaTextoMenu(ventana, fuente, sf::FloatRect({176.0f, 286.0f}, {448.0f, 40.0f}), "Semilla para el Generador", semilla, inputActivo == 2, "Leave blank for a random seed");

    const char* dificultades[4] = {"Pacifico", "Facil", "Normal", "Dificil"};
    sf::Text d(fuente, "Dificultad", 15);
    d.setFillColor(sf::Color::White);
    d.setOutlineColor(sf::Color::Black);
    d.setOutlineThickness(2.0f);
    d.setPosition({176.0f, 356.0f});
    ventana.draw(d);

    dibujarBotonRect(ventana, fuente, sf::FloatRect({400.0f, 348.0f}, {224.0f, 34.0f}), dificultades[std::clamp(dificultad, 0, 3)],
                     sf::FloatRect({400.0f, 348.0f}, {224.0f, 34.0f}).contains(sf::Vector2f(mouse)), 15);
    dibujarBotonRect(ventana, fuente, sf::FloatRect({176.0f, 440.0f}, {288.0f, 40.0f}), "Crear nuevo mundo",
                     sf::FloatRect({176.0f, 440.0f}, {288.0f, 40.0f}).contains(sf::Vector2f(mouse)), 16);
    dibujarBotonRect(ventana, fuente, sf::FloatRect({500.0f, 440.0f}, {124.0f, 40.0f}), "Atras",
                     sf::FloatRect({500.0f, 440.0f}, {124.0f, 40.0f}).contains(sf::Vector2f(mouse)), 15);
    dibujarGuiaControlesMenu(ventana, fuente);
}

inline void dibujarUnirsePartida(sf::RenderWindow& ventana, sf::Font& fuente, float tiempo, sf::Vector2i mouse) {
    dibujarPanoramaMenu(ventana, tiempo);
    dibujarTituloSubmenu(ventana, fuente, "Join Game");

    sf::RectangleShape panel({460.0f, 330.0f});
    panel.setPosition({170.0f, 144.0f});
    panel.setFillColor(sf::Color(34, 34, 34, 232));
    panel.setOutlineColor(sf::Color(142, 142, 142));
    panel.setOutlineThickness(3.0f);
    ventana.draw(panel);

    sf::Text titulo(fuente, "Players in game", 20);
    titulo.setFillColor(sf::Color(255, 236, 120));
    centrarTexto(titulo, {400.0f, 186.0f});
    ventana.draw(titulo);

    const char* jugadores[3] = {"Raymu", "Invitado 01", "Constructor 2D"};
    for (int i = 0; i < 3; ++i) {
        sf::FloatRect r({236.0f, 228.0f + static_cast<float>(i) * 50.0f}, {328.0f, 38.0f});
        sf::RectangleShape fila(r.size);
        fila.setPosition(r.position);
        fila.setFillColor(sf::Color(52, 52, 52, 228));
        fila.setOutlineColor(sf::Color(24, 24, 24));
        fila.setOutlineThickness(2.0f);
        ventana.draw(fila);

        sf::CircleShape avatar(12.0f);
        avatar.setPosition({r.position.x + 12.0f, r.position.y + 7.0f});
        avatar.setFillColor(i == 0 ? sf::Color(74, 162, 220) : sf::Color(110, 110, 160));
        ventana.draw(avatar);

        sf::Text nombre(fuente, jugadores[i], 14);
        nombre.setFillColor(sf::Color::White);
        nombre.setOutlineColor(sf::Color::Black);
        nombre.setOutlineThickness(1.0f);
        nombre.setPosition({r.position.x + 52.0f, r.position.y + 9.0f});
        ventana.draw(nombre);
    }

    dibujarBotonRect(ventana, fuente, sf::FloatRect({326.0f, 416.0f}, {150.0f, 36.0f}), "Atras",
                     sf::FloatRect({326.0f, 416.0f}, {150.0f, 36.0f}).contains(sf::Vector2f(mouse)), 15);
    dibujarGuiaControlesMenu(ventana, fuente);
}

inline void dibujarPanelLibro(sf::RenderWindow& ventana) {
    sf::RectangleShape sombra({560.0f, 318.0f});
    sombra.setPosition({124.0f, 126.0f});
    sombra.setFillColor(sf::Color(0, 0, 0, 145));
    ventana.draw(sombra);

    sf::RectangleShape panel({560.0f, 318.0f});
    panel.setPosition({120.0f, 120.0f});
    panel.setFillColor(sf::Color(205, 190, 150, 238));
    panel.setOutlineColor(sf::Color(68, 48, 32));
    panel.setOutlineThickness(4.0f);
    ventana.draw(panel);

    sf::RectangleShape division({3.0f, 280.0f});
    division.setPosition({398.0f, 138.0f});
    division.setFillColor(sf::Color(120, 92, 58, 120));
    ventana.draw(division);
}

inline void dibujarComoJugar(sf::RenderWindow& ventana, sf::Font& fuente, float tiempo, int pagina, sf::Vector2i mouse) {
    dibujarPanoramaMenu(ventana, tiempo);
    dibujarTituloSubmenu(ventana, fuente, "Como Jugar");
    dibujarPanelLibro(ventana);

    const char* titulos[3] = {"Movimiento", "Recolectar y Construir", "Noche y Supervivencia"};
    const char* textos[3] = {
        "Usa WASD o las flechas para moverte.\nMantener Ctrl te permite correr.\nEl agua reduce la velocidad y puede hundirte.",
        "Corta arboles para obtener troncos.\nUsa el inventario y la mesa de crafteo\npara fabricar herramientas y bloques.",
        "De noche aparecen zombies.\nCrea refugios, vigila tu vida y usa\nla cama cuando el sistema lo permita."
    };

    int p = std::clamp(pagina, 0, 2);
    sf::Text titulo(fuente, titulos[p], 22);
    titulo.setFillColor(sf::Color(46, 30, 18));
    titulo.setOutlineColor(sf::Color(238, 218, 172));
    titulo.setOutlineThickness(1.0f);
    titulo.setPosition({150.0f, 150.0f});
    ventana.draw(titulo);

    sf::Text cuerpo(fuente, textos[p], 16);
    cuerpo.setFillColor(sf::Color(42, 28, 18));
    cuerpo.setLineSpacing(1.25f);
    cuerpo.setPosition({150.0f, 196.0f});
    ventana.draw(cuerpo);

    sf::RectangleShape icono({170.0f, 170.0f});
    icono.setPosition({454.0f, 172.0f});
    icono.setFillColor(p == 0 ? sf::Color(72, 132, 86) : (p == 1 ? sf::Color(122, 78, 42) : sf::Color(42, 48, 86)));
    icono.setOutlineColor(sf::Color(36, 24, 14));
    icono.setOutlineThickness(3.0f);
    ventana.draw(icono);

    sf::Text numero(fuente, std::to_string(p + 1) + "/3", 26);
    numero.setFillColor(sf::Color::White);
    numero.setOutlineColor(sf::Color::Black);
    numero.setOutlineThickness(2.0f);
    centrarTexto(numero, {539.0f, 257.0f});
    ventana.draw(numero);

    dibujarBotonRect(ventana, fuente, sf::FloatRect({128.0f, 472.0f}, {150.0f, 34.0f}), "Anterior", indiceBotonMenu(mouse) == -99, 15);
    dibujarBotonRect(ventana, fuente, sf::FloatRect({326.0f, 472.0f}, {150.0f, 34.0f}), "Siguiente", false, 15);
    dibujarBotonRect(ventana, fuente, sf::FloatRect({524.0f, 472.0f}, {150.0f, 34.0f}), "Atras", false, 15);
}

inline void dibujarSliderMenu(sf::RenderWindow& ventana, sf::Font& fuente, const char* etiqueta, float y, int valor) {
    sf::Text t(fuente, etiqueta, 16);
    t.setFillColor(sf::Color::White);
    t.setOutlineColor(sf::Color::Black);
    t.setOutlineThickness(2.0f);
    t.setPosition({186.0f, y - 6.0f});
    ventana.draw(t);

    sf::RectangleShape barra({220.0f, 8.0f});
    barra.setPosition({400.0f, y + 8.0f});
    barra.setFillColor(sf::Color(32, 32, 32, 230));
    barra.setOutlineColor(sf::Color(140, 140, 140));
    barra.setOutlineThickness(2.0f);
    ventana.draw(barra);

    sf::RectangleShape lleno({2.2f * static_cast<float>(valor), 8.0f});
    lleno.setPosition({400.0f, y + 8.0f});
    lleno.setFillColor(sf::Color(132, 210, 84));
    ventana.draw(lleno);

    sf::Text porcentaje(fuente, std::to_string(valor) + "%", 14);
    porcentaje.setFillColor(sf::Color::White);
    porcentaje.setOutlineColor(sf::Color::Black);
    porcentaje.setOutlineThickness(2.0f);
    porcentaje.setPosition({636.0f, y - 4.0f});
    ventana.draw(porcentaje);
}

inline void dibujarToggleMenu(sf::RenderWindow& ventana, sf::Font& fuente, const char* etiqueta, float y, bool activo) {
    sf::Text t(fuente, etiqueta, 16);
    t.setFillColor(sf::Color::White);
    t.setOutlineColor(sf::Color::Black);
    t.setOutlineThickness(2.0f);
    t.setPosition({186.0f, y - 6.0f});
    ventana.draw(t);

    dibujarBotonRect(ventana, fuente, sf::FloatRect({500.0f, y - 10.0f}, {120.0f, 32.0f}), activo ? "Activado" : "Desactivado", false, 13);
}

inline void dibujarControles(sf::RenderWindow& ventana, sf::Font& fuente, float tiempo, bool invertirY, int sensibilidad) {
    dibujarPanoramaMenu(ventana, tiempo);
    dibujarTituloSubmenu(ventana, fuente, "Controles");

    sf::RectangleShape panel({580.0f, 330.0f});
    panel.setPosition({110.0f, 126.0f});
    panel.setFillColor(sf::Color(34, 34, 34, 218));
    panel.setOutlineColor(sf::Color(142, 142, 142));
    panel.setOutlineThickness(3.0f);
    ventana.draw(panel);

    sf::Text esquema(fuente,
        "WASD / Flechas: Moverse\n"
        "Mouse Izq: Golpear / Minar\n"
        "Mouse Der: Usar / Colocar\n"
        "Q: Inventario\n"
        "F3: Depuracion\n"
        "B: Dormir si es posible", 16);
    esquema.setFillColor(sf::Color::White);
    esquema.setOutlineColor(sf::Color::Black);
    esquema.setOutlineThickness(2.0f);
    esquema.setPosition({148.0f, 158.0f});
    ventana.draw(esquema);

    dibujarToggleMenu(ventana, fuente, "Invertir eje Y", 332.0f, invertirY);
    dibujarSliderMenu(ventana, fuente, "Sensibilidad de mirada", 382.0f, sensibilidad);
    dibujarBotonRect(ventana, fuente, sf::FloatRect({326.0f, 504.0f}, {150.0f, 34.0f}), "Atras", false, 15);
}

inline void dibujarConfiguracion(sf::RenderWindow& ventana, sf::Font& fuente, float tiempo, int brillo, int musica, int efectos, int autosave, bool nombres) {
    dibujarPanoramaMenu(ventana, tiempo);
    dibujarTituloSubmenu(ventana, fuente, "Configuracion");

    sf::RectangleShape panel({600.0f, 390.0f});
    panel.setPosition({100.0f, 112.0f});
    panel.setFillColor(sf::Color(34, 34, 34, 218));
    panel.setOutlineColor(sf::Color(142, 142, 142));
    panel.setOutlineThickness(3.0f);
    ventana.draw(panel);

    sf::Text g(fuente, "Graficos", 18);
    g.setFillColor(sf::Color(255, 236, 120));
    g.setPosition({130.0f, 132.0f});
    ventana.draw(g);
    dibujarSliderMenu(ventana, fuente, "Brillo", 172.0f, brillo);

    sf::Text a(fuente, "Audio", 18);
    a.setFillColor(sf::Color(255, 236, 120));
    a.setPosition({130.0f, 230.0f});
    ventana.draw(a);
    dibujarSliderMenu(ventana, fuente, "Volumen musica", 270.0f, musica);
    dibujarSliderMenu(ventana, fuente, "Volumen efectos", 320.0f, efectos);

    sf::Text j(fuente, "Juego", 18);
    j.setFillColor(sf::Color(255, 236, 120));
    j.setPosition({130.0f, 366.0f});
    ventana.draw(j);

    const char* autosaveTexto[4] = {"Desactivado", "15 min", "30 min", "1 hora"};
    dibujarBotonRect(ventana, fuente, sf::FloatRect({328.0f, 394.0f}, {130.0f, 30.0f}), autosaveTexto[std::clamp(autosave, 0, 3)], false, 13);
    dibujarToggleMenu(ventana, fuente, "Nombres jugador", 436.0f, nombres);
    dibujarBotonRect(ventana, fuente, sf::FloatRect({626.0f, 532.0f}, {130.0f, 34.0f}), "Atras", false, 15);
}

inline void aplicarBrilloPantalla(sf::RenderWindow& ventana, int brillo) {
    int b = std::clamp(brillo, 0, 100);
    sf::RectangleShape filtro({800.0f, 600.0f});
    filtro.setPosition({0.0f, 0.0f});

    if (b < 75) {
        float t = static_cast<float>(75 - b) / 75.0f;
        filtro.setFillColor(sf::Color(0, 0, 0, static_cast<std::uint8_t>(std::clamp(t * 145.0f, 0.0f, 145.0f))));
        ventana.draw(filtro);
    } else if (b > 75) {
        float t = static_cast<float>(b - 75) / 25.0f;
        filtro.setFillColor(sf::Color(255, 246, 214, static_cast<std::uint8_t>(std::clamp(t * 42.0f, 0.0f, 42.0f))));
        ventana.draw(filtro);
    }
}

inline void dibujarCreditos(sf::RenderWindow& ventana, sf::Font& fuente, float tiempo, float scroll) {
    dibujarPanoramaMenu(ventana, tiempo);
    sf::RectangleShape velo({800.0f, 600.0f});
    velo.setFillColor(sf::Color(0, 0, 0, 190));
    ventana.draw(velo);

    sf::Text creditos(fuente,
        "MINECRAFT 2D TOP-DOWN\n\n"
        "Direccion y diseno\nRaymu\n\n"
        "Programacion\nRaymu + Codex\n\n"
        "Arte temporal\nCute Fantasy Free / Minecraft-like assets\n\n"
        "Agradecimientos\nProfesor, testers y comunidad\n\n"
        "Presiona cualquier tecla para volver", 18);
    creditos.setFillColor(sf::Color::White);
    creditos.setOutlineColor(sf::Color::Black);
    creditos.setOutlineThickness(2.0f);
    sf::FloatRect b = creditos.getLocalBounds();
    creditos.setPosition({400.0f - b.size.x * 0.5f, 620.0f - scroll});
    ventana.draw(creditos);
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
    bool mostrandoMenuInicio = true;
    int pantallaMenuInicio = 0;
    int opcionMenuInicio = 0;
    bool clickMenuAnterior = false;
    float tiempoMenuInicio = 0.0f;
    int paginaComoJugar = 0;
    bool invertirEjeY = false;
    int sensibilidadMirada = 50;
    int brilloMenu = 75;
    int volumenMusica = 70;
    int volumenEfectos = 80;
    int autosaveIndice = 1;
    bool nombresJugador = true;
    float scrollCreditos = 0.0f;
    bool partidaOnline = false;
    bool soloInvitados = false;
    std::string nombreMundoNuevo = "New World";
    std::string semillaMundoNuevo;
    int dificultadMundoNuevo = 2;
    int inputCrearMundoActivo = 0;
    sf::Music musicaMenu;
    bool musicaMenuLista = musicaMenu.openFromFile("assets/audio/menu_music.ogg");
    if (musicaMenuLista) {
        musicaMenu.setLooping(true);
        musicaMenu.setVolume(static_cast<float>(volumenMusica));
        musicaMenu.play();
    } else {
        std::cout << "[Audio] No se pudo cargar assets/audio/menu_music.ogg" << std::endl;
    }
    sf::SoundBuffer bufferClickMenu;
    std::vector<std::int16_t> muestrasClick(2205);
    for (std::size_t i = 0; i < muestrasClick.size(); ++i) {
        float t = static_cast<float>(i) / 44100.0f;
        float envolvente = 1.0f - static_cast<float>(i) / static_cast<float>(muestrasClick.size());
        float onda = std::sin(t * 880.0f * 6.2831853f) * envolvente;
        muestrasClick[i] = static_cast<std::int16_t>(onda * 7600.0f);
    }
    bool clickMenuListo = bufferClickMenu.loadFromSamples(
        muestrasClick.data(),
        muestrasClick.size(),
        1,
        44100,
        std::vector<sf::SoundChannel>{sf::SoundChannel::Mono}
    );
    sf::Sound sonidoClickMenu(bufferClickMenu);
    sonidoClickMenu.setVolume(static_cast<float>(volumenEfectos));
    auto reproducirClickMenu = [&]() {
        if (clickMenuListo && volumenEfectos > 0) {
            sonidoClickMenu.setVolume(static_cast<float>(volumenEfectos));
            sonidoClickMenu.play();
        }
    };
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

    auto iniciarPartidaDesdeMenu = [&]() {
        mostrandoMenuInicio = false;
        if (musicaMenuLista) {
            musicaMenu.stop();
        }
        reloj.restart();
    };

    while (ventana.isOpen() && estaCorriendo) {
        float dt = reloj.restart().asSeconds();
        if (dt > 0.05f) {
            dt = 0.05f;
        }
        tiempoMenuInicio += dt;
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

            if (const auto* texto = evento->getIf<sf::Event::TextEntered>()) {
                if (mostrandoMenuInicio && pantallaMenuInicio == 7 && inputCrearMundoActivo != 0) {
                    char32_t c = texto->unicode;
                    std::string& destino = inputCrearMundoActivo == 1 ? nombreMundoNuevo : semillaMundoNuevo;
                    if (c >= 32 && c <= 126 && destino.size() < 24) {
                        destino.push_back(static_cast<char>(c));
                    }
                }
            }

            if (const auto* botonTeclado = evento->getIf<sf::Event::KeyPressed>()) {
                if (mostrandoMenuInicio) {
                    if (pantallaMenuInicio == 5) {
                        pantallaMenuInicio = 1;
                        scrollCreditos = 0.0f;
                        continue;
                    }

                    if (botonTeclado->code == sf::Keyboard::Key::Escape) {
                        if (pantallaMenuInicio == 0) {
                            ventana.close();
                        } else if (pantallaMenuInicio == 6) {
                            pantallaMenuInicio = 0;
                            opcionMenuInicio = 0;
                        } else if (pantallaMenuInicio == 7 || pantallaMenuInicio == 8) {
                            pantallaMenuInicio = 6;
                            opcionMenuInicio = 0;
                            inputCrearMundoActivo = 0;
                        } else if (pantallaMenuInicio == 1) {
                            pantallaMenuInicio = 0;
                            opcionMenuInicio = 2;
                        } else {
                            pantallaMenuInicio = 1;
                            opcionMenuInicio = 0;
                        }
                        continue;
                    }

                    if (botonTeclado->code == sf::Keyboard::Key::Up ||
                        botonTeclado->code == sf::Keyboard::Key::W) {
                        int total = pantallaMenuInicio == 1 ? 5 : 4;
                        if (pantallaMenuInicio == 6) total = 4;
                        opcionMenuInicio = (opcionMenuInicio + total - 1) % total;
                        reproducirClickMenu();
                    }
                    if (botonTeclado->code == sf::Keyboard::Key::Down ||
                        botonTeclado->code == sf::Keyboard::Key::S) {
                        int total = pantallaMenuInicio == 1 ? 5 : 4;
                        if (pantallaMenuInicio == 6) total = 4;
                        opcionMenuInicio = (opcionMenuInicio + 1) % total;
                        reproducirClickMenu();
                    }
                    if (pantallaMenuInicio == 7 && botonTeclado->code == sf::Keyboard::Key::Backspace) {
                        std::string& destino = inputCrearMundoActivo == 1 ? nombreMundoNuevo : semillaMundoNuevo;
                        if (inputCrearMundoActivo != 0 && !destino.empty()) {
                            destino.pop_back();
                        }
                    }
                    if (botonTeclado->code == sf::Keyboard::Key::Enter ||
                        botonTeclado->code == sf::Keyboard::Key::Space) {
                        reproducirClickMenu();
                        if (pantallaMenuInicio == 0) {
                            if (opcionMenuInicio == 0) {
                                pantallaMenuInicio = 6;
                                opcionMenuInicio = 0;
                            } else if (opcionMenuInicio == 2) {
                                pantallaMenuInicio = 1;
                                opcionMenuInicio = 0;
                            } else if (opcionMenuInicio == 3) {
                                ventana.close();
                            }
                        } else if (pantallaMenuInicio == 1) {
                            if (opcionMenuInicio == 0) pantallaMenuInicio = 2;
                            if (opcionMenuInicio == 1) pantallaMenuInicio = 3;
                            if (opcionMenuInicio == 2) pantallaMenuInicio = 4;
                            if (opcionMenuInicio == 3) {
                                pantallaMenuInicio = 5;
                                scrollCreditos = 0.0f;
                            }
                            if (opcionMenuInicio == 4) {
                                pantallaMenuInicio = 0;
                                opcionMenuInicio = 2;
                            }
                        } else if (pantallaMenuInicio == 6) {
                            if (opcionMenuInicio == 0) pantallaMenuInicio = 7;
                            if (opcionMenuInicio == 1 || opcionMenuInicio == 2) iniciarPartidaDesdeMenu();
                            if (opcionMenuInicio == 3) {
                                pantallaMenuInicio = 0;
                                opcionMenuInicio = 0;
                            }
                        } else if (pantallaMenuInicio == 7) {
                            if (inputCrearMundoActivo == 0) {
                                iniciarPartidaDesdeMenu();
                            }
                        } else if (pantallaMenuInicio == 8) {
                            pantallaMenuInicio = 6;
                        }
                    }
                    continue;
                }

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

        if (mostrandoMenuInicio) {
            auto mouseDentro = [&](sf::FloatRect rect) {
                return rect.contains(sf::Vector2f(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)));
            };
            auto valorSlider = [&](float xInicio) {
                float t = (static_cast<float>(mousePos.x) - xInicio) / 220.0f;
                return std::clamp(static_cast<int>(std::round(t * 100.0f)), 0, 100);
            };

            if (pantallaMenuInicio == 0) {
                int hover = indiceBotonMenu(mousePos);
                if (hover >= 0) {
                    opcionMenuInicio = hover;
                }
                if (clickIzquierdo && !clickMenuAnterior && hover >= 0) {
                    reproducirClickMenu();
                    if (hover == 0) {
                        pantallaMenuInicio = 6;
                        opcionMenuInicio = 0;
                    } else if (hover == 2) {
                        pantallaMenuInicio = 1;
                        opcionMenuInicio = 0;
                    } else if (hover == 3) {
                        ventana.close();
                    }
                }
            } else if (pantallaMenuInicio == 1) {
                int hover = indiceListaMenu(mousePos, 5, 190.0f);
                if (hover >= 0) {
                    opcionMenuInicio = hover;
                }
                if (clickIzquierdo && !clickMenuAnterior && hover >= 0) {
                    reproducirClickMenu();
                    if (hover == 0) pantallaMenuInicio = 2;
                    if (hover == 1) pantallaMenuInicio = 3;
                    if (hover == 2) pantallaMenuInicio = 4;
                    if (hover == 3) {
                        pantallaMenuInicio = 5;
                        scrollCreditos = 0.0f;
                    }
                    if (hover == 4) {
                        pantallaMenuInicio = 0;
                        opcionMenuInicio = 2;
                    }
                }
            } else if (pantallaMenuInicio == 2 && clickIzquierdo && !clickMenuAnterior) {
                if (mouseDentro(sf::FloatRect({128.0f, 472.0f}, {150.0f, 34.0f}))) {
                    reproducirClickMenu();
                    paginaComoJugar = std::max(0, paginaComoJugar - 1);
                } else if (mouseDentro(sf::FloatRect({326.0f, 472.0f}, {150.0f, 34.0f}))) {
                    reproducirClickMenu();
                    paginaComoJugar = std::min(2, paginaComoJugar + 1);
                } else if (mouseDentro(sf::FloatRect({524.0f, 472.0f}, {150.0f, 34.0f}))) {
                    reproducirClickMenu();
                    pantallaMenuInicio = 1;
                    opcionMenuInicio = 0;
                }
            } else if (pantallaMenuInicio == 3) {
                if (clickIzquierdo) {
                    if (mouseDentro(sf::FloatRect({500.0f, 322.0f}, {120.0f, 32.0f})) && !clickMenuAnterior) {
                        reproducirClickMenu();
                        invertirEjeY = !invertirEjeY;
                    }
                    if (mouseDentro(sf::FloatRect({400.0f, 390.0f}, {220.0f, 18.0f}))) {
                        sensibilidadMirada = valorSlider(400.0f);
                    }
                    if (mouseDentro(sf::FloatRect({326.0f, 504.0f}, {150.0f, 34.0f})) && !clickMenuAnterior) {
                        reproducirClickMenu();
                        pantallaMenuInicio = 1;
                        opcionMenuInicio = 1;
                    }
                }
            } else if (pantallaMenuInicio == 4) {
                if (clickIzquierdo) {
                    if (mouseDentro(sf::FloatRect({400.0f, 180.0f}, {220.0f, 18.0f}))) brilloMenu = valorSlider(400.0f);
                    if (mouseDentro(sf::FloatRect({400.0f, 278.0f}, {220.0f, 18.0f}))) {
                        volumenMusica = valorSlider(400.0f);
                        if (musicaMenuLista) {
                            musicaMenu.setVolume(static_cast<float>(volumenMusica));
                        }
                    }
                    if (mouseDentro(sf::FloatRect({400.0f, 328.0f}, {220.0f, 18.0f}))) {
                        volumenEfectos = valorSlider(400.0f);
                        sonidoClickMenu.setVolume(static_cast<float>(volumenEfectos));
                    }
                    if (mouseDentro(sf::FloatRect({328.0f, 394.0f}, {130.0f, 30.0f})) && !clickMenuAnterior) {
                        reproducirClickMenu();
                        autosaveIndice = (autosaveIndice + 1) % 4;
                    }
                    if (mouseDentro(sf::FloatRect({500.0f, 426.0f}, {120.0f, 32.0f})) && !clickMenuAnterior) {
                        reproducirClickMenu();
                        nombresJugador = !nombresJugador;
                    }
                    if (mouseDentro(sf::FloatRect({626.0f, 532.0f}, {130.0f, 34.0f})) && !clickMenuAnterior) {
                        reproducirClickMenu();
                        pantallaMenuInicio = 1;
                        opcionMenuInicio = 2;
                    }
                }
            } else if (pantallaMenuInicio == 5) {
                scrollCreditos += dt * 36.0f;
                if (clickIzquierdo && !clickMenuAnterior) {
                    pantallaMenuInicio = 1;
                    scrollCreditos = 0.0f;
                    opcionMenuInicio = 3;
                }
            } else if (pantallaMenuInicio == 6) {
                if (clickIzquierdo && !clickMenuAnterior) {
                    if (mouseDentro(sf::FloatRect({82.0f, 134.0f}, {220.0f, 28.0f}))) {
                        reproducirClickMenu();
                        partidaOnline = !partidaOnline;
                    } else if (mouseDentro(sf::FloatRect({82.0f, 212.0f}, {284.0f, 34.0f}))) {
                        reproducirClickMenu();
                        pantallaMenuInicio = 7;
                        inputCrearMundoActivo = 0;
                    } else if (mouseDentro(sf::FloatRect({82.0f, 254.0f}, {284.0f, 34.0f}))) {
                        reproducirClickMenu();
                        iniciarPartidaDesdeMenu();
                    } else if (mouseDentro(sf::FloatRect({446.0f, 216.0f}, {222.0f, 54.0f}))) {
                        reproducirClickMenu();
                        pantallaMenuInicio = 8;
                    } else if (mouseDentro(sf::FloatRect({576.0f, 482.0f}, {124.0f, 34.0f}))) {
                        reproducirClickMenu();
                        pantallaMenuInicio = 0;
                        opcionMenuInicio = 0;
                    } else {
                        for (int i = 0; i < 3; ++i) {
                            sf::FloatRect mundo({82.0f, 332.0f + static_cast<float>(i) * 48.0f}, {284.0f, 40.0f});
                            if (mouseDentro(mundo)) {
                                reproducirClickMenu();
                                iniciarPartidaDesdeMenu();
                            }
                        }
                    }
                }
            } else if (pantallaMenuInicio == 7) {
                if (clickIzquierdo && !clickMenuAnterior) {
                    if (mouseDentro(sf::FloatRect({176.0f, 132.0f}, {180.0f, 28.0f}))) {
                        reproducirClickMenu();
                        partidaOnline = !partidaOnline;
                        if (!partidaOnline) soloInvitados = false;
                    } else if (mouseDentro(sf::FloatRect({410.0f, 132.0f}, {190.0f, 28.0f})) && partidaOnline) {
                        reproducirClickMenu();
                        soloInvitados = !soloInvitados;
                    } else if (mouseDentro(sf::FloatRect({176.0f, 202.0f}, {448.0f, 40.0f}))) {
                        reproducirClickMenu();
                        inputCrearMundoActivo = 1;
                    } else if (mouseDentro(sf::FloatRect({176.0f, 286.0f}, {448.0f, 40.0f}))) {
                        reproducirClickMenu();
                        inputCrearMundoActivo = 2;
                    } else if (mouseDentro(sf::FloatRect({400.0f, 348.0f}, {224.0f, 34.0f}))) {
                        reproducirClickMenu();
                        dificultadMundoNuevo = (dificultadMundoNuevo + 1) % 4;
                        inputCrearMundoActivo = 0;
                    } else if (mouseDentro(sf::FloatRect({176.0f, 440.0f}, {288.0f, 40.0f}))) {
                        reproducirClickMenu();
                        if (nombreMundoNuevo.empty()) nombreMundoNuevo = "New World";
                        iniciarPartidaDesdeMenu();
                    } else if (mouseDentro(sf::FloatRect({500.0f, 440.0f}, {124.0f, 40.0f}))) {
                        reproducirClickMenu();
                        pantallaMenuInicio = 6;
                        inputCrearMundoActivo = 0;
                    } else {
                        inputCrearMundoActivo = 0;
                    }
                }
            } else if (pantallaMenuInicio == 8) {
                if (clickIzquierdo && !clickMenuAnterior && mouseDentro(sf::FloatRect({326.0f, 416.0f}, {150.0f, 36.0f}))) {
                    reproducirClickMenu();
                    pantallaMenuInicio = 6;
                    opcionMenuInicio = 0;
                }
            }
            clickMenuAnterior = clickIzquierdo;

            ventana.clear(sf::Color(18, 18, 22));
            if (pantallaMenuInicio == 0) {
                dibujarMenuInicio(ventana, fuente, fuenteCargada, tiempoMenuInicio, opcionMenuInicio, mousePos);
            } else if (fuenteCargada && pantallaMenuInicio == 1) {
                dibujarAyudaOpciones(ventana, fuente, tiempoMenuInicio, opcionMenuInicio, mousePos);
            } else if (fuenteCargada && pantallaMenuInicio == 2) {
                dibujarComoJugar(ventana, fuente, tiempoMenuInicio, paginaComoJugar, mousePos);
            } else if (fuenteCargada && pantallaMenuInicio == 3) {
                dibujarControles(ventana, fuente, tiempoMenuInicio, invertirEjeY, sensibilidadMirada);
            } else if (fuenteCargada && pantallaMenuInicio == 4) {
                dibujarConfiguracion(ventana, fuente, tiempoMenuInicio, brilloMenu, volumenMusica, volumenEfectos, autosaveIndice, nombresJugador);
            } else if (fuenteCargada && pantallaMenuInicio == 5) {
                dibujarCreditos(ventana, fuente, tiempoMenuInicio, scrollCreditos);
            } else if (fuenteCargada && pantallaMenuInicio == 6) {
                dibujarMenuJugar(ventana, fuente, tiempoMenuInicio, partidaOnline, mousePos);
            } else if (fuenteCargada && pantallaMenuInicio == 7) {
                dibujarCrearMundo(
                    ventana,
                    fuente,
                    tiempoMenuInicio,
                    partidaOnline,
                    soloInvitados,
                    nombreMundoNuevo,
                    semillaMundoNuevo,
                    dificultadMundoNuevo,
                    inputCrearMundoActivo,
                    mousePos
                );
            } else if (fuenteCargada && pantallaMenuInicio == 8) {
                dibujarUnirsePartida(ventana, fuente, tiempoMenuInicio, mousePos);
            }
            aplicarBrilloPantalla(ventana, brilloMenu);
            ventana.display();
            continue;
        }

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
                if (mapaSuperficie) {
                    mapaSuperficie->dibujarArbolesSobreJugador(ventana, posAnimal.y + 28.0f);
                }
            }
        }

        for (auto* zombie : zombis) {
            if (!zombie) continue;
            sf::Vector2f posZombie = zombie->getPosicion();
            if (posZombie.x >= dibujoIzq && posZombie.x <= dibujoDer &&
                posZombie.y >= dibujoArriba && posZombie.y <= dibujoAbajo) {
                zombie->dibujar(ventana);
                if (mapaSuperficie) {
                    mapaSuperficie->dibujarArbolesSobreJugador(ventana, posZombie.y + 24.0f);
                }
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

            if (item.item == ItemId::BloqueTronco) {
                dibujarTroncoSuelo(ventana, item.posicion);
            } else {
                sf::RectangleShape icono({12.0f, 9.0f});
                icono.setOrigin({6.0f, 4.5f});
                icono.setPosition(item.posicion);
                icono.setFillColor(colorItemSuelo(item.item));
                icono.setOutlineColor(sf::Color(70, 35, 35));
                icono.setOutlineThickness(1.0f);
                ventana.draw(icono);
            }

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

        aplicarBrilloPantalla(ventana, brilloMenu);
        ventana.display();
        clickIzquierdoAnterior = clickIzquierdo;
        clickDerechoAnterior = clickDerecho;
    }
}


