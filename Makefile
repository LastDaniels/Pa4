# Compilador y opciones
CC = gcc
CFLAGS = -Wall -Wshadow -pthread

# Archivos fuente y ejecutables
SRC = bmp.c filter.c
OBJ = $(SRC:.c=.o)
EXECUTABLES = ex5 desenfocador realzador publicador combinador

all: $(EXECUTABLES)

# Compilación del programa principal
ex5: ex5.o $(OBJ)
	$(CC) $(CFLAGS) ex5.o $(OBJ) -o ex5

# Compilación del Desenfocador (filtro en la primera mitad)
desenfocador: desenfocador.o $(OBJ)
	$(CC) $(CFLAGS) desenfocador.o $(OBJ) -o desenfocador

# Compilación del Realzador (detección de bordes en la segunda mitad)
realzador: realzador.o $(OBJ)
	$(CC) $(CFLAGS) realzador.o $(OBJ) -o realzador

# Compilación del Publicador (maneja memoria compartida y lanza procesos)
publicador: publicador.o $(OBJ)
	$(CC) $(CFLAGS) publicador.o $(OBJ) -o publicador -lrt

# Compilación del Combinador (fusiona imágenes procesadas)
combinador: combinador.o $(OBJ)
	$(CC) $(CFLAGS) combinador.o $(OBJ) -o combinador

# Compilar archivos .c a .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpiar archivos generados
clean:
	rm -f $(EXECUTABLES) *.o

