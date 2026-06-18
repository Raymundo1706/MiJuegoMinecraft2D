#include "../../core/Mundo.hpp"
#include "../../core/Item.hpp"
#include <algorithm>
#include <cstdint>
#include <random>
#include <cmath>

namespace {
inline bool esComidaCerdo(ItemId item) {
    return item == ItemId::Zanahoria ||
           item == ItemId::Patata ||
           item == ItemId::Remolacha;
}

inline int indiceAnimal(TipoAnimal tipo) {
    switch (tipo) {
        case TipoAnimal::Cerdo: return 0;
        case TipoAnimal::Vaca: return 1;
        case TipoAnimal::Oveja: return 2;
        case TipoAnimal::Gallina: return 3;
    }
    return 0;
}

inline const char* rutaAnimal(TipoAnimal tipo) {
    switch (tipo) {
        case TipoAnimal::Cerdo: return "assets/textures/animal_pig.png";
        case TipoAnimal::Vaca: return "assets/textures/animal_cow.png";
        case TipoAnimal::Oveja: return "assets/textures/animal_sheep.png";
        case TipoAnimal::Gallina: return "assets/textures/animal_chicken.png";
    }
    return "assets/textures/animal_pig.png";
}

inline float vidaAnimal(TipoAnimal tipo) {
    switch (tipo) {
        case TipoAnimal::Cerdo: return 10.0f;
        case TipoAnimal::Vaca: return 10.0f;
        case TipoAnimal::Oveja: return 8.0f;
        case TipoAnimal::Gallina: return 4.0f;
    }
    return 8.0f;
}

inline float tamAnimal(TipoAnimal tipo) {
    return tipo == TipoAnimal::Gallina ? 18.0f : 28.8f;
}

inline float velocidadBaseAnimal(TipoAnimal tipo) {
    return tipo == TipoAnimal::Gallina ? 18.0f : 10.0f;
}

inline bool animalTocaAgua(const Mundo& mundo, sf::Vector2f posicionAnimal, float ancho, float alto) {
    const float TAMANIO_BLOQUE = TAMANIO_BLOQUE_JUEGO;

    auto puntoEsAgua = [&](float px, float py) {
        int bloqueX = static_cast<int>(std::floor(px / TAMANIO_BLOQUE));
        int bloqueY = static_cast<int>(std::floor(py / TAMANIO_BLOQUE));
        TipoBloque tipo = mundo.getTipoBloque(bloqueX, bloqueY);
        return tipo == TipoBloque::Agua || tipo == TipoBloque::AguaProfunda;
    };

    float margenX = std::max(2.0f, ancho * 0.18f);
    float izquierda = posicionAnimal.x + margenX;
    float derecha = posicionAnimal.x + ancho - margenX;
    float centroX = posicionAnimal.x + ancho * 0.5f;
    float centroY = posicionAnimal.y + alto * 0.55f;
    float patasY = posicionAnimal.y + alto - 2.0f;

    if (puntoEsAgua(centroX, centroY) || puntoEsAgua(centroX, patasY)) {
        return true;
    }

    float muestrasLaterales[2] = {izquierda, derecha};
    for (float x : muestrasLaterales) {
        if (puntoEsAgua(x, centroY) || puntoEsAgua(x, patasY)) {
            return true;
        }
    }
    return false;
}
}

