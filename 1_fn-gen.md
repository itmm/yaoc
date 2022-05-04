# Eine Funktion generieren und aufrufen

Ziel dieses Abschnittes ist es, eine einfache Funktion zu erzeugen, die
von einem C++-Programm aus aufgerufen werden kann. Konkret soll das folgende
Modul `FnTest.mod` kompiliert werden:

```Modula-2
MODULE FnTest;
	PROCEDURE Answer*(): INTEGER;
	BEGIN
		RETURN 42
	END Answer;
END FnTest.
```

Die Erwartung ist, dass der IR-Code die Funktion `FnTest_Answer` exportiert,
die von folgendem Programm `t_fn-test.cpp` aufgerufen werden kann:

```c++
#include <iostream>

extern "C" int FnTest__init();
extern "C" int FnTest_Answer();

int main() {
	FnTest__init();
	constexpr int expected { 42 };
	int got { FnTest_Answer() };
	if (got == expected) {
		std::cout << "fn-test: ok\n";
		return EXIT_SUCCESS;
	} else {
		std::cerr << "fn-test: FAILED: " <<
			got << " != " << expected << "\n";
		return EXIT_FAILURE;
	}
}
```

Die Funktion `FnTest__init()` initialisiert das Modul. Die eigentliche
Name der Funktion wird aus dem Modul-Namen und Funktionsnamen zu
`FnTest_Answer` zusammengesetzt. Da Oberon `_` in Bezeichnern nicht erlaubt
(C/C++ aber schon), kann der Unterstrich als Separator verwendet werden.

Jetzt geht es also um die Implementierung von `yaoc.cpp`. Der erste Wurf
ist in guter TDD-Manier enttäuschend trivial:

```c++
#include <iostream>

int main() {
	// write expected output
	std::cout <<
		"define i32 @FnTest_Answer() {\n"
		"entry:\n"
			"\tret i32 42\n"
		"}\n"
		"define void @FnTest__init() {\n"
		"entry:\n"
			"\tret void\n"
		"}\n";
}
```

Es wird direkt der erwartete IR-Code rausgeschrieben.
Der Vorteil ist jedoch, dass wir damit den Test-Case schon komplett zum Laufen
bringen können:

```
cat FnTest.mod | ./yaoc > FnTest.ll
clang++ t_fn-test.cpp FnTest.ll -o t_fn-test
./t_fn-test
```

Das mit diesem Projekt abgelegte [Makefile](./Makefile) erledigt das Bauen und
Ausführen der Tests automagisch.

Wir können jetzt anfangen die Eingabe zu lesen. Die Eingabe wird in Blöcke
(sogenannte Token) zerlegt. Es gibt folgende Arten von Tokens:

* Schlüsselwörter
  * `BEGIN`, `END`, `MODULE`, `PROCEDURE`, `RETURN`
* Bezeichner
  * Folge aus Buchstaben und Ziffern, die mit einem Buchstaben beginnen und
    kein Schlüsselwort sind
  * zum Beispiel: `Answer`, `FnTest`, `INTEGER`
* Ganzzahlen
  * Folge von Ziffern
  * zum Beispiel: `42`
* Kontrollzeichen
  * `(`, `)`, `*`, `.`, `:`, `;`

Zwischen einzelnen Tokens können Leerzeichen und Zeilenumbrüche stehen. Diese
sind notwendig, um Bezeichner, Zahlen und Schlüsselwörter voneinander zu trennen
(`MODULEFnTest`, `PROCEDUREAnswer`, `BEGINRETURN42` sind gültige Bezeichner).

In der Klasse `Token` in `tok.h` wird neben der Token-Art auch die Zeichen
abgelegt, die das Token repräsentieren:

```c++
#pragma once

#include <string>

class Token {
	public:
		enum class Kind {
			end_of_input, identifier, integer_number,
			left_parenthesis, right_parenthesis, asterisk,
			period, colon, semicolon,
			BEGIN, END, MODULE, PROCEDURE, RETURN
		};
	private:
		const Kind kind_;
		std::string representation_;
		int int_value_;
	public:
		Token(Kind kind, std::string representation, int int_value = 0):
			kind_ { kind }, representation_ { representation },
			int_value_ { int_value }
		{ }
		auto kind() const { return kind_; }
		auto representation() const { return representation_; }
		auto int_value() const { return int_value_; }
};
```

Zusätzlich gibt es einen `Lexer` in `lex.h`, der die Standard-Eingabe
in Token umwandelt:

```c++
#pragma once

#include "tok.h"

class Lexer {
		Token read_number(int ch);
		Token read_identifier_or_keyword(int ch);
	public:
		Token next();
};
```

Die Implementierung in `lex.cpp` ist auch hier zuerst einmal sehr schlicht
gehalten:

```c++
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
```

```c++
// ...
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
```

```c++
// ...
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
```

`err.h`

```c++
#pragma once

#include <stdexcept>
#include <string>

class Error: public std::exception {
		std::string what_;
	public:
		Error(std::string what): what_ { what } { }
		const char *what() const noexcept override { return what_.c_str(); }
};

```
