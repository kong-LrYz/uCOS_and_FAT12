
#include"INCLUDES.H"
/*
************************************************************
*                	GLOBAL VARIABLES
************************************************************
*/
unsigned char FAT12[4608];			/*FAT12 Table*/
unsigned char buffer[512];			/*512 byte buffer size*/

int logical_block = 19;				/*current logical block*/
long index;							/*current index of FAT12 Table*/

int prev_logical_block;				/*2023.12.6:previous logical block,special for the rename command*/


int sector,head,track;
char* token;
unsigned char tmp1;					/*floppy's number*/

int i,j;





/*
************************************************************
*                         FUNCTIONS
************************************************************
*/


/**
*	the function is tansform the logical block to CHS
*/
void LBA_to_CHS(int logical_block,int* sector,int* head,int* track){
	*sector = logical_block % 18 + 1;
	logical_block /= 18;
	*head = logical_block % 2;
	*track = logical_block / 2;
	return;
}

/**
*	the function'goal is determine the floppy is A: or B: or C: etc.... 
*/
void Determine_floppy_number(void){
	if(token[0] == 'A'){
		tmp1 = 0x00;
	}else if(token[0] == 'B'){
		tmp1 = 0x01;
	}else if(token[0] == 'C'){
		tmp1 = 0x80;
	}else if(token[0] == 'D'){
		tmp1 = 0x81;
	}else if(token[0] == 'E'){
		tmp1 = 0x82;
	}else{
		printf("Invalid drive specification.");
    	return;
	}
}


/**
*	the function will operate in FAT12.
*/
int FindInFAT12(unsigned short index){
	long next_index = 0;

	next_index = (index / 2) * 3;

	if(index % 2 == 0){
		next_index = (((long)FAT12[next_index + 2] << 16) | ((long)FAT12[next_index + 1] << 8) | (long)FAT12[next_index]) & 0xFFF;
	}else{
		next_index = (((long)FAT12[next_index + 2] << 16) | ((long)FAT12[next_index + 1] << 8) | (long)FAT12[next_index]) & 0xFFF000;
		next_index >>= 12;
	}
	
	if(next_index == 4095)
		next_index = -1;

	return next_index;
}


/**
*	the function has complicate logic.
*	In brief,it will operate around FCB(32 bytes).
*/
int Research(void){

	int flag = 0;						/*whether research success, 1:success , 2:fail*/
	int length = 0; 
	
	while(token != NULL){

		flag = 0;



		if(logical_block == 19){

			for(i = 0;i < 14;i++){
				LBA_to_CHS(logical_block + i,&sector,&head,&track);
				if(biosdisk(2,tmp1,head,track,sector,1,buffer)){
					printf("can't use the %dth block\n",logical_block);
					delay(1500);
					exit(0);
				}

				for(j = 0;j < 16;j++){
					
					length = 0;
					while(buffer[32 * j + length] != 0x20 && length <= 7){
						length++;
					}

					if(length == strlen(token) && (buffer[j * 32 + 11] & 0x10) && !strncmp(buffer + 32 * j, token,length)){
						index = (unsigned short)((buffer[j * 32 + 27] << 8) | buffer[j * 32 + 26]);
						logical_block = 33 + index - 2;
						token = strtok(NULL,"\\");
						flag = 1;
						break;
					}
				}

				if(flag == 1)
					break;
			}
			if(flag == 0){
				return 0;
			}
			continue;
		}

		do{
			LBA_to_CHS(logical_block,&sector,&head,&track);
			if(biosdisk(2,tmp1,head,track,sector,1,buffer)){
				printf("can't use the %dth block\n",logical_block);
				delay(1500);
				exit(0);
			}

			for(j = 0; j < 16;j++){

				length = 0;
				while(buffer[32 * j + length] != 0x20 && length <= 7){
					length++;
				}

				if(length == strlen(token) && (buffer[j * 32 + 11] & 0x10) && !strncmp(buffer + j * 32, token,length)){

					index = (unsigned short)((buffer[j * 32 + 27] << 8) | buffer[j * 32 + 26]);
					logical_block = 33 + index - 2;
					token = strtok(NULL,"\\");
					flag = 1;
					break;
				}
			}
			if(flag == 1) break;
			else index = FindInFAT12(index);
			if(index != -1)
				logical_block = 33 + index - 2;
			printf("%d\n",logical_block);
		}while(index != -1);

	       if(flag == 0){
				return 0;
	       }

	}

	return 1;
}


