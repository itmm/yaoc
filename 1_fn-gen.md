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


## Test-Szenario

Die Erwartung ist, dass der IR-Code die Funktion `FnTest_Answer` exportiert,
die von folgendem Programm `t_fn-test.cpp` aufgerufen werden kann:

```c++
#include <iostream>

extern "C" int FnTest__init();
extern "C" int FnTest_Answer();

int main(int argc, const char *argv[]) {
	FnTest__init();
	constexpr int expected { 42 };
	int got { FnTest_Answer() };
	if (got == expected) {
		std::cout << argv[0] << ": ok\n";
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


## Schneller Mock

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


## Target-Triple

Die `target`-Zeile ist etwas komisch.
Eigentlich sollte der generierte Code plattform-neutral sein.
LLVM erwartet jedoch trotzdem eine Plattform-Angabe.
Dies ist das `Target_Triple`.

Ermittelt werden kann dies mit dem Shell-Aufruf

```
llvm-config --host-target
```

Das mit diesem Projekt abgelegte [Makefile](./Makefile) extrahiert die
Ausgabe in ein Makro, das dem Compiler beim Bauen mitgegeben wird.
So kann auf Low-Level Code im eigentlichen Compiler verzichtet werden.
Der generierte Code ist für die gleiche Plattform, für die der Compiler
gebaut wurde. Zusätzlich erledigt das [Makefile](./Makefile) auch das
Bauen und Ausführen der Test-Anwendungen.


## Fehler-Behandlung

Die Fehler-Behandlung findet rudimentär in `err.h` statt.
Es gibt einen Exception, die den aktuellen Fehler beschreibt.
Das Programm wird beim ersten Fehler beenndet.

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

## Tokenizer

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

#include "err.h"

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
		Token(Kind kind, std::string representation, int int_value = 0);
		auto kind() const { return kind_; }
		auto representation() const { return representation_; }
		auto int_value() const { return int_value_; }
};

inline Token::Token(Kind kind, std::string representation, int int_value):
	kind_ { kind }, representation_ { representation },
	int_value_ { int_value }
{
	if (int_value_ && kind_ != Kind::integer_number) {
		throw Error { "invalid INTEGER value" };
	}
}
```

## Lexer

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

Beim Parsen von Zahlen wird neben der Repräsentation gleich die Zahl mit
aufgebaut.

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

Da ein Zeichen zuviel gelesen wurde, wird das letzte Zeichen in den
Eingabe-Strom zurückgeschoben.

Die Schlüsselwörter werden in einer eigenen Tabelle abgelegt:

```c++
// ...
static std::map<std::string, Token::Kind> keywords {
	{ "BEGIN", Token::Kind::BEGIN },
	{ "END", Token::Kind::END },
	{ "MODULE", Token::Kind::MODULE },
	{ "PROCEDURE", Token::Kind::PROCEDURE },
	{ "RETURN", Token::Kind::RETURN }
};
```

Wenn ein Bezeichner gelesen wurde, wird geprüft, ob es sich vielleicht um
ein Schlüsselwort handelt:

```c++
// ...
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

Auch hier muss das letzte Zeichen zurückgeschoben werden.

In der Header-Datei `lex.h` werden zusätzlich Zugriffs-Methoden auf das
enthaltene Token geliefert.
Dies vereinfacht den Parser.

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

# Deklarationen

Jetzt betrachten wir Deklaration in `decl.h`. Eine Deklaration hat zwei
Eigenschaften:

1. Sie hat einen Namen
2. Sie kann Kind von anderen Deklarationen sein.

Dabei kann nicht jede Deklaration Kinder haben. Daher wird dies in der
hier betrachteten Basis-Klasse nicht implementiert.
Wenn versucht wird, darauf zuzugreifen, werden Fehler generiert

```c++
#pragma once

#include <memory>
#include <string>

class Declaration {
		std::string name_;
		std::weak_ptr<Declaration> parent_;
	public:
		using Ptr = std::shared_ptr<Declaration>;
	protected:
		Declaration(std::string name, Declaration::Ptr parent):
			name_ { name }, parent_ { parent }
	       	{ }
	public:
		virtual ~Declaration() { }
		auto name() const { return name_; }
		static std::string name(Declaration::Ptr d);
		Declaration::Ptr parent() const { return parent_.lock(); };
		virtual std::string mangle(std::string name);
		virtual bool has(std::string name) { return false; }
		virtual Declaration::Ptr lookup(std::string name);
		virtual void insert(Declaration::Ptr decl);
};
```

Die statische `name()` Methode behandelt den Fall, dass der Zeiger ein
`nullptr` ist.

```c++
// ...
inline std::string Declaration::name(Declaration::Ptr d) {
	return d ? d->name() : "NIL";
}

