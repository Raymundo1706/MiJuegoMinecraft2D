#include "../../core/Mundo.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>

namespace {
inline sf::Color colorRopaAldeano(ProfesionAldeano profesion) {
    switch (profesion) {
        case ProfesionAldeano::Granjero: return sf::Color(132, 82, 44);
        case ProfesionAldeano::Herrero: return sf::Color(42, 44, 48);
        case ProfesionAldeano::Bibliotecario: return sf::Color(224, 224, 214);
    }
    return sf::Color(132, 82, 44);
}

inline std::uint32_t hashAldeano(float x, float y) {
    int ix = static_cast<int>(x * 13.0f);
    int iy = static_cast<int>(y * 17.0f);
    return static_cast<std::uint32_t>(ix * 73856093) ^ static_cast<std::uint32_t>(iy * 19349663);
}

inline void pixelAldeano(sf::RenderWindow& ventana, sf::Vector2f origen, int x, int y, sf::Color color, float escala, bool espejo) {
    int px = espejo ? 15 - x : x;
    sf::RectangleShape p({escala, escala});
    p.setPosition({origen.x + static_cast<float>(px) * escala, origen.y + static_cast<float>(y) * escala});
    p.setFillColor(color);
    ventana.draw(p);
}

inline void rectAldeano(sf::RenderWindow& ventana, sf::Vector2f origen, int x, int y, int w, int h, sf::Color color, float escala, bool espejo = false) {
    for (int yy = y; yy < y + h; ++yy) {
        for (int xx = x; xx < x + w; ++xx) {
            pixelAldeano(ventana, origen, xx, yy, color, escala, espejo);
        }
    }
}

inline bool esBloqueAldeanoInteres(TipoBloque tipo, ProfesionAldeano profesion) {
    if (tipo == TipoBloque::CaminoAldea || tipo == TipoBloque::PuertaCerrada || tipo == TipoBloque::PuertaAbierta) {
        return true;
    }
    if (profesion == ProfesionAldeano::Granjero) {
        return tipo == TipoBloque::CultivoTrigo ||
               tipo == TipoBloque::CultivoZanahoria ||
               tipo == TipoBloque::CultivoPatata ||
               tipo == TipoBloque::TierraArada;
    }
    if (profesion == ProfesionAldeano::Herrero) {
        return tipo == TipoBloque::Horno ||
               tipo == TipoBloque::Yunque ||
               tipo == TipoBloque::Cofre;
    }
    return tipo == TipoBloque::Techo ||
           tipo == TipoBloque::Cristal;
}
}

inline Aldeano::Aldeano(float x, float y, ProfesionAldeano profesion)
    : posicion(x, y),
      velocidad(0.0f, 0.0f),
      objetivo(x, y),
      profesion(profesion),
      tiempoDecision(0.0f),
      tiempoAnimacion(0.0f),
      refugiandose(false),
      direccionMirada(0) {
}

inline Aldeano::~Aldeano() {}

inline bool Aldeano::colisionaConMundo(const Mundo& mundo, sf::Vector2f nuevaPosicion) const {
    constexpr float ancho = 16.0f;
    constexpr float alto = 21.0f;
    float puntos[4][2] = {
        {nuevaPosicion.x + 3.0f, nuevaPosicion.y + 12.0f},
        {nuevaPosicion.x + ancho - 3.0f, nuevaPosicion.y + 12.0f},
        {nuevaPosicion.x + 4.0f, nuevaPosicion.y + alto - 2.0f},
        {nuevaPosicion.x + ancho - 4.0f, nuevaPosicion.y + alto - 2.0f}
    };

    for (auto& punto : puntos) {
        int bx = static_cast<int>(std::floor(punto[0] / TAMANIO_BLOQUE_JUEGO));
        int by = static_cast<int>(std::floor(punto[1] / TAMANIO_BLOQUE_JUEGO));
        TipoBloque tipo = mundo.getTipoBloque(bx, by);
        if (mundo.esBloqueSolido(bx, by) ||
            tipo == TipoBloque::Agua ||
            tipo == TipoBloque::AguaProfunda ||
            tipo == TipoBloque::Lava) {
            return true;
        }
    }
    return false;
}

