main: main.c grafico.c
	cc -Wall -Wextra -ggdb `pkg-config --cflags freetype2` -o main main.c grafico.c -lglfw -lGL -lm `pkg-config --libs freetype2`
