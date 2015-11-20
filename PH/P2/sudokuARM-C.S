.global sudoku_recalcular_arm_c

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
@ int sudoku_recalcular_arm_c(
@ R0  CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS]);
sudoku_recalcular_arm_c:
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
   BL       sudoku_candidatos_c                 @ R0 = sudoku_candidatos_arm(cuadricula, i, j);
   CMP		R0,   #ERROR
   MOVEQS	R5,   #ERROR						@ numCeldasVacias = -1
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
