CC	= gcc
EXE	= AFNDSinLambda
CFLAGS = -g -Wall 

OBJ = main.o afnd.o

all:  $(OBJ)
	@echo "Generando el ejecutable...\n"
	@$(CC) $(CFLAGS) -o $(EXE)  $(OBJ)
	@echo "¡Listo!\n"

clean: 
	@echo "Borrando ejecutable y ficheros objeto...\n"
	@rm -f $(EXE) all *.o

main.o: main.c
#$(CC) -c $< -o $@

afnd.o: afnd.c afnd.h alfabeto.h estado.h palabra.h
#$(CC) -c $< -o $@ 

