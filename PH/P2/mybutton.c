/*********************************************************************************************
* Fichero:	button.c
* Autor:
* Descrip:	Funciones de manejo de los pulsadores (EINT6-7)
* Version:
*********************************************************************************************/

/*--- ficheros de cabecera ---*/
#include <inttypes.h>
#include "44blib.h"
#include "44b.h"
#include "def.h"

/*--- Constantes ---*/
int trp = 1000000;//microseg
int trd = 1000000;//microseg
int tpuls = 10000;//microseg

enum ESTADOS_BOTON { ESPERANDO, PULSANDO, PULSADO, SOLTANDO, MANTENIDO };
enum ESTADOS_SUDOKU { INICIAL, FILA_INIT, FILA_VALOR, COLUMNA_INIT, COLUMNA_VALOR, VALOR};

/*--- variables globales ---*/
int boton;
int estado = ESPERANDO;
int estado_sudoku = INICIAL;
extern int fila;
extern int columna;
extern int valor;
/* int_count la utilizamos para sacar un número por el 8led.
  Cuando se pulsa un botón sumamos y con el otro restamos. ¡A veces hay rebotes! */
unsigned int int_count = 0;

/*--- declaracion de funciones ---*/
void mybutton_ISR(void) __attribute__((interrupt("IRQ")));
void mybutton_init(void);
void aplicar_efecto_boton(void);
void comprobar_boton(void);
extern void D8Led_symbol(int value); // declaramos la función que escribe en el 8led
extern uint32_t timer4_leer();
extern void timer4_empezar(uint32_t nuevo_intervalo);
extern void timer4_inicializar();
extern int wait_timer4();

/*--- codigo de funciones ---*/
void mybutton_init(void)
{
	/* Configuracion del controlador de interrupciones. Estos registros están definidos en 44b.h */
	rI_ISPC    = 0x3ffffff;	// Borra INTPND escribiendo 1s en I_ISPC
	rEXTINTPND = 0xf;       // Borra EXTINTPND escribiendo 1s en el propio registro
	rINTMOD    = 0x0;		// Configura las linas como de tipo IRQ
	rINTCON    = 0x1;	    // Habilita int. vectorizadas y la linea IRQ (FIQ no)
	rINTMSK    &= ~(BIT_GLOBAL | BIT_EINT4567 | BIT_TIMER0); // Enmascara todas las lineas excepto eint4567, el bit global y el timer0

	/* Establece la rutina de servicio para Eint4567 */
	pISR_EINT4567 = (int)mybutton_ISR;

	/* Inicializa el timer*/
	timer4_inicializar();

	/* Configuracion del puerto G */
	rPCONG  = 0xffff;        		// Establece la funcion de los pines (EINT0-7)
	rPUPG   = 0x0;                  // Habilita el "pull up" del puerto
	rEXTINT = rEXTINT | 0x22222222;   // Configura las lineas de int. como de flanco de bajada

	/* Por precaucion, se vuelven a borrar los bits de INTPND y EXTINTPND */
	rI_ISPC    |= (BIT_EINT4567);
	rEXTINTPND = 0xf;
}

void mybutton_ISR(void)
{
	/* Identificar la interrupcion (hay dos pulsadores)*/
	boton = rEXTINTPND;
	estado = PULSANDO;
	// TODO Desactivar interrupciones
	rINTMSK &= BIT_EINT4567;

	timer4_empezar(trp);
	/* Finalizar ISR */
	rEXTINTPND = 0xf;				// borra los bits en EXTINTPND
	rI_ISPC   |= BIT_EINT4567;		// borra el bit pendiente en INTPND
}

void comprobar_boton(void)
{
	// Actualiza con cada interrupcion del timer
	if (estado != ESPERANDO && wait_timer4()==1)
	{
		switch(estado)
		{
			case PULSANDO:
				aplicar_efecto_boton();
				// TODO Activar interrupciones
				rINTMSK &= ~(BIT_EINT4567);
				timer4_empezar(tpuls);// Reiniciar timer.
				estado = PULSADO;
				break;
			case PULSADO:
				if (rPDATG == boton<<4){
					estado = SOLTANDO;
				} else if (timer4_leer()>=50){
					aplicar_efecto_boton();
					timer4_empezar(tpuls);// Reiniciar timer.
					estado = MANTENIDO;
				}
				break;
			case MANTENIDO:
				if ((rPDATG & (boton<<4)) > 0){
					// TODO Desactivar interrupciones
					rINTMSK &= BIT_EINT4567;
					estado = SOLTANDO;
					timer4_empezar(trd);// Reiniciar timer.
				} else if (timer4_leer()>=30){
					aplicar_efecto_boton();
					timer4_empezar(tpuls);// Reiniciar timer.
				}
				break;
			case SOLTANDO:
				// TODO Activar interrupciones
				rINTMSK &= ~(BIT_EINT4567);
				estado = ESPERANDO;
				break;
			default:
			   estado = ESPERANDO;
			   break;
		}
	}
}

void aplicar_efecto_boton(void)
{
	switch (boton)
	{
		case 4:
			int_count++; // incrementamos el contador
			break;
		case 8:
			int_count--; // decrementamos el contador
			break;
		default:
			break;
	}

   D8Led_symbol(int_count&0x000f);// sacamos el valor por pantalla (módulo 16)
   /*
	switch (boton)
	{
		case 4:
			switch(estado_sudoku)
			{
			case INICIAL:
				// Recalcular tablero
				estado_sudoku = FILA_INIT;
				break;
			case FILA_INIT:
				D8Led_Symbol(0xF);
				estado_sudoku = FILA_VALOR;
				break;
			case FILA_VALOR:
				fila%=9;
				fila++;
				D8Led_Symbol(fila);
				break;
			case COLUMNA_INIT:
				D8Led_Symbol(0xC);
				estado_sudoku = FILA_VALOR;
				break;
			case COLUMNA_VALOR:
				columna%=9;
				columna++;
				D8Led_Symbol(columna);
				break;
			case VALOR:
				valor++%10;
				D8Led_Symbol(valor);
				break;
			default:
				break;
			}
			break;
		case 8:
			switch(estado_sudoku)
			{
			case INICIAL:
				// Recalcular tablero
				estado_sudoku = FILA_INIT;
				break;
			case FILA_VALOR:
				estado_sudoku = COLUMNA_INIT;
				break;
			case COLUMNA_VALOR:
				estado_sudoku = VALOR;
				break;
			case VALOR:
				// Guardar valor celda
				// Recalcular tablero
				estado_sudoku = FILA_INIT;
				break;
			default:
				break;
			}
			break;
		default:
			break;
	}
	*/

}
