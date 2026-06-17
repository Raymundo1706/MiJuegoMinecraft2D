#include "../../core/Mundo.hpp"
#include "../../entidades/Jugador.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <array>

namespace {
inline void dibujarPixelZombie(sf::RenderWindow& ventana, sf::Vector2f origen, int x, int y, sf::Color color, float escala, bool espejo) {
    sf::RectangleShape pixel({escala, escala});
    int px = espejo ? 15 - x : x;
    pixel.setPosition({origen.x + static_cast<float>(px) * escala, origen.y + static_cast<float>(y) * escala});
    pixel.setFillColor(color);
    ventana.draw(pixel);
}

inline void dibujarRectZombie(sf::RenderWindow& ventana, sf::Vector2f origen, int x, int y, int ancho, int alto, sf::Color color, float escala, bool espejo) {
    for (int py = y; py < y + alto; ++py) {
        for (int px = x; px < x + ancho; ++px) {
            dibujarPixelZombie(ventana, origen, px, py, color, escala, espejo);
        }
    }
}

inline sf::Color tinteZombie(sf::Color base, float golpe, float quemadura) {
    int r = base.r;
    int g = base.g;
    int b = base.b;

    if (quemadura > 0.0f) {
        r = std::min(255, r + 55);
        g = std::max(0, g - 18);
        b = std::max(0, b - 28);
    }
    if (golpe > 0.0f) {
        r = std::min(255, r + 95);
        g = std::max(0, g - 35);
        b = std::max(0, b - 35);
    }
    return sf::Color(static_cast<std::uint8_t>(r), static_cast<std::uint8_t>(g), static_cast<std::uint8_t>(b), base.a);
}

inline void dibujarZombiePixelArt(sf::RenderWindow& ventana, sf::Vector2f posicion, bool bebe, int direccion, int frame, float golpe, float quemadura) {
    const float escala = bebe ? 0.95f : 1.45f;
    const float anchoVisual = 16.0f * escala;
    const float altoVisual = 22.0f * escala;
    sf::Vector2f origen(posicion.x + (bebe ? -1.0f : 0.5f), posicion.y + (bebe ? -5.0f : -7.0f));

    bool camina = frame > 0;
    int paso = frame % 4;
    int piernaA = (paso == 1 || paso == 2) ? 1 : 0;
    int piernaB = (paso == 3) ? 1 : 0;
    int brazoA = (paso == 1 || paso == 2) ? -1 : 0;
    int brazoB = (paso == 3) ? -1 : 0;

    bool lado = direccion == 1 || direccion == 3;
    bool espejo = direccion == 3;
    bool espalda = direccion == 2;

    sf::Color sombraColor(0, 0, 0, 70);
    sf::RectangleShape sombra({anchoVisual * 0.95f, 4.0f});
    sombra.setPosition({posicion.x + (bebe ? 0.5f : 0.0f), posicion.y + (bebe ? 11.0f : 19.0f)});
    sombra.setFillColor(sombraColor);
    ventana.draw(sombra);

    sf::Color piel = tinteZombie(sf::Color(84, 122, 126), golpe, quemadura);
    sf::Color pielSombra = tinteZombie(sf::Color(54, 82, 92), golpe, quemadura);
    sf::Color pielLuz = tinteZombie(sf::Color(118, 156, 154), golpe, quemadura);
    sf::Color camisa = tinteZombie(sf::Color(78, 68, 104), golpe, quemadura);
    sf::Color camisaLuz = tinteZombie(sf::Color(103, 94, 132), golpe, quemadura);
    sf::Color pantalon = tinteZombie(sf::Color(42, 40, 66), golpe, quemadura);
    sf::Color borde = tinteZombie(sf::Color(24, 21, 33), golpe, quemadura);
    sf::Color ojo = tinteZombie(sf::Color(210, 232, 218), golpe, quemadura);
    sf::Color boca = tinteZombie(sf::Color(68, 36, 50), golpe, quemadura);

    if (lado) {
        dibujarRectZombie(ventana, origen, 5, 1, 6, 2, borde, escala, espejo);
        dibujarRectZombie(ventana, origen, 4, 3, 8, 6, piel, escala, espejo);
        dibujarRectZombie(ventana, origen, 10, 5, 2, 3, pielSombra, escala, espejo);
        dibujarPixelZombie(ventana, origen, 6, 5, ojo, escala, espejo);
        dibujarPixelZombie(ventana, origen, 5, 7, boca, escala, espejo);
        dibujarRectZombie(ventana, origen, 5, 9, 7, 6, camisa, escala, espejo);
        dibujarRectZombie(ventana, origen, 5, 9, 5, 2, camisaLuz, escala, espejo);
        dibujarRectZombie(ventana, origen, 3, 10 + brazoA, 2, 5, piel, escala, espejo);
        dibujarRectZombie(ventana, origen, 11, 10 + brazoB, 2, 5, pielSombra, escala, espejo);
        dibujarRectZombie(ventana, origen, 5, 15, 3, 5 + piernaA, pantalon, escala, espejo);
        dibujarRectZombie(ventana, origen, 9, 15, 3, 5 + piernaB, pantalon, escala, espejo);
        dibujarRectZombie(ventana, origen, 4, 20 + piernaA, 4, 1, borde, escala, espejo);
        dibujarRectZombie(ventana, origen, 9, 20 + piernaB, 4, 1, borde, escala, espejo);
        return;
    }

    dibujarRectZombie(ventana, origen, 4, 1, 8, 2, borde, escala, false);
    dibujarRectZombie(ventana, origen, 3, 3, 10, 6, espalda ? pielSombra : piel, escala, false);
    dibujarRectZombie(ventana, origen, 4, 3, 7, 2, espalda ? piel : pielLuz, escala, false);

    if (!espalda) {
        dibujarPixelZombie(ventana, origen, 5, 5, ojo, escala, false);
        dibujarPixelZombie(ventana, origen, 10, 5, ojo, escala, false);
        dibujarRectZombie(ventana, origen, 6, 7, 4, 1, boca, escala, false);
    } else {
        dibujarRectZombie(ventana, origen, 5, 4, 6, 2, borde, escala, false);
    }

    dibujarRectZombie(ventana, origen, 4, 9, 8, 6, camisa, escala, false);
    dibujarRectZombie(ventana, origen, 5, 9, 6, 2, camisaLuz, escala, false);
    dibujarRectZombie(ventana, origen, 2, 10 + brazoA, 2, 5, pielSombra, escala, false);
    dibujarRectZombie(ventana, origen, 12, 10 + brazoB, 2, 5, piel, escala, false);
    dibujarRectZombie(ventana, origen, 4, 15, 3, 5 + piernaA, pantalon, escala, false);
    dibujarRectZombie(ventana, origen, 9, 15, 3, 5 + piernaB, pantalon, escala, false);
    dibujarRectZombie(ventana, origen, 3, 20 + piernaA, 4, 1, borde, escala, false);
    dibujarRectZombie(ventana, origen, 9, 20 + piernaB, 4, 1, borde, escala, false);

    if (camina) {
        dibujarPixelZombie(ventana, origen, 7, 16, camisaLuz, escala, false);
        dibujarPixelZombie(ventana, origen, 8, 16, camisaLuz, escala, false);
    }
}

struct AnimacionZombieSprite {
    sf::Texture textura;
    int frames = 1;
    int anchoFrame = 1;
    int altoFrame = 1;
    bool cargada = false;
};

struct BancoSpritesZombie {
    std::array<AnimacionZombieSprite, 8> animaciones;
    bool intentado = false;
    bool listo = false;
};

inline BancoSpritesZombie& bancoSpritesZombie() {
    static BancoSpritesZombie banco;
    return banco;
}

inline bool cargarAnimacionZombie(AnimacionZombieSprite& animacion, const char* ruta, int frames) {
    sf::Image imagen;
    if (!imagen.loadFromFile(ruta)) {
        return false;
    }

    if (!animacion.textura.loadFromImage(imagen)) {
        return false;
    }

    animacion.textura.setSmooth(false);
    animacion.frames = frames;
    animacion.anchoFrame = static_cast<int>(imagen.getSize().x) / frames;
    animacion.altoFrame = static_cast<int>(imagen.getSize().y);
    animacion.cargada = animacion.anchoFrame > 0 && animacion.altoFrame > 0;
    return animacion.cargada;
}

inline void prepararSpritesZombie() {
    BancoSpritesZombie& banco = bancoSpritesZombie();
    if (banco.intentado) {
        return;
    }

    banco.intentado = true;
    banco.listo =
        cargarAnimacionZombie(banco.animaciones[0], "assets/textures/down_idle.png", 6) &&
        cargarAnimacionZombie(banco.animaciones[1], "assets/textures/down_walk.png", 8) &&
        cargarAnimacionZombie(banco.animaciones[2], "assets/textures/right_idle.png", 6) &&
        cargarAnimacionZombie(banco.animaciones[3], "assets/textures/right_walk.png", 8) &&
        cargarAnimacionZombie(banco.animaciones[4], "assets/textures/up_idle.png", 6) &&
        cargarAnimacionZombie(banco.animaciones[5], "assets/textures/up_walk.png", 8) &&
        cargarAnimacionZombie(banco.animaciones[6], "assets/textures/left_idle.png", 6) &&
        cargarAnimacionZombie(banco.animaciones[7], "assets/textures/left_walk.png", 8);
}

inline bool dibujarZombieDesdeSprites(sf::RenderWindow& ventana, sf::Vector2f posicion, bool bebe, int direccion, bool caminando, float tiempoSprite, float tiempoAnimacion, float golpe, float quemadura) {
    prepararSpritesZombie();
    BancoSpritesZombie& banco = bancoSpritesZombie();
    if (!banco.listo) {
        return false;
    }

    int base = 0;
    if (direccion == 1) base = 2;
    else if (direccion == 2) base = 4;
    else if (direccion == 3) base = 6;

    AnimacionZombieSprite& animacion = banco.animaciones[base + (caminando ? 1 : 0)];
    if (!animacion.cargada) {
        return false;
    }

    float escala = bebe ? 0.95f : 1.42f;
    int frame = caminando
        ? static_cast<int>(tiempoAnimacion * 1.15f) % animacion.frames
        : static_cast<int>(tiempoSprite * 5.0f) % animacion.frames;

    sf::RectangleShape sombra({static_cast<float>(animacion.anchoFrame) * escala * 0.82f, 4.0f});
    sombra.setOrigin({sombra.getSize().x * 0.5f, 2.0f});
    sombra.setPosition({posicion.x + (bebe ? 7.0f : 12.0f), posicion.y + (bebe ? 15.0f : 24.0f)});
    sombra.setFillColor(sf::Color(0, 0, 0, 75));
    ventana.draw(sombra);

    sf::Sprite sprite(animacion.textura);
    sprite.setTextureRect(sf::IntRect({frame * animacion.anchoFrame, 0}, {animacion.anchoFrame, animacion.altoFrame}));
    sprite.setOrigin({static_cast<float>(animacion.anchoFrame) * 0.5f, static_cast<float>(animacion.altoFrame) - 1.0f});
    sprite.setPosition({posicion.x + (bebe ? 7.0f : 12.0f), posicion.y + (bebe ? 16.0f : 25.0f)});
    sprite.setScale({escala, escala});

    if (golpe > 0.0f) {
        sprite.setColor(sf::Color(255, 108, 108, 255));
    } else if (quemadura > 0.0f) {
        sprite.setColor(sf::Color(255, 158, 92, 245));
    }

    ventana.draw(sprite);
    return true;
}
}

