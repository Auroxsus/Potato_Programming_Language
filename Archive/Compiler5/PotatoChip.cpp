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
#include <stack> // used to break out of loops

using namespace std;

// #define TRACEREADER
// #define TRACESCANNER
 #define TRACEPARSER
//#define TRACEIDENTIFIERTABLE
#define TRACECOMPILER
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
   TO,
   BY,
// Added ***extras*** of SPL5 language
   SWITCH,
   ONE,
   ALL,
   BREAK,
   CASE,
   DEFAULT,
// End added ***extras*** of SPL5 language
	// punctuation
	COMMA,
	PERIOD,
	SEMICOLON,
	OBRACE,
	CBRACE,
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
   { TO          ,"TO"          ,true  },
   { BY          ,"BY"          ,true  },
// Added ***extras*** of SPL5 language
   { SWITCH      ,"TATERTOGGLE"      ,true  }, // rootrelay? PotatoPivot? fryflip
   { CASE        ,"COOK"         ,true  },
   { ONE         ,"ONE"         ,true  },
   { ALL         ,"ALL"         ,true  },
   { BREAK        ,"SERVE"        ,true  }, // :<=
   { DEFAULT         ,"DEFAULT"         ,true  },
// End added ***extras*** of SPL5 language
    { COMMA,          "COMMA",             false },
    { PERIOD,         "PERIOD",            false },
    { COLON,          "COLON",             false },
    { SEMICOLON,      "SEMICOLON",         false },
    { OBRACE,      "OBRACE",         false },
    { CBRACE,      "CBRACE",         false },
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
stack<string> loopExitLabel;


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
	bool isConstant;	// Flag for constants
	bool isInitialized; // Flag to track initialization status
	bool isInTable;	// Flag for identifier exists in table
	bool isInGlobalTable;	// Flag for in global table
	int index;

	EnterModule("DataDefinitions");

	while ((tokens[0].type == CONSTANT) || (tokens[0].type == INTEGER) || (tokens[0].type == BOOLEAN))
	{
		// Initialize flags for each definition
		isConstant = false;
		isInitialized = false;
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
		do { 	
			if (tokens[0].type != IDENTIFIER)
			{
				ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting identifier");
			}

			strcpy(identifier, tokens[0].lexeme);
			GetNextToken(tokens);	// Move to the next token to check for '=' or ';'

			// Expect '=' for constant assignment
			if (isConstant && tokens[0].type != EQ)
			{
				ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting '=' for constant assignment");
			}

			// Handle assignment
			if (tokens[0].type == EQ)
			{
				isInitialized = true; // Mark as initialized
				GetNextToken(tokens);	// Move to the value after '='

				// Handle unary operator for initialization
				bool isNegative = false;
				if (tokens[0].type == MINUS) {
					isNegative = true;
					GetNextToken(tokens);  // Eat '-'
				}else if (tokens[0].type == MINUS){
					GetNextToken(tokens);  // Eat '+' 
				}
				
				// Assign value
				if (dataType == INTTYPE && tokens[0].type == INTEGER)
				{
					strcpy(literal, "0D");
					strcat(literal, tokens[0].lexeme);
					// Add negative sign if applicable
					if (isNegative) {
						char temp[SOURCELINELENGTH + 1];
						strcpy(temp, "-");
						strcat(temp, literal);
						strcpy(literal, temp); // Update literal with negative value
					}
				}
				else if (dataType == BOOLTYPE && (tokens[0].type == TRUE || tokens[0].type == FALSE))
				{
					strcpy(literal, tokens[0].lexeme);
				}
				else
				{
					ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Data type mismatch for constant value");
				}

				GetNextToken(tokens);	// eat '='
			}

			// Check if the identifier is already in the local or global table
			index = identifierTable.GetIndex(identifier, isInTable);

			if (isInTable)
			{
				if (identifierTable.IsInCurrentScope(index))
				{
					ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Multiply-defined identifier in the current scope");
				}
				else if (identifierScope == PROGRAMMODULESCOPE && identifierTable.GetScope(index) == GLOBALSCOPE)
				{
					ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Local variable name conflicts with a global variable");
				}
			}

			// Add the identifier to the table
			if (!isConstant)
			{
				if (isInitialized)
				{
					switch (identifierScope)
					{
						case GLOBALSCOPE:
							code.AddDWToStaticData(literal, identifier, reference);
							identifierTable.AddToTable(identifier, GLOBAL_VARIABLE, dataType, reference);
							break;
						case PROGRAMMODULESCOPE:
							code.AddDWToStaticData(literal, identifier, reference);
							identifierTable.AddToTable(identifier, PROGRAMMODULE_VARIABLE, dataType, reference);
							break;
					}
				} 
				else 
				{
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
    DebugPrintToken(tokens[0], "Entering ParseMAINDefinition");

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

    // Call program body
    sprintf(label, "PROGRAMBODY%04d", code.LabelSuffix());
    code.EmitFormattedLine("", "CALL", label);

    // Termination message
    code.AddDSToStaticData("Normal program termination", "", reference);
    code.EmitFormattedLine("", "PUSHA", reference);
    code.EmitFormattedLine("", "SVC", "#SVC_WRITE_STRING");
    code.EmitFormattedLine("", "SVC", "#SVC_WRITE_ENDL");

    // End program
    code.EmitFormattedLine("", "PUSH", "#0D0", "terminate with status = 0");
    code.EmitFormattedLine("", "SVC", "#SVC_TERMINATE");

    // Program body definition
    code.EmitFormattedLine(label, "EQU", "*");

    GetNextToken(tokens);  // Move past 'crispMAIN'
    DebugPrintToken(tokens[0], "After crispMAIN token");

    identifierTable.EnterNestedStaticScope();
    ParseDataDefinitions(tokens, PROGRAMMODULESCOPE);

    // Check for empty parameter list `()`
    if (tokens[0].type != OPARENTHESIS) {
        ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting '('");
    }
    GetNextToken(tokens);  // Move past `(`
    DebugPrintToken(tokens[0], "After '(' token in MAIN definition");

    if (tokens[0].type != CPARENTHESIS) {
        ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting ')'");
    }
    GetNextToken(tokens);  // Move past `)`
    DebugPrintToken(tokens[0], "After ')' token in MAIN definition");

    // Check for opening curly bracket `{` 
    if (tokens[0].type != OBRACE) {
        ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex,
                             "Expecting opening bracket for MAINCRISP function definition");
    }

    GetNextToken(tokens);  // Move past the opening bracket `{`
    DebugPrintToken(tokens[0], "After '{' token in MAIN definition");

    // Handle **E M P T Y **MAINCRISP block by checking for closing bracket `}`
    if (tokens[0].type == CBRACE) {
        GetNextToken(tokens);  // Move past the closing bracket `}`
    } else {
        // Process statements inside the MAINCRISP block
        while (tokens[0].type != CBRACE && tokens[0].type != EOPTOKEN) {
            ParseStatement(tokens);
            if (tokens[0].type == EOPTOKEN) {
                ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex,
                                     "MAIN: Unexpected end-of-program");
            }
        }
        GetNextToken(tokens);  // Move past the closing bracket `}`
    }

    // Return for body
    code.EmitFormattedLine("", "RETURN");
    code.EmitUnformattedLine("; ****=========");
    sprintf(line, "; ****END (%4d)", tokens[0].sourceLineNumber);
    code.EmitUnformattedLine(line);
    code.EmitUnformattedLine("; ****=========");
	#ifdef TRACECOMPILER
    identifierTable.DisplayTableContents("Contents of identifier table at end of compilation of PROGRAM module definition");
    #endif

    identifierTable.ExitNestedStaticScope();

    ExitModule("PROGRAMDefinition");
}

//-----------------------------------------------------------
void ParseStatement(TOKEN tokens[])
//-----------------------------------------------------------
{
    void ParseAssertion(TOKEN tokens[]);
    void ParsePRINTStatement(TOKEN tokens[]);
    void ParseINPUTStatement(TOKEN tokens[]);
    void ParseAssignmentStatement(TOKEN tokens[], const bool requireSEMICOLON = true);
    void ParseIFStatement(TOKEN tokens[]);
    void ParseDOWHILEStatement(TOKEN tokens[]);
    void ParseFORStatement(TOKEN tokens[]);
    void ParseSWITCHStatement(TOKEN tokens[]);
    void ParseWHILEStatement(TOKEN tokens[]);
    void GetNextToken(TOKEN tokens[]);

    EnterModule("Statement");
    DebugPrintToken(tokens[0], "Entering ParseStatement");

    // Handle assertions enclosed in braces
    while (tokens[0].type == OBRACE) {
        ParseAssertion(tokens);
    }

    switch (tokens[0].type)
    {
        case PRINT:
            ParsePRINTStatement(tokens);
            break;

        case INPUT:
            ParseINPUTStatement(tokens);
            break;

        case IDENTIFIER:
            ParseAssignmentStatement(tokens, true);
            break;

        case INTEGER:
        case BOOLEAN:
        case CONSTANT:
            ParseDataDefinitions(tokens, PROGRAMMODULESCOPE);
            break;

        case IF:
            ParseIFStatement(tokens);
            break;

        case DO:
            ParseDOWHILEStatement(tokens);
            break;

        case FOR:
            ParseFORStatement(tokens);
            break;

        case WHILE:
            ParseWHILEStatement(tokens);
            break;

        case SWITCH:
            ParseSWITCHStatement(tokens);
            break;

        case BREAK: // Handle `serve` as a break
            if (loopExitLabel.empty()) {
                // If there's no loop or switch to break out of, it's an error
                ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Unexpected 'serve' outside of a loop or switch");
            } else {
                // Emit jump to the exit label of the current loop/switch
                code.EmitFormattedLine("", "JMP", loopExitLabel.top().c_str());
                GetNextToken(tokens); // Consume 'serve'
				if (tokens[0].type != SEMICOLON)
					ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Print Expecting ';'");

				GetNextToken(tokens); // eat ';'
				
            }
            break;

        default:
            ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex,
                                 "ParseStatement: Expecting beginning-of-statement");
            break;
    }

    // Handle assertions enclosed in braces (post-statement)
    while (tokens[0].type == OBRACE) {
        ParseAssertion(tokens);
    }

    ExitModule("Statement");
}


