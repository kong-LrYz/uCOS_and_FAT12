#ifndef _MY_COMMAND_H
#define _MY_COMMAND_H


/*
***********************************************************
*                         Command
*                   	DECLARATIONS
***********************************************************
*/
#define MAX_LENGTH 1024


/*
***********************************************************
*         				TIME STRUCTURE
***********************************************************
*/

struct my_tm{
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
};


/*
***********************************************************
*                 	GLOBAL VARIABLES
***********************************************************
*/

extern unsigned char FAT12[4608];			/*FAT12 Table*/
extern unsigned char buffer[];			/*512 byte buffer size*/

extern int logical_block;				/*current logical block*/
extern long index;						/*current index of FAT12 Table*/
extern int flag;							/*whether research success, 1:success , 2:fail*/
extern int sector,head,track;
extern char* token;
extern unsigned char tmp1;					/*floppy's number*/
extern int i,j;




/*
***********************************************************
*                 FUNCTION PROTOTYPES
***********************************************************
*/
void Determine_floppy_number(void);
void LBA_to_CHS(int logical_block,int* sector,int* head,int* track);


void ERR_Printf1(const char * path);
void ERR_Printf2(const char * path);
void myPrintf_dir(void);
void myPrintf_type(void);

/*dir*/
int FindInFAT12(unsigned short index);
int Research(void);


void Clear_buffer(void);
void Fill_0XF6_buffer(void);

/*format*/
void Initialize_ZeroBlk(void);
void Initialize_FAT12(void);
void Initialize_BlkOfData(void);
void Initialize_RootFCB(void);
/*mkdir*/
void Load_FCB(void);
void Make_directory(void);


/*Shell*/
void execute_command(char* command, char* args);
void  Shell(char* input);
char* strtok_r(char *str, const char *delim, char **saveptr);

void Dir(char* argv);
void MkDir(char* argv);
void Type(char* argv);
void Format(char* argv);
void Rename(char* argv);
#endif
