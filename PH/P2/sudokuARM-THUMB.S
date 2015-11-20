.global sudoku_recalcular_arm_thumb

@ Espacio para constantes: .equ CONST value
.equ NUM_FILAS, 9
.equ NUM_COLUMNAS, 9
.equ NUM_TOTALES, 81

@ Espacio para variables inicializadas.
.data

@ Espacio para variables sin inicializar.
.bss

@ Comienzo del código.
.text

.arm
@ int sudoku_recalcular_arm(
@ R0  CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS]);
sudoku_recalcular_arm_thumb:
   MOV      IP,   SP                            @ Guarda el valor de la pila para asignar FP.
   STMDB    SP!,  {R4-R10, FP, IP, LR, PC}      @ Prólogo de inicio de función. Actualiza SP.
   SUB      FP,   IP,   #4                      @ Asigna a FP la posición inicial del marco de pila.

@ int i, j, numCeldasVacias = CELDAS_TOTALES;
   MOV      R4,   R0                            @ R4 = @cuadricula
   MOV      R5,   #NUM_TOTALES                  @ R5 = numCeldasVacias

@ for (i = 0; i < NUM_FILAS; i++)
   MOV      R6,   #0                            @ R6 = i

sudoku_recalcular_arm_thumb_for1:
@ for (j = 0; j < NUM_FILAS; j++)
   MOV      R7,   #0                            @ R7 = j

sudoku_recalcular_arm_thumb_for2:
   MOV      R0,   R4
   MOV      R1,   R6
   ADD      R2,   R7,   #0
   ADR      R14,  sudoku_recalcular_arm_thumb_return
   ADR      R8,   sudoku_candidatos_thumb+1
   BX       R8                                  @ R0 = sudoku_candidatos_thumb(cuadricula, i, j);

sudoku_recalcular_arm_thumb_return:

   CMP		R0,   #ERROR
   MOVEQS	R5,   #ERROR						@ numCeldasVacias = -1
   SUBNE    R5,   R0							@ numCeldasVacias -= ANS

sudoku_recalcular_arm_thumb_for2_end:
   ADD      R7,   #1
   CMP      R7,   #NUM_COLUMNAS
   BLT      sudoku_recalcular_arm_thumb_for2

sudoku_recalcular_arm_thumb_for1_end:
   ADD      R6,   #1
   CMP      R6,   #NUM_FILAS
   BLT      sudoku_recalcular_arm_thumb_for1

@ return numCeldasVacias;
   MOV      R0,   R5
   LDMDB    FP,   {R4-R10, FP, SP, PC}
@ END sudoku_recalcular_arm_thumb


.thumb
   NOP
@ int sudoku_candidatos_thumb(
@ R0  CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS],
@ R1  uint8_t fila,
@ R2  uint8_t columna);
sudoku_candidatos_thumb:
   PUSH     {R4-R7, R14}                        @ Prólogo de inicio de función en Thumb.
   MOV      R4,   R0                            @ Guarda la dirección de la cuadricula.

 @ if ((cuadricula[fila][columna] & 0x0800) != 0) return TRUE;
   LSL      R3,   R1,   #5                      @ Calcula el desplazamiento de la fila.
   LSL      R5,   R2,   #1                      @ Calcula el desplazamiento de la columna.
   ADD      R3,   R0
   ADD      R5,   R3                            @ Calcula la dirección de la celda a evaluar.

   LDRH     R5,   [R5]
   MOV      R3,   #1
   LSL      R3,   R3,   #11
   AND      R3,   R5
   MOV      R0,   #1
   BNE      sudoku_candidatos_thumb_end         @ Si tiene un valor inicial, return TRUE.

   MOV      R5,   R1                            @ Si la celda estaba vacía se guardan los valores
   MOV      R6,   R2                            @ de fila y columna para las llamadas a función.

@ uint16_t candidatos = 0x0000;
   MOV      R7,   #0                            @ R7 = candidatos

@ candidatos |= calculate_fila_c(cuadricula, fila);
   MOV      R0,   R4
   MOV      R1,   R5
   BL       calculate_fila_thumb
   ORR      R7,   R0

@ candidatos |= calculate_columna_c(cuadricula, columna);
   MOV      R0,   R4
   MOV      R1,   R6
   BL       calculate_columna_thumb
   ORR      R7,   R0

@ candidatos |= calculate_region_c(cuadricula, fila, columna);
   MOV      R0,   R4
   MOV      R1,   R5
   MOV      R2,   R6
   BL       calculate_region_thumb
   ORR      R7,   R0

@ cuadricula[fila][columna] |= candidatos;
   LSL      R5,   R5,   #5
   LSL      R6,   R6,   #1
   ADD      R5,   R4
   ADD      R6,   R5

   LDRH     R5,   [R6]
   ORR      R7,   R5
   STRH     R7,   [R6]                          @ Almacena la celda actualizada.

