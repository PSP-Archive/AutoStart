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

#ifndef gamebrowser_h
#define gamebrowser_h

#include <vector>

enum execTypes {
	EXEC_TYPE_UMD = 0,
	EXEC_TYPE_ISO = 1,
	EXEC_TYPE_PSX = 2,
	EXEC_TYPE_NORM = 3
};

struct entrie {
	char *fileName;
	char *filePath;
	char *fileExt;
	char *fileType;
	char *gameName;
};

class gameBrowser
{
	private:
		int gamesNum;
		int sel;
		int offset;
		int MAX_DISP;
		entrie *selected;
		bool active;
		void SetSelected(entrie *set);
		void scanForPBP(const char *folder);
		int IsPBP(const char *dir);
		int IsISO(char *file);
		void alphaSort();
		bool exited;
		std::vector<entrie*> entries;
		
	public:
		gameBrowser();
		void draw();
		void control();
		entrie *getSelected()
		{
			return selected;
		}
		bool Exited()
		{
			bool ret = exited;
			exited = !exited;
			return ret;
		}
		void start()
		{
			active = true;
		}
		bool isActive()
		{
			return active;
		}
};

#endif