inline Zombie::Zombie(float x, float y, bool bebe)
    : posicion(x, y),
      velocidad(0.0f, 0.0f),
      vida(bebe ? 10.0f : 20.0f),
      vidaMaxima(bebe ? 10.0f : 20.0f),
      velocidadBase(bebe ? 54.0f : 42.0f),
      tiempoAtaque(0.0f),
      tiempoQuemadura(0.0f),
      tiempoLejos(0.0f),
      tiempoAnimacion(0.0f),
      tiempoSprite(0.0f),
      tiempoGolpe(0.0f),
      tiempoAtaqueVisual(0.0f),
      empuje(0.0f, 0.0f),
      direccionMirada(0),
      bebe(bebe),
      vivo(true),
      temporal(true),
      eventoAtaque(false),
      soltarDrop(false) {
    float tam = bebe ? 14.0f : 24.0f;
    forma.setSize({tam, tam});
    forma.setFillColor(bebe ? sf::Color(86, 150, 70) : sf::Color(54, 116, 62));
    forma.setOutlineColor(sf::Color(20, 38, 24));
    forma.setOutlineThickness(2.0f);
    forma.setPosition(posicion);
}

inline Zombie::~Zombie() {}

inline bool Zombie::colisionaConMundo(const Mundo& mundo, sf::Vector2f nuevaPosicion) const {
    const float tam = bebe ? 14.0f : 24.0f;
    const float offsetX = bebe ? 3.0f : 6.0f;
    const float offsetY = bebe ? 7.0f : 14.0f;
    const float ancho = bebe ? 8.0f : 12.0f;
    const float alto = bebe ? 7.0f : 10.0f;

    int izq = static_cast<int>(std::floor((nuevaPosicion.x + offsetX) / TAMANIO_BLOQUE_JUEGO));
    int der = static_cast<int>(std::floor((nuevaPosicion.x + offsetX + ancho - 1.0f) / TAMANIO_BLOQUE_JUEGO));
    int arriba = static_cast<int>(std::floor((nuevaPosicion.y + offsetY) / TAMANIO_BLOQUE_JUEGO));
    int abajo = static_cast<int>(std::floor((nuevaPosicion.y + offsetY + alto - 1.0f) / TAMANIO_BLOQUE_JUEGO));

    for (int y = arriba; y <= abajo; ++y) {
        for (int x = izq; x <= der; ++x) {
            if (mundo.esBloqueSolido(x, y)) {
                return true;
            }
        }
    }
    (void)tam;
    return false;
}