/**
*	special research function that can research all type document,not only folder. 
*/
int ResearchAll(void){

	int flag = 0;						/*whether research success, 1:success , 2:fail*/
	int length = 0; 
	
	
	while(token != NULL){

		flag = 0;



		if(logical_block == 19){

			for(i = 0;i < 14;i++){
				LBA_to_CHS(logical_block + i,&sector,&head,&track);
				if(biosdisk(2,tmp1,head,track,sector,1,buffer)){
					printf("can't use the %dth block",logical_block);
					exit(0);
				}

				for(j = 0;j < 16;j++){
					
					length = 0;
					while(buffer[32 * j + length] != 0x20 && length <= 7){
						length++;
					}

					if(length == strlen(token) && (buffer[j * 32 + 11] & 0x30) && !strncmp(buffer + 32 * j, token,length)){
						
						prev_logical_block = logical_block + i;		/*the rename command will use*/
						
						index = (unsigned short)((buffer[j * 32 + 27] << 8) | buffer[j * 32 + 26]);
						logical_block = 33 + index - 2;
						token = strtok(NULL,"\\");
						flag = 1;
						break;
					}
				}

				if(flag == 1)
					break;
			}
			if(flag == 0){
				return 0;
			}
			continue;
		}

		do{
			LBA_to_CHS(logical_block,&sector,&head,&track);
			if(biosdisk(2,tmp1,head,track,sector,1,buffer)){
				printf("can't use the %dth block",logical_block);
				exit(0);
			}

			for(j = 0; j < 16;j++){

				length = 0;
				while(buffer[32 * j + length] != 0x20 && length <= 7){
					length++;
				}

				if(length == strlen(token) && (buffer[j * 32 + 11] & 0x30) && !strncmp(buffer + j * 32, token,length)){

					prev_logical_block = logical_block;   /*the rename command will use*/

					index = (unsigned short)((buffer[j * 32 + 27] << 8) | buffer[j * 32 + 26]);
					logical_block = 33 + index - 2;
					token = strtok(NULL,"\\");
					flag = 1;
					break;
				}
			}
			if(flag == 1) break;
			else index = FindInFAT12(index);
			if(index != -1)
				logical_block = 33 + index - 2;
			printf("%d\n",logical_block);
		}while(index != -1);

	       if(flag == 0){
				return 0;
	       }

	}

	return 1;
}


