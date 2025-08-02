/*
***************************************************************
*                       INCLUDES.H
***************************************************************
*/
#include<STDIO.H>
#include<STRING.H>
#include<CTYPE.H>
#include<STDLIB.H>   
#include<CONIO.H>   
#include<DOS.H>   
#include<bios.h>
#include<direct.h>
#include<time.h>

#include"UCOS186C.H"
#include"UCOS.H"
#include"command.h"

extern void interrupt(*OldTickISR)(void);
