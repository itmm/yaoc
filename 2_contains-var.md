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
	std::cout << "target triple = \"" Target_Triple "\"\n\n";

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
			BEGIN, END, MODULE, PROCEDURE, RETURN, VAR
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
	{ "RETURN", Token::Kind::RETURN },
	{ "VAR", Token::Kind::VAR }
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
			name_ { name }, parent_ { parent }
	       	{ }
	public:
		virtual ~Declaration() { }
		auto name() const { return name_; }
		auto parent() const { return parent_; };
		virtual std::string mangle(std::string name);
		virtual bool has(std::string name) { return false; }
		virtual Declaration::Ptr lookup(std::string name);
		virtual void insert(Declaration::Ptr decl);
};
```
`decl.cpp`

```c++
#include "decl.h"

#include "err.h"

std::string Declaration::mangle(std::string name) {
	auto result { name_ + "_" + name };
	return parent_ ? parent_->mangle(result) : result;
}

Declaration::Ptr Declaration::lookup(std::string name) {
	throw Error { "cannot lookup " + name };
}

void Declaration::insert(Declaration::Ptr decl) {
	throw Error { "cannot insert " + (decl ? decl->name() : std::string { "NIL" }) };
}
```
`type.h`

```c++
#pragma once

#include "decl.h"
#include "lex.h"

class Type: public Declaration {
		std::string ir_name_;
		Type(
			std::string name, Declaration::Ptr parent,
		       	std::string ir_name
		):
			Declaration { name, parent }, ir_name_ { ir_name }
		{ }
	public:
		using Ptr = std::shared_ptr<Type>;
		static Ptr create(
			std::string name, Declaration::Ptr parent,
		       	std::string ir_name
		);
		static Ptr parse(Lexer &l, Declaration::Ptr scope);
		auto ir_name() const { return ir_name_; }
};
```

`type.cpp`

```c++
#include "type.h"

#include "err.h"


Type::Ptr Type::create(
	std::string name, Declaration::Ptr parent, std::string ir_name
) {
	if (! parent) { throw Error { "no parent for type" }; }
	auto result { Ptr { new Type { name, parent, ir_name } } };
	parent->insert(result);
	return result;
}

Type::Ptr Type::parse(Lexer &l, Declaration::Ptr scope) {
	if (! scope) { throw Error { "no scope" }; }
	auto name { l.representation() };
	if (l.is(Token::Kind::identifier)) {
		l.advance();
		auto type { std::dynamic_pointer_cast<Type>(scope->lookup(name)) };
		if (type) { return type; }
	}
	throw Error { "no type " + name };
}
```

`var.h`

```c++
#pragma once

#include "decl.h"
#include "type.h"

#include <vector>

class Variable: public Declaration {
		Type::Ptr type_;
		bool exported_;
		bool var_;
		Variable(
			std::string name, Declaration::Ptr parent,
		       	Type::Ptr type, bool exported, bool var
		): Declaration { name, parent }, type_ { type }, exported_ { exported }, var_ { var } { }
	public:
		static Ptr create(
			std::string name, Declaration::Ptr parent,
			Type::Ptr type, bool exported, bool var
		);
};
```

`var.cpp`

```c++
#include "var.h"

#include "err.h"

Variable::Ptr Variable::create(
	std::string name, Declaration::Ptr parent, Type::Ptr type,
	bool exported, bool var
) {
	if (! parent) { throw Error { "no parent for variable" }; }
	auto result { Ptr { new Variable { name, parent, type, exported, var } } };
	parent->insert(result);
	return result;
}
```

`proc.h`

```c++
#pragma once

#include "decl.h"
#include "lex.h"
#include "mod.h"
#include "type.h"

#include <iostream>

class Procedure: public Declaration {
	public:
		using Ptr = std::shared_ptr<Procedure>;
	private:
		Procedure(std::string name, Type::Ptr return_type, Declaration::Ptr parent):
			Declaration { name, parent }
		{
			std::cout << "define " << (return_type ? return_type->ir_name() : "void") << " @" <<
					parent->mangle(name) << "() {\n"
				"entry:\n";
	       	}

