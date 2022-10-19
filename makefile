TARGET = AutoStart
OBJS = main.o graphics.o error.o ctrl.o intraFont.o Configuration.o gameBrowser.o

INCDIR =
CFLAGS = -G4 -Wall -O2
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PSP_FW_VERSION = 401

LIBDIR =
LDFLAGS =
LIBS = -lpspgu -ljpeg -lpng -lz -lm -lpspsystemctrl_user -lpsprtc -lstdc++

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_ICON = ICON0.png
PSP_EBOOT_UNKPNG = PIC0.png
PSP_EBOOT_PIC1 = PIC1.png
PSP_EBOOT_TITLE = AutoStart Configurator

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

pack:
	psp-packer -s EBOOT.PBP