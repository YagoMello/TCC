.PHONY: debug release clean

debug:
	+$(MAKE) -C build debug

debug-plus:
	+$(MAKE) -C build debug-iterations

release:
	+$(MAKE) -C build release

clean:
	+$(MAKE) -C build clean

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
