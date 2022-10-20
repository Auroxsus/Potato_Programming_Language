# Potato_Programming_Language
Each addition to POTATO requires new scanning logic because new tokens are added, new parsing logic because new rules are added and/or exsisting rules are changed, and newcode generation logic because syntax implies semantics (ALWAYS).

## File Decriptions

Potato.h is the core of our project, the **reader-and-lister** classes 

### Executable Files
**PotatoReader.exe** is a verison of the "driver" program file as a combination of PotatoReader.cpp + Potato.h. It's a single-pass compiler (language translator) that reads source code line-by-line from a source file and processes a .list file. 

**PotatoScanner.exe** is a starter verison of the "lexical analysis" program file as a combination of PotatoScanner.cpp + Potato.h. It's built on the PotatoReader.cpp as a basis and also processes a .list file. The purpose is for it to:
1. “Eat” white-space characters.
2. “Eat” comments. 
3. Recognize the POTATO token types
4. Provide the capability to look-ahead “future” pairs of token-and-lexeme to aid subsequent parsing (syntax analysis).

**PotatoParser.exe** is a beginnning verison of the "parcer" program file as a combination of PotatoParser.cpp + Potato.h. It's built on the PotatoScanner.cpp as a basis and also processes a .list file.

**PotatoCompilier.exe** is the current and continously updated version of the official "compilier" for the POTATO programming language. It is a combination of PotatoComplier.cpp + Potato.h.

### Sample Programs
**Potato1.p** is a source file that we can "compile" to test our "driver" program. The source file is a text file that contains program components written in a specific programming language, ours being the POTATO programming language and ending with the extention _.p_.

**Potato1.list** is the current output of our "driver". The .list file is a text file that contains a recapitualation of the source code interspersed with translation artifiacts that are specific to the compilier.

**Potato2.p** is a source file that we can "compile" to test our "lexical analysis" program. The source file is a text file that contains at least one of each POTATO token and at least one instance of eat POTATO comment syntax to fully **white-box test** the scanner logic it uses to recognise each POTATO pseudo-terminal. By design, this exersizes every flow-of-control path of the source code.

**Potato2.list** is the current output of our "lexical analysis". 

**Potato3.p, Potato4.p, Potato5.p** are a series of source files that we can "compile" to test-and-debug our "parcer" program. It was designed to test the logic the parser uses to recognize **grammatically-correct** POTATO programs.

**Potato3.list, Potato4.list, Potato5.list** are the current output of our "parcer". 