/**
*load a FCB (32 bytes)
*/
void Load_FCB(void){

	struct my_tm *p;
	time_t timep;
	unsigned short time,date;
	int xi = 0;

	p =(struct my_tm *) gmtime(&timep);


	memset(buffer + j * 32,0x20,11);
	time = ((p->tm_hour << 11) | (p->tm_min << 4) | (p->tm_sec >> 1));
	date = (((p->tm_year - 1980) << 9) | (p->tm_mon << 5) | p->tm_mday);


	strncpy(buffer + j * 32,token,strlen(token));

	buffer[j * 32 + 11] = 0x10;

	buffer[j * 32 + 22] = (unsigned char)(time >> 8);
	buffer[j * 32 + 23] = (unsigned char)(time & 0x0ff);

	buffer[j * 32 + 24] = (unsigned char)(date >> 8);
	buffer[j * 32 + 25] = (unsigned char)(date & 0x0ff);


	/*find a empty FAT12 item*/
	while(xi < 4608){

		if(!((unsigned short)(FAT12[xi] | (FAT12[xi + 1] & 0x0f << 4)))){

			FAT12[xi] = 0xFF;
			FAT12[xi + 1] |= 0x0F;
			buffer[j * 32 + 26] = (unsigned char) ((xi / 3 * 2) & 0x0ff);
			buffer[j * 32 + 27] = (unsigned char) ((xi / 3 * 2) >> 8);
			break;
		}

		if (!((unsigned short)((FAT12[xi + 1] >> 4) | (FAT12[xi + 2] << 8)))){
			FAT12[xi + 1] |= 0xF0;
			FAT12[xi + 2] = 0xFF;
			buffer[j * 32 + 26] = (unsigned char) ((xi / 3 * 2 + 1) & 0x0ff);
			buffer[j * 32 + 27] = (unsigned char) ((xi / 3 * 2 + 1) >> 8);
			break;
		}
		xi += 3;
	}



	buffer[j * 32 + 28] = 0x00;
	buffer[j * 32 + 29] = 0x00;
	buffer[j * 32 + 30] = 0x00;
	buffer[j * 32 + 31] = 0x00;
	return;
}



/**
*	create the perious directory.
*	such as: a:/try1/try2/try3	a:/try1 is existing,so try2 will be create under the
*	directory of try1.And then try3 will be create under the directory of try2 etc.... 
*/
void Make_directory(void){

	int pre_index = 0;			/*the previous FAT12 Table index*/
	int xi = 0;

	while(token != NULL){


		if(logical_block == 19){
			for(i = 0;i < 14;i++){
				LBA_to_CHS(logical_block + i,&sector,&head,&track);
				if(biosdisk(2,tmp1,head,track,sector,1,buffer)){
					printf("can't use the %dth block",logical_block);
					delay(1500);
					exit(0); 
				}

				j = 0;
				while(buffer[32 * j] != 0x00 && j < 16) j++;
				if(j < 16){
					Load_FCB();			/*load a FCB (32 bytes)*/
					token = strtok(NULL,"\\");

					LBA_to_CHS(logical_block + i,&sector,&head,&track);		/*write back to floppy*/
					if(biosdisk(3,tmp1,head,track,sector,1,buffer)){
						printf("The action of writing back is wrong");
						delay(1500);
						exit(0);
					}

					index = (unsigned short)((buffer[j * 32 + 27] << 8) | buffer[j * 32 + 26]);		/*the new index*/
					logical_block = 33 + index - 2;													/*the new logical_block*/
					goto destination;
				}
			}
			printf("root directory area not have enough area!!!!!!!");
			delay(1500);
			exit(0);

		}

		else{
			LBA_to_CHS(logical_block,&sector,&head,&track);
			if(biosdisk(2,tmp1,head,track,sector,1,buffer)){
				printf("can't use the %dth block",logical_block);
				delay(1500);
				exit(0);
			}

			for(j = 0; j < 16;j++){
				if(buffer[32 * j] == 0x00 || buffer[32 * j] == 0xf6)
					break;
			}
			if(j == 0)   memset(buffer,0x00,512);
			/*if this sector is full,apply in FAT12 to a new sector*/
			if(j == 16){
				memset(buffer,0x00,512);
				while(index != -1){
					pre_index = index;
					index = FindInFAT12(index);
				}


				/*find a empty FAT12 item*/
				while(xi < 4608){
					if(!((unsigned short)(FAT12[xi] | (FAT12[xi + 1] & 0x0f << 4)))){

						if(pre_index % 2 == 0){
							FAT12[(pre_index / 2) * 3] = (unsigned char)(0x0ff & ((xi / 3) * 2));
							FAT12[(pre_index / 2) * 3 + 1] = (unsigned char)(((xi / 3) * 2) >> 8);
						}else{
							FAT12[(pre_index / 2) * 3 + 1] = (unsigned char)(0x00f & ((xi / 3) * 2) << 4);
							FAT12[(pre_index / 2) * 3 + 2] = (unsigned char)(((xi / 3) * 2) >> 4);
						}

						FAT12[xi] = 0xFF;
						FAT12[xi + 1] |= 0x0F;

						logical_block = (xi / 3) * 2 + 33 - 2;
						break;
					}

					if (!((unsigned short)((FAT12[xi + 1] & 0xf0) | (FAT12[xi + 2] << 8)))){

						if(pre_index % 2 == 0){
							FAT12[(pre_index / 2) * 3] = (unsigned char)(0x0ff & ((xi / 3) * 2));
							FAT12[(pre_index / 2) * 3 + 1] = (unsigned char)(((xi / 3) * 2) >> 8);
						}else{
							FAT12[(pre_index / 2) * 3 + 1] = (unsigned char)(0x00f & ((xi / 3) * 2) << 4);
							FAT12[(pre_index / 2) * 3 + 2] = (unsigned char)(((xi / 3) * 2) >> 4);
						}


						FAT12[xi + 1] |= 0xF0;
						FAT12[xi + 2] = 0xFF;

						logical_block = (xi / 3) * 2 + 1 + 33 - 2;
						break;
					}
					xi += 3;
				}

				j = 0;
			}

			Load_FCB();						/*load a FCB (32 bytes)*/
			token = strtok(NULL,"\\");

			LBA_to_CHS(logical_block,&sector,&head,&track);		/*write back to floppy*/
			if(biosdisk(3,tmp1,head,track,sector,1,buffer)){
				printf("The action of writing back is wrong");
				delay(1500);
				exit(0);
			}

			index = (unsigned short)((buffer[j * 32 + 27] << 8) | buffer[j * 32 + 26]);		/*the new index*/
			logical_block = 33 + index - 2;													/*the new logical_block*/
		}
		destination:
	}
}



