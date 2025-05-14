# To compile
# make MYGLIB_TARGET_LINUX=1 MYGLIB_SUPPORT_SDL=1 MYGLIB_SUPPORT_OPENGL=1
# make MYGLIB_TARGET_WINDOWS=1 MYGLIB_SUPPORT_SDL=1 MYGLIB_SUPPORT_OPENGL=1

CPP = g++-14

TESTS_SRC := $(wildcard tests/*.cpp)

# get my-lib:
# https://github.com/ehmcruz/my-lib
MYLIB = ../my-lib

CPPFLAGS = -std=c++23 -Wall -g -I$(MYLIB)/include -I./include -DMYGLIB_FP_TYPE=float
LDFLAGS = -std=c++23

# ----------------------------------

ifdef MYGLIB_TARGET_LINUX
	CPPFLAGS +=
	LDFLAGS += -lm

	ifdef MYGLIB_SUPPORT_SDL
		CPPFLAGS += -DMYGLIB_SUPPORT_SDL=1 `pkg-config --cflags sdl2 SDL2_mixer SDL2_image`
		LDFLAGS += `pkg-config --libs sdl2 SDL2_mixer SDL2_image`
	endif

	ifdef MYGLIB_SUPPORT_OPENGL
		CPPFLAGS += -DMYGLIB_SUPPORT_OPENGL=1
		LDFLAGS += -lGL -lGLEW
	endif
endif

ifdef MYGLIB_TARGET_WINDOWS
	CPPFLAGS +=
	LDFLAGS += -lm

	ifdef MYGLIB_SUPPORT_SDL
		CPPFLAGS += -DMYGLIB_SUPPORT_SDL=1 `pkg-config --cflags sdl2 SDL2_mixer SDL2_image`
		LDFLAGS += `pkg-config --libs sdl2 SDL2_mixer SDL2_image`
	endif

	ifdef MYGLIB_SUPPORT_OPENGL
		CPPFLAGS += -DMYGLIB_SUPPORT_OPENGL=1
		LDFLAGS += -lglew32 -lopengl32
	endif
endif

# ----------------------------------

# need to add a rule for each .o/.cpp at the bottom
MYLIB_OBJS = #ext/memory.o

SRCS := $(wildcard src/*.cpp)

HEADERS := $(wildcard include/my-game-lib/*.h) $(wildcard $(MYLIB)/include/my-lib/*.h)

ifdef MYGLIB_SUPPORT_SDL
	SRCS += $(wildcard src/sdl/*.cpp)
	HEADERS += $(wildcard include/my-game-lib/sdl/*.h)
endif

ifdef MYGLIB_SUPPORT_OPENGL
	SRCS += $(wildcard src/opengl/*.cpp)
	HEADERS += $(wildcard include/my-game-lib/opengl/*.h)
endif

OBJS := $(patsubst %.cpp,%.o,$(SRCS)) $(MYLIB_OBJS)

TESTS_OBJS := $(patsubst %.cpp,%.o,$(TESTS_SRC))

TESTS_BIN := $(patsubst %.cpp,%.exe,$(TESTS_SRC))

# ----------------------------------

%.o: %.cpp $(HEADERS)
	$(CPP) $(CPPFLAGS) -c -o $@ $<

all: $(TESTS_BIN)
	@echo "Everything compiled! yes!"

tests/test.exe: $(OBJS) $(TESTS_OBJS)
	$(CPP) -o tests/test.exe tests/test.o $(OBJS) $(LDFLAGS)

# ----------------------------------

ext/memory.o: $(MYLIB)/src/memory.cpp $(HEADERS)
	mkdir -p ext
	$(CPP) $(CPPFLAGS) -c -o ext/memory.o $(MYLIB)/src/memory.cpp

# ----------------------------------

clean:
	- rm -rf $(BIN) $(OBJS) $(TESTS_OBJS) $(TESTS_BIN)