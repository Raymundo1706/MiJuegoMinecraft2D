# Maincra 2-D

Proyecto desarrollado por **toñoñongos.ing** para la materia **Proyecto 252**.

**Maincra 2-D** es un juego sandbox de supervivencia en 2D inspirado en **Minecraft Xbox 360 Edition**, adaptado a una perspectiva cenital. El juego incluye exploración, recolección, crafteo, minería, aldeas, animales, enemigos, inventario, día/noche, cuevas, iluminación, cofres, trading y guardado del mundo.

## Descripcion del Proyecto

**Maincra 2-D** es un juego de supervivencia, exploracion y construccion inspirado en la experiencia clasica de Minecraft, pero adaptado a un mundo 2D con vista cenital.

El jugador puede explorar un mundo abierto, recolectar recursos, fabricar herramientas, construir bloques, interactuar con animales, sobrevivir durante la noche, explorar cuevas, encontrar aldeas y comerciar con aldeanos.

El objetivo principal es recrear una experiencia similar a **Minecraft Xbox 360 Edition**, pero con una adaptacion propia en 2D, usando programacion orientada a objetos en C++.

## Objetivo del Juego

El objetivo del juego es sobrevivir, recolectar materiales, fabricar herramientas, explorar el mundo, construir estructuras y progresar mediante mineria, crafteo, exploracion y comercio.

El jugador inicia en un mundo abierto donde debe conseguir recursos basicos como troncos, tierra, piedra y comida. Con estos recursos puede crear herramientas, descubrir cuevas, encontrar aldeas, comerciar con aldeanos y defenderse de enemigos durante la noche.

## Controles

```text
W / A / S / D        Movimiento del jugador
Mouse               Apuntar y seleccionar bloques
Clic izquierdo      Romper bloques / golpear / interactuar segun contexto
Clic derecho        Colocar bloque / interactuar
E                   Abrir inventario
ESC                 Abrir menu de pausa
Q                   Tirar item seleccionado
Shift + Clic        Movimiento rapido de items en inventario
1 - 9               Seleccionar ranura de la hotbar
F                   Usar / intercambiar mano secundaria
```

## Mecanicas Principales

### Recoleccion

El jugador puede recolectar recursos del mundo, como:

- Troncos de arboles
- Tierra
- Piedra
- Cultivos
- Items soltados por animales
- Recursos encontrados en cofres

Los items tirados en el suelo flotan, se mueven suavemente y son atraidos hacia el jugador al acercarse.

### Inventario

El juego cuenta con un sistema de inventario inspirado en Minecraft:

- Hotbar de 9 espacios
- Inventario principal
- Apilamiento de items
- Movimiento de objetos con mouse
- Segunda mano
- Recoleccion automatica de objetos
- Prioridad de llenado en la hotbar

### Crafteo

El juego incluye dos sistemas de crafteo:

- Crafteo basico 2x2 desde el inventario
- Mesa de crafteo con catalogo estilo Minecraft Xbox 360

Algunas recetas implementadas:

- Tronco a tablones
- Tablones a mesa de crafteo
- Palos
- Pico
- Hacha
- Pala
- Barreta
- Horno
- Herramientas basicas

### Construccion

El jugador puede colocar y romper bloques en el mundo usando una cuadricula.

El sistema incluye:

- Seleccion visual del bloque bajo el cursor
- Rango maximo de construccion
- Colocacion solo en celdas validas
- Bloques solidos con colision
- Bloques decorativos sin colision
- Grietas progresivas al romper bloques
- Particulas al minar o talar
- Diferentes tiempos de minado segun herramienta

### Dia y Noche

El mundo tiene un ciclo de dia y noche inspirado en Minecraft.

Durante el dia:

- Hay mayor visibilidad
- Los enemigos dejan de aparecer
- El mundo se ve iluminado

Durante la noche:

- El ambiente se oscurece
- Aparecen enemigos
- La iluminacion del jugador y de las antorchas se vuelve importante

### Cuevas

El juego cuenta con un sistema de cuevas separado de la superficie.

Incluye:

- Entrada de mina
- Mapa subterraneo
- Paredes solidas de piedra
- Caminos naturales
- Minerales visibles
- Oscuridad fuerte
- Antorchas colocables
- Salida de regreso a la superficie

### Agua

El agua afecta al jugador y animales:

- Movimiento mas lento
- Efecto visual de hundimiento
- Animacion de agua
- Sistema de permanencia bajo el agua
- Barra de oxigeno

