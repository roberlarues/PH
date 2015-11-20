#include <inttypes.h>
//#include <string.h>
//#include <stdio.h>

// Tamaños de la cuadricula
// Se utilizan 16 columnas para facilitar la visualización
enum {NUM_FILAS = 9, NUM_COLUMNAS = 16, CELDAS_TOTALES = 81};

enum {FALSE = 0, TRUE = 1, ERROR = -1};

typedef uint16_t CELDA;
#define	ERROR_MASK	0x0400
// La información de cada celda está codificada en 16 bits:
// bbbb  b       b     b     b bbbb bbbb
// VALOR INICIAL ERROR VACÍO CANDIDATOS(indicados con 0)

void init_debug_stack();


uint32_t MEMStacks = 0x0c7FE000;// DIrección de las pilas
uint32_t MEMDStack;// Dirección de la pila de debug
int *SPDStack;// Stack pointer de la pila de debug
int SDStack; // Tamaño de la pila
CELDA cuadricula1[NUM_FILAS][NUM_COLUMNAS];
CELDA cuadricula2[NUM_FILAS][NUM_COLUMNAS];
CELDA cuadricula3[NUM_FILAS][NUM_COLUMNAS];
CELDA cuadricula4[NUM_FILAS][NUM_COLUMNAS];
CELDA cuadricula5[NUM_FILAS][NUM_COLUMNAS];
CELDA cuadricula6[NUM_FILAS][NUM_COLUMNAS];

inline uint8_t celda_leer_valor(CELDA celda)
{
    return celda >> 12;
}

inline void celda_poner_valor(CELDA *celdaptr, uint8_t val)
{
    *celdaptr = (*celdaptr & 0x0FFF) | ((val & 0x000F) << 12);
}

// Funcion a implementar en ARM.
extern int sudoku_recalcular_arm(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS]);
extern int sudoku_recalcular_arm_c(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS]);
extern int sudoku_candidatos_arm(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS], uint8_t fila, uint8_t columna);

extern void timer2_inicializar();
extern void timer2_empezar();
extern int timer2_leer();
// Funcion a implementar en Thumb.
extern int sudoku_candidatos_c_thumb(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS], uint8_t fila, uint8_t columna);
extern int sudoku_recalcular_arm_thumb(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS]);

/**
 * Recorre la fila de la celda y crea la máscara de bits con los valores encontrados.
 *
 * @param cuadricula: Dirección de la primera celda del sudoku.
 * @param fila: Fila de la celda a investigar.
 *
 * @return: Devuelve la máscara de bits con los candidatos a 0.
 */
uint16_t calculate_fila_c(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS], uint8_t fila, uint8_t columna)
{
   uint8_t j, valor;
   uint16_t candidatos = 0x0000;

   for (j = 0; j < NUM_FILAS; j++)
   {
	  if (j!=columna)
	  {
		  valor = celda_leer_valor(cuadricula[fila][j]);
		  if (valor > 0)
			 candidatos |= 1 << (valor - 1);
	  }
   }

   return candidatos;
}

/**
 * Recorre la columna de la celda y crea la máscara de bits con los valores encontrados.
 *
 * @param cuadricula: Dirección de la primera celda del sudoku.
 * @param columna: Columna de la celda a investigar.
 *
 * @return: Devuelve la máscara de bits con los candidatos a 0.
 */
uint16_t calculate_columna_c(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS], uint8_t fila, uint8_t columna)
{
   uint8_t i, valor;
   uint16_t candidatos = 0x0000;

   for (i = 0; i < NUM_FILAS; i++)
   {
	   if (i!=fila)
	   {
		  valor = celda_leer_valor(cuadricula[i][columna]);
		  if (valor > 0)
			 candidatos |= 1 << (valor - 1);
	   }
   }

   return candidatos;
}

/**
 * Recorre la región interna de la celda y crea la máscara de bits con los valores encontrados.
 *
 * @param cuadricula: Dirección de la primera celda del sudoku.
 * @param fila: Fila de la celda a investigar.
 * @param columna: Columna de la celda a investigar.
 *
 * @return: Devuelve la máscara de bits con los candidatos a 0.
 */
uint16_t calculate_region_c(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS], uint8_t fila, uint8_t columna)
{
   uint8_t i, j, k, l, valor;
   uint16_t candidatos = 0x0000;

   k = 3 * (fila / 3);                 // K: Fila inicial de la región: 0, 3, 6.
   l = 3 * (columna / 3);              // L: Columna inicial de la región: 0, 3, 6.

   for (i = k; i < k+3; i++)
      for (j = l; j < l+3; j++)
      {
    	  if (i!=fila && j!=columna)
    	  {
			 valor = celda_leer_valor(cuadricula[i][j]);
			 if (valor > 0)
				candidatos |= 1 << (valor - 1);
    	  }
      }

   return candidatos;
}

/**
 * Dada una determinada celda encuentra los posibles valores y guarda el mapa de bits en la celda.
 *
 * @param cuadricula: Dirección de la primera celda del sudoku.
 * @param fila: Fila de la celda a investigar.
 * @param columna: Columna de la celda a investigar.
 *
 * @return: FALSE si la celda esta vacía.
 *          TRUE  si contiene un valor.
 */
