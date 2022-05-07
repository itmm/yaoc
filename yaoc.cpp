#line 937 "1_fn-gen.md"
#include "lex.h"
#include "mod.h"
#include "sys.h"
#line 55
#include <iostream>

int main() {
	std::cout << "target triple = \"" Target_Triple "\"\n\n";

	// write expected output
#line 942
	auto SYSTEM { get_SYSTEM() };
	Lexer lx;
	Module::parse(lx, SYSTEM);
#line 70
}
