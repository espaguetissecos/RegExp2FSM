/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor. 
 */

/*
 * File:   afnd.c
 * Author: Sergio Castellano y Francisco Andreu
 *
 * Created on 29 de septiembre de 2016, 18:20
 */


#include <inttypes.h>

#include "afnd.h"

short num_id = 0;

typedef struct _Transicion {
    Estado* origen;
    Estado* destino;
    Simbolo* simbolo;

} Transicion;

typedef struct _TransicionLambda {
    Estado* origen;
    Estado* destino;
} TransicionLambda;

typedef struct _AFND {
    char nombre[100];

    /* ESTADOS */
    Estado* estados;
    int num_estados;
    int num_estados_insertados;
    Estado* estado_inicial; /*Apuntara a su respectivo estado*/
    Estado** estado_final; /*Apuntara a sus respectivos estados*/
    int num_estado_final;
    Estado** estado_actual; /*Apuntara a sus respectivos estados*/
    int num_estado_actual;

    /* ALFABETO Y PALABRA */
    Simbolo* alfabeto;
    int longitud_alfabeto;
    int num_simbolos_insertados;
    Simbolo* palabra;
    int longitud_palabra;

    /* TRANSICIONES */
    Transicion* transiciones;
    int num_transiciones_insertadas;

    /* TRANSICIONES LAMBDA*/
    TransicionLambda* transiciones_l;
    int num_transiciones_l_insertadas;

    /* ID DEL AUTOMATA*/
    int id;

} AFND;

AFND * AFNDNuevo(char * nombre, int num_estados, int num_simbolos) {
    if (!nombre || num_estados <= 0 || num_simbolos <= 0)
        return NULL;

    /* Reservamos memoria para la estructura y le copiamos el nombre */
    AFND* afnd = (AFND*) malloc(sizeof (AFND));
    if (strcpy(afnd->nombre, nombre) == NULL)
        return NULL;

    /* ESTADOS */
    /* Al principio no hay estados insertados, actuales, ni finales. Pero reservamos memoria para ellos */
    afnd->num_estados = num_estados;
    afnd->num_estados_insertados = 0;
    afnd->num_estado_final = 0;
    afnd->num_estado_actual = 0;
    afnd->estados = (Estado*) malloc(sizeof (Estado) * num_estados);
    afnd->estado_actual = (Estado**) malloc(sizeof (Estado*) * NUM_ESTADOS_ACTUALES);
    afnd->estado_final = (Estado**) malloc(sizeof (Estado*) * NUM_ESTADOS_FINALES);

    /* ALFABETO (alfabeto+palabra a procesar) */
    /* Al principio no hay simbolos en el alfabeto ni en la palabra a procesar. Pero reservamos memoria para ellos */
    afnd->longitud_alfabeto = num_simbolos;
    afnd->num_simbolos_insertados = 0;
    afnd->alfabeto = (Simbolo*) malloc(sizeof (Simbolo) * num_simbolos);
    afnd->longitud_palabra = 0;
    afnd->palabra = (Simbolo*) malloc(sizeof (Simbolo) * LONG_PALABRA);

    /* TRANSICIONES */
    /* Al principio no hay transiciones. Pero reservamos memoria para ellas */
    afnd->num_transiciones_insertadas = 0;
    afnd->transiciones = (Transicion*) malloc(sizeof (Transicion) * NUM_TRANSICIONES);
    afnd->num_transiciones_l_insertadas = 0;
    afnd->transiciones_l = (TransicionLambda*) malloc(sizeof (TransicionLambda) * NUM_TRANSICIONES);

    /* ASIGNAMOS EL ID DEL AUTOMATA */
    afnd->id = num_id++;

    return afnd;
}

void AFNDElimina(AFND * p_afnd) {
    if (!p_afnd)
        return;

    int i = 0;


    /* ALFABETO y palabra */
    for (i = 0; i < p_afnd->num_simbolos_insertados; i++) {
        free(p_afnd->alfabeto[i]);
        p_afnd->alfabeto[i] = NULL;
    }

    for (i = 0; i < p_afnd->longitud_palabra; i++)
        free(p_afnd->palabra[i]);

    /* ESTADOS */
    for (i = 0; i < p_afnd->num_estado_final; i++)
        free(p_afnd->estado_final[i]);

    free(p_afnd->estado_inicial);

    for (i = 0; i < p_afnd->num_estado_actual; i++)
        free(p_afnd->estado_actual[i]);

    for (i = 0; i < p_afnd->num_estados_insertados; i++)
        free(p_afnd->estados[i]);

    /* El resto de punteros */
    free(p_afnd->transiciones);
    free(p_afnd->transiciones_l);
    free(p_afnd->palabra);
    free(p_afnd->estado_final);
    free(p_afnd->estado_actual);
    free(p_afnd->estados);
    free(p_afnd->alfabeto);
    free(p_afnd);
    p_afnd = NULL;

}

void AFNDImprimeMatrizL(FILE * fd, AFND* p_afnd) {
    int i, j;
    fprintf(fd, "\tRL++*={\n\t\t");

    /* Imprimimos los indices de las columnas */
    for (i = 0; i < p_afnd->num_estados_insertados; i++)
        fprintf(fd, "\t[%d]", i);
    fprintf(fd, "\n");

    /* Y para cada estado, imprimimos su indice, y, un 1 si existe transicion, 0 si no */
    for (i = 0; i < p_afnd->num_estados_insertados; i++) {
        fprintf(fd, "\t\t[%d]", i);

        for (j = 0; j < p_afnd->num_estados_insertados; j++) {
            if (AFNDExisteTransicionL(p_afnd, i, j) == 1)
                fprintf(fd, "\t1");
            else
                fprintf(fd, "\t0");
        }
        fprintf(fd, "\n");
    }
    fprintf(fd, "\t}\n\n");
}

void AFNDImprime(FILE * fd, AFND* p_afnd) {
    if (!fd || !p_afnd)
        return;
    int i, j, k, flag1, flag2;

    fprintf(fd, "%s={\n", p_afnd->nombre);

    /* Imprimimos el alfabeto */
    fprintf(fd, "\tnum_simbolos = %d \n\n\tA={ ", p_afnd->num_simbolos_insertados);
    for (i = 0; i < p_afnd->num_simbolos_insertados; i++) {
        fprintf(fd, "%s ", p_afnd->alfabeto[i]);
    }
    fprintf(fd, "}\n\n");

    /* Imprimimos los estados */
    fprintf(fd, "\tnum_estados = %d \n\n\tQ={", p_afnd->num_estados_insertados);
    for (i = 0; i < p_afnd->num_estados_insertados; i++) {
        flag1 = 0;
        flag2 = 0;

        /* Detectamos si es estado final */
        for (j = 0; j < p_afnd->num_estado_final; j++) {
            if (p_afnd->estados[i] == *p_afnd->estado_final[j]) {
                flag1++;
                break;
            }
        }

        /* Detectamos si es estado inicial */
        if (p_afnd->estados[i] == *p_afnd->estado_inicial)
            flag2++;

        /* Imprimimos los estados segun el tipo */
        if (flag1 == 1 && flag2 == 1) /*Estado final e inicial*/
            fprintf(fd, "->%s* ", p_afnd->estados[i]);
        else if (flag1 == 0 && flag2 == 1) /*Estado inicial*/
            fprintf(fd, "->%s ", p_afnd->estados[i]);
        else if (flag1 == 1 && flag2 == 0) /*Estado final*/
            fprintf(fd, "%s* ", p_afnd->estados[i]);
        else
            fprintf(fd, "%s ", p_afnd->estados[i]);
    }
    fprintf(fd, "}\n\n");

    /* Imprimimos la matriz */
    AFNDImprimeMatrizL(fd, p_afnd);

    /* Imprimimos las transiciones */
    /* Para cada estado y simbolo, escribimos el destino de esa transicion (si la hay) */
    fprintf(fd, "\tFuncion de transicion = {\n");
    for (i = 0; i < p_afnd->num_estados_insertados; i++) {
        for (j = 0; j < p_afnd->num_simbolos_insertados; j++) {
            fprintf(fd, "\t\tf(%s,%s)={ ", p_afnd->estados[i], p_afnd->alfabeto[j]);
            for (k = 0; k < p_afnd->num_transiciones_insertadas; k++) {
                if ((p_afnd->transiciones[k].origen == &p_afnd->estados[i]) && (p_afnd->transiciones[k].simbolo == &p_afnd->alfabeto[j])) {
                    fprintf(fd, "%s ", *p_afnd->transiciones[k].destino);
                }
            }
            fprintf(fd, "}\n");
        }
    }
    fprintf(fd, "\t}\n}");
}

