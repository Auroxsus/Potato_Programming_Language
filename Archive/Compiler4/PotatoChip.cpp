//-----------------------------------------------------------
// PotatoChip.cpp by Auroxsus
// French Fry Productions
// Description: POTATO Compiler program
//-----------------------------------------------------------
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>	// Used in Potato.h for time stamp
#include <cctype>
#include <vector>

using namespace std;

// #define TRACEREADER
// #define TRACESCANNER
// #define TRACEPARSER
//#define TRACEIDENTIFIERTABLE
//#define TRACECOMPILER
#include "PotatoChip.h"

//-----------------------------------------------------------
typedef enum
//-----------------------------------------------------------
{
	// pseudo-terminals
	IDENTIFIER,
	INTEGER,	// SPUD
	STRING,		// CHIPPER
	EOPTOKEN,
	UNKTOKEN,
	// reserved words
	CRISP,
	PROGRAM,	// mainCrisp
	PRINT,		// bite
	ENDL,		// endl and \n
	// logical operators
	OR,			// \\ and mix
	NOR,		// ~\ and spudStop
	XOR,		// +\ and eitherSpud
	AND,		// && and prep
	NAND,		// ~& and mashless
	NOT,		// ! and raw
	ABS,
	TRUE,
	FALSE,
	BOOLEAN,	// TATER
	CONSTANT,	// ROOT
	INPUT,		// UNEARTH
	IF,
	ELIF,
	ELSE,
	DO,
	WHILE,
	FOR,
	// punctuation
	COMMA,
	PERIOD,
	SEMICOLON,
	OCBRACKET,
	CCBRACKET,
	OSQRBRACKET,
	CSQRBRACKET,
	OPARENTHESIS,
	CPARENTHESIS,
	CONCAT,
	COLON,
	// relational operators
	LT,			//<and smallerSpud
	LTEQ,		// <= and notBiggerSpud
	EQEQ,		// == and spudMatch
	GT,			// > and biggerSpud
	GTEQ,		// >= and notSmallerSpud
	NOTEQ,		// <>, !=, and mashApart
	// assignment opperators
	EQ,			// = and plant
	EQPLUS,		// += and mashed
	EQMINUS,	// -= and peeled
	EQMULTIPLY,	// *= and fried
	EQDIVIDE,	// /= and sliced
	// arithmetic operators
	PLUS,		// + and mash
	MINUS,		// - and peel
	MULTIPLY,	// *and fry
	DIVIDE,		// / and slice
	MODULUS,	// % and mod
	POWER,		// ^ and pow
	INC,
	DEC
}TOKENTYPE;

//-----------------------------------------------------------
struct TOKENTABLERECORD
//-----------------------------------------------------------
{
	TOKENTYPE type;
	char description[15 + 1];
	bool isReservedWord;
};

//-----------------------------------------------------------
const TOKENTABLERECORD TOKENTABLE[] =
//-----------------------------------------------------------
{
    { IDENTIFIER,     "IDENTIFIER",        false },
    { INTEGER,        "SPUDLING",          true  },
    { STRING,         "CHIPPER",           true  },
    { EOPTOKEN,       "EOPTOKEN",          false },
    { UNKTOKEN,       "UNKTOKEN",          false },
    { CRISP,          "CRISP",             true  },
    { PROGRAM,        "MAINCRISP",         true  },
    { PRINT,          "BITE",              true  },
    { ENDL,           "ENDL",              true  },
    { OR,             "MIX",               true  },
    { NOR,            "SPUDSTOP",          true  },
    { XOR,            "EITHERSPUD",        true  },
    { AND,            "PREP",              true  },
    { NAND,           "MASHLESS",          true  },
    { NOT,            "RAW",               true  },
    { ABS,            "ABS",               true  },
    { TRUE,           "TRUE",              true  },
    { FALSE,          "FALSE",             true  },
    { BOOLEAN,        "TATER",             true  },
    { CONSTANT,       "SPUD",              true  },
    { INPUT,          "UNEARTH",           true  },
    { IF,             "BAKE",              true  },
    { ELIF,           "ELBAKE",            true  },
    { ELSE,           "ELSE",              true  },
    { DO,             "DO",                true  },
    { WHILE,          "FRYASLONGAS",       true  },
    { FOR,            "SEASON",             true  },
    { COMMA,          "COMMA",             false },
    { PERIOD,         "PERIOD",            false },
    { COLON,          "COLON",             false },
    { SEMICOLON,      "SEMICOLON",         false },
    { OCBRACKET,      "OCBRACKET",         false },
    { CCBRACKET,      "CCBRACKET",         false },
    { OSQRBRACKET,    "OSQRBRACKET",       false },
    { CSQRBRACKET,    "CSQRBRACKET",       false },
    { OPARENTHESIS,   "OPARENTHESIS",      false },
    { CPARENTHESIS,   "CPARENTHESIS",      false },
    { LT,             "SMALLERSPUD",       true  },
    { LTEQ,           "NOTBIGGERSPUD",     true  },
    { EQEQ,           "SPUDMATCH",         true  },
    { GT,             "BIGGERSPUD",        true  },
    { GTEQ,           "NOTSMALLERSPUD",    true  },
    { NOTEQ,          "MASHAPART",         true  },
    { EQ,             "PLANT",             true  },
	{ EQPLUS,         "MASHED",            true  },
	{ EQMINUS,        "PEELED",            true  },
	{ EQMULTIPLY,     "FRIED",             true  },
	{ EQDIVIDE,       "SLICED",            true  },
    { PLUS,           "MASH",              true  },
    { MINUS,          "PEEL",              true  },
    { MULTIPLY,       "FRY",               true  },
    { DIVIDE,         "SLICE",             true  },
    { MODULUS,        "MOD",               true  },
    { POWER,          "POW",               true  },
    { CONCAT,         "CONCAT",            false },
    { INC,            "SPROUT",            true  },
    { DEC,            "ROOT",              true  },
};

//-----------------------------------------------------------
struct TOKEN
//-----------------------------------------------------------
{
	TOKENTYPE type;
	char lexeme[SOURCELINELENGTH + 1];
	int sourceLineNumber;
	int sourceLineIndex;
};

//--------------------------------------------------
// Global variables
//--------------------------------------------------
READER<CALLBACKSUSED> reader(SOURCELINELENGTH, LOOKAHEAD);
LISTER lister(LINESPERPAGE);
// CODEGENERATION
CODE code;
IDENTIFIERTABLE identifierTable(&lister, MAXIMUMIDENTIFIERS);
// ENDCODEGENERATION

#ifdef TRACEPARSER
int level;
#endif

//--------------------------------------------------
// Function to get token description from TOKENTABLE
//--------------------------------------------------
const char *GetTokenDescription(TOKENTYPE type)
{
	for (int i = 0; i < (sizeof(TOKENTABLE) / sizeof(TOKENTABLERECORD)); ++i)
	{  
		if (TOKENTABLE[i].type == type)
		{
			return TOKENTABLE[i].description;
		}
	}
	return "UNKNOWN";
}

//--------------------------------------------------
// Debugging function to print token information
//--------------------------------------------------
void DebugPrintToken(const TOKEN &token, const char *location)
{
	const char *description = GetTokenDescription(token.type);

	// Check if the token is a reserved word
	bool isReserved = false;
	for (int i = 0; i < sizeof(TOKENTABLE) / sizeof(TOKENTABLERECORD); ++i)
	{
		if (TOKENTABLE[i].type == token.type)
		{
			isReserved = TOKENTABLE[i].isReservedWord;
			break;
		}
	}

    // Print debug information
    cout << "Debug: Token[0] - Type: " << description
         << ", Lexeme: " << token.lexeme << ", Line: " << token.sourceLineNumber
         << ", Lexeme: " << token.lexeme << ", Line: " << token.sourceLineNumber
         << ", Reserved: " << (isReserved ? "Yes" : "No")
         << ", Location: " << location << endl;
}

//-----------------------------------------------------------
void EnterModule(const char module[])
//-----------------------------------------------------------
{
	#ifdef TRACEPARSER
	char information[SOURCELINELENGTH + 1];

	level++;
	sprintf(information, "   %*s>%s", level *2, " ", module);
	lister.ListInformationLine(information);
	#endif
}

//-----------------------------------------------------------
void ExitModule(const char module[])
//-----------------------------------------------------------
{
	#ifdef TRACEPARSER
	char information[SOURCELINELENGTH + 1];

	sprintf(information, "   %*s<%s", level *2, " ", module);
	lister.ListInformationLine(information);
	level--;
	#endif
}

//--------------------------------------------------
void ProcessCompilerError(int sourceLineNumber, int sourceLineIndex, const char errorMessage[])
//--------------------------------------------------
{
	char information[SOURCELINELENGTH + 1];

	// Use "panic mode" error recovery technique: report error message and terminate compilation!
	sprintf(information, "     At (%4d:%3d) %s", sourceLineNumber, sourceLineIndex, errorMessage);
	lister.ListInformationLine(information);
	lister.ListInformationLine("POTATO compiler ending with a potato famine!\n");
	throw (POTATOEXCEPTION("POTATO compiler ending with a potato famine!"));
}

//-----------------------------------------------------------
int main()
//-----------------------------------------------------------
{
	void Callback1(int sourceLineNumber, const char sourceLine[]);
	void Callback2(int sourceLineNumber, const char sourceLine[]);
	void ParsePOTATOProgram(TOKEN tokens[]);
	void GetNextToken(TOKEN tokens[]);

	char sourceFileName[80 + 1];
	TOKEN tokens[LOOKAHEAD + 1];

	cout << "Source filename? "; cin >> sourceFileName;

	try
	{
		lister.OpenFile(sourceFileName);
		code.OpenFile(sourceFileName);

		// CODEGENERATION
		code.EmitBeginningCode(sourceFileName);
		// ENDCODEGENERATION

		reader.SetLister(&lister);
		reader.AddCallbackFunction(Callback1);
		reader.AddCallbackFunction(Callback2);
		reader.OpenFile(sourceFileName);

		// Fill tokens[] for look-ahead
		for (int i = 0; i <= LOOKAHEAD; i++)
			GetNextToken(tokens);

		#ifdef TRACEPARSER
		level = 0;
		#endif

		ParsePOTATOProgram(tokens);

		// CODEGENERATION
		code.EmitEndingCode();
		// ENDCODEGENERATION

	}
	catch (POTATOEXCEPTION potatoException)
	{
		cout << "POTATO exception: " << potatoException.GetDescription() << endl;
	}
	lister.ListInformationLine("*******POTATO was cooked thoroughly");
	cout << "POTATO was cooked thoroughly\n";

	system("PAUSE");
	return (0);
}

