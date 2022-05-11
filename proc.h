#line 703 "1_fn-gen.md"
#pragma once

#include "lex.h"
#include "mod.h"
#include "scope.h"
#include "type.h"

#include <iostream>

class Procedure: public Scoping {
	public:
		using Ptr = std::shared_ptr<Procedure>;
	private:
		bool exported_;
		Type::Ptr return_type_;

		Procedure(
			std::string name, bool exported,
			Type::Ptr return_type, Declaration::Ptr parent
		);
		static void parse_statements(Lexer &l, Procedure::Ptr p);
		static void return_integer_value(Procedure::Ptr p, int value);
	protected:
		static Procedure::Ptr create(
			std::string name, bool exported, Type::Ptr return_type,
			Declaration::Ptr parent
		);
	public:
		static Procedure::Ptr parse(Lexer &l, Declaration::Ptr parent);
		static Procedure::Ptr parse_init(
			Lexer &l, Declaration::Ptr parent
		);
		auto exported() const { return exported_; }
		auto return_type() const { return return_type_; }
};

inline std::string quote(Procedure::Ptr proc) {
	return "PROCEDURE " + quote(Declaration::name(proc));
}
