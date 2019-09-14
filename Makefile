# Notes:
#      To compile for release with MINGW32 do "mingw32-make".
#      To compile for debug with MINGW32 do "mingw32-make TARGET=debug".

# Release version.
CFLAGS = -O2 -Wall

# To compile for debug call with "mingw32-make TARGET=debug" 
ifeq "$(strip $(filter debug,$(TARGET)))" "debug"
# debug
CFLAGS = -g -Wall
endif

## To compile for release call with "mingw32.make TARGET=release" 
#ifeq "$(strip $(filter release,$(TARGET)))" "release"
## Release version.
#CFLAGS = -O2 -Wall
#endif

CC = g++

SRCS = mandelbrot.cpp
PROG = mandelbrot.exe

SDLROOT = C:\SDL2\SDL2-devel-2.0.10-mingw\SDL2-2.0.10\x86_64-w64-mingw32\\

SDL2 = -I"$(SDLROOT)include" -L"$(SDLROOT)lib" -w -Wl,-subsystem,windows -lmingw32 -lSDL2main -lSDL2

$(PROG) : $(SRCS)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) $(SDL2)


clean: 
	del *.o mandelbrot.exe

