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

#include "main.h"

PSP_MODULE_INFO("AutoStart", 0,1,1);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER | PSP_THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(-1024);

//static int logFile;
//static char data[256];

//#define logPrint() logFile = sceIoOpen("./log.txt", PSP_O_RDWR, 0777); sceIoLseek(logFile, 0, SEEK_END); sprintf(data, "File: %-24s Line: %4i\n", __FILE__,__LINE__); sceIoWrite(logFile, data, strlen(data)); sceIoClose(logFile);

int exit_callback(int arg1, int arg2, void *common)
{
	running = false;
	sceKernelExitGame();
	return 0;
}

int CallbackThread(SceSize args, void *argp) 
{
	int cbid;

	cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);

	sceKernelSleepThreadCB();

	return 0;
}

int SetupCallbacks(void) 
{
	int thid = 0;

	thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
	if(thid >= 0) 
	{
		sceKernelStartThread(thid, 0, 0);
	}

	return thid;
}

void initApp()
{
	pspDebugScreenInit();
	SetupCallbacks();
	graphicsInit();
	intraFontInit();
	initKeys();
	tzset();
	
	menuSel = 0;
	running = true;
	
	config = new Configuration();
	browser = new gameBrowser();
	
	menu = imageLoadPNG("./Data/Images/Main.png");
	options = imageLoadPNG("./Data/Images/Options.png");
	credits = imageLoadPNG("./Data/Images/Credits.png");
	gamebrowser = imageLoadPNG("./Data/Images/Gamebrowser.png");
	main_font = intraFontLoad("./Data/Fonts/main.pgf", INTRAFONT_CACHE_ASCII);
	intraFontSetStyle(main_font, .6f, COLOR_WHITE, COLOR_BLACK, 0);
	
	APP_STATE = APP_STATE_MAIN;
}

void appMain()
{
	static int buttonTimer;
	imageDrawAlpha(0, 0, menu);
	intraFontSetStyle(main_font, .5f, COLOR_WHITE,COLOR_BLACK, 0);
	for(int i = 0; i < 11; i++)
	{
		if(i == menuSel)
		{
			float t = ((float)(clock() % CLOCKS_PER_SEC)) / ((float)CLOCKS_PER_SEC);
			int val = int((t < 0.5f) ? t*511 : (1.0f-t)*511);
			intraFontSetStyle(main_font, .5f, COLOR_BLACK,(0xFF<<24)+(val<<16)+(val<<8)+(val), 0);
			intraFontPrintf(main_font, 15, i*13+55, "%s (%s)", menuItem[i], config->getPath(i));
			intraFontSetStyle(main_font, .5f, COLOR_WHITE,COLOR_BLACK, 0);
		} else intraFontPrintf(main_font, 15, i*13+55, "%s (%s)", menuItem[i], config->getPath(i));
	}
	intraFontSetStyle(main_font, .6f, COLOR_WHITE,COLOR_BLACK, 0);
	if(Button(DOWN) == PRESSED)
	{
		buttonTimer = 0;
		if(menuSel != 10)
			menuSel++;
	}
	if(Button(UP) == PRESSED)
	{
		buttonTimer = 0;
		if(menuSel != 0)
			menuSel--;
	}
	buttonTimer++;
	if(buttonTimer > 30 && Button(DOWN) == HELD)
	{
		if(menuSel != 10)
		{
			buttonTimer = 27;
			menuSel++;
		}
	}
	if(buttonTimer > 30 && Button(UP) == HELD)
	{
		if(menuSel != 0)
		{
			buttonTimer = 27;
			menuSel--;
		}
	}
	if(Button(TRIANGLE) == PRESSED)
	{
		config->setPathLen(menuSel, 7);
		config->setPath(menuSel, "NOTHING");
		config->save();
	}
	if(Button(CROSS) == PRESSED)
	{
		browser->start();
		APP_STATE = APP_STATE_GAME_BROWSER;
	}
}

void appBrowser()
{
	imageDrawAlpha(0, 0, gamebrowser);
	browser->draw();
	browser->control();
	if(!browser->isActive())
	{
		entrie *temp = browser->getSelected();
		if(!browser->Exited())
		{
			if(strcmp(temp->fileExt, "ISO") == 0)
			{
				config->setPath(menuSel, temp->filePath);
				config->setPathLen(menuSel, strlen(temp->filePath));
				config->setType(menuSel, EXEC_TYPE_ISO);
			}
			else if(strcmp(temp->fileExt, "PBP") == 0)
			{
				char cTemp[256];
				char *pos = strstr(temp->filePath, "%__SCE__");
				if(pos != NULL)
				memcpy(pos, pos+1, strlen(temp->filePath));
				pos = strrchr(temp->filePath, '%');
				if(pos != NULL)
				{
					if(temp->filePath[strlen(temp->filePath)-1] == '%')
					{
						snprintf(cTemp, strlen(temp->filePath), temp->filePath);
						strcpy(temp->filePath, cTemp);
					}
				}
				strcpy(cTemp, temp->filePath);
				strcat(cTemp, "/EBOOT.PBP");
				config->setPath(menuSel, cTemp);
				config->setPathLen(menuSel, strlen(cTemp));
				if(!strcmp(temp->fileType, "ME"))
					config->setType(menuSel, EXEC_TYPE_PSX);
				else config->setType(menuSel, EXEC_TYPE_NORM);
			}
			config->save();
		}
		APP_STATE = APP_STATE_MAIN;
	}
}

