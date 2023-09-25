CPP = g++

TESTS_SRC := $(wildcard tests/*.cpp)

# get my-lib:
# https://github.com/ehmcruz/my-lib
MYLIB = ../my-lib

CPPFLAGS = -std=c++23 -Wall -g -I$(MYLIB)/include -I./include
LDFLAGS = -std=c++23

# ----------------------------------

ifdef MYGLIB_TARGET_LINUX
	CPPFLAGS += -DMYGLIB_TARGET_LINUX=1
	LDFLAGS += -lm

	ifdef MYGLIB_SUPPORT_SDL
		CPPFLAGS += -DMYGLIB_SUPPORT_SDL=1 `pkg-config --cflags sdl2 SDL2_mixer`
		LDFLAGS += `pkg-config --libs sdl2 SDL2_mixer`
	endif
endif

# ----------------------------------

# need to add a rule for each .o/.cpp at the bottom
MYLIB_OBJS = ext/math.o ext/memory.o

SRCS := $(wildcard src/*.cpp)

ifdef MYGLIB_SUPPORT_SDL
	SRCS += $(wildcard src/sdl/*.cpp)
endif

OBJS := $(patsubst %.cpp,%.o,$(SRCS)) $(MYLIB_OBJS)

TESTS_OBJS := $(patsubst %.cpp,%.o,$(TESTS_SRC))

TESTS_BIN := $(patsubst %.cpp,%.exe,$(TESTS_SRC))

HEADERS = $(wildcard include/my-game-lib/*.h) $(wildcard include/my-game-lib/sdl/*.h) $(wildcard $(MYLIB)/include/my-lib/*.h)

# ----------------------------------

%.o: %.cpp $(HEADERS)
	$(CPP) $(CPPFLAGS) -c -o $@ $<

all: $(TESTS_BIN)
	@echo "Everything compiled! yes!"

tests/play-audio.exe: $(OBJS) $(TESTS_OBJS)
	$(CPP) -o tests/play-audio.exe tests/play-audio.o $(OBJS) $(LDFLAGS)

# ----------------------------------

ext/math.o: $(MYLIB)/src/math.cpp $(HEADERS)
	mkdir -p ext
	$(CPP) $(CPPFLAGS) -c -o ext/math.o $(MYLIB)/src/math.cpp

ext/memory.o: $(MYLIB)/src/memory.cpp $(HEADERS)
	mkdir -p ext
	$(CPP) $(CPPFLAGS) -c -o ext/memory.o $(MYLIB)/src/memory.cpp

# ----------------------------------

clean:
	- rm -rf $(BIN) $(OBJS) $(TESTS_OBJS) $(TESTS_BIN)