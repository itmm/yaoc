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