AFND * AFNDInsertaSimbolo(AFND * p_afnd, char * simbolo) {
    if (!p_afnd || !simbolo)
        return NULL;

    /* Comprobamos el numero de simbolos insertados*/
    if (p_afnd->num_estados_insertados == p_afnd->num_estados) {
        printf("El numero de simbolos insertados no puede ser mayor que el máximo establecido");
        return NULL;
    }

    /* Reservamos memoria y copiamos el simbolo en el alfabeto. Tambien aumentamos el numero de simbolos insertados */
    p_afnd->alfabeto[p_afnd->num_simbolos_insertados] = (Simbolo) malloc(sizeof (char) * (strlen(simbolo) + 1));
    strcpy(p_afnd->alfabeto[p_afnd->num_simbolos_insertados], simbolo);
    p_afnd->num_simbolos_insertados++;

    return p_afnd;
}

AFND * AFNDInsertaEstado(AFND * p_afnd, char * nombre, int tipo) {
    if (!p_afnd || !nombre)
        return NULL;

    /* Reservamos memoria para el "nombre del estado+id" y lo creamos */
    char id[MAX_LEN_ESTADO] = "";

    sprintf(id, "%d", p_afnd->id);
    p_afnd->estados[p_afnd->num_estados_insertados] = (Estado) malloc(sizeof (char)*(strlen(nombre) + strlen(id) + 1 + 1));
    sprintf(p_afnd->estados[p_afnd->num_estados_insertados], "%s_%d", nombre, p_afnd->id);

    /* Si es de algun tipo en específico, hacemos que un puntero de ese array apunte al estado */
    /* Si es de tipo final, hacemos que un puntero del array estado_final apunte a dicho estado. Y asi con todos los tipos */
    if (tipo == FINAL) {
        p_afnd->estado_final[p_afnd->num_estado_final] = (Estado*) malloc(sizeof (Estado));
        *(p_afnd->estado_final[p_afnd->num_estado_final]) = (p_afnd->estados[p_afnd->num_estados_insertados]);
        p_afnd->num_estado_final++;
    } else if (tipo == INICIAL) {
        p_afnd->estado_inicial = (Estado*) malloc(sizeof (Estado));
        *(p_afnd->estado_inicial) = (p_afnd->estados[p_afnd->num_estados_insertados]);
    }
    if (tipo == INICIALFINAL) {
        p_afnd->estado_inicial = (Estado*) malloc(sizeof (Estado));
        p_afnd->estado_final[p_afnd->num_estado_final] = (Estado*) malloc(sizeof (Estado));
        *(p_afnd->estado_inicial) = p_afnd->estados[p_afnd->num_estados_insertados];
        *(p_afnd->estado_final[p_afnd->num_estado_final]) = (p_afnd->estados[p_afnd->num_estados_insertados]);
        p_afnd->num_estado_final++;
    }

    /* Finalmente, aumentamos el numero de estados insertados */
    p_afnd->num_estados_insertados++;
    return p_afnd;
}

AFND * AFNDInsertaTransicion(AFND * p_afnd, char * nombre_estado_i, char * nombre_simbolo_entrada, char * nombre_estado_f) {
    if (!p_afnd || !nombre_estado_i || !nombre_simbolo_entrada || !nombre_estado_f)
        return NULL;

    int i = 0;

    char nombre_estado_i_id[MAX_LEN_ESTADO] = "";
    char nombre_estado_f_id[MAX_LEN_ESTADO] = "";
    sprintf(nombre_estado_i_id, "%s_%d", nombre_estado_i, p_afnd->id);
    sprintf(nombre_estado_f_id, "%s_%d", nombre_estado_f, p_afnd->id);

    /* Comprobramos que el estado de inicio y final de esa transicion esten en nuestro AFND. Y si es asi, insertamos la transicion */
    for (i = 0; i < p_afnd->num_estados_insertados; i++) {
        if (strcmp(p_afnd->estados[i], nombre_estado_i_id) == 0)
            p_afnd->transiciones[p_afnd->num_transiciones_insertadas].origen = &p_afnd->estados[i];
        if (strcmp(p_afnd->estados[i], nombre_estado_f_id) == 0)
            p_afnd->transiciones[p_afnd->num_transiciones_insertadas].destino = &p_afnd->estados[i];
    }

    /* Tambien comprobamos que la letra este en el alfabeto del AFND */
    for (i = 0; i < p_afnd->num_simbolos_insertados; i++) {
        if (strcmp(p_afnd->alfabeto[i], nombre_simbolo_entrada) == 0) {
            p_afnd->transiciones[p_afnd->num_transiciones_insertadas].simbolo = &p_afnd->alfabeto[i];
            break;
        }
    }

    /* Si alguna de las 3 condiciones anteriores no se ha cumplido, devolvemos error */
    if (!p_afnd->transiciones[p_afnd->num_transiciones_insertadas].origen || !p_afnd->transiciones[p_afnd->num_transiciones_insertadas].destino || !p_afnd->transiciones[p_afnd->num_transiciones_insertadas].simbolo) {
        printf("Transicion no insertada correctamente\n");
        return NULL;
    }

    /* E incrementamos el numero de transiciones insertadas en nuestro AFND */
    p_afnd->num_transiciones_insertadas++;

    return p_afnd;
}

AFND * AFNDInsertaLetra(AFND * p_afnd, char * letra) {
    if (!p_afnd || !letra)
        return NULL;

    int i;

    /* Si la letra esta incluida en nuestro alfabeto, reservamos memoria para copiar la letra al final de la palabra */
    for (i = 0; i < p_afnd->longitud_alfabeto; i++) {
        if (strcmp(p_afnd->alfabeto[i], letra) == 0) {
            p_afnd->palabra[p_afnd->longitud_palabra] = (Simbolo) malloc(sizeof (char) * (strlen(letra) + 1));
            strcpy(p_afnd->palabra[p_afnd->longitud_palabra], letra);
            p_afnd->longitud_palabra++;
            break;
        }
    }

    return p_afnd;
}

