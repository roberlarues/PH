/*********************************************************************************************
* Fichero:	main.c
* Autor:
* Descrip:	punto de entrada de C
* Version:  <P4-ARM.timer-leds>
*********************************************************************************************/

/*--- ficheros de cabecera ---*/
#include <inttypes.h>
//#include "44blib.h"
//#include "44b.h"
#include "stdio.h"

uint32_t MEMStacks = 0x0c7FE000;// DIrección de las pilas
uint32_t MEMDStack;// Dirección de la pila de debug
int *SPDStack;// Stack pointer de la pila de debug
int SDStack; // Tamaño de la pila

/*--- variables globales ---*/
extern int switch_leds;
extern int estado_boton4;
extern int estado_boton8;
extern int time_start_button4;
extern int time_start_button8;
extern int trp;
extern int trd;

/*--- funciones externas ---*/
extern void leds_off();
extern void led1_on();
extern void leds_switch();
extern void timer_init();
extern void Eint4567_init();
extern void D8Led_init();

/*--- declaracion de funciones ---*/
void Main(void);
void init_debug_stack();

/*--- codigo de funciones ---*/
void Main(void)
{

	/* Inicializa controladores */
	sys_init();        // Inicializacion de la placa, interrupciones y puertos
	timer_init();	   // Inicializacion del temporizador
	Eint4567_init();	// inicializamos los pulsadores. Cada vez que se pulse se verá reflejado en el 8led
	D8Led_init(); // inicializamos el 8led


	/* Valor inicial de los leds */
	leds_off();
	led1_on();

	while (1)
	{
		/* Cambia los leds con cada interrupcion del temporizador */
		if (switch_leds == 1)
		{
			leds_switch();
			switch_leds = 0;
		}

		if (estado_boton4 == 1){//Evitar rebotes al pulsar
			if (timer2_leer()-time_start_button4 >= trp) {
				estado_boton4 = 2;
				time_start_button4 = timer2_leer();
			}
		} else if (estado_boton4 == 2){//Pulsado
			if (timer2_leer() - time_start_button4 >= 10000){
				if
				time_start_button4 = timer2_leer();
			}
		} else if (estado_boton4 == 3){//Evitar rebotes al soltar
			if (timer2_leer() - time_start_button4 >= trd){
				estado_boton4 = 0;
			}
		}
	}
}


/**
 * Reserva espacio para n llamadas y pone el puntero al principio.
 */
void init_debug_stack(int n)
{
	MEMDStack = MEMStacks - n*3*sizeof(int);// 3 enteros
	SDStack = n;
	SPDStack = (int*)MEMDStack;
}

/**
 * Guarda la id del evento, información adicional y el momento en el que se ha
 * llamado al método.
 */
void push_debug(int ID_evento, int auxData)
{
	*SPDStack = ID_evento;
	*(SPDStack+sizeof(int)) = auxData;
	*(SPDStack+sizeof(int)*2) = timer2_leer();
	SPDStack += sizeof(int)*3;
	if (SPDStack==(int*)MEMStacks) SPDStack = (int*)MEMDStack;//Si se llena la pila volver al comienzo.

}

