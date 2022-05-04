#line 377 "1_fn-gen.md"
#pragma once

#include "decl.h"
#include "lex.h"
#include "mod.h"

#include <iostream>

class Procedure: public Declaration {
	public:
		using Ptr = std::shared_ptr<Procedure>;
	private:
		Procedure(std::string name, std::string return_type, Module::Ptr module):
			Declaration { name, module }
		{
			std::cout << "define " << return_type << " @" <<
					module->mangle(name) << "() {\n"
				"entry:\n";
	       	}

		static Procedure::Ptr create(
			std::string name, std::string return_type,
		       	Module::Ptr mod
		);
		static void parse_statements(Lexer &l, Procedure::Ptr p);
	public:
		static Procedure::Ptr parse(Lexer &l, Module::Ptr mod);
		static Procedure::Ptr parse_init(Lexer &l, Module::Ptr mod);
};
