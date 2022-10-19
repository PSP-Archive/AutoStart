TARGET = AutoStart
OBJS = main.o hook.o
#InterruptManagerForKernel.o 
INCDIR = 
CFLAGS = -O2 -G0 -Wall -g
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti -fno-pic
ASFLAGS = $(CFLAGS)

PSP_FW_VERSION = 401
BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC=1
USE_KERNEL_LIBS=1

LIBDIR = 
LDFLAGS = -mno-crt0 -nostartfiles
LIBS = -lpspsystemctrl_kernel -lpspkubridge -lpspctrl_driver

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak
