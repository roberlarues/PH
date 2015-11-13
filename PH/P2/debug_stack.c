
#include <inttypes.h>

uint32_t MEMStacks = 0x0c7FE000;// DIrección de las pilas
uint32_t MEMDStack;// Dirección de la pila de debug
int *SPDStack;// Stack pointer de la pila de debug
int SDStack; // Tamaño de la pila

/**
 * Reserva espacio para n llamadas y pone el puntero al principio.
 */
void init_debug_stack(int n)
{
	MEMDStack = MEMStacks - n*3*4;// 3 enteros
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
	*(SPDStack+1) = auxData;
	*(SPDStack+2) = 7;//timer2_leer();
	SPDStack += 3;
	if (SPDStack==(int*)MEMStacks) SPDStack = (int*)MEMDStack;//Si se llena la pila volver al comienzo.

}
