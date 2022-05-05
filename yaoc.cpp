#line 484 "1_fn-gen.md"
#include "lex.h"
#include "mod.h"
#line 49
#include <iostream>

int main() {
	std::cout << "target triple = \"" Target_Triple "\"\n\n";

	// write expected output
#line 488
	Lexer lx;
	Module::parse(lx);
#line 64
}