```

In `decl.cpp` wird neben der Fehler-Generierung auch das Name-Mangling
implementiert. Um den Namen eines Kindes zu ermitteln, wird der eigene Name
mit einem Unterstrich vorangestellt.
Sollte diese Deklaration wieder ein Kind einer anderen Deklaration sein,
wird dieses Schema rekursiv fortgesetzt.
Dies kann zum Beispiel bei Prozeduren geschehen, die weitere Prozeduren
enthalten dürfen,

```c++
#include "decl.h"

#include "err.h"

std::string Declaration::mangle(std::string name) {
	auto result { name_ + "_" + name };
	auto p { parent() };
	return p ? p->mangle(result) : result;
}

Declaration::Ptr Declaration::lookup(std::string name) {
	throw Error { "cannot lookup " + name };
}

void Declaration::insert(Declaration::Ptr decl) {
	throw Error { "cannot insert " + Declaration::name(decl) };
}
```

Es gibt viele Arten von Deklarationen.
Alleine in diesem Abschnitt werden wir auf folgende treffen:

* `Module`
* `Procedure`
* `Type`

Dabei haben `Module` und `Procedure` die Möglichkeit, Kinder zu haben.
Dieser Aspekt wird in einer eigenen Subklasse gekapselt.

Vorgestellt werden die Deklarationen in umgekehrter Reihenfolge.
Dies liegt daran, dass `Procedure` `Type` benötigt (für den Rückgabewert)
und `Module` `Procedure`n enthält.

Vorher gibt es noch eine Funktion in `qual.h`, um qualifizierte Identifier zu
parsen:

```c++
#pragma once

#include "decl.h"
#include "lex.h"

Declaration::Ptr parse_qualified_ident(Lexer &l, Declaration::Ptr scope);
```

Die Idee ist, dass es sich hierbei um einen Bezeichner handelt, der ein Kind
von `scope` ist.
Wenn dies ein Modul ist und ein `.` folgt, wird statt dessen die Deklaration
zum Bezeichner in diesem Modul zurückgeliefert.
Dies ist in `qual.cpp` umgesetzt:

```c++
#include "qual.h"
#include "mod.h"

Declaration::Ptr parse_qualified_ident(Lexer &l, Declaration::Ptr scope) {
	if (! scope) { throw Error { "NIL scope in parse_qualified" }; }
	l.expect(Token::Kind::identifier);
	auto name { l.representation() };
	l.advance();
	auto got { scope->lookup(name) };
	if (! got) { throw Error { name + " not found" }; }
	if (auto mod { std::dynamic_pointer_cast<Module>(got) }) {
		if (l.is(Token::Kind::period)) {
			l.advance();
			l.expect(Token::Kind::identifier);
			auto name { l.representation() };
			l.advance();
			auto got { mod->lookup(name) };
			if (! got) {
				throw Error {
				       	name + " not found in " + mod->name()
			       	};
			}
			return got;
		}
	}
	return got;
}
```



## Typen

Beginnen wir mit `Type` in `type.h`: Die Klasse hat neben dem Namen
eine IR-Repräsentation:

```c++
#pragma once

#include "decl.h"
#include "lex.h"

class Type: public Declaration {
		std::string ir_representation_;
		Type(
			std::string name, Declaration::Ptr parent,
		       	std::string ir_representation
		):
			Declaration { name, parent },
			ir_representation_ { ir_representation }
		{ }
	public:
		using Ptr = std::shared_ptr<Type>;
		static Ptr create(
			std::string name, Declaration::Ptr parent,
		       	std::string ir_representation
		);
		static Ptr parse(Lexer &l, Declaration::Ptr scope);
		auto ir_representation() const { return ir_representation_; }
		static std::string ir_representation(Type::Ptr type);
};
```

Wie bei der `Declaration` gibt es für die `ir_representation` eine statische
Hilfsmethode, die den `nullptr`-Fall abfängt:

```c++
// ...
inline std::string Type::ir_representation(Type::Ptr type) {
	return type ? type->ir_representation() : "void";
}
```

Momentan müssen wir für das Beispiel nur den Typ `INTEGER` (mit
Repräsentation `i32`) unterstützen.
In späteren Abschnitten werden wir diese Klasse gehörig erweitern müssen.

Die Implementierung in `type.cpp` erzeugt die Instanzen.
Beim Parsen eines Typs wird auf die (noch nicht implementierten) Methoden
zum Suchen eines Bezeichners zurückgegriffen.

```c++
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
```

Mit dem `parse_qualified_ident` ist es möglich, den Modul-Namen bei Bezeichnern
mit anzugeben.  So kann anstatt des Bezeichners `INTEGER` auch
`SYSTEM.INTEGER` verwendet werden.

Dies sehen wir im Modul `FnTest2.mod`, das ebenfalls kompiliert und
ausgeführt werden soll:


```Modula-2
MODULE FnTest;
	PROCEDURE Answer*(): SYSTEM.INTEGER;
	BEGIN
		RETURN 42
	END Answer;
