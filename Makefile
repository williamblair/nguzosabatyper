CC = mingw32-g++
TARGET = main
CFLAGS = -g 
LIBDIRS = 
INCDIRS = -ID:/MinGW/include -I.
LIBS = -lSDL2 -lSDL2_image -lSDL2_mixer
SOURCES = main.cpp \
          Renderer.cpp \
          Input.cpp \
          Texture.cpp \
          Music.cpp \
          SoundEffect.cpp \
          Font.cpp

all:
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET) $(INCDIRS) $(LIBDIRS) $(LIBS)