inline bool Zombie::estaEnAgua(const Mundo& mundo) const {
    float tam = bebe ? 14.0f : 24.0f;
    float centroX = posicion.x + tam * 0.5f;
    float centroY = posicion.y + tam * 0.72f;
    int bx = static_cast<int>(std::floor(centroX / TAMANIO_BLOQUE_JUEGO));
    int by = static_cast<int>(std::floor(centroY / TAMANIO_BLOQUE_JUEGO));
    TipoBloque tipo = mundo.getTipoBloque(bx, by);
    return tipo == TipoBloque::Agua || tipo == TipoBloque::AguaProfunda;
}

inline void Zombie::actualizar(float dt, const Mundo& mundo, Jugador& jugador, int skyLight) {
    if (!vivo) {
        return;
    }

    tiempoAtaque = std::max(0.0f, tiempoAtaque - dt);
    tiempoGolpe = std::max(0.0f, tiempoGolpe - dt);
    tiempoAtaqueVisual = std::max(0.0f, tiempoAtaqueVisual - dt);
    tiempoSprite += dt;
    sf::Vector2f centroJugador = jugador.getPosicion() + sf::Vector2f(12.0f, 12.0f);
    float tam = bebe ? 14.0f : 24.0f;
    sf::Vector2f centroZombie = posicion + sf::Vector2f(tam * 0.5f, tam * 0.5f);
    sf::Vector2f delta = centroJugador - centroZombie;
    float distancia = std::sqrt(delta.x * delta.x + delta.y * delta.y);

    if (distancia > 128.0f * TAMANIO_BLOQUE_JUEGO && temporal) {
        vivo = false;
        return;
    }

    if (distancia > 32.0f * TAMANIO_BLOQUE_JUEGO) {
        tiempoLejos += dt;
        if (tiempoLejos > 30.0f) {
            unsigned int azar = static_cast<unsigned int>(posicion.x * 13.0f + posicion.y * 17.0f + tiempoLejos * 31.0f);
            if ((azar % 40u) == 0u) {
                vivo = false;
                return;
            }
        }
    } else {
        tiempoLejos = 0.0f;
    }

    if (!bebe && skyLight > 10 && !estaEnAgua(mundo)) {
        tiempoQuemadura += dt;
        if (tiempoQuemadura >= 1.0f) {
            vida -= 1.0f;
            tiempoQuemadura = 0.0f;
            if (vida <= 0.0f) {
                soltarDrop = true;
                vivo = false;
                return;
            }
        }
    } else {
        tiempoQuemadura = 0.0f;
    }

    if (distancia < 35.0f * TAMANIO_BLOQUE_JUEGO && distancia > 1.0f) {
        sf::Vector2f direccion = delta / distancia;
        velocidad = direccion * velocidadBase;
        if (std::abs(velocidad.x) > std::abs(velocidad.y)) {
            direccionMirada = velocidad.x >= 0.0f ? 1 : 3;
        } else {
            direccionMirada = velocidad.y < 0.0f ? 2 : 0;
        }
    } else {
        velocidad = {0.0f, 0.0f};
    }

    if (std::abs(velocidad.x) + std::abs(velocidad.y) > 0.1f) {
        tiempoAnimacion += dt * (bebe ? 11.0f : 8.0f);
    } else {
        tiempoAnimacion = 0.0f;
    }

    if (distancia <= 1.5f * TAMANIO_BLOQUE_JUEGO && tiempoAtaque <= 0.0f) {
        int danio = bebe ? 2 : 3;
        jugador.recibirDanio(danio);
        jugador.aplicarEmpuje(delta, bebe ? 10.0f : 14.0f, mundo);
        tiempoAtaque = 1.0f;
        tiempoAtaqueVisual = 0.24f;
        eventoAtaque = true;
    }

    if (std::abs(empuje.x) + std::abs(empuje.y) > 0.1f) {
        sf::Vector2f nuevaEmpujeX = posicion + sf::Vector2f(empuje.x * dt, 0.0f);
        if (!colisionaConMundo(mundo, nuevaEmpujeX)) {
            posicion.x = nuevaEmpujeX.x;
        }
        sf::Vector2f nuevaEmpujeY = posicion + sf::Vector2f(0.0f, empuje.y * dt);
        if (!colisionaConMundo(mundo, nuevaEmpujeY)) {
            posicion.y = nuevaEmpujeY.y;
        }
        empuje *= std::pow(0.03f, dt);
    }

    sf::Vector2f nuevaX = posicion + sf::Vector2f(velocidad.x * dt, 0.0f);
    if (!colisionaConMundo(mundo, nuevaX)) {
        posicion.x = nuevaX.x;
    }

    sf::Vector2f nuevaY = posicion + sf::Vector2f(0.0f, velocidad.y * dt);
    if (!colisionaConMundo(mundo, nuevaY)) {
        posicion.y = nuevaY.y;
    }

    forma.setPosition(posicion);
}