/**
*	initialize the first sector
*/
void Initialize_ZeroBlk(void){
	logical_block = 0;
	
	Clear_buffer();
	buffer[0] = 0xEB;
	buffer[1] = 0x3C;
	buffer[2] = 0x90;

	buffer[3] = 0x4D;
	buffer[4] = 0x53;
	buffer[5] = 0x44;
	buffer[6] = 0x4F;
	buffer[7] = 0x53;

	buffer[11] = 0x00;
	buffer[12] = 0x02;

	buffer[13] = 0x01;

	buffer[14] = 0x01;
	buffer[15] = 0x00;

	buffer[16] = 0x02;

	buffer[17] = 0xE0;
	buffer[18] = 0x00;

	buffer[19] = 0x40;
	buffer[20] = 0x0E;

	buffer[21] = 0xF0;

	buffer[22] = 0x09;
	buffer[23] = 0x00;

	buffer[24] = 0x12;
	buffer[25] = 0x00;

	buffer[26] = 0x02;
	buffer[27] = 0x00;

	buffer[38] = 0x29;

	buffer[39] = 0x14;
	buffer[40] = 0xE6;
	buffer[41] = 0x63;
	buffer[42] = 0xE0;

	buffer[43] = 0x4E;
	buffer[44] = 0x4F;
	buffer[45] = 0x20;
	buffer[46] = 0x4E;
	buffer[47] = 0x41;
	buffer[48] = 0x4D;
	buffer[49] = 0x45;
	buffer[50] = 0x20;
	buffer[51] = 0x20;
	buffer[52] = 0x20;
	buffer[53] = 0x20;

	buffer[54] = 0x46;
	buffer[55] = 0x41;
	buffer[56] = 0x54;
	buffer[57] = 0x31;
	buffer[58] = 0x32;

	buffer[510] = 0x055;
	buffer[511] = 0x0AA;
	
	LBA_to_CHS(logical_block,&sector,&head,&track);
	if(biosdisk(3,tmp1,head,track,sector,1,buffer)){
		printf("something is wrong:can't write the %dth block",logical_block);
		delay(1500);
		exit(0);
	}
	
	return;
}




