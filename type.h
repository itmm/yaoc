#line 340 "1_fn-gen.md"
#pragma once

#include "decl.h"
#include "lex.h"

class Type: public Declaration {
		std::string ir_name_;
		Type(
			std::string name, Declaration::Ptr parent,
		       	std::string ir_name
		):
			Declaration { name, parent }, ir_name_ { ir_name }
		{ }
	public:
		using Ptr = std::shared_ptr<Type>;
		static Ptr create(
			std::string name, Declaration::Ptr parent,
		       	std::string ir_name
		);
		static Ptr parse(Lexer &l, Declaration::Ptr scope);
		auto ir_name() const { return ir_name_; }
};