//-----------------------------------------------------------
void ParsePOTATOProgram(TOKEN tokens[])
//-----------------------------------------------------------
{
	void ParseDataDefinitions(TOKEN tokens[], IDENTIFIERSCOPE identifierScope);
	void ParseMAINDefinition(TOKEN tokens[]);
	void GetNextToken(TOKEN tokens[]);

	EnterModule("POTATOProgram");

	ParseDataDefinitions(tokens, GLOBALSCOPE);

	#ifdef TRACECOMPILER
	identifierTable.DisplayTableContents("Contents of identifier table after compilation of global data definitions");
	#endif

	if (tokens[0].type != CRISP)
	{
		ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting `crisp` keyword");
		return;
	}

	GetNextToken(tokens);	// Move past `crisp`

	if (tokens[0].type == PROGRAM)
		ParseMAINDefinition(tokens);
	else
		ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex,
			"Expecting MAIN");

	if (tokens[0].type != EOPTOKEN)
		ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex,
			"Expecting end-of-program");

	ExitModule("POTATOProgram");
}

//-----------------------------------------------------------
void ParseDataDefinitions(TOKEN tokens[], IDENTIFIERSCOPE identifierScope)
//-----------------------------------------------------------
{
	void GetNextToken(TOKEN tokens[]);

	char identifier[MAXIMUMLENGTHIDENTIFIER + 1];	// identifier being parsed
	char reference[MAXIMUMLENGTHIDENTIFIER + 1];	// Reference for memory location
	char literal[MAXIMUMLENGTHIDENTIFIER + 1];	// Stores literal value of a constant
	DATATYPE dataType;
	bool isConstant = false;	// Flag for constants
	bool isInTable;	// Flag for identifier exists in table
	bool isInGlobalTable;	// Flag for in global table
	int index;

	EnterModule("DataDefinitions");

	while ((tokens[0].type == CONSTANT) || (tokens[0].type == INTEGER) || (tokens[0].type == BOOLEAN))
	{
		// Check if constant
		if (tokens[0].type == CONSTANT)
		{
			isConstant = true;
			GetNextToken(tokens);	// Move to the datatype (spud/tater)
		}

		// Determine datatype
		switch (tokens[0].type)
		{
			case INTEGER:	// 'spud'
				dataType = INTTYPE;
				break;
			case BOOLEAN:	// 'tater'
				dataType = BOOLTYPE;
				break;
			default:
				ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting a valid data type (spud, tater)");
		}

		GetNextToken(tokens);	// Move to the identifier after datatype

		// Parse identifiers (e.g., `spud x1: x2`)
		do { 	if (tokens[0].type != IDENTIFIER)
			{
				ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting identifier");
			}

			strcpy(identifier, tokens[0].lexeme);
			GetNextToken(tokens);	// Move to the next token to check for '=' or ';'

			// Expect '=' for constant assignment
			if (isConstant)
			{
				if (tokens[0].type != EQ)
				{
					ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting '=' for constant assignment");
				}
			}

			// but if it has an '=' (regardless of constant) assign it a value
			if (tokens[0].type == EQ)
			{
				GetNextToken(tokens);	// Move to the value after '='

				// Assign value
				if ((dataType == INTTYPE && tokens[0].type == INTEGER))
				{
					strcpy(literal, "0D");
					strcat(literal, tokens[0].lexeme);
				}
				else if ((dataType == BOOLTYPE) && (tokens[0].type == TRUE || tokens[0].type == FALSE))
				{
					strcpy(literal, tokens[0].lexeme);
				}
				else
				{
					ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Data type mismatch for constant value");
				}

				GetNextToken(tokens);	// Move to the value after '=' 
			}

			// Check if the identifier is already in the local or global table
			index = identifierTable.GetIndex(identifier, isInTable);

			// If it is already in the table, we need to verify its scope
			if (isInTable)
			{
				if (identifierTable.IsInCurrentScope(index))
				{
				 		// Identifier is already in the current local scope
					ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Multiply-defined identifier in the current scope");
				}
				else if (identifierScope == PROGRAMMODULESCOPE && identifierTable.GetScope(index) == GLOBALSCOPE)
				{
				 		// Identifier exists in the global scope, and we are in the local scope (module scope)
					ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Local variable name conflicts with a global variable");
				}
			}

			// Add the identifier to the table
			if (!isConstant)
			{
			 	// Determine scope and add to table
				switch (identifierScope)
				{
					case GLOBALSCOPE:
						code.AddRWToStaticData(1, identifier, reference);
						identifierTable.AddToTable(identifier, GLOBAL_VARIABLE, dataType, reference);
						break;
					case PROGRAMMODULESCOPE:
						code.AddRWToStaticData(1, identifier, reference);
						identifierTable.AddToTable(identifier, PROGRAMMODULE_VARIABLE, dataType, reference);
						break;
				}
			}
			else
			{
			 	// Add constant to identifier table
				switch (identifierScope)
				{
					case GLOBALSCOPE:
						code.AddDWToStaticData(literal, identifier, reference);
						identifierTable.AddToTable(identifier, GLOBAL_CONSTANT, dataType, reference);
						break;
					case PROGRAMMODULESCOPE:
						code.AddDWToStaticData(literal, identifier, reference);
						identifierTable.AddToTable(identifier, PROGRAMMODULE_CONSTANT, dataType, reference);
						break;
				}
			}

			// Check for more identifiers of the same data type
			if (tokens[0].type == COLON)
			{
				GetNextToken(tokens);	// Move to the next identifier after ':'
				if (tokens[0].type != IDENTIFIER)
				{
					ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting identifier after ':'");
				}
			}
			else
			{
				break;	// No more identifiers, exit the loop
			}
		} while (TRUE);	// Continue parsing 

		if (tokens[0].type != SEMICOLON)
		{
			ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Data Expecting ';' at the end of declaration");
		}

		GetNextToken(tokens);	// Move to the next token after the semicolon

		// Reset
		isConstant = false;
	}

	ExitModule("DataDefinitions");
}

//-----------------------------------------------------------
void ParseMAINDefinition(TOKEN tokens[])
//-----------------------------------------------------------
{
	void ParseDataDefinitions(TOKEN tokens[], IDENTIFIERSCOPE identifierScope);
	void ParseStatement(TOKEN tokens[]);
	void GetNextToken(TOKEN tokens[]);

	char line[SOURCELINELENGTH + 1];
	char label[SOURCELINELENGTH + 1];
	char reference[SOURCELINELENGTH + 1];

	EnterModule("PROGRAMDefinition");

	// CODEGENERATION
	code.EmitUnformattedLine("; ****=========");
	sprintf(line, "; ****PROGRAM module (%4d)", tokens[0].sourceLineNumber);
	code.EmitUnformattedLine(line);
	code.EmitUnformattedLine("; ****=========");
	code.EmitFormattedLine("PROGRAMMAIN", "EQU", "*");

	// Initialize stack and heap
	code.EmitFormattedLine("", "PUSH", "#RUNTIMESTACK", "set SP");
	code.EmitFormattedLine("", "POPSP");
	code.EmitFormattedLine("", "PUSHA", "STATICDATA", "set SB");
	code.EmitFormattedLine("", "POPSB");
	code.EmitFormattedLine("", "PUSH", "#HEAPBASE", "initialize heap");
	code.EmitFormattedLine("", "PUSH", "#HEAPSIZE");
	code.EmitFormattedLine("", "SVC", "#SVC_INITIALIZE_HEAP");

	// Call empty body or return immediately
	sprintf(label, "PROGRAMBODY%04d", code.LabelSuffix());
	code.EmitFormattedLine("", "CALL", label);
	code.AddDSToStaticData("Normal program termination", "", reference);
	code.EmitFormattedLine("", "PUSHA", reference);
	code.EmitFormattedLine("", "SVC", "#SVC_WRITE_STRING");
	code.EmitFormattedLine("", "SVC", "#SVC_WRITE_ENDL");

	// End program
	code.EmitFormattedLine("", "PUSH", "#0D0", "terminate with status = 0");
	code.EmitFormattedLine("", "SVC", "#SVC_TERMINATE");
	code.EmitFormattedLine(label, "EQU", "*");
	// ENDCODEGENERATION

	GetNextToken(tokens);	// Move past 'crispMAIN'

	identifierTable.EnterNestedStaticScope();
	ParseDataDefinitions(tokens, PROGRAMMODULESCOPE);

	// Check for empty parameter list `()`
	if (tokens[0].type != OPARENTHESIS)
	{
        ProcessCompilerError (tokens[0].sourceLineNumber,
                              tokens[0].sourceLineIndex, "Expecting '('");
	}

	GetNextToken(tokens);	// move past `(`

	if (tokens[0].type != CPARENTHESIS)
	{
        ProcessCompilerError (tokens[0].sourceLineNumber,
                              tokens[0].sourceLineIndex, "Expecting ')'");
	}

	GetNextToken(tokens);	// move past `)`

	// Check for opening curly bracket `{` 
	if (tokens[0].type != OCBRACKET)
	{
		ProcessCompilerError(			tokens[0].sourceLineNumber, tokens[0].sourceLineIndex,
			"Expecting opening bracket for MAINCRISP function definition");
		return;	// Exit if opening bracket is missing
	}

	GetNextToken(tokens);	// Move past the opening bracket `{` 

	// Handle **E M P T Y **MAINCRISP block by check for closing bracket `}`
	if (tokens[0].type == CCBRACKET)
	{
		GetNextToken(tokens);	// Move past the closing bracket `}`
	}
	else
	{
		// Process statements inside the MAINCRISP block
		while (tokens[0].type != CCBRACKET && tokens[0].type != EOPTOKEN)
		{
			ParseStatement(tokens);
			if (tokens[0].type == EOPTOKEN)
			{
				ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex,
					"MAIN: Unexpected end-of-program");
				//break;	// Exit if end-of-program is reached
			}
		}

		GetNextToken(tokens);	// Move past the closing bracket `}`
	}

	// RETURN for body
	// CODEGENERATION
	code.EmitFormattedLine("", "RETURN");
	code.EmitUnformattedLine("; ****=========");
	sprintf(line, "; ****END (%4d)", tokens[0].sourceLineNumber);
	code.EmitUnformattedLine(line);
	code.EmitUnformattedLine("; ****=========");
	// ENDCODEGENERATION

	#ifdef TRACECOMPILER
	identifierTable.DisplayTableContents("Contents of identifier table at end of compilation of PROGRAM module definition");
	#endif

	identifierTable.ExitNestedStaticScope();

	//GetNextToken(tokens);

	ExitModule("PROGRAMDefinition");
}

