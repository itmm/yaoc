#line 608 "1_fn-gen.md"
#include "mod.h"

#include "proc.h"

Module::Ptr Module::create(std::string name, Declaration::Ptr parent) {
	auto result { Ptr { new Module { name, parent } } };
	if (parent) { parent->insert(result); }
	return result;
}

Module::Ptr Module::parse(Lexer &l, Declaration::Ptr parent) {
	l.consume(Token::Kind::MODULE);
	l.expect(Token::Kind::identifier);
	auto module_name { l.representation() };
	l.advance();
	l.consume(Token::Kind::semicolon);
	auto mod { create(module_name, parent) };
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

std::string Module::mangle(std::string name) {
	return this->name() + "_" + name;
}

