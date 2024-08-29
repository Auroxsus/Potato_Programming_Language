# Potato_Programming_Language

>[!CAUTION]
>### ***[UPDATE: August 20th, 2024]***
>After a year away from this project, I am returning with renewed enthusiasm and fresh inspiration. I will be reviewing and updating the archived files to align with new syntax and design choices. The original archived files will be preserved, and a new folder will be created for the updated versions. Please note that no additional functionality will be added until I have caught up with the current progress.

---

<img src="https://github.com/user-attachments/assets/654f1cae-8cf5-4b56-a642-1269b11243c7" align="left" width="300" height=auto/>

Potato Chip, also known as PotatoC or Potato Programming Language, is inspired by the code structure of C and the syntax of Python. While the core syntax remains familiar, Potato Chip introduces the thematic twist of potato puns.

**Development Phases:**

- **Reader to Scanner:** Minor adjustments are made to adapt the syntax of PotatoC from the C language.
- **Scanner to Parser:** Similar incremental changes are implemented to refine the parsing process.
- **Parser to Compiler:** Initially, the changes from parser to compiler are minimal, but the goal is to evolve the compiler into a sophisticated tool capable of translating the complete PotatoC language. This evolution will be a gradual process, with complexity increasing over time.

**Version Management:**

- As new features and improvements are added, they will be incorporated into the compiler code.
- Previous versions of the compiler, including PotatoXScanner.cpp, PotatoXParser.cpp, and PotatoXCompiler.cpp, will remain in the repository under the "Previous_Versions" folder for reference and historical purposes.

**Documentation:**

- For a detailed understanding of the compiler’s logic and the latest updates, please refer to the attached wiki. The wiki provides explanations of recent additions and functions introduced in the source code.

---

## File Descriptions
- **Potato.h:** This header file contains the core definitions and declarations for our project. It includes the **Reader** and **Lister** classes and evolves alongside the compiler.
- **PotatoCompiler.cpp:** This source file serves as the primary implementation for the project. It is updated with each addition of new features and functions to Potato Chip.
- **PotatoCompiler.exe:** This executable represents the current version of the Potato Chip Compiler. It is created by compiling **PotatoCompiler.cpp** together with **Potato.h**.

---

## Language Capabilities
### 1. **Basic Data Types and Constants**
   - Specify unnamed literal constants with data types integer, boolean, and string.
### 2. **Identifiers and Variable Declarations**
   - Name program variables and modules with identifiers.
   - Define scalar variables for data types integer and boolean.
   - Define 1-dimensional array variables for data types integer and boolean.
### 3. **Expressions and Computation**
   - Express computation using expression syntax with a collection of unary and binary operators for integer and boolean operands.
   - Specify an assignment statement.
### 4. **Control Flow**
   - Specify structured flow-of-control statements, including:
     - If-statement
     - Bounded-loop-statement
     - Unbounded pretest and/or posttest-loop-statement.
### 5. **Program Structure**
   - Define the main program module.
   - Specify assertions.
   - Include comments.
### 6. **Procedures and Functions**
   - Define directly-recursive procedure subprogram modules with IN, OUT, IO parameters.
   - Specify a call-procedure-module statement with parameter passing.
   - Specify a return-from-procedure-module statement.
   - Define directly-recursive, pure function subprogram modules with IN parameters.
   - Specify reference-to-function-module syntax as part of an expression with parameter passing.
   - Specify a return-from-function-module statement.
### 7. **Input and Output**
   - Specify an input-statement for console text input.
   - Specify an output-statement for console formatted-text output.