/**
*	initialize the FAT12 Table
*/
void Initialize_FAT12(void){
	logical_block = 1;
	
	Clear_buffer();
	buffer[0] = 0xF0;
	buffer[1] = 0xFF;
	buffer[2] = 0xFF;
	
	LBA_to_CHS(logical_block,&sector,&head,&track);
	if(biosdisk(3,tmp1,head,track,sector,1,buffer)){
		printf("something is wrong:can't write the %dth block",logical_block);
		exit(0);
	}
	Clear_buffer();
	for(logical_block = 2;logical_block < 10;logical_block++){
		LBA_to_CHS(logical_block,&sector,&head,&track);
		if(biosdisk(3,tmp1,head,track,sector,1,buffer)){
			printf("something is wrong:can't write the %dth block",logical_block);
			exit(0);
		}
	}


	logical_block = 10;

	buffer[0] = 0xF0;
	buffer[1] = 0xFF;
	buffer[2] = 0xFF;
	
	LBA_to_CHS(logical_block,&sector,&head,&track);
	if(biosdisk(3,tmp1,head,track,sector,1,buffer)){
		printf("something is wrong:can't write the %dth block",logical_block);
		exit(0);
	}

	Clear_buffer();
	for(logical_block = 11;logical_block < 19;logical_block++){
		LBA_to_CHS(logical_block,&sector,&head,&track);
		if(biosdisk(3,tmp1,head,track,sector,1,buffer)){
			printf("something is wrong:can't write the %dth block",logical_block);
			exit(0);
		}
	}

	
	return;
}


/**
*	initialize the root of FCB
*/
void Initialize_RootFCB(void){
	Clear_buffer();
	for(logical_block = 19; logical_block < 33;logical_block++){
		LBA_to_CHS(logical_block,&sector,&head,&track);
		if(biosdisk(3,tmp1,head,track,sector,1,buffer)){
			printf("something is wrong:can't write the %dth block",logical_block);
			delay(1500);
			exit(0);
		}
	}
}

/**
*	Initialize the block of data
*/
void Initialize_BlkOfData(void){
	Fill_0XF6_buffer();
	
	for(logical_block = 33; logical_block < 2880 ; logical_block ++){
		LBA_to_CHS(logical_block,&sector,&head,&track);
		if(biosdisk(3,tmp1,head,track,sector,1,buffer)){
			printf("something is wrong:can't write the %dth block",logical_block);
			delay(1500);
			exit(0);
		}
	}
	
	return;
}



/**
*	clear the buffer(512 bytes)
*/
void Clear_buffer(void){
	int xi;
	for(xi = 0; xi < 512; xi++)
		buffer[xi] = 0x00;
	return;
}

/**
*	Fill the 0XF6 in buffer,indicate the area is data?
*/
void Fill_0XF6_buffer(void){
	int xi; 
	for(xi = 0; xi < 512; xi++)
		buffer[xi] = 0xF6;
	return;
}


/**
*	print the err(the precious directory is not exist).
*/
void ERR_Printf1(const char * path){

	textcolor(RED);

	printf("dir : can't find directory %s ,because the directory is missing\n	\
the Location Line:1 character: 1\n	\
+ dir %s\n \
+ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n \
\t+ CategoryInfo          : ObjectNotFound:%s:String) [Get-ChildItem], ItemNotFoundException \n \
\t+ FullyQualifiedErrorId : PathNotFound,Microsoft.Windows_xp.Commands.GetChildItemCommand  \n \
\n",path,path,path);

	textcolor(LIGHTGRAY);

	return;
}



