.PHONY: juego run maincra2

juego:
	g++ src/main.cpp -o bin/JuegoProyecto.exe -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lbox2d -Iinclude

run: juego
	./bin/JuegoProyecto.exe

maincra2:
	@echo Maincra 2-D listo.
