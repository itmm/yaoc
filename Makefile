.PHONY: tests real_tests clean mdp_clean lines

tests:
	mdp README.md
	$(MAKE) real_tests

include $(wildcard deps/*.dep)

CXXFLAGS += -g -Wall -std=c++17

build/%.o: %.cpp
	mkdir -p build deps
	clang++ $(CXXFLAGS) -c $(notdir $(@:.o=.cpp)) -o $@ -MMD -MF deps/$(notdir $(@:.o=.dep))

yaoc: build/yaoc.o build/lex.o build/mod.o build/proc.o build/decl.o
	clang++ $(CXXFLAGS) -o $@ build/yaoc.o build/lex.o build/mod.o build/proc.o build/decl.o

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