inline void Zombie::dibujar(sf::RenderWindow& ventana) {
    if (!vivo) {
        return;
    }

    float tam = bebe ? 14.0f : 24.0f;
    bool caminando = std::abs(velocidad.x) + std::abs(velocidad.y) > 0.1f;
    if (dibujarZombieDesdeSprites(ventana, posicion, bebe, direccionMirada, caminando, tiempoSprite, tiempoAnimacion, tiempoGolpe, tiempoQuemadura)) {
        if (tiempoAtaqueVisual > 0.0f) {
            sf::RectangleShape golpe({bebe ? 9.0f : 13.0f, 3.0f});
            golpe.setFillColor(sf::Color(72, 32, 58, 180));
            sf::Vector2f centro = posicion + sf::Vector2f(tam * 0.5f, tam * 0.55f);
            if (direccionMirada == 1) {
                golpe.setPosition({centro.x + tam * 0.28f, centro.y});
            } else if (direccionMirada == 3) {
                golpe.setPosition({centro.x - tam * 0.72f, centro.y});
            } else if (direccionMirada == 2) {
                golpe.setPosition({centro.x - 6.0f, centro.y - tam * 0.6f});
                golpe.setSize({3.0f, bebe ? 9.0f : 13.0f});
            } else {
                golpe.setPosition({centro.x - 6.0f, centro.y + tam * 0.22f});
                golpe.setSize({3.0f, bebe ? 9.0f : 13.0f});
            }
            ventana.draw(golpe);
        }
        if (vida < vidaMaxima) {
            sf::RectangleShape fondo({tam, 3.0f});
            fondo.setPosition({posicion.x, posicion.y - 6.0f});
            fondo.setFillColor(sf::Color(40, 8, 8, 180));
            ventana.draw(fondo);

            sf::RectangleShape barra({tam * std::max(0.0f, vida / vidaMaxima), 3.0f});
            barra.setPosition({posicion.x, posicion.y - 6.0f});
            barra.setFillColor(sf::Color(170, 32, 32));
            ventana.draw(barra);
        }
        return;
    }

    int frame = static_cast<int>(tiempoAnimacion) % 4;
    if (tiempoAnimacion <= 0.0f) {
        frame = 0;
    }
    dibujarZombiePixelArt(ventana, posicion, bebe, direccionMirada, frame, tiempoGolpe, tiempoQuemadura);

    if (vida < vidaMaxima) {
        sf::RectangleShape fondo({tam, 3.0f});
        fondo.setPosition({posicion.x, posicion.y - 6.0f});
        fondo.setFillColor(sf::Color(40, 8, 8, 180));
        ventana.draw(fondo);

        sf::RectangleShape barra({tam * std::max(0.0f, vida / vidaMaxima), 3.0f});
        barra.setPosition({posicion.x, posicion.y - 6.0f});
        barra.setFillColor(sf::Color(170, 32, 32));
        ventana.draw(barra);
    }
}

