COMMON_OBJ_FILES=src/common/TOP_Data.o

include LibMakefile

CPPFLAGS_JSON=-I$(NLOHMANNJSON)/include
LDFLAGS_JSON=

CPPFLAGS_HTTP=-I$(LIBHTTPSERVER)/include
LDFLAGS_HTTP+=-L$(LIBHTTPSERVER)/lib -lhttpserver

LINUX_LD_PATH=$(LIBHTTPSERVER)/lib

CPPFLAGS=-std=c++17 -O3 -Wall -Wno-unknown-pragmas -Wno-sign-compare
LDFLAGS=

ALL_EXE=MainWeb.exe MainTest.exe MainSolver.exe MainMapper.exe MainBT.exe

all: $(ALL_EXE)

### Imposta le dipendenze ###

MainWeb.exe: CPPFLAGS+=$(CPPFLAGS_HTTP) $(CPPFLAGS_JSON)
MainWeb.exe: LDFLAGS+=$(LDFLAGS_HTTP) $(LDFLAGS_JSON)

# WebViewer #
MainWeb.exe: src/MainWeb.o src/greedy/Kevin.o $(COMMON_OBJ_FILES)

# Parameter Tester #
MainTest.exe: src/MainTest.o src/greedy/Kevin.o $(COMMON_OBJ_FILES)

# Greedy Solver #
MainSolver.exe: src/MainSolver.o src/greedy/Kevin.o $(COMMON_OBJ_FILES)

# maxdeviation Scaler #
MainMapper.exe: src/MainMapper.o src/greedy/Kevin.o $(COMMON_OBJ_FILES)

MainBT.exe: src/MainBT.o src/backTracking/TOP_Backtracking.o $(COMMON_OBJ_FILES)

%.o: %.cpp
	g++ $(CPPFLAGS) -c -o $@ $< -MD

%.exe:
	g++ -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(ALL_EXE)
	find ./src -type f -name "*.o" -delete
	find ./src -type f -name "*.d" -delete

runWebLinux: MainWeb.exe
	LD_LIBRARY_PATH=$(LINUX_LD_PATH) ./MainWeb.exe

buildWeb:
	cd webSrc && yarn run tsc
buildWebWatch:
	cd webSrc && yarn run tsc --watch

.PHONY: clean runLinux buildWeb buildWebWatch

include $(wildcard src/*.d)
