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

/*--- funciones externas ---*/
extern void leds_off();
extern void led1_on();
extern void leds_switch();
extern void timer_init();
extern void mybutton_init();
extern void comprobar_boton();
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
	mybutton_init();	// inicializamos los pulsadores. Cada vez que se pulse se verá reflejado en el 8led
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

		comprobar_boton();
	}
}