//-----------------------------------------------------------
void ParseAssertion(TOKEN tokens[])
//-----------------------------------------------------------
{
   void ParseExpression(TOKEN tokens[], DATATYPE &datatype);
   void GetNextToken(TOKEN tokens[]);

   char line[SOURCELINELENGTH+1];
   DATATYPE datatype;

   EnterModule("Assertion");

   sprintf(line, "; **** %4d: { assertion }", tokens[0].sourceLineNumber);
   code.EmitUnformattedLine(line);

   DebugPrintToken(tokens[0], "Entering ParseAssertion"); // Debug: Entry

   GetNextToken(tokens); // eat '{'
   DebugPrintToken(tokens[0], "After GetNextToken before ParseExpression"); // Debug: Before ParseExpression

   ParseExpression(tokens, datatype);

   DebugPrintToken(tokens[0], "After ParseExpression in ParseAssertion"); // Debug: After ParseExpression

   // STATICSEMANTICS
   if (datatype != BOOLTYPE) {
      DebugPrintToken(tokens[0], "Error: Non-boolean expression in assertion");
      ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting boolean expression");
   }
   // ENDSTATICSEMANTICS

   // CODEGENERATION
   /*
         SETT
         JMPT      E????
         PUSH      #0D(sourceLineNumber)
         PUSH      #0D1
         JMP       HANDLERUNTIMEERROR
   E???? EQU       *
         DISCARD   #0D1
   */
   char Elabel[SOURCELINELENGTH+1], operand[SOURCELINELENGTH+1];

   DebugPrintToken(tokens[0], "Before code generation in ParseAssertion");

   code.EmitFormattedLine("", "SETT");
   sprintf(Elabel, "E%04d", code.LabelSuffix());
   code.EmitFormattedLine("", "JMPT", Elabel);
   sprintf(operand, "#0D%d", tokens[0].sourceLineNumber);
   code.EmitFormattedLine("", "PUSH", operand);
   code.EmitFormattedLine("", "PUSH", "#0D1");
   code.EmitFormattedLine("", "JMP", "HANDLERUNTIMEERROR");
   code.EmitFormattedLine(Elabel, "EQU", "*");
   code.EmitFormattedLine("", "DISCARD", "#0D1");

   DebugPrintToken(tokens[0], "After code generation in ParseAssertion");

   // Check for closing brace
   if (tokens[0].type != CBRACE) {
      DebugPrintToken(tokens[0], "Error: Missing closing brace in assertion");
      ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting }");
   }

	GetNextToken(tokens); // eat '}'
   DebugPrintToken(tokens[0], "After GetNextToken post-closing brace check");

   ExitModule("Assertion");
   DebugPrintToken(tokens[0], "Exiting ParseAssertion");
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

	GetNextToken(tokens); // eat ';'

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

