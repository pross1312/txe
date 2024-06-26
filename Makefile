SRC = source/*.cpp source/*.hpp
DEP = source/*
LIBDIR=./raylib-5.0_linux_amd64/lib
INCDIR=./raylib-5.0_linux_amd64/include
CFLAGS=-Wall -Wextra -Wstrict-overflow=5 -Wswitch -ggdb -DUSE_SDF_FONT -I$(INCDIR)
LIBS=-Wl,-rpath=$(LIBDIR) -L$(LIBDIR) -lraylib -lm
txe: $(DEP)
	g++  $(SRC) $(CFLAGS) $(LIBS)  -o txe
