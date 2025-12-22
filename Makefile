COMPILER = clang++
SOURCE_LIBS = -Ilib/
OSX_OPT = -Llib/ -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL lib/libraylib.a
OSX_OUT = -o "bin/build_osx"
CFILES = src/*.cpp
CFLAGS = -std=c++11

build_osx:
	$(COMPILER) $(CFILES) $(CFLAGS) $(SOURCE_LIBS) $(OSX_OUT) $(OSX_OPT)