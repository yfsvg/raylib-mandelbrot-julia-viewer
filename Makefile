COMPILER = clang++
SOURCE_LIBS = -Ilib/ -I$(shell brew --prefix gmp)/include
OSX_OPT = -Llib/ -L$(shell brew --prefix gmp)/lib -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL lib/libraylib.a -lgmp -lgmpxx
OSX_OUT = -o "bin/build_osx"
CFILES = src/*.cpp
CFLAGS = -std=c++11

build_osx:
	$(COMPILER) $(CFILES) $(CFLAGS) $(SOURCE_LIBS) $(OSX_OUT) $(OSX_OPT)