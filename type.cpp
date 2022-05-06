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
	for (;;) {
		if (! scope) { throw Error { "no scope for TYPE" }; }
		auto name { l.representation() };
		if (l.is(Token::Kind::identifier)) {
			l.advance();
			auto got { scope->lookup(name) };
			if (auto type { 
				std::dynamic_pointer_cast<Type>(got) 
			}) {
				return type;
			};
			l.consume(Token::Kind::period);
			scope = got;
			continue;
		}
		throw Error { "no TYPE " + name };
	}
}
