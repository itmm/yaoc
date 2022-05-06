#line 23 "1_fn-gen.md"
#include <iostream>

extern "C" int FnTest__init();
extern "C" int FnTest_Answer();

int main(int argc, const char *argv[]) {
	FnTest__init();
	constexpr int expected { 42 };
	int got { FnTest_Answer() };
	if (got == expected) {
		std::cout << argv[0] << ": ok\n";
		return EXIT_SUCCESS;
	} else {
		std::cerr << "fn-test: FAILED: " <<
			got << " != " << expected << "\n";
		return EXIT_FAILURE;
	}
}
