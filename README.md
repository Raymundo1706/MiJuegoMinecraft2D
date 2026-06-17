# MiJuegoMinecraft2D

Proyecto de supervivencia/sandbox 2D inspirado en Minecraft Xbox 360 Edition.

## Estructura requerida

```text
proyecto-252/
|-- .github/
|   `-- workflows/
|       `-- publish.yml
|-- video/
|   `-- demo.mp4
|-- gallery/
|   `-- cover.png
|-- screenshots/
|   |-- screenshot1.png
|   |-- screenshot2.png
|   `-- screenshot3.png
|-- bin/
|   `-- JuegoProyecto.exe
|-- assets/
|   |-- textures/
|   |-- sounds/
|   `-- ...
|-- README.md
`-- .gitignore
```

## Compilar

```bash
make juego
```

## Ejecutar

```bash
./bin/JuegoProyecto.exe
```

Las partidas locales se guardan en `saves/`, pero esa carpeta se mantiene fuera del repositorio para no subir mundos generados.
