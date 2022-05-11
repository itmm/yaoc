#line 498 "1_fn-gen.md"
#include "qual.h"
#include "mod.h"

Declaration::Ptr parse_qualified_ident(Lexer &l, Declaration::Ptr scope) {
	if (! scope) { err(quote(scope), "scope in parse_qualified"); }
	l.expect(Token::Kind::identifier);
	auto name { l.representation() };
	l.advance();
	auto got { scope->lookup(name) };
	if (! got) { err(quote(name), " not found"); }
	if (auto mod { std::dynamic_pointer_cast<Module>(got) }) {
		if (l.is(Token::Kind::period)) {
			l.advance();
			l.expect(Token::Kind::identifier);
			auto name { l.representation() };
			l.advance();
			auto got { mod->lookup(name) };
			if (! got) {
				err(quote(name), " not found in ", quote(mod));
			}
			return got;
		}
	}
	return got;
}
