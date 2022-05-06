#line 500 "1_fn-gen.md"
#pragma once

#include "decl.h"
#include "lex.h"

class Type: public Declaration {
		std::string ir_representation_;
		Type(
			std::string name, Declaration::Ptr parent,
			std::string ir_representation
		):
			Declaration { name, parent },
			ir_representation_ { ir_representation }
		{ }
	public:
		using Ptr = std::shared_ptr<Type>;
		static Ptr create(
			std::string name, Declaration::Ptr parent,
			std::string ir_representation
		);
		static Ptr parse(Lexer &l, Declaration::Ptr scope);
		auto ir_representation() const { return ir_representation_; }
		static std::string ir_representation(Type::Ptr type);
};
#line 531
inline std::string Type::ir_representation(Type::Ptr type) {
	return type ? type->ir_representation() : "void";
}
