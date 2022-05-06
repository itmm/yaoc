.PHONY: tests real_tests clean mdp_clean lines

tests:
	mdp README.md
	$(MAKE) real_tests

include $(wildcard deps/*.dep)

Triple := $(shell llvm-config --host-target)
CXXFLAGS += -g -Wall -std=c++17 -DTarget_Triple="\"${Triple}\""

build/%.o: %.cpp
	mkdir -p build deps
	clang++ $(CXXFLAGS) -c $(notdir $(@:.o=.cpp)) -o $@ -MMD -MF deps/$(notdir $(@:.o=.dep))

CPPs := yaoc.cpp lex.cpp mod.cpp proc.cpp decl.cpp type.cpp sys.cpp scope.cpp
OBJs := $(addprefix build/,$(CPPs:.cpp=.o))

yaoc: $(OBJs)
	clang++ $(CXXFLAGS) -o $@ $(OBJs)

real_tests: t_fn-test
	./t_fn-test

t_fn-test: yaoc t_fn-test.cpp FnTest.mod
	cat FnTest.mod | ./yaoc > FnTest.ll
	clang++ $(CXXFLAGS) t_fn-test.cpp FnTest.ll -o $@

clean:
	rm -Rf yaoc t_fn-test build deps

mdp_clean: clean
	rm *.cpp *.h *.mod *.ll

lines:
	cat $(wildcard *.cpp) $(wildcard *.h) $(wildcard *.mod) Makefile | wc -l
