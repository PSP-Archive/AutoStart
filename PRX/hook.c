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

#include <stdio.h>
#include <psputilsforkernel.h>
#include <systemctrl.h>
#include <string.h>
//#include "interruptman.h"
#include "hook.h"

// MIPS Opcodes
#define MIPS_NOP 0x00000000
#define MIPS_SYSCALL(NUM) 0x0C000000 | ( ( ( unsigned int )( NUM ) & 0x0ffffffc ) >> 2 )
#define MIPS_J(ADDR) (0x08000000 + ((((unsigned int)(ADDR))&0x0ffffffc)>>2))
#define MIPS_STACK_SIZE(SIZE) (0x27BD0000 + (((int)(SIZE)) & 0x0000FFFF))
#define MIPS_PUSH_RA(STACKPOS) (0xAFBF0000 + (((unsigned int)(STACKPOS)) & 0x0000FFFF))
#define MIPS_POP_RA(STACKPOS) (0x8FBF0000 + (((unsigned int)(STACKPOS)) & 0x0000FFFF))
#define MIPS_RETURN 0x03E00008


#define LOGFILE "ms0:/psphook.log"

int appendBufferToFile(const char * path, void * buffer, int buflen)
{
  // Written Bytes
  int written = 0;
 
  // Open File for Appending
  SceUID file = sceIoOpen(path, PSP_O_APPEND | PSP_O_CREAT | PSP_O_WRONLY, 0777);
 
  // Opened File
  if(file >= 0)
  {
    // Write Buffer
    written = sceIoWrite(file, buffer, buflen);
   
    // Close File
    sceIoClose(file);
  }
 
  // Return Written Bytes
  return written;
} 

int debuglog(const char * string)
{
  // Append Data
  return 0;//appendBufferToFile(LOGFILE, (void*)string, strlen(string));
}

int hookJump(const char * module, const char * library, unsigned int nid, void * function, unsigned int * orig_loader)
{
  // Hooking Result
  int result = 0;
 
  // Check Arguments
  if(module && library && function)
  {
    // Find Function
    unsigned int * sfunc = (unsigned int*)sctrlHENFindFunction(module, library, nid);
   
    // Found Function
    if(sfunc)
    {
      // Backup Interrupts
      int interrupt = pspSdkDisableInterrupts();
     
      // Create Original Loader
      if(orig_loader)
      {
        // Backup Original Instructions
        orig_loader[0] = sfunc[0];
        orig_loader[1] = sfunc[1];
        orig_loader[2] = sfunc[2];
        orig_loader[3] = sfunc[3];
        orig_loader[4] = sfunc[4];
        orig_loader[5] = sfunc[5];
        orig_loader[6] = sfunc[6];
        orig_loader[7] = sfunc[7];
       
        // Jump Delay Slot (Just to be on the safe side...)
        orig_loader[8] = MIPS_NOP;
       
        // Jump to Original Code
        orig_loader[9] = MIPS_J(&sfunc[8]);
       
        // Jump Delay Slot
        orig_loader[10] = MIPS_NOP;
      }

      // Patch Function with Jump
      sfunc[0] = MIPS_STACK_SIZE(-4); // Allocate 4 Bytes on Stack
      sfunc[1] = MIPS_PUSH_RA(0); // Backup RA on Stack
      sfunc[2] = MIPS_SYSCALL(function); // Syscall to our Hook
      sfunc[3] = MIPS_NOP; // Delay Slot
      sfunc[4] = MIPS_POP_RA(0); // Get RA from Stack
      sfunc[5] = MIPS_STACK_SIZE(4); // Free 4 Bytes on Stack
      sfunc[6] = MIPS_RETURN; // Return
      sfunc[7] = MIPS_NOP; // Delay Slot
     
      // Force Memory Mirroring
      sceKernelDcacheWritebackInvalidateRange(sfunc, sizeof(unsigned int) * 8);
      sceKernelIcacheInvalidateRange(sfunc, sizeof(unsigned int) * 8);
     
      // Enable Interrupts
      pspSdkEnableInterrupts(interrupt);
     
      // Hooking Debug Log
      char log[128];
      sprintf(log, "hookJump: Set Jump Hook on %08X to %08X (Module: %s, Library: %s, NID: %08X).\n", (unsigned int)sfunc, (unsigned int)function, module, library, nid);
      debuglog(log);
    }
   
    // Failed Finding Function
    else
    {
      // Result
      result = -5;
     
      // Log Error
      debuglog("hookJump: Couldn't find Function. NID might be incorrect.\n");
    }
  }
 
  // Invalid Arguments
  else
  {
    // Result
    result = -4;
   
    // Log Error
    debuglog("hookJump: Invalid Arguments.\n");
  }
 
  // Return Hooking Result
  return result;
}

int hookSyscall(const char * module, const char * library, unsigned int nid, void * function, unsigned int * orig_loader)
{
  // Hooking Result
  int result = 0;
 
  // Check Arguments
  if(module && library && function)
  {
    // Find Function
    unsigned int * sfunc = (unsigned int*)sctrlHENFindFunction(module, library, nid);
   
    // Found Function
    if(sfunc)
    {
      // Create Original Loader
      if(orig_loader)
      {
        // Jump to Original Code
        orig_loader[0] = MIPS_J(sfunc);
       
        // Jump Delay Slot
        orig_loader[1] = MIPS_NOP;
      }
     
      // Patch Syscall
      sctrlHENPatchSyscall((unsigned int)sfunc, function);
     
      // Hooking Debug Log
      char log[128];
      sprintf(log, "hookSyscall: Set Syscall Hook on %08X to %08X (Module: %s, Library: %s, NID: %08X).\n", (unsigned int)sfunc, (unsigned int)function, module, library, nid);
      debuglog(log);
    }
   
    // Failed Finding Function
    else
    {
      // Result
      result = -5;
     
      // Log Error
      debuglog("hookSyscall: Couldn't find Target Function. NID might be incorrect.\n");
    }
  }
 
  // Invalid Arguments
  else
  {
    // Result
    result = -4;
   
    // Log Error
    debuglog("hookSyscall: Invalid Arguments.\n");
  }
 
  // Return Hooking Result
  return result;
}

int hookAPI(const char * module, const char * library, unsigned int nid, void * function, int mode, unsigned int * orig_loader)
{
  // Hooking Result
  int result = 0;
 
  // Avoid Crash
  sceKernelDelayThread(10000);
 
  // Check Arguments
  if(module && library && function)
  {
    // Find Module in Memory
    SceModule * findmodule = (SceModule*)sceKernelFindModuleByName(module);
   
    // Found Module
    if(findmodule)
    {
      // Hook via Syscall
      if(mode == HOOK_SYSCALL) result = hookSyscall(module, library, nid, function, orig_loader);
     
      // Hook via Jump
      else if(mode == HOOK_JUMP) result = hookJump(module, library, nid, function, orig_loader);
     
      // Invalid Hook Mode
      else
      {
        // Result
        result = -3;
       
        // Log Error
        debuglog("hookAPI: Invalid Hook Mode.\n");
      }
    }
   
    // Couldn't Find Module
    else
    {
      // Result
      result = -2;
     
      // Log Error
      debuglog("hookAPI: Couldn't find Module. Might not be loaded yet.\n");
    }
  }
 
  // Invalid Arguments
  else
  {
    // Result
    result = -1;
   
    // Log Error
    debuglog("hookAPI: Invalid Arguments.\n");
  }
 
  // Avoid Crash
  sceKernelDelayThread(10000);
 
  // Return Hooking Result
  return result;
}
