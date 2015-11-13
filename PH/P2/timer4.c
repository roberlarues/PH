#include <inttypes.h>
#include "44b.h"
#include "44blib.h"

uint32_t timer4_num_int = 0;
uint32_t intervalo = 10000;

void timer4_ISR(void) __attribute__((interrupt("IRQ")));

void timer4_ISR(void)
{
    timer4_num_int++;
	/* borrar bit en I_ISPC para desactivar la solicitud de interrupción*/
	rI_ISPC |= BIT_TIMER4; // BIT_TIMER2 está definido en 44b.h y pone un uno en el bit 11 que correponde al Timer2
}

void timer4_empezar(uint32_t nuevo_intervalo)
{
   rTCNTB4 = nuevo_intervalo;// valor inicial de cuenta (la cuenta es descendente)

   uint32_t TCON_tmp;

   /* establecer update=manual (bit 21) + inverter=on (¿? será inverter off un cero en el bit 2 pone el inverter en off)*/
   TCON_tmp = rTCON;
   rTCON = (TCON_tmp | 1 << 21);

   /* iniciar timer (bit 20) con auto-reload (bit 23)*/
   rTCON = (TCON_tmp | 1 << 20 | 1 << 23);
}

void timer4_inicializar()
{
	/* Configuraion controlador de interrupciones */
	rINTMOD = 0x0; // Configura las linas como de tipo IRQ
	rINTCON = 0x1; // Habilita int. vectorizadas y la linea IRQ (FIQ no)
	rINTMSK &= ~(BIT_GLOBAL | BIT_TIMER4); // Emascara todas las lineas excepto Timer0 y el bit global (bits 26 y 13, BIT_GLOBAL y BIT_TIMER0 están definidos en 44b.h)

	/* Establece la rutina de servicio para TIMER0 */
	pISR_TIMER4 = (unsigned) timer4_ISR;

	/* Configura el Timer0 */
	rTCFG0 |= 31 << 16; // ajusta el preescalado
	rTCFG1 |= 0 << 16; // selecciona la entrada del mux que proporciona el reloj. La 00 corresponde a un divisor de 1/2.
	rTCMPB4 = 0;// valor de comparación
}

uint32_t timer4_leer()
{
	static uint32_t valor_leido;

	valor_leido = timer4_num_int;

	return valor_leido;
}