//-----------------------------------------------------------
void ParseStatement(TOKEN tokens[])
//-----------------------------------------------------------
{
	void ParsePRINTStatement(TOKEN tokens[]);
	void ParseINPUTStatement(TOKEN tokens[]);
	void ParseAssignmentStatement(TOKEN tokens[]);
	void ParseIFStatement(TOKEN tokens[]);
	void ParseDOWHILEStatement(TOKEN tokens[]);
	void ParseFORStatement(TOKEN tokens[]);
	void ParseWHILEStatement(TOKEN tokens[]);
	void GetNextToken(TOKEN tokens[]);

	EnterModule("Statement");


	switch (tokens[0].type)
	{
		case PRINT:
			ParsePRINTStatement(tokens);
			break;
		case INPUT:
			ParseINPUTStatement(tokens);
			break;
		case IDENTIFIER:
			ParseAssignmentStatement(tokens);
			break;
		case INTEGER:
		case BOOLEAN:
			ParseDataDefinitions(tokens, PROGRAMMODULESCOPE);
			break;
		case IF:
			ParseIFStatement(tokens);
			break;
		case DO:
			ParseDOWHILEStatement(tokens);
			break;
			// Added ***extras***of SPL4 language
		case FOR:
			ParseFORStatement(tokens);
			break;
		case WHILE:
			ParseWHILEStatement(tokens);
			break;
			// End added ***extras***of SPL4 language
		default:
			ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex,
				"ParseStatement Expecting beginning-of-statement");
			break;
	}

	ExitModule("Statement");
}

//-----------------------------------------------------------
void ParsePRINTStatement(TOKEN tokens[])
//-----------------------------------------------------------
{
	void ParseExpression(TOKEN tokens[], DATATYPE &datatype);
	void GetNextToken(TOKEN tokens[]);

	char line[SOURCELINELENGTH + 1];
	DATATYPE datatype;

	EnterModule("PRINTStatement");

	sprintf(line, "; ****PRINT statement (%4d)", tokens[0].sourceLineNumber);
	code.EmitUnformattedLine(line);

	do {
		GetNextToken(tokens);	// move past 'bite'

		switch (tokens[0].type)
		{
			case STRING:
				// CODEGENERATION
				char reference[SOURCELINELENGTH + 1];

				code.AddDSToStaticData(tokens[0].lexeme, "", reference);
				code.EmitFormattedLine("", "PUSHA", reference);
				code.EmitFormattedLine("", "SVC", "#SVC_WRITE_STRING");
				// ENDCODEGENERATION

				GetNextToken(tokens);
				break;
			case ENDL:

				// CODEGENERATION
				code.EmitFormattedLine("", "SVC", "#SVC_WRITE_ENDL");
				// ENDCODEGENERATION

				GetNextToken(tokens);
				break;
			default:
				{
					ParseExpression(tokens, datatype);

					// CODEGENERATION
					switch (datatype)
					{
						case INTTYPE:
							code.EmitFormattedLine("", "SVC", "#SVC_WRITE_INTEGER");
							break;
						case BOOLTYPE:
							code.EmitFormattedLine("", "SVC", "#SVC_WRITE_BOOLEAN");
							break;
					}
					// ENDCODEGENERATION

				}
		}
	} while (tokens[0].type == COMMA);

	if (tokens[0].type != SEMICOLON)
		ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Print Expecting ';'");

	GetNextToken(tokens);

	ExitModule("PRINTStatement");
}

//-----------------------------------------------------------
void ParseINPUTStatement(TOKEN tokens[])
//-----------------------------------------------------------
{
	void ParseVariable(TOKEN tokens[], bool asLValue, DATATYPE &datatype);
	void GetNextToken(TOKEN tokens[]);

	char reference[SOURCELINELENGTH + 1];
	char line[SOURCELINELENGTH + 1];
	DATATYPE datatype;

	EnterModule("INPUTStatement");

	sprintf(line, "; ****INPUT statement (%4d)", tokens[0].sourceLineNumber);
	code.EmitUnformattedLine(line);

	GetNextToken(tokens); // eat '"'

	if (tokens[0].type == STRING)
	{
		// CODEGENERATION
		code.AddDSToStaticData(tokens[0].lexeme, "", reference);
		code.EmitFormattedLine("", "PUSHA", reference);
		code.EmitFormattedLine("", "SVC", "#SVC_WRITE_STRING");
		// ENDCODEGENERATION

		GetNextToken(tokens); // eat '"'
	}

	ParseVariable(tokens, true, datatype);

	// CODEGENERATION
	switch (datatype)
	{
		case INTTYPE:
			code.EmitFormattedLine("", "SVC", "#SVC_READ_INTEGER");
			break;
		case BOOLTYPE:
			code.EmitFormattedLine("", "SVC", "#SVC_READ_BOOLEAN");
			break;
	}

	code.EmitFormattedLine("", "POP", "@SP:0D1");
	code.EmitFormattedLine("", "DISCARD", "#0D1");
	// ENDCODEGENERATION

	if (tokens[0].type != SEMICOLON)
		ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Input Expecting ';'");

	GetNextToken(tokens); // eat ';'

	ExitModule("INPUTStatement");
}

//-----------------------------------------------------------
void ParseAssignmentStatement(TOKEN tokens[])
//-----------------------------------------------------------
{
    void ParseVariable(TOKEN tokens[], bool asLValue, DATATYPE &datatype);
    void ParseExpression(TOKEN tokens[], DATATYPE &datatype);
    void GetNextToken(TOKEN tokens[]);

    char line[SOURCELINELENGTH + 1];
    DATATYPE datatypeLHS, datatypeRHS;

    EnterModule("AssignmentStatement");

    sprintf(line, "; ****assignment statement (%4d)", tokens[0].sourceLineNumber);
    code.EmitUnformattedLine(line);

    ParseVariable(tokens, true, datatypeLHS);
	DebugPrintToken(tokens[0], "Parsed first variable");


    while (true)
    {
        // Check for assignment operator (`=`, `+=`, `-=`, `*=`, `/=`)
        if (tokens[0].type == EQ 
		 || tokens[0].type == EQPLUS 
		 || tokens[0].type == EQMINUS
		 || tokens[0].type == EQMULTIPLY 
		 || tokens[0].type == EQDIVIDE)
        {
			DebugPrintToken(tokens[0], "Found assignment operator");

            TOKENTYPE type = tokens[0].type;
            GetNextToken(tokens); // eat the assignment operator
			DebugPrintToken(tokens[0], "Ate assignment operator");

			/* if (type != EQ){
				GetNextToken(tokens); // eat '=' from compound
				DebugPrintToken(tokens[0], "Ate '=' for compound assignment");
			}  */
				 

            ParseExpression(tokens, datatypeRHS);
			DebugPrintToken(tokens[0], "Parsed expression");


            if (datatypeLHS != datatypeRHS)
            {
                ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Data type mismatch");
            }

            switch (type)
            {
                case EQ:
					// LHS = RHS
					code.EmitFormattedLine("", "POP", "@SB:0D0");
					break;


				case EQPLUS:
					// LHS += RHS
					//code.EmitFormattedLine("", "PUSH", "@SB:0D0");
					code.EmitFormattedLine("", "ADDI", "");  
					code.EmitFormattedLine("", "POP", "@SB:0D0"); 
					break;


				case EQMINUS:
					// LHS -= RHS
					//code.EmitFormattedLine("", "PUSH", "@SB:0D0");
					code.EmitFormattedLine("", "SUBI", "");  
					code.EmitFormattedLine("", "POP", "@SB:0D0"); 
					break;

				case EQMULTIPLY:
					// LHS *= RHS
					//code.EmitFormattedLine("", "PUSH", "@SB:0D0");
					code.EmitFormattedLine("", "MULI", "");  
					code.EmitFormattedLine("", "POP", "@SB:0D0"); 
					break;


				case EQDIVIDE:
					// LHS /= RHS
					//ode.EmitFormattedLine("", "PUSH", "@SB:0D0");
					code.EmitFormattedLine("", "DIVI", "");  
					code.EmitFormattedLine("", "POP", "@SB:0D0"); 
					break;


				default:
					ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Invalid assignment operator");
					break;
			}


            if (type != EQ)
            {
                code.EmitFormattedLine("", "DISCARD", "#0D1");
            }
        }
        else
        {
            // No assignment operator; leave the current variable uninitialized
			DebugPrintToken(tokens[0], "No assignment operator found; leaving variable uninitialized");

        }

        // Check if there are more variables to parse separated by ':'
        if (tokens[0].type == COLON)
        {
            GetNextToken(tokens); // eat ':'
            ParseVariable(tokens, true, datatypeLHS);
			DebugPrintToken(tokens[0], "Parsed additional LHS variable");

        }
        else
        {
			DebugPrintToken(tokens[0], "No more variables to parse, breaking out of the loop");
            break;
        }
    }

    if (tokens[0].type != SEMICOLON)
    {
        ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting ';' at the end of the assignment statement");
    }
	
	DebugPrintToken(tokens[0], "Found ';', completing assignment statement");
    GetNextToken(tokens); // eat ';'

    ExitModule("AssignmentStatement");
	DebugPrintToken(tokens[0], "Exiting ParseAssignmentStatement");

}