### Animales

El juego incluye animales pasivos:

- Cerdos
- Vacas
- Ovejas
- Gallinas

Los animales tienen:

- Movimiento propio
- Sistema de dano
- Animacion al recibir golpes
- Estado de panico
- Muerte con animacion
- Drops de items
- Interaccion con agua

### Enemigos

El juego incluye zombies como enemigos basicos.

Los zombies:

- Aparecen durante la noche o en zonas oscuras
- Persiguen al jugador
- Hacen dano
- Reciben dano
- Tienen animacion
- Pueden soltar objetos al morir

### Aldeas

El mundo puede generar aldeas con:

- Caminos
- Casas
- Granjas
- Herreria
- Cofres
- Aldeanos
- Cultivos
- Interiores con techos

### Aldeanos

Los aldeanos son entidades pacificas con profesiones:

- Granjero
- Herrero
- Bibliotecario

Durante el dia caminan cerca de la aldea.
Durante la noche buscan refugio en casas.

### Trading

El jugador puede comerciar con aldeanos usando esmeraldas.

La interfaz de tradeo incluye:

- Item solicitado
- Flecha de intercambio
- Item recibido
- Cambio entre ofertas
- Validacion de inventario
- Bloqueo temporal tras muchos intercambios

### Cofres

Las aldeas pueden contener cofres, especialmente en herrerias.

Los cofres tienen:

- Inventario propio
- Loot aleatorio
- Guardado persistente
- Interfaz estilo Minecraft
- Contenido que no se regenera infinitamente

### Guardado

El sistema de guardado conserva:

- Mundo generado
- Semilla del mundo
- Posicion del jugador
- Inventario
- Bloques modificados
- Cofres abiertos
- Contenido de cofres
- Aldeanos
- Animales
- Enemigos
- Hora del mundo
- Estado de superficie o cueva

Las partidas locales se guardan en `saves/`, pero esa carpeta se mantiene fuera del repositorio para no subir mundos generados.

## Caracteristicas

- Mundo 2D estilo Minecraft con vista cenital
- Generacion procedural de terreno
- Biomas, arboles, agua, piedra y aldeas
- Inventario estilo Minecraft
- Crafteo 2x2 y mesa de crafteo estilo Xbox 360
- Sistema de bloques con colision
- Herramientas y tiempos de minado
- Animales con IA basica
- Zombies con aparicion nocturna
- Ciclo de dia y noche
- Iluminacion global y por antorchas
- Cuevas explorables
- Aldeas con cofres, cultivos y aldeanos
- Sistema de comercio con esmeraldas
- HUD con vida, hambre, experiencia y oxigeno
- Menu principal inspirado en Minecraft Xbox 360
- Menu de pausa completo
- Sonidos y musica de ambiente
- Guardado persistente del mundo

## Tecnologias

```text
Lenguaje: C++
Libreria grafica: SFML
Fisica / apoyo: Box2D
Paradigma: Programacion Orientada a Objetos
Compilacion: makefile
Sistema operativo objetivo: Windows
```

## Organizacion del Codigo

```text
include/
|-- core/              Clases principales del juego
|-- entidades/         Jugador, animales, zombies, aldeanos
|-- sistemas/          Sistemas de herramientas, crafteo, guardado, etc.
|-- ui/                Inventario, menus, HUD e interfaces
`-- inline/            Implementaciones inline solicitadas por el profesor

src/
`-- main.cpp           Punto de entrada del programa
```

## Equipo

```text
Lider: Raymundo Javier Renteria Flores (@Raymundo1706)
Estudio: tononongos.ing
```

## Creditos

Proyecto desarrollado por:

```text
Raymundo Javier Renteria Flores
Estudio: tononongos.ing
```

Proyecto inspirado en:

- Minecraft
- Minecraft Xbox 360 Edition
- Juegos sandbox de supervivencia
- Estetica pixel art clasica

Assets utilizados:

- Sprites personalizados y adaptados para el proyecto
- Texturas pixel art para jugador, animales, zombies, bloques e items
- Sonidos y musica usados con fines educativos

Agradecimientos:

- Profesor de Proyecto 252
- CETUS
- Comunidad de desarrollo de videojuegos en C++

## Flujo de Trabajo

Para actualizar el proyecto en GitHub:

```bash
git add .
git commit -m "Actualizacion del proyecto"
git push origin main
```

Cada push a la rama `main` ejecuta automaticamente el GitHub Action de publicacion.

