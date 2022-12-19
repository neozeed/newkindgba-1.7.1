# this makefile converts the .pcx graphics files to a lib file
# so that the main makefile just has to include this lib and not all the graphics stuff

include ../../gbalib/master.mk

PROJECT = libelitegfx.a

BMPS = radar.bmp big_s.bmp big_e.bmp big_c.bmp
CONTROLS = controls.bmp

PALETTES = master2.pal master.pal

ALLDATARAW = $(CONTROLS:%.bmp=%.raw) $(BMPS:%.bmp=%.raw) 
ALLDATAMAP = $(CONTROLS:%.bmp=%.map) $(BMPS:%.bmp=%.map) 

OFILES = $(ALLDATARAW:%.raw=%.all.rodata.o) $(ALLDATAMAP:%.map=%.map.all.rodata.o) \
	$(PALETTES:%.pal=%.pal.all.rodata.o)

######################################################################

all: \
	$(PROJECT)

clean:
	rm $(OFILES)

$(PROJECT): $(OFILES)
	@echo $(OFILES)
	@echo $(ALLDATA)
	$(AR) rcs $@ $(OFILES)

$(ALLDATAMAP):
	$(GFX2GBA) -t8 -c16 -m -F -zt radar.bmp big_s.bmp big_e.bmp big_c.bmp
	$(GFX2GBA) -t8 -c16 -m -F -zt -pmaster2.pal controls.bmp
$(ALLDATARAW):
	$(GFX2GBA) -t8 -c16 -m -F -zt radar.bmp big_s.bmp big_e.bmp big_c.bmp
	$(GFX2GBA) -t8 -c16 -m -F -zt -pmaster2.pal controls.bmp

# create gcc object from map file
%.map.all.rodata.o: %.map
	$(OBJCOPY) $(OBJCOPYFLAGS) $< $@
		
# create gcc object from raw binary file
%.all.rodata.o: %.raw
	$(OBJCOPY) $(OBJCOPYFLAGS) $< $@
#	@$(INTERFLIP) -mthumb-interwork $@

# create gcc object from palette file
%.pal.all.rodata.o: %.pal
	$(OBJCOPY) $(OBJCOPYFLAGS) $< $@
#	@$(INTERFLIP) -mthumb-interwork $@


