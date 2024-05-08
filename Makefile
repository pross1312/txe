SRC = source/*
CFLAGS=-ggdb -lraylib -lm -DUSE_SDF_FONT
LIBDIR=/home/dvtuong/softwares/raylib-5.0/raylib
INCDIR=./raylib-5.0_linux_amd64/include
txe: $(SRC)
	g++  $(SRC) -o txe -L$(LIBDIR) -I$(INCDIR) $(CFLAGS)
