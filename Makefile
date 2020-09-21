COMMON_OBJ_FILES=src/common/TOP_Data.o
GREEDY_OBJ_FILES=src/greedy/TOP_Greedy.o
BT_OBJ_FILES=src/backTracking/TOP_Backtracking.o
LS_OBJ_FILES=src/localSearch/TOP_Helpers.o src/localSearch/TOP_Costs.o src/localSearch/Moves/Swap.o

include LibMakefile

CPPFLAGS_JSON=-I$(NLOHMANNJSON)/include
LDFLAGS_JSON=

CPPFLAGS_CTPL=-I$(CTPL)/include
LDFLAGS_CTPL=-pthread

CPPFLAGS_HTTP=-I$(LIBHTTPSERVER)/include
LDFLAGS_HTTP=-L$(LIBHTTPSERVER)/lib -lhttpserver

CPPFLAGS_EASYLOCAL=-I$(EASYLOCAL)/include
LDFLAGS_EASYLOCAL=-lboost_program_options -pthread

LINUX_LD_PATH=$(LIBHTTPSERVER)/lib

CPPFLAGS=-std=c++17 -O3 -Wall -Wno-unknown-pragmas -Wno-sign-compare
LDFLAGS=

ALL_EXE = MainWeb.exe MainParamGr.exe MainMapGr.exe MainGreedy.exe MainBackTracking.exe MainLocal.exe MainLocalSearch.exe ParamBisectionTest.exe Parallel.exe

all: $(ALL_EXE)

### Set the dependences ###

MainWeb.exe: CPPFLAGS+=$(CPPFLAGS_HTTP) $(CPPFLAGS_JSON) $(CPPFLAGS_EASYLOCAL)
MainWeb.exe: LDFLAGS+=$(LDFLAGS_HTTP) $(LDFLAGS_JSON) $(LDFLAGS_EASYLOCAL)

MainLocal.exe: CPPFLAGS+=$(CPPFLAGS_EASYLOCAL)
MainLocal.exe: LDFLAGS+=$(LDFLAGS_EASYLOCAL)

MainLocalSearch.exe: CPPFLAGS+=$(CPPFLAGS_EASYLOCAL)
MainLocalSearch.exe: LDFLAGS+=$(LDFLAGS_EASYLOCAL)

ParamBisectionTest.exe: CPPFLAGS+=$(CPPFLAGS_CTPL)
ParamBisectionTest.exe: LDFLAGS+=$(LDFLAGS_CTPL)

Parallel.exe: CPPFLAGS+=$(CPPFLAGS_CTPL) $(CPPFLAGS_EASYLOCAL) $(CPPFLAGS_JSON)
Parallel.exe: LDFLAGS+=$(LDFLAGS_CTPL) $(LDFLAGS_EASYLOCAL) $(LDFLAGS_JSON)

# WebViewer #
MainWeb.exe: src/MainWeb.o src/web/SolverLocal.o $(GREEDY_OBJ_FILES) $(BT_OBJ_FILES) $(LS_OBJ_FILES) $(COMMON_OBJ_FILES)
# Parameter Analysis #
MainParamGr.exe: src/MainParamGr.o $(GREEDY_OBJ_FILES) $(COMMON_OBJ_FILES)
# Map Analysis #
MainMapGr.exe: src/MainMapGr.o $(GREEDY_OBJ_FILES) $(COMMON_OBJ_FILES)
# Greedy Solver #
MainGreedy.exe: src/MainGreedy.o $(GREEDY_OBJ_FILES) $(COMMON_OBJ_FILES)
# Backtracking Solver #
MainBackTracking.exe: src/MainBackTracking.o $(BT_OBJ_FILES) $(COMMON_OBJ_FILES)
# Loacl Search Single Solver #
MainLocal.exe: src/MainLocal.o $(LS_OBJ_FILES) $(COMMON_OBJ_FILES)
# Loacl Search Multi Solver #
MainLocalSearch.exe: src/MainLocalSearch.o $(LS_OBJ_FILES) $(COMMON_OBJ_FILES)
# Test C param bisection in Greedy #
ParamBisectionTest.exe: src/ParamBisectionTest.o $(GREEDY_OBJ_FILES) $(COMMON_OBJ_FILES)
# ??? #
Parallel.exe: src/Parallel.o src/web/SolverLocal.o $(GREEDY_OBJ_FILES) $(BT_OBJ_FILES) $(LS_OBJ_FILES) $(COMMON_OBJ_FILES)

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

.PHONY: clean runWebLinux buildWeb buildWebWatch

include $(wildcard src/*.d)
