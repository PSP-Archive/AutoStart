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

#ifndef ctrl_h
#define ctrl_h

#ifdef __cplusplus
extern "C" {
#endif

#define SELECT   0 // This is the button number representing the SELECT button.
#define START    3 //This is the button number representing the START button.
#define UP         4 // This is the button number representing the DPAD UP button.
#define RIGHT      5 // This is the button number representing the DPAD RIGHT button.
#define DOWN       6 // This is the button number representing the DPAD DOWN button.
#define LEFT       7 // This is the button number representing the DPAD LEFT button.
#define LTRIGGER 8 // This is the button number representing the LEFT TRIGGER button.
#define RTRIGGER 9 // This is the button number representing the RIGHT TRIGGER button.
#define TRIANGLE 12 // This is the button number representing the TRIANGLE button.
#define CIRCLE   13 // This is the button number representing the CIRCLE button.
#define CROSS    14 // This is the button number representing the CROSS button.
#define SQUARE   15 // This is the button number representing the SQUARE button.

#define PRESSED 3
#define HELD 1

void initKeys();
void readKeys();
u8 Button(u8 button);

#ifdef __cplusplus
}
#endif

#endif