END FnTest.
```
## Deklarationen mit Kindern

Bevor wir Prozeduren und Module parsen, kommt in `scope.h` ein Sub-Typ
von Declaration, um eine beliebige Anzahl von Kindern zu verwalten:

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

In `scope.cpp` steht die passende Umsetzung.
Interessant ist nur, dass bei `lookup` im `parent` weitergesucht wird,
wenn kein Eintrag in der aktuellen Instanz gefunden wurde.
Auch ist es nicht erlaubt, in den gleichen `scope` mehrere Deklarationen
mit dem gleichen Namen einzufügen.

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

## Prozeduren

Nach dieser ganzen Vorarbeit sollte es endlich möglich sein, Prozeduren in
`proc.h` zu definieren und zu parsen.
Jede Prozedur hat einen Rückgabe-Typ und einen Indikator, ob sie exportiert
werden soll oder nicht.

```c++
#pragma once

#include "lex.h"
#include "mod.h"
#include "scope.h"
#include "type.h"

#include <iostream>

class Procedure: public Scoping {
	public:
		using Ptr = std::shared_ptr<Procedure>;
	private:
		bool exported_;
		Type::Ptr return_type_;

		Procedure(
			std::string name, bool exported,
		       	Type::Ptr return_type, Declaration::Ptr parent
		);
		static void parse_statements(Lexer &l, Procedure::Ptr p);
	protected:
		static Procedure::Ptr create(
			std::string name, bool exported, Type::Ptr return_type,
		       	Declaration::Ptr parent
		);
	public:
		static Procedure::Ptr parse(Lexer &l, Declaration::Ptr parent);
		static Procedure::Ptr parse_init(
			Lexer &l, Declaration::Ptr parent
		);
		auto exported() const { return exported_; }
		auto return_type() const { return return_type_; }
};
```

`proc.cpp`

```c++
#include "proc.h"

Procedure::Ptr Procedure::create(
	std::string name, bool exported, Type::Ptr return_type,
       	Declaration::Ptr parent
) {
	if (! parent) { throw Error { "no parent for procedure" }; }
	auto result { Ptr { new Procedure {
		name, exported, return_type, parent
       	} } };
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
	auto module { std::dynamic_pointer_cast<Module>(parent) };
	l.consume(Token::Kind::PROCEDURE);
	l.expect(Token::Kind::identifier);
	auto procedure_name { l.representation() };
	l.advance();
	bool exported { false };
	if (l.is(Token::Kind::asterisk)) {
		if (module) {
			exported = true;
		} else { throw Error { "cannot export " + procedure_name }; }
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
	auto proc { create(procedure_name, exported, return_type, parent) };
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
	auto proc { create("_init", true, nullptr, parent) };
	parse_statements(l, proc);
	return proc;
}
```

```c++
// ...
Procedure::Procedure(
	std::string name, bool exported,
	Type::Ptr return_type, Declaration::Ptr parent
):
	Scoping { name, parent }, exported_ { exported },
	return_type_ { return_type }
{
	if (! exported_) {
	       	throw Error { "only exported PROCEDUREs are supported yet" };
       	}
	std::cout << "define " << Type::ir_representation(return_type) <<
			" @" << parent->mangle(name) << "() {\n"
		"entry:\n";
}
```


## Module

`mod.h`

```c++
#pragma once

#include "scope.h"
#include "lex.h"

class Module: public Scoping {
	private:
		Module(std::string name, Declaration::Ptr parent):
			Scoping { name, parent }
	       	{ }
	public:
		using Ptr = std::shared_ptr<Module>;
		static Module::Ptr create(
			std::string name, Declaration::Ptr parent
		);
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

## Das `SYSTEM`-Modul

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
	static Module::Ptr sys;
	if (sys) { return sys; }
	sys = Module::create("SYSTEM", nullptr);
	Type::create("INTEGER", sys, "i32");
	sys->insert(sys);
	return sys;
}
```


# Alles zusammensetzen

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