void appOptions()
{
	static int buttonTimer;
	imageDrawAlpha(0, 0, options);
	for(int i = 0; i < 3; i++)
	{
		if(i == menuSel)
		{
			float t = ((float)(clock() % CLOCKS_PER_SEC)) / ((float)CLOCKS_PER_SEC);
			int val = int((t < 0.5f) ? t*511 : (1.0f-t)*511);
			intraFontSetStyle(main_font, .6f, COLOR_BLACK,(0xFF<<24)+(val<<16)+(val<<8)+(val), 0);
			if(i == 0)
				intraFontPrintf(main_font, 15, i*15+55, "%s", config->getAutoboot() ? optionsItem[i+2] : optionsItem[i]);
			else if(i == 1)
				intraFontPrintf(main_font, 15, i*15+55, "%s", config->getWlanCheck() ? optionsItem[i+2] : optionsItem[i]);
			else if(i == 2) intraFontPrintf(main_font, 15, i*15+55, "%s", optionsItem[i+2]);
			intraFontSetStyle(main_font, .6f, COLOR_WHITE,COLOR_BLACK, 0);
		} else {
			if(i == 0)
				intraFontPrintf(main_font, 15, i*15+55, "%s", config->getAutoboot() ? optionsItem[i+2] : optionsItem[i]);
			else if(i == 1)
				intraFontPrintf(main_font, 15, i*15+55, "%s", config->getWlanCheck() ? optionsItem[i+2] : optionsItem[i]);
			else if(i == 2) intraFontPrintf(main_font, 15, i*15+55, "%s", optionsItem[i+2]);
		}
	}
	
	if(Button(DOWN) == PRESSED)
	{
		buttonTimer = 0;
		if(menuSel != 2)
			menuSel++;
	}
	if(Button(UP) == PRESSED)
	{
		buttonTimer = 0;
		if(menuSel != 0)
			menuSel--;
	}
	buttonTimer++;
	if(buttonTimer > 30 && Button(DOWN) == HELD)
	{
		if(menuSel != 2)
		{
			buttonTimer = 27;
			menuSel++;
		}
	}
	if(buttonTimer > 30 && Button(UP) == HELD)
	{
		if(menuSel != 0)
		{
			buttonTimer = 27;
			menuSel--;
		}
	}
	if(Button(CROSS) == PRESSED)
	{
		switch(menuSel)
		{
			case 0:
			{
				config->toggleAutoboot();
				break;
			}
			
			case 1:
			{
				config->toggleWlanCheck();
				break;
			}
			
			case 2:
			{
				sceKernelExitGame();
				break;
			}
		}
		config->save();
	}
}

void appCredits()
{
	imageDrawAlpha(0, 0, credits);
}

int main()
{
	initApp();

	while(running)
	{
		readKeys();
		switch(APP_STATE)
		{
			case APP_STATE_MAIN:
			{
				appMain();
				break;
			}
			
			case APP_STATE_GAME_BROWSER:
			{
				appBrowser();
				break;
			}
			
			case APP_STATE_OPTIONS:
			{
				appOptions();
				break;
			}
			
			case APP_STATE_CREDITS:
			{
				appCredits();
				break;
			}
		};
		
		pspTime ptm;
		sceRtcGetCurrentClockLocalTime(&ptm);
		intraFontPrintf(main_font, 427, 18, "%2.2d:%2.2d", ptm.hour, ptm.minutes);
		
		graphicsSwapBuffers();
		
		if(Button(RTRIGGER) == PRESSED)
		{
			if(APP_STATE == APP_STATE_CREDITS)
				APP_STATE = APP_STATE_MAIN;
			else APP_STATE++;
			menuSel = 0;
		}
		else if(Button(LTRIGGER) == PRESSED)
		{
			if(APP_STATE == APP_STATE_MAIN)
				APP_STATE = APP_STATE_CREDITS;
			else APP_STATE--;
			menuSel = 0;
		}
	}
	
	sceKernelExitGame();
	return 0;
}
