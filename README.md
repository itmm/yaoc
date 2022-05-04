# Yet Another Oberon Compiler

Dieses Projekt hat mit dem [LLVM Projekt](https://llvm.org) angefangen.
Ich wollte gerne einen eigenen Compiler damit schreiben.
Die Hoffnung war, dass dieser Compiler dann automagisch für alle möglichen
Rechner-Architekturen ausführbare Dateien erstellen kann.

Ich bin jedoch mit der Bibliothek nicht zurecht gekommen.
Ich habe es in dem von mir gesetzten Zeitrahmen nicht geschafft, eine
`Target_Machine` zu instanziieren, die auf meinem Raspberry-Pi vernünftig
arbeitet.
Naiv wie ich war habe ich gedacht, dass dies eigentlich die Vorgabe sein
sollte.
Aber scheinbar ist es eine Wissenschaft für sich, die notwendigen `Triple`,
`CPU`s, `Target`s, `Features` und sonstige Instanzen zusammenzusuchen, um den
Code für die aktuelle Plattform zu generieren.
Vermutlich habe ich den richtigen Weg einfach noch nicht gefunden.

Daher gehe ich hier erst einmal einen Schritt zurück.
Anstatt direkt den ausführbaren Code zu erzeugen, erzeuge ich hier _nur_
den Assembler-Code in der Intermediate Representation (IR).
Mit Hilfe z.B. vom Clang kann daraus dann der ausführbare Code für die
unterschiedlichsten Plattformen erstellt werden.

Der Vorteil für mich ist, dass wir uns nicht mit der LLVM-Bibliothek
herumärgern müssen. Statt dessen implementiere ich den Compiler zuerst als
ganz einfachen Filter in C++: Der Oberon Source-Code wird über die
Standard-Eingabe gelesen und der IR Assembler-Code über die Standard-Ausgabe
geschrieben.

Der Source-Code wird aus dieser Dokumentation mit dem Tool
[md-patcher](https://github.com/itmm/md-patcher) generiert.
Dies hat den Vorteil, dass diese Dokumentation stets die eine Referenz ist,
aus der alles generiert werden kann. Das mitgelieferter `Makefile` erledigt
das komplette Bauen und Testen des Compilers.

Beginnen wir nun mit dem ersten Schritt: eine Funktion in Oberon definieren und
aus C++ heraus aufrufen. Dies wird in [1_fn-gen.md](./1_fn-gen.md)
beschrieben.

