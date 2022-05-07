.PHONY: tests real_tests clean mdp_clean lines

tests:
	@echo MDP
	@mdp README.md
	@$(MAKE) --no-print-directory real_tests

include $(wildcard deps/*.dep)

Triple := $(shell llvm-config --host-target)
CXXFLAGS += -g -Wall -std=c++17 -DTarget_Triple="\"${Triple}\""

build/%.o: %.cpp
	@echo C++ $@
	@mkdir -p build deps
	@clang++ $(CXXFLAGS) -c $(notdir $(@:.o=.cpp)) -o $@ -MMD -MF deps/$(notdir $(@:.o=.dep))

CPPs := yaoc.cpp lex.cpp mod.cpp proc.cpp decl.cpp type.cpp sys.cpp scope.cpp qual.cpp
OBJs := $(addprefix build/,$(CPPs:.cpp=.o))

yaoc: $(OBJs)
	@echo LINK $@
	@clang++ $(CXXFLAGS) -o $@ $(OBJs)

real_tests: t_fn-test t_fn-test2
	@./t_fn-test
	@./t_fn-test2


t_fn-test: yaoc t_fn-test.cpp FnTest.mod
	@echo BUILD $@
	@cat FnTest.mod | ./yaoc > FnTest.ll
	@clang++ $(CXXFLAGS) t_fn-test.cpp FnTest.ll -o $@

t_fn-test2: yaoc t_fn-test.cpp FnTest2.mod
	@echo BUILD $@
	@cat FnTest2.mod | ./yaoc > FnTest2.ll
	@clang++ $(CXXFLAGS) t_fn-test.cpp FnTest2.ll -o $@

clean:
	@echo CLEAN
	@rm -Rf yaoc t_fn-test build deps

mdp_clean: clean
	@echo MDP CLEAN
	@rm *.cpp *.h *.mod *.ll

lines:
	@echo LINES
	@cat $(wildcard *.cpp) $(wildcard *.h) $(wildcard *.mod) Makefile | wc -l
