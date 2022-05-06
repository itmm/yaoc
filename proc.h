#line 395 "1_fn-gen.md"
#pragma once

#include "decl.h"
#include "lex.h"
#include "mod.h"
#include "type.h"

#include <iostream>

class Procedure: public Declaration {
	public:
		using Ptr = std::shared_ptr<Procedure>;
	private:
		Procedure(std::string name, Type::Ptr return_type, Declaration::Ptr parent):
			Declaration { name, parent }
		{
			std::cout << "define " << (return_type ? return_type->ir_name() : "void") << " @" <<
					parent->mangle(name) << "() {\n"
				"entry:\n";
	       	}

		static Procedure::Ptr create(
			std::string name, Type::Ptr return_type,
		       	Declaration::Ptr parent
		);
		static void parse_statements(Lexer &l, Procedure::Ptr p);
	public:
		static Procedure::Ptr parse(Lexer &l, Declaration::Ptr parent);
		static Procedure::Ptr parse_init(Lexer &l, Declaration::Ptr parent);
};
