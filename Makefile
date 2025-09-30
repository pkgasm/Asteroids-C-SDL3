# Makefile para Asteroids con SDL3

# Compilador
CC = gcc

# Flags del compilador
# -g: para debugging
# -Wall: para mostrar todos los warnings
# -O2: para optimización
CFLAGS = -g -Wall -O2

# Flags del enlazador (Linker)
# Necesitamos enlazar con SDL3, SDL3_ttf y la librería matemática (m)
LDFLAGS = -lSDL3 -lSDL3_ttf -lm

# Archivos fuente (.c)
SRCS = main.c game.c entities.c utils.c

# Archivos objeto (.o) que se generarán a partir de los .c
OBJS = $(SRCS:.c=.o)

# Nombre del ejecutable final
TARGET = asteroids

# Regla principal: se ejecuta por defecto con 'make'
all: $(TARGET)

# Regla para enlazar los archivos objeto y crear el ejecutable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Regla para compilar cada archivo .c en su .o correspondiente
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regla para limpiar los archivos generados (ejecutable y archivos objeto)
clean:
	rm -f $(OBJS) $(TARGET) highscore.txt

# Phony targets no son nombres de archivos
.PHONY: all clean