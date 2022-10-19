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

#ifndef main_h
#define main_h

#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <pspctrl.h>
#include <kubridge.h>
#include <stdio.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include "hook.h"

#include <string.h>

enum execTypes {
	EXEC_TYPE_UMD = 0,
	EXEC_TYPE_ISO = 1,
	EXEC_TYPE_PSX = 2,
	EXEC_TYPE_NORM = 3
};

typedef struct Configuration
{
	u32 button[11];
	int type[11];
	int pathLen[11];
	char path[11][256];
	char *defaultPath;
	int autoboot;
	int wlan_check;
} Configuration;

SceUID thid;

#endif
