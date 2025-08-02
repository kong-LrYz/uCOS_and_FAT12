/*
***********************************************************
*                      UCOS186C.C
*               80186/80188 Specific Code
*                  SMALL MEMORY MODEL
***********************************************************
*                     INCLUDE FILES
***********************************************************
*/
#include "INCLUDES.H"
#pragma  inline

extern UBYTE    OSMapTbl[];
extern UBYTE    OSUnMapTbl[];

/*
***********************************************************
*                    CREATE A TASK
***********************************************************
*/

UBYTE OSTaskCreate(void (far *task)(void *dptr), void  *data, void  *pstk, UBYTE  p)
{
    OS_TCB *ptr;
    UWORD  *stk;

	ptr              = OSTCBGetFree();
	
	ptr->OSTCBID = x_id++;
	
    ptr->OSTCBPrio   = (UBYTE)p;
    ptr->OSTCBStat   = OS_STAT_RDY;
    ptr->OSTCBDly    = 0;

    stk              = (UWORD *)pstk;                       /* 80186/80188 Small Model              */
    *--stk           = (UWORD)FP_OFF(data);
    *--stk           = (UWORD)FP_SEG(task);
    *--stk           = (UWORD)FP_OFF(task);
    *--stk           = (UWORD)0x0200;                       /* PSW = Int. En.                       */
    *--stk           = (UWORD)FP_SEG(task);
    *--stk           = (UWORD)FP_OFF(task);
    *--stk           = (UWORD)0x0000;                       /* AX = 0                               */
    *--stk           = (UWORD)0x0000;                       /* CX = 0                               */
    *--stk           = (UWORD)0x0000;                       /* DX = 0                               */
    *--stk           = (UWORD)0x0000;                       /* BX = 0                               */
    *--stk           = (UWORD)0x0000;                       /* SP = 0                               */
    *--stk           = (UWORD)0x0000;                       /* BP = 0                               */
    *--stk           = (UWORD)0x0000;                       /* SI = 0                               */
    *--stk           = (UWORD)0x0000;                       /* DI = 0                               */
    *--stk           = (UWORD)0x0000;                       /* ES = 0                               */
    ptr->OSTCBStkPtr = (void *)stk;                         /* Load SP in TCB						*/
	
	OS_ENTER_CRITICAL(); 
	
    ptr->OSTCBPrioPtr = OSTCBPrioTbl[p];
    OSTCBPrioTbl[p] = ptr;
    
	
	ptr->OSTCBNext        = OSTCBList;
    ptr->OSTCBPrev        = (OS_TCB *)0;
    if (OSTCBList != (OS_TCB *)0) {                         /* Rev. A, This line was missing        */
        OSTCBList->OSTCBPrev = ptr;
    }
	
	
	
	OSTCBList             = ptr;
    OSRdyGrp             |= OSMapTbl[p >> 3];
    OSRdyTbl[p >> 3]     |= OSMapTbl[p & 0x07];
    OS_EXIT_CRITICAL();


    
    if (OSRunning) {
        OSSched();
    }
    return (OS_NO_ERR);
}
