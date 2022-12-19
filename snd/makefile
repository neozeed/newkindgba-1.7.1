# you might have to customize this stuff a little

include ../../gbalib/master.mk
SHELL = sh
MODULES = danube.s3m theme.s3m elitesfx.s3m
ASFLAGS = -mthumb-interwork
PROJECT = modules.lib
CONVERT = ../../../krawall/converter/converter.linux
#CONVERT = $(GCCARM)/bin2o.exe
RM = rm -f

all : $(PROJECT)

$(PROJECT) : makefile $(MODULES)
	-$(RM) *.s
	$(CONVERT) $(MODULES)
	$(AS) $(ASFLAGS) *.s -o $@

.PHONY : clean

clean :
	-$(RM) *.s *.h *.lib

