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
#include <pspctrl.h>

#include <cstring>

#include "Configuration.h"

Configuration::Configuration()
{
	button[0] = PSP_CTRL_SQUARE;
	button[1] = PSP_CTRL_TRIANGLE;
	button[2] = PSP_CTRL_CIRCLE;
	button[3] = PSP_CTRL_CROSS;
	button[4] = PSP_CTRL_LTRIGGER;
	button[5] = PSP_CTRL_START;
	button[6] = PSP_CTRL_SELECT;
	button[7] = PSP_CTRL_NOTE;
	button[8] = PSP_CTRL_SCREEN;
	button[9] = PSP_CTRL_HOME;
	button[10] = 0;
	defaultPath = "NOTHING";
	for(int i = 0; i < 11; i++)
	{
		type[i] = 0;
		pathLen[i] = strlen(defaultPath);
		strcpy(path[i], defaultPath);
	}
	autoboot = 1;
	wlan_check = 0;
	load();
	save();
}

void Configuration::save()
{
	int tempConfig = sceIoOpen("ms0:/seplugins/Config.ROE", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	if(tempConfig < 0)
		sceKernelExitGame();
	
	char ROEdesc[] = "\0ROE";
	sceIoWrite(tempConfig, &ROEdesc, 4);

	for(int i = 0; i < 11; i++)
	{
		sceIoWrite(tempConfig, (void *)&button[i], 4);
		sceIoWrite(tempConfig, (void *)&type[i], 4);
		sceIoWrite(tempConfig, (void *)&pathLen[i], 4);
		sceIoWrite(tempConfig, (void *)path[i], pathLen[i]);
	}
	sceIoWrite(tempConfig, (void *)&autoboot, 4);
	sceIoWrite(tempConfig, (void *)&wlan_check, 4);
	
	sceIoClose(tempConfig);
}

void Configuration::load()
{
	int tempConfig = sceIoOpen("ms0:/seplugins/Config.ROE", PSP_O_RDONLY, 0777);
	if(tempConfig < 0)
		return;
	char desc[4];
	sceIoRead(tempConfig, &desc, 4);
	if(desc[0] != '\0')
	{
		sceIoClose(tempConfig);
		return;
	}
	if(desc[1] != 'R')
	{
		sceIoClose(tempConfig);
		return;
	}
	if(desc[2] != 'O')
	{
		sceIoClose(tempConfig);
		return;
	}
	if(desc[3] != 'E')
	{
		sceIoClose(tempConfig);
		return;
	}

	for(int i = 0; i < 11; i++)
	{
		sceIoRead(tempConfig, &button[i], 4);
		sceIoRead(tempConfig, &type[i], 4);
		sceIoRead(tempConfig, &pathLen[i], 4);
		sceIoRead(tempConfig, path[i], pathLen[i]);
	}
	sceIoRead(tempConfig, &autoboot, 4);
	sceIoRead(tempConfig, &wlan_check, 4);
	
	sceIoClose(tempConfig);
}
