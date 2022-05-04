#line 411 "1_fn-gen.md"
#include "proc.h"

Procedure::Ptr Procedure::create(
	std::string name, std::string return_type, Module::Ptr mod
) {
	return Ptr { new Procedure { name, return_type, mod } };
}

void Procedure::parse_statements(Lexer &l, Procedure::Ptr p) {
	if (l.is(Token::Kind::BEGIN)) {
		l.advance();
		// statement sequence
	}
	if (l.is(Token::Kind::RETURN)) {
		l.advance();
		if (l.is(Token::Kind::integer_number)) {
			std::cout << "\tret i32 " << l.int_value() << "\n";
			l.advance();
		} else { std::cout << "\tret void\n"; }
	} else { std::cout << "\tret void\n"; }
	std::cout << "}\n\n";
}

Procedure::Ptr Procedure::parse(Lexer &l, Module::Ptr mod) {
	l.consume(Token::Kind::PROCEDURE);
	l.expect(Token::Kind::identifier);
	auto procedure_name { l.representation() };
	l.advance();
	if (l.is(Token::Kind::asterisk)) {
		l.advance();
	}
	if (l.is(Token::Kind::left_parenthesis)) {
		l.advance();
		// parameter list
		l.consume(Token::Kind::right_parenthesis);
	}
	std::string return_type;
	if (l.is(Token::Kind::colon)) {
		l.advance();
		l.expect(Token::Kind::identifier);
		if (l.representation() == "INTEGER") {
			return_type = "i32";
	       	} else { throw Error { "unknown return type " + l.representation() }; }
		l.advance();
	} else { return_type = "void"; }
	auto proc { create(procedure_name, return_type, mod) };
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

Procedure::Ptr Procedure::parse_init(Lexer &l, Module::Ptr mod) {
	auto proc { create("_init", "void", mod) };
	parse_statements(l, proc);
	return proc;
}
