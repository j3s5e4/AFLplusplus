
CFLAGS = -O3 -funroll-loops -fPIC -Wl,-Bsymbolic

all: proto nyx-net-mutator.so

# Generate protobuf files with `protoc input.proto --cpp_out=.`
proto: input.proto input.pb.cc

nyx-net-mutator.so:	nyx-net-mutator.c
	$(CC) $(CFLAGS) -I../../include -I. -shared -o nyx-net-mutator.so nyx-net-mutator.c  ../../src/afl-performance.c

clean:
	rm -f *.o *~ *.so core
