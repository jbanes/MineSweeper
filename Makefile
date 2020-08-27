TARGET = minesweeper.dge
OBJS = control.o \
display.o \
fixmath.o  \
graphics.o \
main.o \
timer.o

ARCH=mipsel-linux-
CC=$(ARCH)gcc
STRIP=$(ARCH)strip

SDL_CFLAGS  := $(shell sdl-config --cflags)
SDL_LIBS    := $(shell sdl-config --libs)

CFLAGS = $(SDL_CFLAGS) 
LDFLAGS = $(SDL_LIBS)


.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.cpp.o:
	$(CXX) $(CFLAGS) -c $< -o $@

all: $(OBJS)
	$(CC) $(LDFLAGS) $(CFLAGS) $(OBJS) $(LIBS) -o $(TARGET) && $(STRIP) $(TARGET)
	mv $(TARGET) opk/
	mkdir -p dist
	mksquashfs opk/ dist/MineSweeper.opk
	rm opk/$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET) MineSweeper.opk
	rm -Rf dist
