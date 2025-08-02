

#include"INCLUDES.H"

char* strtok_r(char *str, const char *delim, char **saveptr) {
    char *end;
	if (str == NULL) {
        str = *saveptr;
    }

    str += strspn(str, delim);
    if (*str == '\0') {
        return NULL;
    }

    end = str + strcspn(str, delim);
    if (*end == '\0') {
        *saveptr = end;
        return str;
    }

    *end = '\0';
    *saveptr = end + 1;
    return str;
}

//do command
void execute_command(char* command, char* argv) {
   if(!strcmp(command,"dir") || !strcmp(command,"DIR")) Dir(argv); 
   else if(!strcmp(command,"mkdir") || !strcmp(command,"MKDIR")) MkDir(argv);
   else if(!strcmp(command,"type") || !strcmp(command,"TYPE")) Type(argv);
   else if(!strcmp(command,"format") || !strcmp(command,"FORMAT")) Format(argv);
   else if(!strcmp(command,"rename") || !strcmp(command,"RENAME")) Rename(argv);
   else{
   		printf("\n\nBad command\n\n");
   }
}


void  Shell(char* input){

	char* command;
	char* argv;

	if(!strcmp(input,"exit")) {
		setvect(0x08, OldTickISR);
		exit(0);
	}
	//parse string
	command = strtok_r(input, " " , &argv);

	execute_command(command, argv);
			
	
}

