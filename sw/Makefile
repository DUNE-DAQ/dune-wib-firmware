CFLAGS = -march=native -mtune=native -g -O3 -Isrc
LFLAGS = -s


# output binary for each of these (no headers)
BSRC = src/read_sensors.c
BOBJ = $(addprefix build/,$(notdir $(BSRC:.c=.o)))
BDEP = $(BOBJ:.o=.d)

# component object for each src/*.c
LSRC = $(filter-out $(BSRC),$(wildcard src/*.c))
LOBJ = $(addprefix build/,$(notdir $(LSRC:.c=.o)))
LDEP = $(LOBJ:.o=.d)

BINS = $(notdir $(basename $(BSRC))) 

all: $(BINS)
	@echo Finished building

# binaries depend on all component objects
$(BINS): %: build/%.o $(LOBJ)
	$(CXX) $< $(LOBJ) $(LFLAGS) -o $@

$(BDEP): build/%.d: src/%.c
	@mkdir -p build
	@set -e; rm -f $@
	$(CC) -M $(CFLAGS) -MT $(<:.cxx=.o) $< > $@
	@sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' -i $@

$(LDEP): build/%.d: src/%.c src/%.h
	@mkdir -p build
	@set -e; rm -f $@
	$(CC) -M $(CFLAGS) -MT $(<:.c=.o) $< > $@
	@sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' -i $@

# these won't exist the first build
-include $(LDEP) $(BDEP)

$(BOBJ): build/%.o: build/%.d
	$(CC) $(CFLAGS) -c $(addprefix src/,$(notdir $(<:.d=.c))) -o $@

$(LOBJ): build/%.o: build/%.d
	$(CC) $(CFLAGS) -c $(addprefix src/,$(notdir $(<:.d=.c))) -o $@