/*
||-----------------------------------------------------------
|| <IFStatement> ::= BAKE ( <expression> ) 
||                      { <statement> }* 
||                  { ELBAKE ( <expression> ) 
||                      { <statement> }* }*
||                  [ ELSE 
||                      { <statement> }* ]
||-----------------------------------------------------------
*/
//-----------------------------------------------------------
void ParseIFStatement(TOKEN tokens[])
//-----------------------------------------------------------
{
	void ParseExpression(TOKEN tokens[], DATATYPE &datatype);
	void ParseStatement(TOKEN tokens[]);
	void GetNextToken(TOKEN tokens[]);

	char line[SOURCELINELENGTH + 1];
	char Ilabel[SOURCELINELENGTH + 1], Elabel[SOURCELINELENGTH + 1];
	DATATYPE datatype;

	EnterModule("IFStatement");

	sprintf(line, "; ****IF statement (%4d)", tokens[0].sourceLineNumber);
	code.EmitUnformattedLine(line);

	GetNextToken(tokens);	// move past if
	DebugPrintToken(tokens[0], "Ate 'if'");

	if (tokens[0].type != OPARENTHESIS)
		ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting '('");
	GetNextToken(tokens);	// eat '('
	DebugPrintToken(tokens[0], "Ate '('");
	ParseExpression(tokens, datatype);
	if (tokens[0].type != CPARENTHESIS)
		ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting ')'");
	GetNextToken(tokens);	// eat ')'
	DebugPrintToken(tokens[0], "Ate ')'");
	if (tokens[0].type != OCBRACKET)
		ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting '{'");
	GetNextToken(tokens);	// eat '{'
	DebugPrintToken(tokens[0], "Ate '{'");

	if (datatype != BOOLTYPE)
		ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting boolean expression");

	// CODEGENERATION
	/*
	   Plan for the generalized IF statement with n ELIFs and 1 ELSE (*Note* n
	      can be 0 and the ELSE may be missing and the plan still "works.")

	   ...expression...           ; boolean expression on top-of-stack
	      SETT
	      DISCARD   #0D1
	      JMPNT     I???1
	   ...statements...
	      JMP       E????
	I???1 EQU       *            ; 1st ELIF clause
	   ...expression...
	      SETT
	      DISCARD   #0D1
	      JMPNT     I???2
	   ...statements...
	      JMP       E????
	      .
	      .
	I???n EQU       *            ; nth ELIF clause
	   ...expression...
	      SETT
	      DISCARD   #0D1
	      JMPNT     I????
	   ...statements...
	      JMP       E????
	I???? EQU       *            ; ELSE clause
	   ...statements...
	E???? EQU       *
	*/
	sprintf(Elabel, "E%04d", code.LabelSuffix());
	code.EmitFormattedLine("", "SETT");
	code.EmitFormattedLine("", "DISCARD", "#0D1");
	sprintf(Ilabel, "I%04d", code.LabelSuffix());
	code.EmitFormattedLine("", "JMPNT", Ilabel);
	// ENDCODEGENERATION
	
	while ((tokens[0].type != ELIF) &&
			(tokens[0].type != ELSE) &&
			(tokens[0].type != CCBRACKET))
		ParseStatement(tokens);
		
	GetNextToken(tokens);	// Move past '}' 

	// CODEGENERATION
	code.EmitFormattedLine("", "JMP", Elabel);
	code.EmitFormattedLine(Ilabel, "EQU", "*");
	// ENDCODEGENERATION

	while (tokens[0].type == ELIF)
	{
		GetNextToken(tokens);	// Move past elif
		DebugPrintToken(tokens[0], "Ate elif");
		if (tokens[0].type != OPARENTHESIS)
			ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting '(' after ELIF");
		GetNextToken(tokens);	// eat '('
		DebugPrintToken(tokens[0], "Ate '('");
		ParseExpression(tokens, datatype);
		if (tokens[0].type != CPARENTHESIS)
			ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting ')'");
		GetNextToken(tokens);	// eat ')'
		DebugPrintToken(tokens[0], "Ate ')'");
		if (tokens[0].type != OCBRACKET)
			ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting '{' after condition in ELIF (ELBAKE)");
		GetNextToken(tokens);	// eat '{'
		DebugPrintToken(tokens[0], "Ate '{'");

		if (datatype != BOOLTYPE)
			ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting boolean expression");

		// CODEGENERATION
		code.EmitFormattedLine("", "SETT");
		code.EmitFormattedLine("", "DISCARD", "#0D1");
		sprintf(Ilabel, "I%04d", code.LabelSuffix());
		code.EmitFormattedLine("", "JMPNT", Ilabel);
		// ENDCODEGENERATION

		while ((tokens[0].type != ELIF) &&
			(tokens[0].type != ELSE) &&
			(tokens[0].type != CCBRACKET))
			ParseStatement(tokens);

		// CODEGENERATION
		code.EmitFormattedLine("", "JMP", Elabel);
		code.EmitFormattedLine(Ilabel, "EQU", "*");
		// ENDCODEGENERATION

		GetNextToken(tokens);	// Move past ELIF block's '}' 
		DebugPrintToken(tokens[0], "Ate '}', else, or elif (in elif)");
	}

	// Handle `ELSE` clause
	if (tokens[0].type == ELSE)
	{
		GetNextToken(tokens);	// move past 'else'
		DebugPrintToken(tokens[0], "Ate else");

		if (tokens[0].type != OCBRACKET)
			ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting '{' after ELSE");

		GetNextToken(tokens);	// eat '{'
		DebugPrintToken(tokens[0], "Ate '{'");

		while (tokens[0].type != CCBRACKET)
		{
			ParseStatement(tokens);
		}

		GetNextToken(tokens);	// eat '}' 
		DebugPrintToken(tokens[0], "Ate '}'");
	}

	//GetNextToken(tokens);	// for next stuff

	// CODEGENERATION
	code.EmitFormattedLine(Elabel, "EQU", "*");
	// ENDCODEGENERATION

	ExitModule("IFStatement");
}

/*
||-----------------------------------------------------------
||<DOWHILEStatement>   ::= DO
||                              {<statement> }*
||                         fryAsLongAs( < expression> ) ;
||-----------------------------------------------------------
*/
//-----------------------------------------------------------
void ParseDOWHILEStatement(TOKEN tokens[])
//-----------------------------------------------------------
{
	void ParseExpression(TOKEN tokens[], DATATYPE &datatype);
	void ParseStatement(TOKEN tokens[]);
	void GetNextToken(TOKEN tokens[]);

	char line[SOURCELINELENGTH + 1];
	char Dlabel[SOURCELINELENGTH + 1], Elabel[SOURCELINELENGTH + 1];
	DATATYPE datatype;

	EnterModule("DOWHILEStatement");

	sprintf(line, "; ****DO-WHILE statement (%4d)", tokens[0].sourceLineNumber);
	code.EmitUnformattedLine(line);

	GetNextToken(tokens); // eat 'do'
	
	// CODEGENERATION
	/*
	D???? EQU       *
	   ...statements...
	E???? EQU       *
	   ...expression...
	      SETT
	      DISCARD   #0D1
	      JMPNT     E????
	   ...statements...
	      JMP       D????
	*/

	sprintf(Dlabel, "D%04d", code.LabelSuffix());
	sprintf(Elabel, "E%04d", code.LabelSuffix());
	code.EmitFormattedLine(Dlabel, "EQU", "*");
	// ENDCODEGENERATION

	if (tokens[0].type != OCBRACKET)
		ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting '{'");
	GetNextToken(tokens); // eat '{'
	
	while (tokens[0].type != CCBRACKET)
		ParseStatement(tokens);
	GetNextToken(tokens); // eat '}'

	if (tokens[0].type != WHILE)
		ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting 'while'");
	GetNextToken(tokens); // eat 'while'

	if (tokens[0].type != OPARENTHESIS)
		ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting '('");
	GetNextToken(tokens); // eat '('
	ParseExpression(tokens, datatype);

	if (tokens[0].type != CPARENTHESIS)
		ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting ')'");
	GetNextToken(tokens); // eat ')'
	if (tokens[0].type != SEMICOLON)
		ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting ';'");
	GetNextToken(tokens); // eat ';'

	if (datatype != BOOLTYPE)
		ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting boolean expression");

	// CODEGENERATION
	code.EmitFormattedLine("", "SETT");
	code.EmitFormattedLine("", "DISCARD", "#0D1");
	code.EmitFormattedLine("", "JMPNT", Elabel);
	code.EmitFormattedLine("", "JMP", Dlabel);
	code.EmitFormattedLine(Elabel,"EQU","*");
	// ENDCODEGENERATION

	ExitModule("DOWHILEStatement");
}

/*
||-----------------------------------------------------------
||<WHILEStatement>      ::= fryAsLongAs( < expression> )
||                              {<statement> }*
||-----------------------------------------------------------
*/
//-----------------------------------------------------------
void ParseWHILEStatement(TOKEN tokens[])
//-----------------------------------------------------------
{
	void ParseExpression(TOKEN tokens[], DATATYPE &datatype);
	void ParseStatement(TOKEN tokens[]);
	void GetNextToken(TOKEN tokens[]);

	char line[SOURCELINELENGTH + 1];
	char Dlabel[SOURCELINELENGTH + 1], Elabel[SOURCELINELENGTH + 1];
	DATATYPE datatype;

	EnterModule("WHILEStatement");

	sprintf(line, "; ****WHILE statement (%4d)", tokens[0].sourceLineNumber);
	code.EmitUnformattedLine(line);

	GetNextToken(tokens);	// Move past 'fryAsLongAs'

	// CODEGENERATION
	/*
	D???? EQU       *
	   ...expression...
	      SETT
	      DISCARD   #0D1
	      JMPNT     E????
	   ...statements...
	      JMP       D????
	E???? EQU       *
	*/

	sprintf(Dlabel, "D%04d", code.LabelSuffix());
	sprintf(Elabel, "E%04d", code.LabelSuffix());
	code.EmitFormattedLine(Dlabel, "EQU", "*");
	// ENDCODEGENERATION

	if (tokens[0].type != OPARENTHESIS)
		ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting '('");
	GetNextToken(tokens);	// move past '('
	ParseExpression(tokens, datatype);
	if (tokens[0].type != CPARENTHESIS)
		ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting ')'");
	GetNextToken(tokens);	// Move past ')

	if (datatype != BOOLTYPE)
		ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting boolean expression");

	// CODEGENERATION
	code.EmitFormattedLine("", "SETT");
	code.EmitFormattedLine("", "DISCARD", "#0D1");
	code.EmitFormattedLine("", "JMPNT", Elabel);
	// ENDCODEGENERATION

	GetNextToken(tokens);	// Move past '{'
	while (tokens[0].type != CCBRACKET)
		ParseStatement(tokens);

	GetNextToken(tokens);	// move past '}'

	// CODEGENERATION
	code.EmitFormattedLine("", "JMP", Dlabel);
	code.EmitFormattedLine(Elabel, "EQU", "*");
	// ENDCODEGENERATION

	ExitModule("WHILEStatement");
}