inline Animal::Animal(float x, float y, TipoAnimal tipo) 
    : posicion(x, y),
      tipo(tipo),
      tiempoCambiandoDireccion(0.0f),
      tiempoAnimacion(0.0f),
      vidaMaxima(vidaAnimal(tipo)),
      vida(vidaMaxima),
      tiempoPanico(0.0f),
      anchoAnimal(tamAnimal(tipo)),
      altoAnimal(tamAnimal(tipo)),
      mirandoDerecha(false),
      muriendo(false),
      tiempoMuerte(0.0f),
      tiempoGolpe(0.0f),
      tieneAmenaza(false),
      posicionAmenaza(0.0f, 0.0f),
      tiempoParticulas(0.0f),
      enAgua(false),
      hundido(false),
      tiempoEnAgua(0.0f),
      tiempoHundimiento(0.0f) {
    
    forma.setSize({anchoAnimal, altoAnimal});
    
    forma.setFillColor(sf::Color(240, 240, 240));

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
    float velocidadCaminado = velocidadBaseAnimal(tipo);
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
        tiempoParticulas += dt;
        velocidad = {0.0f, 0.0f};
        return;
    }

    if (vida <= 0.0f) return;

    enAgua = animalTocaAgua(mundo, posicion, anchoAnimal, altoAnimal);
    if (enAgua) {
        tiempoEnAgua += dt;
        if (tiempoEnAgua >= 20.0f) {
            hundido = true;
        }
    } else {
        tiempoEnAgua = 0.0f;
        tiempoHundimiento = 0.0f;
        hundido = false;
    }

    if (hundido) {
        tiempoHundimiento += dt;
        velocidad = {0.0f, 0.0f};
        tiempoAnimacion = 0.0f;
        return;
    }

    if (tiempoPanico > 0.0f) {
        tiempoPanico = std::max(0.0f, tiempoPanico - dt);
    }

    if (tiempoGolpe > 0.0f) {
        tiempoGolpe = std::max(0.0f, tiempoGolpe - dt);
    }

    if (std::abs(velocidad.x) + std::abs(velocidad.y) > 0.1f) {
        tiempoAnimacion += dt * (tiempoPanico > 0.0f ? 2.4f : 1.0f);
    } else {
        tiempoAnimacion = 0.0f;
    }

    bool huyendo = false;
    if (tiempoPanico > 0.0f && tieneAmenaza) {
        sf::Vector2f centroAnimal = posicion + sf::Vector2f(anchoAnimal * 0.5f, altoAnimal * 0.5f);
        sf::Vector2f direccionHuida = centroAnimal - posicionAmenaza;
        float distancia = std::sqrt(direccionHuida.x * direccionHuida.x + direccionHuida.y * direccionHuida.y);

        if (distancia > 0.01f) {
            direccionHuida /= distancia;
            float velocidadHuida = velocidadBaseAnimal(tipo) * 7.0f;
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

        if (distancia < 8.0f * TAMANIO_BLOQUE_JUEGO && distancia > 1.0f) {
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

    const float TAMANIO_BLOQUE = TAMANIO_BLOQUE_JUEGO;
    float factorAgua = enAgua ? 0.33f : 1.0f;
    sf::Vector2f velocidadMovimiento = velocidad * factorAgua;

    // --- COLISIÓN EN EJE X ---
    posicion.x += velocidadMovimiento.x * dt;
    
    int blqIzq = static_cast<int>(posicion.x / TAMANIO_BLOQUE);
    int blqDer = static_cast<int>((posicion.x + anchoAnimal - 1.0f) / TAMANIO_BLOQUE);
    int blqArribaY = static_cast<int>(posicion.y / TAMANIO_BLOQUE);
    int blqAbajoY = static_cast<int>((posicion.y + altoAnimal - 1.0f) / TAMANIO_BLOQUE);

    if (velocidadMovimiento.x > 0) { // Moviéndose a la derecha
        if (mundo.esBloqueSolido(blqDer, blqArribaY) || mundo.esBloqueSolido(blqDer, blqAbajoY)) {
            posicion.x = blqDer * TAMANIO_BLOQUE - anchoAnimal - 0.1f;
            elegirNuevaDireccion(); // Cambia de rumbo si choca
        }
    } else if (velocidadMovimiento.x < 0) { // Moviéndose a la izquierda
        if (mundo.esBloqueSolido(blqIzq, blqArribaY) || mundo.esBloqueSolido(blqIzq, blqAbajoY)) {
            posicion.x = (blqIzq + 1) * TAMANIO_BLOQUE + 0.1f;
            elegirNuevaDireccion();
        }
    }

    // --- COLISIÓN EN EJE Y ---
    posicion.y += velocidadMovimiento.y * dt;
    
    blqIzq = static_cast<int>(posicion.x / TAMANIO_BLOQUE);
    blqDer = static_cast<int>((posicion.x + anchoAnimal - 1.0f) / TAMANIO_BLOQUE);
    int blqArriba = static_cast<int>(posicion.y / TAMANIO_BLOQUE);
    int blqAbajo = static_cast<int>((posicion.y + altoAnimal - 1.0f) / TAMANIO_BLOQUE);

    if (velocidadMovimiento.y > 0) { // Moviéndose hacia abajo
        if (mundo.esBloqueSolido(blqIzq, blqAbajo) || mundo.esBloqueSolido(blqDer, blqAbajo)) {
            posicion.y = blqAbajo * TAMANIO_BLOQUE - altoAnimal - 0.1f;
            elegirNuevaDireccion();
        }
    } else if (velocidadMovimiento.y < 0) { // Moviéndose hacia arriba
        if (mundo.esBloqueSolido(blqIzq, blqArriba) || mundo.esBloqueSolido(blqDer, blqArriba)) {
            posicion.y = (blqArriba + 1) * TAMANIO_BLOQUE + 0.1f;
            elegirNuevaDireccion();
        }
    }
}

inline void Animal::dibujar(sf::RenderWindow& ventana) {
    if (vida <= 0.0f && !muriendo) return;

    dibujarAnimal(ventana);
}

inline void Animal::dibujarAnimal(sf::RenderWindow& ventana) {
    static bool texturasListas[4] = {false, false, false, false};
    static sf::Texture texturas[4];
    int idx = indiceAnimal(tipo);

    if (!texturasListas[idx]) {
        texturasListas[idx] = texturas[idx].loadFromFile(rutaAnimal(tipo));
        texturas[idx].setSmooth(false);
    }

    if (muriendo && tiempoMuerte >= 0.72f) {
        dibujarParticulasMuerte(ventana);
        return;
    }

    bool usandoTextura = texturasListas[idx];
    if (usandoTextura) {
        bool moviendose = std::abs(velocidad.x) + std::abs(velocidad.y) > 0.1f;
        int columna = moviendose ? static_cast<int>(tiempoAnimacion * 8.0f) % 2 : 0;
        int fila = moviendose ? 1 : 0;
        float hundimientoVisual = enAgua ? altoAnimal * 0.18f : 0.0f;
        if (hundido) {
            hundimientoVisual = std::min(altoAnimal * 0.85f, altoAnimal * 0.18f + tiempoHundimiento * 10.0f);
        }

        sf::Sprite sprite(texturas[idx]);
        sprite.setTextureRect(sf::IntRect({columna * 32, fila * 32}, {32, 32}));
        sprite.setOrigin({16.0f, 24.0f});
        sprite.setPosition({posicion.x + anchoAnimal * 0.5f, posicion.y + altoAnimal + hundimientoVisual});
        float escala = tipo == TipoAnimal::Gallina ? 0.95f : 1.15f;
        sprite.setScale({mirandoDerecha ? -escala : escala, escala});

        float sacudida = tiempoGolpe > 0.0f ? std::sin(tiempoGolpe * 80.0f) * 2.0f : 0.0f;
        sprite.move({sacudida, 0.0f});

        if (muriendo) {
            sprite.setRotation(sf::degrees(mirandoDerecha ? -85.0f : 85.0f));
            sprite.setColor(sf::Color(255, 85, 85, 220));
        } else if (hundido) {
            float alpha = std::max(45.0f, 255.0f - tiempoHundimiento * 90.0f);
            sprite.setColor(sf::Color(185, 220, 255, static_cast<std::uint8_t>(alpha)));
        } else if (enAgua) {
            sprite.setColor(sf::Color(215, 238, 255, 245));
        } else if (tiempoGolpe > 0.0f) {
            sprite.setColor(sf::Color(255, 120, 120, 255));
        }

        ventana.draw(sprite);

        if (enAgua && !muriendo) {
            float alturaAgua = hundido ? altoAnimal * 0.72f : altoAnimal * 0.35f;
            float yAgua = posicion.y + (hundido ? altoAnimal * 0.22f : altoAnimal * 0.58f);
            sf::RectangleShape laminaAgua({anchoAnimal + 4.0f, alturaAgua});
            laminaAgua.setPosition({posicion.x - 2.0f, yAgua});
            laminaAgua.setFillColor(sf::Color(55, 155, 230, hundido ? 155 : 115));
            ventana.draw(laminaAgua);

            sf::RectangleShape brillo({anchoAnimal - 2.0f, 2.0f});
            brillo.setPosition({posicion.x + 1.0f, yAgua});
            brillo.setFillColor(sf::Color(135, 215, 255, hundido ? 145 : 125));
            ventana.draw(brillo);
        }
    } else {
        forma.setPosition({posicion.x, posicion.y + (enAgua ? altoAnimal * 0.18f : 0.0f)});
        ventana.draw(forma);
    }

    if (!usandoTextura && tiempoGolpe > 0.0f && !muriendo) {
        sf::RectangleShape impacto({anchoAnimal, altoAnimal});
        impacto.setPosition(posicion);
        impacto.setFillColor(sf::Color(255, 255, 255, 70));
        ventana.draw(impacto);
    }

    if (!usandoTextura && tiempoPanico > 0.0f) {
        sf::RectangleShape brillo({anchoAnimal, altoAnimal});
        brillo.setPosition(posicion);
        brillo.setFillColor(sf::Color(255, 60, 60, 55));
        ventana.draw(brillo);
    }
}

inline void Animal::dibujarParticulasMuerte(sf::RenderWindow& ventana) {
    float t = std::min(1.0f, (tiempoMuerte - 0.72f) / 0.42f);
    sf::Vector2f centro = posicion + sf::Vector2f(anchoAnimal * 0.5f, altoAnimal * 0.5f);
    sf::Color colores[3] = {
        sf::Color(255, 95, 95, static_cast<std::uint8_t>(220 * (1.0f - t))),
        sf::Color(255, 210, 160, static_cast<std::uint8_t>(200 * (1.0f - t))),
        sf::Color(235, 235, 235, static_cast<std::uint8_t>(170 * (1.0f - t)))
    };

    for (int i = 0; i < 9; ++i) {
        float angulo = static_cast<float>(i) * 0.698f;
        float distancia = 4.0f + t * 18.0f + static_cast<float>(i % 3) * 2.0f;
        sf::RectangleShape p({3.0f, 3.0f});
        p.setPosition({
            centro.x + std::cos(angulo) * distancia,
            centro.y + std::sin(angulo) * distancia - t * 8.0f
        });
        p.setFillColor(colores[i % 3]);
        ventana.draw(p);
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
            float velocidadHuida = velocidadBaseAnimal(tipo) * 7.0f;
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
    return muriendo && tiempoMuerte >= 1.2f;
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

