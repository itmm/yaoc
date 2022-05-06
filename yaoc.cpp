#line 679 "1_fn-gen.md"
#include "lex.h"
#include "mod.h"
#include "sys.h"
#line 49
#include <iostream>

int main() {
	std::cout << "target triple = \"" Target_Triple "\"\n\n";

	// write expected output
#line 684
	auto SYSTEM { create_SYSTEM() };
	Lexer lx;
	Module::parse(lx, SYSTEM);
#line 64
}
