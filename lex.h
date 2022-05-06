#line 200 "1_fn-gen.md"
#pragma once

#include "tok.h"
#line 329
#include "err.h"
#line 203

class Lexer {
		Token tok_;
		const Token &read_number(int ch);
		const Token &read_identifier_or_keyword(int ch);
	public:
#line 334
		auto representation() const { return tok_.representation(); }
		bool is(Token::Kind kind) const { return tok_.kind() == kind; }
		void expect(Token::Kind kind) const {
			if (! is(kind)) {
				throw Error {
					"not expected " + tok_.representation()
				};
			}
		}
		const Token& consume(Token::Kind kind) {
			expect(kind);
			return advance();
		}
		auto int_value() const { return tok_.int_value(); }
#line 209
		const Token &advance();
		Lexer():
			tok_ { Token::Kind::end_of_input, "" }
		{ advance(); }
};
