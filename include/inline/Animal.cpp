#include "Mundo.hpp"
#include "Item.hpp"
#include <algorithm>
#include <random>
#include <cmath>

namespace {
inline bool esComidaCerdo(ItemId item) {
    return item == ItemId::Zanahoria ||
           item == ItemId::Patata ||
           item == ItemId::Remolacha;
}
}

inline Animal::Animal(float x, float y, TipoAnimal tipo) 
    : posicion(x, y),
      tipo(tipo),
      tiempoCambiandoDireccion(0.0f),
      vidaMaxima(tipo == TipoAnimal::Cerdo ? 10.0f : 8.0f),
      vida(vidaMaxima),
      tiempoPanico(0.0f),
      anchoAnimal(tipo == TipoAnimal::Cerdo ? 28.8f : 20.0f),
      altoAnimal(tipo == TipoAnimal::Cerdo ? 28.8f : 20.0f),
      mirandoDerecha(false),
      muriendo(false),
      tiempoMuerte(0.0f),
      tiempoGolpe(0.0f),
      tieneAmenaza(false),
      posicionAmenaza(0.0f, 0.0f) {
    
    forma.setSize({anchoAnimal, altoAnimal});
    
    // Color segÃºn la criatura estilo Minecraft
    if (tipo == TipoAnimal::Cerdo) {
        forma.setFillColor(sf::Color(255, 105, 180)); // Rosa Cerdito
    } else {
        forma.setFillColor(sf::Color(240, 240, 240)); // Blanco Oveja
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(1.5f, 4.0f);
    tiempoMaximoDireccion = dis(gen);

    elegirNuevaDireccion();
}

inline Animal::~Animal() {}

inline void Animal::elegirNuevaDireccion() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> disDir(0, 4); // 0=Quieto, 1=Izquierda, 2=Derecha, 3=Arriba, 4=Abajo
    std::uniform_real_distribution<> disTiempo(2.0f, 5.0f);

    int dir = disDir(gen);
    float velocidadCaminado = (tipo == TipoAnimal::Cerdo) ? 8.0f : 40.0f;
    if (tiempoPanico > 0.0f) {
        velocidadCaminado *= 1.5f;
    }

    if (dir == 0) velocidad = {0.0f, 0.0f};
    else if (dir == 1) velocidad = {-velocidadCaminado, 0.0f};
    else if (dir == 2) velocidad = {velocidadCaminado, 0.0f};
    else if (dir == 3) velocidad = {0.0f, -velocidadCaminado};
    else if (dir == 4) velocidad = {0.0f, velocidadCaminado};

    if (velocidad.x > 0.0f) mirandoDerecha = true;
    if (velocidad.x < 0.0f) mirandoDerecha = false;

    tiempoCambiandoDireccion = 0.0f;
    tiempoMaximoDireccion = disTiempo(gen);
}

inline void Animal::actualizar(float dt, const Mundo& mundo) {
    actualizar(dt, mundo, {-99999.0f, -99999.0f}, ItemId::Ninguno);
}