void AFNDImprimeConjuntoEstadosActual(FILE * fd, AFND * p_afnd) {
    if (!fd || !p_afnd)
        return;
    int i = 0, j, flag1, flag2;
    fprintf(fd, "\nACTUALMENTE EN {");

    /* Tenemos que comprobar de que tipo son cada uno de los estados en los que estamos */
    for (i = 0; i < p_afnd->num_estado_actual; i++) {
        flag1 = 0;
        flag2 = 0;

        /* Detectamos si estamos en un estado final */
        for (j = 0; j < p_afnd->num_estado_final; j++) {
            if (*p_afnd->estado_actual[i] == *p_afnd->estado_final[j]) {
                flag1++;
                break;
            }
        }

        /* Detectamos si estamos en el estado inicial */
        if (*p_afnd->estado_actual[i] == *p_afnd->estado_inicial)
            flag2++;

        /* E imprimimos los estados segun el tipo */
        if (flag1 == 1 && flag2 == 1) /*Estado final e inicial*/
            fprintf(fd, "->%s* ", *p_afnd->estado_actual[i]);
        else if (flag1 == 0 && flag2 == 1) /*Estado inicial*/
            fprintf(fd, "->%s ", *p_afnd->estado_actual[i]);
        else if (flag1 == 1 && flag2 == 0) /*Estado final*/
            fprintf(fd, "%s* ", *p_afnd->estado_actual[i]);
        else
            fprintf(fd, "%s ", *p_afnd->estado_actual[i]);
    }
    fprintf(fd, "}\n");

    return;
}

void AFNDImprimeCadenaActual(FILE *fd, AFND * p_afnd) {
    if (!fd || !p_afnd)
        return;
    int i;

    /* Imprimimos cada una de las letras de la palabra */
    fprintf(fd, "[(%d)", p_afnd->longitud_palabra);
    for (i = 0; i < p_afnd->longitud_palabra; i++)
        fprintf(fd, " %c", *p_afnd->palabra[i]);

    fprintf(fd, "]\n");

    return;
}

AFND * AFNDInicializaEstado(AFND * p_afnd) {
    if (!p_afnd)
        return NULL;
    int i;

    /* Asignamos el estado inicial como estado actual al principio del automata */
    p_afnd->estado_actual[p_afnd->num_estado_actual] = (Estado*) malloc(sizeof (Estado));
    *p_afnd->estado_actual[p_afnd->num_estado_actual] = *(p_afnd->estado_inicial);
    p_afnd->num_estado_actual++;


    /* Comprobamos los estados a los que puede llegar mediante transiciones landa, y los insertamos al conjuntos de estados actuales */
    for (i = 0; i < p_afnd->num_transiciones_l_insertadas; i++) {
        if (*p_afnd->transiciones_l[i].origen == *p_afnd->estado_actual[0] && *p_afnd->transiciones_l[i].destino != *p_afnd->estado_actual[0]) {
            p_afnd->estado_actual[p_afnd->num_estado_actual] = (Estado*) malloc(sizeof (Estado));
            *p_afnd->estado_actual[p_afnd->num_estado_actual] = *p_afnd->transiciones_l[i].destino;
            p_afnd->num_estado_actual++;
        }
    }

    return p_afnd;
}

int estaEnEstadosActuales(AFND* p_afnd, Estado estado) {
    if (!p_afnd || !estado)
        return -1;
    int i;
    Estado e1;
    for (i = 0; i < p_afnd->num_estado_actual; i++) {
        e1 = *p_afnd->estado_actual[i];
        if (strcmp(e1, estado) == 0)
            return 1;
    }
    return 0;
}

void AFNDTransita(AFND * p_afnd) {
    if (!p_afnd)
        return;
    int i, j, k = 0;
    Estado **estados_aux = (Estado**) malloc(sizeof (Estado*)*50);

    /* Comprobamos en todos los estados actuales si hay alguna transicion posible desde dicho estado al procesar el siguiente simbolo */
    for (i = 0; i < p_afnd->num_transiciones_insertadas; i++) {
        for (j = 0; j < p_afnd->num_estado_actual; j++) {
            if ((*p_afnd->transiciones[i].origen == *p_afnd->estado_actual[j]) && (strcmp(p_afnd->palabra[0], *p_afnd->transiciones[i].simbolo) == 0)) {
                /* Copiamos el estado de destino de la transicion en estados_aux */
                estados_aux[k] = (Estado*) malloc(sizeof (Estado));
                *estados_aux[k] = *(p_afnd->transiciones[i].destino);
                k++;
            }
        }
    }

    /* Borramos los estados actuales (de antes de procesar el simbolo) */
    for (i = 0; i < p_afnd->num_estado_actual; i++) {
        free(p_afnd->estado_actual[i]);
        p_afnd->estado_actual[i] = NULL;
    }
    p_afnd->num_estado_actual = 0;

    /* Y copiamos en los estados actuales los estados_aux */
    for (i = 0; i < k; i++) {
        p_afnd->estado_actual[i] = (Estado*) malloc(sizeof (Estado));
        *p_afnd->estado_actual[i] = *estados_aux[i];
        p_afnd->num_estado_actual++;
    }

    /* Finalmente, liberamos los estados_aux */
    for (i = 0; i < k; i++)
        free(estados_aux[i]);
    free(estados_aux);

    /* Tambien realizamos las transiciones landa posibles desde los nuevos estados. Comprobamos el estado de origen de todas las transiciones landa insertadas. Y que el destino no sea uno de los estados actuales */
    for (i = 0; i < p_afnd->num_transiciones_l_insertadas; i++)
        for (j = 0; j < p_afnd->num_estado_actual; j++) {
            if ((*p_afnd->transiciones_l[i].origen == *p_afnd->estado_actual[j]) && (*p_afnd->transiciones_l[i].destino != *p_afnd->estado_actual[j])) {
                if (estaEnEstadosActuales(p_afnd, *p_afnd->transiciones_l[i].destino) != 1) {
                    p_afnd->estado_actual[p_afnd->num_estado_actual] = (Estado*) malloc(sizeof (Estado));
                    *p_afnd->estado_actual[p_afnd->num_estado_actual] = *p_afnd->transiciones_l[i].destino;
                    p_afnd->num_estado_actual++;
                }
            }
        }

    return;
}

void AFNDProcesaEntrada(FILE * fd, AFND * p_afnd) {
    if (!fd || !p_afnd)
        return;
    int i, long_palabra_original;
    Simbolo* palabra_aux = p_afnd->palabra; /*Guardamos el puntero para no perder la referencia*/
    long_palabra_original = p_afnd->longitud_palabra;

    /* Procesamos los simbolos uno por uno (transitando), e imprimimos en cada paso los estados actuales y la cadena restante por procesar
     * Recorremos el bucle siempre y cuando el num de estados actuales sea mayor que */
    for (i = 0; (i < long_palabra_original) && (p_afnd->num_estado_actual != 0); i++) {
        AFNDImprimeConjuntoEstadosActual(fd, p_afnd);
        AFNDImprimeCadenaActual(fd, p_afnd);
        AFNDTransita(p_afnd);
        free(p_afnd->palabra[0]);
        p_afnd->palabra++;
        p_afnd->longitud_palabra--;

    }

    /* Imprimimos los estados actuales y la cadena */
    AFNDImprimeConjuntoEstadosActual(fd, p_afnd);
    AFNDImprimeCadenaActual(fd, p_afnd);

    /* Liberamos la palabra restante en caso de que el conjunto de estados actuales sea vacio */
    while (p_afnd->longitud_palabra != 0) {
        free(p_afnd->palabra[0]);
        p_afnd->palabra++;
        p_afnd->longitud_palabra--;
    }

    /* Al acabar de procesar dicha palabra en el automata, liberamos los estados actuales y la palabra procesada */
    for (i = 0; i < p_afnd->num_estado_actual; i++) {
        free(p_afnd->estado_actual[i]);
        p_afnd->estado_actual[i] = NULL;
    }
    p_afnd->num_estado_actual = 0;
    p_afnd->palabra = palabra_aux; /*Realocamos "palabra" a su pos. de memoria original*/
    return;
}

