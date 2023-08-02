
CFLAGS = -O3 -funroll-loops -fPIC
CXXFLAGS = -O3 -funroll-loops -fPIC -Wl,-Bsymbolic

CC = clang
CXX = clang++

PROTODIR = ../../nyx_mode/packer/packer/linux_x86_64-userspace/src/proto/

CXXSOURCES = $(PROTODIR)/afl_input.cpp $(PROTODIR)/input.pb.cc
CXXOBJECTS = afl_input.o input.pb.o

CSOURCES = ../../src/afl-performance.c nyx-net-mutator.c
COBJECTS = nyx-net-mutator.o afl-performance.o

.PHONY: all proto lib clean test

all: proto lib

proto:
	protoc -I$(PROTODIR) input.proto --cpp_out=$(PROTODIR)

lib: proto
	$(CXX) $(CXXFLAGS) -c $(CXXSOURCES) -I$(PROTODIR)
	$(CC) $(CFLAGS) -c $(CSOURCES) -I../../include -I$(PROTODIR)
	$(CXX) $(CXXFLAGS) -I../../include -I. -I$(PROTODIR) -shared -o nyx-net-mutator.so $(COBJECTS) $(CXXOBJECTS) -lprotobuf
	rm $(CXXOBJECTS) $(COBJECTS)

test: proto
	clang++ -o test/test test/test.cpp $(PROTODIR)/input.pb.cc -I$(PROTODIR) -lprotobuf

clean:
	rm -f $(PROTODIR)/input.pb.* *.o *.so
	rm -f test/test.bin test/test