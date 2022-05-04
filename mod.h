#line 325 "1_fn-gen.md"
#pragma once

#include "decl.h"
#include "lex.h"

class Module: public Declaration {
	private:
		Module(std::string name): Declaration { name, nullptr } { }
	public:
		using Ptr = std::shared_ptr<Module>;
		static Module::Ptr parse(Lexer &l);
};

