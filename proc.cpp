#line 707 "1_fn-gen.md"
#include "proc.h"

Procedure::Ptr Procedure::create(
	std::string name, bool exported, Type::Ptr return_type,
       	Declaration::Ptr parent
) {
	return Declaration::insert(
		Ptr { new Procedure { name, exported, return_type, parent } },
		parent
	);
}

Procedure::Procedure(
	std::string name, bool exported,
	Type::Ptr return_type, Declaration::Ptr parent
):
	Scoping { name, parent }, exported_ { exported },
	return_type_ { return_type }
{
	if (! exported_) {
		throw Error { "only exported PROCEDUREs are supported yet" };
	}
	std::cout << "define " << Type::ir_representation(return_type) <<
			" @" << parent->mangle(name) << "() {\n"
		"entry:\n";
}
#line 737
Procedure::Ptr Procedure::parse(Lexer &l, Declaration::Ptr parent) {
	auto module { std::dynamic_pointer_cast<Module>(parent) };
	l.consume(Token::Kind::PROCEDURE);
	l.expect(Token::Kind::identifier);
	auto procedure_name { l.representation() };
	l.advance();
	bool exported { false };
	if (l.is(Token::Kind::asterisk)) {
		if (module) {
			exported = true;
		} else { throw Error { "cannot export " + procedure_name }; }
		l.advance();
	}
	if (l.is(Token::Kind::left_parenthesis)) {
		l.advance();
		// parameter list
		l.consume(Token::Kind::right_parenthesis);
	}
	Type::Ptr return_type;
	if (l.is(Token::Kind::colon)) {
		l.advance();
		return_type = Type::parse(l, parent);
	} else { return_type = nullptr; }
	auto proc { create(procedure_name, exported, return_type, parent) };
	l.consume(Token::Kind::semicolon);
	parse_statements(l, proc);
	l.consume(Token::Kind::END);
	l.expect(Token::Kind::identifier);
	if (l.representation() != procedure_name) {
		throw Error {
			"procedure name " + procedure_name +
			" does not match END " + l.representation()
		};
	};
	l.advance();
	l.consume(Token::Kind::semicolon);
	return proc;
}
#line 781
void Procedure::parse_statements(Lexer &l, Procedure::Ptr p) {
	if (! p) { throw Error { "no PROCEDURE" }; }
	if (l.is(Token::Kind::BEGIN)) {
		l.advance();
		// statement sequence
	}
	if (l.is(Token::Kind::RETURN)) {
		l.advance();
		if (l.is(Token::Kind::integer_number)) {
			return_integer_value(p, l.int_value());
			l.advance();
			return;
		}
	}
	if (p->return_type()) {
		throw Error { "PROCEDURE needs RETURN with value" };
	}
	std::cout << "\tret void\n}\n\n";
}
#line 806
void Procedure::return_integer_value(Procedure::Ptr p, int value) {
	if (! p) { throw Error { "no PROCEDURE" }; }
	auto rep { Type::ir_representation(p->return_type()) };
	if (rep != "i32") {
		throw Error { "PROCEDURE has wrong RETURN TYPE" };
	}
	std::cout << "\tret " + rep + " " << value << "\n}\n\n";
}
#line 821
Procedure::Ptr Procedure::parse_init(Lexer &l, Declaration::Ptr parent) {
	auto proc { create("_init", true, nullptr, parent) };
	parse_statements(l, proc);
	return proc;
}
