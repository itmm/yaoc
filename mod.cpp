#line 341 "1_fn-gen.md"
#include "mod.h"

#include "proc.h"

Module::Ptr Module::parse(Lexer &l) {
	l.consume(Token::Kind::MODULE);
	l.expect(Token::Kind::identifier);
	auto module_name { l.representation() };
	l.advance();
	l.consume(Token::Kind::semicolon);
	auto mod = Ptr { new Module(module_name) };
	// imports
	// types
	// consts
	// vars
	while (l.is(Token::Kind::PROCEDURE)) {
		Procedure::parse(l, mod);
	}
	Procedure::parse_init(l, mod);
	l.consume(Token::Kind::END);
	l.expect(Token::Kind::identifier);
	if (l.representation() != module_name) {
		throw Error {
			"module name " + module_name +
			" does not match END " + l.representation()
	       	};
	}
	l.advance();
	l.consume(Token::Kind::period);
	return mod;
}
