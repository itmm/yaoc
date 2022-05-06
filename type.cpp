#line 490 "1_fn-gen.md"
#include "type.h"

#include "err.h"

Type::Ptr Type::create(
	std::string name, Declaration::Ptr parent, std::string ir_name
) {
	if (! parent) { throw Error { "no parent for TYPE" }; }
	auto result { Ptr { new Type { name, parent, ir_name } } };
	parent->insert(result);
	return result;
}

Type::Ptr Type::parse(Lexer &l, Declaration::Ptr scope) {
	if (! scope) { throw Error { "no scope for TYPE" }; }
	auto name { l.representation() };
	if (l.is(Token::Kind::identifier)) {
		l.advance();
		auto type { std::dynamic_pointer_cast<Type>(
			scope->lookup(name))
		};
		if (type) { return type; }
	}
	throw Error { "no TYPE " + name };
}