inline void Zombie::recibirDanio(float danio) {
    recibirDanio(danio, posicion);
}

inline void Zombie::recibirDanio(float danio, sf::Vector2f origenDanio) {
    if (danio <= 0.0f || !vivo) {
        return;
    }
    float danioFinal = danio * 0.92f;
    vida -= danioFinal;
    tiempoGolpe = 0.18f;
    sf::Vector2f centro = posicion + sf::Vector2f((bebe ? 7.0f : 12.0f), (bebe ? 7.0f : 12.0f));
    sf::Vector2f direccion = centro - origenDanio;
    float longitud = std::sqrt(direccion.x * direccion.x + direccion.y * direccion.y);
    if (longitud > 0.01f) {
        direccion /= longitud;
        empuje += direccion * (bebe ? 105.0f : 82.0f);
    }
    if (vida <= 0.0f) {
        soltarDrop = true;
        vivo = false;
    }
}

inline bool Zombie::estaVivo() const {
    return vivo;
}

inline bool Zombie::debeEliminarse() const {
    return !vivo;
}

inline bool Zombie::debeSoltarDrop() const {
    return soltarDrop;
}

inline bool Zombie::consumirEventoAtaque() {
    bool evento = eventoAtaque;
    eventoAtaque = false;
    return evento;
}

inline bool Zombie::esBebe() const {
    return bebe;
}

inline bool Zombie::contienePunto(sf::Vector2f punto) const {
    return forma.getGlobalBounds().contains(punto);
}

inline sf::Vector2f Zombie::getPosicion() const {
    return posicion;
}