/*
||-----------------------------------------------------------
|| <FORStatement> ::= SEASON ( <initialization> ; <condition> ; <iteration> )
||                       	{ <statement> }* 
||-----------------------------------------------------------
*/
//-----------------------------------------------------------
void ParseFORStatement(TOKEN tokens[])
//-----------------------------------------------------------
{
    void ParseExpression(TOKEN tokens[], DATATYPE &datatype);
    void ParseStatement(TOKEN tokens[]);
    void GetNextToken(TOKEN tokens[]);

    char line[SOURCELINELENGTH + 1];
    char Dlabel[SOURCELINELENGTH + 1], Elabel[SOURCELINELENGTH + 1];
    DATATYPE datatype;
    char iterationStatement[SOURCELINELENGTH + 1] = "";  // Variable to store the iteration statement

    EnterModule("FORStatement");

    sprintf(line, "; **** FOR statement (%4d)", tokens[0].sourceLineNumber);
    code.EmitUnformattedLine(line);

    GetNextToken(tokens); // eat 'for'

    // Parsing Initialization Part
    if (tokens[0].type != OPARENTHESIS) {
        ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting '('");
    }
    GetNextToken(tokens); // eat '('

    if (tokens[0].type != SEMICOLON) {
        // Parse the initialization expression
        ParseExpression(tokens, datatype);
        if (tokens[0].type != SEMICOLON) {
            ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting ';' after initialization");
        }
    }
    GetNextToken(tokens); // eat ';'

    // CODEGENERATION
    sprintf(Dlabel, "D%04d", code.LabelSuffix());
    sprintf(Elabel, "E%04d", code.LabelSuffix());
    code.EmitFormattedLine(Dlabel, "EQU", "*");
    // ENDCODEGENERATION

    // Parsing Condition Part
    if (tokens[0].type != SEMICOLON) {
        ParseExpression(tokens, datatype);
        if (tokens[0].type != SEMICOLON) {
            ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting ';' after condition");
        }
        if (datatype != BOOLTYPE) {
            ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting boolean expression in condition");
        }
    }
    GetNextToken(tokens); // eat ';'

    // CODEGENERATION
    code.EmitFormattedLine("", "SETT");
    code.EmitFormattedLine("", "DISCARD", "#0D1");
    code.EmitFormattedLine("", "JMPNT", Elabel); // Jump out if the condition is false
    // ENDCODEGENERATION

    // Parsing Iteration Part
    if (tokens[0].type != CPARENTHESIS) {
        // Save the iteration statement for later execution in the loop body
        int startIndex = tokens[0].sourceLineIndex;
        int sourceLineNumber = tokens[0].sourceLineNumber;
        ParseExpression(tokens, datatype);
        sprintf(iterationStatement, "; **** Iteration statement (%4d)", sourceLineNumber);
        code.EmitFormattedLine("", iterationStatement);
        if (tokens[0].type != CPARENTHESIS) {
            ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting ')' after iteration");
        }
    }
    GetNextToken(tokens); // eat ')'

    // Parsing the Loop Body
    if (tokens[0].type != OCBRACKET) {
        ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting '{'");
    }
    GetNextToken(tokens); // eat '{'

    while (tokens[0].type != CCBRACKET) {
        ParseStatement(tokens);
    }
    GetNextToken(tokens); // eat '}'

    // CODEGENERATION
    // Emit iteration statement here after the body
    if (strlen(iterationStatement) > 0) {
        code.EmitFormattedLine("", iterationStatement);
    }
    code.EmitFormattedLine("", "JMP", Dlabel);  // Jump back to the beginning of the loop
    code.EmitFormattedLine(Elabel, "EQU", "*"); // End of loop label
    // ENDCODEGENERATION

    ExitModule("FORStatement");
}



//-----------------------------------------------------------
void ParseExpression(TOKEN tokens[], DATATYPE &datatype)
//-----------------------------------------------------------
{
	// CODEGENERATION
	/*
	   An expression is composed of a collection of one or more operands (SPL calls them
	      primaries) and operators (and perhaps sets of parentheses to modify the default 
	      order-of-evaluation established by precedence and associativity rules).
	      Expression evaluation computes a single value as the expression's result.
	      The result has a specific data type. By design, the expression result is 
	      "left" at the top of the run-time stack for subsequent use.

	   SPL expressions must be single-mode with operators working on operands of
	      the appropriate type (for example, boolean AND boolean) and not mixing
	      modes. Static semantic analysis guarantees that operators are
	      operating on operands of appropriate data type.
	*/
	// ENDCODEGENERATION

	void ParseConjunction(TOKEN tokens[], DATATYPE &datatype);
	void GetNextToken(TOKEN tokens[]);

	DATATYPE datatypeLHS, datatypeRHS;

	EnterModule("Expression");

	ParseConjunction(tokens, datatypeLHS);

	if ((tokens[0].type == OR) ||
		(tokens[0].type == NOR) ||
		(tokens[0].type == XOR))
	{
		while ((tokens[0].type == OR) ||
			(tokens[0].type == NOR) ||
			(tokens[0].type == XOR))
		{
			TOKENTYPE operation = tokens[0].type;

			GetNextToken(tokens);
			ParseConjunction(tokens, datatypeRHS);

			// CODEGENERATION
			switch (operation)
			{
				case OR:

					// STATICSEMANTICS
					if (!((datatypeLHS == BOOLTYPE) && (datatypeRHS == BOOLTYPE)))
						ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting boolean operands");
					// ENDSTATICSEMANTICS

					code.EmitFormattedLine("", "OR");
					datatype = BOOLTYPE;
					break;
				case NOR:

					// STATICSEMANTICS
					if (!((datatypeLHS == BOOLTYPE) && (datatypeRHS == BOOLTYPE)))
						ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting boolean operands");
					// ENDSTATICSEMANTICS

					code.EmitFormattedLine("", "NOR");
					datatype = BOOLTYPE;
					break;
				case XOR:

					// STATICSEMANTICS
					if (!((datatypeLHS == BOOLTYPE) && (datatypeRHS == BOOLTYPE)))
						ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting boolean operands");
					// ENDSTATICSEMANTICS

					code.EmitFormattedLine("", "XOR");
					datatype = BOOLTYPE;
					break;
			}
		}

		// CODEGENERATION

	}
	else
		datatype = datatypeLHS;

	ExitModule("Expression");
}

//-----------------------------------------------------------
void ParseConjunction(TOKEN tokens[], DATATYPE &datatype)
//-----------------------------------------------------------
{
	void ParseNegation(TOKEN tokens[], DATATYPE &datatype);
	void GetNextToken(TOKEN tokens[]);

	DATATYPE datatypeLHS, datatypeRHS;

	EnterModule("Conjunction");

	ParseNegation(tokens, datatypeLHS);

	if ((tokens[0].type == AND) ||
		(tokens[0].type == NAND))
	{
		while ((tokens[0].type == AND) ||
			(tokens[0].type == NAND))
		{
			TOKENTYPE operation = tokens[0].type;

			GetNextToken(tokens);
			ParseNegation(tokens, datatypeRHS);

			switch (operation)
			{
				case AND:
					if (!((datatypeLHS == BOOLTYPE) && (datatypeRHS == BOOLTYPE)))
						ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting boolean operands");
					code.EmitFormattedLine("", "AND");
					datatype = BOOLTYPE;
					break;
				case NAND:
					if (!((datatypeLHS == BOOLTYPE) && (datatypeRHS == BOOLTYPE)))
						ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting boolean operands");
					code.EmitFormattedLine("", "NAND");
					datatype = BOOLTYPE;
					break;
			}
		}
	}
	else
		datatype = datatypeLHS;

	ExitModule("Conjunction");
}

//-----------------------------------------------------------
void ParseNegation(TOKEN tokens[], DATATYPE &datatype)
//-----------------------------------------------------------
{
	void ParseComparison(TOKEN tokens[], DATATYPE &datatype);
	void GetNextToken(TOKEN tokens[]);

	DATATYPE datatypeRHS;

	EnterModule("Negation");

	if (tokens[0].type == NOT)
	{
		GetNextToken(tokens);
		ParseComparison(tokens, datatypeRHS);

		if (!(datatypeRHS == BOOLTYPE))
			ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting boolean operand");
		code.EmitFormattedLine("", "NOT");
		datatype = BOOLTYPE;
	}
	else
		ParseComparison(tokens, datatype);

	ExitModule("Negation");
}

//-----------------------------------------------------------
void ParseComparison(TOKEN tokens[], DATATYPE &datatype)
//-----------------------------------------------------------
{
	void ParseComparator(TOKEN tokens[], DATATYPE &datatype);
	void GetNextToken(TOKEN tokens[]);

	DATATYPE datatypeLHS, datatypeRHS;

	EnterModule("Comparison");

	ParseComparator(tokens, datatypeLHS);
	if ((tokens[0].type == LT) ||
		(tokens[0].type == LTEQ) ||
		(tokens[0].type == EQEQ) ||
		(tokens[0].type == GT) ||
		(tokens[0].type == GTEQ) ||
		(tokens[0].type == NOTEQ)
)
	{
		TOKENTYPE operation = tokens[0].type;

		GetNextToken(tokens);
		ParseComparator(tokens, datatypeRHS);

		if ((datatypeLHS != INTTYPE) || (datatypeRHS != INTTYPE))
			ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting integer operands");
		/*
		      CMPI
		      JMPXX     T????         ; XX = L,E,G,LE,NE,GE (as required)
		      PUSH      #0X0000       ; push FALSE
		      JMP       E????         ;    or 
		T???? PUSH      #0XFFFF       ; push TRUE (as required)
		E???? EQU       *
		*/
		char Tlabel[SOURCELINELENGTH + 1], Elabel[SOURCELINELENGTH + 1];

		code.EmitFormattedLine("", "CMPI");
		sprintf(Tlabel, "T%04d", code.LabelSuffix());
		sprintf(Elabel, "E%04d", code.LabelSuffix());
		switch (operation)
		{
			case LT:
				code.EmitFormattedLine("", "JMPL", Tlabel);
				break;
			case LTEQ:
				code.EmitFormattedLine("", "JMPLE", Tlabel);
				break;
			case EQEQ:
				code.EmitFormattedLine("", "JMPE", Tlabel);
				break;
			case GT:
				code.EmitFormattedLine("", "JMPG", Tlabel);
				break;
			case GTEQ:
				code.EmitFormattedLine("", "JMPGE", Tlabel);
				break;
			case NOTEQ:
				code.EmitFormattedLine("", "JMPNE", Tlabel);
				break;
		}

		datatype = BOOLTYPE;
		code.EmitFormattedLine("", "PUSH", "#0X0000");
		code.EmitFormattedLine("", "JMP", Elabel);
		code.EmitFormattedLine(Tlabel, "PUSH", "#0XFFFF");
		code.EmitFormattedLine(Elabel, "EQU", "*");
	}
	else
		datatype = datatypeLHS;

	ExitModule("Comparison");
}

