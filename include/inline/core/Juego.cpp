#include <iostream>
#include <random>
#include <sstream>
#include <cmath>
#include <cstdint>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <SFML/Audio.hpp>
#include "../../sistemas/SistemaHerramientas.hpp"
#include "../../ui/InventarioGrid.hpp"

namespace {
constexpr float TICKS_POR_SEGUNDO_MUNDO = 216.67f;
constexpr float TICKS_DIA_COMPLETO = 24000.0f;
constexpr float TICK_FIN_DIA = 12000.0f;
constexpr float TICK_INICIO_NOCHE = 13000.0f;
constexpr float TICK_FIN_NOCHE = 22000.0f;
constexpr float TICK_PUEDE_DORMIR = 12542.0f;

struct MundoGuardado {
    std::string nombre;
    std::string carpeta;
    std::string ultimaVez;
    std::string semillaTexto;
    unsigned int semilla = 0;
    int dificultad = 2;
};

inline std::string dificultadTexto(int dificultad) {
    static const char* nombres[4] = {"Pacifico", "Facil", "Normal", "Dificil"};
    return nombres[std::clamp(dificultad, 0, 3)];
}

inline std::string limpiarNombreArchivo(std::string texto) {
    if (texto.empty()) {
        texto = "Nuevo Mundo";
    }

    for (char& c : texto) {
        bool valido = (c >= '0' && c <= '9') ||
                      (c >= 'A' && c <= 'Z') ||
                      (c >= 'a' && c <= 'z') ||
                      c == ' ' || c == '_' || c == '-';
        if (!valido) {
            c = '_';
        }
    }
    while (!texto.empty() && texto.back() == ' ') texto.pop_back();
    if (texto.empty()) {
        texto = "Nuevo Mundo";
    }
    return texto;
}

inline unsigned int semillaDesdeTexto(const std::string& entrada) {
    if (entrada.empty()) {
        auto ahora = std::chrono::system_clock::now().time_since_epoch().count();
        return static_cast<unsigned int>((static_cast<std::uint64_t>(ahora) >> 7u) ^ static_cast<std::uint64_t>(ahora));
    }

    bool numerica = true;
    std::size_t inicio = (entrada[0] == '-' || entrada[0] == '+') ? 1u : 0u;
    if (inicio >= entrada.size()) {
        numerica = false;
    }
    for (std::size_t i = inicio; i < entrada.size(); ++i) {
        if (entrada[i] < '0' || entrada[i] > '9') {
            numerica = false;
            break;
        }
    }

    if (numerica) {
        try {
            long long valor = std::stoll(entrada);
            return static_cast<unsigned int>(valor);
        } catch (...) {
        }
    }

    std::uint32_t hash = 2166136261u;
    for (unsigned char c : entrada) {
        hash ^= c;
        hash *= 16777619u;
    }
    return hash;
}

inline std::string fechaActualTexto() {
    auto ahora = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(ahora);
    std::tm tmLocal{};
#ifdef _WIN32
    localtime_s(&tmLocal, &t);
#else
    localtime_r(&t, &tmLocal);
#endif
    std::ostringstream ss;
    ss << std::put_time(&tmLocal, "%Y-%m-%d %H:%M");
    return ss.str();
}

inline std::filesystem::path carpetaSaves() {
    return std::filesystem::path("saves");
}

inline std::filesystem::path rutaMetaMundo(const std::string& carpeta) {
    return carpetaSaves() / carpeta / "world.meta";
}

inline std::filesystem::path rutaBloquesMundo(const std::string& carpeta) {
    return carpetaSaves() / carpeta / "blocks.bin";
}

inline std::filesystem::path rutaInventarioMundo(const std::string& carpeta) {
    return carpetaSaves() / carpeta / "inventory.txt";
}

inline std::filesystem::path rutaJugadorMundo(const std::string& carpeta) {
    return carpetaSaves() / carpeta / "player.txt";
}

inline MundoGuardado leerMetaMundo(const std::filesystem::path& ruta) {
    MundoGuardado mundo;
    mundo.carpeta = ruta.parent_path().filename().string();
    std::ifstream in(ruta);
    std::string linea;
    while (std::getline(in, linea)) {
        std::size_t eq = linea.find('=');
        if (eq == std::string::npos) continue;
        std::string clave = linea.substr(0, eq);
        std::string valor = linea.substr(eq + 1);
        if (clave == "name") mundo.nombre = valor;
        if (clave == "seedText") mundo.semillaTexto = valor;
        if (clave == "seed") mundo.semilla = static_cast<unsigned int>(std::stoul(valor));
        if (clave == "difficulty") mundo.dificultad = std::stoi(valor);
        if (clave == "lastPlayed") mundo.ultimaVez = valor;
    }
    if (mundo.nombre.empty()) mundo.nombre = mundo.carpeta;
    if (mundo.ultimaVez.empty()) mundo.ultimaVez = "Sin fecha";
    return mundo;
}

inline std::vector<MundoGuardado> escanearMundosGuardados() {
    std::vector<MundoGuardado> mundos;
    std::filesystem::create_directories(carpetaSaves());
    for (const auto& entrada : std::filesystem::directory_iterator(carpetaSaves())) {
        if (!entrada.is_directory()) continue;
        std::filesystem::path meta = entrada.path() / "world.meta";
        if (std::filesystem::exists(meta)) {
            mundos.push_back(leerMetaMundo(meta));
        }
    }
    std::sort(mundos.begin(), mundos.end(), [](const MundoGuardado& a, const MundoGuardado& b) {
        return a.ultimaVez > b.ultimaVez;
    });
    return mundos;
}

inline void guardarMetaMundo(const MundoGuardado& mundo) {
    std::filesystem::create_directories(carpetaSaves() / mundo.carpeta);
    std::ofstream out(rutaMetaMundo(mundo.carpeta));
    out << "name=" << mundo.nombre << "\n";
    out << "seedText=" << mundo.semillaTexto << "\n";
    out << "seed=" << mundo.semilla << "\n";
    out << "difficulty=" << mundo.dificultad << "\n";
    out << "lastPlayed=" << mundo.ultimaVez << "\n";
}

inline std::string carpetaUnicaMundo(const std::string& nombre) {
    std::string base = limpiarNombreArchivo(nombre);
    std::string carpeta = base;
    int sufijo = 2;
    while (std::filesystem::exists(carpetaSaves() / carpeta)) {
        carpeta = base + " " + std::to_string(sufijo++);
    }
    return carpeta;
}

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

inline void dibujarSelectorBloque(sf::RenderWindow& ventana, int bloqueX, int bloqueY, bool dentroRango, float tiempo) {
    if (bloqueX < 0 || bloqueY < 0) {
        return;
    }

    float x = static_cast<float>(bloqueX) * TAMANIO_BLOQUE_JUEGO;
    float y = static_cast<float>(bloqueY) * TAMANIO_BLOQUE_JUEGO;
    float pulso = 0.55f + 0.45f * (0.5f + 0.5f * std::sin(tiempo * 7.0f));
    sf::Color color = dentroRango
        ? sf::Color(245, 245, 245, static_cast<std::uint8_t>(115 + 70 * pulso))
        : sf::Color(255, 80, 70, 115);

    sf::RectangleShape relleno({TAMANIO_BLOQUE_JUEGO, TAMANIO_BLOQUE_JUEGO});
    relleno.setPosition({x, y});
    relleno.setFillColor(dentroRango ? sf::Color(255, 255, 255, 24) : sf::Color(255, 60, 50, 22));
    ventana.draw(relleno);

    sf::RectangleShape borde({TAMANIO_BLOQUE_JUEGO, TAMANIO_BLOQUE_JUEGO});
    borde.setPosition({x, y});
    borde.setFillColor(sf::Color::Transparent);
    borde.setOutlineThickness(1.4f);
    borde.setOutlineColor(color);
    ventana.draw(borde);

    const float esquina = 6.0f;
    const float grosor = 2.0f;
    auto pieza = [&](float px, float py, float w, float h) {
        sf::RectangleShape r({w, h});
        r.setPosition({x + px, y + py});
        r.setFillColor(color);
        ventana.draw(r);
    };

    pieza(0.0f, 0.0f, esquina, grosor);
    pieza(0.0f, 0.0f, grosor, esquina);
    pieza(TAMANIO_BLOQUE_JUEGO - esquina, 0.0f, esquina, grosor);
    pieza(TAMANIO_BLOQUE_JUEGO - grosor, 0.0f, grosor, esquina);
    pieza(0.0f, TAMANIO_BLOQUE_JUEGO - grosor, esquina, grosor);
    pieza(0.0f, TAMANIO_BLOQUE_JUEGO - esquina, grosor, esquina);
    pieza(TAMANIO_BLOQUE_JUEGO - esquina, TAMANIO_BLOQUE_JUEGO - grosor, esquina, grosor);
    pieza(TAMANIO_BLOQUE_JUEGO - grosor, TAMANIO_BLOQUE_JUEGO - esquina, grosor, esquina);
}

inline void dibujarPixelMundo(sf::RenderWindow& ventana, sf::Vector2f origen, int x, int y, sf::Color color, float escala) {
    sf::RectangleShape pixel({escala, escala});
    pixel.setPosition({origen.x + static_cast<float>(x) * escala, origen.y + static_cast<float>(y) * escala});
    pixel.setFillColor(color);
    ventana.draw(pixel);
}

inline void dibujarTroncoSuelo(sf::RenderWindow& ventana, sf::Vector2f centro, float escalaVisual = 1.0f, float giroY = 1.0f) {
    const float escala = escalaVisual;
    const float anchoVisual = std::max(0.24f, giroY);
    sf::Vector2f origen(centro.x - 8.0f * escala * anchoVisual, centro.y - 8.0f * escala);
    sf::Color borde(20, 13, 11);
    sf::Color madera(112, 68, 48);
    sf::Color maderaLuz(150, 95, 66);
    sf::Color maderaOscura(64, 38, 32);
    sf::Color corte(178, 128, 77);
    sf::Color corteOscuro(105, 70, 44);
    sf::Color cuerda(190, 144, 86);

    auto pixel = [&](int x, int y, sf::Color color) {
        sf::RectangleShape px({escala * anchoVisual, escala});
        px.setPosition({origen.x + static_cast<float>(x) * escala * anchoVisual, origen.y + static_cast<float>(y) * escala});
        px.setFillColor(color);
        ventana.draw(px);
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

inline void dibujarPixelItemSuelo(
    sf::RenderWindow& ventana,
    sf::Vector2f origen,
    int x,
    int y,
    sf::Color color,
    float escala,
    float giroY
) {
    if (color.a == 0) return;
    float ancho = std::max(0.24f, giroY);
    sf::RectangleShape pixel({escala * ancho, escala});
    pixel.setPosition({origen.x + static_cast<float>(x) * escala * ancho, origen.y + static_cast<float>(y) * escala});
    pixel.setFillColor(color);
    ventana.draw(pixel);
}

inline sf::Color colorPixelTierraItemSuelo(int x, int y) {
    static const char* patron[16] = {
        "bbcbbdabbcabbdcb",
        "bDaabbccbaadbbab",
        "abbcCbbdabccadbb",
        "cbbadbbcaabbcDab",
        "bbccabbdabbcbaac",
        "adbbcaabDcbbadbb",
        "bbadbbcabbccabba",
        "cabbadbbaacbbdCb",
        "bbccbaadbCbbcaab",
        "adbbcabbccabbdba",
        "bbadCbbcaabbdabb",
        "cabbccadbbaacbba",
        "bbdabbcCbbadbbca",
        "aabbcDabbccbaadb",
        "bbcabbadbbcaabCb",
        "cbaadbbccabbadbb"
    };

    switch (patron[y][x]) {
        case 'a': return sf::Color(104, 67, 38);
        case 'b': return sf::Color(126, 82, 46);
        case 'c': return sf::Color(151, 99, 55);
        case 'd': return sf::Color(82, 52, 31);
        case 'C': return sf::Color(172, 114, 64);
        case 'D': return sf::Color(57, 36, 24);
        default: return sf::Color(126, 82, 46);
    }
}

inline void dibujarBloqueTierraItemSuelo(sf::RenderWindow& ventana, sf::Vector2f origen, float escala, float giroY) {
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            dibujarPixelItemSuelo(ventana, origen, x, y, colorPixelTierraItemSuelo(x, y), escala, giroY);
        }
    }
}

inline void lineaItemSuelo(
    sf::RenderWindow& ventana,
    sf::Vector2f origen,
    int x1,
    int y1,
    int x2,
    int y2,
    sf::Color color,
    float escala,
    float giroY
) {
    int dx = std::abs(x2 - x1);
    int dy = -std::abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx + dy;
    int x = x1;
    int y = y1;

    while (true) {
        dibujarPixelItemSuelo(ventana, origen, x, y, color, escala, giroY);
        if (x == x2 && y == y2) break;
        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y += sy;
        }
    }
}

