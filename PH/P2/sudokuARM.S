.global sudoku_recalcular_arm
.global sudoku_candidatos_arm

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
sudoku_recalcular_arm:
   MOV      IP,   SP                            @ Guarda el valor de la pila para asignar FP.
   STMDB    SP!,  {R4-R10, FP, IP, LR, PC}      @ Prólogo de inicio de función. Actualiza SP.
   SUB      FP,   IP,   #4                      @ Asigna a FP la posición inicial del marco de pila.

@ int i, j, numCeldasVacias = CELDAS_TOTALES;
   MOV      R4,   R0                            @ R4 = @cuadricula
   MOV      R5,   #NUM_TOTALES                  @ R5 = numCeldasVacias

@ for (i = 0; i < NUM_FILAS; i++)
   MOV      R6,   #0                            @ R6 = i

sudoku_recalcular_arm_for1:
@ for (j = 0; j < NUM_FILAS; j++)
   MOV      R7,   #0                            @ R7 = j

sudoku_recalcular_arm_for2:
   MOV      R0,   R4
   MOV      R1,   R6
   ADD      R2,   R7,   #0
   BL       sudoku_candidatos_arm               @ R0 = sudoku_candidatos_arm(cuadricula, i, j);

   CMP		R0,   #ERROR
   MOVEQ	R5,   #ERROR						@ numCeldasVacias = -1
   SUBNE    R5,   R0							@ numCeldasVacias -= ANS

sudoku_recalcular_arm_for2_end:
   ADD      R7,   #1
   CMP      R7,   #NUM_COLUMNAS
   BLT      sudoku_recalcular_arm_for2

sudoku_recalcular_arm_for1_end:
   ADD      R6,   #1
   CMP      R6,   #NUM_FILAS
   BLT      sudoku_recalcular_arm_for1

@ return numCeldasVacias;
   MOV      R0,   R5
   LDMDB    FP,   {R4-R10, FP, SP, PC}
@ END sudoku_recalcular_arm


@ int sudoku_candidatos_arm(
@ R0  CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS],
@ R1  uint8_t fila,
@ R2  uint8_t columna);
sudoku_candidatos_arm:
   MOV      IP,   SP                            @ Guarda el valor de la pila para asignar FP.
   STMDB    SP!,  {R4-R10, FP, IP, LR, PC}      @ Prólogo de inicio de función. Actualiza SP.
   SUB      FP,   IP,   #4                      @ Asigna a FP la posición inicial del marco de pila.


   LDRH     R9,   [R4]                          @ Descarga el valor.
   MOV		R8,	  #0
   STRH     R8,   [R4]                          @ Valor temporal = 0;

@ uint16_t candidatos = 0x0000;
   MOV      R4,   R0                            @ R4 = @cuadricula
   MOV      R5,   R1                            @ R5 = fila
   MOV      R6,   R2                            @ R6 = columna
   MOV      R7,   #0                            @ R7 = candidatos

@ candidatos |= calculate_fila_c(cuadricula, fila);
   MOV      R0,   R4
   MOV      R1,   R5
   BL       calculate_fila_arm
   ORR      R7,   R0

@ candidatos |= calculate_columna_c(cuadricula, columna);
   MOV      R0,   R4
   MOV      R1,   R6
   BL       calculate_columna_arm
   ORR      R7,   R0

@ candidatos |= calculate_region_c(cuadricula, fila, columna);
   MOV      R0,   R4
   MOV      R1,   R5
   MOV      R2,   R6
   BL       calculate_region_arm
   ORR      R7,   R0

@ cuadricula[fila][columna] |= candidatos;
   ADD      R4,   R4,   R5,   LSL #5            @ Calcula la fila.
   ADD      R4,   R4,   R6,   LSL #1            @ Calcula la columna.
   ORR      R9,   R5                            @ Actualiza el valor.
   STRH     R7,   [R4]                          @ Guarda el valor actualizado.



   MOVS		R5, R9, LSR #12						@ R5 = Valor
   BLE		not_error
@ if (candidatos && 1<<(valor-1))
   MOV		R1, #1
   SUB		R5, #1
   ANDS		R7, R1, LSL R5
@ cuadricula[fila][columna] |= ERROR_MASK;
   ORREQ	R7, #0x0400
   STREQH	R7, [R8]
   MOVEQ	R0, #-1								@ return ERROR;
   BEQ		sudoku_candidatos_arm_end

not_error:



@ if ((cuadricula[fila][columna] & 0xF000) != 0) return TRUE;
@ else return FALSE;
   MOV      R0,   #0								@ return FALSE;
   ANDS     R7,   #0xF000
   MOVNE    R0,   #1								@ return TRUE;

sudoku_candidatos_arm_end:
   LDMDB    FP,   {R4-R10, FP, SP, PC}
@ END sudoku_candidatos_arm


@ uint16_t calculate_fila_arm(
@ R0  CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS],
@ R1  uint8_t fila);
calculate_fila_arm:
   MOV      IP,   SP                            @ Guarda el valor de la pila para asignar FP.
   STMDB    SP!,  {R4-R10, FP, IP, LR, PC}      @ Prólogo de inicio de función. Actualiza SP.
   SUB      FP,   IP,   #4                      @ Asigna a FP la posición inicial del marco de pila.

   ADD      R1,   R0,   R1,   LSL #5            @ R1 = @cuadricula[fila][0]

