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

SceCtrlData inputData[2];
u8 inputStates[32];

void initKeys() 
{
	sceCtrlReadBufferPositive(&inputData[0], 1);
}

u8 Button(u8 button) {
	return inputStates[button];
}

void readKeys() 
{
	inputData[0] = inputData[1];
	sceCtrlPeekBufferPositive(&inputData[1], 1);
	int i;
	for(i = 0; i < 32; i++) {
		if(inputData[1].Buttons & (1 << i))
			inputStates[i] = 1;
		else
			inputStates[i] = 0;
		if((inputData[0].Buttons & (1 << i)) != (inputData[1].Buttons & (1 << i)))
			inputStates[i] |= 2;
	}
}
