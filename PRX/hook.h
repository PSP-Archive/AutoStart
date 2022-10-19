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

#ifndef PSP_HOOK_H
#define PSP_HOOK_H

// Hook Modes
#define HOOK_SYSCALL 0
#define HOOK_JUMP 1

// Hook Function - returns 0 on success, < 0 on error.
int hookAPI(const char * module, const char * library, unsigned int nid, void * function, int mode, unsigned int * orig_loader);
int debuglog(const char * string);
#endif
