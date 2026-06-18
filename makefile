.PHONY: juego run maincra2

INCLUDES = -Idocs/include

juego:
	g++ src/main.cpp -o bin/JuegoProyecto.exe -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lbox2d $(INCLUDES)

run: juego
	./bin/JuegoProyecto.exe

maincra2: run
