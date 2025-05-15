main: main.c grafico.c grafico.h
	cc -Wall -Wextra -ggdb `pkg-config --cflags freetype2` -o main main.c grafico.c -lglfw -lGL -lm `pkg-config --libs freetype2`