/**
*	print err(beacuse the directory what to create is existing). 
*/
void ERR_Printf2(const char * path){

	textcolor(RED);

	printf("\n\nmkdir : has %s 's item exists\n	\
the Location Line:1 character: 1\n	\
+ mkdir %s\n \
+ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n \
\t+ CategoryInfo          : ResourceExists :(%s:String) [New-Item], IOException \n \
\t+ FullyQualifiedErrorId : DirectoryExists,Microsoft.Windows_xp.Commands.NewItemCommand  \n \
\n",path,path,path);

	textcolor(LIGHTGRAY);

	return;
}


/**
*	print current directory includes what.
*/
void myPrintf_dir(void){

	unsigned short lwtime;			// File last write time, 2 bytes
	unsigned short lwdate;			// File last write date, 2 bytes
	unsigned long flength=0;	// File size, 4 bytes
	int xi;
	for(xi = 0; xi < 16;xi++){
		if(!(buffer[32 * xi + 11] & 0x30))
			continue;
		if (buffer[32 * xi] == 0xE5)
            continue;



		lwtime = (unsigned short)((buffer[xi * 32 + 0x17] << 8) | buffer[xi * 32 + 0x16] );
		lwdate = (unsigned short)((buffer[xi * 32 + 0x19] << 8) | buffer[xi * 32 + 0x18] );
		flength = (unsigned long)((buffer[xi * 32 + 0x1f] << 24) | (buffer[xi * 32 + 0x1e] << 16) | (buffer[xi * 32 + 0x1d] << 8) | buffer[xi * 32 + 0x1c]);
		if(buffer[xi * 32 + 11] & 0x20)	// if normal file
			printf("\na-----");
		else if(buffer[xi * 32 + 11] & 0x10)	// if directory
			printf("\nd-----");



			printf("\t\t%hu-%hu-%hu", ((lwdate & 0xfe00) >> 9) + 1980, ((lwdate & 0x01e0) >> 5), ((lwdate & 0x001f)));		// File lastwrite date
			printf(" %hu:%hu:%hu", ((lwtime & 0xf800) >> 11), ((lwtime & 0x07e0) >> 5), ((lwtime & 0x001f) << 1));		// File lastwrite time
		if(buffer[xi * 32 + 11] & 0x20)
			printf("\t%d", flength);
		if(buffer[xi * 32 + 11] & 0x20)
			printf("  %.*s",8,buffer + 32 * xi);
		else
			printf("\t\t%.*s",8,buffer + 32 * xi);

		if(buffer[32 * i + 8] != 0x20)
			printf(".%.*s",3,buffer + 32 * xi + 8);
	}

}


/**
*	print a txt etc........
*/
void myPrintf_type(void){
	
	int xi = 0;
	while(buffer[xi] != 0x00)
		printf("%c",buffer[xi++]);
}



/**
*	the command function of Dir
*/
void Dir(char* argv){
	int flag = 0;						/*whether research success, 1:success , 2:fail*/
	char precious_path[1024];
	strcpy(precious_path,argv);
	
	/*initialize logical_block and index*/
	logical_block = 19;
	index = 0;
	
	
	for (i = 0; precious_path[i]; i++) {
        precious_path[i] = toupper(precious_path[i]);
    }


	token = strtok(precious_path,"\\");


	Determine_floppy_number();

	/*test the disk is truth*/
	if(biosdisk(2,tmp1,0,0,1,1,buffer)){
		printf("can't use this floppy");
		delay(1500);
		exit(0);
	}

	/*load FAT12 Table*/
	LBA_to_CHS(1,&sector,&head,&track);
	if(biosdisk(2,tmp1,head,track,sector,9,FAT12)){
		printf("can't load the FAT Table");
		delay(1500);
		exit(0);
	}

	token = strtok(NULL,"\\");


	flag = Research();

	if(flag == 0){
		ERR_Printf1(argv);
		return;
	}

	/*show*/
	if(logical_block == 19){

	printf("\n\n \
\tdirectory:%s \
\n\n\
Mode\t\tLastWriteTime\t\tLength Name\n\
----\t\t-------------\t\t------ ----\
\n",argv);

		for(i = 0;i < 14;i++){
			LBA_to_CHS(logical_block + i,&sector,&head,&track);
			if(biosdisk(2,tmp1,head,track,sector,1,buffer)){
				printf("can't use the %dth block",logical_block);
				delay(1500);
				exit(0);
			}

		       myPrintf_dir();

		}
		return;
	}



	printf("\n\n \
\tdirectory:%s \
\n\n\
Mode\t\tLastWriteTime\t\tLength Name\n\
----\t\t-------------\t\t------ ----\
\n",argv);


	do{
		LBA_to_CHS(logical_block,&sector,&head,&track);
		if(biosdisk(2,tmp1,head,track,sector,1,buffer)){
			printf("can't use the %dth block",logical_block);
			delay(1500);
			exit(0);
		}

		myPrintf_dir();
		index = FindInFAT12(index);

		if(index != -1)
			logical_block = 33 + index - 2;

		}while(index != -1);

	return;
}



