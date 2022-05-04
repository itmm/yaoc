.PHONY: tests real_tests

tests:
	mdp README.md
	$(MAKE) real_tests

yaoc: yaoc.cpp

real_tests: t_fn-test
	./t_fn-test

t_fn-test: yaoc t_fn-test.cpp FnTest.mod
	cat FnTest.mod | ./yaoc > FnTest.ll
	clang++ t_fn-test.cpp FnTest.ll -o $@

