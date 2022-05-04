# Eine Funktion generieren und aufrufen

Ziel dieses Abschnittes ist es, eine einfache Funktion zu erzeugen, die
von einem C++-Programm aus aufgerufen werden kann. Konkret soll das folgende
Modul `FnTest.mod` kompiliert werden:

```oberon
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

extern "C" int FnTest_Answer();

int main() {
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

