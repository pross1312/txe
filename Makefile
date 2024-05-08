SRC = source/*
txe: $(SRC)
	g++  $(SRC) -o txe -L./raylib-5.0_linux_amd64/lib -I./raylib-5.0_linux_amd64/include -lraylib -lm -DUSE_SDF_FONT
