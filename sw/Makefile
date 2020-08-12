CFLAGS = -g -O3 -Isrc
LFLAGS = -lprotobuf -lzmq

# output binary for each of these (no headers)
BSRC = src/get_status.cxx src/wib_server.cxx src/initialize.cxx
BOBJ = $(addprefix build/,$(notdir $(BSRC:.cxx=.o)))
BDEP = $(BOBJ:.o=.d)

# component object for each src/*.cc
LSRC = $(filter-out $(BSRC),$(wildcard src/*.cc))
LOBJ = $(addprefix build/,$(notdir $(LSRC:.cc=.o)))
LDEP = $(LOBJ:.o=.d)

# protobuf dependencies
PBSRCS   := $(wildcard src/*.proto)
PBOBJS   := $(PBSRCS:.proto=.pb.o)
PBGENS   := $(PBSRCS:.proto=.pb.cc) $(PBSRCS:.proto=.pb.h)

BINS = $(notdir $(basename $(BSRC))) 

all: $(BINS)
	@echo Finished building

clean:
	@rm -rf build $(PBGENS)

src/%.pb.h: src/%.proto
	protoc -I=src --cpp_out=src $<

src/%.pb.cc: src/%.proto
	protoc -I=src --cpp_out=src $<

# binaries depend on all component objects
$(BINS): %: build/%.o $(LOBJ) $(PGOBJS)
	$(CXX) $< $(LOBJ) $(LFLAGS) -o $@

$(BDEP): build/%.d: src/%.cxx $(PBGENS)
	@mkdir -p build
	@set -e; rm -f $@
	$(CXX) -M $(CFLAGS) -MT $(<:.cxx=.o) $< > $@
	@sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' -i $@

$(LDEP): build/%.d: src/%.cc src/%.h $(PBGENS)
	@mkdir -p build
	@set -e; rm -f $@
	$(CXX) -M $(CFLAGS) -MT $(<:.cc=.o) $< > $@
	@sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' -i $@

# these won't exist the first build
-include $(LDEP) $(BDEP)

$(BOBJ): build/%.o: build/%.d $(PBGENS)
	$(CXX) $(CFLAGS) -c $(addprefix src/,$(notdir $(<:.d=.cxx))) -o $@

$(LOBJ): build/%.o: build/%.d $(PBGENS)
	$(CXX) $(CFLAGS) -c $(addprefix src/,$(notdir $(<:.d=.cc))) -o $@
