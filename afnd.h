/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   afnd.h
 * Author: root
 *
 * Created on 2 de octubre de 2016, 20:15
 */

#ifndef AFND_H
#define AFND_H

#include <stdio.h>
#include <stdlib.h>
#include "estado.h"
#include "alfabeto.h"
#include <string.h>


#define NUM_TRANSICIONES 100
#define LONG_PALABRA 30
#define NUM_ESTADOS_ACTUALES 100
#define NUM_ESTADOS_FINALES 100

#define MAX_LEN_ESTADO 50
typedef struct _AFND AFND;

AFND * AFNDNuevo(char * nombre, int num_estados, int num_simbolos);


void AFNDElimina(AFND * p_afnd);


void AFNDImprime(FILE * fd, AFND* p_afnd);


AFND * AFNDInsertaSimbolo(AFND * p_afnd, char * simbolo);


AFND * AFNDInsertaEstado(AFND * p_afnd, char * nombre, int tipo);

AFND * AFNDInsertaTransicion(AFND * p_afnd, 
                             char * nombre_estado_i, 
                             char * nombre_simbolo_entrada, 
                             char * nombre_estado_f );


AFND * AFNDInsertaLetra(AFND * p_afnd, char * letra);

void AFNDImprimeConjuntoEstadosActual(FILE * fd, AFND * p_afnd);


void AFNDImprimeCadenaActual(FILE *fd, AFND * p_afnd);


AFND * AFNDInicializaEstado (AFND * p_afnd);


void AFNDTransita(AFND * p_afnd);


void AFNDProcesaEntrada(FILE * fd, AFND * p_afnd);

AFND * AFNDInsertaLTransicion(
       AFND * p_afnd, 
       char * nombre_estado_i, 
       char * nombre_estado_f );



AFND * AFNDCierraLTransicion (AFND * p_afnd);


AFND * AFNDInicializaCadenaActual (AFND * p_afnd );

AFND * AFNDCierreDijkstra(AFND * p_afnd);

int AFNDExisteTransicionL(AFND * p_afnd, int x, int y);

/* Rutinas de la práctica 3 */

AFND * AFND1ODeSimbolo( char * simbolo);

AFND * AFND1ODeLambda();

AFND * AFND1ODeVacio();

AFND * AFNDAAFND1O(AFND * p_afnd);

AFND * AFND1OUne(AFND * p_afnd1O_1, AFND * p_afnd1O_2);

AFND * AFND1OConcatena(AFND * p_afnd_origen1, AFND * p_afnd_origen2);

AFND * AFND1OEstrella(AFND * p_afnd_origen);


/* Funciones auxiliares */

Simbolo* getSimbolo(AFND * p_afnd, char* nombresimbolo);

Estado* getEstado(AFND * p_afnd, char* nombreEstado, int numautomata);

void AFNDADot(AFND * p_afnd);

int AFNDExisteTransicionLDirecta(AFND * p_afnd, int x, int y);

#endif /* AFND_H */

