# $Header: Exp $
 include $(PS2SDK)/Defs.make

PS2_EXTRA = /works/devel/ps2/sdk-extra
PS2_EXTRA_INCS = /zlib/include /libmad/ee/include /SjPcm/ee/src /tremor
PS2_EXTRA_LIBS = /zlib/lib /libmad/ee/lib /SjPcm/ee/lib /tremor/tremor

ENABLED=STATIC_PLUGIN

#control build
DISABLE_SCALERS = true
DISABLE_HQ_SCALERS = true

ENABLE_SCUMM = $(ENABLED)
ENABLE_SCUMM_7_8 = $(ENABLED)
# ENABLE_HE = $(ENABLED)
# ENABLE_AGI = $(ENABLED)
# ENABLE_AGOS = $(ENABLED)
# ENABLE_AGOS2 = $(ENABLED)
# ENABLE_CINE = $(ENABLED)
# ENABLE_CRUISE = $(ENABLED)
# ENABLE_DRACI = $(ENABLED)
# ENABLE_DRASCULA = $(ENABLED)
# ENABLE_GOB = $(ENABLED)
# ENABLE_GROOVIE = $(ENABLED)
## ENABLE_GROOVIE2 = $(ENABLED)
# ENABLE_IHNM = $(ENABLED)
# ENABLE_KYRA = $(ENABLED)
## ENABLE_LOL = $(ENABLED)
# ENABLE_LURE = $(ENABLED)
## ENABLE_M4 = $(ENABLED)
# ENABLE_MADE = $(ENABLED)
# ENABLE_PARALLACTION = $(ENABLED)
# ENABLE_QUEEN = $(ENABLED)
# ENABLE_SAGA = $(ENABLED)
# ENABLE_SAGA2 = $(ENABLED)
# ENABLE_SCI = $(ENABLED)
## ENABLE_SCI32 = $(ENABLED)
# ENABLE_SKY = $(ENABLED)
# ENABLE_SWORD1 = $(ENABLED)
# ENABLE_SWORD2 = $(ENABLED)
# ENABLE_TEENAGENT = $(ENABLED)
# ENABLE_TINSEL = $(ENABLED)
# ENABLE_TOUCHE = $(ENABLED)
# ENABLE_TUCKER = $(ENABLED)


HAVE_GCC3 = true

CC      = ee-gcc
CXX     = ee-g++
AS      = ee-gcc
LD      = ee-gcc
AR      = ee-ar cru
RANLIB  = ee-ranlib
STRIP   = ee-strip
MKDIR   = mkdir -p
RM      = rm -f

srcdir = ../../..
VPATH = $(srcdir)
INCDIR = ../../../
# DEPDIR = .deps

DEFINES  = -DRELEASE_BUILD -DUSE_VORBIS -DUSE_TREMOR -DUSE_MAD -DUSE_ZLIB -DFORCE_RTL -DDISABLE_SAVEGAME_SORTING -D_EE -D__PLAYSTATION2__ -D__PS2_DEBUG__ -g -Wall -Wno-multichar -fno-rtti -fno-exceptions # -DNO_ADAPTOR

INCLUDES  = $(addprefix -I$(PS2_EXTRA),$(PS2_EXTRA_INCS)) 
INCLUDES += -I $(PS2GDB)/ee -I $(PS2SDK)/ee/include -I $(PS2SDK)/common/include -I ./common -I . -I $(srcdir) -I $(srcdir)/engines

CXX_UPDATE_DEP_FLAG = -Wp,-MMD,"$(*D)/$(DEPDIR)/$(*F).d",-MQ,"$@",-MP

TARGET = elf/scummvm.elf

OBJS := $(srcdir)/backends/platform/ps2/DmaPipe.o \
	$(srcdir)/backends/platform/ps2/Gs2dScreen.o \
	$(srcdir)/backends/platform/ps2/irxboot.o \
	$(srcdir)/backends/platform/ps2/ps2input.o \
	$(srcdir)/backends/platform/ps2/ps2pad.o \
	$(srcdir)/backends/platform/ps2/savefilemgr.o \
	$(srcdir)/backends/platform/ps2/fileio.o \
	$(srcdir)/backends/platform/ps2/asyncfio.o \
	$(srcdir)/backends/platform/ps2/icon.o \
	$(srcdir)/backends/platform/ps2/cd.o \
	$(srcdir)/backends/platform/ps2/eecodyvdfs.o \
	$(srcdir)/backends/platform/ps2/rpckbd.o \
	$(srcdir)/backends/platform/ps2/systemps2.o \
	$(srcdir)/backends/platform/ps2/ps2mutex.o \
	$(srcdir)/backends/platform/ps2/ps2time.o \
	$(srcdir)/backends/platform/ps2/ps2debug.o
    
MODULE_DIRS += .

include $(srcdir)/Makefile.common

LDFLAGS += -mno-crt0 $(PS2SDK)/ee/startup/crt0.o -T $(PS2SDK)/ee/startup/linkfile
LDFLAGS += -L $(PS2GDB)/lib  -L $(PS2SDK)/ee/lib -L .
LDFLAGS += $(addprefix -L$(PS2_EXTRA),$(PS2_EXTRA_LIBS))
LDFLAGS += -lmc -lpad -lmouse -lhdd -lpoweroff -lsjpcm -lmad -ltremor -lz -lm -lc -lfileXio -lps2gdbStub -lps2ip -ldebug -lkernel -lstdc++

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LD) $^ $(LDFLAGS) -o $@

