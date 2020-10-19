CCFLAGS = -g -O2 -Isrc
CXXFLAGS = -g -O2 -Isrc
LFLAGS = -lprotobuf -lzmq -lreadline -li2c

# output binary for each of these (no headers)
BSRC = src/wib_client.cxx src/wib_server.cxx
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

simulation: CXXFLAGS += -DSIMULATION
simulation: CCFLAGS += -DSIMULATION
simulation: $(BINS)

install:
	install -d "$(DESTDIR)/bin/"
	install -m 0755 wib_server "$(DESTDIR)/bin/"
	install -m 0755 wib_client "$(DESTDIR)/bin/"
	install -m 0755 extras/wib_update.sh "$(DESTDIR)/bin/"
	install -d "$(DESTDIR)/etc/rc5.d/"
	install -m 0755 extras/wib_init.sh "$(DESTDIR)/etc/rc5.d/S99wib_init.sh"
	install -d "$(DESTDIR)/etc/wib/"
	install -m 0755 scripts/* "$(DESTDIR)/etc/wib/"


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
	$(CXX) $(DEFINES) -M $(CXXFLAGS) -MT $(<:.cxx=.o) $< > $@
	@sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' -i $@

$(LDEP): build/%.d: src/%.cc src/%.h $(PBGENS)
	@mkdir -p build
	@set -e; rm -f $@
	$(CXX) $(DEFINES) -M $(CXXFLAGS) -MT $(<:.cc=.o) $< > $@
	@sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' -i $@

# these won't exist the first build
-include $(LDEP) $(BDEP)

$(BOBJ): build/%.o: build/%.d $(PBGENS)
	$(CXX) $(CXXFLAGS) -c $(addprefix src/,$(notdir $(<:.d=.cxx))) -o $@

$(LOBJ): build/%.o: build/%.d $(PBGENS)
	$(CXX) $(CXXFLAGS) -c $(addprefix src/,$(notdir $(<:.d=.cc))) -o $@
