#include <inttypes.h>

#define MEMStacks 0x0C7FE000           // Dirección de las pilas

uint32_t MEMDStack;                    // Dirección de la pila de debug
uint32_t *SPDStack;                    // Stack pointer de la pila de debug

/**
 * Reserva espacio para n llamadas y coloca el puntero al principio.
 */
void init_debug_stack(int n)
{
   MEMDStack = MEMStacks - n * 3 * 4;  // 3 elementos.
   SPDStack = (uint32_t *) MEMDStack;
}

/**
 * Guarda la id del evento, información adicional y el momento en el que se ha
 * llamado al método.
 */
void push_debug(int ID_evento, int auxData)
{
   *(SPDStack++) = ID_evento;
   *(SPDStack++) = auxData;
   *(SPDStack++) = timer2_leer();

// Si se llena la pila vuelve al inicio de la pila.
   if (SPDStack == (uint32_t *) MEMStacks)
      SPDStack = (uint32_t *) MEMDStack;
}
