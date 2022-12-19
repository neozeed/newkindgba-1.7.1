
#export DEVKITARM := 1
# I just CANNOT get this to work with DKA
# I think interrupts are the problem
include ../gbalib/master.mk

.SUFFIX : .s .c .S

PROGNAME = elite
PROJECT = $(PROGNAME).gba
# get the palette directories..

OUTPUTDIR = o
ifdef DEVKITARM
LDFLAGS = -Wl,-Map,$(PROGNAME).map -mthumb-interwork -specs=gba.specs
else
LDFLAGS = -Wl,-Map,$(PROGNAME).map -mthumb-interwork -nostartfiles -Tlnkscript 
endif
#LIBS = -L../gbalib -lquirkygba -Limages -limg
LIBS = -L ../gbalib_asm -lgba ../krawall/krawall.lib snd/modules.lib -Limg -lelitegfx -lm
INCLUDES = -I../gbalib  -I../krawall

#-Ldata -ldata

ifdef test
  DEBUG=-g -DDEBUG
else
  DEBUG=
endif
ifdef DEVKITARM
  DEBUG += -DDevkitARM
endif

CFLAGS +=    -g -O2

#-I../krawall

ASFLAGS= -mthumb-interwork -g

ifdef usezip
ZIP = zip
ZIPFLAGS =  -rq
ZIPSUFFIX = zip
else
ifdef usebzip 
ZIPFLAGS =  cjf
ZIPSUFFIX = tar.bz2
endif
endif

CFILES := $(wildcard *.c)
ifndef DEVKITARM
NONDEVKITARM_ASM := crt0.S 
else
NONDEVKITARM_ASM := 
endif
ASMFILES := $(NONDEVKITARM_ASM) 

OFILES1 =   $(ASMFILES:%.S=%.o)  $(CFILES:%.c=%.o)

OFILES := $(OFILES1:%.o=$(OUTPUTDIR)/%.o) 
DEPENDS := $(CFILES:%.c=$(OUTPUTDIR)/%.d)

#SH := sh
.PHONY : clean cleanall releasedir dist distsrc

all: $(DEPENDS)
	$(MAKE) $(PROJECT)

clean: 
	-rm -f $(OUTPUTDIR)/*.o



relink:
	@-rm -f $(PROGNAME).elf
	@$(MAKE) all

cleanall: clean
	rm -f  $(OUTPUTDIR)/*.d
	$(MAKE) -C modules clean

RELEASEDIR := newkindgba-

releasedir :
# create a file that contains, eg. pocketbeeb-0.68
	@echo -n \
	$(RELEASEDIR)`grep version_str version.c | tr '"' ' ' | cut -d ' ' -f7` \
	> .fname
# remove the dist directory, remake it, then copy all the stuff in there 
	@-rm -rf `cat .fname`
	@-mkdir `cat .fname`

releasedirsrc: releasedir
	@for i in `find . -type d -not -name "$(RELEASEDIR)*" -mindepth 1`; do \
	mkdir `cat .fname`/$${i};\
	done

# make the distribution tar files 
dist: releasedir
	@echo "Building the distribution.."
	@cp $(PROJECT)  `cat .fname`
	@cp readme.txt  `cat .fname`/README.txt
	@cp history.txt  `cat .fname`/history.txt
	@$(ZIP) $(ZIPFLAGS) `cat .fname`.$(ZIPSUFFIX) `cat .fname`
	@echo "... done for" `cat .fname` "!" 
	@-rm -rf `cat .fname` .fname

DISTSRC_FILES=$(shell find "." -newer morning -type f -and \( -iname "*.[chs]" -or -iname "*.pcx" \
-or -name "Makefile" -or -name "*.asm" -or -name "mkimg*" -or -name "newest.sh" \
-or -name "*.txt" \) )
distsrc: releasedirsrc
	@echo "Building source distribution.."
	@cp lnkscript `cat .fname`
	@for i in `find "." -type f -and \( -iname "*.[chs]" -or -iname "*.pcx" \
-or -iname "make*" -or -name "*.s3m" -or -name "mkimg*" -or -name "newest.sh" \
-or -name "*.txt" -or -iname "*.bmp" \)`; do\
	cp $${i} `cat .fname`/$${i};\
	done;
	@echo "... done for" `cat .fname` "!" 
	@$(ZIP) $(ZIPFLAGS) `cat .fname`.src.$(ZIPSUFFIX) `cat .fname`
	@-rm -rf `cat .fname` .fname
	

# final step, create bin file from elf file
$(PROJECT) : $(PROGNAME).elf
	@$(OBJCOPY) -v -O binary $(PROGNAME).elf $(PROGNAME).tofix.gba
	@$(GBAFIX) $(GBAFIX_FLAGS) -n "Elite" -o $@ $(PROGNAME).tofix.gba
	@-rm $(PROGNAME).tofix.gba

# link all the object files into an elf file 
$(PROGNAME).elf : $(OFILES) $(LIBDEPENDS)
	@echo link $(PROGNAME)
	@$(GCC) $(LDFLAGS) -o $(PROGNAME).elf $(OFILES) $(LIBS)


# make object files from the source code
$(OUTPUTDIR)/%.arm.o : %.arm.c
	@echo compile arm $(CFLAGS) $(DEBUG) $<
	@$(GCC) $(INCLUDES) $(CFLAGS) -marm -mthumb-interwork $(DEBUG)  $< -o $@

# make object files from the source code
$(OUTPUTDIR)/%.o : %.c
	@echo compile thumb $(CFLAGS) $(DEBUG) $< 
	@$(GCC) $(INCLUDES) $(CFLAGS) $(CFLAGS-$<) -mthumb $(DEBUG)  $< -o $@

# make object files from binary data
$(OUTPUTDIR)/%.all.rodata.o : %.bin
	@echo objcopy rodata bin $<
	@$(OBJCOPY) $(OBJCOPYFLAGS) $< $@

$(OUTPUTDIR)/%.d:%.c
	@echo "build $@ from $<"
	@echo -n $@ $(OUTPUTDIR)/ > $@.tmp
	@set -e;$(GCC) $(INCLUDES) -M $< >> $@.tmp
	@mv $@.tmp $@

#$(OUTPUTDIR)/%.d: %.c
#	@echo "build $@ from $<"
#	set -e; $(GCC) -M $(INCLUDES) $< \
#	| sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
#	[ -s $@ ] || rm -f $@

# make .s object file
$(OUTPUTDIR)/%.o : %.S
	@echo assemble $(ASFLAGS) $(DEBUG) $<
	@$(GCC) $(ASFLAGS) -c $(INCLUDES) $(DEBUG) -o $@ $<

$(OUTPUTDIR)/%.o : %.s
	@echo assemble $(ASFLAGS) $(DEBUG) $<
	@$(GCC) $(ASFLAGS) -c $(INCLUDES) $(DEBUG) -o $@ $<

-include $(DEPENDS)