		static Procedure::Ptr create(
			std::string name, Type::Ptr return_type,
		       	Declaration::Ptr parent
		);
		static void parse_statements(Lexer &l, Procedure::Ptr p);
	public:
		static Procedure::Ptr parse(Lexer &l, Declaration::Ptr parent);
		static Procedure::Ptr parse_init(Lexer &l, Declaration::Ptr parent);
};
```

`proc.cpp`

```c++
#include "proc.h"

Procedure::Ptr Procedure::create(
	std::string name, Type::Ptr return_type, Declaration::Ptr parent
) {
	if (! parent) { throw Error { "no parent for procedure" }; }
	auto result { Ptr { new Procedure { name, return_type, parent } } };
	parent->insert(result);
	return result;
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

Procedure::Ptr Procedure::parse(Lexer &l, Declaration::Ptr parent) {
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
	Type::Ptr return_type;
	if (l.is(Token::Kind::colon)) {
		l.advance();
		return_type = Type::parse(l, parent);
	} else { return_type = nullptr; }
	auto proc { create(procedure_name, return_type, parent) };
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

Procedure::Ptr Procedure::parse_init(Lexer &l, Declaration::Ptr parent) {
	auto proc { create("_init", nullptr, parent) };
	parse_statements(l, proc);
	return proc;
}
```

`scope.h`

```c++
#pragma once

#include "decl.h"

#include <map>

class Scoping: public Declaration {
		std::map<std::string, Declaration::Ptr> entries_;
	protected:
		Scoping(std::string name, Declaration::Ptr parent):
			Declaration { name, parent }
	       	{ }
	public:
		Declaration::Ptr lookup(std::string name) override;
		void insert(Declaration::Ptr decl) override;
};
```

`scope.cpp`

```c++
#include "scope.h"

#include "err.h"

Declaration::Ptr Scoping::lookup(std::string name) {
	auto got { entries_.find(name) };
	if (got != entries_.end()) { return got->second; }
	if (parent()) { return (parent())->lookup(name); }
	return nullptr;
}

void Scoping::insert(Declaration::Ptr decl) {
	auto res { entries_.insert({ decl->name(), decl }) };
	if (! res.second) {
		throw Error { "Element " + decl->name() + " inserted twice" };
	}
}
```

`mod.h`

```c++
#pragma once

#include "scope.h"
#include "lex.h"

class Module: public Scoping {
	private:
		Module(std::string name, Declaration::Ptr parent): Scoping { name, parent } { }
	public:
		using Ptr = std::shared_ptr<Module>;
		static Module::Ptr create(std::string name, Declaration::Ptr parent);
		static Module::Ptr parse(Lexer &l, Declaration::Ptr parent);
		std::string mangle(std::string name) override;
};

```

`mod.cpp`

```c++
#include "mod.h"

#include "proc.h"

Module::Ptr Module::create(std::string name, Declaration::Ptr parent) {
	auto result { Ptr { new Module { name, parent } } };
	if (parent) { parent->insert(result); }
	return result;
}

Module::Ptr Module::parse(Lexer &l, Declaration::Ptr parent) {
	l.consume(Token::Kind::MODULE);
	l.expect(Token::Kind::identifier);
	auto module_name { l.representation() };
	l.advance();
	l.consume(Token::Kind::semicolon);
	auto mod { create(module_name, parent) };
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

std::string Module::mangle(std::string name) {
	return this->name() + "_" + name;
}

```

`sys.h`

```c++
#pragma once

#include "mod.h"

Module::Ptr create_SYSTEM();
```

`sys.cpp`

```c++
#include "sys.h"

#include "type.h"

Module::Ptr create_SYSTEM() {
	auto sys { Module::create("SYSTEM", nullptr) };
	Type::create("INTEGER", sys, "i32");
	return sys;
}
```

`yaoc.cpp`

```c++
#include "lex.h"
#include "mod.h"
#include "sys.h"
// ...
	// write expected output
	auto SYSTEM { create_SYSTEM() };
	Lexer lx;
	Module::parse(lx, SYSTEM);
	#if 0
	// ...
		"}\n";
	#endif
// ...
```
