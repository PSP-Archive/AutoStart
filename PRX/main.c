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

PSP_MODULE_INFO("AutoStart_PRX", 0x1007,1,1);
PSP_MAIN_THREAD_ATTR(0);

void save(Configuration *config)
{
	int tempConfig = sceIoOpen("ms0:/seplugins/Config.ROE", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	if(tempConfig < 0)
		return;
	char ROEdesc[] = "\0ROE";
	sceIoWrite(tempConfig, &ROEdesc, 4);

	int i;
	for(i = 0; i < 11; i++)
	{
		sceIoWrite(tempConfig, &config->button[i], 4);
		sceIoWrite(tempConfig, &config->type[i], 4);
		sceIoWrite(tempConfig, &config->pathLen[i], 4);
		sceIoWrite(tempConfig, config->path[i], config->pathLen[i]);
	}
	sceIoWrite(tempConfig, &config->autoboot, 4);
	sceIoWrite(tempConfig, &config->wlan_check, 4);
	
	sceIoClose(tempConfig);
}

void load(Configuration *config)
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

	int i;
	for(i = 0; i < 11; i++)
	{
		sceIoRead(tempConfig, &config->button[i], 4);
		sceIoRead(tempConfig, &config->type[i], 4);
		sceIoRead(tempConfig, &config->pathLen[i], 4);
		sceIoRead(tempConfig, config->path[i], config->pathLen[i]);
	}
	sceIoRead(tempConfig, &config->autoboot, 4);
	sceIoRead(tempConfig, &config->wlan_check, 4);
	
	sceIoClose(tempConfig);
}
char configInited = 0;
void ConfigInit(Configuration *config)
{
	if(configInited)
		return;
	int i;
	memset(config, 0, sizeof(Configuration));
	config->button[0] = PSP_CTRL_SQUARE;
	config->button[1] = PSP_CTRL_TRIANGLE;
	config->button[2] = PSP_CTRL_CIRCLE;
	config->button[3] = PSP_CTRL_CROSS;
	config->button[4] = PSP_CTRL_LTRIGGER;
	config->button[5] = PSP_CTRL_START;
	config->button[6] = PSP_CTRL_SELECT;
	config->button[7] = PSP_CTRL_NOTE;
	config->button[8] = PSP_CTRL_SCREEN;
	config->button[9] = PSP_CTRL_HOME;
	config->button[10] = 0;
	config->defaultPath = "NOTHING";
	for(i = 0; i < 11; i++)
	{
		config->type[i] = 0;
		config->pathLen[i] = strlen(config->defaultPath);
		strcpy(config->path[i], config->defaultPath);
	}
	config->autoboot = 1;
	config->wlan_check = 0;
	load(config);
	save(config);
	configInited = 1;
}

void exitGamePatched()
{
	int k1 = pspSdkSetK1(0);

	sctrlKernelExitVSH(NULL);
	
	pspSdkSetK1(k1);
}

void Exec(char *path, int type)
{
	int k1 = pspSdkSetK1(0);
	
	u32 vshmain_args[0x0400/4];
	memset(vshmain_args, 0, 0x0400);
	vshmain_args[0/4] = 0x0400;
	vshmain_args[4/4] = 0x20;
	vshmain_args[0x40/4] = 1;
	vshmain_args[0x280/4] = 1;
	vshmain_args[0x284/4] = 0x40003;
	
	struct SceKernelLoadExecVSHParam param;
	
	memset(&param, 0, sizeof(param));
	param.size = sizeof(param);
	param.args = strlen(path)+1;
	param.argp = path;
	if(type != EXEC_TYPE_PSX)
		param.key = "game";
	else param.key = "pops";
	param.vshmain_args_size = 0x0400;
	param.vshmain_args = vshmain_args;

	if(type == EXEC_TYPE_ISO)
	{
		sctrlSESetUmdFile(path);
		sctrlSESetBootConfFileIndex(2);
		sctrlKernelLoadExecVSHWithApitype(0x120, "disc0:/PSP_GAME/SYSDIR/EBOOT.BIN", &param); 
	} else {
		if(type == EXEC_TYPE_PSX)
			sctrlKernelLoadExecVSHWithApitype(0x143, path, &param);
		else sctrlKernelLoadExecVSHMs2(path, &param);
	}
	pspSdkSetK1(k1);
}

int main_thread(SceSize args, void *argp)
{
	while(!sceKernelFindModuleByName("sceLoadExec"))
		sceKernelDelayThread(10000);
	SceCtrlData pad;
	Configuration config;
	int i;
	sceCtrlPeekBufferPositive(&pad, 1);
	ConfigInit(&config);
	int rawr = kuKernelInitApitype();
	if(rawr == 0x210)
	{
		for(i = 0; i < 10; i++) {
			if(pad.Buttons & config.button[i])
				if(strcmp(config.path[i], "NOTHING") != 0)
					Exec(config.path[i], config.type[i]);
		}
	
		if(config.wlan_check == 1) {
			if(!(pad.Buttons & PSP_CTRL_WLAN_UP))
			{
				if(config.autoboot == 1)
					if(strcmp(config.path[10], "NOTHING") != 0)
						Exec(config.path[10], config.type[10]);
			}
		} else {
			if(config.autoboot == 1)
				if(strcmp(config.path[10], "NOTHING") != 0)
					Exec(config.path[10], config.type[10]);
		}
	} 
	if(rawr == 0x220 || rawr == 0x210) {
		while(thid)
		{
			sceCtrlPeekBufferPositive(&pad, 1);
			for(i = 0; i < 10; i++)
			{
				if(pad.Buttons & PSP_CTRL_RTRIGGER && pad.Buttons & config.button[i])
					if(strcmp(config.path[i], "NOTHING") != 0)
						Exec(config.path[i], config.type[i]);
				sceKernelDelayThread(1);
			}
			sceKernelDelayThread(500000);
		}
	} else {
		unsigned int orig_call[11];
		hookAPI("sceLoadExec", "LoadExecForUser", 0x05572A5F, exitGamePatched, HOOK_JUMP, orig_call);
	}

	sceKernelSelfStopUnloadModule(1, 0, NULL);
	return 0;
}

int module_start(SceSize args, void *argp)
{
	SceUID fd = sceIoOpen("ms0:/seplugins/AutoStart.prx", PSP_O_RDONLY, 0);
	if (fd < 0)
	{
		return 0;
	}

	int size = sceIoLseek(fd, 0, PSP_SEEK_END);
	sceIoLseek(fd, 0, PSP_SEEK_SET);

	SceUID pid = sceKernelAllocPartitionMemory(PSP_MEMORY_PARTITION_KERNEL, "", PSP_SMEM_Low, size, NULL);
	if (pid < 0)
		return 0;

	sceIoRead(fd, sceKernelGetBlockHeadAddr(pid), size);
	
	sctrlHENLoadModuleOnReboot("/kd/usersystemlib.prx", sceKernelGetBlockHeadAddr(pid), size, BOOTLOAD_GAME | BOOTLOAD_POPS | BOOTLOAD_UMDEMU);

	thid = sceKernelCreateThread("AutoStart_thread", main_thread, 0x0F, 0x10000, 0, NULL);
	if(thid >= 0)
	{
		sceKernelStartThread(thid, args, argp);
	}
	
	return 0;
}

int module_stop() 
{
	sceKernelTerminateDeleteThread(thid);
	return 0;
}
