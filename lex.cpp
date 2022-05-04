#line 148 "1_fn-gen.md"
#include "lex.h"

#include "err.h"

#include <cstring>
#include <iostream>
#include <limits>
#include <map>

Token Lexer::next() {
	auto ch = std::cin.get();
	while (ch != EOF && ch <= ' ') { ch = std::cin.get(); }
	if (ch == EOF) { return Token { Token::Kind::end_of_input, "" }; }
	if (std::isalpha(ch)) { return read_identifier_or_keyword(ch); }
	else if (std::isdigit(ch)) { return read_number(ch); }
	else switch (ch) {
		case '(': return Token { Token::Kind::left_parenthesis, "(" };
		case ')': return Token { Token::Kind::right_parenthesis, ")" };
		case '*': return Token { Token::Kind::asterisk, "*" };
		case '.': return Token { Token::Kind::period, "." };
		case ':': return Token { Token::Kind::colon, ":" };
		case ';': return Token { Token::Kind::semicolon, ";" };
		default: throw Error {
			 std::string { "unknown char '" } +
			 static_cast<char>(ch) + "'"
		};
	}
}
#line 180
Token Lexer::read_number(int ch) {
	std::string rep { }; int value { 0 };
	for (; ch != EOF && std::isdigit(ch); ch = std::cin.get()) {
		rep += static_cast<char>(ch);
		int digit { ch - '0' };
		int limit { std::numeric_limits<int>::max() - digit };
		int carry { limit % 10 };
		if (value + carry > limit/10) {
			throw Error { "INTEGER too big: " + rep };
		}
		value = value * 10 + digit;
	}
	if (ch != EOF) { std::cin.putback(ch); }
	return Token { Token::Kind::integer_number, rep, value };
}
#line 199
static std::map<std::string, Token::Kind> keywords {
	{ "BEGIN", Token::Kind::BEGIN },
	{ "END", Token::Kind::END },
	{ "MODULE", Token::Kind::MODULE },
	{ "PROCEDURE", Token::Kind::PROCEDURE },
	{ "RETURN", Token::Kind::RETURN }
};

Token Lexer::read_identifier_or_keyword(int ch) {
	std::string rep { };
	for (; ch != EOF && std::isalnum(ch); ch = std::cin.get()) {
		rep += static_cast<char>(ch);
	}
	if (ch != EOF) { std::cin.putback(ch); }
	auto got { keywords.find(rep) };
	return Token {
		got != keywords.end() ? got->second : Token::Kind::identifier,
		rep
	};
}

