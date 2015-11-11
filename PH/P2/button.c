/*********************************************************************************************
* Fichero:	button.c
* Autor:
* Descrip:	Funciones de manejo de los pulsadores (EINT6-7)
* Version:
*********************************************************************************************/

/*--- ficheros de cabecera ---*/
#include "44blib.h"
#include "44b.h"
#include "def.h"

/*--- variables globales ---*/
int trp = 100;
int trd = 100;
int ESPERAR = 0;
int PULSANDO = 1;
int PULSADO = 2;
int SOLTANDO = 3;
int autoincrement = 0;
int nuevo_tiempo = 0;
int boton = rEXTINTPND;
int estado = PULSANDO;
int pasa1 = 0;
int pasan50 = 0;
int pasan30 = 0;
int tiempo_puls = timer4_leer();
/* int_count la utilizamos para sacar un n�mero por el 8led.
  Cuando se pulsa un bot�n sumamos y con el otro restamos. �A veces hay rebotes! */
unsigned int int_count = 0;

/*--- declaracion de funciones ---*/
void Eint4567_ISR(void) __attribute__((interrupt("IRQ")));
void Eint4567_init(void);
extern void D8Led_symbol(int value); // declaramos la funci�n que escribe en el 8led

/*--- codigo de funciones ---*/
void Eint4567_init(void)
{
	/* Configuracion del controlador de interrupciones. Estos registros est�n definidos en 44b.h */
	rI_ISPC    = 0x3ffffff;	// Borra INTPND escribiendo 1s en I_ISPC
	rEXTINTPND = 0xf;       // Borra EXTINTPND escribiendo 1s en el propio registro
	rINTMOD    = 0x0;		// Configura las linas como de tipo IRQ
	rINTCON    = 0x1;	    // Habilita int. vectorizadas y la linea IRQ (FIQ no)
	rINTMSK    = ~(BIT_GLOBAL | BIT_EINT4567 | BIT_TIMER0); // Enmascara todas las lineas excepto eint4567, el bit global y el timer0

	/* Establece la rutina de servicio para Eint4567 */
	pISR_EINT4567 = (int)Eint4567_sin_rebotes;

	/* Configuracion del puerto G */
	rPCONG  = 0xffff;        		// Establece la funcion de los pines (EINT0-7)
	rPUPG   = 0x0;                  // Habilita el "pull up" del puerto
	rEXTINT = rEXTINT | 0x22222222;   // Configura las lineas de int. como de flanco de bajada

	/* Por precaucion, se vuelven a borrar los bits de INTPND y EXTINTPND */
	rI_ISPC    |= (BIT_EINT4567);
	rEXTINTPND = 0xf;
}

void Eint4567_sin_rebotes(void)
{
	/* Identificar la interrupcion (hay dos pulsadores)*/
	int which_int = rEXTINTPND;
	switch (which_int)
	{
		case 4:
			if (estado_boton4==0) {
				time_start_button4 = timer2_leer();
				estado_boton4=1;
			}
			break;
		case 8:
			if (estado_boton8==0) {
				time_start_button8 = timer2_leer();
				estado_boton8=1; // incrementamos el contador
			}
			break;
		default:
			break;
	}
	// }
	D8Led_symbol(int_count&0x000f); // sacamos el valor por pantalla (m�dulo 16)
	/* Finalizar ISR */
	rEXTINTPND = 0xf;				// borra los bits en EXTINTPND
	rI_ISPC   |= BIT_EINT4567;		// borra el bit pendiente en INTPND

}

void Eint4567_ISR(void)
{
	/* Identificar la interrupcion (hay dos pulsadores)*/
	boton = rEXTINTPND;
	estado = PULSANDO;
	tiempo_puls = timer4_leer();
	autoincrement = 0;
	/* Finalizar ISR */
	rEXTINTPND = 0xf;				// borra los bits en EXTINTPND
	rI_ISPC   |= BIT_EINT4567;		// borra el bit pendiente en INTPND

}

void comprobar_boton(void)
{
	if(estado==PULSANDO || estado==PULSADO){
		if (pasa1){
			pasa1=0;
			if (estado == PULSADO && (rPDATG&boton==0)) {// Ya no est� pulsado
				estado = SOLTANDO;
			}
		}
		int nuevo_tiempo = timer4_leer();

		if (estado == PULSANDO && nuevo_tiempo-tiempo_puls>=trp){
			estado = PULSADO;
		}
		if (pasan50 && !autoincrement){
			pasan50=0;
			autoincrement=1;
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
			D8Led_symbol(int_count&0x000f); // sacamos el valor por pantalla (m�dulo 16)
		}
		if (pasan30 && autoincrement){
			pasan30=0;
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
			D8Led_symbol(int_count&0x000f); // sacamos el valor por pantalla (m�dulo 16)
		}
	} else if(estado==SOLTANDO){

		int nuevo_tiempo = timer4_leer();

		if (estado == PULSANDO && nuevo_tiempo-tiempo_puls>=trd){
			estado = ESPERAR;
		}
	}
}
