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
#include <pspctrl.h>
#include <psprtc.h>

#include <vector>
#include <cstring>
#include <cstdio>

#include "graphics.h"
#include "ctrl.h"
#include "intraFont.h"
#include "Configuration.h"
#include "gameBrowser.h"

enum APP_STATES {
	APP_STATE_MAIN = 1,
	APP_STATE_OPTIONS = 2,
	APP_STATE_CREDITS = 3,
	APP_STATE_GAME_BROWSER = 4
};

int APP_STATE;

Image *menu, *options, *credits, *gamebrowser;
intraFont *main_font;
int menuSel;
bool running;
Configuration *config;
gameBrowser *browser;

char menuItem[][35] = {
	"Square",
	"Triangle",
	"Circle",
	"Cross",
	"L Trigger",
	"Start",
	"Select",
	"Note",
	"Screen",
	"Home",
	"Autoboot",
};

char optionsItem[][35] = {
	"Autoboot is OFF",
	"WLAN switch check is OFF",
	"Autoboot is ON",
	"WLAN switch check ON",
	"Exit."
};

#endif
