#line 256 "1_fn-gen.md"
#include "lex.h"

#include "err.h"

#include <cstring>
#include <iostream>
#include <limits>
#include <map>

const Token &Lexer::advance() {
	auto ch = std::cin.get();
	while (ch != EOF && ch <= ' ') { ch = std::cin.get(); }
	if (ch == EOF) { tok_ = Token { Token::Kind::end_of_input, "" }; }
	else if (std::isalpha(ch)) { tok_ = read_identifier_or_keyword(ch); }
	else if (std::isdigit(ch)) { tok_ = read_number(ch); }
	else switch (ch) {
		case '(': 
			tok_ = Token { Token::Kind::left_parenthesis, "(" };
			break;
		case ')':
			tok_ = Token { Token::Kind::right_parenthesis, ")" };
			break;
		case '*':
			tok_ = Token { Token::Kind::asterisk, "*" };
			break;
		case '.':
			tok_ = Token { Token::Kind::period, "." };
			break;
		case ':':
			tok_ = Token { Token::Kind::colon, ":" };
			break;
		case ';':
			tok_ = Token { Token::Kind::semicolon, ";" };
			break;
		default: 
			err("unknown char '", static_cast<char>(ch), "'");
	}
	return tok_;
}
#line 302
const Token &Lexer::read_number(int ch) {
	std::string rep { }; int value { 0 };
	for (; ch != EOF && std::isdigit(ch); ch = std::cin.get()) {
		rep += static_cast<char>(ch);
		int digit { ch - '0' };
		if (value > (std::numeric_limits<int>::max() - digit) / 10) {
			err("INTEGER too big: ", quote(rep));
		}
		value = value * 10 + digit;
	}
	if (ch != EOF) { std::cin.putback(ch); }
	tok_ = Token { Token::Kind::integer_number, rep, value };
	return tok_;
}
#line 325
static std::map<std::string, Token::Kind> keywords {
	{ "BEGIN", Token::Kind::BEGIN },
	{ "END", Token::Kind::END },
	{ "MODULE", Token::Kind::MODULE },
	{ "PROCEDURE", Token::Kind::PROCEDURE },
	{ "RETURN", Token::Kind::RETURN }
};
#line 339
const Token &Lexer::read_identifier_or_keyword(int ch) {
	std::string rep { };
	for (; ch != EOF && std::isalnum(ch); ch = std::cin.get()) {
		rep += static_cast<char>(ch);
	}
	if (ch != EOF) { std::cin.putback(ch); }
	auto got { keywords.find(rep) };
	tok_ = Token {
		got != keywords.end() ? got->second : Token::Kind::identifier,
		rep
	};
	return tok_;
}
