# Potato_Programming_Language
Ultimatly, there are only slight changes made from the reader to scanner and scanner to parser. Parser to compilier will start off with _light_ changes as well but I hope it will spiral into a complex compiler that will be able to translate the entire POTATO language, or at least as much as I can develop.

As I am continously improving and adding new features, it will be updated in the compiler code. There are no plans to create a sperate updated PotatoXScanner.cpp, PotatoXParcer.cpp, PotatoXCompilier.cpp. The previous versions and executable files will stay in the repository and in the "Previous_Versions" folder for historical purposes. 

These are for **MY** understanding of how my compilier works but to get a 'deeper look' into the logic of each, feel free to browse the attached wiki where I attempt to explain additions and new functions introduced in the source code. 

## File Decriptions
**Potato.h** is the core of our project, the **reader-and-lister** classes and grows along with the compilier.

**PotatoCompiler.cpp** is the source file of our project and is updated as additional features and functions are added to the POTATO programming language.

**PotatoCompilier.exe** is the current version of the official "compilier" for the POTATO programming language. It is a combination of PotatoComplier.cpp + Potato.h

## Language Capabilities
* specify unnamed literal constants with data types integer, boolean, and string (strings are usually used to label output);
* name program variables and modules with identifiers;
* define scalar variables for data types integer and boolean;
* define 1-dimensional array variables for data types integer and boolean;
* express computation using expression syntax that makes use of an “interesting” collection of unary and binary operators for operands with data types integer and boolean;
* specify an assignment-statement;
* specify several “classic” structured flow-of-control statements, including, (1) an if-statement;  (2) a bounded-loop-statement ; and (3) an unbounded pretest- and/or posttest-loop-statement;
* define the main program module;
* specify assertions;
* define directly-recursive procedure subprogram modules with IN, OUT, IO parameters;
* specify a call-procedure-module-statement (with parameter passing);
* specify a return-from-procedure-module-statement;
* define directly-recursive, pure function subprogram modules with IN parameters;
* specify reference-to-function-module syntax as part of expression (with parameter passing);
* specify a return-from-function-module-statement;
* specify an input-statement used for console text input;
* specify an output-statement used for console formatted-text output; and
* include comments.