AFND * AFNDInsertaLTransicionActualizada(AFND * p_afnd, char * nombre_estado_i, char * nombre_estado_f) {
    if (!p_afnd || !nombre_estado_i || !nombre_estado_f)
        return NULL;
    int i;

    /* Comprobamos que dicha transcion no este ya insertada */
    for (i = 0; i < p_afnd->num_transiciones_l_insertadas; i++)
        if ((*p_afnd->transiciones_l[i].origen == nombre_estado_i) && (*p_afnd->transiciones_l[i].destino == nombre_estado_f))
            return p_afnd;

    /* Insertamos la transicion */
    for (i = 0; i < p_afnd->num_estados_insertados; i++) {
        if (strcmp(p_afnd->estados[i], nombre_estado_i) == 0)
            p_afnd->transiciones_l[p_afnd->num_transiciones_l_insertadas].origen = &p_afnd->estados[i];
        if (strcmp(p_afnd->estados[i], nombre_estado_f) == 0)
            p_afnd->transiciones_l[p_afnd->num_transiciones_l_insertadas].destino = &p_afnd->estados[i];
    }

    /* Si alguna de las 3 condiciones anteriores no se ha cumplido, devolvemos error */
    if (!p_afnd->transiciones_l[p_afnd->num_transiciones_l_insertadas].origen || !p_afnd->transiciones_l[p_afnd->num_transiciones_l_insertadas].destino) {
        printf("Transicion lambda no insertada correctamente\n");
        return NULL;
    }

    /* E incrementamos el numero de transiciones insertadas en nuestro AFND */
    p_afnd->num_transiciones_l_insertadas++;

    return p_afnd;

}

AFND * AFNDInsertaLTransicion(AFND * p_afnd, char * nombre_estado_i, char * nombre_estado_f) {
    if (!p_afnd || !nombre_estado_i || !nombre_estado_f)
        return NULL;
    int i;


    char nombre_estado_i_id[MAX_LEN_ESTADO] = "";
    char nombre_estado_f_id[MAX_LEN_ESTADO] = "";

    sprintf(nombre_estado_i_id, "%s_%d", nombre_estado_i, p_afnd->id);
    sprintf(nombre_estado_f_id, "%s_%d", nombre_estado_f, p_afnd->id);


    /* Comprobamos que dicha transcion no este ya insertada */
    for (i = 0; i < p_afnd->num_transiciones_l_insertadas; i++)
        if ((*p_afnd->transiciones_l[i].origen == nombre_estado_i_id) && (*p_afnd->transiciones_l[i].destino == nombre_estado_f_id))
            return p_afnd;

    /* Insertamos la transicion */
    for (i = 0; i < p_afnd->num_estados_insertados; i++) {
        if (strcmp(p_afnd->estados[i], nombre_estado_i_id) == 0)
            p_afnd->transiciones_l[p_afnd->num_transiciones_l_insertadas].origen = &p_afnd->estados[i];
        if (strcmp(p_afnd->estados[i], nombre_estado_f_id) == 0)
            p_afnd->transiciones_l[p_afnd->num_transiciones_l_insertadas].destino = &p_afnd->estados[i];
    }

    /* Si alguna de las 3 condiciones anteriores no se ha cumplido, devolvemos error */
    if (!p_afnd->transiciones_l[p_afnd->num_transiciones_l_insertadas].origen || !p_afnd->transiciones_l[p_afnd->num_transiciones_l_insertadas].destino) {
        printf("Transicion lambda no insertada correctamente\n");
        return NULL;
    }

    /* E incrementamos el numero de transiciones insertadas en nuestro AFND */
    p_afnd->num_transiciones_l_insertadas++;

    return p_afnd;

}

AFND* AFNDCierreTransitivo(AFND * p_afnd) {
    if (!p_afnd)
        return NULL;

    /* Utilizamos el algoritmo de Dijkstra para comprobar todas las transiciones posibles entre los nodos */
    if (!AFNDCierreDijkstra(p_afnd))
        return NULL;

    return p_afnd;
}

AFND* AFNDCierreReflexivo(AFND * p_afnd) {
    if (!p_afnd)
        return NULL;

    int i;

    /* Cada estado tendra una transicion landa hacia si mismo */
    for (i = 0; i < p_afnd->num_estados_insertados; i++)
        AFNDInsertaLTransicionActualizada(p_afnd, p_afnd->estados[i], p_afnd->estados[i]);

    return p_afnd;
}

AFND* AFNDCierraLTransicion(AFND * p_afnd) {
    if (!p_afnd)
        return NULL;

    /* Realizamos el cierre reflexivo de los estados */
    if (!AFNDCierreReflexivo(p_afnd))
        return NULL;

    /* Realizamos el cierre transitivo de los estados */
    if (!AFNDCierreTransitivo(p_afnd))
        return NULL;

    return p_afnd;
}

AFND* AFNDInicializaCadenaActual(AFND * p_afnd) {

    /* Nosotros reservamos en AFNDInsertaLetra, y liberamos memoria en AFNDProcesaEntrada */

    return p_afnd;
}

AFND* AFNDCierreDijkstra(AFND* p_afnd) {
    if (!p_afnd)
        return NULL;
    int i, j, k;
    int matriz_dijkstra[p_afnd->num_estados_insertados][p_afnd->num_estados_insertados][(p_afnd->num_estados_insertados) + 1]; /*N+1 matrices de N*N*/

    /* Primero inicializamos todas las matrices a 0 */
    for (k = 0; k < p_afnd->num_estados_insertados + 1; k++) {
        for (i = 0; i < p_afnd->num_estados_insertados; i++) {
            for (j = 0; j < p_afnd->num_estados_insertados; j++) {
                matriz_dijkstra[i][j][k] = 0;
            }
        }
    }

    /* Ahora ponemos un 1 si existe transicion landa, en la primera de las matrices */
    for (i = 0; i < p_afnd->num_estados_insertados; i++) {
        for (j = 0; j < p_afnd->num_estados_insertados; j++) {
            if (AFNDExisteTransicionL(p_afnd, i, j) == 1)
                matriz_dijkstra[i][j][0] = 1;
            else
                matriz_dijkstra[i][j][0] = 0;
        }
    }

    /* Ahora insertamos en las matrices de dijkstra y en el automata las transiciones transitivas */
    for (k = 0; k < p_afnd->num_estados_insertados + 1; k++) {
        for (i = 0; i < p_afnd->num_estados_insertados; i++) {
            for (j = 0; j < p_afnd->num_estados_insertados; j++) {

                /* Tanto si existe transicion directa(i->j), o indirecta (i->k->j), insertamos la nueva transicion */
                if ((matriz_dijkstra[i][j][k] == 1) || ((matriz_dijkstra[i][k][k] == 1) && (matriz_dijkstra[k][j][k] == 1))) {
                    matriz_dijkstra[i][j][k + 1] = 1;
                    AFNDInsertaLTransicionActualizada(p_afnd, p_afnd->estados[i], p_afnd->estados[j]);
                }
            }
        }
    }

    return p_afnd;
}

