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
		Kind kind_;
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
		Token tok_;
		const Token &read_number(int ch);
		const Token &read_identifier_or_keyword(int ch);
	public:
		const Token &advance();
		Lexer():
			tok_ { Token::Kind::end_of_input, "" }
		{ advance(); }
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
		default: throw Error {
			std::string { "unknown char '" } +
			static_cast<char>(ch) + "'"
		};
	}
	return tok_;
}
```

```c++
// ...
const Token &Lexer::read_number(int ch) {
	std::string rep { }; int value { 0 };
	for (; ch != EOF && std::isdigit(ch); ch = std::cin.get()) {
		rep += static_cast<char>(ch);
		int digit { ch - '0' };
		if (value > (std::numeric_limits<int>::max() - digit) / 10) {
			throw Error { "INTEGER too big: " + rep };
		}
		value = value * 10 + digit;
	}
	if (ch != EOF) { std::cin.putback(ch); }
	tok_ = Token { Token::Kind::integer_number, rep, value };
	return tok_;
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
		const char *what() const noexcept override {
			return what_.c_str();
		}
};

```

`lex.h`

```c++
// ...
#include "tok.h"
#include "err.h"
// ...
class Lexer {
		// ...
	public:
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
		// ...
};
```

`decl.h`

```c++
#pragma once

#include <memory>
#include <string>

class Declaration {
	public:
		using Ptr = std::shared_ptr<Declaration>;
	private:
		std::string name_;
		Declaration::Ptr parent_;

	protected:
		Declaration(std::string name, Declaration::Ptr parent):
			name_ { name }, parent_ { parent } { }
	public:
		virtual ~Declaration() { }
		auto parent() const { return parent_; };
		std::string mangle(std::string name);
};
```
`decl.cpp`

```c++
#include "decl.h"

std::string Declaration::mangle(std::string name) {
	auto result { name_ + "_" + name };
	return parent_ ? parent_->mangle(result) : result;
}
```

`mod.h`

```c++
#pragma once

#include "decl.h"
#include "lex.h"

class Module: public Declaration {
	private:
		Module(std::string name): Declaration { name, nullptr } { }
	public:
		using Ptr = std::shared_ptr<Module>;
		static Module::Ptr parse(Lexer &l);
};

```

`mod.cpp`

```c++
#include "mod.h"

#include "proc.h"

Module::Ptr Module::parse(Lexer &l) {
	l.consume(Token::Kind::MODULE);
	l.expect(Token::Kind::identifier);
	auto module_name { l.representation() };
	l.advance();
	l.consume(Token::Kind::semicolon);
	auto mod = Ptr { new Module(module_name) };
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
```

`proc.h`

```c++
#pragma once

#include "decl.h"
#include "lex.h"
#include "mod.h"

#include <iostream>

class Procedure: public Declaration {
	public:
		using Ptr = std::shared_ptr<Procedure>;
	private:
		Procedure(std::string name, std::string return_type, Module::Ptr module):
			Declaration { name, module }
		{
			std::cout << "define " << return_type << " @" <<
					module->mangle(name) << "() {\n"
				"entry:\n";
	       	}

		static Procedure::Ptr create(
			std::string name, std::string return_type,
		       	Module::Ptr mod
		);
		static void parse_statements(Lexer &l, Procedure::Ptr p);
	public:
		static Procedure::Ptr parse(Lexer &l, Module::Ptr mod);
		static Procedure::Ptr parse_init(Lexer &l, Module::Ptr mod);
};
```

`proc.cpp`

```c++
#include "proc.h"

Procedure::Ptr Procedure::create(
	std::string name, std::string return_type, Module::Ptr mod
) {
	return Ptr { new Procedure { name, return_type, mod } };
}

void Procedure::parse_statements(Lexer &l, Procedure::Ptr p) {
	if (l.is(Token::Kind::BEGIN)) {
		l.advance();
		// statement sequence
	}
	if (l.is(Token::Kind::RETURN)) {
		l.advance();
		if (l.is(Token::Kind::integer_number)) {
			std::cout << "\tret i32 " << l.int_value() << "\n";
			l.advance();
		} else { std::cout << "\tret void\n"; }
	} else { std::cout << "\tret void\n"; }
	std::cout << "}\n\n";
}

Procedure::Ptr Procedure::parse(Lexer &l, Module::Ptr mod) {
	l.consume(Token::Kind::PROCEDURE);
	l.expect(Token::Kind::identifier);
	auto procedure_name { l.representation() };
	l.advance();
	if (l.is(Token::Kind::asterisk)) {
		l.advance();
	}
	if (l.is(Token::Kind::left_parenthesis)) {
		l.advance();
		// parameter list
		l.consume(Token::Kind::right_parenthesis);
	}
	std::string return_type;
	if (l.is(Token::Kind::colon)) {
		l.advance();
		l.expect(Token::Kind::identifier);
		if (l.representation() == "INTEGER") {
			return_type = "i32";
	       	} else { throw Error { "unknown return type " + l.representation() }; }
		l.advance();
	} else { return_type = "void"; }
	auto proc { create(procedure_name, return_type, mod) };
	l.consume(Token::Kind::semicolon);
	parse_statements(l, proc);
	l.consume(Token::Kind::END);
	l.expect(Token::Kind::identifier);
	if (l.representation() != procedure_name) {
		throw Error {
			"procedure name " + procedure_name +
			" does not match END " + l.representation()
		};
	};
	l.advance();
	l.consume(Token::Kind::semicolon);
	return proc;
}

Procedure::Ptr Procedure::parse_init(Lexer &l, Module::Ptr mod) {
	auto proc { create("_init", "void", mod) };
	parse_statements(l, proc);
	return proc;
}
```

`yaoc.cpp`

```c++
#include "lex.h"
#include "mod.h"
// ...
	// write expected output
	Lexer lx;
	Module::parse(lx);
	#if 0
	// ...
		"}\n";
	#endif
// ...
```
