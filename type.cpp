#line 603 "1_fn-gen.md"
#include "type.h"

#include "err.h"
#include "qual.h"

Type::Ptr Type::create(
	std::string name, Declaration::Ptr parent, std::string ir_name
) {
	return Declaration::insert(
		Ptr { new Type { name, parent, ir_name } }, parent
	);
}

Type::Ptr Type::parse(Lexer &l, Declaration::Ptr scope) {
	auto got  { parse_qualified_ident(l, scope) };
	auto t { std::dynamic_pointer_cast<Type>(got) };
	if (! t) { err(quote(got), " is no TYPE"); }
	return t;
}
