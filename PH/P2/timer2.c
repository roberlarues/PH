#include <inttypes.h>
#include "44b.h"
#include "44blib.h"

uint8_t switch_leds2 = 0;
uint32_t timer2_num_int = 0;

void timer2_ISR(void) __attribute__((interrupt("IRQ")));

void timer2_ISR(void)
{
	switch_leds2 = 1;
	timer2_num_int++;
	/* borrar bit en I_ISPC para desactivar la solicitud de interrupci�n*/
	rI_ISPC |= BIT_TIMER2; // BIT_TIMER2 est� definido en 44b.h y pone un uno en el bit 11 que correponde al Timer2
}

void timer2_empezar()
{
	/* establecer update=manual (bit 13) + inverter=on (�? ser� inverter off un cero en el bit 2 pone el inverter en off)*/
	rTCON = 0x2000;

	/* iniciar timer (bit 12) con auto-reload (bit 15)*/
	rTCON = 0x9000;
}

void timer2_inicializar()
{
	/* Configuraion controlador de interrupciones */
	rINTMOD = 0x0; // Configura las linas como de tipo IRQ
	rINTCON = 0x1; // Habilita int. vectorizadas y la linea IRQ (FIQ no)
	rINTMSK = ~(BIT_GLOBAL | BIT_TIMER2); // Emascara todas las lineas excepto Timer0 y el bit global (bits 26 y 13, BIT_GLOBAL y BIT_TIMER0 est�n definidos en 44b.h)

	/* Establece la rutina de servicio para TIMER0 */
	pISR_TIMER2 = (unsigned) timer2_ISR;

	/* Configura el Timer0 */
	rTCFG0 = 31; // ajusta el preescalado
	rTCFG1 = 0x0; // selecciona la entrada del mux que proporciona el reloj. La 00 corresponde a un divisor de 1/2.
	rTCNTB2 = 1000;// valor inicial de cuenta (la cuenta es descendente)
	rTCMPB2 = 0;// valor de comparaci�n
}

uint32_t timer2_leer()
{
	static uint32_t valor_leido;

	valor_leido = timer2_num_int;

	return valor_leido;
}
