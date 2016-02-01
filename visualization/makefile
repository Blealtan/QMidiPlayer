CC= g++
CXXFLAGS= -g -std=c++11 -Wall -I/home/chrisoft/devel/BulletLabRemixIII/include/ -I/usr/include/freetype2
LINK= -L/home/chrisoft/devel/BulletLabRemixIII/smelt/sdl/ -L/home/chrisoft/devel/BulletLabRemixIII/extensions/ -lfluidsynth -lstdc++ -pthread -lSDL2 -ljpeg -lpng -lfreetype -lz -lsmeltext -lsmelt-dumb -lCxImage

all: midiplay

midiplay: midiplay.o midiread.o midivisual.o
	$(CC) midiplay.o midiread.o midivisual.o $(LINK) -o midiplay

midiplay.o:
	$(CC) midiplay.cpp -c $(CXXFLAGS)

midiread.o:
	$(CC) midiread.cpp -c $(CXXFLAGS)

midivisual.o:
	$(CC) midivisual.cpp -c $(CXXFLAGS)
clean:
	rm *.o midiplay -f