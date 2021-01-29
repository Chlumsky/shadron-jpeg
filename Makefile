
all:
	g++ -dynamiclib -std=c++11 -O2 -I. -ljpeg src/*.cpp -o shadron-jpeg.dylib

static:
	g++ -dynamiclib -std=c++11 -O2 -I. /usr/local/lib/libjpeg.a src/*.cpp -o shadron-jpeg.dylib

install:
	mkdir -p ~/.config/Shadron/extensions
	cp -f shadron-jpeg.dylib ~/.config/Shadron/extensions/shadron-jpeg.dylib

clean:
	rm -f shadron-jpeg.dylib
