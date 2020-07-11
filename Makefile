LIBHTTPSERVER=/home/dario/Test/libhttpserver/build/install
NLOHMANNJSON=/home/dario/Test/json

OBJ_FILES=src/TOP_Data.o src/Solvers/Kevin.o
CPPFLAGS=-std=c++17 -O3 -I$(LIBHTTPSERVER)/include -I$(NLOHMANNJSON)/include
LDFLAGS=-L$(LIBHTTPSERVER)/lib  -lhttpserver

include LibMakefile

Main.exe: src/Main.o $(OBJ_FILES)
	g++ -o $@ $^ $(LDFLAGS)

src/TOP_Data.o: src/TOP_Data.hpp
src/Main.o: src/TOP_Data.hpp src/Utils.hpp src/Solvers/Solver.hpp src/Solvers/Kevin.hpp
src/Solvers/Kevin.o: src/TOP_Data.hpp src/Utils.hpp

clean:
	rm -f Main.exe src/Main.o $(OBJ_FILES)

runLinux: Main.exe
	LD_LIBRARY_PATH=$(LINUX_LD_PATH) ./Main.exe

buildWeb:
	cd webSrc && yarn run tsc
buildWebWatch:
	cd webSrc && yarn run tsc --watch

.PHONY: clean runLinux buildWeb buildWebWatch
