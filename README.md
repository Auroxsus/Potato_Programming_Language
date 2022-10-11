# Potato_Programming_Language
A small esoteric language with limited capabilities, possibilities, &amp; architecture. Includes metalanguage BNF design to specify language syntax, a Scanner, Reader, and Parser 


PotatoReader.exe is the current verison of the "driver" program file combination of PotatoReader.cpp + Potato.h. It's a single-pass compiler (language translator) that reads source code line-by-line from a source file and processes both a .list file and an .object file.

Potato1.p is a program that we can "compile" to test our "driver" program. The source file is a text file that contains program components written in a specific programming language, ours being the POTATO programming language.

Potato1.list is the current output of our "driver". The .list file is a text file that contains a recapitualation of the osurce code interspersed with translation artifiacts that are specific to the compilier. 

Potato1.object is the other output of our "driver". The .object file is often a binary file that contains machine instructions and binary-coded data for a specific computing platform (Windows for us) such that, when executed on this target machine, has that same semantics as (means the same thing as) the original source code.