//-----------------------------------------------------------
void ParseComparator(TOKEN tokens[], DATATYPE &datatype)
//-----------------------------------------------------------
{
	void ParseTerm(TOKEN tokens[], DATATYPE &datatype);
	void GetNextToken(TOKEN tokens[]);

	DATATYPE datatypeLHS, datatypeRHS;

	EnterModule("Comparator");

	ParseTerm(tokens, datatypeLHS);

	if ((tokens[0].type == PLUS) ||
		(tokens[0].type == MINUS))
	{
		while ((tokens[0].type == PLUS) ||
			(tokens[0].type == MINUS))
		{
			TOKENTYPE operation = tokens[0].type;

			GetNextToken(tokens);
			ParseTerm(tokens, datatypeRHS);

			if ((datatypeLHS != INTTYPE) || (datatypeRHS != INTTYPE))
				ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting integer operands");

			switch (operation)
			{
				case PLUS:
					code.EmitFormattedLine("", "ADDI");
					break;
				case MINUS:
					code.EmitFormattedLine("", "SUBI");
					break;
			}

			datatype = INTTYPE;
		}
	}
	else
		datatype = datatypeLHS;

	ExitModule("Comparator");
}

//-----------------------------------------------------------
void ParseTerm(TOKEN tokens[], DATATYPE &datatype)
//-----------------------------------------------------------
{
	void ParseFactor(TOKEN tokens[], DATATYPE &datatype);
	void GetNextToken(TOKEN tokens[]);

	DATATYPE datatypeLHS, datatypeRHS;

	EnterModule("Term");

	ParseFactor(tokens, datatypeLHS);
	if ((tokens[0].type == MULTIPLY) ||
		(tokens[0].type == DIVIDE) ||
		(tokens[0].type == MODULUS))
	{
		while ((tokens[0].type == MULTIPLY) ||
			(tokens[0].type == DIVIDE) ||
			(tokens[0].type == MODULUS))
		{
			TOKENTYPE operation = tokens[0].type;

			GetNextToken(tokens);
			ParseFactor(tokens, datatypeRHS);

			if ((datatypeLHS != INTTYPE) || (datatypeRHS != INTTYPE))
				ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting integer operands");

			switch (operation)
			{
				case MULTIPLY:
					code.EmitFormattedLine("", "MULI");
					break;
				case DIVIDE:
					code.EmitFormattedLine("", "DIVI");
					break;
				case MODULUS:
					code.EmitFormattedLine("", "REMI");
					break;
			}

			datatype = INTTYPE;
		}
	}
	else
		datatype = datatypeLHS;

	ExitModule("Term");
}

//-----------------------------------------------------------
void ParseFactor(TOKEN tokens[], DATATYPE &datatype)
//-----------------------------------------------------------
{
	void ParseSecondary(TOKEN tokens[], DATATYPE &datatype);
	void GetNextToken(TOKEN tokens[]);

	EnterModule("Factor");

	if ((tokens[0].type == ABS) ||
		(tokens[0].type == PLUS) ||
		(tokens[0].type == MINUS)
)
	{
		DATATYPE datatypeRHS;
		TOKENTYPE operation = tokens[0].type;

		GetNextToken(tokens);
		ParseSecondary(tokens, datatypeRHS);

		if (datatypeRHS != INTTYPE)
			ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting integer operand");

		switch (operation)
		{
			case ABS:
				/*
				      SETNZPI
				      JMPNN     E????
				      NEGI                    ; NEGI or NEGF (as required)
				E???? EQU       *
				*/
				{
					char Elabel[SOURCELINELENGTH + 1];

					sprintf(Elabel, "E%04d", code.LabelSuffix());
					code.EmitFormattedLine("", "SETNZPI");
					code.EmitFormattedLine("", "JMPNN", Elabel);
					code.EmitFormattedLine("", "NEGI");
					code.EmitFormattedLine(Elabel, "EQU", "*");
				}

				break;
			case PLUS:
				// Do nothing (identity operator)
				break;
			case MINUS:
				code.EmitFormattedLine("", "NEGI");
				break;
		}

		datatype = INTTYPE;
	}
	else
		ParseSecondary(tokens, datatype);

	ExitModule("Factor");
}

//-----------------------------------------------------------
void ParseSecondary(TOKEN tokens[], DATATYPE &datatype)
//-----------------------------------------------------------
{
	void ParsePrefix(TOKEN tokens[], DATATYPE &datatype);
	void ParsePostfix(TOKEN tokens[], DATATYPE &datatype);
	void GetNextToken(TOKEN tokens[]);

	DATATYPE datatypeLHS, datatypeRHS;

	EnterModule("Secondary");

	ParsePrefix(tokens, datatypeLHS);

	if (tokens[0].type == POWER)
	{
		GetNextToken(tokens);	// Move past the POWER operator

		ParsePrefix(tokens, datatypeRHS);

		if ((datatypeLHS != INTTYPE) || (datatypeRHS != INTTYPE))
			ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting integer operands");

		code.EmitFormattedLine("", "POWI");
		datatype = INTTYPE;
	}
	else
	{
		ParsePostfix(tokens, datatypeLHS);
		datatype = datatypeLHS;
	}

	ExitModule("Secondary");
}

//-----------------------------------------------------------
void ParsePrefix(TOKEN tokens[], DATATYPE &datatype)
//-----------------------------------------------------------
{
	void ParseVariable(TOKEN tokens[], bool asLValue, DATATYPE &datatype);
	void ParsePrimary(TOKEN tokens[], DATATYPE &datatype);
	void GetNextToken(TOKEN tokens[]);

	EnterModule("Prefix");

	if ((tokens[0].type == INC) ||
		(tokens[0].type == DEC)
)
	{
		DATATYPE datatypeRHS;
		TOKENTYPE operation = tokens[0].type;

		GetNextToken(tokens);
		if ((tokens[0].type == PLUS) || (tokens[0].type == MINUS))
			GetNextToken(tokens);	// get extra +
		ParseVariable(tokens, true, datatypeRHS);

		if (datatypeRHS != INTTYPE)
			ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Increment/Decrement requires an integer operand");

		switch (operation)
		{
			case INC:	// PRE-INCREMENT
				code.EmitFormattedLine("", "PUSH", "@SP:0D0");
				code.EmitFormattedLine("", "PUSH", "#0D1");
				code.EmitFormattedLine("", "ADDI");
				code.EmitFormattedLine("", "POP", "@SP:0D1");	// side-effect
				code.EmitFormattedLine("", "PUSH", "@SP:0D0");
				code.EmitFormattedLine("", "SWAP");
				code.EmitFormattedLine("", "DISCARD", "#0D1");	// value
				break;
			case DEC:	// PRE-DECREMENT
				code.EmitFormattedLine("", "PUSH", "@SP:0D0");
				code.EmitFormattedLine("", "PUSH", "#0D1");
				code.EmitFormattedLine("", "SUBI");
				code.EmitFormattedLine("", "POP", "@SP:0D1");	// side-effect
				code.EmitFormattedLine("", "PUSH", "@SP:0D0");
				code.EmitFormattedLine("", "SWAP");
				code.EmitFormattedLine("", "DISCARD", "#0D1");	// value
				break;
		}

		datatype = INTTYPE;
	}
	else
		ParsePrimary(tokens, datatype);

	ExitModule("Prefix");
}

//-----------------------------------------------------------
void ParsePostfix(TOKEN tokens[], DATATYPE &datatype)
//-----------------------------------------------------------
{
	void ParseVariable(TOKEN tokens[], bool asLValue, DATATYPE &datatype);
	void GetNextToken(TOKEN tokens[]);

	EnterModule("Postfix");

	if ((tokens[0].type == INC) || (tokens[0].type == DEC))
	{
		TOKENTYPE operation = tokens[0].type;

		switch (operation)
		{
			case INC:
				// POST-INCREMENT
				code.EmitFormattedLine("", "PUSH", "@SP:0D0");	// Push current value for use
				code.EmitFormattedLine("", "PUSH", "#0D1");	// Increment by 1
				code.EmitFormattedLine("", "ADDI");
				code.EmitFormattedLine("", "POP", "@SP:0D1");	// Store back
				break;

			case DEC:
				// POST-DECREMENT
				code.EmitFormattedLine("", "PUSH", "@SP:0D0");	// Push current value for use
				code.EmitFormattedLine("", "PUSH", "#0D1");	// Decrement by 1
				code.EmitFormattedLine("", "SUBI");
				code.EmitFormattedLine("", "POP", "@SP:0D1");	// Store back
				break;
		}

		if ((tokens[0].type == PLUS) || (tokens[0].type == MINUS))
			GetNextToken(tokens);

		datatype = INTTYPE;
	}

	ExitModule("Postfix");
}

//-----------------------------------------------------------
void ParsePrimary(TOKEN tokens[], DATATYPE &datatype)
//-----------------------------------------------------------
{
	void ParseVariable(TOKEN tokens[], bool asLValue, DATATYPE &datatype);
	void ParseExpression(TOKEN tokens[], DATATYPE &datatype);
	void GetNextToken(TOKEN tokens[]);

	EnterModule("Primary");

	switch (tokens[0].type)
	{
		case INTEGER:
			{
				char operand[SOURCELINELENGTH + 1];

				sprintf(operand, "#0D%s", tokens[0].lexeme);
				code.EmitFormattedLine("", "PUSH", operand);
				datatype = INTTYPE;
				GetNextToken(tokens);
			}

			break;
			//*************Thanks to Cayden Garcia (FA2023)
			// ***BEWARE***when you choose a different lexeme for either boolean value!
			//*************
		case TRUE:
			code.EmitFormattedLine("", "PUSH", "#0XFFFF");	// or "#true"
			datatype = BOOLTYPE;
			GetNextToken(tokens);
			break;
		case FALSE:
			code.EmitFormattedLine("", "PUSH", "#0X0000");	// or "false"
			datatype = BOOLTYPE;
			GetNextToken(tokens);
			break;
		case OPARENTHESIS:
			GetNextToken(tokens);
			ParseExpression(tokens, datatype);
			if (tokens[0].type != CPARENTHESIS)
				ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting ')'");
			GetNextToken(tokens);
			break;
		case IDENTIFIER:
			ParseVariable(tokens, false, datatype);
			break;
		default:
			ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex,
				"Expecting integer, true, false, '(', variable");
			break;
	}

	ExitModule("Primary");
}