inline sf::Color colorBloqueItem(ItemId item) {
    switch (item) {
        case ItemId::BloquePasto: return sf::Color(58, 150, 65);
        case ItemId::BloqueTierra: return sf::Color(122, 78, 45);
        case ItemId::BloquePiedra: return sf::Color(126, 126, 126);
        case ItemId::TablonMadera: return sf::Color(182, 126, 62);
        case ItemId::MineralHierro: return sf::Color(136, 126, 116);
        case ItemId::MineralPlata: return sf::Color(176, 184, 188);
        case ItemId::MineralOro: return sf::Color(191, 148, 55);
        case ItemId::MineralDiamante: return sf::Color(70, 210, 225);
        case ItemId::Lana:
        case ItemId::LanaCruda: return sf::Color(226, 226, 218);
        case ItemId::MesaCrafteo: return sf::Color(128, 82, 42);
        case ItemId::Horno: return sf::Color(86, 86, 86);
        case ItemId::Cama: return sf::Color(202, 66, 62);
        default: return sf::Color(210, 180, 110);
    }
}

inline void dibujarBloqueItemSuelo(sf::RenderWindow& ventana, sf::Vector2f origen, ItemId item, float escala, float giroY) {
    sf::Color base = colorBloqueItem(item);
    sf::Color luz(std::min(255, base.r + 36), std::min(255, base.g + 36), std::min(255, base.b + 36), base.a);
    sf::Color sombra(base.r / 2, base.g / 2, base.b / 2, base.a);
    sf::Color borde(34, 28, 24);

    for (int y = 3; y < 13; ++y) {
        for (int x = 3; x < 13; ++x) {
            sf::Color c = base;
            if ((x * 7 + y * 5) % 11 == 0) c = luz;
            if ((x * 13 + y * 3) % 17 == 0) c = sombra;
            if (item == ItemId::BloquePasto && y <= 5) c = ((x + y) % 3 == 0) ? sf::Color(72, 176, 73) : sf::Color(48, 136, 58);
            if (item == ItemId::TablonMadera && y % 3 == 0) c = sf::Color(112, 70, 36);
            if ((item == ItemId::MineralHierro || item == ItemId::MineralPlata || item == ItemId::MineralOro || item == ItemId::MineralDiamante) &&
                ((x == 5 && y == 6) || (x == 9 && y == 9) || (x == 7 && y == 11))) {
                c = colorBloqueItem(item);
            }
            dibujarPixelItemSuelo(ventana, origen, x, y, c, escala, giroY);
        }
    }

    for (int i = 3; i < 13; ++i) {
        dibujarPixelItemSuelo(ventana, origen, i, 3, borde, escala, giroY);
        dibujarPixelItemSuelo(ventana, origen, i, 12, sombra, escala, giroY);
        dibujarPixelItemSuelo(ventana, origen, 3, i, borde, escala, giroY);
        dibujarPixelItemSuelo(ventana, origen, 12, i, sombra, escala, giroY);
    }
}