int AFNDExisteTransicionL(AFND * p_afnd, int x, int y) {
    int i;

    /* Comprobamos si existe transicion landa entre el estado de origen x al destino y. Si existe, devolvemos 1 */
    for (i = 0; i < p_afnd->num_transiciones_l_insertadas; i++)
        if ((*p_afnd->transiciones_l[i].origen == p_afnd->estados[x]) && (*p_afnd->transiciones_l[i].destino == p_afnd->estados[y]))
            return 1;

    return 0;
}

int estaEnEstadosFinales(AFND* p_afnd, Estado estado) {
    if (!p_afnd || !estado)
        return -1;
    int i;
    Estado e1;
    for (i = 0; i < p_afnd->num_estado_final; i++) {
        e1 = *p_afnd->estado_final[i];
        if (strcmp(e1, estado) == 0)
            return 1;
    }
    return 0;
}

void AFNDADot(AFND * p_afnd) {

    if (!p_afnd)
        return;
    int i;
    /* Creamos el archivo */
    FILE* pf = fopen("AFNDDot.txt", "w+");
    if (!pf)
        return;
    fprintf(pf, "digraph %s  { rankdir=LR;\n", p_afnd->nombre);
    fprintf(pf, "\t_invisible [style=\"invis\"];\n");

    /* Imprimimos todos los estados excepto los finales */
    for (i = 0; i < p_afnd->num_estados_insertados; i++)
        if (estaEnEstadosFinales(p_afnd, p_afnd->estados[i]) != 1)
            fprintf(pf, "\t%s;\n", p_afnd->estados[i]);

    /* Imprimimos ahora los estados finales con el formato pedido*/
    for (i = 0; i < p_afnd->num_estado_final; i++)
        fprintf(pf, "\t%s [penwidth=\"2\"];\n", *p_afnd->estado_final[i]);

    fprintf(pf, "\t_invisible -> %s ;\n", *p_afnd->estado_inicial);

    for (i = 0; i < p_afnd->num_transiciones_insertadas; i++)
        fprintf(pf, "\t%s -> %s [label=\"%s\"];\n", *p_afnd->transiciones[i].origen, *p_afnd->transiciones[i].destino, *p_afnd->transiciones[i].simbolo);

    for (i = 0; i < p_afnd->num_transiciones_l_insertadas; i++)
        fprintf(pf, "\t%s -> %s [label=\"&lambda;\"];\n", *p_afnd->transiciones_l[i].origen, *p_afnd->transiciones_l[i].destino);

    fprintf(pf, "}");

    fclose(pf);
    pf = NULL;

    return;
}

AFND * AFND1ODeSimbolo(char * simbolo) {

    if (!simbolo)
        return NULL;

    /* Creamos el AFND símbolo */
    AFND* p_afnd = AFNDNuevo("AFNDSimbolo", 100, 100);

    if (!p_afnd)
        return NULL;

    /* Insertamos el simbolo en el alfabeto */
    if (!AFNDInsertaSimbolo(p_afnd, simbolo))
        return NULL;
    /* Insertamos los dos estados y la transicion con el simbolo del argumento
     * y devolvemos este AFND
     */
    AFNDInsertaEstado(p_afnd, "q0", INICIAL);
    AFNDInsertaEstado(p_afnd, "q1", FINAL);
    AFNDInsertaTransicion(p_afnd, "q0", simbolo, "q1");

    AFNDInicializaEstado(p_afnd);

    return p_afnd;
}

AFND * AFND1ODeLambda() {
    /* Creamos el AFND Lambda */
    AFND* p_afnd = AFNDNuevo("AFNDLambda", 2, 1);

    if (!p_afnd)
        return NULL;

    /* Insertamos los dos estados y la transicion lambda y devolvemos este AFND */
    AFNDInsertaEstado(p_afnd, "q0", INICIAL);
    AFNDInsertaEstado(p_afnd, "q1", FINAL);
    AFNDInsertaLTransicion(p_afnd, "q0", "q1");

    AFNDInicializaEstado(p_afnd);

    return p_afnd;
}

AFND * AFND1ODeVacio() {
    /* Creamos el AFND vacio */
    AFND* p_afnd = AFNDNuevo("AFNDVacio", 2, 1);

    if (!p_afnd)
        return NULL;

    /* Insertamos los dos estados y devolvemos este AFND */
    AFNDInsertaEstado(p_afnd, "q0", INICIAL);
    AFNDInsertaEstado(p_afnd, "q1", FINAL);
    return p_afnd;
}

AFND * AFNDAAFND1O(AFND * p_afnd) {
    if (!p_afnd)
        return NULL;


    int i;
    char inicial[MAX_LEN_ESTADO] = "INICIAL10";
    char final[MAX_LEN_ESTADO] = "FINAL10";
    Estado* estado;

    char nombre[256] = "";
    char id[256] = "";

    sprintf(nombre, "%s_10", p_afnd->nombre);

    AFND* p_afndret = AFNDNuevo(nombre, p_afnd->num_estados + 2, p_afnd->longitud_alfabeto);
    sprintf(inicial, "INICIAL10_%d", p_afndret->id);
    sprintf(final, "FINAL10_%d", p_afndret->id);

    /*Copiamos alfabeto en el AFND de retorno*/
    for (i = 0; i < p_afnd->num_simbolos_insertados; i++) {
        p_afndret->alfabeto[p_afndret->num_simbolos_insertados] = (Simbolo) malloc(sizeof (char) * (strlen(p_afnd->alfabeto[i]) + 1));
        strcpy(p_afndret->alfabeto[p_afndret->num_simbolos_insertados], p_afnd->alfabeto[i]);
        p_afndret->num_simbolos_insertados++;
    }

    /*Copiamos estados en el AFND de retorno*/
    sprintf(id, "%d", p_afndret->id);
    for (i = 0; i < p_afnd->num_estados_insertados; i++) {
        p_afndret->estados[p_afndret->num_estados_insertados] = (Estado) malloc(sizeof (char)*(strlen(p_afnd->estados[i]) + strlen(id) + 1));
        sprintf(p_afndret->estados[p_afndret->num_estados_insertados], "%s_%d", p_afnd->estados[i], p_afndret->id);
        p_afndret->num_estados_insertados++;
    }
    /*Copiamos transiciones en el AFND de retorno*/
    for (i = 0; i < p_afnd->num_transiciones_insertadas; i++) {
        p_afndret->transiciones[p_afndret->num_transiciones_insertadas].origen = getEstado(p_afndret, *p_afnd->transiciones[i].origen, 0);
        p_afndret->transiciones[p_afndret->num_transiciones_insertadas].simbolo = getSimbolo(p_afndret, *p_afnd->transiciones[i].simbolo);
        p_afndret->transiciones[p_afndret->num_transiciones_insertadas].destino = getEstado(p_afndret, *p_afnd->transiciones[i].destino, 0);
        p_afndret->num_transiciones_insertadas++;

    }
    /*Copiamos transiciones lambda en el AFND de retorno*/
    for (i = 0; i < p_afnd->num_transiciones_l_insertadas; i++) {
        p_afndret->transiciones_l[p_afndret->num_transiciones_l_insertadas].origen = getEstado(p_afndret, *p_afnd->transiciones_l[i].origen, 1);
        p_afndret->transiciones_l[p_afndret->num_transiciones_l_insertadas].destino = getEstado(p_afndret, *p_afnd->transiciones_l[i].destino, 1);
        p_afndret->num_transiciones_l_insertadas++;
    }
    AFNDInsertaEstado(p_afndret, "INICIAL10", INICIAL);
    AFNDInsertaEstado(p_afndret, "FINAL10", FINAL);

    for (i = 0; i < p_afnd->num_estado_final; i++)
        if ((estado = getEstado(p_afndret, *p_afnd->estado_final[i], 0)) != NULL)
            AFNDInsertaLTransicionActualizada(p_afndret, *estado, final);

    estado = getEstado(p_afndret, *p_afnd->estado_inicial, 0);
    AFNDInsertaLTransicionActualizada(p_afndret, inicial, *estado);

    return p_afndret;

}

