#line 132 "1_fn-gen.md"
#pragma once

#include "tok.h"

class Lexer {
		Token read_number(int ch);
		Token read_identifier_or_keyword(int ch);
	public:
		Token next();
};