inline void dibujarComidaItemSuelo(sf::RenderWindow& ventana, sf::Vector2f origen, ItemId item, float escala, float giroY) {
    sf::Color carne(218, 102, 110);
    if (item == ItemId::ChuletaCerdoCocinada) carne = sf::Color(135, 76, 43);
    if (item == ItemId::CarneResCruda) carne = sf::Color(178, 58, 66);
    if (item == ItemId::PolloCrudo) carne = sf::Color(232, 178, 156);
    sf::Color borde(carne.r / 2, carne.g / 2, carne.b / 2);
    sf::Color brillo(std::min(255, carne.r + 35), std::min(255, carne.g + 35), std::min(255, carne.b + 35));

    for (int y = 5; y < 13; ++y) {
        for (int x = 4; x < 13; ++x) {
            if (!((x == 4 || x == 12) && (y == 5 || y == 12))) {
                dibujarPixelItemSuelo(ventana, origen, x, y, carne, escala, giroY);
            }
        }
    }
    dibujarPixelItemSuelo(ventana, origen, 4, 8, borde, escala, giroY);
    dibujarPixelItemSuelo(ventana, origen, 5, 12, borde, escala, giroY);
    dibujarPixelItemSuelo(ventana, origen, 11, 6, brillo, escala, giroY);
    dibujarPixelItemSuelo(ventana, origen, 10, 7, brillo, escala, giroY);
}