inline void Animal::actualizar(float dt, const Mundo& mundo, sf::Vector2f posicionJugador, ItemId itemJugador) {
    if (muriendo) {
        tiempoMuerte += dt;
        velocidad = {0.0f, 0.0f};
        return;
    }

    if (vida <= 0.0f) return;

    if (tiempoPanico > 0.0f) {
        tiempoPanico = std::max(0.0f, tiempoPanico - dt);
    }

    if (tiempoGolpe > 0.0f) {
        tiempoGolpe = std::max(0.0f, tiempoGolpe - dt);
    }

    bool huyendo = false;
    if (tiempoPanico > 0.0f && tieneAmenaza) {
        sf::Vector2f centroAnimal = posicion + sf::Vector2f(anchoAnimal * 0.5f, altoAnimal * 0.5f);
        sf::Vector2f direccionHuida = centroAnimal - posicionAmenaza;
        float distancia = std::sqrt(direccionHuida.x * direccionHuida.x + direccionHuida.y * direccionHuida.y);

        if (distancia > 0.01f) {
            direccionHuida /= distancia;
            float velocidadHuida = (tipo == TipoAnimal::Cerdo ? 64.0f : 70.0f);
            velocidad = direccionHuida * velocidadHuida;
            if (velocidad.x > 0.0f) mirandoDerecha = true;
            if (velocidad.x < 0.0f) mirandoDerecha = false;
            huyendo = true;
        }
    }

    bool atraidoPorComida = false;
    if (!huyendo && tipo == TipoAnimal::Cerdo && tiempoPanico <= 0.0f && esComidaCerdo(itemJugador)) {
        sf::Vector2f centroAnimal = posicion + sf::Vector2f(anchoAnimal * 0.5f, altoAnimal * 0.5f);
        sf::Vector2f delta = posicionJugador - centroAnimal;
        float distancia = std::sqrt(delta.x * delta.x + delta.y * delta.y);

        if (distancia < 8.0f * 32.0f && distancia > 1.0f) {
            sf::Vector2f direccion = delta / distancia;
            velocidad = direccion * 8.0f;
            if (velocidad.x > 0.0f) mirandoDerecha = true;
            if (velocidad.x < 0.0f) mirandoDerecha = false;
            atraidoPorComida = true;
        }
    }

    if (!atraidoPorComida && !huyendo) {
        tiempoCambiandoDireccion += dt;
        float limiteCambio = tiempoPanico > 0.0f ? 0.45f : tiempoMaximoDireccion;
        if (tiempoCambiandoDireccion >= limiteCambio) {
            elegirNuevaDireccion();
        }
    }

    const float TAMANIO_BLOQUE = 32.0f;

    // --- COLISIÃ“N EN EJE X ---
    posicion.x += velocidad.x * dt;
    
    int blqIzq = static_cast<int>(posicion.x / TAMANIO_BLOQUE);
    int blqDer = static_cast<int>((posicion.x + anchoAnimal - 1.0f) / TAMANIO_BLOQUE);
    int blqArribaY = static_cast<int>(posicion.y / TAMANIO_BLOQUE);
    int blqAbajoY = static_cast<int>((posicion.y + altoAnimal - 1.0f) / TAMANIO_BLOQUE);

    if (velocidad.x > 0) { // MoviÃ©ndose a la derecha
        if (mundo.esBloqueSolido(blqDer, blqArribaY) || mundo.esBloqueSolido(blqDer, blqAbajoY)) {
            posicion.x = blqDer * TAMANIO_BLOQUE - anchoAnimal - 0.1f;
            elegirNuevaDireccion(); // Cambia de rumbo si choca
        }
    } else if (velocidad.x < 0) { // MoviÃ©ndose a la izquierda
        if (mundo.esBloqueSolido(blqIzq, blqArribaY) || mundo.esBloqueSolido(blqIzq, blqAbajoY)) {
            posicion.x = (blqIzq + 1) * TAMANIO_BLOQUE + 0.1f;
            elegirNuevaDireccion();
        }
    }

    // --- COLISIÃ“N EN EJE Y ---
    posicion.y += velocidad.y * dt;
    
    blqIzq = static_cast<int>(posicion.x / TAMANIO_BLOQUE);
    blqDer = static_cast<int>((posicion.x + anchoAnimal - 1.0f) / TAMANIO_BLOQUE);
    int blqArriba = static_cast<int>(posicion.y / TAMANIO_BLOQUE);
    int blqAbajo = static_cast<int>((posicion.y + altoAnimal - 1.0f) / TAMANIO_BLOQUE);

    if (velocidad.y > 0) { // MoviÃ©ndose hacia abajo
        if (mundo.esBloqueSolido(blqIzq, blqAbajo) || mundo.esBloqueSolido(blqDer, blqAbajo)) {
            posicion.y = blqAbajo * TAMANIO_BLOQUE - altoAnimal - 0.1f;
            elegirNuevaDireccion();
        }
    } else if (velocidad.y < 0) { // MoviÃ©ndose hacia arriba
        if (mundo.esBloqueSolido(blqIzq, blqArriba) || mundo.esBloqueSolido(blqDer, blqArriba)) {
            posicion.y = (blqArriba + 1) * TAMANIO_BLOQUE + 0.1f;
            elegirNuevaDireccion();
        }
    }
}

inline void Animal::dibujar(sf::RenderWindow& ventana) {
    if (vida <= 0.0f && !muriendo) return;

    if (tipo == TipoAnimal::Cerdo) {
        dibujarCerdo(ventana);
        return;
    }

    forma.setPosition(posicion);
    ventana.draw(forma);
}

