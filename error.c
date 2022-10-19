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
#include <pspdebug.h>
#include <pspctrl.h>
#include <stdio.h>
#include "error.h"

void errorThrow(char* inMessage) {
     SceCtrlData inputPad;
     int inputLock = 25;
     while(1) {
          pspDebugScreenClear ();
          
          pspDebugScreenPrintf("ERROR!\n\n");
          pspDebugScreenPrintf(inMessage);
          pspDebugScreenPrintf("\n\nPress [X] to continue or [O] to quit.\n");
          
          if(inputLock > 0) {
               inputLock--;
          } else {
               sceCtrlReadBufferPositive(&inputPad, 1);
               if(inputPad.Buttons & PSP_CTRL_CROSS) {
                    break;
               } else if(inputPad.Buttons & PSP_CTRL_CIRCLE) {
                    sceKernelExitGame();
               }
          }     
     }
}