AFND * AFND1OUne(AFND * p_afnd1O_1, AFND * p_afnd1O_2) {

    if (!p_afnd1O_1 || !p_afnd1O_2)
        return NULL;

    int i;

    char nombre[256] = "";
    char id[256] = "";
    sprintf(nombre, "%sU%s", p_afnd1O_1->nombre, p_afnd1O_2->nombre);
    AFND* p_afnd;
    p_afnd = p_afnd1O_1;
    p_afnd = p_afnd1O_2;

    AFND* p_afndret = AFNDNuevo(nombre, p_afnd1O_1->num_estados + p_afnd1O_2->num_estados + 2, p_afnd1O_1->longitud_alfabeto + p_afnd1O_2->longitud_alfabeto);
    p_afnd = p_afndret;

    /* Copiamos alfabeto de AFND1 y 2 en el AFND union*/
    for (i = 0; i < p_afnd1O_1->num_simbolos_insertados; i++) {
        p_afndret->alfabeto[p_afndret->num_simbolos_insertados] = (Simbolo) malloc(sizeof (char) * (strlen(p_afnd1O_1->alfabeto[i]) + 1));
        strcpy(p_afndret->alfabeto[p_afndret->num_simbolos_insertados], p_afnd1O_1->alfabeto[i]);
        p_afndret->num_simbolos_insertados++;
    }

    /*¿Es necesario eliminar duplicados?*/
    for (i = 0; i < p_afnd1O_2->num_simbolos_insertados; i++) {
        p_afndret->alfabeto[p_afndret->num_simbolos_insertados] = (Simbolo) malloc(sizeof (char) * (strlen(p_afnd1O_2->alfabeto[i]) + 1));
        strcpy(p_afndret->alfabeto[p_afndret->num_simbolos_insertados], p_afnd1O_2->alfabeto[i]);
        p_afndret->num_simbolos_insertados++;
    }

    /* Copiamos Estados de AFND 1 y 2 en AFND Union*/
    sprintf(id, "%d", p_afndret->id);
    for (i = 0; i < p_afnd1O_1->num_estados_insertados; i++) {
        p_afndret->estados[p_afndret->num_estados_insertados] = (Estado) malloc(sizeof (char)*(strlen(p_afnd1O_1->estados[i]) + strlen(id) + 1 + 4)); /*EL 4 es por "_ _A1" o "_A2"*/
        sprintf(p_afndret->estados[p_afndret->num_estados_insertados], "%s_%s_A1", p_afnd1O_1->estados[i], id);
        p_afndret->num_estados_insertados++;
    }
    for (i = 0; i < p_afnd1O_2->num_estados_insertados; i++) {
        p_afndret->estados[p_afndret->num_estados_insertados] = (Estado) malloc(sizeof (char)*(strlen(p_afnd1O_2->estados[i]) + strlen(id) + 1 + 4)); /*EL 4 es por "_A1" o "_A2"*/
        sprintf(p_afndret->estados[p_afndret->num_estados_insertados], "%s_%s_A2", p_afnd1O_2->estados[i], id);
        p_afndret->num_estados_insertados++;
    }
    /* Copiamos Transiciones y Transiciones Lambda de AFND1 y AFND2 en AFND Union*/
    for (i = 0; i < p_afnd1O_1->num_transiciones_insertadas; i++) {
        p_afndret->transiciones[p_afndret->num_transiciones_insertadas].origen = getEstado(p_afndret, *p_afnd1O_1->transiciones[i].origen, 1);
        p_afndret->transiciones[p_afndret->num_transiciones_insertadas].simbolo = getSimbolo(p_afndret, *p_afnd1O_1->transiciones[i].simbolo);
        p_afndret->transiciones[p_afndret->num_transiciones_insertadas].destino = getEstado(p_afndret, *p_afnd1O_1->transiciones[i].destino, 1);
        p_afndret->num_transiciones_insertadas++;

    }

    for (i = 0; i < p_afnd1O_2->num_transiciones_insertadas; i++) {
        p_afndret->transiciones[p_afndret->num_transiciones_insertadas].origen = getEstado(p_afndret, *p_afnd1O_2->transiciones[i].origen, 2);
        p_afndret->transiciones[p_afndret->num_transiciones_insertadas].simbolo = getSimbolo(p_afndret, *p_afnd1O_2->transiciones[i].simbolo);
        p_afndret->transiciones[p_afndret->num_transiciones_insertadas].destino = getEstado(p_afndret, *p_afnd1O_2->transiciones[i].destino, 2);
        p_afndret->num_transiciones_insertadas++;

    }
    /* Ahora transiciones Lambda */
    for (i = 0; i < p_afnd1O_1->num_transiciones_l_insertadas; i++) {
        p_afndret->transiciones_l[p_afndret->num_transiciones_l_insertadas].origen = getEstado(p_afndret, *p_afnd1O_1->transiciones_l[i].origen, 1);
        p_afndret->transiciones_l[p_afndret->num_transiciones_l_insertadas].destino = getEstado(p_afndret, *p_afnd1O_1->transiciones_l[i].destino, 1);
        p_afndret->num_transiciones_l_insertadas++;
    }

    for (i = 0; i < p_afnd1O_2->num_transiciones_l_insertadas; i++) {
        p_afndret->transiciones_l[p_afndret->num_transiciones_l_insertadas].origen = getEstado(p_afndret, *p_afnd1O_2->transiciones_l[i].origen, 2);
        p_afndret->transiciones_l[p_afndret->num_transiciones_l_insertadas].destino = getEstado(p_afndret, *p_afnd1O_2->transiciones_l[i].destino, 2);
        p_afndret->num_transiciones_l_insertadas++;
    }
    
    AFNDInsertaEstado(p_afndret, "INICIAL10UNION", INICIAL);
    AFNDInsertaEstado(p_afndret, "FINAL10UNION", FINAL);

    /* Transicion del nuevo estado inicial a los iniciales de los automatas del arg. */
    p_afndret->transiciones_l[p_afndret->num_transiciones_l_insertadas].origen = p_afndret->estado_inicial;
    p_afndret->transiciones_l[p_afndret->num_transiciones_l_insertadas].destino = getEstado(p_afndret, *p_afnd1O_1->estado_inicial, 1);
    p_afndret->num_transiciones_l_insertadas++;

    p_afndret->transiciones_l[p_afndret->num_transiciones_l_insertadas].origen = p_afndret->estado_inicial;
    p_afndret->transiciones_l[p_afndret->num_transiciones_l_insertadas].destino = getEstado(p_afndret, *p_afnd1O_2->estado_inicial, 2);
    p_afndret->num_transiciones_l_insertadas++;

    /* Lo mismo para transiciones_l_iniciales */


    /* Transicion de los estados finales(1 por automata al estar ya formalizados) al estado final del nuevo automata */
    for (i = 0; i < p_afnd1O_1->num_estado_final; i++) {
        p_afndret->transiciones_l[p_afndret->num_transiciones_l_insertadas].origen = getEstado(p_afndret, *p_afnd1O_1->estado_final[i], 1);
        p_afndret->transiciones_l[p_afndret->num_transiciones_l_insertadas].destino = p_afndret->estado_final[0];
        p_afndret->num_transiciones_l_insertadas++;

    }
    for (i = 0; i < p_afnd1O_2->num_estado_final; i++) {
        p_afndret->transiciones_l[p_afndret->num_transiciones_l_insertadas].origen = getEstado(p_afndret, *p_afnd1O_2->estado_final[i], 2);
        p_afndret->transiciones_l[p_afndret->num_transiciones_l_insertadas].destino = p_afndret->estado_final[0];
        p_afndret->num_transiciones_l_insertadas++;

    }
        
    /* Hacemos el cierre transitivo y reflexivo */
    p_afndret = AFNDCierraLTransicion(p_afndret);
    
    return p_afndret;

}

