#line 545 "1_fn-gen.md"
#include "type.h"

#include "err.h"
#include "qual.h"

Type::Ptr Type::create(
	std::string name, Declaration::Ptr parent, std::string ir_name
) {
	if (! parent) { throw Error { "no parent for TYPE" }; }
	auto result { Ptr { new Type { name, parent, ir_name } } };
	parent->insert(result);
	return result;
}

Type::Ptr Type::parse(Lexer &l, Declaration::Ptr scope) {
	auto got  { parse_qualified_ident(l, scope) };
	auto t { std::dynamic_pointer_cast<Type>(got) };
	if (! t) { throw Error { Declaration::name(got) + " is no TYPE" }; }
	return t;
}
