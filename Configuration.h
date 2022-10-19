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

#ifndef configuration_h
#define configuration_h

class Configuration
{
	private:
		u32 button[11];
		int type[11];
		int pathLen[11];
		char path[11][256];
		char *defaultPath;
		int autoboot;
		int wlan_check;
	
	public:
		Configuration();
		void save();
		void load();
		char *getPath(int index)
		{
			return path[index];
		}
		int getAutoboot()
		{
			return autoboot;
		}
		int getWlanCheck()
		{
			return wlan_check;
		}
		int getType(int index)
		{
			return type[index];
		}
		int getButton(int index)
		{
			return button[index];
		}
		void setType(int index, int value)
		{
			type[index] = value;
		}
		void setPathLen(int index, int value)
		{
			pathLen[index] = value;
		}
		void setPath(int index, char *text)
		{
			strcpy(path[index], text);
		}
		void toggleAutoboot()
		{
			if(autoboot == 1) autoboot = 0;
			else if(autoboot == 0) autoboot = 1;
		}
		void toggleWlanCheck()
		{
			if(wlan_check == 1) wlan_check = 0;
			else if(wlan_check == 0) wlan_check = 1;
		}
};

#endif