@ uint16_t candidatos = 0x0000;
   MOV      R0,   #0

@ for (j = 0; j < NUM_FILAS; j++)
   MOV      R2,   #0                            @ R2 = j
   MOV      R3,   #1

calculate_fila_arm_for:
@ valor = cuadricula[fila][j] >> 12;
@ if (valor > 0) candidatos |= 1 << (valor - 1);
   LDRH     R4,   [R1], #2                      @ R4 = cuadricula[fila][j]
   MOV      R4,   R4,   LSR #12
   LSL      R4,   R3,   R4                      @ R4 = 1 << valor
   ORR      R0,   R0,   R4,   LSR #1            @ R0 = R0 || (1 << (valor - 1))

calculate_fila_arm_for_end:
   ADD      R2,   #1
   CMP      R2,   #NUM_COLUMNAS
   BLT      calculate_fila_arm_for

@ return candidatos;
calculate_fila_arm_end:
   LDMDB    FP,   {R4-R10, FP, SP, PC}
@ END calculate_fila_arm


@ uint16_t calculate_columna_arm(
@ R0  CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS],
@ R1  uint8_t columna);
calculate_columna_arm:
   MOV      IP,   SP                            @ Guarda el valor de la pila para asignar FP.
   STMDB    SP!,  {R4-R10, FP, IP, LR, PC}      @ Prólogo de inicio de función. Actualiza SP.
   SUB      FP,   IP,   #4                      @ Asigna a FP la posición inicial del marco de pila.

   ADD      R1,   R0,   R1,   LSL #1            @ R1 = @cuadricula[0][columna]

@ uint16_t candidatos = 0x0000;
   MOV      R0,   #0

@ for (j = 0; j < NUM_FILAS; j++)
   MOV      R2,   #0                            @ R2 = i
   MOV      R3,   #1

calculate_columna_arm_for:
@ valor = cuadricula[fila][j] >> 12;
@ if (valor > 0) candidatos |= 1 << (valor - 1);
   LDRH     R4,   [R1], #32                     @ R4 = cuadricula[i][columna]
   MOV      R4,   R4,   LSR #12
   LSL      R4,   R3,   R4                      @ R4 = 1 << valor
   ORR      R0,   R0,   R4,   LSR #1            @ R0 = R0 || (1 << (valor - 1))

calculate_columna_arm_for_end:
   ADD      R2,   #1
   CMP      R2,   #NUM_FILAS
   BLT      calculate_columna_arm_for

@ return candidatos;
calculate_columna_arm_end:
   LDMDB    FP,   {R4-R10, FP, SP, PC}
@ END calculate_columna_arm


@ uint16_t calculate_region_arm(
@ R0  CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS],
@ R1  uint8_t fila);
@ R2  uint8_t columna);
calculate_region_arm:
   MOV      IP,   SP                            @ Guarda el valor de la pila para asignar FP.
   STMDB    SP!,  {R4-R10, FP, IP, LR, PC}      @ Prólogo de inicio de función. Actualiza SP.
   SUB      FP,   IP,   #4                      @ Asigna a FP la posición inicial del marco de pila.

@ k = 3 * (fila / 3);                           @ K: Fila inicial de la región: 0, 3, 6.
   MOV      R3,   #6
   CMP      R1,   #6
   MOVLT    R3,   #3
   CMP      R1,   #3
   MOVLT    R3,   #0                            @ R3 = k

@ l = 3 * (columna / 3);                        @ L: Columna inicial de la región: 0, 3, 6.
   MOV      R4,   #6
   CMP      R2,   #6
   MOVLT    R4,   #3
   CMP      R2,   #3
   MOVLT    R4,   #0                            @ R4 = l

 @ valor = celda_leer_valor(cuadricula[i][j]);
   ADD      R1,   R0,   R3,   LSL #5
   ADD      R1,   R1,   R4,   LSL #1            @ R1 = @cuadricula[k][l]

@ uint16_t candidatos = 0x0000;
   MOV      R0,   #0

@ for (i = k; i < k+3; i++)
   MOV      R2,   #0                            @ R2 = i
   MOV      R3,   #1

calculate_region_arm_for1:
@ for (j = l; j < l+3; j++)
   MOV      R4,   #0                            @ R4 = j

calculate_region_arm_for2:
@ valor = cuadricula[i][j] >> 12;
@ if (valor > 0) candidatos |= 1 << (valor - 1);
   LDRH     R5,   [R1], #2                      @ R5 = cuadricula[i][j]
   MOV      R5,   R5,   LSR #12
   LSL      R5,   R3,   R5                      @ R5 = 1 << valor
   ORR      R0,   R0,   R5,   LSR #1            @ R0 = R0 || (1 << (valor - 1))

calculate_region_arm_for2_end:
   ADD      R4,   #1
   CMP      R4,   #3
   BLT      calculate_region_arm_for2

calculate_region_arm_for1_end:
   ADD      R1,   #26
   ADD      R2,   #1
   CMP      R2,   #3
   BLT      calculate_region_arm_for1

@ return candidatos;
calculate_region_arm_end:
   LDMDB    FP,   {R4-R10, FP, SP, PC}
@ END calculate_region_arm