/**
*	the command function of MkDir
*/
void MkDir(char* argv){
	int flag = 0;						/*whether research success, 1:success , 2:fail*/
	char precious_path[1024];
	strcpy(precious_path,argv);
	
	/*initialize logical_block and index*/
	logical_block = 19;
	index = 0;
	
	
	for (i = 0; precious_path[i]; i++) {
	precious_path[i] = toupper(precious_path[i]);
    }

    token = strtok(precious_path,"\\");

    Determine_floppy_number();

	/*test the disk is truth*/
	if(biosdisk(2,tmp1,0,0,1,1,buffer)){
		printf("can't use this floppy");
		delay(1500);
		exit(0);
	}

	/*load FAT12 Table*/
	LBA_to_CHS(1,&sector,&head,&track);
	if(biosdisk(2,tmp1,head,track,sector,9,FAT12)){
		printf("can't load the FAT Table");
		delay(1500);
		exit(0);
	}

	token = strtok(NULL,"\\");

	flag = Research();

	/*the wanted to created directory has existed*/
	if(flag == 1) {
		ERR_Printf2(argv);
		return;
	}

	/*create directory*/

	Make_directory();


	/*finish the command of mymkdir,write back the FAT12 Table*/
	LBA_to_CHS(1,&sector,&head,&track);
	if(biosdisk(3,tmp1,head,track,sector,9,FAT12)){
		printf("something is wrong:can't write the %dth block",logical_block);
		delay(1500);
		exit(0);
	}

	/*finish the command of mymkdir,write back the FAT12 Table's backup*/
	LBA_to_CHS(10,&sector,&head,&track);
	if(biosdisk(3,tmp1,head,track,sector,9,FAT12)){
		printf("something is wrong:can't write the %dth block",logical_block);
		delay(1500);
		exit(0);
	}

	return;
}


/**
* the command of Type
*/
void Type(char* argv){
	int flag = 0;						/*whether research success, 1:success , 2:fail*/
	char precious_path[1024];

	/*initialize logical_block and index*/
	logical_block = 19;
	index = 0;

	/*delete such as .txt .exe etc*/
	for(i = 0; argv[i] ; i++){
		if(argv[i] == '.')
			break;
	}


	strncpy(precious_path,argv,i);
	precious_path[i] = '\0';
//	printf("%s", precious_path);

	for (i = 0; precious_path[i]; i++) {
		precious_path[i] = toupper(precious_path[i]);
    }

	token = strtok(precious_path,"\\");


	Determine_floppy_number();


	/*test the disk is truth*/
	if(biosdisk(2,tmp1,0,0,1,1,buffer)){
		printf("can't use this floppy");
		return;
	}

	/*load FAT12 Table*/
	LBA_to_CHS(1,&sector,&head,&track);
	if(biosdisk(2,tmp1,head,track,sector,9,FAT12)){
		printf("can't load the FAT Table");
		return;
	}

	token = strtok(NULL,"\\");


	flag = ResearchAll();

	if(flag == 0){
		ERR_Printf1(argv);
		return;
	}

	/*show*/
	do{
		LBA_to_CHS(logical_block,&sector,&head,&track);
		if(biosdisk(2,tmp1,head,track,sector,1,buffer)){
			printf("can't use the %dth block",logical_block);
			return;
		}

		myPrintf_type();
		index = FindInFAT12(index);

		if(index != -1)
			logical_block = 33 + index - 2;

	}while(index != -1);

	return;
}


