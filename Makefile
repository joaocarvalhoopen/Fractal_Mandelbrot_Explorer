# List of files that make the project.
OBJS = mandelbrot.cpp

# Exectuable name
OBJ_NAME = mandelbrot

# Compilation target
all : $(OBJS)
	g++ $(OBJS) -O2 -IC:\SDL2\SDL2-devel-2.0.10-mingw\SDL2-2.0.10\x86_64-w64-mingw32\include -LC:\SDL2\SDL2-devel-2.0.10-mingw\SDL2-2.0.10\x86_64-w64-mingw32\lib -w -Wl,-subsystem,windows -lmingw32 -lSDL2main -lSDL2 -o $(OBJ_NAME)