AFND * AFND1OConcatena(AFND * p_afnd1O_1, AFND * p_afnd1O_2) {

    if (!p_afnd1O_1 || !p_afnd1O_2)
        return NULL;

    int i, j, flag = 0;

    /* Reservamos memoria para el "nombre del estado+id" y lo creamos */
    char nombre[256] = "";
    char id[256] = "";
    sprintf(nombre, "%sU%s", p_afnd1O_1->nombre, p_afnd1O_2->nombre);

    AFND* p_afndret = AFNDNuevo(nombre, p_afnd1O_1->num_estados + p_afnd1O_2->num_estados + 2, p_afnd1O_1->longitud_alfabeto + p_afnd1O_2->longitud_alfabeto);


    /* Copiamos alfabeto de AFND1 y 2 en el AFND union*/
    for (i = 0; i < p_afnd1O_1->num_simbolos_insertados; i++) {
        p_afndret->alfabeto[p_afndret->num_simbolos_insertados] = (Simbolo) malloc(sizeof (char) * (strlen(p_afnd1O_1->alfabeto[i]) + 1));
        strcpy(p_afndret->alfabeto[p_afndret->num_simbolos_insertados], p_afnd1O_1->alfabeto[i]);
        p_afndret->num_simbolos_insertados++;
    }

    /*¿Es necesario eliminar duplicados?*/
    for (i = 0; i < p_afnd1O_2->num_simbolos_insertados; i++) {
        flag = 0;
        for (j= 0; j < p_afndret->num_simbolos_insertados; j++){
            if ( strcmp(p_afndret->alfabeto[j], p_afnd1O_2->alfabeto[i]) == 0)
                flag = 1;
        }
        if (flag == 0){
            p_afndret->alfabeto[p_afndret->num_simbolos_insertados] = (Simbolo) malloc(sizeof (char) * (strlen(p_afnd1O_2->alfabeto[i]) + 1));
            strcpy(p_afndret->alfabeto[p_afndret->num_simbolos_insertados], p_afnd1O_2->alfabeto[i]);
            p_afndret->num_simbolos_insertados++;        
        }
    }

    /* Copiamos Estados de AFND 1 y 2 en AFND Union*/
    sprintf(id, "%d", p_afndret->id);
    for (i = 0; i < p_afnd1O_1->num_estados_insertados; i++) {
        p_afndret->estados[p_afndret->num_estados_insertados] = (Estado) malloc(sizeof (char)*(strlen(p_afnd1O_1->estados[i]) + strlen(id) + 1 + 4)); /*EL 4 es por "_A1" o "_A2"*/
        sprintf(p_afndret->estados[p_afndret->num_estados_insertados], "%s_%s_A1", p_afnd1O_1->estados[i], id);
        p_afndret->num_estados_insertados++;
    }
    for (i = 0; i < p_afnd1O_2->num_estados_insertados; i++) {
        p_afndret->estados[p_afndret->num_estados_insertados] = (Estado) malloc(sizeof (char)*(strlen(p_afnd1O_2->estados[i]) + strlen(id) + 1 + 4)); /*EL 4 es por "_A1" o "_A2"*/
        sprintf(p_afndret->estados[p_afndret->num_estados_insertados], "%s_%s_A2", p_afnd1O_2->estados[i], id);
        p_afndret->num_estados_insertados++;
    }
    /* Copiamos Transiciones y Transiciones Lambda de AFND1 y AFND2 en AFND Union*/
    for (i = 0; i < p_afnd1O_1->num_transiciones_insertadas; i++) {
        p_afndret->transiciones[p_afndret->num_transiciones_insertadas].origen = getEstado(p_afndret, *p_afnd1O_1->transiciones[i].origen, 1);
        p_afndret->transiciones[p_afndret->num_transiciones_insertadas].simbolo = getSimbolo(p_afndret, *p_afnd1O_1->transiciones[i].simbolo);
        p_afndret->transiciones[p_afndret->num_transiciones_insertadas].destino = getEstado(p_afndret, *p_afnd1O_1->transiciones[i].destino, 1);
        p_afndret->num_transiciones_insertadas++;

    }

    for (i = 0; i < p_afnd1O_2->num_transiciones_insertadas; i++) {
        p_afndret->transiciones[p_afndret->num_transiciones_insertadas].origen = getEstado(p_afndret, *p_afnd1O_2->transiciones[i].origen, 2);
        p_afndret->transiciones[p_afndret->num_transiciones_insertadas].simbolo = getSimbolo(p_afndret, *p_afnd1O_2->transiciones[i].simbolo);
        p_afndret->transiciones[p_afndret->num_transiciones_insertadas].destino = getEstado(p_afndret, *p_afnd1O_2->transiciones[i].destino, 2);
        p_afndret->num_transiciones_insertadas++;

    }
    /* TRANSICIONES LAMBDA DE LOS DOS AUTOMATAS */
    for (i = 0; i < p_afnd1O_1->num_transiciones_l_insertadas; i++) {
        p_afndret->transiciones_l[p_afndret->num_transiciones_l_insertadas].origen = getEstado(p_afndret, *p_afnd1O_1->transiciones_l[i].origen, 1);
        p_afndret->transiciones_l[p_afndret->num_transiciones_l_insertadas].destino = getEstado(p_afndret, *p_afnd1O_1->transiciones_l[i].destino, 1);
        p_afndret->num_transiciones_l_insertadas++;
    }

    for (i = 0; i < p_afnd1O_2->num_transiciones_l_insertadas; i++) {
        p_afndret->transiciones_l[p_afndret->num_transiciones_l_insertadas].origen = getEstado(p_afndret, *p_afnd1O_2->transiciones_l[i].origen, 2);
        p_afndret->transiciones_l[p_afndret->num_transiciones_l_insertadas].destino = getEstado(p_afndret, *p_afnd1O_2->transiciones_l[i].destino, 2);
        p_afndret->num_transiciones_l_insertadas++;
    }

    AFNDInsertaEstado(p_afndret, "INICIAL1CONCATENA", INICIAL);
    AFNDInsertaEstado(p_afndret, "FINAL10CONCATENA", FINAL);

    /* Transicion del nuevo estado inicial al estado inicial del primer automata(En transiciones Lambda y transiciones Lambda iniciales) */
    p_afndret->transiciones_l[p_afndret->num_transiciones_l_insertadas].origen = p_afndret->estado_inicial;
    p_afndret->transiciones_l[p_afndret->num_transiciones_l_insertadas].destino = getEstado(p_afndret, *p_afnd1O_1->estado_inicial, 1);
    p_afndret->num_transiciones_l_insertadas++;

    /* Transicion del estado final del primer automata al estado inicial del segundo (En transiciones Lambda y transiciones Lambda iniciales)*/
    for (i = 0; i < p_afnd1O_1->num_estado_final; i++) {
        p_afndret->transiciones_l[p_afndret->num_transiciones_l_insertadas].origen = getEstado(p_afndret, *p_afnd1O_1->estado_final[i], 1);
        p_afndret->transiciones_l[p_afndret->num_transiciones_l_insertadas].destino = getEstado(p_afndret, *p_afnd1O_2->estado_inicial, 2);
        p_afndret->num_transiciones_l_insertadas++;

    }

    /* Transiciones de los estados finales del segundo al estado final mediante lambdas (En transiciones Lambda y transiciones Lambda iniciales)*/
    for (i = 0; i < p_afnd1O_2->num_estado_final; i++) {
        p_afndret->transiciones_l[p_afndret->num_transiciones_l_insertadas].origen = getEstado(p_afndret, *p_afnd1O_2->estado_final[i], 2);
        p_afndret->transiciones_l[p_afndret->num_transiciones_l_insertadas].destino = p_afndret->estado_final[0];
        p_afndret->num_transiciones_l_insertadas++;

    }
        
    /* Hacemos el cierre transitivo y reflexivo */
    p_afndret = AFNDCierraLTransicion(p_afndret);
    
    return p_afndret;

}