/*
||-----------------------------------------------------------
|| <AssignmentStatement> ::= <Variable> '=' <Expression> [ ':' <Variable> '=' <Expression> ]* ';'
||-----------------------------------------------------------
|| 
|| Features:
|| - Supports multiple left-hand side variables separated by commas.
|| - Ensures compatibility between left-hand and right-hand side data types.
|| - Generates intermediate code for assignment operations.
|| - Optionally requires a terminating semicolon.
|| 
|| Example Code Outline:
||  Assignment Statement:
||  <Variable> = <Expression>
||  <Intermediate Code>
||      MAKEDUP
||      POP       @SP:0D2
||      SWAP
||      DISCARD   #0D1
||      ...
||  End of Code
|| 
|| Syntax Errors:
|| - Missing '=' in the assignment statement.
|| - Data type mismatch between LHS and RHS.
|| - Missing ';' at the end of the statement.
||--------------------------------------------------------
*/
//----------------------------------------------------------
void ParseAssignmentStatement(TOKEN tokens[], const bool requireSEMICOLON = true)
//-----------------------------------------------------------
{
   void ParseVariable(TOKEN tokens[], bool asLValue, DATATYPE &datatype);
   void ParseExpression(TOKEN tokens[], DATATYPE &datatype);
   void GetNextToken(TOKEN tokens[]);

   char line[SOURCELINELENGTH + 1];
   DATATYPE datatypeLHS, datatypeRHS;

   EnterModule("AssignmentStatement");

   sprintf(line, "; **** assignment statement (%4d)", tokens[0].sourceLineNumber);
   code.EmitUnformattedLine(line);

   // Parse the left-hand side variable
   ParseVariable(tokens, true, datatypeLHS);


   bool isRHSSet = false; // Flag to track if RHS has been set
   // Handle additional variables and assignments
   while (tokens[0].type == COLON || tokens[0].type == EQ)
   {
	  if (tokens[0].type == COLON)
	  {
		 // Handle colon-separated variable
		 DATATYPE datatype;

		 GetNextToken(tokens); // eat ':'
		 ParseVariable(tokens, true, datatype);
		 
		 if (datatype != datatypeLHS)
			ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Mixed-mode variables not allowed");
		 
         // Ensure no `=` follows a `:`
         if (tokens[1].type == EQ)
            ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Invalid syntax: '=' cannot follow ':' in assignment");
	  }
	  else if (tokens[0].type == EQ)
	  {
		 GetNextToken(tokens); // eat '='
		 
         // Ensure no `:` follows the `=`
		 if (tokens[0].type == COLON)
			ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Invalid syntax: ':' cannot follow '=' in assignment");
		 
		 ParseExpression(tokens, datatypeRHS); 

		 if (datatypeLHS != datatypeRHS)
			ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Data type mismatch");
	  
		 isRHSSet = true;
	  }
	 code.EmitFormattedLine("", "; Assigning RHS value");
	 code.EmitFormattedLine("", "MAKEDUP");
	 code.EmitFormattedLine("", "POP", "@SP:0D2");
	 code.EmitFormattedLine("", "SWAP");
	 code.EmitFormattedLine("", "DISCARD", "#0D1");
   }
   

   if (isRHSSet)
   {
      code.EmitFormattedLine("", "; Cleaning up the stack");
     code.EmitFormattedLine("", "DISCARD", "#0D1"); // Discard the extra values left on the stack
   }

   
   // Handle optional semicolon for program loops
   if (requireSEMICOLON)
   {
      if (tokens[0].type != SEMICOLON)
         ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting ';'");
      GetNextToken(tokens);
   }

   ExitModule("AssignmentStatement");
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
||
|| Parses an `IF` statement construct from the source tokens.
||
|| Syntax:
|| - `BAKE` introduces the `IF` condition enclosed in parentheses `( )`.
|| - The body of the `IF` clause is enclosed in braces `{ }`.
|| - Zero or more `ELBAKE` clauses (ELIF equivalents) may follow, each with 
||   a condition in parentheses and a body enclosed in braces.
|| - An optional `ELSE` clause may appear, containing a body enclosed in braces.
||
|| Possible Syntax Errors:
|| - Missing '(' or ')' around the condition expression.
|| - Missing '{' after the condition or ELBAKE/ELSE keyword.
|| - Non-boolean condition in the `BAKE` or `ELBAKE` clause.
|| - Missing `}` to close the body of an `IF`, `ELBAKE`, or `ELSE` clause.
||
|| Example Generated Code Outline:
||
|| Initial Condition:
||     <Condition evaluation>  ; Evaluate the first condition
||     SETT                    ; Evaluate condition as true/false
||     DISCARD   #0D1          ; Clean up the stack if necessary
||     JMPNT     Else????      ; Jump to next clause if condition is false
||     <IF statements>         ; Emit code for `IF` block
||     JMP       End????       ; Skip the remaining clauses if condition is true
|| 
|| ELBAKE Clauses:
|| Else????  EQU       *       ; Start of an `ELBAKE` clause
||     <Condition evaluation>  ; Evaluate the condition
||     SETT
||     DISCARD   #0D1
||     JMPNT     Else????      ; Jump to next clause if condition is false
||     <ELBAKE statements>     ; Emit code for `ELBAKE` block
||     JMP       End????       ; Skip remaining clauses if condition is true
|| 
|| ELSE Clause:
|| Else????  EQU       *       ; Start of `ELSE` clause
||     <ELSE statements>       ; Emit code for the `ELSE` block
|| 
|| End:
|| End???? EQU       *         ; Define the end label for the `IF` statement
||
|| Behavior:
|| - Evaluates the condition in the `BAKE` block.
|| - If the condition evaluates to true, executes the body of the `BAKE` block and skips any subsequent `ELBAKE` or `ELSE` blocks.
|| - If the condition evaluates to false:
||     - Moves to the first `ELBAKE` clause, evaluates its condition, and executes its body if true.
||     - Skips to the next `ELBAKE` clause or the `ELSE` block if the condition is false.
|| - If no conditions evaluate to true, and an `ELSE` block is present, executes the `ELSE` block.
|| - Execution skips remaining blocks once a true condition is found and executed.
*/
//-----------------------------------------------------------
void ParseIFStatement(TOKEN tokens[])
//-----------------------------------------------------------
{
	void ParseExpression(TOKEN tokens[], DATATYPE &datatype);
	void ParseStatement(TOKEN tokens[]);
	void GetNextToken(TOKEN tokens[]);

	char line[SOURCELINELENGTH + 1];
	char ElseLabel[SOURCELINELENGTH + 1], EndLabel[SOURCELINELENGTH + 1];
	DATATYPE datatype;

	EnterModule("IFStatement");

	sprintf(line, "; ****IF statement (%4d)", tokens[0].sourceLineNumber);
	code.EmitUnformattedLine(line);

	GetNextToken(tokens);	// eat 'bake'

	if (tokens[0].type != OPARENTHESIS)
		ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting '('");
	GetNextToken(tokens);	// eat '('
	ParseExpression(tokens, datatype);
	if (tokens[0].type != CPARENTHESIS)
		ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting ')'");
	GetNextToken(tokens);	// eat ')'
	if (tokens[0].type != OBRACE)
		ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting '{'");
	GetNextToken(tokens);	// eat '{'

	if (datatype != BOOLTYPE)
		ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting boolean expression");

	// CODEGENERATION
	sprintf(EndLabel, "End%04d", code.LabelSuffix());
	code.EmitFormattedLine("", "SETT");
	code.EmitFormattedLine("", "DISCARD", "#0D1");
	sprintf(ElseLabel, "Else%04d", code.LabelSuffix());
	code.EmitFormattedLine("", "JMPNT", ElseLabel);
	// ENDCODEGENERATION
	
	while ((tokens[0].type != ELIF) &&
			(tokens[0].type != ELSE) &&
			(tokens[0].type != CBRACE))
		ParseStatement(tokens);
		
	GetNextToken(tokens);	// Move past '}' 

	// CODEGENERATION
	code.EmitFormattedLine("", "JMP", EndLabel);
	code.EmitFormattedLine(ElseLabel, "EQU", "*");
	// ENDCODEGENERATION

	while (tokens[0].type == ELIF)
	{
		GetNextToken(tokens);	// eat 'elbake'
		if (tokens[0].type != OPARENTHESIS)
			ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting '(' after ELIF");
		GetNextToken(tokens);	// eat '('
		ParseExpression(tokens, datatype);
		if (tokens[0].type != CPARENTHESIS)
			ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting ')'");
		GetNextToken(tokens);	// eat ')'
		if (tokens[0].type != OBRACE)
			ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting '{' after condition in ELIF (ELBAKE)");
		GetNextToken(tokens);	// eat '{'

		if (datatype != BOOLTYPE)
			ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting boolean expression");

		// CODEGENERATION
		code.EmitFormattedLine("", "SETT");
		code.EmitFormattedLine("", "DISCARD", "#0D1");
		sprintf(ElseLabel, "Else%04d", code.LabelSuffix());
		code.EmitFormattedLine("", "JMPNT", ElseLabel);
		// ENDCODEGENERATION

		while ((tokens[0].type != ELIF) &&
			(tokens[0].type != ELSE) &&
			(tokens[0].type != CBRACE))
			ParseStatement(tokens);

		// CODEGENERATION
		code.EmitFormattedLine("", "JMP", EndLabel);
		code.EmitFormattedLine(ElseLabel, "EQU", "*");
		// ENDCODEGENERATION

		GetNextToken(tokens);	// Move past ELIF block's '}' 
	}

	// Handle `ELSE` clause
	if (tokens[0].type == ELSE)
	{
		GetNextToken(tokens);	// move past 'else'

		if (tokens[0].type != OBRACE)
			ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting '{' after ELSE");

		GetNextToken(tokens);	// eat '{'

		while (tokens[0].type != CBRACE)
		{
			ParseStatement(tokens);
		}

		GetNextToken(tokens);	// eat '}' 
	}

	// CODEGENERATION
	code.EmitFormattedLine(EndLabel, "EQU", "*");
	// ENDCODEGENERATION

	ExitModule("IFStatement");
}

/*
||-----------------------------------------------------------
|| <DOWHILEStatement> ::= DO
||                              { <statement> }*
||                         fryAsLongAs ( <expression> ) ;
||-----------------------------------------------------------
||
|| Syntax:
|| - The loop body is enclosed in `{}` and must follow the `do` keyword.
|| - The condition follows the `fryAsLongAs` keyword and must be a boolean 
||   expression enclosed in `()` and terminated with `;`.
||
|| Possible Syntax Errors:
|| - Missing '{' after the 'do' keyword.
|| - Missing 'fryAsLongAs' keyword after the loop body.
|| - Missing '(' and ')' around the condition expression.
|| - Missing ';' at the end of the statement.
|| - Non-boolean condition in the 'while' clause.
||
|| Example Generated Code Outline:
||
|| Loop Body:
|| Body????  EQU       *       ; Start of the loop body
||     <Body statements>       ; Emit code for all statements in the loop body
|| 
|| Condition Check:
||     <Condition evaluation>  ; Evaluate the condition at the end of the body
||     SETT                    ; Evaluate condition as true/false
||     DISCARD   #0D1          ; Clean up the stack if necessary
||     JMPNT     End????       ; Exit the loop if condition is false
|| 
|| Repeat Loop:
||     JMP       Body????      ; Jump back to the loop body for the next iteration
|| 
|| Loop End:
|| End????  EQU       *        ; Define the end label for the loop
||
|| Behavior:
|| - Executes the loop body at least once, regardless of the condition.
|| - Evaluates the condition after the loop body has been executed.
|| - If the condition evaluates to true, the loop body is executed again.
|| - If the condition evaluates to false, the loop terminates.
|| - Ensures the loop body always runs once before checking the condition.
*/
//-----------------------------------------------------------
void ParseDOWHILEStatement(TOKEN tokens[])
//-----------------------------------------------------------
{
	void ParseExpression(TOKEN tokens[], DATATYPE &datatype);
	void ParseStatement(TOKEN tokens[]);
	void GetNextToken(TOKEN tokens[]);

	char line[SOURCELINELENGTH + 1];
	char BodyLabel[SOURCELINELENGTH + 1], EndLabel[SOURCELINELENGTH + 1];
	DATATYPE datatype;

	EnterModule("DOWHILEStatement");

	sprintf(line, "; ****DO-WHILE statement (%4d)", tokens[0].sourceLineNumber);
	code.EmitUnformattedLine(line);

	GetNextToken(tokens); // eat 'do'

    // CODEGENERATION
	sprintf(BodyLabel, "Body%04d", code.LabelSuffix());
	sprintf(EndLabel, "End%04d", code.LabelSuffix());
	code.EmitFormattedLine(BodyLabel, "EQU", "*");
	// ENDCODEGENERATION

   loopExitLabel.push(EndLabel); // Push the exit label for this loop

	if (tokens[0].type != OBRACE)
		ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting '{'");
	GetNextToken(tokens); // eat '{'
	
	while (tokens[0].type != CBRACE)
	{
		if (tokens[0].type == BREAK) { // Handle `serve` as break
			GetNextToken(tokens); // Eat 'serve'
			code.EmitFormattedLine("", "JMP", EndLabel); // Jump to loop exit
		} else {
			ParseStatement(tokens); // Parse regular statements
		}
	}
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
	code.EmitFormattedLine("", "JMPNT", EndLabel);
	code.EmitFormattedLine("", "JMP", BodyLabel);
	code.EmitFormattedLine(EndLabel,"EQU","*");
	// ENDCODEGENERATION
	
	loopExitLabel.pop(); // Pop the exit label for this loop

	ExitModule("DOWHILEStatement");
}

/*
||-----------------------------------------------------------
|| <FORStatement> ::= SEASON ( <initialization> ; <condition> ; <iteration> )
||                        { <statement> }* 
||
|| <initialization> ::= <assignment> ( : <assignment> )*
||
|| <condition> ::= <boolean_expression>
|| 
|| <iteration> ::= <assignment> ( : <assignment> )*
||-----------------------------------------------------------
||
|| Syntax:
|| - The `SEASON` loop consists of three main sections enclosed in parentheses `()`:
||     1. <initialization>: Executes once at the start of the loop.
||     2. <condition>: Evaluates before each iteration; the loop continues if true.
||     3. <iteration>: Executes at the end of each iteration.
|| - Sections are separated by semicolons (`;`), and the loop body is enclosed in `{}`.
|| - All sections must be explicitly defined. Missing any section will result in a syntax error.
|| 
|| Possible Syntax Errors:
|| - Missing '(' or ')' around initialization, condition, or iteration.
|| - Missing `;` separators between initialization, condition, or iteration.
|| - Missing '{' or '}' around the loop body.
|| - Non-boolean condition in the 'SEASON' condition clause.
|| - Use of `serve` outside of any loop context.
|| 
|| Example Generated Code Outline:
||
|| Initialization:
|| Init???? EQU       *       ; Start of initialization
||     <Initialization code>
||     JMP       Cond????    ; Jump to condition
|| 
|| Condition:
|| Cond???? EQU       *       ; Condition label
||     <Condition evaluation>
||     JMPT      Body????     ; Jump to loop body if condition true 
||     JMP       End????      ; Exit loop if condition false
|| 
|| Iteration:
|| Iter???? EQU       *       ; Iteration label
||     <Iteration code>
||     JMP       Cond????     ; Jump back to condition block
|| 
|| Body:
|| Body???? EQU       *       ; Loop body label
||     <Body statements>
||     JMP       Iter????     ; Jump to iteration block
||     `serve` Keyword:
||         - Jumps to End???? when encountered inside the loop body.
|| 
|| End:
|| End???? EQU       *        ; End label
|| 
|| Behavior:
|| - Executes the initialization block once, followed by the condition check.
|| - If the condition evaluates to true, executes the loop body and then the iteration block.
|| - Repeats until the condition evaluates to false or the `serve` keyword is encountered.
|| - Supports nested loops with independent scoping of `serve` exit points.
*/ 

//-----------------------------------------------------------
void ParseFORStatement(TOKEN tokens[])
//-----------------------------------------------------------
 {
    void ParseAssignmentStatement(TOKEN tokens[], const bool requireSEMICOLON = true);
    void ParseExpression(TOKEN tokens[], DATATYPE &datatype);
    void ParseStatement(TOKEN tokens[]);
    void GetNextToken(TOKEN tokens[]);
	  
	char line[SOURCELINELENGTH + 1];
    char InitLabel[SOURCELINELENGTH + 1], CondLabel[SOURCELINELENGTH + 1],
         IterLabel[SOURCELINELENGTH + 1], BodyLabel[SOURCELINELENGTH + 1],
         EndLabel[SOURCELINELENGTH + 1];
    DATATYPE datatype;

    EnterModule("FORStatement");
	
   sprintf(line,"; **** FOR statement (%4d)",tokens[0].sourceLineNumber);
   code.EmitUnformattedLine(line);

    GetNextToken(tokens); // Eat 'season'

    if (tokens[0].type != OPARENTHESIS) {
        ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting '('");
    }
    GetNextToken(tokens); // Eat '('
   
    // Generate Labels
    sprintf(InitLabel, "Init%04d", code.LabelSuffix());  // Initialization
    sprintf(CondLabel, "Cond%04d", code.LabelSuffix());  // Condition
    sprintf(IterLabel, "Iter%04d", code.LabelSuffix());  // Iteration
    sprintf(BodyLabel, "Body%04d", code.LabelSuffix());  // Loop Body
    sprintf(EndLabel, "End%04d", code.LabelSuffix());    // End of Loop

    // Initialization
    code.EmitFormattedLine(InitLabel, "EQU", "*");       // Define initialization label
    ParseAssignmentStatement(tokens, false);             // Parse the initialization statement
    code.EmitFormattedLine("", "JMP", CondLabel);        // Jump to the condition
  
    if (tokens[0].type != SEMICOLON) {
	  ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting ';' after initialization");
    }
    GetNextToken(tokens); // Eat ';'
	   
    // Condition
    code.EmitFormattedLine(CondLabel, "EQU", "*");       // Define condition label
    ParseExpression(tokens, datatype);                   // Parse the condition expression

    if (datatype != BOOLTYPE) {
        ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Condition must evaluate to a boolean type");
    }
   
    if (tokens[0].type != SEMICOLON) {
         ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting ';' after condition");
    }
    GetNextToken(tokens); // Eat ';'
	
	// Emit condition jump logic
    code.EmitFormattedLine("", "SETT", "");              // Evaluate condition
    code.EmitFormattedLine("","DISCARD","#0D1","(empty)");
    code.EmitFormattedLine("", "JMPT", BodyLabel);       // Jump to body if true 
    code.EmitFormattedLine("", "JMP", EndLabel);         // Exit loop if false

    // Iteration
    code.EmitFormattedLine(IterLabel, "EQU", "*");       // Define iteration label
	ParseAssignmentStatement(tokens, false);             // Parse the iteration statement
    code.EmitFormattedLine("", "JMP", CondLabel);        // Jump back to condition

    if (tokens[0].type != CPARENTHESIS) {
        ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting ')'");
    }
    GetNextToken(tokens); // Eat ')'

	// Push the exit label for this loop
	loopExitLabel.push(EndLabel);
    
	// Body
    if (tokens[0].type != OBRACE) {
        ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting '{'");
    }
    GetNextToken(tokens); // Eat '{'

    code.EmitFormattedLine(BodyLabel, "EQU", "*");       // Define body label
	  while (tokens[0].type != CBRACE) {
		 if (tokens[0].type == BREAK) { // Handle `serve` as break
			   GetNextToken(tokens); // Eat 'serve'
			   code.EmitFormattedLine("", "JMP", EndLabel); // Jump to loop exit
		} else {
			ParseStatement(tokens);
		}
	}
    GetNextToken(tokens); // Eat '}'

    // Emit jump to iteration **after the body finishes**
    code.EmitFormattedLine("", "JMP", IterLabel);        // Jump to iteration block
    code.EmitFormattedLine(EndLabel, "EQU", "*");        // Define end label
	
	// Pop the exit label for this loop
	loopExitLabel.pop();

    ExitModule("FORStatement");
}

//-----------------------------------------------------------
void ParseSWITCHStatement(TOKEN tokens[])
//-----------------------------------------------------------
{
/*
||-----------------------------------------------------------
|| <SWITCHStatement> ::= TATERTOGGLE [ (( ONE | ALL )) ] {
||                          { COOK <expression> : { <statement> }* <break_statment> }*
||                       }
||    *Notes* 
||       <expression>s are evaluated one-at-a-time from top-to-bottom
||       <expression>s must be boolean
||       mode ONE: (default) means execute <statement>s for only ONE <expression> WHEN true
||       mode ALL: execute <statement>s for ALL <expression>s WHEN true
||       ELSE means execute <statement>s when no <expression>s are true
||-----------------------------------------------------------
*/
   void ParseExpression(TOKEN tokens[],DATATYPE &datatype);
   void ParseStatement(TOKEN tokens[]);
   void GetNextToken(TOKEN tokens[]);

   char line[SOURCELINELENGTH+1];
   char Ilabel[SOURCELINELENGTH+1],Elabel1[SOURCELINELENGTH+1],Elabel2[SOURCELINELENGTH+1];
   DATATYPE datatype;
   char mode; // 'O' = ONE, 'A' = ALL

   EnterModule("SWITCHStatement");

   sprintf(line,"; **** SWITCH statement (%4d)",tokens[0].sourceLineNumber);
   code.EmitUnformattedLine(line);

   GetNextToken(tokens); // eat tattertoggle?

   if       ( tokens[0].type == ONE )
   {
      mode = 'O';
      GetNextToken(tokens);
   } else if ( tokens[0].type == ALL )
   {
      mode = 'A';
      GetNextToken(tokens);
   } else
   {
      mode = 'O'; // default
   }
// CODEGENERATION
   sprintf(Elabel1,"E%04d",code.LabelSuffix());
   sprintf(Elabel2,"E%04d",code.LabelSuffix());
   code.EmitFormattedLine("","PUSH","#0D0","count-of-true <expressions> := 0");
// ENDCODEGENERATION

   while ( tokens[0].type == CASE )
   {
      GetNextToken(tokens); // eat 'cook'
      ParseExpression(tokens,datatype);
      if ( datatype != BOOLTYPE )
         ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting boolean expression");
      if ( tokens[0].type != COLON )
         ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting ':'");
      GetNextToken(tokens); // eat ':'

// CODEGENERATION
      code.EmitFormattedLine("","SETT");
      code.EmitFormattedLine("","DISCARD","#0D1");
      sprintf(Ilabel,"I%04d",code.LabelSuffix());
      code.EmitFormattedLine("","JMPNT",Ilabel);
      code.EmitFormattedLine("","PUSH","#0D1","increment count-of-true <expressions>");
      code.EmitFormattedLine("","ADDI");
// ENDCODEGENERATION

      while ( (tokens[0].type != CASE)    &&
			  (tokens[0].type != DEFAULT) &&
			  (tokens[0].type != CBRACE)) {
          if (tokens[0].type == BREAK) { 
                GetNextToken(tokens);      // eat SERVE
                if (mode == 'O') {
                    code.EmitFormattedLine("", "JMP", Elabel1); // Exit switch
                } else {
                    code.EmitFormattedLine("", "JMP", Elabel2); // Exit case in ALL mode
                }
                break; // Exit current case parsing
            } else {
                ParseStatement(tokens); // Parse regular statements
		 }
			  }

// CODEGENERATION
      if ( mode == 'O' )
         code.EmitFormattedLine("","JMP",Elabel1);
      code.EmitFormattedLine(Ilabel,"EQU","*");
// ENDCODEGENERATION
   }
   
   // Handle DEFAULT case
    if (tokens[0].type == DEFAULT) {
       while (tokens[0].type != CBRACE) {
            if (tokens[0].type == BREAK) { // Handle 'serve' in DEFAULT
                GetNextToken(tokens);      // eat SERVE
                code.EmitFormattedLine("", "JMP", Elabel2); // Exit default
                break;
            } else {
                ParseStatement(tokens);
            }
    }
}

if (tokens[0].type != CBRACE) {
ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting '}'");
    }
    GetNextToken(tokens);

// CODEGENERATION
   code.EmitFormattedLine(Elabel1,"EQU","*");
   code.EmitFormattedLine("","SETNZPI","","if ( count-of-true <expressions> = 0 ) goto E???2");
   code.EmitFormattedLine("","DISCARD","#0D1");
   code.EmitFormattedLine("","JMPNZ",Elabel2);
      code.EmitFormattedLine(Elabel2,"EQU","*");
// ENDCODEGENERATION

   ExitModule("SWITCHStatement");
}

/*
||-----------------------------------------------------------
||<WHILEStatement>      ::= fryAsLongAs( < expression> )
||                              {<statement> }*
||-----------------------------------------------------------
*/

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
//-----------------------------------------------------------
void ParseWHILEStatement(TOKEN tokens[]) 
//-----------------------------------------------------------
{
    void ParseExpression(TOKEN tokens[], DATATYPE &datatype);
    void ParseStatement(TOKEN tokens[]);
    void GetNextToken(TOKEN tokens[]);

    char line[SOURCELINELENGTH + 1];
    char BodyLabel[SOURCELINELENGTH + 1], EndLabel[SOURCELINELENGTH + 1];
    DATATYPE datatype;

    EnterModule("WHILEStatement");

    sprintf(line, "; ****WHILE statement (%4d)", tokens[0].sourceLineNumber);
    code.EmitUnformattedLine(line);

    GetNextToken(tokens); // Move past 'fryAsLongAs'

    // CODEGENERATION
    sprintf(BodyLabel, "Body%04d", code.LabelSuffix());
    sprintf(EndLabel, "End%04d", code.LabelSuffix());
    code.EmitFormattedLine(BodyLabel, "EQU", "*");
    // ENDCODEGENERATION

    loopExitLabel.push(EndLabel); // Push the exit label for this loop

    if (tokens[0].type != OPARENTHESIS)
        ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting '('");
    GetNextToken(tokens); // Move past '('
    ParseExpression(tokens, datatype);
    if (tokens[0].type != CPARENTHESIS)
        ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting ')'");
    GetNextToken(tokens); // Move past ')'

    if (datatype != BOOLTYPE)
        ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting boolean expression");

    // CODEGENERATION
    code.EmitFormattedLine("", "SETT");
    code.EmitFormattedLine("", "DISCARD", "#0D1");
    code.EmitFormattedLine("", "JMPNT", EndLabel);
    // ENDCODEGENERATION

    if (tokens[0].type != OBRACE)
        ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting '{'");
    GetNextToken(tokens); // Move past '{'

    while (tokens[0].type != CBRACE) {
        ParseStatement(tokens); // Handles `serve` internally
    }
    GetNextToken(tokens); // Move past '}'

    // CODEGENERATION
    code.EmitFormattedLine("", "JMP", BodyLabel);
    code.EmitFormattedLine(EndLabel, "EQU", "*");
    // ENDCODEGENERATION

    loopExitLabel.pop(); // Pop the exit label for this loop

    ExitModule("WHILEStatement");
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

    // Debug: Start of Expression
    DebugPrintToken(tokens[0], "Entering ParseExpression");

    ParseConjunction(tokens, datatypeLHS);

    // Debug: After first conjunction in expression
    DebugPrintToken(tokens[0], "After parsing first conjunction in ParseExpression");

    if ((tokens[0].type == OR) ||
        (tokens[0].type == NOR) ||
        (tokens[0].type == XOR))
    {
        while ((tokens[0].type == OR) ||
               (tokens[0].type == NOR) ||
               (tokens[0].type == XOR))
        {
            TOKENTYPE operation = tokens[0].type;

            // Debug: Logical operation detected
            DebugPrintToken(tokens[0], "Logical operation in ParseExpression");

            GetNextToken(tokens);
            ParseConjunction(tokens, datatypeRHS);

            // Debug: After parsing conjunction in logical operation
            DebugPrintToken(tokens[0], "After parsing conjunction for logical operation in ParseExpression");

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
                    if (!((datatypeLHS == BOOLTYPE) && (datatypeRHS == BOOLTYPE)))
                        ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting boolean operands");

                    code.EmitFormattedLine("", "NOR");
                    datatype = BOOLTYPE;
                    break;
                case XOR:
                    if (!((datatypeLHS == BOOLTYPE) && (datatypeRHS == BOOLTYPE)))
                        ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting boolean operands");

                    code.EmitFormattedLine("", "XOR");
                    datatype = BOOLTYPE;
                    break;
            }
        }

        // Debug: End of logical operation block
        DebugPrintToken(tokens[0], "End of logical operation block in ParseExpression");
    }
    else
    {
        datatype = datatypeLHS;
        // Debug: No logical operation, single conjunction
        DebugPrintToken(tokens[0], "Single conjunction detected in ParseExpression");
    }

    // Debug: Exiting ParseExpression
    DebugPrintToken(tokens[0], "Exiting ParseExpression");

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

	// Debug: Entering Conjunction
	DebugPrintToken(tokens[0], "Entering ParseConjunction");

	ParseNegation(tokens, datatypeLHS);

	// Debug: After first negation
	DebugPrintToken(tokens[0], "After parsing first negation in ParseConjunction");

	if ((tokens[0].type == AND) ||
		(tokens[0].type == NAND))
	{
		// Debug: Logical operation detected
		DebugPrintToken(tokens[0], "Logical operation detected in ParseConjunction");

		while ((tokens[0].type == AND) ||
			(tokens[0].type == NAND))
		{
			TOKENTYPE operation = tokens[0].type;

			// Debug: Current logical operation
			DebugPrintToken(tokens[0], "Processing logical operation");

			GetNextToken(tokens);
			ParseNegation(tokens, datatypeRHS);

			// Debug: After parsing RHS negation
			DebugPrintToken(tokens[0], "After parsing right-hand side negation in ParseConjunction");

			switch (operation)
			{
				case AND:
					if (!((datatypeLHS == BOOLTYPE) && (datatypeRHS == BOOLTYPE)))
						ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting boolean operands");
					// Debug: Emitting AND operation
					DebugPrintToken(tokens[0], "Emitting AND operation");
					code.EmitFormattedLine("", "AND");
					datatype = BOOLTYPE;
					break;
				case NAND:
					if (!((datatypeLHS == BOOLTYPE) && (datatypeRHS == BOOLTYPE)))
						ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting boolean operands");
					// Debug: Emitting NAND operation
					DebugPrintToken(tokens[0], "Emitting NAND operation");
					code.EmitFormattedLine("", "NAND");
					datatype = BOOLTYPE;
					break;
			}
		}
	}
	else
	{
		// Debug: No logical operation, single negation
		DebugPrintToken(tokens[0], "Single negation detected, no logical operation in ParseConjunction");
		datatype = datatypeLHS;
	}

	// Debug: Exiting Conjunction
	DebugPrintToken(tokens[0], "Exiting ParseConjunction");

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

	// Debug: Entering Negation
	DebugPrintToken(tokens[0], "Entering ParseNegation");

	if (tokens[0].type == NOT)
	{
		// Debug: NOT operator detected
		DebugPrintToken(tokens[0], "NOT operator detected in ParseNegation");

		GetNextToken(tokens);

		// Debug: Before parsing comparison in negation
		DebugPrintToken(tokens[0], "Before ParseComparison in ParseNegation");

		ParseComparison(tokens, datatypeRHS);

		// Debug: After parsing comparison in negation
		DebugPrintToken(tokens[0], "After ParseComparison in ParseNegation");

		if (!(datatypeRHS == BOOLTYPE))
		{
			// Debug: Error for non-boolean operand
			DebugPrintToken(tokens[0], "Error: Expecting boolean operand in ParseNegation");
			ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting boolean operand");
		}

		// Debug: Emitting NOT operation
		DebugPrintToken(tokens[0], "Emitting NOT operation in ParseNegation");

		code.EmitFormattedLine("", "NOT");
		datatype = BOOLTYPE;
	}
	else
	{
		// Debug: No NOT operator, parsing comparison
		DebugPrintToken(tokens[0], "No NOT operator, parsing comparison in ParseNegation");

		ParseComparison(tokens, datatype);
	}

	// Debug: Exiting Negation
	DebugPrintToken(tokens[0], "Exiting ParseNegation");

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

	// Debug: Entering Comparison
	DebugPrintToken(tokens[0], "Entering ParseComparison");

	ParseComparator(tokens, datatypeLHS);

	// Debug: After parsing LHS comparator
	DebugPrintToken(tokens[0], "After parsing left-hand side comparator in ParseComparison");

	if ((tokens[0].type == LT) ||
		(tokens[0].type == LTEQ) ||
		(tokens[0].type == EQEQ) ||
		(tokens[0].type == GT) ||
		(tokens[0].type == GTEQ) ||
		(tokens[0].type == NOTEQ))
	{
		// Debug: Comparison operator detected
		DebugPrintToken(tokens[0], "Comparison operator detected in ParseComparison");

		TOKENTYPE operation = tokens[0].type;

		GetNextToken(tokens);

		// Debug: Before parsing RHS comparator
		DebugPrintToken(tokens[0], "Before parsing right-hand side comparator in ParseComparison");

		ParseComparator(tokens, datatypeRHS);

		// Debug: After parsing RHS comparator
		DebugPrintToken(tokens[0], "After parsing right-hand side comparator in ParseComparison");

		if ((datatypeLHS != INTTYPE) || (datatypeRHS != INTTYPE))
		{
			// Debug: Error for non-integer operands
			DebugPrintToken(tokens[0], "Error: Expecting integer operands in ParseComparison");
			ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting integer operands");
		}

		// Debug: Emitting comparison code
		DebugPrintToken(tokens[0], "Emitting comparison operation in ParseComparison");

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
	{
		// Debug: No comparison operator detected
		DebugPrintToken(tokens[0], "No comparison operator detected, using single comparator in ParseComparison");
		datatype = datatypeLHS;
	}

	// Debug: Exiting Comparison
	DebugPrintToken(tokens[0], "Exiting ParseComparison");

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

	// Debug: Entering Comparator
	DebugPrintToken(tokens[0], "Entering ParseComparator");

	ParseTerm(tokens, datatypeLHS);

	// Debug: After parsing LHS term
	DebugPrintToken(tokens[0], "After parsing left-hand side term in ParseComparator");

	if ((tokens[0].type == PLUS) || (tokens[0].type == MINUS))
	{
		// Debug: Arithmetic operation detected
		DebugPrintToken(tokens[0], "Arithmetic operation detected in ParseComparator");

		while ((tokens[0].type == PLUS) || (tokens[0].type == MINUS))
		{
			TOKENTYPE operation = tokens[0].type;

			// Debug: Processing arithmetic operation
			DebugPrintToken(tokens[0], "Processing arithmetic operation in ParseComparator");

			GetNextToken(tokens);
			ParseTerm(tokens, datatypeRHS);

			// Debug: After parsing RHS term
			DebugPrintToken(tokens[0], "After parsing right-hand side term in ParseComparator");

			if ((datatypeLHS != INTTYPE) || (datatypeRHS != INTTYPE))
			{
				// Debug: Error for non-integer operands
				DebugPrintToken(tokens[0], "Error: Expecting integer operands in ParseComparator");
				ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting integer operands");
			}

			switch (operation)
			{
				case PLUS:
					// Debug: Emitting ADDI operation
					DebugPrintToken(tokens[0], "Emitting ADDI operation in ParseComparator");
					code.EmitFormattedLine("", "ADDI");
					break;
				case MINUS:
					// Debug: Emitting SUBI operation
					DebugPrintToken(tokens[0], "Emitting SUBI operation in ParseComparator");
					code.EmitFormattedLine("", "SUBI");
					break;
			}

			datatype = INTTYPE;
		}
	}
	else
	{
		// Debug: No arithmetic operator, single term
		DebugPrintToken(tokens[0], "Single term detected, no arithmetic operator in ParseComparator");
		datatype = datatypeLHS;
	}

	// Debug: Exiting Comparator
	DebugPrintToken(tokens[0], "Exiting ParseComparator");

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

	// Debug: Entering Term
	DebugPrintToken(tokens[0], "Entering ParseTerm");

	ParseFactor(tokens, datatypeLHS);

	// Debug: After parsing LHS factor
	DebugPrintToken(tokens[0], "After parsing left-hand side factor in ParseTerm");

	if ((tokens[0].type == MULTIPLY) || (tokens[0].type == DIVIDE) || (tokens[0].type == MODULUS))
	{
		// Debug: Multiplicative operation detected
		DebugPrintToken(tokens[0], "Multiplicative operation detected in ParseTerm");

		while ((tokens[0].type == MULTIPLY) || (tokens[0].type == DIVIDE) || (tokens[0].type == MODULUS))
		{
			TOKENTYPE operation = tokens[0].type;

			// Debug: Processing multiplicative operation
			DebugPrintToken(tokens[0], "Processing multiplicative operation in ParseTerm");

			GetNextToken(tokens);
			ParseFactor(tokens, datatypeRHS);

			// Debug: After parsing RHS factor
			DebugPrintToken(tokens[0], "After parsing right-hand side factor in ParseTerm");

			if ((datatypeLHS != INTTYPE) || (datatypeRHS != INTTYPE))
			{
				// Debug: Error for non-integer operands
				DebugPrintToken(tokens[0], "Error: Expecting integer operands in ParseTerm");
				ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting integer operands");
			}

			switch (operation)
			{
				case MULTIPLY:
					// Debug: Emitting MULI operation
					DebugPrintToken(tokens[0], "Emitting MULI operation in ParseTerm");
					code.EmitFormattedLine("", "MULI");
					break;
				case DIVIDE:
					// Debug: Emitting DIVI operation
					DebugPrintToken(tokens[0], "Emitting DIVI operation in ParseTerm");
					code.EmitFormattedLine("", "DIVI");
					break;
				case MODULUS:
					// Debug: Emitting REMI operation
					DebugPrintToken(tokens[0], "Emitting REMI operation in ParseTerm");
					code.EmitFormattedLine("", "REMI");
					break;
			}

			datatype = INTTYPE;
		}
	}
	else
	{
		// Debug: No multiplicative operator, single factor
		DebugPrintToken(tokens[0], "Single factor detected, no multiplicative operator in ParseTerm");
		datatype = datatypeLHS;
	}

	// Debug: Exiting Term
	DebugPrintToken(tokens[0], "Exiting ParseTerm");

	ExitModule("Term");
}

//-----------------------------------------------------------
void ParseFactor(TOKEN tokens[], DATATYPE &datatype)
//-----------------------------------------------------------
{
	void ParseSecondary(TOKEN tokens[], DATATYPE &datatype);
	void GetNextToken(TOKEN tokens[]);

	EnterModule("Factor");

	// Debug: Entering Factor
	DebugPrintToken(tokens[0], "Entering ParseFactor");

	if ((tokens[0].type == ABS) || (tokens[0].type == PLUS) || (tokens[0].type == MINUS))
	{
		// Debug: Unary operator detected
		DebugPrintToken(tokens[0], "Unary operator detected in ParseFactor");

		DATATYPE datatypeRHS;
		TOKENTYPE operation = tokens[0].type;

		GetNextToken(tokens);

		// Debug: Before parsing secondary in unary operation
		DebugPrintToken(tokens[0], "Before ParseSecondary in unary operation");

		ParseSecondary(tokens, datatypeRHS);

		// Debug: After parsing secondary in unary operation
		DebugPrintToken(tokens[0], "After ParseSecondary in unary operation");

		if (datatypeRHS != INTTYPE)
		{
			// Debug: Error for non-integer operand
			DebugPrintToken(tokens[0], "Error: Expecting integer operand in ParseFactor");
			ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting integer operand");
		}

		switch (operation)
		{
			case ABS:
				// Debug: Emitting ABS operation
				DebugPrintToken(tokens[0], "Emitting ABS operation in ParseFactor");
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
				// Debug: PLUS operation (no action required)
				DebugPrintToken(tokens[0], "PLUS operation detected (no action required) in ParseFactor");
				break;
			case MINUS:
				// Debug: Emitting NEG operation
				DebugPrintToken(tokens[0], "Emitting NEG operation in ParseFactor");
				code.EmitFormattedLine("", "NEGI");
				break;
		}

		datatype = INTTYPE;
	}
	else
	{
		// Debug: No unary operator, parsing secondary
		DebugPrintToken(tokens[0], "No unary operator detected, parsing secondary in ParseFactor");
		ParseSecondary(tokens, datatype);
	}

	// Debug: Exiting Factor
	DebugPrintToken(tokens[0], "Exiting ParseFactor");

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

	// Debug: Entering Secondary
	DebugPrintToken(tokens[0], "Entering ParseSecondary");

	ParsePrefix(tokens, datatypeLHS);

	// Debug: After parsing Prefix in Secondary
	DebugPrintToken(tokens[0], "After parsing Prefix in ParseSecondary");

	if (tokens[0].type == POWER)
	{
		// Debug: POWER operator detected
		DebugPrintToken(tokens[0], "POWER operator detected in ParseSecondary");

		GetNextToken(tokens);	// Move past the POWER operator

		// Debug: Before parsing RHS Prefix in Secondary
		DebugPrintToken(tokens[0], "Before parsing right-hand side Prefix in ParseSecondary");

		ParsePrefix(tokens, datatypeRHS);

		// Debug: After parsing RHS Prefix in Secondary
		DebugPrintToken(tokens[0], "After parsing right-hand side Prefix in ParseSecondary");

		if ((datatypeLHS != INTTYPE) || (datatypeRHS != INTTYPE))
		{
			// Debug: Error for non-integer operands
			DebugPrintToken(tokens[0], "Error: Expecting integer operands in ParseSecondary");
			ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting integer operands");
		}

		// Debug: Emitting POWER operation
		DebugPrintToken(tokens[0], "Emitting POWER operation in ParseSecondary");
		code.EmitFormattedLine("", "POWI");
		datatype = INTTYPE;
	}
	else
	{
		// Debug: No POWER operator, parsing Postfix
		DebugPrintToken(tokens[0], "No POWER operator detected, parsing Postfix in ParseSecondary");

		ParsePostfix(tokens, datatypeLHS);
		datatype = datatypeLHS;
	}

	// Debug: Exiting Secondary
	DebugPrintToken(tokens[0], "Exiting ParseSecondary");

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

	// Debug: Entering Prefix
	DebugPrintToken(tokens[0], "Entering ParsePrefix");

	if ((tokens[0].type == INC) || (tokens[0].type == DEC))
	{
		// Debug: INC/DEC operator detected
		DebugPrintToken(tokens[0], "INC/DEC operator detected in ParsePrefix");

		DATATYPE datatypeRHS;
		TOKENTYPE operation = tokens[0].type;

		GetNextToken(tokens);

		if ((tokens[0].type == PLUS) || (tokens[0].type == MINUS))
		{
			// Debug: Skipping extra unary operator
			DebugPrintToken(tokens[0], "Skipping extra unary operator in ParsePrefix");
			GetNextToken(tokens);	// get extra +
		}

		// Debug: Before parsing variable in Prefix
		DebugPrintToken(tokens[0], "Before parsing variable in ParsePrefix");

		ParseVariable(tokens, true, datatypeRHS);

		// Debug: After parsing variable in Prefix
		DebugPrintToken(tokens[0], "After parsing variable in ParsePrefix");

		if (datatypeRHS != INTTYPE)
		{
			// Debug: Error for non-integer operand
			DebugPrintToken(tokens[0], "Error: INC/DEC requires an integer operand in ParsePrefix");
			ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Increment/Decrement requires an integer operand");
		}

		switch (operation)
		{
			case INC:	// PRE-INCREMENT
				// Debug: Emitting PRE-INCREMENT operation
				DebugPrintToken(tokens[0], "Emitting PRE-INCREMENT operation in ParsePrefix");
				code.EmitFormattedLine("", "PUSH", "@SP:0D0");
				code.EmitFormattedLine("", "PUSH", "#0D1");
				code.EmitFormattedLine("", "ADDI");
				code.EmitFormattedLine("", "POP", "@SP:0D1");	// side-effect
				code.EmitFormattedLine("", "PUSH", "@SP:0D0");
				code.EmitFormattedLine("", "SWAP");
				code.EmitFormattedLine("", "DISCARD", "#0D1");	// value
				break;
			case DEC:	// PRE-DECREMENT
				// Debug: Emitting PRE-DECREMENT operation
				DebugPrintToken(tokens[0], "Emitting PRE-DECREMENT operation in ParsePrefix");
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
	{
		// Debug: No INC/DEC operator, parsing Primary
		DebugPrintToken(tokens[0], "No INC/DEC operator detected, parsing Primary in ParsePrefix");
		ParsePrimary(tokens, datatype);
	}

	// Debug: Exiting Prefix
	DebugPrintToken(tokens[0], "Exiting ParsePrefix");

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

	// Debug: Entering Primary
	DebugPrintToken(tokens[0], "Entering ParsePrimary");

	switch (tokens[0].type)
	{
		case INTEGER:
			{
				// Debug: Integer constant detected
				DebugPrintToken(tokens[0], "Integer constant detected in ParsePrimary");

				char operand[SOURCELINELENGTH + 1];
				sprintf(operand, "#0D%s", tokens[0].lexeme);
				code.EmitFormattedLine("", "PUSH", operand);
				datatype = INTTYPE;
				GetNextToken(tokens);
			}
			break;
		case TRUE:
			// Debug: TRUE constant detected
			DebugPrintToken(tokens[0], "TRUE constant detected in ParsePrimary");

			code.EmitFormattedLine("", "PUSH", "#0XFFFF");	// or "#true"
			datatype = BOOLTYPE;
			GetNextToken(tokens);
			break;
		case FALSE:
			// Debug: FALSE constant detected
			DebugPrintToken(tokens[0], "FALSE constant detected in ParsePrimary");

			code.EmitFormattedLine("", "PUSH", "#0X0000");	// or "false"
			datatype = BOOLTYPE;
			GetNextToken(tokens);
			break;
		case OPARENTHESIS:
			// Debug: Parentheses detected
			DebugPrintToken(tokens[0], "Opening parenthesis detected in ParsePrimary");

			GetNextToken(tokens);
			ParseExpression(tokens, datatype);

			if (tokens[0].type != CPARENTHESIS)
			{
				// Debug: Error for missing closing parenthesis
				DebugPrintToken(tokens[0], "Error: Missing closing parenthesis in ParsePrimary");
				ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting ')'");
			}

			// Debug: Closing parenthesis matched
			DebugPrintToken(tokens[0], "Closing parenthesis matched in ParsePrimary");

			GetNextToken(tokens);
			break;
		case IDENTIFIER:
			// Debug: Identifier detected
			DebugPrintToken(tokens[0], "Identifier detected in ParsePrimary");

			ParseVariable(tokens, false, datatype);
			break;
		default:
			// Debug: Error for unexpected token type
			DebugPrintToken(tokens[0], "Error: Unexpected token in ParsePrimary");
			ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting integer, true, false, '(', variable");
			break;
	}

	// Debug: Exiting Primary
	DebugPrintToken(tokens[0], "Exiting ParsePrimary");

	ExitModule("Primary");
}

//-----------------------------------------------------------
void ParseVariable(TOKEN tokens[], bool asLValue, DATATYPE &datatype)
//-----------------------------------------------------------
{
	void GetNextToken(TOKEN tokens[]);

	bool isInTable;
	int index;
	IDENTIFIERTYPE identifierType;

	EnterModule("Variable");

	// Debug: Entering Variable
	DebugPrintToken(tokens[0], "Entering ParseVariable");

	if (tokens[0].type != IDENTIFIER)
	{
		// Debug: Error for missing identifier
		DebugPrintToken(tokens[0], "Error: Missing identifier in ParseVariable");
		ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting identifier");
	}

	// Debug: Checking identifier in table
	DebugPrintToken(tokens[0], "Checking identifier in symbol table in ParseVariable");

	// STATICSEMANTICS
	index = identifierTable.GetIndex(tokens[0].lexeme, isInTable);
	if (!isInTable)
	{
		// Debug: Error for undefined identifier
		DebugPrintToken(tokens[0], "Error: Undefined identifier in ParseVariable");
		ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Undefined identifier");
	}

	identifierType = identifierTable.GetType(index);
	datatype = identifierTable.GetDatatype(index);

	// Debug: Validating identifier type
	DebugPrintToken(tokens[0], "Validating identifier type in ParseVariable");

	if (!((identifierType == GLOBAL_VARIABLE) ||
		  (identifierType == GLOBAL_CONSTANT) ||
		  (identifierType == PROGRAMMODULE_VARIABLE) ||
		  (identifierType == PROGRAMMODULE_CONSTANT)))
	{
		// Debug: Error for invalid identifier type
		DebugPrintToken(tokens[0], "Error: Invalid identifier type in ParseVariable");
		ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting variable or constant identifier");
	}

	if (asLValue && ((identifierType == GLOBAL_CONSTANT) || (identifierType == PROGRAMMODULE_CONSTANT)))
	{
		// Debug: Error for constant as L-value
		DebugPrintToken(tokens[0], "Error: Constant used as L-value in ParseVariable");
		ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Constant may not be l-value");
	}
	// ENDSTATICSEMANTICS

	// Debug: Emitting address/value push
	DebugPrintToken(tokens[0], asLValue ? "Emitting address push in ParseVariable" : "Emitting value push in ParseVariable");

	// CODEGENERATION
	if (asLValue)
		code.EmitFormattedLine("", "PUSHA", identifierTable.GetReference(index));
	else
		code.EmitFormattedLine("", "PUSH", identifierTable.GetReference(index));
	// ENDCODEGENERATION

	GetNextToken(tokens);

	// Debug: Exiting Variable
	DebugPrintToken(tokens[0], "Exiting ParseVariable");

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
				type = OBRACE;
				lexeme[0] = nextCharacter; lexeme[1] = '\0';
				reader.GetNextCharacter();
				break;
			case '}':
				type = CBRACE;
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