//-----------------------------------------------------------
void ParseVariable(TOKEN tokens[], bool asLValue, DATATYPE &datatype)
//-----------------------------------------------------------
{
	/*
	Syntax "locations"                 l- or r-value
	---------------------------------  -------------
	<expression>                       	r-value
	<prefix>                           	l-value
	<INPUTStatement>                   	l-value
	LHS of<assignmentStatement>       	l-value

	r-value (read-only): value is pushed on run-time stack
	l-value (read/write): address of value is pushed on run-time stack
	*/
	void GetNextToken(TOKEN tokens[]);

	bool isInTable;
	int index;
	IDENTIFIERTYPE identifierType;

	EnterModule("Variable");

	if (tokens[0].type != IDENTIFIER)
		ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting identifier");

	// STATICSEMANTICS
	index = identifierTable.GetIndex(tokens[0].lexeme, isInTable);
	if (!isInTable)
		ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Undefined identifier");

	identifierType = identifierTable.GetType(index);
	datatype = identifierTable.GetDatatype(index);

	if (!((identifierType == GLOBAL_VARIABLE) ||
			(identifierType == GLOBAL_CONSTANT) ||
			(identifierType == PROGRAMMODULE_VARIABLE) ||
			(identifierType == PROGRAMMODULE_CONSTANT)))
		ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting variable or constant identifier");

	if (asLValue && ((identifierType == GLOBAL_CONSTANT) || (identifierType == PROGRAMMODULE_CONSTANT)))
		ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Constant may not be l-value");
	// ENDSTATICSEMANTICS

	// CODEGENERATION
	if (asLValue)
		code.EmitFormattedLine("", "PUSHA", identifierTable.GetReference(index));
	else
		code.EmitFormattedLine("", "PUSH", identifierTable.GetReference(index));
	// ENDCODEGENERATION

	GetNextToken(tokens);

	ExitModule("Variable");
}

//-----------------------------------------------------------
void Callback1(int sourceLineNumber, const char sourceLine[])
//-----------------------------------------------------------
{
	cout << setw(4) << sourceLineNumber << " " << sourceLine << endl;
}

//-----------------------------------------------------------
void Callback2(int sourceLineNumber, const char sourceLine[])
//-----------------------------------------------------------
{
	char line[SOURCELINELENGTH + 1];

	// CODEGENERATION
	sprintf(line, "; %4d %s", sourceLineNumber, sourceLine);
	code.EmitUnformattedLine(line);
	// ENDCODEGENERATION
}