/**
* the command of Rename
*/
void Rename(char* argv){
	
	
	int flag = 0;						/*whether research success, 1:success , 2:fail*/
	char precious_path[1024];
	char* temp1;
	char* want_name;	
	
	
	
	strcpy(precious_path,argv);
	want_name = precious_path;

	temp1 = strtok_r(precious_path," ",&want_name); 

	for (i = 0; temp1[i]; i++) {
        temp1[i] = toupper(temp1[i]);
    }
	for(i = 0;want_name[i];i++){
	want_name[i] = toupper(want_name[i]);
    }    
    
    
    token = strtok(temp1,"\\");
    
    
    Determine_floppy_number();
    
    /*test the disk is truth*/
	if(biosdisk(2,tmp1,0,0,1,1,buffer)){
		printf("can't use this floppy");
		return;
	}

	/*load FAT12 Table*/
	LBA_to_CHS(1,&sector,&head,&track);
	if(biosdisk(2,tmp1,head,track,sector,9,FAT12)){
		printf("can't load the FAT Table");
		return;
	}
	
	token = strtok(NULL,"\\");
	
		
	flag = ResearchAll();
	
	if(flag == 0){
		ERR_Printf1(argv);
		exit(0);
	}

	
	
	
	/*rename*/
	memset(buffer + j * 32,0x20,8);
	
	strncpy(buffer + j * 32,want_name,strlen(want_name));
	
	LBA_to_CHS(prev_logical_block,&sector,&head,&track);
	if(biosdisk(3,tmp1,head,track,sector,1,buffer)){
		printf("can't use the %dth block",prev_logical_block);
		return;
	}
	
	return;
}




/**
* the command of format
*/
void Format(char* argv){
	char precious_path[1024],ch;

	/*initialize logical_block and index*/
	logical_block = 19;
	index = 0;
	
		


	strcpy(precious_path,argv);
	for (i = 0; precious_path[i]; i++) {
		precious_path[i] = toupper(precious_path[i]);
    }

    if(precious_path[1] != ':' || strlen(precious_path) != 2){
		printf("Invalid drive specification.");
		return;
	}

	token = strtok(precious_path,"\\");

	Determine_floppy_number();


	printf("Insert new disk for drive %s\n",token);
	
    for(i = 1; i < 30;i++){

		printf(".");
		delay(100);
	}
	
//	while(getch() != '\r');

	printf("The type of the file system is RAW\n");
	printf("The new file system is FAT\n");
	printf("Verifying 1.44M\n");


    for(i = 1; i < 30;i++){

		printf(".");
		delay(100);
	}

	printf("percent completed.");


    printf("Initializing the File Allocation Table (FAT)...\n",i);
//    printf("Volume label (11 characters, ENTER for none)?");
//    while(getchar() != '\n');


	/*initialize the zero logical block*/
	Initialize_ZeroBlk();

	/*initialize the FAT12 Table*/
	Initialize_FAT12();

	/*initialize the root of FCB*/
	Initialize_RootFCB();

	/*initialize the logical blocks of 33-2847*/
	Initialize_BlkOfData();



	printf("Format complete.\n\n\
    1,457,644 bytes total disk space.\n\
    1,457,644 bytes available on disk.\n\n\
	  512 bytes in each allocation unit.\n\
	 2847 allocation units available on disk.\n\n\
	   12 bits in each FAT entry.\n\n\
volume Serial Number is: \n\n");

	return;
}
