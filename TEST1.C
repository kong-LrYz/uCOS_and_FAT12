/*
************************************************************
*                         TEST1.C
*                         EXAMPLE
************************************************************
*/
#include "INCLUDES.H"


#define          OS_MAX_TASKS    10
#define          STK_SIZE       500
OS_TCB           OSTCBTbl[OS_MAX_TASKS];
UWORD            OSIdleTaskStk[STK_SIZE];
UWORD            Stk1[STK_SIZE];
UWORD		     KeyStk[STK_SIZE];
OS_SEM           Sem;
char 			 input[1024];


void interrupt (*OldTickISR)(void);

void far         KeyTask(void *data);

void main(void)
{
    UBYTE err;


    clrscr();
    OldTickISR = getvect(0x08);
    setvect(UCOS, (void interrupt (*)(void))OSCtxSw);
    setvect(0xF2, OldTickISR);
    OSSemInit(&Sem, 1);
    OSInit(&OSIdleTaskStk[STK_SIZE], OS_MAX_TASKS);
 //   OSTaskCreate(Shell, (void *)0, (void *)&Stk1[STK_SIZE], 1);
    OSTaskCreate(KeyTask,  (void *)0, (void *)&KeyStk[STK_SIZE],    1);

    OSStart();
}

void far KeyTask(void *data){

	int              i_Key= -1;
	char ch;

	data = data;
    setvect(0x08,(void interrupt (*)(void))OSTickISR);
	printf("\nwang@hostname$");

	while(1){

		OSTimeDly(1);
		if(kbhit()){
			OSSemPend(&Sem, 0);
			ch = getch();
			if(ch == '\b' && i_Key == -1){}
			else if(ch == '\b'){

				input[i_Key] = 0x00;
				i_Key--;
				putch('\b');
				putch(' ');
		      		putch('\b');
			}
			else{

			i_Key++;
			input[i_Key] = ch;
			putch(input[i_Key]);
                        }

			OSSemPost(&Sem);

			if(input[i_Key] == '\r') {
				input[i_Key] = '\0';
		 //		printf("%s",input);
				Shell(input);


				for(i = 0;i < 1024;i++)
					input[i] = 0x00;
				printf("\nwang@hostname$");
				i_Key= -1;
			}
		}
	}
}


