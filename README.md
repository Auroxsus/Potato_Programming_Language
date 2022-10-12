# Potato_Programming_Language

### File Decriptions

**PotatoReader.exe** is the current verison of the "driver" program file as a combination of PotatoReader.cpp + Potato.h. It's a single-pass compiler (language translator) that reads source code line-by-line from a source file and processes a .list file.

**Potato1.p** is a program that we can "compile" to test our "driver" program. The source file is a text file that contains program components written in a specific programming language, ours being the POTATO programming language and ending with the extention _.p_.

**Potato1.list** is the current output of our "driver". The .list file is a text file that contains a recapitualation of the source code interspersed with translation artifiacts that are specific to the compilier. 

**PotatoScanner.exe** is the current verison of the "lexical analysis" program file as a combination of PotatoScanner.cpp + Potato.h. It's built on the PotatoReader.cpp as a basis and also processes a .list file. The purpose is for it to:
1. “Eat” white-space characters.
2. “Eat” comments. 
3. Recognize the POTATO token types
4. Provide the capability to look-ahead “future” pairs of token-and-lexeme to aid subsequent parsing (syntax analysis).

**Potato2.p** is a program that we can "compile" to test our "lexical analysis" program. The source file is a text file that contains at least one of each POTATO token and at least one instance of eat POTATO comment syntax to fully **white-box test** the scanner logic it uses to recognise eat POTATO pseudo-terminal. By design, this exersizes every flow-of-control path of the source code.

**Potato2.list** is the current output of our "lexical analysis". 