inline bool Aldeano::buscarPuertaCercana(const Mundo& mundo, sf::Vector2f& destino) const {
    sf::Vector2f centro = posicion + sf::Vector2f(8.0f, 12.0f);
    int cx = static_cast<int>(std::floor(centro.x / TAMANIO_BLOQUE_JUEGO));
    int cy = static_cast<int>(std::floor(centro.y / TAMANIO_BLOQUE_JUEGO));
    float mejorDist = 999999.0f;
    bool encontrada = false;

    for (int y = cy - 18; y <= cy + 18; ++y) {
        for (int x = cx - 18; x <= cx + 18; ++x) {
            TipoBloque tipo = mundo.getTipoBloque(x, y);
            if (tipo != TipoBloque::PuertaCerrada && tipo != TipoBloque::PuertaAbierta) {
                continue;
            }
            sf::Vector2f p(static_cast<float>(x) * TAMANIO_BLOQUE_JUEGO + 4.0f,
                           static_cast<float>(y) * TAMANIO_BLOQUE_JUEGO + 2.0f);
            sf::Vector2f d = p - posicion;
            float dist = d.x * d.x + d.y * d.y;
            if (dist < mejorDist) {
                mejorDist = dist;
                destino = p;
                encontrada = true;
            }
        }
    }
    return encontrada;
}

inline void Aldeano::elegirObjetivoDia(const Mundo& mundo) {
    sf::Vector2f centro = posicion + sf::Vector2f(8.0f, 12.0f);
    int cx = static_cast<int>(std::floor(centro.x / TAMANIO_BLOQUE_JUEGO));
    int cy = static_cast<int>(std::floor(centro.y / TAMANIO_BLOQUE_JUEGO));

    std::mt19937 gen(hashAldeano(posicion.x + tiempoDecision * 31.0f, posicion.y));
    std::uniform_int_distribution<int> offset(-13, 13);

    for (int intento = 0; intento < 32; ++intento) {
        int bx = cx + offset(gen);
        int by = cy + offset(gen);
        TipoBloque tipo = mundo.getTipoBloque(bx, by);
        if (!esBloqueAldeanoInteres(tipo, profesion)) {
            continue;
        }

        sf::Vector2f candidato(static_cast<float>(bx) * TAMANIO_BLOQUE_JUEGO + 4.0f,
                               static_cast<float>(by) * TAMANIO_BLOQUE_JUEGO + 2.0f);
        if (!colisionaConMundo(mundo, candidato)) {
            objetivo = candidato;
            return;
        }
    }

    for (int intento = 0; intento < 16; ++intento) {
        sf::Vector2f candidato(
            posicion.x + static_cast<float>(offset(gen)) * 0.55f * TAMANIO_BLOQUE_JUEGO,
            posicion.y + static_cast<float>(offset(gen)) * 0.55f * TAMANIO_BLOQUE_JUEGO
        );
        if (!colisionaConMundo(mundo, candidato)) {
            objetivo = candidato;
            return;
        }
    }
    objetivo = posicion;
}

inline void Aldeano::actualizar(float dt, const Mundo& mundo, bool esNoche) {
    tiempoDecision -= dt;
    if (tiempoDecision <= 0.0f) {
        if (esNoche && buscarPuertaCercana(mundo, objetivo)) {
            refugiandose = true;
        } else {
            refugiandose = false;
            elegirObjetivoDia(mundo);
        }
        std::mt19937 gen(hashAldeano(posicion.x, posicion.y + tiempoAnimacion * 19.0f));
        std::uniform_real_distribution<float> espera(refugiandose ? 1.2f : 2.2f, refugiandose ? 2.2f : 5.0f);
        tiempoDecision = espera(gen);
    }

    sf::Vector2f delta = objetivo - posicion;
    float distancia = std::sqrt(delta.x * delta.x + delta.y * delta.y);
    if (distancia > 2.0f) {
        sf::Vector2f direccion = delta / distancia;
        float rapidez = refugiandose ? 30.0f : 16.0f;
        velocidad = direccion * rapidez;
        if (std::abs(velocidad.x) > std::abs(velocidad.y)) {
            direccionMirada = velocidad.x > 0.0f ? 1 : 3;
        } else {
            direccionMirada = velocidad.y < 0.0f ? 2 : 0;
        }
    } else {
        velocidad = {0.0f, 0.0f};
    }

    sf::Vector2f nueva = posicion + velocidad * dt;
    if (!colisionaConMundo(mundo, {nueva.x, posicion.y})) {
        posicion.x = nueva.x;
    } else {
        objetivo = posicion;
        tiempoDecision = 0.0f;
    }
    if (!colisionaConMundo(mundo, {posicion.x, nueva.y})) {
        posicion.y = nueva.y;
    } else {
        objetivo = posicion;
        tiempoDecision = 0.0f;
    }

    if (std::abs(velocidad.x) + std::abs(velocidad.y) > 0.1f) {
        tiempoAnimacion += dt * (refugiandose ? 2.0f : 1.0f);
    }
}

