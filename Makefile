CPP = g++

TESTS_SRC := $(wildcard tests/*.cpp)

# get my-lib:
# https://github.com/ehmcruz/my-lib
MYLIB = ../my-lib

CPPFLAGS = -std=c++23 -Wall -g -I$(MYLIB)/include -I./include
LDFLAGS = -std=c++23

# ----------------------------------

ifdef MYGLIB_TARGET_LINUX
	BIN=$(BIN_LINUX)
	CPPFLAGS += -DMYGLIB_TARGET_LINUX=1
	LDFLAGS += -lm

	ifdef MYGLIB_SUPPORT_SDL
		CPPFLAGS += -DMYGLIB_SUPPORT_SDL=1 `sdl2-config --cflags`
		LDFLAGS += `sdl2-config --libs`
	endif
endif

# ----------------------------------

# need to add a rule for each .o/.cpp at the bottom
MYLIB_OBJS = ext/math.o

SRCS := $(wildcard src/*.cpp)

ifdef MYGLIB_SUPPORT_SDL
	SRCS += $(wildcard src/sdl/*.cpp)
endif

OBJS := $(patsubst %.cpp,%.o,$(SRCS)) $(MYLIB_OBJS)

TESTS_BIN := $(patsubst %.cpp,%.exe,$(TESTS_SRC))

HEADERS = $(wildcard src/*.h) $(wildcard src/graphics/*.h) $(wildcard $(MYLIB)/include/my-lib/*.h)

# ----------------------------------

%.o: %.cpp $(HEADERS)
	$(CPP) $(CPPFLAGS) -c -o $@ $<

all: $(TESTS_BIN)
	@echo "Everything compiled! yes!"

play-audio.exe: $(OBJS) tests/play-audio.cpp
	$(CPP) -o play-audio.exe tests/play-audio.cpp $(OBJS) $(LDFLAGS)

# ----------------------------------

ext/math.o: $(MYLIB)/src/math.cpp $(HEADERS)
	mkdir -p ext
	$(CPP) $(CPPFLAGS) -c -o ext/math.o $(MYLIB)/src/math.cpp

# ----------------------------------

clean:
	- rm -rf $(BIN) $(OBJS)