int sudoku_candidatos_c(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS], uint8_t fila, uint8_t columna)
{
   // Inicializa el mapa de bits de candidatos.
   uint16_t candidatos = 0x0000;
   uint8_t valor = cuadricula[fila][columna]>>12;
   
	
   candidatos |= calculate_fila_c(cuadricula, fila, columna);
   candidatos |= calculate_columna_c(cuadricula, fila, columna);
   candidatos |= calculate_region_c(cuadricula, fila, columna);

   cuadricula[fila][columna] |= candidatos;

   if (valor>0)
   {
	if (candidatos && 1<<(valor-1))
	{
		cuadricula[fila][columna] |= ERROR_MASK;
		return ERROR;
	}
   }
   // Si la celda ya tiene un valor devuelve no vacia.
   if ((cuadricula[fila][columna] & 0xF000) != 0)
      return TRUE;

   return FALSE;
}

/**
 * Calcula los posibles candidatos para todas las celdas vacías.
 *
 * @param cuadricula: Dirección de la primera celda del sudoku.
 *
 * @return: Devuelve el número de celdas vacías.
 */
int sudoku_recalcular_c(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS])
{
   int i, j, numCeldasVacias = CELDAS_TOTALES;
   int valor;
   for (i = 0; i < NUM_FILAS; i++)
   {
      for (j = 0; j < NUM_FILAS; j++)
      {
    	 valor = sudoku_candidatos_c(cuadricula, i, j);
		 if (valor == ERROR) {
			numCeldasVacias =-1;
		 } else {
			numCeldasVacias -= valor;
		 }
      }
   }

   return numCeldasVacias;
}

/**
 * Calcula los posibles candidatos para todas las celdas vacías. Cada celda
 * la calcula usando instrucciones ARM.
 *
 * @param cuadricula: Dirección de la primera celda del sudoku.
 *
 * @return: Devuelve el número de celdas vacías.
 */
int sudoku_recalcular_c_arm(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS])
{
   int i, j, numCeldasVacias = CELDAS_TOTALES;
   int valor;

   for (i = 0; i < NUM_FILAS; i++){
	   for (j = 0; j < NUM_FILAS; j++)
	   {
		 valor = sudoku_candidatos_armU9(cuadricula, i, j);
		 if (valor == ERROR) {
			numCeldasVacias =-1;
		 } else {
			numCeldasVacias -= valor;
		 }
	   }
   }

   return numCeldasVacias;
}

/**
 * Calcula los posibles candidatos para todas las celdas vacías. Cada celda
 * la calcula usando instrucciones Thumb.
 *
 * @param cuadricula: Dirección de la primera celda del sudoku.
 *
 * @return: Devuelve el número de celdas vacías.
 */
int sudoku_recalcular_c_thumb(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS])
{
   int i, j, numCeldasVacias = CELDAS_TOTALES;
   int valor;

   for (i = 0; i < NUM_FILAS; i++){
	   for (j = 0; j < NUM_FILAS; j++)
	   {
		 valor = sudoku_candidatos_c_thumb(cuadricula, i, j);
		 if (valor == ERROR) {
			numCeldasVacias =-1;
		 } else {
			numCeldasVacias -= valor;
		 }
	   }
   }
   return numCeldasVacias;
}


/**
 * Función principal del juego que recibe el tablero,
 * y la señal de ready que indica que se han actualizado fila y columna
 *
 * @param cuadricula: Dirección de la primera celda del sudoku.
 * @param ready: Indica que se han actualizado fila y columna
 *
 */
void sudoku9x9(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS], char *ready)
{
   unsigned int celdas_vacias;         // Número de celdas aún vacias.
   unsigned char value1, value2, value3, value4, value5;
   /*timer2_inicializar();
   init_debug_stack(10);
   int i = 0;
   while(i<15){
	   push_debug(i,55);
	   i++;
   }*/

   /*
    * Usado para la verificación del resultado de las ejecuciones.
    * Copia el tablero inicial para cada una de las combinaciones posibles.
    */

   // Usado para realizar calculos de tiempo.
   //int i;
   //for(i=0;i<10000;i++)
   celdas_vacias = sudoku_recalcular_c(cuadricula);
   //celdas_vacias = sudoku_recalcular_c_arm(cuadricula2);
   //celdas_vacias = sudoku_recalcular_armU9(cuadricula3);
   //celdas_vacias = sudoku_recalcular_arm_c(cuadricula4);
   //celdas_vacias = sudoku_recalcular_c_thumb(cuadricula5);
   //celdas_vacias = sudoku_recalcular_arm_thumb(cuadricula6);

   // Comparar resultado de la ejecucion c-c con el resto. Usado para verificar el resultado.
   //value1 = memcmp(cuadricula1, cuadricula2, NUM_FILAS * NUM_COLUMNAS * sizeof(CELDA));
   //value2 = memcmp(cuadricula1, cuadricula3, NUM_FILAS * NUM_COLUMNAS * sizeof(CELDA));
   //value3 = memcmp(cuadricula1, cuadricula4, NUM_FILAS * NUM_COLUMNAS * sizeof(CELDA));
   //value4 = memcmp(cuadricula1, cuadricula5, NUM_FILAS * NUM_COLUMNAS * sizeof(CELDA));
   //value5 = memcmp(cuadricula1, cuadricula6, NUM_FILAS * NUM_COLUMNAS * sizeof(CELDA));

}