//-----------------------------------------------------------
void GetNextToken(TOKEN tokens[])
//-----------------------------------------------------------
{
	const char *TokenDescription(TOKENTYPE type);

	int i;
	TOKENTYPE type;
	char lexeme[SOURCELINELENGTH + 1];
	int sourceLineNumber;
	int sourceLineIndex;
	char information[SOURCELINELENGTH + 1];

	//============================================================
	// Move look-ahead "window" to make room for next token/lexeme
	//============================================================
	for (int i = 1; i <= LOOKAHEAD; i++)
		tokens[i - 1] = tokens[i];

	char nextCharacter = reader.GetLookAheadCharacter(0).character;

	//============================================================
	// "Eat" white space and comments
	//============================================================
	do {
		//    "Eat" any white-space (blanks and EOLCs and TABCs)
		while ((nextCharacter == ' ')
				|| (nextCharacter == READER<CALLBACKSUSED>::EOLC)
				|| (nextCharacter == READER<CALLBACKSUSED>::TABC))
			nextCharacter = reader.GetNextCharacter().character;

		//  "Eat" line comment
		if ((nextCharacter == '|') && (reader.GetLookAheadCharacter(1).character == '|'))
		{ 
			#ifdef TRACESCANNER
			sprintf(information, "At (%4d:%3d) begin line comment",
				reader.GetLookAheadCharacter(0).sourceLineNumber,
				reader.GetLookAheadCharacter(0).sourceLineIndex);
			lister.ListInformationLine(information);
			#endif

			do
				nextCharacter = reader.GetNextCharacter().character;
			while (nextCharacter != READER<CALLBACKSUSED>::EOLC);
		}

		//    "Eat" block comments (nesting allowed)
		if ((nextCharacter == '|') && (reader.GetLookAheadCharacter(1).character == '['))
		{
			int depth = 0;

			do { 	if ((nextCharacter == '|') && (reader.GetLookAheadCharacter(1).character == '['))
				{
					depth++;

					#ifdef TRACESCANNER
					sprintf(information, "At (%4d:%3d) begin block comment depth = %d",
						reader.GetLookAheadCharacter(0).sourceLineNumber,
						reader.GetLookAheadCharacter(0).sourceLineIndex,
						depth);
					lister.ListInformationLine(information);
					#endif

					nextCharacter = reader.GetNextCharacter().character;
					nextCharacter = reader.GetNextCharacter().character;
				}
				else if ((nextCharacter == ']') && (reader.GetLookAheadCharacter(1).character == '|'))
				{ 		
					#ifdef TRACESCANNER
					sprintf(information, "At (%4d:%3d)   end block comment depth = %d",
						reader.GetLookAheadCharacter(0).sourceLineNumber,
						reader.GetLookAheadCharacter(0).sourceLineIndex,
						depth);
					lister.ListInformationLine(information);
					#endif

					depth--;
					nextCharacter = reader.GetNextCharacter().character;
					nextCharacter = reader.GetNextCharacter().character;
				}
				else
					nextCharacter = reader.GetNextCharacter().character;
			}

			while ((depth != 0) && (nextCharacter != READER<CALLBACKSUSED>::EOPC));
			if (depth != 0)
				ProcessCompilerError(reader.GetLookAheadCharacter(0).sourceLineNumber,
					reader.GetLookAheadCharacter(0).sourceLineIndex,
					"Unexpected end-of-program");
		}

		/*WHILE ((nextCharacter is not a white-space character)
		    or (nextCharacter is not beginning-of-comment character)) */
	} while ((nextCharacter == ' ')
           || (nextCharacter == READER<CALLBACKSUSED>::EOLC)
           || (nextCharacter == READER<CALLBACKSUSED>::TABC)
           || ((nextCharacter == '|') && (reader.GetLookAheadCharacter (1).character == '[')) // allows block comments back to back
           || ((nextCharacter == '|') && (reader.GetLookAheadCharacter (1).character == '|'))); // allows inline comments after block comment

	//============================================================
	// Scan token
	//============================================================
	sourceLineNumber = reader.GetLookAheadCharacter(0).sourceLineNumber;
	sourceLineIndex = reader.GetLookAheadCharacter(0).sourceLineIndex;

	// reserved word or < identifier>
	if (isalpha(nextCharacter))
	{
		// Build lexeme
		char UCLexeme[SOURCELINELENGTH + 1];

		i = 0;
		lexeme[i++] = nextCharacter;
		nextCharacter = reader.GetNextCharacter().character;
		while (isalpha(nextCharacter) || isdigit(nextCharacter) || nextCharacter == '_')
		{
			lexeme[i++] = nextCharacter;
			nextCharacter = reader.GetNextCharacter().character;
		}

		lexeme[i] = '\0';
		for (i = 0; i <= (int) strlen(lexeme); i++)
			UCLexeme[i] = toupper(lexeme[i]);

		// Try to find the lexeme in the table of reserved words
		bool isFound = false;

		i = 0;
		while (!isFound && (i <= (sizeof(TOKENTABLE) / sizeof(TOKENTABLERECORD)) - 1))
		{
			if (TOKENTABLE[i].isReservedWord && (strcmp(UCLexeme, TOKENTABLE[i].description) == 0))
			{
				isFound = true;
				type = TOKENTABLE[i].type;
			}
			else
				i++;
		}

		if (!isFound)	// Not a reserved word, must be an < identifier>.
			type = IDENTIFIER;
	}

	//<integer>
	else if (isdigit(nextCharacter))
	{
		i = 0;
		lexeme[i++] = nextCharacter;
		nextCharacter = reader.GetNextCharacter().character;
		while (isdigit(nextCharacter))
		{
			lexeme[i++] = nextCharacter;
			nextCharacter = reader.GetNextCharacter().character;
		}

		lexeme[i] = '\0';
		type = INTEGER;
	}
	else
	{
		// Determine both the type and the lexeme of the next token
		switch (nextCharacter)
		{
			//<string> literal *Note* escape character sequences \n,\t,\b,\r,\\,\" supported
			case '"':
				i = 0;
				nextCharacter = reader.GetNextCharacter().character;
                    while ((nextCharacter != '"')
                           && (nextCharacter != READER<CALLBACKSUSED>::EOLC)
                           && (nextCharacter != READER<CALLBACKSUSED>::EOPC))
				{
					if (nextCharacter == '\\')
					{
						lexeme[i++] = nextCharacter;
						nextCharacter = reader.GetNextCharacter().character;
						if ((nextCharacter == 'n') ||
							(nextCharacter == 't') ||
							(nextCharacter == 'b') ||
							(nextCharacter == 'r') ||
							(nextCharacter == '\\') ||
							(nextCharacter == '"'))
						{
							lexeme[i++] = nextCharacter;
						}
						else
							ProcessCompilerError(sourceLineNumber, sourceLineIndex,
								"Illegal escape character sequence in string literal");
					}
					else
					{
						lexeme[i++] = nextCharacter;
					}

					nextCharacter = reader.GetNextCharacter().character;
				}

				if (nextCharacter != '"')
					ProcessCompilerError(sourceLineNumber, sourceLineIndex,
						"Un-terminated string literal");
				lexeme[i] = '\0';
				type = STRING;
				reader.GetNextCharacter();
				break;
			case READER<CALLBACKSUSED>::EOPC:
				{
					static int count = 0;

					if (++count > (LOOKAHEAD + 1))
						ProcessCompilerError(sourceLineNumber, sourceLineIndex,
							"Unexpected end-of-program");
					else
					{
						type = EOPTOKEN;
						reader.GetNextCharacter();
						lexeme[0] = '\0';
					}
				}

				break;
			case ',':
				type = COMMA;
				lexeme[0] = nextCharacter; lexeme[1] = '\0';
				reader.GetNextCharacter();
				break;
			case '.':
				type = PERIOD;
				lexeme[0] = nextCharacter; lexeme[1] = '\0';
				reader.GetNextCharacter();
				break;
			case '(':
				type = OPARENTHESIS;
				lexeme[0] = nextCharacter; lexeme[1] = '\0';
				reader.GetNextCharacter();
				break;
			case ')':
				type = CPARENTHESIS;
				lexeme[0] = nextCharacter; lexeme[1] = '\0';
				reader.GetNextCharacter();
				break;
			case ':':
				type = COLON;
				lexeme[0] = nextCharacter;
				reader.GetNextCharacter();
				break;
			case '<':
				lexeme[0] = nextCharacter;
				nextCharacter = reader.GetNextCharacter().character;
				if (nextCharacter == '=')
				{
					type = LTEQ;
					lexeme[1] = nextCharacter;
					lexeme[2] = '\0';
					reader.GetNextCharacter();
				}
				else if (nextCharacter == '>')
				{
					type = NOTEQ;
					lexeme[1] = nextCharacter;
					lexeme[2] = '\0';
					reader.GetNextCharacter();
				}
				else
				{
					type = LT;
					lexeme[1] = '\0';
				}

				break;
			case '=':
				if (reader.GetLookAheadCharacter(1).character == '=')
				{
					nextCharacter = reader.GetNextCharacter().character;
					lexeme[1] = nextCharacter;
					lexeme[2] = '\0';
					reader.GetNextCharacter();
					type = EQEQ;
				}
				else
				{
					type = EQ;
					lexeme[1] = '\0';
					reader.GetNextCharacter();
				}

				break;
			case '>':
				lexeme[0] = nextCharacter;
				nextCharacter = reader.GetNextCharacter().character;
				if (nextCharacter == '=')
				{
					type = GTEQ;
					lexeme[1] = nextCharacter;
					lexeme[2] = '\0';
					reader.GetNextCharacter();
				}
				else
				{
					type = GT;
					lexeme[1] = '\0';
				}

				break;
				// use character look-ahead to "find" '='
			case '!':
				lexeme[0] = nextCharacter; 
				if (reader.GetLookAheadCharacter(1).character == '=')
				{
					nextCharacter = reader.GetNextCharacter().character;  
					lexeme[1] = nextCharacter;
					lexeme[2] = '\0';  
					type = NOTEQ;  
					reader.GetNextCharacter();  // Move past '='
				}
				else
				{
					type = NOT; 
					lexeme[1] = '\0';
					reader.GetNextCharacter();  
				}
				break;

			case '+':
				lexeme[0] = nextCharacter; 
				if (reader.GetLookAheadCharacter(1).character == '\\')
				{
					nextCharacter = reader.GetNextCharacter().character; 
					lexeme[1] = nextCharacter;
					lexeme[2] = '\0';
					type = XOR;  
					reader.GetNextCharacter();  // Move past '\\'
				}
				else if (reader.GetLookAheadCharacter(1).character == '+')
				{
					nextCharacter = reader.GetNextCharacter().character;  
					lexeme[1] = nextCharacter;
					lexeme[2] = '\0';
					type = INC; 
					reader.GetNextCharacter();  // Move past '+'
				}
				else if (reader.GetLookAheadCharacter(1).character == '=')
				{
					nextCharacter = reader.GetNextCharacter().character;  
					lexeme[1] = nextCharacter;
					lexeme[2] = '\0';
					type = EQPLUS;  
					reader.GetNextCharacter();  // Move past '='
				}
				else
				{
					type = PLUS;  
					lexeme[1] = '\0';
					reader.GetNextCharacter(); 
				}
				break;

			case '-':
				lexeme[0] = nextCharacter;  
				if (reader.GetLookAheadCharacter(1).character == '-')
				{
					nextCharacter = reader.GetNextCharacter().character; 
					lexeme[1] = nextCharacter;
					lexeme[2] = '\0';
					type = DEC;  
					reader.GetNextCharacter();  // Move past '-'
				}
				else if (reader.GetLookAheadCharacter(1).character == '=')
				{
					nextCharacter = reader.GetNextCharacter().character;  
					lexeme[1] = nextCharacter;
					lexeme[2] = '\0';
					type = EQMINUS; 
					reader.GetNextCharacter();  // Move past '='
				}
				else
				{
					type = MINUS;  
					lexeme[1] = '\0';
					reader.GetNextCharacter();  
				}
				break;

			case '*':
				lexeme[0] = nextCharacter;  
				if (reader.GetLookAheadCharacter(1).character == '=')
				{
					nextCharacter = reader.GetNextCharacter().character; 
					lexeme[1] = nextCharacter;
					lexeme[2] = '\0';
					type = EQMULTIPLY;  
					reader.GetNextCharacter();  // Move past '='
				}
				else
				{
					type = MULTIPLY; 
					lexeme[1] = '\0';
					reader.GetNextCharacter();  
				}
				break;

			case '/':
				lexeme[0] = nextCharacter; 
				if (reader.GetLookAheadCharacter(1).character == '=')
				{
					nextCharacter = reader.GetNextCharacter().character;  
					lexeme[1] = nextCharacter;
					lexeme[2] = '\0';
					type = EQDIVIDE;  
					reader.GetNextCharacter();  // Move past '='
				}
				else
				{
					type = DIVIDE;  
					lexeme[1] = '\0';
					reader.GetNextCharacter(); 
				}
				break;

			case '%':
				type = MODULUS;
				lexeme[0] = nextCharacter; lexeme[1] = '\0';
				reader.GetNextCharacter();
				break;
			case '^':
				type = POWER;
				lexeme[0] = nextCharacter; lexeme[1] = '\0';
				reader.GetNextCharacter();
				break;
			case ';':
				type = SEMICOLON;
				lexeme[0] = nextCharacter;
				lexeme[1] = '\0';
				reader.GetNextCharacter();
				break;
			case '{':
				type = OCBRACKET;
				lexeme[0] = nextCharacter; lexeme[1] = '\0';
				reader.GetNextCharacter();
				break;
			case '}':
				type = CCBRACKET;
				lexeme[0] = nextCharacter; lexeme[1] = '\0';
				reader.GetNextCharacter();
				break;
			case '[':
				if (reader.GetLookAheadCharacter(1).character == ']')	// --------------- ----  '[]'
				{
					lexeme[0] = nextCharacter;
					nextCharacter = reader.GetNextCharacter().character;	// Move past '['
					lexeme[1] = nextCharacter;	// Capture ']'
					lexeme[2] = '\0';
					type = CONCAT;
					reader.GetNextCharacter();	// Move past ']' 
				}
				else
				{
				 		// Otherwise, it's just an opening square bracket
					lexeme[0] = nextCharacter;
					lexeme[1] = '\0';
					type = OSQRBRACKET;
					reader.GetNextCharacter();
				}

				break;
			case ']':
				type = CSQRBRACKET;
				lexeme[0] = nextCharacter; lexeme[1] = '\0';
				reader.GetNextCharacter();
				break;

			case '\\':
				lexeme[0] = nextCharacter;
				if (reader.GetLookAheadCharacter(1).character == '\\')
				{
					nextCharacter = reader.GetNextCharacter().character;
					lexeme[1] = nextCharacter;
					lexeme[2] = '\0';
					reader.GetNextCharacter();
					type = OR;
				}
				else
				{
					type = UNKTOKEN;
					lexeme[1] = '\0';
					reader.GetNextCharacter();
				}

				break;
			case '&':
				lexeme[0] = nextCharacter;
				if (reader.GetLookAheadCharacter(1).character == '&')
				{
					nextCharacter = reader.GetNextCharacter().character;
					lexeme[1] = nextCharacter;
					lexeme[2] = '\0';
					reader.GetNextCharacter();
					type = AND;
				}
				else
				{
					type = UNKTOKEN;
					lexeme[1] = '\0';
					reader.GetNextCharacter();
				}

				break;
			case '~':
				lexeme[0] = nextCharacter;
				if (reader.GetLookAheadCharacter(1).character == '\\')
				{
					nextCharacter = reader.GetNextCharacter().character;
					lexeme[1] = nextCharacter;
					lexeme[2] = '\0';
					reader.GetNextCharacter();
					type = NOR;
				}
				else if (reader.GetLookAheadCharacter(1).character == '&')
				{
					nextCharacter = reader.GetNextCharacter().character;
					lexeme[1] = nextCharacter;
					lexeme[2] = '\0';
					reader.GetNextCharacter();
					type = NAND;
				}
				else
				{
					type = UNKTOKEN;
					lexeme[1] = '\0';
					reader.GetNextCharacter();
				}

				break;
			default:
				type = UNKTOKEN;
				lexeme[0] = nextCharacter; lexeme[1] = '\0';
				reader.GetNextCharacter();
				break;
		}
	}

	tokens[LOOKAHEAD].type = type;
	strcpy(tokens[LOOKAHEAD].lexeme, lexeme);
	tokens[LOOKAHEAD].sourceLineNumber = sourceLineNumber;
	tokens[LOOKAHEAD].sourceLineIndex = sourceLineIndex;

	#ifdef TRACESCANNER
	sprintf(information, "At (%4d:%3d) token = %12s lexeme = |%s|",
		tokens[LOOKAHEAD].sourceLineNumber,
		tokens[LOOKAHEAD].sourceLineIndex,
		TokenDescription(type), lexeme);
	lister.ListInformationLine(information);
	#endif

}

//-----------------------------------------------------------
const char *TokenDescription(TOKENTYPE type)
//-----------------------------------------------------------
{
	int i;
	bool isFound;

	isFound = false;
	i = 0;
	while (!isFound && (i <= (sizeof(TOKENTABLE) / sizeof(TOKENTABLERECORD)) - 1))
	{
		if (TOKENTABLE[i].type == type)
			isFound = true;
		else
			i++;
	}

	return (isFound ? TOKENTABLE[i].description : "???????");
}
