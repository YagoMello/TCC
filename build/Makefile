.PHONY: debug release clean debug-iterations
#MAKEFLAGS += -j$(NPROCS)
MAKEFLAGS += -j11

CXXFLAGS = -Wall -Wextra -Wshadow -Wundef -Wconversion -std=c++20 -pipe -pthread
DEBUG = -Og -ggdb3 -fsignaling-nans -D DEBUG -D_FORTIFY_SOURCE=2 -fsanitize=address -fsanitize=undefined -D DEBUG_SUPERNODES
RELEASE = -Ofast -D NDEBUG -flto=11 -fuse-linker-plugin
LIBS = -pthread -I/usr/include/boost
CXX = g++

PROG = ../src/program
SIM = ../src/simulator
COMP = ../src/components

FILES = $(wildcard $(PROG)/*) $(wildcard $(SIM)/*) $(wildcard $(COMP)/*) $(wildcard $(COMP)/*/*)
SRCS = $(filter %.c %.cpp, $(FILES))
OBJECTS = $(SRCS:.cpp=.o)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(LIBS) -c $<

debug: CXXFLAGS += $(DEBUG)
debug: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LIBS) $(notdir $^) -o ../bin/$@
	rm -f $(notdir $^)

debug-iterations: CXXFLAGS += $(DEBUG) -D DEBUG_ITERATIONS -D DEBUG_TS
debug-iterations: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LIBS) $(notdir $^) -o ../bin/$@
	rm -f $(notdir $^)

release: CXXFLAGS += $(RELEASE)
release: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LIBS) $(notdir $^) -o ../bin/$@
	rm -f $(notdir $^)

clean:
	rm -rf ../bin/* ./*.o

#debug:
#	g++ $(FLAGS) $(LIBS) $(DEBUG) $(FILES) -o bin/basic
#	g++ $(FLAGS) $(LIBS) $(DEBUG) $(FILES) -D DEBUG_ITERATIONS -o bin/basic-di
# 	g++ $(FLAGS) $(LIBS) $(DEBUG) -march=native $(FILES) -o bin/native
# 	g++ $(FLAGS) $(LIBS) $(DEBUG) -flto $(FILES) -o bin/lto
# 	g++ $(FLAGS) $(LIBS) $(DEBUG) -flto -fuse-linker-plugin $(FILES) -o bin/ltoplugin
# 	g++ $(FLAGS) $(LIBS) $(DEBUG) -fwhole-program $(FILES) -o bin/whole
#release:
#	g++ $(FLAGS) $(LIBS) $(RELEASE) $(FILES) -o bin/basic
#	g++ $(FLAGS) $(LIBS) $(RELEASE) -march=native $(FILES) -o bin/native
#	g++ $(FLAGS) $(LIBS) $(RELEASE) -flto $(FILES) -o bin/lto
#	g++ $(FLAGS) $(LIBS) $(RELEASE) -flto -fuse-linker-plugin $(FILES) -o bin/ltoplugin
# 	g++ $(FLAGS) $(LIBS) $(RELEASE) -fwhole-program $(FILES) -o bin/whole
#	g++ $(FLAGS) $(LIBS) $(RELEASE) -march=native -flto -fuse-linker-plugin $(FILES) -o bin/ltoplugin