inline void dibujarItemSueloSprite(sf::RenderWindow& ventana, ItemId item, sf::Vector2f centro, float escalaVisual, float giroY) {
    float ancho = std::max(0.24f, giroY);
    float escala = escalaVisual;
    sf::Vector2f origen(centro.x - 8.0f * escala * ancho, centro.y - 8.0f * escala);

    if (item == ItemId::BloqueTronco) {
        dibujarTroncoSuelo(ventana, centro, escalaVisual, giroY);
        return;
    }

    switch (item) {
        case ItemId::BloquePasto:
        case ItemId::BloquePiedra:
        case ItemId::TablonMadera:
        case ItemId::MineralHierro:
        case ItemId::MineralPlata:
        case ItemId::MineralOro:
        case ItemId::MineralDiamante:
        case ItemId::Lana:
        case ItemId::LanaCruda:
        case ItemId::MesaCrafteo:
        case ItemId::Horno:
        case ItemId::Cama:
            dibujarBloqueItemSuelo(ventana, origen, item, escala, giroY);
            return;
        case ItemId::BloqueTierra:
            dibujarBloqueTierraItemSuelo(ventana, origen, escala, giroY);
            return;
        case ItemId::ChuletaCerdoCruda:
        case ItemId::ChuletaCerdoCocinada:
        case ItemId::CarneResCruda:
        case ItemId::PolloCrudo:
            dibujarComidaItemSuelo(ventana, origen, item, escala, giroY);
            return;
        case ItemId::SemillaArbol:
            dibujarPixelItemSuelo(ventana, origen, 8, 11, sf::Color(68, 110, 38), escala, giroY);
            dibujarPixelItemSuelo(ventana, origen, 7, 10, sf::Color(68, 110, 38), escala, giroY);
            dibujarPixelItemSuelo(ventana, origen, 8, 9, sf::Color(42, 145, 58), escala, giroY);
            dibujarPixelItemSuelo(ventana, origen, 9, 8, sf::Color(70, 180, 76), escala, giroY);
            dibujarPixelItemSuelo(ventana, origen, 7, 8, sf::Color(56, 156, 66), escala, giroY);
            dibujarPixelItemSuelo(ventana, origen, 9, 10, sf::Color(52, 137, 56), escala, giroY);
            return;
        case ItemId::Pluma:
            lineaItemSuelo(ventana, origen, 5, 12, 11, 4, sf::Color(220, 220, 210), escala, giroY);
            lineaItemSuelo(ventana, origen, 6, 12, 12, 4, sf::Color(104, 104, 112), escala, giroY);
            dibujarPixelItemSuelo(ventana, origen, 7, 8, sf::Color(245, 245, 236), escala, giroY);
            dibujarPixelItemSuelo(ventana, origen, 8, 7, sf::Color(245, 245, 236), escala, giroY);
            return;
        case ItemId::PaloMadera:
            lineaItemSuelo(ventana, origen, 5, 13, 11, 4, sf::Color(150, 88, 36), escala, giroY);
            lineaItemSuelo(ventana, origen, 6, 13, 12, 4, sf::Color(82, 48, 24), escala, giroY);
            return;
        case ItemId::Zanahoria:
            for (int y = 6; y < 14; ++y) {
                int anchoZ = y < 9 ? 4 : (y < 12 ? 3 : 2);
                for (int x = 7; x < 7 + anchoZ; ++x) {
                    dibujarPixelItemSuelo(ventana, origen, x, y, sf::Color(232, 112, 35), escala, giroY);
                }
            }
            dibujarPixelItemSuelo(ventana, origen, 7, 5, sf::Color(62, 158, 62), escala, giroY);
            dibujarPixelItemSuelo(ventana, origen, 9, 4, sf::Color(62, 158, 62), escala, giroY);
            return;
        default:
            dibujarBloqueItemSuelo(ventana, origen, item, escala, giroY);
            return;
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

inline void dibujarMenuJugar(sf::RenderWindow& ventana, sf::Font& fuente, float tiempo, bool online,
                             const std::vector<MundoGuardado>& mundosGuardados, sf::Vector2i mouse) {
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

    int visibles = std::min<int>(3, mundosGuardados.size());
    for (int i = 0; i < visibles; ++i) {
        const MundoGuardado& mundo = mundosGuardados[i];
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

        sf::Text nombre(fuente, mundo.nombre, 13);
        nombre.setFillColor(sf::Color::White);
        nombre.setOutlineColor(sf::Color::Black);
        nombre.setOutlineThickness(1.0f);
        nombre.setPosition({r.position.x + 46.0f, r.position.y + 5.0f});
        ventana.draw(nombre);

        std::string detalleMundo = mundo.ultimaVez + " - " + dificultadTexto(mundo.dificultad);
        sf::Text seed(fuente, detalleMundo, 11);
        seed.setFillColor(sf::Color(190, 190, 190));
        seed.setOutlineColor(sf::Color::Black);
        seed.setOutlineThickness(1.0f);
        seed.setPosition({r.position.x + 46.0f, r.position.y + 23.0f});
        ventana.draw(seed);
    }

    if (mundosGuardados.empty()) {
        sf::Text vacio(fuente, "No hay mundos guardados todavia", 12);
        vacio.setFillColor(sf::Color(190, 190, 190));
        vacio.setOutlineColor(sf::Color::Black);
        vacio.setOutlineThickness(1.0f);
        vacio.setPosition({86.0f, 336.0f});
        ventana.draw(vacio);
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

inline void dibujarFondoPausa(sf::RenderWindow& ventana, sf::Font& fuente, const char* titulo) {
    ventana.setView(ventana.getDefaultView());

    sf::RectangleShape velo({800.0f, 600.0f});
    velo.setFillColor(sf::Color(0, 0, 0, 118));
    ventana.draw(velo);

    sf::RectangleShape sombra({468.0f, 394.0f});
    sombra.setPosition({170.0f, 112.0f});
    sombra.setFillColor(sf::Color(0, 0, 0, 145));
    ventana.draw(sombra);

    sf::RectangleShape panel({468.0f, 394.0f});
    panel.setPosition({166.0f, 108.0f});
    panel.setFillColor(sf::Color(68, 68, 68, 242));
    panel.setOutlineColor(sf::Color(25, 25, 25));
    panel.setOutlineThickness(4.0f);
    ventana.draw(panel);

    sf::Text texto(fuente, titulo, 28);
    texto.setFillColor(sf::Color(232, 232, 232));
    texto.setOutlineColor(sf::Color::Black);
    texto.setOutlineThickness(3.0f);
    centrarTexto(texto, {400.0f, 144.0f});
    ventana.draw(texto);
}

inline sf::FloatRect rectPausa(int indice, float yInicial = 190.0f, float ancho = 320.0f) {
    return sf::FloatRect({400.0f - ancho * 0.5f, yInicial + static_cast<float>(indice) * 48.0f}, {ancho, 36.0f});
}

inline int hoverPausa(sf::Vector2i mouse, int cantidad, float yInicial = 190.0f, float ancho = 320.0f) {
    sf::Vector2f p(static_cast<float>(mouse.x), static_cast<float>(mouse.y));
    for (int i = 0; i < cantidad; ++i) {
        if (rectPausa(i, yInicial, ancho).contains(p)) return i;
    }
    return -1;
}

inline void dibujarMenuPausaPrincipal(sf::RenderWindow& ventana, sf::Font& fuente, sf::Vector2i mouse) {
    dibujarFondoPausa(ventana, fuente, "Menu de Pausa");
    const char* opciones[4] = {"Volver al juego", "Logros", "Ayuda y Opciones", "Salir del juego"};
    int hover = hoverPausa(mouse, 4);
    for (int i = 0; i < 4; ++i) {
        dibujarBotonRect(ventana, fuente, rectPausa(i), opciones[i], hover == i, 17);
    }
}

inline void dibujarPausaLogros(sf::RenderWindow& ventana, sf::Font& fuente, sf::Vector2i mouse) {
    dibujarFondoPausa(ventana, fuente, "Logros");
    struct LogroVista { const char* titulo; const char* descripcion; bool listo; };
    LogroVista logros[4] = {
        {"Primer tronco", "Pica tu primer bloque de madera", true},
        {"Hora de minar", "Consigue piedra con un pico", false},
        {"Artesano", "Crea una mesa de crafteo", false},
        {"Sobrevive la noche", "Derrota o evita a un zombie", false}
    };

    for (int i = 0; i < 4; ++i) {
        sf::FloatRect r({196.0f, 178.0f + static_cast<float>(i) * 58.0f}, {408.0f, 48.0f});
        sf::RectangleShape fila(r.size);
        fila.setPosition(r.position);
        fila.setFillColor(sf::Color(48, 48, 48, 225));
        fila.setOutlineColor(sf::Color(22, 22, 22));
        fila.setOutlineThickness(2.0f);
        ventana.draw(fila);

        sf::RectangleShape icono({30.0f, 30.0f});
        icono.setPosition({r.position.x + 10.0f, r.position.y + 9.0f});
        icono.setFillColor(logros[i].listo ? sf::Color(76, 160, 70) : sf::Color(82, 82, 82));
        icono.setOutlineColor(sf::Color(20, 20, 20));
        icono.setOutlineThickness(2.0f);
        ventana.draw(icono);

        sf::Text titulo(fuente, logros[i].titulo, 14);
        titulo.setFillColor(logros[i].listo ? sf::Color(255, 236, 120) : sf::Color(190, 190, 190));
        titulo.setOutlineColor(sf::Color::Black);
        titulo.setOutlineThickness(1.0f);
        titulo.setPosition({r.position.x + 52.0f, r.position.y + 6.0f});
        ventana.draw(titulo);

        sf::Text desc(fuente, logros[i].descripcion, 11);
        desc.setFillColor(sf::Color(220, 220, 220));
        desc.setPosition({r.position.x + 52.0f, r.position.y + 26.0f});
        ventana.draw(desc);
    }

    dibujarBotonRect(ventana, fuente, rectPausa(0, 438.0f, 160.0f), "Atras",
                     rectPausa(0, 438.0f, 160.0f).contains(sf::Vector2f(mouse)), 15);
}

inline void dibujarPausaAyuda(sf::RenderWindow& ventana, sf::Font& fuente, sf::Vector2i mouse) {
    dibujarFondoPausa(ventana, fuente, "Ayuda y Opciones");
    const char* opciones[6] = {"Como jugar", "Controles", "Configuracion", "Cambiar de aspecto", "Creditos", "Atras"};
    int hover = hoverPausa(mouse, 6, 176.0f, 320.0f);
    for (int i = 0; i < 6; ++i) {
        dibujarBotonRect(ventana, fuente, rectPausa(i, 176.0f, 320.0f), opciones[i], hover == i, 16);
    }
}

inline void dibujarPausaConfiguracion(sf::RenderWindow& ventana, sf::Font& fuente, sf::Vector2i mouse) {
    dibujarFondoPausa(ventana, fuente, "Configuracion");
    const char* opciones[4] = {"Audio", "Graficos", "Opciones de Juego", "Atras"};
    int hover = hoverPausa(mouse, 4);
    for (int i = 0; i < 4; ++i) {
        dibujarBotonRect(ventana, fuente, rectPausa(i), opciones[i], hover == i, 17);
    }
}

inline void dibujarPausaAudio(sf::RenderWindow& ventana, sf::Font& fuente, int musica, int efectos, sf::Vector2i mouse) {
    dibujarFondoPausa(ventana, fuente, "Audio");
    dibujarSliderMenu(ventana, fuente, "Volumen de Musica", 218.0f, musica);
    dibujarSliderMenu(ventana, fuente, "Volumen de Sonidos", 292.0f, efectos);
    dibujarBotonRect(ventana, fuente, rectPausa(0, 418.0f, 160.0f), "Atras",
                     rectPausa(0, 418.0f, 160.0f).contains(sf::Vector2f(mouse)), 15);
}

inline void dibujarPausaGraficos(sf::RenderWindow& ventana, sf::Font& fuente, int brillo, bool balanceo, sf::Vector2i mouse) {
    dibujarFondoPausa(ventana, fuente, "Graficos");
    dibujarSliderMenu(ventana, fuente, "Brillo", 218.0f, brillo);
    dibujarCheckboxMenu(ventana, fuente, {236.0f, 306.0f}, balanceo ? "Balanceo de camara: Si" : "Balanceo de camara: No", balanceo);
    dibujarBotonRect(ventana, fuente, rectPausa(0, 418.0f, 160.0f), "Atras",
                     rectPausa(0, 418.0f, 160.0f).contains(sf::Vector2f(mouse)), 15);
}

inline void dibujarPausaJuego(sf::RenderWindow& ventana, sf::Font& fuente, int dificultad, bool nombres, sf::Vector2i mouse) {
    dibujarFondoPausa(ventana, fuente, "Opciones de Juego");
    sf::Text d(fuente, "Dificultad", 16);
    d.setFillColor(sf::Color::White);
    d.setOutlineColor(sf::Color::Black);
    d.setOutlineThickness(2.0f);
    d.setPosition({236.0f, 218.0f});
    ventana.draw(d);
    std::string dificultadActual = dificultadTexto(dificultad);
    dibujarBotonRect(ventana, fuente, sf::FloatRect({410.0f, 210.0f}, {150.0f, 34.0f}),
                     dificultadActual.c_str(), sf::FloatRect({410.0f, 210.0f}, {150.0f, 34.0f}).contains(sf::Vector2f(mouse)), 15);
    dibujarCheckboxMenu(ventana, fuente, {236.0f, 292.0f}, "Nombres de jugador", nombres);
    dibujarBotonRect(ventana, fuente, rectPausa(0, 418.0f, 160.0f), "Atras",
                     rectPausa(0, 418.0f, 160.0f).contains(sf::Vector2f(mouse)), 15);
}

inline void dibujarPausaSkins(sf::RenderWindow& ventana, sf::Font& fuente, sf::Vector2i mouse) {
    dibujarFondoPausa(ventana, fuente, "Cambiar de aspecto");
    for (int i = 0; i < 4; ++i) {
        sf::RectangleShape skin({74.0f, 96.0f});
        skin.setPosition({194.0f + static_cast<float>(i) * 104.0f, 220.0f});
        skin.setFillColor(i == 0 ? sf::Color(54, 168, 190) : sf::Color(78 + i * 30, 92, 120 + i * 22));
        skin.setOutlineColor(i == 0 ? sf::Color(180, 255, 116) : sf::Color(25, 25, 25));
        skin.setOutlineThickness(3.0f);
        ventana.draw(skin);
    }
    sf::Text nota(fuente, "Galeria lista para conectar mas sprites.", 13);
    nota.setFillColor(sf::Color(220, 220, 220));
    nota.setOutlineColor(sf::Color::Black);
    nota.setOutlineThickness(1.0f);
    centrarTexto(nota, {400.0f, 356.0f});
    ventana.draw(nota);
    dibujarBotonRect(ventana, fuente, rectPausa(0, 418.0f, 160.0f), "Atras",
                     rectPausa(0, 418.0f, 160.0f).contains(sf::Vector2f(mouse)), 15);
}

inline void dibujarPausaConfirmarSalida(sf::RenderWindow& ventana, sf::Font& fuente, sf::Vector2i mouse) {
    dibujarFondoPausa(ventana, fuente, "Salir del juego");
    sf::Text advertencia(fuente, "Quieres guardar antes de volver al menu principal?", 14);
    advertencia.setFillColor(sf::Color::White);
    advertencia.setOutlineColor(sf::Color::Black);
    advertencia.setOutlineThickness(2.0f);
    centrarTexto(advertencia, {400.0f, 194.0f});
    ventana.draw(advertencia);

    const char* opciones[3] = {"Guardar y salir", "Salir sin guardar", "Cancelar"};
    int hover = hoverPausa(mouse, 3, 244.0f);
    for (int i = 0; i < 3; ++i) {
        dibujarBotonRect(ventana, fuente, rectPausa(i, 244.0f), opciones[i], hover == i, 16);
    }
}

}

inline Juego::Juego()
    : ventana(sf::VideoMode({800, 600}), "TEST DE CAMBIOS REALES"),
      estaCorriendo(true),
      fuenteCargada(false),
      worldTime(0.0f),
      skyLight(15),
      spawnHostilesHabilitado(false),
      enSubsuelo(false),
      posicionEntradaSuperficie(0.0f, 0.0f) {
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
    std::vector<MundoGuardado> mundosGuardados = escanearMundosGuardados();
    MundoGuardado mundoActivo;
    bool hayMundoActivo = false;
    bool menuPausaAbierto = false;
    int pantallaPausa = 0;
    int paginaPausaComoJugar = 0;
    bool invertirEjesPausa = false;
    bool balanceoCamara = false;
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
    sf::SoundBuffer bufferRecolectar;
    std::vector<std::int16_t> muestrasRecolectar(3308);
    for (std::size_t i = 0; i < muestrasRecolectar.size(); ++i) {
        float t = static_cast<float>(i) / 44100.0f;
        float progreso = static_cast<float>(i) / static_cast<float>(muestrasRecolectar.size());
        float envolvente = std::sin(progreso * 3.1415926f);
        float frecuencia = 580.0f + 360.0f * (1.0f - progreso);
        float onda = std::sin(t * frecuencia * 6.2831853f) * envolvente;
        muestrasRecolectar[i] = static_cast<std::int16_t>(onda * 9000.0f);
    }
    bool recolectarListo = bufferRecolectar.loadFromSamples(
        muestrasRecolectar.data(),
        muestrasRecolectar.size(),
        1,
        44100,
        std::vector<sf::SoundChannel>{sf::SoundChannel::Mono}
    );
    sf::Sound sonidoRecolectar(bufferRecolectar);
    auto reproducirRecolectar = [&]() {
        if (recolectarListo && volumenEfectos > 0) {
            sonidoRecolectar.setVolume(static_cast<float>(volumenEfectos));
            sonidoRecolectar.play();
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

    auto guardarMundoActivo = [&]() {
        if (!hayMundoActivo || !mapaSuperficie) {
            return;
        }
        Mundo* mundoParaGuardar = (enSubsuelo && mapaExterior) ? mapaExterior.get() : mapaSuperficie.get();
        mundoActivo.ultimaVez = fechaActualTexto();
        guardarMetaMundo(mundoActivo);
        mundoParaGuardar->guardarEstado(rutaBloquesMundo(mundoActivo.carpeta).string());
        {
            std::ofstream inv(rutaInventarioMundo(mundoActivo.carpeta));
            const auto& slots = inventarioGrid.getSlots();
            for (std::size_t i = 0; i < slots.size(); ++i) {
                inv << i << ' ' << static_cast<int>(slots[i].item) << ' ' << slots[i].cantidad << '\n';
            }
        }
        if (jugador) {
            std::ofstream pj(rutaJugadorMundo(mundoActivo.carpeta));
            sf::Vector2f pos = enSubsuelo ? posicionEntradaSuperficie : jugador->getPosicion();
            pj << pos.x << ' ' << pos.y << '\n';
        }
        mundosGuardados = escanearMundosGuardados();
    };

    auto iniciarMundo = [&](MundoGuardado mundo, bool cargarBloques) {
        for (auto* animal : animales) delete animal;
        animales.clear();
        for (auto* zombie : zombis) delete zombie;
        zombis.clear();
        itemsSuelo.clear();
        mapaExterior.reset();
        enSubsuelo = false;
        posicionEntradaSuperficie = {0.0f, 0.0f};

        mapaSuperficie = std::make_unique<Mundo>(1000, 1000, mundo.semilla);
        if (cargarBloques) {
            mapaSuperficie->cargarEstado(rutaBloquesMundo(mundo.carpeta).string());
        }

        std::mt19937 genMundo(mundo.semilla ^ 0x5A17C3u);
        std::uniform_int_distribution<> spawnBloqueX(30, mapaSuperficie->getAncho() - 31);
        std::uniform_int_distribution<> spawnBloqueY(30, mapaSuperficie->getAlto() - 31);

        auto bloqueSeguroParaSpawn = [&](int bx, int by) {
            if (mapaSuperficie->getTipoBloque(bx, by) != TipoBloque::Pasto) return false;
            for (int y = by - 1; y <= by + 1; ++y) {
                for (int x = bx - 1; x <= bx + 1; ++x) {
                    if (mapaSuperficie->esBloqueSolido(x, y)) return false;
                }
            }
            return true;
        };

        int bloqueSpawnX = 50;
        int bloqueSpawnY = 50;
        bool spawnEncontrado = false;
        for (int intento = 0; intento < 8000 && !spawnEncontrado; ++intento) {
            int bx = spawnBloqueX(genMundo);
            int by = spawnBloqueY(genMundo);
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

        float jugadorX = static_cast<float>(bloqueSpawnX) * TAMANIO_BLOQUE_JUEGO;
        float jugadorY = static_cast<float>(bloqueSpawnY) * TAMANIO_BLOQUE_JUEGO;
        if (cargarBloques) {
            std::ifstream pj(rutaJugadorMundo(mundo.carpeta));
            if (pj) {
                pj >> jugadorX >> jugadorY;
            }
        }
        jugador = std::make_unique<Jugador>(jugadorX, jugadorY);
        camara.setSize({560.0f, 420.0f});

        std::uniform_int_distribution<> animalBloqueX(30, mapaSuperficie->getAncho() - 31);
        std::uniform_int_distribution<> animalBloqueY(30, mapaSuperficie->getAlto() - 31);
        std::uniform_int_distribution<> tipoAnimal(0, 3);
        auto bloqueSeguroParaAnimal = [&](int bx, int by) {
            return mapaSuperficie->getTipoBloque(bx, by) == TipoBloque::Pasto &&
                   !mapaSuperficie->esBloqueSolido(bx, by);
        };

        constexpr int TOTAL_ANIMALES = 420;
        int animalesCreados = 0;
        for (int intento = 0; intento < TOTAL_ANIMALES * 40 && animalesCreados < TOTAL_ANIMALES; ++intento) {
            int bx = animalBloqueX(genMundo);
            int by = animalBloqueY(genMundo);
            if (!bloqueSeguroParaAnimal(bx, by)) continue;

            TipoAnimal tipo = TipoAnimal::Cerdo;
            int t = tipoAnimal(genMundo);
            if (t == 1) tipo = TipoAnimal::Oveja;
            if (t == 2) tipo = TipoAnimal::Vaca;
            if (t == 3) tipo = TipoAnimal::Gallina;
            animales.push_back(new Animal(
                static_cast<float>(bx) * TAMANIO_BLOQUE_JUEGO,
                static_cast<float>(by) * TAMANIO_BLOQUE_JUEGO,
                tipo
            ));
            ++animalesCreados;
        }

        mundoActivo = mundo;
        hayMundoActivo = true;
        mapaInicialGenerado = false;
        dificultadMundoNuevo = mundo.dificultad;
        {
            auto& slots = inventarioGrid.getSlots();
            for (auto& slot : slots) {
                slot = {};
            }
            if (cargarBloques) {
                std::ifstream inv(rutaInventarioMundo(mundo.carpeta));
                int indice = 0;
                int item = 0;
                int cantidad = 0;
                while (inv >> indice >> item >> cantidad) {
                    if (indice >= 0 && indice < static_cast<int>(slots.size())) {
                        slots[indice].item = static_cast<ItemId>(item);
                        slots[indice].cantidad = cantidad;
                    }
                }
            }
            bool inventarioVacio = true;
            for (const auto& slot : slots) {
                if (!esItemVacio(slot.item)) {
                    inventarioVacio = false;
                    break;
                }
            }
            if (inventarioVacio) {
                inventarioGrid.agregarItem(ItemId::MapaInicial, 1);
            }
        }
        if (mundoActivo.ultimaVez.empty()) {
            mundoActivo.ultimaVez = fechaActualTexto();
        }
        guardarMundoActivo();
    };

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

    auto crearNuevoMundoDesdeFormulario = [&]() {
        MundoGuardado nuevo;
        nuevo.nombre = nombreMundoNuevo.empty() ? "Nuevo Mundo" : nombreMundoNuevo;
        nuevo.semillaTexto = semillaMundoNuevo.empty() ? "random" : semillaMundoNuevo;
        nuevo.semilla = semillaDesdeTexto(semillaMundoNuevo);
        nuevo.dificultad = std::clamp(dificultadMundoNuevo, 0, 3);
        nuevo.ultimaVez = fechaActualTexto();
        nuevo.carpeta = carpetaUnicaMundo(nuevo.nombre);
        guardarMetaMundo(nuevo);
        iniciarMundo(nuevo, false);
        iniciarPartidaDesdeMenu();
    };

    auto entrarSubsuelo = [&]() {
        if (!jugador || !mapaSuperficie || enSubsuelo) return;

        sf::Vector2f posJugador = jugador->getPosicion();
        sf::Vector2f centroJugador = posJugador + sf::Vector2f(12.0f, 12.0f);
        int bx = static_cast<int>(std::floor(centroJugador.x / TAMANIO_BLOQUE_JUEGO));
        int by = static_cast<int>(std::floor(centroJugador.y / TAMANIO_BLOQUE_JUEGO));
        if (!mapaSuperficie->esMinaAbierta(bx, by)) return;

        posicionEntradaSuperficie = posJugador;
        mapaExterior = std::move(mapaSuperficie);
        unsigned int semillaCueva = mapaExterior->getSemilla() ^ 0xC0A7E5u ^
                                    static_cast<unsigned int>(bx * 928371u + by * 1237u);
        mapaSuperficie = std::make_unique<Mundo>(1000, 1000, semillaCueva);
        mapaSuperficie->generarMundo(true);

        int entradaX = mapaSuperficie->getAncho() / 2;
        int entradaY = mapaSuperficie->getAlto() / 2;
        mapaSuperficie->crearZonaEntradaSubterranea(entradaX, entradaY);
        jugador->setPosicion({
            static_cast<float>(entradaX) * TAMANIO_BLOQUE_JUEGO,
            static_cast<float>(entradaY) * TAMANIO_BLOQUE_JUEGO
        });
        itemsSuelo.clear();
        for (auto* zombie : zombis) delete zombie;
        zombis.clear();
        enSubsuelo = true;
    };

    auto salirSubsuelo = [&]() {
        if (!jugador || !mapaExterior || !enSubsuelo) return;

        sf::Vector2f centroJugador = jugador->getPosicion() + sf::Vector2f(12.0f, 12.0f);
        int bx = static_cast<int>(std::floor(centroJugador.x / TAMANIO_BLOQUE_JUEGO));
        int by = static_cast<int>(std::floor(centroJugador.y / TAMANIO_BLOQUE_JUEGO));
        if (mapaSuperficie->getTipoBloque(bx, by) != TipoBloque::CuevaEntrada) return;

        mapaSuperficie = std::move(mapaExterior);
        jugador->setPosicion(posicionEntradaSuperficie + sf::Vector2f(0.0f, TAMANIO_BLOQUE_JUEGO));
        itemsSuelo.clear();
        enSubsuelo = false;
    };

    while (ventana.isOpen() && estaCorriendo) {
        float dt = reloj.restart().asSeconds();
        if (dt > 0.05f) {
            dt = 0.05f;
        }
        tiempoMenuInicio += dt;
        if (!mostrandoMenuInicio && !menuPausaAbierto) {
            actualizarTiempo(dt);
        }
        if (menuPausaAbierto && pantallaPausa == 11) {
            scrollCreditos += dt * 36.0f;
        }
        if (hayMundoActivo && mundoActivo.dificultad == 0) {
            spawnHostilesHabilitado = false;
            for (auto* zombie : zombis) {
                delete zombie;
            }
            zombis.clear();
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
                guardarMundoActivo();
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
                            guardarMundoActivo();
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
                                guardarMundoActivo();
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
                                crearNuevoMundoDesdeFormulario();
                            }
                        } else if (pantallaMenuInicio == 8) {
                            pantallaMenuInicio = 6;
                        }
                    }
                    continue;
                }

                if (botonTeclado->code == sf::Keyboard::Key::Escape) {
                    reproducirClickMenu();
                    if (!menuPausaAbierto) {
                        menuPausaAbierto = true;
                        pantallaPausa = 0;
                    } else if (pantallaPausa == 0) {
                        menuPausaAbierto = false;
                    } else if (pantallaPausa == 3 || pantallaPausa == 4 || pantallaPausa == 5) {
                        pantallaPausa = 2;
                    } else if (pantallaPausa >= 6 && pantallaPausa <= 8) {
                        pantallaPausa = 5;
                    } else {
                        pantallaPausa = 0;
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

                if (botonTeclado->code == sf::Keyboard::Key::E &&
                    !menuPausaAbierto &&
                    !inventarioGrid.esMenuAbierto() &&
                    !inventarioGrid.esMesaCrafteoAbierta()) {
                    if (enSubsuelo) {
                        salirSubsuelo();
                    } else {
                        entrarSubsuelo();
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
                        guardarMundoActivo();
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
                        int visibles = std::min<int>(3, mundosGuardados.size());
                        for (int i = 0; i < visibles; ++i) {
                            sf::FloatRect mundo({82.0f, 332.0f + static_cast<float>(i) * 48.0f}, {284.0f, 40.0f});
                            if (mouseDentro(mundo)) {
                                reproducirClickMenu();
                                iniciarMundo(mundosGuardados[i], true);
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
                        crearNuevoMundoDesdeFormulario();
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
                dibujarMenuJugar(ventana, fuente, tiempoMenuInicio, partidaOnline, mundosGuardados, mousePos);
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

        auto mouseDentro = [&](sf::FloatRect rect) {
            return rect.contains(sf::Vector2f(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)));
        };
        auto valorSlider = [&](float xInicio) {
            float t = (static_cast<float>(mousePos.x) - xInicio) / 220.0f;
            return std::clamp(static_cast<int>(std::round(t * 100.0f)), 0, 100);
        };

        if (menuPausaAbierto && clickIzquierdo && !clickMenuAnterior) {
            reproducirClickMenu();
            if (pantallaPausa == 0) {
                int h = hoverPausa(mousePos, 4);
                if (h == 0) menuPausaAbierto = false;
                if (h == 1) pantallaPausa = 1;
                if (h == 2) pantallaPausa = 2;
                if (h == 3) pantallaPausa = 9;
            } else if (pantallaPausa == 1) {
                if (mouseDentro(rectPausa(0, 438.0f, 160.0f))) pantallaPausa = 0;
            } else if (pantallaPausa == 2) {
                int h = hoverPausa(mousePos, 6, 176.0f, 320.0f);
                if (h == 0) pantallaPausa = 3;
                if (h == 1) pantallaPausa = 4;
                if (h == 2) pantallaPausa = 5;
                if (h == 3) pantallaPausa = 10;
                if (h == 4) {
                    pantallaPausa = 11;
                    scrollCreditos = 0.0f;
                }
                if (h == 5) pantallaPausa = 0;
            } else if (pantallaPausa == 3) {
                if (mouseDentro(sf::FloatRect({128.0f, 472.0f}, {150.0f, 34.0f}))) paginaPausaComoJugar = std::max(0, paginaPausaComoJugar - 1);
                else if (mouseDentro(sf::FloatRect({326.0f, 472.0f}, {150.0f, 34.0f}))) paginaPausaComoJugar = std::min(2, paginaPausaComoJugar + 1);
                else if (mouseDentro(sf::FloatRect({524.0f, 472.0f}, {150.0f, 34.0f}))) pantallaPausa = 2;
            } else if (pantallaPausa == 4) {
                if (mouseDentro(sf::FloatRect({400.0f, 390.0f}, {220.0f, 18.0f}))) sensibilidadMirada = valorSlider(400.0f);
                else if (mouseDentro(sf::FloatRect({500.0f, 322.0f}, {120.0f, 32.0f}))) invertirEjesPausa = !invertirEjesPausa;
                else if (mouseDentro(sf::FloatRect({326.0f, 504.0f}, {150.0f, 34.0f}))) pantallaPausa = 2;
            } else if (pantallaPausa == 5) {
                int h = hoverPausa(mousePos, 4);
                if (h == 0) pantallaPausa = 6;
                if (h == 1) pantallaPausa = 7;
                if (h == 2) pantallaPausa = 8;
                if (h == 3) pantallaPausa = 2;
            } else if (pantallaPausa == 6) {
                if (mouseDentro(sf::FloatRect({400.0f, 226.0f}, {220.0f, 18.0f}))) {
                    volumenMusica = valorSlider(400.0f);
                    if (musicaMenuLista) musicaMenu.setVolume(static_cast<float>(volumenMusica));
                } else if (mouseDentro(sf::FloatRect({400.0f, 300.0f}, {220.0f, 18.0f}))) {
                    volumenEfectos = valorSlider(400.0f);
                } else if (mouseDentro(rectPausa(0, 418.0f, 160.0f))) pantallaPausa = 5;
            } else if (pantallaPausa == 7) {
                if (mouseDentro(sf::FloatRect({400.0f, 226.0f}, {220.0f, 18.0f}))) brilloMenu = valorSlider(400.0f);
                else if (mouseDentro(sf::FloatRect({236.0f, 306.0f}, {260.0f, 28.0f}))) balanceoCamara = !balanceoCamara;
                else if (mouseDentro(rectPausa(0, 418.0f, 160.0f))) pantallaPausa = 5;
            } else if (pantallaPausa == 8) {
                if (mouseDentro(sf::FloatRect({410.0f, 210.0f}, {150.0f, 34.0f}))) {
                    dificultadMundoNuevo = (dificultadMundoNuevo + 1) % 4;
                    if (hayMundoActivo) mundoActivo.dificultad = dificultadMundoNuevo;
                } else if (mouseDentro(sf::FloatRect({236.0f, 292.0f}, {220.0f, 28.0f}))) {
                    nombresJugador = !nombresJugador;
                } else if (mouseDentro(rectPausa(0, 418.0f, 160.0f))) pantallaPausa = 5;
            } else if (pantallaPausa == 9) {
                int h = hoverPausa(mousePos, 3, 244.0f);
                if (h == 0) {
                    guardarMundoActivo();
                    menuPausaAbierto = false;
                    if (inventarioGrid.esMesaCrafteoAbierta()) inventarioGrid.cerrarMesaCrafteo();
                    if (inventarioGrid.esMenuAbierto()) inventarioGrid.alternarMenu();
                    mostrandoMenuInicio = true;
                    pantallaMenuInicio = 0;
                    if (musicaMenuLista) musicaMenu.play();
                }
                if (h == 1) {
                    menuPausaAbierto = false;
                    if (inventarioGrid.esMesaCrafteoAbierta()) inventarioGrid.cerrarMesaCrafteo();
                    if (inventarioGrid.esMenuAbierto()) inventarioGrid.alternarMenu();
                    mostrandoMenuInicio = true;
                    pantallaMenuInicio = 0;
                    if (musicaMenuLista) musicaMenu.play();
                }
                if (h == 2) pantallaPausa = 0;
            } else if (pantallaPausa == 10) {
                if (mouseDentro(rectPausa(0, 418.0f, 160.0f))) pantallaPausa = 2;
            } else if (pantallaPausa == 11) {
                pantallaPausa = 2;
            }
        }

        clickMenuAnterior = clickIzquierdo;

        if (!menuPausaAbierto) {
            inventarioGrid.manejarClicks(mousePos, clickIzquierdo, clickDerecho);
        }

        bool uiAbierta = inventarioGrid.esMenuAbierto() || inventarioGrid.esMesaCrafteoAbierta() || menuPausaAbierto;

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

        if (!enSubsuelo) {
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
        }

        if (spawnHostilesHabilitado && !uiAbierta && !enSubsuelo) {
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
            sf::Vector2f pechoJugador = jugador->getPosicion() + sf::Vector2f(12.0f, 10.0f);
            for (auto it = itemsSuelo.begin(); it != itemsSuelo.end();) {
                if (!it->fisicaInicializada) {
                    std::uniform_real_distribution<float> angulo(0.0f, 6.2831853f);
                    std::uniform_real_distribution<float> fuerza(24.0f, 48.0f);
                    std::uniform_real_distribution<float> impulso(28.0f, 44.0f);
                    float a = angulo(genLoot);
                    float f = fuerza(genLoot);
                    it->velocidad = {std::cos(a) * f, std::sin(a) * f};
                    it->velocidadAltura = impulso(genLoot);
                    it->altura = 0.0f;
                    it->rebotesRestantes = 2;
                    it->fisicaInicializada = true;
                }

                it->tiempo += dt;
                sf::Vector2f deltaBase = pechoJugador - it->posicionBase;
                float distanciaBase = std::sqrt(deltaBase.x * deltaBase.x + deltaBase.y * deltaBase.y);
                constexpr float RADIO_ABSORCION = 48.0f;

                if (!it->absorbiendo &&
                    distanciaBase <= RADIO_ABSORCION &&
                    inventarioGrid.puedeAgregarItem(it->item, it->cantidad)) {
                    it->absorbiendo = true;
                }

                if (it->absorbiendo) {
                    sf::Vector2f delta = pechoJugador - it->posicionBase;
                    float distancia = std::sqrt(delta.x * delta.x + delta.y * delta.y);
                    if (distancia > 0.01f) {
                        sf::Vector2f direccion = delta / distancia;
                        float avance = std::min(distancia, 420.0f * dt);
                        it->posicionBase += direccion * avance;
                    }

                    it->altura = std::max(0.0f, it->altura - 90.0f * dt);
                    it->escala = std::clamp(distancia / RADIO_ABSORCION, 0.05f, 1.0f);
                    it->giroY = std::max(0.25f, std::abs(std::cos(it->tiempo * 8.0f)));
                    it->posicion = it->posicionBase + sf::Vector2f(0.0f, -it->altura);

                    if (distancia <= 5.0f || it->escala <= 0.08f) {
                        inventarioGrid.agregarItem(it->item, it->cantidad);
                        reproducirRecolectar();
                        it = itemsSuelo.erase(it);
                        continue;
                    }
                } else {
                    if (it->rebotesRestantes > 0 || std::abs(it->velocidadAltura) > 0.01f || it->altura > 0.0f) {
                        it->posicionBase += it->velocidad * dt;
                        it->velocidad *= std::pow(0.18f, dt);
                        it->velocidadAltura -= 120.0f * dt;
                        it->altura += it->velocidadAltura * dt;

                        if (it->altura <= 0.0f) {
                            it->altura = 0.0f;
                            if (it->rebotesRestantes > 0) {
                                it->velocidadAltura = 17.0f + 7.0f * static_cast<float>(it->rebotesRestantes);
                                it->velocidad *= 0.55f;
                                --it->rebotesRestantes;
                            } else {
                                it->velocidad = {0.0f, 0.0f};
                                it->velocidadAltura = 0.0f;
                            }
                        }
                    }

                    float flote = std::sin(it->tiempo * 3.0f) * 2.6f;
                    it->escala = 1.0f;
                    it->giroY = 0.35f + 0.65f * std::abs(std::cos(it->tiempo * 2.4f));
                    it->posicion = it->posicionBase + sf::Vector2f(0.0f, -it->altura + flote);
                }

                if (it->absorbiendo && !inventarioGrid.puedeAgregarItem(it->item, it->cantidad)) {
                    it->absorbiendo = false;
                    it->escala = 1.0f;
                }

                ++it;
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
        bool selectorBloqueEnRango = false;

        if (mapaEnSegundaMano && !mapaInicialGenerado) {
            generarMapaInicial();
        }

        if (jugador && mapaSuperficie) {
            sf::Vector2f centroJugador = jugador->getPosicion() + sf::Vector2f(12.0f, 12.0f);
            bloqueJugadorX = static_cast<int>(std::floor(centroJugador.x / TAMANIO_BLOQUE_JUEGO));
            bloqueJugadorY = static_cast<int>(std::floor(centroJugador.y / TAMANIO_BLOQUE_JUEGO));
            jugadorSobreEntradaMina = mapaSuperficie->getTipoBloque(bloqueJugadorX, bloqueJugadorY) == TipoBloque::CuevaEntrada;
            sf::Vector2f centroBloqueMouse((bloqueMouseX + 0.5f) * TAMANIO_BLOQUE_JUEGO, (bloqueMouseY + 0.5f) * TAMANIO_BLOQUE_JUEGO);
            sf::Vector2f deltaMouse = centroBloqueMouse - centroJugador;
            selectorBloqueEnRango = std::sqrt(deltaMouse.x * deltaMouse.x + deltaMouse.y * deltaMouse.y) <= 115.0f;

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
                                sf::Vector2f posDrop((bloqueX + 0.5f) * TAMANIO_BLOQUE_JUEGO, (bloqueY + 0.5f) * TAMANIO_BLOQUE_JUEGO);
                                itemsSuelo.push_back({ItemId::BloqueTronco, troncosObtenidos, posDrop});
                                if (soltoSemilla) {
                                    itemsSuelo.push_back({ItemId::SemillaArbol, 1, posDrop + sf::Vector2f(8.0f, -5.0f)});
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
                                    sf::Vector2f posDrop((bloqueX + 0.5f) * TAMANIO_BLOQUE_JUEGO, (bloqueY + 0.5f) * TAMANIO_BLOQUE_JUEGO);
                                    itemsSuelo.push_back({itemDesdeBloque(tipoActual), 1, posDrop});
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
        if (!uiAbierta && mapaSuperficie &&
            bloqueMouseX >= 0 && bloqueMouseY >= 0 &&
            bloqueMouseX < mapaSuperficie->getAncho() &&
            bloqueMouseY < mapaSuperficie->getAlto()) {
            dibujarSelectorBloque(ventana, bloqueMouseX, bloqueMouseY, selectorBloqueEnRango, tiempoMenuInicio);
        }

        sf::Vector2f centroVista = camara.getCenter();
        sf::Vector2f tamanoVista = camara.getSize();
        float margenDibujo = 96.0f;
        float dibujoIzq = centroVista.x - tamanoVista.x / 2.0f - margenDibujo;
        float dibujoDer = centroVista.x + tamanoVista.x / 2.0f + margenDibujo;
        float dibujoArriba = centroVista.y - tamanoVista.y / 2.0f - margenDibujo;
        float dibujoAbajo = centroVista.y + tamanoVista.y / 2.0f + margenDibujo;

        if (!enSubsuelo) {
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

            float escalaItem = std::clamp(item.escala, 0.05f, 1.0f);
            float giroItem = std::clamp(item.giroY, 0.24f, 1.0f);

            sf::RectangleShape sombra({16.0f * escalaItem * giroItem, 5.0f * escalaItem});
            sombra.setOrigin({8.0f * escalaItem * giroItem, 2.5f * escalaItem});
            sombra.setPosition({item.posicionBase.x, item.posicionBase.y + 9.0f});
            sombra.setFillColor(sf::Color(20, 20, 20, 80));
            ventana.draw(sombra);

            dibujarItemSueloSprite(ventana, item.item, item.posicion, escalaItem, giroItem);

            if (item.cantidad > 1) {
                sf::CircleShape brillo(2.0f * escalaItem);
                brillo.setPosition({item.posicion.x + 3.0f * escalaItem, item.posicion.y - 4.0f * escalaItem});
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

                sf::Text textoMina(fuente, minaAbierta ? (enSubsuelo ? "Salida de mina - presiona E" : "Mina abierta - presiona E") : "Picando entrada de mina", 12);
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

            if (menuPausaAbierto) {
                if (pantallaPausa == 0) {
                    dibujarMenuPausaPrincipal(ventana, fuente, mousePos);
                } else if (pantallaPausa == 1) {
                    dibujarPausaLogros(ventana, fuente, mousePos);
                } else if (pantallaPausa == 2) {
                    dibujarPausaAyuda(ventana, fuente, mousePos);
                } else if (pantallaPausa == 3) {
                    dibujarComoJugar(ventana, fuente, tiempoMenuInicio, paginaPausaComoJugar, mousePos);
                } else if (pantallaPausa == 4) {
                    dibujarControles(ventana, fuente, tiempoMenuInicio, invertirEjesPausa, sensibilidadMirada);
                } else if (pantallaPausa == 5) {
                    dibujarPausaConfiguracion(ventana, fuente, mousePos);
                } else if (pantallaPausa == 6) {
                    dibujarPausaAudio(ventana, fuente, volumenMusica, volumenEfectos, mousePos);
                } else if (pantallaPausa == 7) {
                    dibujarPausaGraficos(ventana, fuente, brilloMenu, balanceoCamara, mousePos);
                } else if (pantallaPausa == 8) {
                    dibujarPausaJuego(ventana, fuente, hayMundoActivo ? mundoActivo.dificultad : dificultadMundoNuevo, nombresJugador, mousePos);
                } else if (pantallaPausa == 9) {
                    dibujarPausaConfirmarSalida(ventana, fuente, mousePos);
                } else if (pantallaPausa == 10) {
                    dibujarPausaSkins(ventana, fuente, mousePos);
                } else if (pantallaPausa == 11) {
                    dibujarCreditos(ventana, fuente, tiempoMenuInicio, scrollCreditos);
                }
            }
        }

        aplicarBrilloPantalla(ventana, brilloMenu);
        ventana.display();
        clickIzquierdoAnterior = clickIzquierdo;
        clickDerechoAnterior = clickDerecho;
    }
}


