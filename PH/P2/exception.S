.global MyHandler
.global DoAbort
.global DoUndef
.global DoSWI

.equ  FIQ_MASK,   0x11        @ FIQ mask
.equ  IRQ_MASK,   0x12        @ IRQ mask
.equ  SVC_MASK,   0x13        @ SWI mask
.equ  ABT_MASK,   0x17        @ Data Abort mask and Prefetch Abort mask
.equ  UND_MASK,   0x1B        @ Undefined Instruction mask
.equ  ALL_MASK,   0x1F        @ Mascara para capturar los modos del procesador.


DoAbort:
   LDR      R0,      =0x0A333333
   STR      R0,      [R0]
   MOV      PC,      LR

DoUndef:
   .word    0xE6000010
   MOV      PC,      LR

DoSWI:
   SWI      1
   MOV      PC,      LR


MyHandler:
   STMFD    SP!,     {R0-R12}          @ Guarda todos los registros del estado erróneo.
   MRS      R0,      CPSR              @ Analiza la excepción llamadora.
   AND      R0,      #ALL_MASK

   CMP      R0,      #ABT_MASK
   BEQ      MyHandlerDAbort

   CMP      R0,      #FIQ_MASK
   BEQ      MyHandlerFiq

   CMP      R0,      #IRQ_MASK
   BEQ      MyHandlerIrq

   CMP      R0,      #ABT_MASK
   BEQ      MyHandlerPAbort

   CMP      R0,      #UND_MASK
   BEQ      MyHandlerUndef

   CMP      R0,      #SVC_MASK
   BEQ      MyHandlerSwi


.extern push_debug
@.extern D8Led_symbol
MyHandlerFiq:
   MOV      R0,      #FIQ_MASK
   MOV      R1,      R14
   BL       push_debug

   MOV      R0,      #0xB
   @BL       D8Led_symbol
   B        .

@  Return
   LDMIA    SP!,     {R0-R12}          @ Recupera todos los registros del estado erróneo.
   SUBS     PC,      R14,     #4

MyHandlerIrq:
   MOV      R0,      #IRQ_MASK
   MOV      R1,      R14
   BL       push_debug

   MOV      R0,      #0xB
   @BL       D8Led_symbol
   B        .

@  Return
   LDMIA    SP!,     {R0-R12}          @ Recupera todos los registros del estado erróneo.
   SUBS     PC,      R14,     #4

MyHandlerSwi:
   MOV      R0,      #SVC_MASK
   MOV      R1,      R14
   BL       push_debug

   MOV      R0,      #0xE
   @BL       D8Led_symbol
   B        .

@  Return
   LDMIA    SP!,     {R0-R12}          @ Recupera todos los registros del estado erróneo.
   MOVS     PC,      R14

MyHandlerDAbort:
   MOV      R0,      #ABT_MASK
   MOV      R1,      R14
   BL       push_debug

   MOV      R0,      #0xA
   @BL       D8Led_symbol
   B        .

@  Return
   LDMIA    SP!,     {R0-R12}          @ Recupera todos los registros del estado erróneo.
   SUBS     PC,      R14,     #8

MyHandlerPAbort:
   MOV      R0,      #ABT_MASK
   MOV      R1,      R14
   BL       push_debug

   MOV      R0,      #0xA
   @BL       D8Led_symbol
   B        .

@  Return
   LDMIA    SP!,     {R0-R12}          @ Recupera todos los registros del estado erróneo.
   SUBS     PC,      R14,     #4

MyHandlerUndef:
   MOV      R0,      #UND_MASK
   MOV      R1,      R14
   BL       push_debug

   MOV      R0,      #0xD
   @BL       D8Led_symbol
   B        .

@  Return
   LDMIA    SP!,     {R0-R12}          @ Recupera todos los registros del estado erróneo.
   MOVS     PC,      R14

.end