inline void Animal::dibujarCerdo(sf::RenderWindow& ventana) {
    static bool texturaLista = false;
    static sf::Texture texturaCerdo;

    if (!texturaLista) {
        sf::Image imagen({20, 16}, sf::Color::Transparent);
        const sf::Color rosaBase(226, 121, 119);
        const sf::Color rosaClaro(247, 157, 157);
        const sf::Color rosaOscuro(206, 92, 96);
        const sf::Color sombra(176, 73, 81);

        auto pixel = [&](int x, int y, sf::Color color) {
            if (x >= 0 && x < 20 && y >= 0 && y < 16) {
                imagen.setPixel(sf::Vector2u(static_cast<unsigned int>(x), static_cast<unsigned int>(y)), color);
            }
        };

        auto rect = [&](int x, int y, int w, int h, sf::Color color) {
            for (int py = y; py < y + h; ++py) {
                for (int px = x; px < x + w; ++px) {
                    pixel(px, py, color);
                }
            }
        };

        rect(3, 5, 15, 7, rosaBase);
        rect(5, 3, 10, 3, rosaClaro);
        rect(2, 8, 4, 4, rosaOscuro);
        rect(6, 10, 11, 3, rosaBase);
        rect(5, 12, 3, 2, sombra);
        rect(14, 12, 3, 2, sombra);
        rect(4, 2, 2, 3, rosaBase);
        rect(13, 2, 2, 3, rosaBase);
        rect(2, 9, 2, 2, rosaClaro);
        rect(17, 8, 2, 2, rosaBase);
        rect(19, 9, 1, 1, rosaClaro);
        pixel(4, 7, sf::Color::Black);
        pixel(8, 7, sf::Color::Black);
        pixel(5, 9, rosaClaro);
        pixel(6, 9, rosaClaro);
        pixel(5, 10, sombra);
        pixel(6, 10, sombra);

        texturaLista = texturaCerdo.loadFromImage(imagen);
        texturaCerdo.setSmooth(false);
    }

    if (texturaLista) {
        sf::Sprite cerdo(texturaCerdo);
        float sacudida = tiempoGolpe > 0.0f ? std::sin(tiempoGolpe * 80.0f) * 2.0f : 0.0f;
        if (muriendo) {
            cerdo.setOrigin({10.0f, 8.0f});
            cerdo.setPosition({posicion.x + anchoAnimal * 0.5f, posicion.y + altoAnimal * 0.5f});
            cerdo.setScale({2.0f, 2.0f});
            cerdo.setRotation(sf::degrees(mirandoDerecha ? -90.0f : 90.0f));
            cerdo.setColor(sf::Color(255, 70, 70, 210));
        } else if (mirandoDerecha) {
            cerdo.setPosition({posicion.x - 2.0f + 40.0f + sacudida, posicion.y + 4.0f});
            cerdo.setScale({-2.0f, 2.0f});
        } else {
            cerdo.setPosition({posicion.x - 2.0f + sacudida, posicion.y + 4.0f});
            cerdo.setScale({2.0f, 2.0f});
        }
        if (tiempoGolpe > 0.0f && !muriendo) {
            cerdo.setColor(sf::Color(255, 95, 95, 255));
        }
        ventana.draw(cerdo);
    }

    if (tiempoGolpe > 0.0f && !muriendo) {
        sf::RectangleShape impacto({anchoAnimal, altoAnimal});
        impacto.setPosition(posicion);
        impacto.setFillColor(sf::Color(255, 255, 255, 70));
        ventana.draw(impacto);
    }

    if (tiempoPanico > 0.0f) {
        sf::RectangleShape brillo({anchoAnimal, altoAnimal});
        brillo.setPosition(posicion);
        brillo.setFillColor(sf::Color(255, 60, 60, 55));
        ventana.draw(brillo);
    }
}

inline void Animal::recibirDanio(float danio) {
    recibirDanio(danio, posicion);
}

inline void Animal::recibirDanio(float danio, sf::Vector2f origenDanio) {
    if (muriendo || vida <= 0.0f || danio <= 0.0f) return;
    vida = std::max(0.0f, vida - danio);
    tiempoGolpe = 0.22f;
    if (vida <= 0.0f) {
        muriendo = true;
        tiempoMuerte = 0.0f;
        tiempoPanico = 0.0f;
        velocidad = {0.0f, 0.0f};
    } else {
        tiempoPanico = 8.0f;
        tieneAmenaza = true;
        posicionAmenaza = origenDanio;
        sf::Vector2f centroAnimal = posicion + sf::Vector2f(anchoAnimal * 0.5f, altoAnimal * 0.5f);
        sf::Vector2f direccionHuida = centroAnimal - origenDanio;
        float distancia = std::sqrt(direccionHuida.x * direccionHuida.x + direccionHuida.y * direccionHuida.y);

        if (distancia > 0.01f) {
            direccionHuida /= distancia;
            float velocidadHuida = (tipo == TipoAnimal::Cerdo ? 64.0f : 70.0f);
            velocidad = direccionHuida * velocidadHuida;
            if (velocidad.x > 0.0f) mirandoDerecha = true;
            if (velocidad.x < 0.0f) mirandoDerecha = false;
            tiempoCambiandoDireccion = 0.0f;
        } else {
            elegirNuevaDireccion();
        }
    }
}

inline bool Animal::estaVivo() const {
    return vida > 0.0f && !muriendo;
}

inline bool Animal::estaMuriendo() const {
    return muriendo;
}

inline bool Animal::muerteFinalizada() const {
    return muriendo && tiempoMuerte >= 0.85f;
}

inline bool Animal::contienePunto(sf::Vector2f punto) const {
    return punto.x >= posicion.x &&
           punto.x <= posicion.x + anchoAnimal &&
           punto.y >= posicion.y &&
           punto.y <= posicion.y + altoAnimal;
}

inline TipoAnimal Animal::getTipo() const {
    return tipo;
}

inline sf::Vector2f Animal::getPosicion() const {
    return posicion;
}

