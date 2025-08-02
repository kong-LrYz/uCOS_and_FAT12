;****************************************************************
;                        UCOS186A.ASM
;            80186/80188 Real Time Operating System
;                     SMALL MEMORY MODEL
;****************************************************************
            PUBLIC _OSStartHighRdy
            PUBLIC _OSCtxSw
            PUBLIC _OSIntCtxSw
            PUBLIC _OSTickISR
            EXTRN  _OSIntEnter:NEAR
            EXTRN  _OSIntExit:NEAR
            EXTRN  _OSTimeTick:NEAR
            EXTRN  _OSTCBCur:WORD
            EXTRN  _OSTCBHighRdy:WORD
.MODEL      SMALL
.CODE
.186
;****************************************************************
;                      START MULTITASKING
;                   void OSStartHighRdy(void)
;****************************************************************
_OSStartHighRdy    PROC NEAR
            MOV    BX,[_OSTCBHighRdy] ;Get highest prio. task
            MOV    [_OSTCBCur],BX
            MOV    AX,[BX]            ;Get ptr to top of stack
            MOV    SP,AX
            MOV    AX,DS              ;Stacks are in DATA segment
            MOV    SS,AX              ;Thus set SS to DS
            POP    ES
            POPA
            IRET                      ;Return to task
_OSStartHighRdy    ENDP
;****************************************************************
;            PERFORM A CONTEXT SWITCH (From task level)
;                       void OSCtxSw(void)
;****************************************************************
_OSCtxSw    PROC   FAR
            PUSHA                     ;Save current task's context
            PUSH   ES
            MOV    BX,[_OSTCBCur]     ;Save stack ptr in TCB
            MOV    [BX],SP
            MOV    BX,[_OSTCBHighRdy] ;Point to HI Prio. Task Rdy
            MOV    [_OSTCBCur],BX     ;This is now current TCB
            MOV    SP,[BX]            ;Get new task's stack ptr
            POP    ES
            POPA
            IRET                      ;Return to new task
_OSCtxSw    ENDP
;****************************************************************
;            PERFORM A CONTEXT SWITCH (From an ISR)
;                     void OSIntCtxSw(void)
;****************************************************************
_OSIntCtxSw PROC   NEAR
	    ADD    SP,10              ;Ignore calls to OSIntExit,
;                                     ;OSIntCtxSw and locals.
            MOV    BX,[_OSTCBCur]     ;Save stack ptr in TCB
            MOV    [BX],SP
            MOV    BX,[_OSTCBHighRdy] ;Point to HI Prio. Task Rdy
            MOV    [_OSTCBCur],BX     ;This is now current TCB
            MOV    SP,[BX]            ;Get new task's stack ptr
            POP    ES
            POPA
            IRET
_OSIntCtxSw ENDP
;****************************************************************
;                        HANDLE TICK ISR
;                      void OSTickISR(void)
;****************************************************************
_OSTickISR  PROC   FAR
            STI                       ;Enable interrupt nesting
            PUSHA                     ;Save current task's context
            PUSH   ES
            CALL   _OSIntEnter
            CALL   _OSTimeTick
            INT    0F2H               ;Run old tick ISR (DEMO only)
            CALL   _OSIntExit
            POP    ES                 ;Restore new task's context
            POPA
            IRET                      ;Return to interrupted task
_OSTickISR  ENDP
            END