AFND * AFND1OEstrella(AFND * p_afnd1O_1) {
    if (!p_afnd1O_1)
        return NULL;

    int i;

    /* Reservamos memoria para el "nombre del estado+id" y lo creamos */
    char nombre[256] = "";
    char id[256] = "";
    sprintf(nombre, "ESTRELLA_%s", p_afnd1O_1->nombre);
    AFND* p_afndret = AFNDNuevo(nombre, p_afnd1O_1->num_estados + 2, p_afnd1O_1->longitud_alfabeto);


    /* Copiamos alfabeto de AFND1 y 2 en el AFND union*/
    for (i = 0; i < p_afnd1O_1->num_simbolos_insertados; i++) {
        p_afndret->alfabeto[p_afndret->num_simbolos_insertados] = (Simbolo) malloc(sizeof (char) * (strlen(p_afnd1O_1->alfabeto[i]) + 1));
        strcpy(p_afndret->alfabeto[p_afndret->num_simbolos_insertados], p_afnd1O_1->alfabeto[i]);
        p_afndret->num_simbolos_insertados++;
    }

    /* Copiamos Estados de AFND 1 y 2 en AFND Union*/
    sprintf(id, "%d", p_afndret->id);
    for (i = 0; i < p_afnd1O_1->num_estados_insertados; i++) {
        p_afndret->estados[p_afndret->num_estados_insertados] = (Estado) malloc(sizeof (char)*(strlen(p_afnd1O_1->estados[i]) + strlen(id) + 1 + 4)); /*EL 3 es por "_A1" o "_A2"*/
        sprintf(p_afndret->estados[p_afndret->num_estados_insertados], "%s_%s_A1", p_afnd1O_1->estados[i], id);
        p_afndret->num_estados_insertados++;
    }

    /* Copiamos Transiciones y Transiciones Lambda de AFND1 y AFND2 en AFND Union*/
    for (i = 0; i < p_afnd1O_1->num_transiciones_insertadas; i++) {
        p_afndret->transiciones[p_afndret->num_transiciones_insertadas].origen = getEstado(p_afndret, *p_afnd1O_1->transiciones[i].origen, 1);
        p_afndret->transiciones[p_afndret->num_transiciones_insertadas].simbolo = getSimbolo(p_afndret, *p_afnd1O_1->transiciones[i].simbolo);
        p_afndret->transiciones[p_afndret->num_transiciones_insertadas].destino = getEstado(p_afndret, *p_afnd1O_1->transiciones[i].destino, 1);
        p_afndret->num_transiciones_insertadas++;

    }

    for (i = 0; i < p_afnd1O_1->num_transiciones_l_insertadas; i++) {
        p_afndret->transiciones_l[p_afndret->num_transiciones_l_insertadas].origen = getEstado(p_afndret, *p_afnd1O_1->transiciones_l[i].origen, 1);
        p_afndret->transiciones_l[p_afndret->num_transiciones_l_insertadas].destino = getEstado(p_afndret, *p_afnd1O_1->transiciones_l[i].destino, 1);
        p_afndret->num_transiciones_l_insertadas++;
    }


    AFNDInsertaEstado(p_afndret, "INICIAL10ESTRELLA", INICIAL);
    AFNDInsertaEstado(p_afndret, "FINAL10ESTRELLA", FINAL);

    /* Transicion del nuevo estado inicial al estado inicial del primer automata. */
    p_afndret->transiciones_l[p_afndret->num_transiciones_l_insertadas].origen = p_afndret->estado_inicial;
    p_afndret->transiciones_l[p_afndret->num_transiciones_l_insertadas].destino = getEstado(p_afndret, *p_afnd1O_1->estado_inicial, 1);
    p_afndret->num_transiciones_l_insertadas++;

    /* Transicion del estado final del primer automata al estado inicial del segundo */
    for (i = 0; i < p_afnd1O_1->num_estado_final; i++) {
        p_afndret->transiciones_l[p_afndret->num_transiciones_l_insertadas].origen = getEstado(p_afndret, *p_afnd1O_1->estado_final[i], 1);
        p_afndret->transiciones_l[p_afndret->num_transiciones_l_insertadas].destino = p_afndret->estado_final[0];
        p_afndret->num_transiciones_l_insertadas++;
    }

    p_afndret->transiciones_l[p_afndret->num_transiciones_l_insertadas].origen = p_afndret->estado_inicial;
    p_afndret->transiciones_l[p_afndret->num_transiciones_l_insertadas].destino = p_afndret->estado_final[0];
    p_afndret->num_transiciones_l_insertadas++;
 

    p_afndret->transiciones_l[p_afndret->num_transiciones_l_insertadas].origen = p_afndret->estado_final[0];
    p_afndret->transiciones_l[p_afndret->num_transiciones_l_insertadas].destino = p_afndret->estado_inicial;    
    p_afndret->num_transiciones_l_insertadas++;
        
    /* Hacemos el cierre transitivo y reflexivo */
    p_afndret = AFNDCierraLTransicion(p_afndret);
    
    return p_afndret;
}

Simbolo* getSimbolo(AFND * p_afnd, char* nombreSimbolo) {

    if (!p_afnd || !nombreSimbolo)
        return NULL;
    int i;
    for (i = 0; i < p_afnd->num_simbolos_insertados; i++)
        if (strcmp(nombreSimbolo, p_afnd->alfabeto[i]) == 0)
            return &p_afnd->alfabeto[i];
    return NULL;

}

Estado* getEstado(AFND * p_afnd, char* nombreEstado, int numautomata) {

    if (!p_afnd || !nombreEstado)
        return NULL;
    int i;
    char nombre[256] = "";

    if (numautomata == 1)
        sprintf(nombre, "%s_%d_A1", nombreEstado, p_afnd->id);
    else if (numautomata == 2)
        sprintf(nombre, "%s_%d_A2", nombreEstado, p_afnd->id);
    else
        sprintf(nombre, "%s_%d", nombreEstado, p_afnd->id);

    for (i = 0; i < p_afnd->num_estados_insertados; i++)
        if (strcmp(nombre, p_afnd->estados[i]) == 0)
            return &p_afnd->estados[i];
    return NULL;

}

