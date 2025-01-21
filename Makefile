# Levi Hung 2009.10.13
#
# Default Setting
# To overwrite the setting, you may append parameters along with make 
# Ex: make NAND=1
#
export DEBUG	= 0
export THUMB	= 0
export NAND	    = 1
BSP		        = bsp
BSPNAME		    = SoC
DRV		        = drivers


ifeq ($(THUMB),0)
# This is Faraday's default gcc, however it has problems to generate thumb instructions 
XCOMPILE	= /opt/crosstool/arm-linux/gcc-3.4.4-glibc-2.3.5/arm-linux/bin/arm-linux-
else
# This is my own gcc which is able to generate thumb instructions
# and even in ARM mode, it generates smaller code size.
XCOMPILE	= /opt/arm-uclibc-4.3.2-nofpu-oabi/build_arm/staging_dir/bin/arm-linux-
endif

ROOTDIR		= $(shell pwd)
export ROOTDIR
INCLUDE		= include
export INCLUDE

CC          = $(XCOMPILE)gcc
CXX		    = $(XCOMPILE)g++
AS		    = $(XCOMPILE)as
LD		    = $(XCOMPILE)ld
AR		    = $(XCOMPILE)ar
NM		    = $(XCOMPILE)nm
STRIP		= $(XCOMPILE)strip
RANLIB		= $(XCOMPILE)ranlib
OBJDUMP		= $(XCOMPILE)objdump
OBJCOPY		= $(XCOMPILE)objcopy
export CC CXX AS LD AR NM STRIP RANLIB OBJDUMP OBJCOPY

-include $(BSP)/config.mk

DBGFLAGS	=
OPTFLAGS	= -Os
#OPTFLAGS	= -O1
BSPFLAGS	= -march=armv4

ifeq ($(DEBUG),1)

BOOTCODE2_LIMIT	= 65536
DBGFLAGS += -D_DEBUG
BSPFLAGS += -mno-thumb-interwork

else

BOOTCODE2_LIMIT	= 2048
ifeq ($(THUMB),1)
BSPFLAGS += -mthumb -D__THUMB__
else
BSPFLAGS += -mno-thumb-interwork
endif

endif

ifeq ($(NAND),1)
BSPFLAGS	+= -D_NANDBOOT -DFW_DEBUG
endif

CFLAGS		= $(DBGFLAGS) $(OPTFLAGS) $(BSPFLAGS) \
				-fno-strict-aliasing -fno-common -fno-builtin -ffreestanding -msoft-float -pipe \
				-Wall -Wstrict-prototypes \
				-nostdinc -isystem $(shell $(CC) -print-file-name=include) \
				-DTEXT_BASE=$(TEXT_BASE) \
				-I$(ROOTDIR) -I$(ROOTDIR)/$(INCLUDE) -g
ASFLAGS		= -D__ASSEMBLY__ $(CFLAGS)
export CFLAGS ASFLAGS

OBJS		= $(BSP)/board.o \
		  $(BSP)/main.o 
OBJS		+= $(DRV)/serial.o \
		   $(DRV)/nandc.o \
		   $(DRV)/sdc.o \
		   $(DRV)/sdc_cntr.o \
		   $(DRV)/scu.o
HEADER		+= $(INCLUDE)/bsp_common.h \
		   $(INCLUDE)/nandc.h \
		   $(INCLUDE)/sdc.h 
.PHONY : lib rom_bl.bin clean

all:  lib rom_bl.bin

lib:
	make clean
	make -C lib

rom_bl.bin: start.o $(OBJS) $(HEADER)
	$(LD) -Bstatic -T bsp/rom_bl.lds -Ttext $(TEXT_BASE) start.o \
		--start-group $(OBJS) lib/minilib.a --end-group \
		-L $(shell dirname `$(CC) $(CFLAGS) -print-libgcc-file-name`) -lgcc \
		-Map rom_bl.map -o rom_bl.elf
	$(OBJDUMP) -D rom_bl.elf > rom_bl.S
	$(OBJCOPY) --gap-fill=0xff -O binary rom_bl.elf rom_bl.bin
	$(NM) -n rom_bl.elf > system.map
	@echo ""
	@echo "[rom_bl.elf] Platform: $(BSPNAME)"
	@echo ""

clean:
	rm -rf $(OBJS) *.o *.bak bsp/*.bak *.map rom_bl.elf rom_bl.bin rom_bl.S
	make -C lib clean