inline void Aldeano::dibujarPixelArt(sf::RenderWindow& ventana) const {
    float escala = 1.45f;
    sf::Vector2f origen(posicion.x - 3.0f, posicion.y - 7.0f);
    bool espejo = direccionMirada == 3;
    bool lado = direccionMirada == 1 || direccionMirada == 3;
    bool espalda = direccionMirada == 2;
    int paso = static_cast<int>(tiempoAnimacion * 7.0f) % 4;
    int bob = (paso == 1 || paso == 2) ? 1 : 0;

    sf::RectangleShape sombra({18.0f, 4.0f});
    sombra.setPosition({posicion.x - 1.0f, posicion.y + 17.0f});
    sombra.setFillColor(sf::Color(0, 0, 0, 70));
    ventana.draw(sombra);

    sf::Color piel(174, 116, 74);
    sf::Color pielSombra(116, 72, 48);
    sf::Color ropa = colorRopaAldeano(profesion);
    sf::Color ropaLuz(std::min(255, ropa.r + 42), std::min(255, ropa.g + 42), std::min(255, ropa.b + 42));
    sf::Color borde(36, 24, 18);
    sf::Color nariz(132, 78, 52);

    if (lado) {
        rectAldeano(ventana, origen, 5, 1 + bob, 6, 2, borde, escala, espejo);
        rectAldeano(ventana, origen, 4, 3 + bob, 8, 6, piel, escala, espejo);
        rectAldeano(ventana, origen, 10, 5 + bob, 2, 3, nariz, escala, espejo);
        rectAldeano(ventana, origen, 5, 9 + bob, 7, 7, ropa, escala, espejo);
        rectAldeano(ventana, origen, 5, 9 + bob, 5, 2, ropaLuz, escala, espejo);
        rectAldeano(ventana, origen, 4, 16, 3, 4 + (paso % 2), borde, escala, espejo);
        rectAldeano(ventana, origen, 10, 16, 3, 4 + ((paso + 1) % 2), borde, escala, espejo);
        return;
    }

    rectAldeano(ventana, origen, 4, 1 + bob, 8, 2, borde, escala);
    rectAldeano(ventana, origen, 3, 3 + bob, 10, 6, espalda ? pielSombra : piel, escala);
    if (!espalda) {
        rectAldeano(ventana, origen, 7, 6 + bob, 2, 3, nariz, escala);
        pixelAldeano(ventana, origen, 5, 5 + bob, sf::Color(26, 20, 16), escala, false);
        pixelAldeano(ventana, origen, 10, 5 + bob, sf::Color(26, 20, 16), escala, false);
    } else {
        rectAldeano(ventana, origen, 5, 4 + bob, 6, 2, borde, escala);
    }
    rectAldeano(ventana, origen, 4, 9 + bob, 8, 7, ropa, escala);
    rectAldeano(ventana, origen, 5, 9 + bob, 6, 2, ropaLuz, escala);
    rectAldeano(ventana, origen, 3, 16, 4, 4 + (paso % 2), borde, escala);
    rectAldeano(ventana, origen, 9, 16, 4, 4 + ((paso + 1) % 2), borde, escala);
}

inline void Aldeano::dibujar(sf::RenderWindow& ventana) const {
    dibujarPixelArt(ventana);
}

inline ProfesionAldeano Aldeano::getProfesion() const {
    return profesion;
}

inline sf::Vector2f Aldeano::getPosicion() const {
    return posicion;
}
