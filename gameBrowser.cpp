/*
	This file is part of AutoStart.

    AutoStart is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AutoStart is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AutoStart.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <pspkernel.h>
#include <systemctrl_se.h>

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>

#include "gameBrowser.h"
#include "ctrl.h"
#include "graphics.h"
#include "intraFont.h"

//static int logFile;
//static char data[256];

//#define logPrint() logFile = sceIoOpen("./log.txt", PSP_O_RDWR, 0777); sceIoLseek(logFile, 0, SEEK_END); sprintf(data, "File: %-24s Line: %4i\n", __FILE__,__LINE__); sceIoWrite(logFile, data, strlen(data)); sceIoClose(logFile);

struct SFO {
	int key_name_offset; // Offset of Key Name in Key Table
	int size_of_value_data; // Size of Value data 
	int value_offset; // Offset of Data Value in Data Table 
	char data_type[50];
	char data_value[50];
};


char* strToUpper(char* str) {
	int i;
	for(i = 0; str[i]; i++) {
		if((str[i] >= 'a') && (str[i] <= 'z'))
			str[i] -= ('a' - 'A');
	}
	return str;
}

char *getExtension(char *file)
{
	if(!file || !file[0]) {
		return NULL;
	}
	int tempLen = 0;
	char* tempStrPtr = &file[strlen(file) - 1];
	while(tempStrPtr[0] != '.') {
		tempLen++;
		tempStrPtr--;
	}
	if(!tempLen)
		return NULL;
	char* tempOut = (char*)malloc(tempLen + 1);
	if(!tempOut) {
		return NULL;
	}
	int i;
	for(i = 1; i <= tempLen; i++) {
		if((tempStrPtr[i] == '\r') ||(tempStrPtr[i] == '\n') || (tempStrPtr[i] == ' '))
			tempOut[i - 1] = 0;
		else
			tempOut[i - 1] = tempStrPtr[i];
	}
	tempOut[tempLen] = 0;
	strToUpper(tempOut);
	return tempOut;
}

gameBrowser::gameBrowser()
{
	SceIoDirent dir;
	int openDir;
	int scanned = 0;
	exited = false;
	gamesNum = 0;
	sel = 0;
	offset = 0;
	MAX_DISP = 11;
	active = false;
	char temp[256];
	char folders[][50] = {
		"ms0:/PSP/GAME",
		"ms0:/PSP/GAME340",
		"ms0:/PSP/GAME352",
		"ms0:/PSP/GAME371",
		"ms0:/PSP/GAME380",
		"ms0:/PSP/GAME390",
		"ms0:/PSP/GAME4XX",
		"ms0:/PSP/GAME5XX",
		"ms0:/PSP/GAME150",
		"ms0:/ISO"
	};
	while(scanned < 10)
	{
		openDir = sceIoDopen(folders[scanned]);
		if (openDir <= 0)
		{
			scanned++;
			continue;
		}
		while (1)
		{
			memset(&dir, 0, sizeof(SceIoDirent));
			if (sceIoDread(openDir, &dir) <= 0)
				break;
			if (!strcmp(dir.d_name, ".") || !strcmp(dir.d_name, ".."))
				continue;
			if(scanned < 9)
			{
				// Deal with game categories plugin
				if(!strncmp(dir.d_name, "CAT_", 4))
				{
					SceIoDirent catDir;
					strcpy(temp, folders[scanned]);
					strcat(temp, "/");
					strcat(temp, dir.d_name);
					int openCatDir = sceIoDopen(temp);
					while(1)
					{
						memset(&catDir, 0, sizeof(SceIoDirent));
						if (sceIoDread(openCatDir, &catDir) <= 0)
							break;
						if (!strcmp(catDir.d_name, ".") || !strcmp(catDir.d_name, ".."))
							continue;
						strcpy(temp, folders[scanned]);
						strcat(temp, "/");
						strcat(temp, dir.d_name);
						strcat(temp, "/");
						strcat(temp, catDir.d_name);
					
						scanForPBP(temp);
					}
					sceIoDclose(openCatDir);
				} else {
					strcpy(temp, folders[scanned]);
					strcat(temp, "/");
					strcat(temp, dir.d_name);

					scanForPBP(temp);
				}
			} else {
				// Deal with game categories plugin
				if(!strncmp(dir.d_name, "CAT_", 4))
				{
					SceIoDirent catDir;
					strcpy(temp, folders[scanned]);
					strcat(temp, "/");
					strcat(temp, dir.d_name);
					int openCatDir = sceIoDopen(temp);
					while(1)
					{
						memset(&catDir, 0, sizeof(SceIoDirent));
						if (sceIoDread(openCatDir, &catDir) <= 0)
							break;
						if (!strcmp(catDir.d_name, ".") || !strcmp(catDir.d_name, ".."))
							continue;
						strcpy(temp, folders[scanned]);
						strcat(temp, "/");
						strcat(temp, dir.d_name);
						strcat(temp, "/");
						strcat(temp, catDir.d_name);

						if(!strcmp(getExtension(catDir.d_name), "ISO") || !strcmp(getExtension(catDir.d_name), "CSO"))
						{
							if(IsISO(temp))
							{
								entries.push_back(new entrie());
								entries[gamesNum]->fileName = (char*)malloc(strlen(catDir.d_name)+1);
								entries[gamesNum]->filePath = (char*)malloc(strlen(temp)+1);
								entries[gamesNum]->fileExt = (char*)malloc(4);
								strcpy(entries[gamesNum]->fileName, catDir.d_name);
								strcpy(entries[gamesNum]->filePath, temp);
								strcpy(entries[gamesNum]->fileExt, "ISO");
								
								int sfo_size = 0, key_table_offset = 0, value_table_offset = 0, num_of_data_items = 0, index_offset = 20;
								int sfo = sceIoOpen("disc0:/PSP_GAME/PARAM.SFO", PSP_O_RDONLY, 0777);
								sfo_size = sceIoLseek32(sfo, 0, SEEK_END);
								short sfoS[sfo_size];
								char sfoB[sfo_size];
								sceIoLseek32(sfo, 0, SEEK_SET);
								sceIoRead(sfo, sfoS, sfo_size);
								sceIoLseek32(sfo, 0, SEEK_SET);
								sceIoRead(sfo, sfoB, sfo_size);
								sceIoClose(sfo);
								
								key_table_offset = sfoS[4];
								value_table_offset = sfoS[6];
								num_of_data_items = sfoS[8];
								SFO param[num_of_data_items];
								int x;
								int i;
								for(i = 0; i < num_of_data_items; i++)
								{
									param[i].key_name_offset = 0;
									param[i].size_of_value_data = 0;
									param[i].value_offset = 0;
									memset(&param[i].data_type, 0, sizeof(char));
									memset(&param[i].data_value, 0, sizeof(char));
								}
								for(i = 0; i < num_of_data_items; i++)
								{
									param[i].key_name_offset = sfoS[(index_offset+(i*16))/2];
									param[i].size_of_value_data = sfoS[(index_offset+4+(i*16))/2];
									param[i].value_offset = sfoS[(index_offset+12+(i*16))/2];
									for(x = 0; sfoB[param[i].key_name_offset+key_table_offset+x] != 0x00; x++)
									{
										sprintf(param[i].data_type, "%s%c", param[i].data_type, sfoB[param[i].key_name_offset+key_table_offset+x]);
									}
									for(x = 0; x < param[i].size_of_value_data; x++)
									{
										sprintf(param[i].data_value, "%s%c", param[i].data_value, sfoB[param[i].value_offset+value_table_offset+x]);
									}
								}
								for(i = 0; i < num_of_data_items; i++)
								{
									if(strcmp(param[i].data_type, "TITLE")==0){
										entries[gamesNum]->gameName = (char*)malloc(strlen(param[i].data_value[0]=='\0'?"No Name":param[i].data_value)+1);
										sprintf(entries[gamesNum]->gameName, param[i].data_value[0]=='\0'?"No Name":param[i].data_value);
										break;
									}
									else {
										entries[gamesNum]->gameName = (char*)malloc(strlen("No Name")+1);
										sprintf(entries[gamesNum]->gameName, "No Name");
									}
								}
								for(i = 0; i < num_of_data_items; i++)
								{
									if(strcmp(param[i].data_type, "CATEGORY")==0){
										entries[gamesNum]->fileType = (char*)malloc(strlen(param[i].data_value)+1);
										sprintf(entries[gamesNum]->fileType, param[i].data_value);
										break;
									}
									else {
										entries[gamesNum]->fileType = (char*)malloc(strlen("UNKNOWN")+1);
										sprintf(entries[gamesNum]->fileType, "UNKNOWN");
									}
								}
														
								gamesNum++;
							}
						}
					}
					sceIoDclose(openCatDir);
				}
				else if(!strcmp(getExtension(dir.d_name), "ISO") || !strcmp(getExtension(dir.d_name), "CSO"))
				{
					strcpy(temp, folders[scanned]);
					strcat(temp, "/");
					strcat(temp, dir.d_name);
				
					if(IsISO(temp))
					{
						entries.push_back(new entrie());
						entries[gamesNum]->fileName = (char*)malloc(strlen(dir.d_name)+1);
						strcpy(entries[gamesNum]->fileName, dir.d_name);
						strcpy(temp, folders[scanned]);
						strcat(temp, "/");
						strcat(temp, dir.d_name);
						entries[gamesNum]->filePath = (char*)malloc(strlen(temp)+1);
						entries[gamesNum]->fileExt = (char*)malloc(4);
						strcpy(entries[gamesNum]->filePath, temp);
						strcpy(entries[gamesNum]->fileExt, "ISO");
						
						int sfo_size = 0, key_table_offset = 0, value_table_offset = 0, num_of_data_items = 0, index_offset = 20;
						int sfo = sceIoOpen("disc0:/PSP_GAME/PARAM.SFO", PSP_O_RDONLY, 0777);
						sfo_size = sceIoLseek32(sfo, 0, SEEK_END);
						short sfoS[sfo_size];
						char sfoB[sfo_size];
						sceIoLseek32(sfo, 0, SEEK_SET);
						sceIoRead(sfo, sfoS, sfo_size);
						sceIoLseek32(sfo, 0, SEEK_SET);
						sceIoRead(sfo, sfoB, sfo_size);
						sceIoClose(sfo);
						
						key_table_offset = sfoS[4];
						value_table_offset = sfoS[6];
						num_of_data_items = sfoS[8];
						SFO param[num_of_data_items];
						int x;
						int i;
						for(i = 0; i < num_of_data_items; i++)
						{
							param[i].key_name_offset = 0;
							param[i].size_of_value_data = 0;
							param[i].value_offset = 0;
							memset(&param[i].data_type, 0, sizeof(char));
							memset(&param[i].data_value, 0, sizeof(char));
						}
						for(i = 0; i < num_of_data_items; i++)
						{
							param[i].key_name_offset = sfoS[(index_offset+(i*16))/2];
							param[i].size_of_value_data = sfoS[(index_offset+4+(i*16))/2];
							param[i].value_offset = sfoS[(index_offset+12+(i*16))/2];
							for(x = 0; sfoB[param[i].key_name_offset+key_table_offset+x] != 0x00; x++)
							{
								sprintf(param[i].data_type, "%s%c", param[i].data_type, sfoB[param[i].key_name_offset+key_table_offset+x]);
							}
							for(x = 0; x < param[i].size_of_value_data; x++)
							{
								sprintf(param[i].data_value, "%s%c", param[i].data_value, sfoB[param[i].value_offset+value_table_offset+x]);
							}
						}
						for(i = 0; i < num_of_data_items; i++)
						{
							if(strcmp(param[i].data_type, "TITLE")==0){
								entries[gamesNum]->gameName = (char*)malloc(strlen(param[i].data_value[0]=='\0'?"No Name":param[i].data_value)+1);
								sprintf(entries[gamesNum]->gameName, param[i].data_value[0]=='\0'?"No Name":param[i].data_value);
								break;
							}
							else {
								entries[gamesNum]->gameName = (char*)malloc(strlen("No Name")+1);
								sprintf(entries[gamesNum]->gameName, "No Name");
							}
						}
						for(i = 0; i < num_of_data_items; i++)
						{
							if(strcmp(param[i].data_type, "CATEGORY")==0){
								entries[gamesNum]->fileType = (char*)malloc(strlen(param[i].data_value)+1);
								sprintf(entries[gamesNum]->fileType, param[i].data_value);
								break;
							}
							else {
								entries[gamesNum]->fileType = (char*)malloc(strlen("UNKNOWN")+1);
								sprintf(entries[gamesNum]->fileType, "UNKNOWN");
							}
						}
												
						gamesNum++;
					}
				}
			}
		}
		sceIoDclose(openDir);
		scanned++;
	}
	alphaSort();
	selected = new entrie();
	selected->gameName = (char*)malloc(8);
	strcpy(selected->gameName, "NOTHING");
}

extern intraFont *main_font;
void gameBrowser::draw()
{
	for(int i = offset; i <= MAX_DISP+offset && i < gamesNum; i++)
	{
		if(i == sel)
		{
			float t = ((float)(clock() % CLOCKS_PER_SEC)) / ((float)CLOCKS_PER_SEC);
			int val = int((t < 0.5f) ? t*511 : (1.0f-t)*511);
			intraFontSetStyle(main_font, .5f, COLOR_BLACK,(0xFF<<24)+(val<<16)+(val<<8)+(val), 0);
			intraFontPrintf(main_font, 15, ((i-offset)*13)+55, "%s", entries[i]->gameName);
			intraFontSetStyle(main_font, .5f, COLOR_WHITE,COLOR_BLACK, 0);
		} else intraFontPrintf(main_font, 15, ((i-offset)*13)+55, "%s", entries[i]->gameName);
	}
}

extern int APP_STATE;
void gameBrowser::control()
{
	static int buttonTimer = 0;
	if(Button(DOWN) == PRESSED)
	{
		if(sel < gamesNum-1)
		{
			buttonTimer = 0;
			if(++sel > offset+MAX_DISP)
				offset++;
		}
	}
	if(Button(UP) == PRESSED)
	{
		if(sel > 0)
		{
			buttonTimer = 0;
			if(--sel < offset)
				offset--;
		}
	}

	buttonTimer++;
	if(buttonTimer > 30 && Button(DOWN) == HELD)
	{
		if(sel < gamesNum-1)
		{
			buttonTimer = 27;
			if(++sel > offset+MAX_DISP)
				offset++;
		}
	}
	if(buttonTimer > 30 && Button(UP) == HELD)
	{
		if(sel > 0)
		{
			buttonTimer = 27;
			if(--sel < offset)
				offset--;
		}
	}
	if(Button(CROSS) == PRESSED) 
	{
		buttonTimer = 0;
		selected = entries[sel];
		exited = false;
		active = false;
	}
	if(Button(SELECT) == PRESSED) 
	{
		buttonTimer = 0;
		exited = true;
		active = false;
	}
}

void gameBrowser::SetSelected(entrie *set)
{
	realloc(selected->fileName, strlen(set->fileName)+1);
	realloc(selected->filePath, strlen(set->filePath)+1);
	realloc(selected->fileExt, strlen(set->fileExt)+1);
	realloc(selected->fileType, strlen(set->fileType)+1);
	realloc(selected->gameName, strlen(set->gameName)+1);
	strcpy(selected->fileName, set->fileName);
	strcpy(selected->filePath, set->filePath);
	strcpy(selected->fileExt, set->fileExt);
	strcpy(selected->fileType, set->fileType);
	strcpy(selected->gameName, set->gameName);
}

int gameBrowser::IsISO(char *file)
{
	sctrlSEMountUmdFromFile(file, 1, 1);
	int sfo = sceIoOpen("disc0:/PSP_GAME/PARAM.SFO", PSP_O_RDONLY, 0777);	
	if(sfo >= 0)
	{
		sceIoClose(sfo);
		return 1;
	}
	
	return 0;
}

int gameBrowser::IsPBP(const char *dir)
{
	int ret = 1;
	char path[256];
	strcpy(path, dir);
	strcat(path, "/EBOOT.PBP");
	
	int eboot = sceIoOpen(path, PSP_O_RDONLY, 0777);	
	char desc[4];
	sceIoRead(eboot,desc,4); //Read the descriptor
	sceIoClose(eboot);
	
	if(desc[0] != 0x00)
		ret = 0;
	if(desc[1] != 0x50)
		ret = 0;
	if(desc[2] != 0x42)
		ret = 0;
	if(desc[3] != 0x50)
		ret = 0;
	
	return ret;
}

void gameBrowser::scanForPBP(const char *folder)
{
	SceIoDirent gameDir;
	memset(&gameDir, 0, sizeof(SceIoDirent));
	int dir = sceIoDopen(folder);
	if(dir > 0)
	{
		sceIoDread(dir, &gameDir);
		sceIoDread(dir, &gameDir);
		while(sceIoDread(dir, &gameDir) > 0)
		{
			strToUpper(gameDir.d_name);
			if(!strcmp(gameDir.d_name, "EBOOT.PBP")){
				if(IsPBP(folder))
				{
					entries.push_back(new entrie());
					entries[gamesNum]->fileName = (char*)malloc(strlen("EBOOT.PBP")+1);
					entries[gamesNum]->filePath = (char*)malloc(strlen(folder)+1);
					entries[gamesNum]->fileExt = (char*)malloc(strlen("PBP")+1);
					strcpy(entries[gamesNum]->fileName, "EBOOT.PBP");
					strcpy(entries[gamesNum]->filePath, folder);
					strcpy(entries[gamesNum]->fileExt, "PBP");
					short temp[40];
					int sfo_size = 0, key_table_offset = 0, value_table_offset = 0, num_of_data_items = 0, index_offset = 20;
	
					char tempC[256];
					strcpy(tempC, entries[gamesNum]->filePath);
					strcat(tempC, "/EBOOT.PBP");
					int eboot = sceIoOpen(tempC, PSP_O_RDONLY, 0777);
					sceIoRead(eboot, temp, 40);
					sfo_size = temp[6];
					sceIoLseek(eboot, 40, SEEK_SET);
					short sfoS[sfo_size];
					char sfoB[sfo_size];
					sceIoRead(eboot, sfoS, sfo_size);
					sceIoLseek(eboot, 40, SEEK_SET);
					sceIoRead(eboot, sfoB, sfo_size);
					sceIoClose(eboot);
					key_table_offset = sfoS[4];
					value_table_offset = sfoS[6];
					num_of_data_items = sfoS[8];
					SFO param[num_of_data_items];
					int x;
					int i;
					for(i = 0; i < num_of_data_items; i++)
					{
						param[i].key_name_offset = 0;
						param[i].size_of_value_data = 0;
						param[i].value_offset = 0;
						memset(&param[i].data_type, 0, sizeof(char));
						memset(&param[i].data_value, 0, sizeof(char));
					}
					for(i = 0; i < num_of_data_items; i++)
					{
						param[i].key_name_offset = sfoS[(index_offset+(i*16))/2];
						param[i].size_of_value_data = sfoS[(index_offset+4+(i*16))/2];
						param[i].value_offset = sfoS[(index_offset+12+(i*16))/2];
						for(x = 0; sfoB[param[i].key_name_offset+key_table_offset+x] != 0x00; x++)
						{
							sprintf(param[i].data_type, "%s%c", param[i].data_type, sfoB[param[i].key_name_offset+key_table_offset+x]);
						}
						for(x = 0; x < param[i].size_of_value_data; x++)
						{
							sprintf(param[i].data_value, "%s%c", param[i].data_value, sfoB[param[i].value_offset+value_table_offset+x]);
						}
					}
					for(i = 0; i < num_of_data_items; i++)
					{
						if(strcmp(param[i].data_type, "TITLE")==0){
							entries[gamesNum]->gameName = (char*)malloc(strlen(param[i].data_value[0]=='\0'?"No Name":param[i].data_value)+1);
							sprintf(entries[gamesNum]->gameName, param[i].data_value[0]=='\0'?"No Name":param[i].data_value);
							break;
						}
						else {
							entries[gamesNum]->gameName = (char*)malloc(strlen("No Name")+1);
							sprintf(entries[gamesNum]->gameName, "No Name");
						}
					}
					for(i = 0; i < num_of_data_items; i++)
					{
						if(strcmp(param[i].data_type, "CATEGORY")==0){
							entries[gamesNum]->fileType = (char*)malloc(strlen(param[i].data_value)+1);
							sprintf(entries[gamesNum]->fileType, param[i].data_value);
							break;
						}
						else {
							entries[gamesNum]->fileType = (char*)malloc(strlen("UNKNOWN")+1);
							sprintf(entries[gamesNum]->fileType, "UNKNOWN");
						}
					}
					gamesNum++;
				}
			}
		}
	}
	sceIoDclose(dir);
}

void gameBrowser::alphaSort()
{
	int i;
	int sorted = 0;
	char tempa[256], tempb[256];

	while(!sorted)
	{
		sorted = 1;
		for(i = 0; i < gamesNum-1; i++)
		{
			strcpy(tempa, entries[i]->gameName);
			strcpy(tempb, entries[i+1]->gameName);
			strToUpper(tempa);
			strToUpper(tempb);
			if(strcmp(tempa, tempb) > 0)
			{
				sorted = 0;
				entrie *p = entries[i];
				entries[i] = entries[i + 1];
				entries[i + 1]  = p;
			}
		}
	}
}