@ if ((cuadricula[fila][columna] & 0xF000) != 0) return TRUE;
@ else return FALSE;
   MOV      R0,   #0
   LSR      R7,   R7,   #12                     @ Si la celda tiene un valor
   SUB      R7,   #0                            @ la resta da mayor que cero
   ADC      R0,   R0                            @ y suma 1 a R0, devuelve TRUE.

sudoku_candidatos_thumb_end:
   POP      {R4-R7, PC}                         @ Salta a la posición de retorno de ARM.
@ END sudoku_candidatos_thumb


@ uint16_t calculate_fila_arm(
@ R0  CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS],
@ R1  uint8_t fila);
calculate_fila_thumb:
   PUSH     {R4-R7, R14}                        @ Prólogo de inicio de función en Thumb.

   LSL      R1,   #5
   ADD      R1,   R0                            @ R1 = @cuadricula[fila][0]

   MOV      R0,   #0                            @ R0 = return
   MOV      R2,   #0                            @ R2 = j

calculate_fila_thumb_for:
@ valor = cuadricula[fila][j] >> 12;
@ if (valor > 0) candidatos |= 1 << (valor - 1);
   LDRH     R3,   [R1]
   LSR      R3,   #12
   MOV      R4,   #1
   LSL      R4,   R3
   LSR      R4,   #1
   ORR      R0,   R4

calculate_fila_thumb_for_end:
   ADD      R1,   #2
   ADD      R2,   #1
   CMP      R2,   #NUM_COLUMNAS
   BLT      calculate_fila_thumb_for

   POP      {R4-R7, PC}                         @ Epílogo con retorno a la función Thumb previa.
# END calculate_fila_thumb


@ uint16_t calculate_columna_arm(
@ R0  CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS],
@ R1  uint8_t columna);
calculate_columna_thumb:
   PUSH     {R4-R7, R14}                        @ Prólogo de inicio de función en Thumb.

   LSL      R1,   #1
   ADD      R1,   R0                            @ R1 = @cuadricula[0][columna]

   MOV      R0,   #0                            @ R0 = return
   MOV      R2,   #0                            @ R2 = i

calculate_columna_thumb_for:
@ valor = cuadricula[fila][j] >> 12;
@ if (valor > 0) candidatos |= 1 << (valor - 1);
   LDRH     R3,   [R1]
   LSR      R3,   #12
   MOV      R4,   #1
   LSL      R4,   R3
   LSR      R4,   #1
   ORR      R0,   R4

calculate_columna_thumb_for_end:
   ADD      R1,   #32
   ADD      R2,   #1
   CMP      R2,   #NUM_FILAS
   BLT      calculate_columna_thumb_for

   POP      {R4-R7, PC}                         @ Epílogo con retorno a la función Thumb previa.
# END calculate_columna_thumb


@ uint16_t calculate_region_arm(
@ R0  CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS],
@ R1  uint8_t fila);
@ R2  uint8_t columna);
calculate_region_thumb:
   PUSH     {R4-R7, R14}                        @ Prólogo de inicio de función en Thumb.

@ k = 3 * (fila / 3);                           @ K: Fila inicial de la región: 0, 3, 6.
   MOV      R3,   #0
   CMP      R1,   #3
   BLT      doneK
   MOV      R3,   #3
   CMP      R1,   #6
   BLT      doneK
   MOV      R3,   #6                            @ R3 = i
doneK:

@ l = 3 * (columna / 3);                        @ L: Columna inicial de la región: 0, 3, 6.
   MOV      R1,   #0
   CMP      R2,   #3
   BLT      doneL
   MOV      R1,   #3
   CMP      R2,   #6
   BLT      doneL
   MOV      R1,   #6                            @ R1 = j
doneL:

   LSL      R3,   #5
   LSL      R1,   #1
   ADD      R3,   R0
   ADD      R1,   R3                            @ R1 = @cuadricula[fila][columna]

   MOV      R0,   #0                            @ R0 = return

@ for (i = k; i < k+3; i++)
   MOV      R2,   #0                            @ R2 = i

calculate_region_thumb_for1:
@ for (j = l; j < l+3; j++)
   MOV      R3,   #0                            @ R3 = j

calculate_region_thumb_for2:
@ valor = cuadricula[fila][j] >> 12;
@ if (valor > 0) candidatos |= 1 << (valor - 1);
   LDRH     R4,   [R1]
   LSR      R4,   #12
   MOV      R5,   #1
   LSL      R5,   R4
   LSR      R5,   #1
   ORR      R0,   R5

calculate_region_thumb_for2_end:
   ADD      R1,   #2
   ADD      R3,   #1
   CMP      R3,   #3
   BLT      calculate_region_thumb_for2

calculate_region_thumb_for1_end:
   ADD      R1,   #26
   ADD      R2,   #1
   CMP      R2,   #3
   BLT      calculate_region_thumb_for1

   POP      {R4-R7, PC}                         @ Epílogo con retorno a la función Thumb previa.
@ END calculate_region_thumb
