// PotatoCompiler.cpp by Auroxsus
// French Fry Productions
// Description: POTATO Compiler Program Ver. 6
//-----------------------------------------------------------
#include <iostream>
#include <iomanip>

#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <ctime>	// Used in Potato.h for time stamp
#include <vector>

using namespace std;

//#define TRACEREADER
//#define TRACESCANNER
//#define TRACEPARSER
//#define TRACEIDENTIFIERTABLE							
#define TRACECOMPILER

#include "Potato.h"

typedef enum
{
	// pseudo-terminals
	IDENTIFIER,
	INTEGER,
	STRING,
	EOPTOKEN,
	UNKTOKEN,
	// reserved words
	PROGRAM,	
	END,
	PRINT,
	ENDL,
	OR,
	NOR,
	XOR,
	AND,
	NAND,
	NOT,
	ABS,		   
	TRUE,
	FALSE,
	VAR,   
	INT,   
	BOOL,	
	CON,   
	INPUT,	
	IF,
	ELIF,
	ELSE,
	DO,
	WHILE,
	FOR,   
	TO,  
	BY,  
	PROCEDURE,
	IN,
	OUT,
	IO,
	REF,
	CALL,
	RETURN,
	FUNCTION,
//	DO2,		// POTATO 4 ONLY
//	FOR2,		// POTATO 5 ONLY
//	CHOOSE,
//	ONE,   
//	ALL,   
//	WHEN,		  
	// punctuation
	COMMA,
	PERIOD,
    OPARENTHESIS,
    CPARENTHESIS,
	COLON,	 
	COLONEQ,	   
	OBRACE,	  
    CBRACE,
	// operators
	LT, 		// <
	LTEQ, 		// <=
	EQ, 		// ==
	EQUAL,		// =
	GT, 		// >
	GTEQ, 		// >=
	NOTEQ, 		// <> and !=
	PLUS, 		// +
	MINUS,		// -
	MULTIPLY, 	// *
	DIVIDE, 	// /
	MODULUS, 	// %
	POWER,  	// ^ and **
	INC,   
	DEC  
} TOKENTYPE;

struct TOKENTABLERECORD
{
	TOKENTYPE type;
	char description[12+1];
	bool isReservedWord;
};

const TOKENTABLERECORD TOKENTABLE[] =
{
	{ IDENTIFIER  ,"IDENTOTY"    ,false }, 
	{ INTEGER     ,"INTEGROOT"   ,false },
	{ STRING      ,"STRING"      ,false },
	{ EOPTOKEN    ,"EOPTOKEN"    ,false },
	{ UNKTOKEN    ,"UNKTOKEN"    ,false },
	{ PROGRAM  	  ,"POTATO"      ,true  },
	{ END 		  ,"COOKEDPOTATO",true  }, // steamed, baked, or boilded?
	{ PRINT   	  ,"FRY"   		 ,true  },
	{ ENDL        ,"PEEL"        ,true  },
    { OR          ,"OR"          ,true  },
    { NOR         ,"NOR"         ,true  },
    { XOR         ,"XOR"         ,true  },
    { AND         ,"AND"         ,true  },
    { NAND        ,"NAND"        ,true  },
    { NOT         ,"NOT"         ,true  },
	{ ABS         ,"ABS"         ,true  },
    { TRUE        ,"TRUE"        ,true  },
    { FALSE       ,"FALSE"       ,true  },
	{ VAR         ,"VAR"         ,true  },
	{ INT         ,"INT"         ,true  },
	{ BOOL        ,"BOOL"        ,true  },
	{ CON         ,"CON"         ,true  },
	{ INPUT       ,"INPUT"       ,true  },
	{ IF          ,"IF"          ,true  },
	{ ELIF        ,"ELIF"        ,true  },
	{ ELSE        ,"ELSE"        ,true  },
	{ DO          ,"DO"          ,true  },
	{ WHILE       ,"WHILE"       ,true  },
    { FOR         ,"FOR"         ,true  },
    { TO          ,"TO"          ,true  },
    { BY          ,"BY"          ,true  },
//	{ DO2         ,"DO2"         ,true  }, // POTATO 4 ONLY
//  { FOR2        ,"FOR2"        ,true  }, // POTATO 5 ONLY
//	{ CHOOSE      ,"CHOOSE"      ,true  },
//	{ ONE         ,"ONE"         ,true  },
//	{ ALL         ,"ALL"         ,true  },
//	{ WHEN        ,"WHEN"        ,true  },
//	{ BY          ,"BY"          ,true  },
	{ PROCEDURE   ,"PROCEDURE"   ,true  },
	{ IN          ,"IN"		     ,true  },
	{ OUT         ,"OUT"         ,true  },
	{ IO          ,"IO"      	 ,true  },
	{ REF         ,"REF"         ,true  },
	{ CALL        ,"CALL"        ,true  },
	{ RETURN      ,"RETURN"      ,true  },	
    { FUNCTION    ,"FUNCTION"    ,true  },
	{ COMMA       ,"COMMA"       ,false },
	{ PERIOD      ,"PERIOD"      ,false },
	{ OPARENTHESIS,"OPARENTHESIS",false },
	{ CPARENTHESIS,"CPARENTHESIS",false },
	{ COLON       ,"COLON"       ,false },
	{ COLONEQ     ,"COLONEQ"     ,false },
	{ OBRACE      ,"OBRACE"      ,false },
	{ CBRACE      ,"CBRACE"      ,false },
	{ LT          ,"LT"          ,false },
	{ LTEQ        ,"LTEQ"        ,false },
	{ EQ          ,"EQ"          ,false },
	{ EQUAL       ,"EQUAL"       ,false },
	{ GT          ,"GT"          ,false },
	{ GTEQ        ,"GTEQ"        ,false },
	{ NOTEQ       ,"NOTEQ"       ,false },
	{ PLUS        ,"PLUS"        ,false },
	{ MINUS       ,"MINUS"       ,false },
	{ MULTIPLY    ,"MULTIPLY"    ,false },
	{ DIVIDE      ,"DIVIDE"      ,false },
	{ MODULUS     ,"MODULUS"     ,false },
	{ POWER       ,"POWER"       ,false },
	{ INC         ,"INC"         ,false },
	{ DEC         ,"DEC"         ,false }										
};

struct TOKEN
{
	TOKENTYPE type;
	char lexeme[SOURCELINELENGTH+1];
	int sourceLineNumber;
	int sourceLineIndex;
};

//--------------------------------------------------
// Global variables
//--------------------------------------------------
READER<CALLBACKSUSED> reader(SOURCELINELENGTH,LOOKAHEAD);
LISTER lister(LINESPERPAGE);

// CODEGENERATION
CODE code;
IDENTIFIERTABLE identifierTable(&lister,MAXIMUMIDENTIFIERS);															
// ENDCODEGENERATION

#ifdef TRACEPARSER
	int level;
#endif

void EnterModule(const char module[])
{
#ifdef TRACEPARSER
	char information[SOURCELINELENGTH+1];

	level++;
	sprintf(information,"   %*s>%s",level*2," ",module);
	lister.ListInformationLine(information);
#endif
}

void ExitModule(const char module[])
{
#ifdef TRACEPARSER
	char information[SOURCELINELENGTH+1];

	sprintf(information,"   %*s<%s",level*2," ",module);
	lister.ListInformationLine(information);
	level--;
#endif
}

void ProcessCompilerError(int sourceLineNumber,int sourceLineIndex,const char errorMessage[])
{
	char information[SOURCELINELENGTH+1];

	// Use "panic mode" error recovery technique: report error message and terminate compilation!
	sprintf(information,"     At (%4d:%3d) %s",sourceLineNumber,sourceLineIndex,errorMessage);
	lister.ListInformationLine(information); 
	lister.ListInformationLine("POTATO compiler ending with a potato famine!\n");
	throw( POTATOEXCEPTION("POTATO compiler ending with a potato famine!") );
}

//-----------------------------------------------------------
int main()
//-----------------------------------------------------------
{
	void Callback1(int sourceLineNumber,const char sourceLine[]);
	void Callback2(int sourceLineNumber,const char sourceLine[]);
	void ParsePOTATOProgram(TOKEN tokens[]);
	void GetNextToken(TOKEN tokens[]);

	char sourceFileName[80+1];
	TOKEN tokens[LOOKAHEAD+1];
   
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
	catch (POTATOEXCEPTION POTATOException)
	{
		cout << "POTATO exception: " << POTATOException.GetDescription() << endl;
	}
	lister.ListInformationLine("******* POTATO was cooked thoroughly");
	cout << "POTATO was cooked thoroughly\n";

	system("PAUSE");
	return( 0 );
   
}

void ParsePOTATOProgram(TOKEN tokens[])
{
	void ParseDataDefinitions(TOKEN tokens[],IDENTIFIERSCOPE identifierScope);
	void ParsePROCEDUREDefinition(TOKEN tokens[]);	
	void ParseFUNCTIONDefinition(TOKEN tokens[]);	
	void ParsePROGRAMDefinition(TOKEN tokens[]);	
	void GetNextToken(TOKEN tokens[]);									 
   
	EnterModule("POTATOProgram");
	
	ParseDataDefinitions(tokens,GLOBALSCOPE);

#ifdef TRACECOMPILER
	identifierTable.DisplayTableContents("Contents of identifier table after compilation of global data definitions");
#endif

	while ( ( tokens[0].type == PROCEDURE ) || (tokens[0].type ==  FUNCTION) )
	{
		switch ( tokens[0].type )
		{
			case PROCEDURE:
				ParsePROCEDUREDefinition(tokens);
				break;
			case FUNCTION:
				ParseFUNCTIONDefinition(tokens);
				break;
		}
	}
	
	if ( tokens[0].type == PROGRAM )
		ParsePROGRAMDefinition(tokens);
	else
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,
                           "Expecting PROGRAM");
						   
	if ( tokens[0].type != EOPTOKEN )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,
                           "Expecting end-of-program");

	ExitModule("POTATOProgram");
}

void ParseDataDefinitions(TOKEN tokens[],IDENTIFIERSCOPE identifierScope)
{
	void GetNextToken(TOKEN tokens[]);

	EnterModule("DataDefinitions");

	while ( (tokens[0].type == VAR) || (tokens[0].type == CON) )
	{
		switch ( tokens[0].type )
		{
			case VAR:
				do
				{
					char identifier[MAXIMUMLENGTHIDENTIFIER+1];
					char reference[MAXIMUMLENGTHIDENTIFIER+1];
					DATATYPE datatype;
					bool isInTable;
					int index;
      
					GetNextToken(tokens);
					
					switch ( tokens[0].type )
					{
						case INT:
							datatype = INTEGERTYPE;
							break;
						case BOOL:
							datatype = BOOLEANTYPE;
							break;
						default:
							ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting INT or BOOL VAR");
					}
					GetNextToken(tokens);
         
					if ( tokens[0].type != IDENTIFIER )
						ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting identifier var");
					strcpy(identifier,tokens[0].lexeme);
					GetNextToken(tokens);
         
					index = identifierTable.GetIndex(identifier,isInTable);
					if ( isInTable && identifierTable.IsInCurrentScope(index) )
						ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Multiply-defined identifier");
      
					switch ( identifierScope )
					{
						case GLOBALSCOPE:
							// CODEGENERATION
							code.AddRWToStaticData(1,identifier,reference);
							// ENDCODEGENERATION
							identifierTable.AddToTable(identifier,GLOBAL_VARIABLE,datatype,reference);
							break;
						case PROGRAMMODULESCOPE:
							// CODEGENERATION
							code.AddRWToStaticData(1,identifier,reference);
							// ENDCODEGENERATION
							identifierTable.AddToTable(identifier,PROGRAMMODULE_VARIABLE,datatype,reference);
							break;
			            case SUBPROGRAMMODULESCOPE:
							// CODEGENERATION
							sprintf(reference,"FB:0D%d",code.GetFBOffset());
							code.IncrementFBOffset(1);
							// ENDCODEGENERATION
							identifierTable.AddToTable(identifier,SUBPROGRAMMODULE_VARIABLE,datatype,reference);
							break;
					}

				} while ( tokens[0].type == COMMA );
      
				if ( tokens[0].type != PERIOD )
				ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting '.'");
				GetNextToken(tokens);
				break;
			case CON:
				do
				{
					char identifier[MAXIMUMLENGTHIDENTIFIER+1];
					char literal[MAXIMUMLENGTHIDENTIFIER+1];
					char operand[MAXIMUMLENGTHIDENTIFIER+1];
					char comment[MAXIMUMLENGTHIDENTIFIER+1];
					char reference[MAXIMUMLENGTHIDENTIFIER+1];
					DATATYPE datatype;
					bool isInTable;
					int index;
      
					GetNextToken(tokens);
					
					switch ( tokens[0].type )
				    {
						case INT:
							datatype = INTEGERTYPE;
							break;
						case BOOL:
							datatype = BOOLEANTYPE;
							break;
						default:
							ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting INT or BOOL CON");
					}
					GetNextToken(tokens);
					
					if ( tokens[0].type != IDENTIFIER )
						ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting identifier con");
				    strcpy(identifier,tokens[0].lexeme);
				    GetNextToken(tokens);
				
					index = identifierTable.GetIndex(identifier,isInTable);
					if ( isInTable && identifierTable.IsInCurrentScope(index) )
						ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Multiply-defined identifier");

					switch ( identifierScope )
					{
						case GLOBALSCOPE:
							// CODEGENERATION
							code.AddDWToStaticData(literal,identifier,reference);
							// ENDCODEGENERATION
							identifierTable.AddToTable(identifier,GLOBAL_CONSTANT,datatype,reference);
							break;
						case PROGRAMMODULESCOPE:
							// CODEGENERATION
							code.AddDWToStaticData(literal,identifier,reference);
							// ENDCODEGENERATION
							identifierTable.AddToTable(identifier,PROGRAMMODULE_CONSTANT,datatype,reference);
							break;
						case SUBPROGRAMMODULESCOPE:
							// CODEGENERATION
							sprintf(reference,"FB:0D%d",code.GetFBOffset());
							strcpy(operand,"#"); strcat(operand,literal);
							sprintf(comment,"initialize constant %s",identifier);
							code.AddInstructionToInitializeFrameData("PUSH",operand,comment);
							code.AddInstructionToInitializeFrameData("POP",reference);
							code.IncrementFBOffset(1);
							// ENDCODEGENERATION
							identifierTable.AddToTable(identifier,SUBPROGRAMMODULE_CONSTANT,datatype,reference);
							break;
               }

					
					GetNextToken(tokens);
					
					if ( (datatype == INTEGERTYPE) && (tokens[0].type == INTEGER) )
					{
						strcpy(literal,"0D");
						strcat(literal,tokens[0].lexeme);
					}
					else if ( ((datatype == BOOLEANTYPE) && ((tokens[0].type == TRUE) || (tokens[0].type == FALSE))) )
						strcpy(literal,tokens[0].lexeme);
					else
						ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Data type mismatch");
					
					GetNextToken(tokens);
					
				} while ( tokens[0].type == COMMA );
      
				if ( tokens[0].type != PERIOD )
					ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting '.'");
				GetNextToken(tokens);
				break;
		}
	}

   ExitModule("DataDefinitions");
}

void ParsePROCEDUREDefinition(TOKEN tokens[])
{
	void ParseFormalParameter(TOKEN tokens[],IDENTIFIERTYPE &identifierType,int &n);
	void ParseDataDefinitions(TOKEN tokens[],IDENTIFIERSCOPE identifierScope);
	void ParseStatement(TOKEN tokens[]);
	void GetNextToken(TOKEN tokens[]);

	bool isInTable;
	char line[SOURCELINELENGTH+1];
	int index;
	char reference[SOURCELINELENGTH+1];

	// n = # formal parameters, m = # words of "save-register" space and locally-defined variables/constants
	int n,m;
	char label[SOURCELINELENGTH+1],operand[SOURCELINELENGTH+1],comment[SOURCELINELENGTH+1];

	EnterModule("PROCEDUREDefinition");

	GetNextToken(tokens);

	if ( tokens[0].type != IDENTIFIER )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting identifier prod");

	index = identifierTable.GetIndex(tokens[0].lexeme,isInTable);
	if ( isInTable && identifierTable.IsInCurrentScope(index) )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Multiply-defined identifier");

	identifierTable.AddToTable(tokens[0].lexeme,PROCEDURE_SUBPROGRAMMODULE,NOTYPE,tokens[0].lexeme);

	// CODEGENERATION
	code.EnterModuleBody(PROCEDURE_SUBPROGRAMMODULE,index);
	code.ResetFrameData();
	code.EmitUnformattedLine("; **** =========");
	sprintf(line,"; **** PROCEDURE module (%4d)",tokens[0].sourceLineNumber);
	code.EmitUnformattedLine(line);
	code.EmitUnformattedLine("; **** =========");
	code.EmitFormattedLine(tokens[0].lexeme,"EQU","*");
	// ENDCODEGENERATION

	identifierTable.EnterNestedStaticScope();

	GetNextToken(tokens);
	n = 0;
	if ( tokens[0].type == OPARENTHESIS )
	{
		do
		{
			IDENTIFIERTYPE identifierType;

			GetNextToken(tokens);
			ParseFormalParameter(tokens,identifierType,n);
		} while ( tokens[0].type == COMMA );

		if ( tokens[0].type != CPARENTHESIS )
			ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting ')'");
		GetNextToken(tokens);
	}

#ifdef TRACECOMPILER
	identifierTable.DisplayTableContents("Contents of identifier table after compilation of PROCEDURE module header");
#endif

	// CODEGENERATION
	code.IncrementFBOffset(2); // makes room in frame for caller's saved FB register and the CALL return address
	// ENDCODEGENERATION

	ParseDataDefinitions(tokens,SUBPROGRAMMODULESCOPE);

#ifdef TRACECOMPILER
	identifierTable.DisplayTableContents("Contents of identifier table after compilation of PROCEDURE local data definitions");
#endif

	// CODEGENERATION
	m = code.GetFBOffset()-(n+2);
	code.EmitFormattedLine("","PUSHSP","","set PROCEDURE module FB = SP-on-entry + 2(n+2)");
	sprintf(operand,"#0D%d",2*(n+2));
	sprintf(comment,"n = %d",n);
	code.EmitFormattedLine("","PUSH",operand,comment);
	code.EmitFormattedLine("","ADDI");
	code.EmitFormattedLine("","POPFB");
	code.EmitFormattedLine("","PUSHSP","","PROCEDURE module SP = SP-on-entry + 2m");	
	sprintf(operand,"#0D%d",2*m);
	sprintf(comment,"m = %d",m);
	code.EmitFormattedLine("","PUSH",operand,comment);
	code.EmitFormattedLine("","SUBI");
	code.EmitFormattedLine("","POPSP");
	code.EmitUnformattedLine("; statements to initialize frame data (if necessary)");
	code.EmitFrameData();
	sprintf(label,"MODULEBODY%04d",code.LabelSuffix());
	code.EmitFormattedLine("","CALL",label);
	code.EmitFormattedLine("","PUSHFB","","restore caller's SP-on-entry = FB - 2(n+2)");
	sprintf(operand,"#0D%d",2*(n+2));
	code.EmitFormattedLine("","PUSH",operand);
	code.EmitFormattedLine("","SUBI");
	code.EmitFormattedLine("","POPSP");
	code.EmitFormattedLine("","RETURN","","return to caller");
	code.EmitUnformattedLine("");
	code.EmitFormattedLine(label,"EQU","*");
	code.EmitUnformattedLine("; statements in body of PROCEDURE module (may include RETURN)");
	// ENDCODEGENERATION

	while ( tokens[0].type != END )
		ParseStatement(tokens);

	// CODEGENERATION
	code.EmitFormattedLine("","RETURN");
	code.EmitUnformattedLine("");
	code.EmitUnformattedLine("; **** =========");
	sprintf(line,"; **** END (%4d)",tokens[0].sourceLineNumber);
	code.EmitUnformattedLine(line);
	code.EmitUnformattedLine("; **** =========");
	code.ExitModuleBody();
	// ENDCODEGENERATION

	identifierTable.ExitNestedStaticScope();

#ifdef TRACECOMPILER
	identifierTable.DisplayTableContents("Contents of identifier table at end of compilation of PROCEDURE module definition");
#endif

	GetNextToken(tokens);

	ExitModule("PROCEDUREDefinition");
}

void ParseFUNCTIONDefinition(TOKEN tokens[])
{
	void ParseFormalParameter(TOKEN tokens[],IDENTIFIERTYPE &identifierType,int &n);
	void ParseDataDefinitions(TOKEN tokens[],IDENTIFIERSCOPE identifierScope);
	void ParseStatement(TOKEN tokens[]);
	void GetNextToken(TOKEN tokens[]);

	bool isInTable;
	DATATYPE datatype;
	char identifier[SOURCELINELENGTH+1];
	char line[SOURCELINELENGTH+1];
	int index;
	char reference[SOURCELINELENGTH+1];

	// n = # formal parameters, m = # words of return-value, "save-register" space, and locally-defined variables/constants
	int n,m;
	char label[SOURCELINELENGTH+1],operand[SOURCELINELENGTH+1],comment[SOURCELINELENGTH+1];

	EnterModule("FUNCTIONDefinition");

	GetNextToken(tokens);

	if ( tokens[0].type != IDENTIFIER )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting identifier");

	strcpy(identifier,tokens[0].lexeme);
	index = identifierTable.GetIndex(identifier,isInTable);
	if ( isInTable && identifierTable.IsInCurrentScope(index) )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Multiply-defined identifier");
	GetNextToken(tokens);

	if ( tokens[0].type != COLON )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting ':'");
	GetNextToken(tokens);

	switch ( tokens[0].type )
	{
		case INT:
			datatype = INTEGERTYPE;
			break;
		case BOOL:
			datatype = BOOLEANTYPE;
			break;
		default:
			ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting INT or BOOL");
	}
	GetNextToken(tokens);

	identifierTable.AddToTable(identifier,FUNCTION_SUBPROGRAMMODULE,datatype,identifier);
	index = identifierTable.GetIndex(identifier,isInTable);

	// CODEGENERATION
	code.EnterModuleBody(FUNCTION_SUBPROGRAMMODULE,index);
	code.ResetFrameData();

	// Reserve frame-space for FUNCTION return value
	code.IncrementFBOffset(1);

	code.EmitUnformattedLine("; **** =========");
	sprintf(line,"; **** FUNCTION module (%4d)",tokens[0].sourceLineNumber);
	code.EmitUnformattedLine(line);
	code.EmitUnformattedLine("; **** =========");
	code.EmitFormattedLine(identifier,"EQU","*");
	// ENDCODEGENERATION

	identifierTable.EnterNestedStaticScope();

	n = 0;
	if ( tokens[0].type != OPARENTHESIS )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting '('");
	// Use token look-ahead to make parsing decision
	if ( tokens[1].type != CPARENTHESIS )
	{
		do
		{
			IDENTIFIERTYPE identifierType;

			GetNextToken(tokens);
			ParseFormalParameter(tokens,identifierType,n);

			// STATICSEMANTICS
			if ( identifierType != IN_PARAMETER )
				ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"FUNCTION parameter must be IN");
			// ENDSTATICSEMANTICS

		} while ( tokens[0].type == COMMA );
	}
	else
		GetNextToken(tokens);
	if ( tokens[0].type != CPARENTHESIS )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting ')'");
	GetNextToken(tokens);

#ifdef TRACECOMPILER
	identifierTable.DisplayTableContents("Contents of identifier table after compilation of FUNCTION module header");
#endif

	// CODEGENERATION
	code.IncrementFBOffset(2); // makes room in frame for caller's saved FB register and the CALL return address
	// ENDCODEGENERATION

	ParseDataDefinitions(tokens,SUBPROGRAMMODULESCOPE);

#ifdef TRACECOMPILER
	identifierTable.DisplayTableContents("Contents of identifier table after compilation of FUNCTION local data definitions");
#endif

	// CODEGENERATION
	m = code.GetFBOffset()-(n+3);
	code.EmitFormattedLine("","PUSHSP","","set FUNCTION module FB = SP-on-entry + 2(n+3)");
	sprintf(operand,"#0D%d",2*(n+3));
	sprintf(comment,"n = %d",n);
	code.EmitFormattedLine("","PUSH",operand,comment);
	code.EmitFormattedLine("","ADDI");
	code.EmitFormattedLine("","POPFB");
	code.EmitFormattedLine("","PUSHSP","","FUNCTION module SP = SP-on-entry + 2m");
	sprintf(operand,"#0D%d",2*m);
	sprintf(comment,"m = %d",m);
	code.EmitFormattedLine("","PUSH",operand,comment);
	code.EmitFormattedLine("","SUBI");
	code.EmitFormattedLine("","POPSP");
	code.EmitUnformattedLine("; statements to initialize frame data (if necessary)");
	code.EmitFrameData();
	sprintf(label,"MODULEBODY%04d",code.LabelSuffix());
	code.EmitFormattedLine("","CALL",label);
	code.EmitFormattedLine("","PUSHFB","","restore caller's SP-on-entry = FB - 2(n+3)");
	sprintf(operand,"#0D%d",2*(n+3));
	code.EmitFormattedLine("","PUSH",operand);
	code.EmitFormattedLine("","SUBI");
	code.EmitFormattedLine("","POPSP");
	code.EmitFormattedLine("","RETURN","","return to caller");
	code.EmitUnformattedLine("");
	code.EmitFormattedLine(label,"EQU","*");
	code.EmitUnformattedLine("; statements in body of FUNCTION module (*MUST* execute RETURN)");
	// ENDCODEGENERATION

    while ( tokens[0].type != END )
		ParseStatement(tokens);

	// CODEGENERATION
	sprintf(operand,"#0D%d",tokens[0].sourceLineNumber);
	code.EmitFormattedLine("","PUSH",operand);
	code.EmitFormattedLine("","PUSH","#0D3");
	code.EmitFormattedLine("","JMP","HANDLERUNTIMEERROR");
	code.EmitUnformattedLine("; **** =========");
	sprintf(line,"; **** END (%4d)",tokens[0].sourceLineNumber);
	code.EmitUnformattedLine(line);
	code.EmitUnformattedLine("; **** =========");
	code.ExitModuleBody();
	// ENDCODEGENERATION

	identifierTable.ExitNestedStaticScope();

#ifdef TRACECOMPILER
	identifierTable.DisplayTableContents("Contents of identifier table at end of compilation of FUNCTION module definition");
#endif

	GetNextToken(tokens);

	ExitModule("FUNCTIONDefinition");
}
void ParseFormalParameter(TOKEN tokens[],IDENTIFIERTYPE &identifierType,int &n)
{
   void GetNextToken(TOKEN tokens[]);

	char identifier[MAXIMUMLENGTHIDENTIFIER+1],reference[MAXIMUMLENGTHIDENTIFIER+1];
	bool isInTable;
	int index;
	DATATYPE datatype;

	EnterModule("FormalParameter");

	// CODEGENERATION
	switch ( tokens[0].type )
	{
		case IN:
			identifierType = IN_PARAMETER;
			sprintf(reference,"FB:0D%d",code.GetFBOffset());
			code.IncrementFBOffset(1);
			n += 1;
			GetNextToken(tokens);
			break;
		case OUT:
			identifierType = OUT_PARAMETER;
			code.IncrementFBOffset(1);
			sprintf(reference,"FB:0D%d",code.GetFBOffset());
			code.IncrementFBOffset(1);
			n += 2;
			GetNextToken(tokens);
			break;
		case IO:
			identifierType = IO_PARAMETER;
			code.IncrementFBOffset(1);
			sprintf(reference,"FB:0D%d",code.GetFBOffset());
			code.IncrementFBOffset(1);
			n += 2;
			GetNextToken(tokens);
			break;
		case REF:
			identifierType = REF_PARAMETER;
			sprintf(reference,"@FB:0D%d",code.GetFBOffset());
			code.IncrementFBOffset(1);
			n += 1;
			GetNextToken(tokens);
			break;
		default:
			identifierType = IN_PARAMETER;
			sprintf(reference,"FB:0D%d",code.GetFBOffset());
			code.IncrementFBOffset(1);
			n += 1;
			break;
	}
	// ENDCODEGENERATION


	switch ( tokens[0].type )
	{
		case INT:
			datatype = INTEGERTYPE;
			break;
		case BOOL:
			datatype = BOOLEANTYPE;
			break;
		default:
			ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting INT or BOOL");
	}
	GetNextToken(tokens);
	
	if ( tokens[0].type != IDENTIFIER )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting identifier formal param");
	strcpy(identifier,tokens[0].lexeme);
	GetNextToken(tokens);

	index = identifierTable.GetIndex(identifier,isInTable);
	if ( isInTable && identifierTable.IsInCurrentScope(index) )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Multiply-defined identifier");

	identifierTable.AddToTable(identifier,identifierType,datatype,reference);

	ExitModule("FormalParameter");
}



void ParsePROGRAMDefinition(TOKEN tokens[])
{
	void ParseStatement(TOKEN tokens[]);
	void GetNextToken(TOKEN tokens[]);
	void ParseDataDefinitions(TOKEN tokens[],IDENTIFIERSCOPE identifierScope);

	char line[SOURCELINELENGTH+1];
	char label[SOURCELINELENGTH+1];
	char reference[SOURCELINELENGTH+1];
	
	EnterModule("PROGRAMDefinition");

	// CODEGENERATION
	code.EmitUnformattedLine("; **** =========");
	sprintf(line,"; **** PROGRAM module (%4d)",tokens[0].sourceLineNumber);
	code.EmitUnformattedLine(line);
	code.EmitUnformattedLine("; **** =========");
	code.EmitFormattedLine("PROGRAMMAIN","EQU"  ,"*");
	code.EmitFormattedLine("","PUSH" ,"#RUNTIMESTACK","set SP");
	code.EmitFormattedLine("","POPSP");
	code.EmitFormattedLine("","PUSHA","STATICDATA","set SB");
	code.EmitFormattedLine("","POPSB");
	code.EmitFormattedLine("","PUSH","#HEAPBASE","initialize heap");
	code.EmitFormattedLine("","PUSH","#HEAPSIZE");
	code.EmitFormattedLine("","SVC","#SVC_INITIALIZE_HEAP");
	sprintf(label,"PROGRAMBODY%04d",code.LabelSuffix());
	code.EmitFormattedLine("","CALL",label);
	code.AddDSToStaticData("Normal program termination","",reference);
	code.EmitFormattedLine("","PUSHA",reference);
	code.EmitFormattedLine("","SVC","#SVC_WRITE_STRING");
	code.EmitFormattedLine("","SVC","#SVC_WRITE_ENDL");
	code.EmitFormattedLine("","PUSH","#0D0","terminate with status = 0");
	code.EmitFormattedLine("","SVC" ,"#SVC_TERMINATE");
	code.EmitUnformattedLine("");
	code.EmitFormattedLine(label,"EQU","*");
	// ENDCODEGENERATION

	GetNextToken(tokens);
	
	identifierTable.EnterNestedStaticScope();
	ParseDataDefinitions(tokens,PROGRAMMODULESCOPE);											
	while ( tokens[0].type != END )
		ParseStatement(tokens);

	// CODEGENERATION
	code.EmitFormattedLine("","RETURN");
	code.EmitUnformattedLine("; **** =========");
	sprintf(line,"; **** END (%4d)",tokens[0].sourceLineNumber);
	code.EmitUnformattedLine(line);
	code.EmitUnformattedLine("; **** =========");
	// ENDCODEGENERATION

#ifdef TRACECOMPILER
	identifierTable.DisplayTableContents("Contents of identifier table at end of compilation of PROGRAM module definition");
#endif

	identifierTable.ExitNestedStaticScope();										   
	GetNextToken(tokens);
	ExitModule("PROGRAMDefinition");
}

void ParseStatement(TOKEN tokens[])
{
	void ParseAssertion(TOKEN tokens[]);								   
	void ParsePRINTStatement(TOKEN tokens[]);
	void ParseINPUTStatement(TOKEN tokens[]);
	void ParseAssignmentStatement(TOKEN tokens[]);	
	void ParseIFStatement (TOKEN tokens[]);
	void ParseDOWHILEStatement (TOKEN tokens[]);
	void ParseFORStatement(TOKEN tokens[]);
	void ParseCALLStatement(TOKEN tokens[]); 
	void ParseRETURNStatement(TOKEN tokens[]);
	// POTATO 4 ONLY
/*	void ParseDO2WHILEStatement(TOKEN tokens[]);
	void ParseWHILEStatement(TOKEN tokens[]);		*/
	// POTATO 5 ONLY
/*	void ParseFOR2Statement(TOKEN tokens[]);
	void ParseCHOOSEStatement(TOKEN tokens[]);		*/								
	void GetNextToken(TOKEN tokens[]);

	EnterModule("Statement");

	while ( tokens[0].type == OBRACE )
		ParseAssertion(tokens);
	switch ( tokens[0].type )
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
		case IF:
			ParseIFStatement(tokens);
			break;
		case DO:
			ParseDOWHILEStatement(tokens);
			break;
		case FOR:
			ParseFORStatement(tokens);
			break;	 
		case CALL:
			ParseCALLStatement(tokens);
			break;
		case RETURN:
			ParseRETURNStatement(tokens);
			break;
		/* // POTATO 4 ONLY
		case DO2: 
			ParseDO2WHILEStatement(tokens);
			break;
		case WHILE:
			ParseWHILEStatement(tokens);
			break;
		  // POTATO 5 ONLY
		case FOR2:
			ParseFOR2Statement(tokens);
			break;
		case CHOOSE:
			ParseCHOOSEStatement(tokens);
			break;
		*/
		default:
			ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,
                              "Expecting beginning-of-statement");
			break;
	}

	while ( tokens[0].type == OBRACE )
		ParseAssertion(tokens);

	ExitModule("Statement");
}

void ParseAssertion(TOKEN tokens[])
{
	void ParseExpression(TOKEN tokens[],DATATYPE &datatype);
	void GetNextToken(TOKEN tokens[]);

	char line[SOURCELINELENGTH+1];
	DATATYPE datatype;

	EnterModule("Assertion");

	sprintf(line,"; **** %4d: { assertion }",tokens[0].sourceLineNumber);
	code.EmitUnformattedLine(line);

	GetNextToken(tokens);

	ParseExpression(tokens,datatype);

	// STATICSEMANTICS
	if ( datatype != BOOLEANTYPE )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting boolean expression");
	// ENDSTATICSEMANTICS

	// CODEGENERATION
	char Elabel[SOURCELINELENGTH+1],operand[SOURCELINELENGTH+1];

	code.EmitFormattedLine("","SETT");
	sprintf(Elabel,"E%04d",code.LabelSuffix());
	code.EmitFormattedLine("","JMPT",Elabel);
	sprintf(operand,"#0D%d",tokens[0].sourceLineNumber);
	code.EmitFormattedLine("","PUSH",operand);
	code.EmitFormattedLine("","PUSH","#0D1");
	code.EmitFormattedLine("","JMP","HANDLERUNTIMEERROR");
	code.EmitFormattedLine(Elabel,"EQU","*");
	code.EmitFormattedLine("","DISCARD","#0D1");
	// ENDCODEGENERATION

	if ( tokens[0].type != CBRACE )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting }");

	GetNextToken(tokens);

	ExitModule("Assertion");
}

void ParsePRINTStatement(TOKEN tokens[])
{
	void ParseExpression(TOKEN tokens[],DATATYPE &datatype);												   
	void GetNextToken(TOKEN tokens[]);

	char line[SOURCELINELENGTH+1];		
	DATATYPE datatype;					 				 
	EnterModule("PRINTStatement");
	
	sprintf(line,"; **** PRINT statement (%4d)",tokens[0].sourceLineNumber);
	code.EmitUnformattedLine(line);

	do
	{
		// Necessary to discard the token-and-lexeme
		GetNextToken(tokens);

		switch ( tokens[0].type )
		{
			case STRING:
				// CODEGENERATION
				char reference[SOURCELINELENGTH+1];
				code.AddDSToStaticData(tokens[0].lexeme,"",reference);
				code.EmitFormattedLine("","PUSHA",reference);
				code.EmitFormattedLine("","SVC","#SVC_WRITE_STRING");
				// ENDCODEGENERATION
				GetNextToken(tokens);
				break;
			case ENDL:
				// CODEGENERATION
				code.EmitFormattedLine("","SVC","#SVC_WRITE_ENDL");
				// ENDCODEGENERATION		
				GetNextToken(tokens);
				break;
			default:
			{
				ParseExpression(tokens,datatype);
				// CODEGENERATION
				switch ( datatype )
				{
					case INTEGERTYPE:
						code.EmitFormattedLine("","SVC","#SVC_WRITE_INTEGER");
						break;
					case BOOLEANTYPE:
						code.EmitFormattedLine("","SVC","#SVC_WRITE_BOOLEAN");
						break;
				}
				// ENDCODEGENERATION
			}
		}
	} while ( tokens[0].type == COMMA );

	if ( tokens[0].type != PERIOD )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,
                           "Expecting '.'");

	GetNextToken(tokens);
	ExitModule("PRINTStatement");
}

void ParseINPUTStatement(TOKEN tokens[])
{
	void ParseVariable(TOKEN tokens[],bool asLValue,DATATYPE &datatype);
	void GetNextToken(TOKEN tokens[]);

	char reference[SOURCELINELENGTH+1];
	char line[SOURCELINELENGTH+1];
	DATATYPE datatype;

	EnterModule("INPUTStatement");

	sprintf(line,"; **** INPUT statement (%4d)",tokens[0].sourceLineNumber);
	code.EmitUnformattedLine(line);

	GetNextToken(tokens);

	if ( tokens[0].type == STRING )
	{

		// CODEGENERATION
		code.AddDSToStaticData(tokens[0].lexeme,"",reference);
		code.EmitFormattedLine("","PUSHA",reference);
		code.EmitFormattedLine("","SVC","#SVC_WRITE_STRING");
		// ENDCODEGENERATION

		GetNextToken(tokens);
	}

	ParseVariable(tokens,true,datatype);

	// CODEGENERATION
	switch ( datatype )
	{
		case INTEGERTYPE:
			code.EmitFormattedLine("","SVC","#SVC_READ_INTEGER");
			break;
		case BOOLEANTYPE:
			code.EmitFormattedLine("","SVC","#SVC_READ_BOOLEAN");
			break;
	}
	code.EmitFormattedLine("","POP","@SP:0D1");
	code.EmitFormattedLine("","DISCARD","#0D1");
	// ENDCODEGENERATION

	if ( tokens[0].type != PERIOD )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting '.'");

	GetNextToken(tokens);

	ExitModule("INPUTStatement");
}

void ParseAssignmentStatement(TOKEN tokens[])
{
	void ParseVariable(TOKEN tokens[],bool asLValue,DATATYPE &datatype);
	void ParseExpression(TOKEN tokens[],DATATYPE &datatype);
	void GetNextToken(TOKEN tokens[]);

	char line[SOURCELINELENGTH+1];
	DATATYPE datatypeLHS,datatypeRHS;
	int n;

	EnterModule("AssignmentStatement");

	sprintf(line,"; **** assignment statement (%4d)",tokens[0].sourceLineNumber);
	code.EmitUnformattedLine(line);

	ParseVariable(tokens,true,datatypeLHS);
	n = 1;

	while ( tokens[0].type == COMMA )
	{
		DATATYPE datatype;

		GetNextToken(tokens);
		ParseVariable(tokens,true,datatype);
		n++;

		if ( datatype != datatypeLHS )
			ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Mixed-mode variables not allowed");
	}
	
	if ( tokens[0].type != EQUAL )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting '='");
	GetNextToken(tokens);

	ParseExpression(tokens,datatypeRHS);

	if ( datatypeLHS != datatypeRHS )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Data type mismatch");

	// CODEGENERATION
	for (int i = 1; i <= n; i++)
	{
		code.EmitFormattedLine("","MAKEDUP");
		code.EmitFormattedLine("","POP","@SP:0D2");
		code.EmitFormattedLine("","SWAP");
		code.EmitFormattedLine("","DISCARD","#0D1");
	}
	code.EmitFormattedLine("","DISCARD","#0D1");
	// ENDCODEGENERATION

	if ( tokens[0].type != PERIOD )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting '.'");
	GetNextToken(tokens);

	ExitModule("AssignmentStatement");
}

void ParseIFStatement(TOKEN tokens[])
{
	void ParseExpression(TOKEN tokens[],DATATYPE &datatype);
	void ParseStatement(TOKEN tokens[]);
	void GetNextToken(TOKEN tokens[]);

	char line[SOURCELINELENGTH+1];
	char Ilabel[SOURCELINELENGTH+1],Elabel[SOURCELINELENGTH+1];
	DATATYPE datatype;

	EnterModule("IFStatement");

	sprintf(line,"; **** IF statement (%4d)",tokens[0].sourceLineNumber);
	code.EmitUnformattedLine(line);

	GetNextToken(tokens);

	if ( tokens[0].type != OPARENTHESIS )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting '('");
	GetNextToken(tokens);
	ParseExpression(tokens,datatype);
	if ( tokens[0].type != CPARENTHESIS )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting ')'");
	GetNextToken(tokens);

	if ( datatype != BOOLEANTYPE )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting boolean expression");

	// CODEGENERATION
	sprintf(Elabel,"E%04d",code.LabelSuffix());
	code.EmitFormattedLine("","SETT");
	code.EmitFormattedLine("","DISCARD","#0D1");
	sprintf(Ilabel,"I%04d",code.LabelSuffix());
	code.EmitFormattedLine("","JMPNT",Ilabel);
	// ENDCODEGENERATION

	while ( (tokens[0].type != ELIF) && 
            (tokens[0].type != ELSE) && 
            (tokens[0].type !=  END) )
		ParseStatement(tokens);

	// CODEGENERATION
	code.EmitFormattedLine("","JMP",Elabel);
	code.EmitFormattedLine(Ilabel,"EQU","*");
	// ENDCODEGENERATION

	while ( tokens[0].type == ELIF )
	{
		GetNextToken(tokens);
		if ( tokens[0].type != OPARENTHESIS )
			ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting '('");
		GetNextToken(tokens);
		ParseExpression(tokens,datatype);
		if ( tokens[0].type != CPARENTHESIS )
			ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting ')'");
		GetNextToken(tokens);

		if ( datatype != BOOLEANTYPE )
			ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting boolean expression");

		// CODEGENERATION
		code.EmitFormattedLine("","SETT");
		code.EmitFormattedLine("","DISCARD","#0D1");
		sprintf(Ilabel,"I%04d",code.LabelSuffix());
		code.EmitFormattedLine("","JMPNT",Ilabel);
		// ENDCODEGENERATION

		while ( (tokens[0].type != ELIF) && 
                (tokens[0].type != ELSE) && 
                (tokens[0].type !=  END) )
			ParseStatement(tokens);

		// CODEGENERATION
		code.EmitFormattedLine("","JMP",Elabel);
		code.EmitFormattedLine(Ilabel,"EQU","*");
		// ENDCODEGENERATION

	}
	if ( tokens[0].type == ELSE )
	{
		GetNextToken(tokens);
		while ( tokens[0].type != END )
			ParseStatement(tokens);
	}

	GetNextToken(tokens);

	// CODEGENERATION
	code.EmitFormattedLine(Elabel,"EQU","*");
	// ENDCODEGENERATION

   ExitModule("IFStatement");
}

/* // POTATO5 ONLY 
void ParseCHOOSEStatement(TOKEN tokens[])
{
	void ParseExpression(TOKEN tokens[],DATATYPE &datatype);
	void ParseStatement(TOKEN tokens[]);
	void GetNextToken(TOKEN tokens[]);

	char line[SOURCELINELENGTH+1];
	char Ilabel[SOURCELINELENGTH+1],Elabel1[SOURCELINELENGTH+1],Elabel2[SOURCELINELENGTH+1];
	DATATYPE datatype;
	char mode; // 'O' = ONE, 'A' = ALL

	EnterModule("CHOOSEStatement");

	sprintf(line,"; **** CHOOSE statement (%4d)",tokens[0].sourceLineNumber);
	code.EmitUnformattedLine(line);

	GetNextToken(tokens);

	if ( tokens[0].type == ONE )
	{
		mode = 'O';
		GetNextToken(tokens);
	} else if ( tokens[0].type == ALL )
	{
		mode = 'A';
		GetNextToken(tokens);
	} else
	{
		mode = 'O';
	}
		
	// CODEGENERATION
	sprintf(Elabel1,"E%04d",code.LabelSuffix());
	sprintf(Elabel2,"E%04d",code.LabelSuffix());
	code.EmitFormattedLine("","PUSH","#0D0","count-of-true <expressions> := 0");
	// ENDCODEGENERATION

	while ( tokens[0].type == WHEN )
	{
		GetNextToken(tokens);
		ParseExpression(tokens,datatype);
		if ( datatype != BOOLEANTYPE )
			ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting boolean expression");
		
		//GetNextToken(tokens);
		if ( tokens[0].type != COLON )
			ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting ':'");
		GetNextToken(tokens);

		// CODEGENERATION
		code.EmitFormattedLine("","SETT");
		code.EmitFormattedLine("","DISCARD","#0D1");
		sprintf(Ilabel,"I%04d",code.LabelSuffix());
		code.EmitFormattedLine("","JMPNT",Ilabel);
		code.EmitFormattedLine("","PUSH","#0D1","increment count-of-true <expressions>");
		code.EmitFormattedLine("","ADDI");
		// ENDCODEGENERATION

		while ( (tokens[0].type != WHEN) && 
              (tokens[0].type != ELSE) &&
              (tokens[0].type != END) )
			ParseStatement(tokens);

		// CODEGENERATION
		if ( mode == 'O' )
			code.EmitFormattedLine("","JMP",Elabel1);
		code.EmitFormattedLine(Ilabel,"EQU","*");
		// ENDCODEGENERATION
		
	}

	// CODEGENERATION
	code.EmitFormattedLine(Elabel1,"EQU","*");
	code.EmitFormattedLine("","SETNZPI","","if ( count-of-true <expressions> = 0 ) goto E???2");
	code.EmitFormattedLine("","DISCARD","#0D1");
	code.EmitFormattedLine("","JMPNZ",Elabel2);
	// ENDCODEGENERATION

	if ( tokens[0].type == ELSE )
	{
		GetNextToken(tokens);

		while ( tokens[0].type != END )
			ParseStatement(tokens);
	}

	if ( tokens[0].type != END )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting END");
	GetNextToken(tokens);

	// CODEGENERATION
		code.EmitFormattedLine(Elabel2,"EQU","*");
	// ENDCODEGENERATION

		ExitModule("CHOOSEStatement");
}
*/

void ParseDOWHILEStatement(TOKEN tokens[])
{
	void ParseExpression(TOKEN tokens[],DATATYPE &datatype);
	void ParseStatement(TOKEN tokens[]);
	void GetNextToken(TOKEN tokens[]);

	char line[SOURCELINELENGTH+1];
	char Dlabel[SOURCELINELENGTH+1],Elabel[SOURCELINELENGTH+1];
	DATATYPE datatype;

	EnterModule("DOWHILEStatement");

	sprintf(line,"; **** DO-WHILE statement (%4d)",tokens[0].sourceLineNumber);
	code.EmitUnformattedLine(line);

	GetNextToken(tokens);

	// CODEGENERATION
	sprintf(Dlabel,"D%04d",code.LabelSuffix());
	sprintf(Elabel,"E%04d",code.LabelSuffix());
	code.EmitFormattedLine(Dlabel,"EQU","*");
	// ENDCODEGENERATION

	while ( tokens[0].type != WHILE )
		ParseStatement(tokens);
	GetNextToken(tokens);
	if ( tokens[0].type != OPARENTHESIS )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting '('");
	GetNextToken(tokens);
	ParseExpression(tokens,datatype);
	if ( tokens[0].type != CPARENTHESIS )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting ')'");
	GetNextToken(tokens);

	if ( datatype != BOOLEANTYPE )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting boolean expression");

	// CODEGENERATION
	code.EmitFormattedLine("","SETT");
	code.EmitFormattedLine("","DISCARD","#0D1");
	code.EmitFormattedLine("","JMPNT",Elabel);
	// ENDCODEGENERATION

	while ( tokens[0].type != END )
		ParseStatement(tokens);

	GetNextToken(tokens);

	// CODEGENERATION
	code.EmitFormattedLine("","JMP",Dlabel);
	code.EmitFormattedLine(Elabel,"EQU","*");
	// ENDCODEGENERATION

   ExitModule("DOWHILEStatement");
}

/* // POTATO 4 ONLY													 
void ParseDO2WHILEStatement(TOKEN tokens[])
{
	void ParseExpression(TOKEN tokens[],DATATYPE &datatype);
	void ParseStatement(TOKEN tokens[]);
	void GetNextToken(TOKEN tokens[]);

	char line[SOURCELINELENGTH+1];
	char Dlabel[SOURCELINELENGTH+1],Elabel[SOURCELINELENGTH+1];
	DATATYPE datatype;

	EnterModule("DO2WHILEStatement");

	sprintf(line,"; **** DO2-WHILE statement (%4d)",tokens[0].sourceLineNumber);
	code.EmitUnformattedLine(line);

	GetNextToken(tokens);

	// CODEGENERATION										
	sprintf(Dlabel,"D%04d",code.LabelSuffix());
	sprintf(Elabel,"E%04d",code.LabelSuffix());
	
	code.EmitFormattedLine(Dlabel,"EQU","*");
	// ENDCODEGENERATION

	while ( tokens[0].type != WHILE )
		ParseStatement(tokens);
	GetNextToken(tokens);
	if ( tokens[0].type != OPARENTHESIS )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting '('");
	GetNextToken(tokens);
	ParseExpression(tokens,datatype);
	if ( tokens[0].type != CPARENTHESIS )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting ')'");
	GetNextToken(tokens);
	if ( tokens[0].type != PERIOD )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting '.'");
	GetNextToken(tokens);

	if ( datatype != BOOLEANTYPE )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting boolean expression");

	// CODEGENERATION
	code.EmitFormattedLine("","SETT");
	code.EmitFormattedLine("","DISCARD","#0D1");
	code.EmitFormattedLine("","JMPNT",Elabel);
	code.EmitFormattedLine("","JMP",Dlabel);
	code.EmitFormattedLine(Elabel,"EQU","*");
	// ENDCODEGENERATION

	ExitModule("DO2WHILEStatement");
}

void ParseWHILEStatement(TOKEN tokens[])
{
	void ParseExpression(TOKEN tokens[],DATATYPE &datatype);
	void ParseStatement(TOKEN tokens[]);
	void GetNextToken(TOKEN tokens[]);

	char line[SOURCELINELENGTH+1];
	char Dlabel[SOURCELINELENGTH+1],Elabel[SOURCELINELENGTH+1];
	DATATYPE datatype;

	EnterModule("WHILEStatement");

	sprintf(line,"; **** WHILE statement (%4d)",tokens[0].sourceLineNumber);
	code.EmitUnformattedLine(line);

	GetNextToken(tokens);

	// CODEGENERATION
	sprintf(Dlabel,"D%04d",code.LabelSuffix());
	sprintf(Elabel,"E%04d",code.LabelSuffix());
	code.EmitFormattedLine(Dlabel,"EQU","*");
	// ENDCODEGENERATION

	if ( tokens[0].type != OPARENTHESIS )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting '('");
	GetNextToken(tokens);
	ParseExpression(tokens,datatype);
	if ( tokens[0].type != CPARENTHESIS )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting ')'");
	GetNextToken(tokens);

	if ( datatype != BOOLEANTYPE )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting boolean expression");

	// CODEGENERATION
	code.EmitFormattedLine("","SETT");
	code.EmitFormattedLine("","DISCARD","#0D1");
	code.EmitFormattedLine("","JMPNT",Elabel);
	// ENDCODEGENERATION

	while ( tokens[0].type != END )
		ParseStatement(tokens);

	GetNextToken(tokens);

	// CODEGENERATION
	code.EmitFormattedLine("","JMP",Dlabel);
	code.EmitFormattedLine(Elabel,"EQU","*");
	// ENDCODEGENERATION

	ExitModule("WHILEStatement");
}
*/

void ParseFORStatement(TOKEN tokens[])
{
	void ParseAssignmentStatement(TOKEN tokens[]);
	void ParseExpression(TOKEN tokens[],DATATYPE &datatype);
	void ParseStatement(TOKEN tokens[]);
	void GetNextToken(TOKEN tokens[]);

	char line[SOURCELINELENGTH+1];
	char E2label[SOURCELINELENGTH+1],S3label[SOURCELINELENGTH+1],
         Clabel[SOURCELINELENGTH+1],Elabel[SOURCELINELENGTH+1];
	char operand[SOURCELINELENGTH+1];
	DATATYPE datatype;

	EnterModule("FORStatement");

	sprintf(line,"; **** FOR statement (%4d)",tokens[0].sourceLineNumber);
	code.EmitUnformattedLine(line);

	GetNextToken(tokens);

	if ( tokens[0].type != OPARENTHESIS )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting '('");
	GetNextToken(tokens);

	if ( tokens[0].type != PERIOD )
	{
		ParseAssignmentStatement(tokens);

		// CODEGENERATION
		sprintf(E2label,"E2%04d",code.LabelSuffix());
		sprintf(S3label,"S3%04d",code.LabelSuffix());
		sprintf(Clabel, "C%04d", code.LabelSuffix());
		sprintf(Elabel, "E%04d", code.LabelSuffix());

		code.EmitFormattedLine("","PUSH","#0X0000","doS3 := FALSE");
		code.EmitFormattedLine("","JMP",S3label,"doS3");
		code.EmitFormattedLine(E2label,"EQU","*");
		// ENDCODEGENERATION
	} else
		GetNextToken(tokens);
	
	ParseExpression(tokens,datatype);
									 
	if ( datatype != BOOLEANTYPE )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting boolean expression");
	if ( tokens[0].type != PERIOD )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting '.'");
	GetNextToken(tokens);

	// CODEGENERATION
	code.EmitFormattedLine("","SETT","","e2");
	code.EmitFormattedLine("","DISCARD","#0D1","(empty)");
	code.EmitFormattedLine("","JMPT",Clabel,"(empty)");
	code.EmitFormattedLine("","JMP",Elabel,"(empty)");
	code.EmitFormattedLine(S3label,"EQU","*","doS3");
	code.EmitFormattedLine("","SETT","","doS3");
	code.EmitFormattedLine("","DISCARD","#0D1","(empty)");
	code.EmitFormattedLine("","JMPNT",E2label,"(empty)");
	// ENDCODEGENERATION

	if ( tokens[0].type != CPARENTHESIS )
	{
		ParseAssignmentStatement(tokens);

		// CODEGENERATION
		code.EmitFormattedLine("","JMP",E2label,"(empty)");
		code.EmitFormattedLine(Clabel,"EQU","*","");
		// ENDCODEGENERATION
	}
	
	if ( tokens[0].type != CPARENTHESIS )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting ')'");
	
	GetNextToken(tokens);
		
	while ( tokens[0].type != END )
		ParseStatement(tokens);

	GetNextToken(tokens);

	// CODEGENERATION
	code.EmitFormattedLine("","PUSH","#0XFFFF","doS3 := TRUE");
	code.EmitFormattedLine("","JMP",S3label,"doS3");
	code.EmitFormattedLine(Elabel,"EQU","*");
	// ENDCODEGENERATION

	ExitModule("FORStatement");
}
  
/*// POTATO5 ONLY
void ParseFOR2Statement(TOKEN tokens[])
{
   void ParseVariable(TOKEN tokens[],bool asLValue,DATATYPE &datatype);
   void ParseExpression(TOKEN tokens[],DATATYPE &datatype);
   void ParseStatement(TOKEN tokens[]);
   void GetNextToken(TOKEN tokens[]);

   char line[SOURCELINELENGTH+1];
   char Dlabel[SOURCELINELENGTH+1],Llabel[SOURCELINELENGTH+1],
        Clabel[SOURCELINELENGTH+1],Elabel[SOURCELINELENGTH+1];
   char operand[SOURCELINELENGTH+1];
   DATATYPE datatype;

   EnterModule("FOR2Statement");

   sprintf(line,"; **** FOR2 statement (%4d)",tokens[0].sourceLineNumber);
   code.EmitUnformattedLine(line);

   GetNextToken(tokens);

   ParseVariable(tokens,true,datatype);

   if ( datatype != INTEGERTYPE )
      ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting integer variable");

   if ( tokens[0].type != EQUAL)
      ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting '='");
   GetNextToken(tokens);

   ParseExpression(tokens,datatype);
   if ( datatype != INTEGERTYPE )
      ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting integer data type");

// CODEGENERATION
   code.EmitFormattedLine("","POP","@SP:0D1");
// ENDCODEGENERATION

   if ( tokens[0].type != TO )
      ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting TO");
   GetNextToken(tokens);

   ParseExpression(tokens,datatype);
   if ( datatype != INTEGERTYPE )
      ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting integer data type");

   if ( tokens[0].type == BY )
   {
      GetNextToken(tokens);

      ParseExpression(tokens,datatype);
      if ( datatype != INTEGERTYPE )
         ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting integer data type");
   }
   else
   {

// CODEGENERATION
      code.EmitFormattedLine("","PUSH","#0D1");
// ENDCODEGENERATION

   }

// CODEGENERATION
   sprintf(Dlabel,"D%04d",code.LabelSuffix());
   sprintf(Llabel,"L%04d",code.LabelSuffix());
   sprintf(Clabel,"C%04d",code.LabelSuffix());
   sprintf(Elabel,"E%04d",code.LabelSuffix());

   code.EmitFormattedLine("","SETNZPI");
   code.EmitFormattedLine("","JMPNZ",Dlabel);
   sprintf(operand,"#0D%d",tokens[0].sourceLineNumber);
   code.EmitFormattedLine("","PUSH",operand);
   code.EmitFormattedLine("","PUSH","#0D2");
   code.EmitFormattedLine("","JMP","HANDLERUNTIMEERROR");

   code.EmitFormattedLine(Dlabel,"SETNZPI");
   code.EmitFormattedLine("","JMPN",Llabel);
   code.EmitFormattedLine("","SWAP");
   code.EmitFormattedLine("","MAKEDUP");
   code.EmitFormattedLine("","PUSH","@SP:0D3");
   code.EmitFormattedLine("","SWAP");
   code.EmitFormattedLine("","CMPI");
   code.EmitFormattedLine("","JMPLE",Clabel);
   code.EmitFormattedLine("","JMP",Elabel);
   code.EmitFormattedLine(Llabel,"SWAP");
   code.EmitFormattedLine("","MAKEDUP");
   code.EmitFormattedLine("","PUSH","@SP:0D3");
   code.EmitFormattedLine("","SWAP");
   code.EmitFormattedLine("","CMPI");
   code.EmitFormattedLine("","JMPGE",Clabel);
   code.EmitFormattedLine("","JMP",Elabel);
   code.EmitFormattedLine(Clabel,"EQU","*");
// ENDCODEGENERATION

   while ( tokens[0].type != END )
      ParseStatement(tokens);

   GetNextToken(tokens);

// CODEGENERATION
   code.EmitFormattedLine("","SWAP");
   code.EmitFormattedLine("","MAKEDUP");
   code.EmitFormattedLine("","PUSH","@SP:0D3");
   code.EmitFormattedLine("","ADDI");
   code.EmitFormattedLine("","POP","@SP:0D3");
   code.EmitFormattedLine("","JMP",Dlabel);
   code.EmitFormattedLine(Elabel,"DISCARD","#0D3");
// ENDCODEGENERATION

   ExitModule("FOR2Statement");
}
*/

void ParseCALLStatement(TOKEN tokens[])
{
	void ParseVariable(TOKEN tokens[],bool asLValue,DATATYPE &datatype);
	void ParseExpression(TOKEN tokens[],DATATYPE &datatype);
	void GetNextToken(TOKEN tokens[]);

	char line[SOURCELINELENGTH+1];
	bool isInTable;
	int index,parameters;

	EnterModule("CALLStatement");

	sprintf(line,"; **** CALL statement (%4d)",tokens[0].sourceLineNumber);
	code.EmitUnformattedLine(line);

	GetNextToken(tokens);

	if ( tokens[0].type != IDENTIFIER )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting identifier call");

	// STATICSEMANTICS
	index = identifierTable.GetIndex(tokens[0].lexeme,isInTable);
	if ( !isInTable )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Undefined identifier");
	if ( identifierTable.GetType(index) != PROCEDURE_SUBPROGRAMMODULE )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting PROCEDURE identifier");
	// ENDSTATICSEMANTICS

	GetNextToken(tokens);
	parameters = 0;
	if ( tokens[0].type == OPARENTHESIS )
	{
		DATATYPE expressionDatatype,variableDatatype;

		do
		{
			GetNextToken(tokens);
			parameters++;

			// CODEGENERATION   
			// STATICSEMANTICS
			switch ( identifierTable.GetType(index+parameters) )
			{
				case IN_PARAMETER:
					ParseExpression(tokens,expressionDatatype);
					if ( expressionDatatype != identifierTable.GetDatatype(index+parameters) )
						ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,
							"Actual parameter data type does not match formal parameter data type");
					break;
				case OUT_PARAMETER:
					ParseVariable(tokens,true,variableDatatype);
					if ( variableDatatype != identifierTable.GetDatatype(index+parameters) )
						ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,
							"Actual parameter data type does not match formal parameter data type");
					code.EmitFormattedLine("","PUSH","#0X0000");
					break;
				case IO_PARAMETER:
					ParseVariable(tokens,true,variableDatatype);
					if ( variableDatatype != identifierTable.GetDatatype(index+parameters) )
						ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,
							"Actual parameter data type does not match formal parameter data type");
					code.EmitFormattedLine("","PUSH","@SP:0D0");
					break;
				case REF_PARAMETER:
					ParseVariable(tokens,true,variableDatatype);
					if ( variableDatatype != identifierTable.GetDatatype(index+parameters) )
						ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,
							"Actual parameter data type does not match formal parameter data type");
					break;
			}
			// ENDSTATICSEMANTICS
			// ENDCODEGENERATION
      } while ( tokens[0].type == COMMA );

		if ( tokens[0].type != CPARENTHESIS )
			ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting )");

		GetNextToken(tokens);
	}
                     
	// STATICSEMANTICS
	if ( identifierTable.GetCountOfFormalParameters(index) != parameters )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,
			"Number of actual parameters does not match number of formal parameters");
	// ENDSTATICSEMANTICS

	// CODEGENERATION
	code.EmitFormattedLine("","PUSHFB");
	code.EmitFormattedLine("","CALL",identifierTable.GetReference(index));
	code.EmitFormattedLine("","POPFB");
	for (parameters = identifierTable.GetCountOfFormalParameters(index); parameters >= 1; parameters--)
	{
		switch ( identifierTable.GetType(index+parameters) )
		{
			case IN_PARAMETER:
				code.EmitFormattedLine("","DISCARD","#0D1");
				break;
			case OUT_PARAMETER:
				code.EmitFormattedLine("","POP","@SP:0D1");
				code.EmitFormattedLine("","DISCARD","#0D1");
				break;
			case IO_PARAMETER:
				code.EmitFormattedLine("","POP","@SP:0D1");
				code.EmitFormattedLine("","DISCARD","#0D1");
				break;
			case REF_PARAMETER:
				code.EmitFormattedLine("","DISCARD","#0D1");
				break;
		}
	}
	// ENDCODEGENERATION

	if ( tokens[0].type != PERIOD )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting '.'");

	GetNextToken(tokens);

	ExitModule("CALLStatement");
}

void ParseRETURNStatement(TOKEN tokens[])
{
   void ParseExpression(TOKEN tokens[],DATATYPE &datatype);													   
	void GetNextToken(TOKEN tokens[]);

	char line[SOURCELINELENGTH+1];

	EnterModule("RETURNStatement");

	sprintf(line,"; **** RETURN statement (%4d)",tokens[0].sourceLineNumber);
	code.EmitUnformattedLine(line);

	GetNextToken(tokens);

	// STATICSEMANTICS
	if ( code.IsInModuleBody(PROCEDURE_SUBPROGRAMMODULE) )
	// CODEGENERATION
		code.EmitFormattedLine("","RETURN");
	// ENDCODEGENERATION
	else if ( code.IsInModuleBody( FUNCTION_SUBPROGRAMMODULE) )
	{
		DATATYPE datatype;

		if ( tokens[0].type != OPARENTHESIS )
			ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting '('");
		GetNextToken(tokens);
   
		ParseExpression(tokens,datatype);

		if ( datatype != identifierTable.GetDatatype(code.GetModuleIdentifierIndex()) )
			ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,
				"RETURN expression data type must match FUNCTION data type");
   
		// CODEGENERATION
		code.EmitFormattedLine("","POP","FB:0D0","pop RETURN expression into function return value");
		code.EmitFormattedLine("","RETURN");
		// ENDCODEGENERATION

		if ( tokens[0].type != CPARENTHESIS )
			ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting ')'");
		GetNextToken(tokens);
	}
	else
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,
			"RETURN only allowed in PROCEDURE or FUNCTION module body");
	// ENDSTATICSEMANTICS

	if ( tokens[0].type != PERIOD )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting .");

	GetNextToken(tokens);

	ExitModule("RETURNStatement");
}
void ParseExpression(TOKEN tokens[],DATATYPE &datatype)
{

	void ParseConjunction(TOKEN tokens[],DATATYPE &datatype);
	void GetNextToken(TOKEN tokens[]);

	DATATYPE datatypeLHS,datatypeRHS;

	EnterModule("Expression");

	ParseConjunction(tokens,datatypeLHS);

	if ( (tokens[0].type ==  OR) ||
        (tokens[0].type == NOR) ||
        (tokens[0].type == XOR) )
	{
		while ( (tokens[0].type ==  OR) ||
              (tokens[0].type == NOR) ||
              (tokens[0].type == XOR) )
		{
			TOKENTYPE operation = tokens[0].type;

			GetNextToken(tokens);
			ParseConjunction(tokens,datatypeRHS);
   
			// CODEGENERATION
			switch ( operation )
			{
				case OR:
					// STATICSEMANTICS
					if ( !((datatypeLHS == BOOLEANTYPE) && (datatypeRHS == BOOLEANTYPE)) )
						ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting boolean operands");
					// ENDSTATICSEMANTICS
					code.EmitFormattedLine("","OR");
					datatype = BOOLEANTYPE;
					break;
				case NOR:
					// STATICSEMANTICS
					if ( !((datatypeLHS == BOOLEANTYPE) && (datatypeRHS == BOOLEANTYPE)) )
						ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting boolean operands");
					// ENDSTATICSEMANTICS
					code.EmitFormattedLine("","NOR");
					datatype = BOOLEANTYPE;
					break;
				case XOR:
					// STATICSEMANTICS
					if ( !((datatypeLHS == BOOLEANTYPE) && (datatypeRHS == BOOLEANTYPE)) )
						ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting boolean operands");
					// ENDSTATICSEMANTICS
					code.EmitFormattedLine("","XOR");
					datatype = BOOLEANTYPE;
				break;
			}
		}
	// CODEGENERATION
	}
	else
		datatype = datatypeLHS;

	ExitModule("Expression");
}

void ParseConjunction(TOKEN tokens[],DATATYPE &datatype)
{
	void ParseNegation(TOKEN tokens[],DATATYPE &datatype);
	void GetNextToken(TOKEN tokens[]);

	DATATYPE datatypeLHS,datatypeRHS;

	EnterModule("Conjunction");

	ParseNegation(tokens,datatypeLHS);

	if ( (tokens[0].type ==  AND) ||
        (tokens[0].type == NAND) )
	{
		while ( (tokens[0].type ==  AND) ||
              (tokens[0].type == NAND) )
		{
			TOKENTYPE operation = tokens[0].type;
  
			GetNextToken(tokens);
			ParseNegation(tokens,datatypeRHS);
   
			switch ( operation )
			{
				case AND:
					if ( !((datatypeLHS == BOOLEANTYPE) && (datatypeRHS == BOOLEANTYPE)) )
						ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting boolean operands");
					code.EmitFormattedLine("","AND");
					datatype = BOOLEANTYPE;
					break;
				case NAND:
					if ( !((datatypeLHS == BOOLEANTYPE) && (datatypeRHS == BOOLEANTYPE)) )
						ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting boolean operands");
					code.EmitFormattedLine("","NAND");
					datatype = BOOLEANTYPE;
					break;
			}
		}	
	}
	else
		datatype = datatypeLHS;

	ExitModule("Conjunction");
}

void ParseNegation(TOKEN tokens[],DATATYPE &datatype)
{
	void ParseComparison(TOKEN tokens[],DATATYPE &datatype);
	void GetNextToken(TOKEN tokens[]);

	DATATYPE datatypeRHS;

	EnterModule("Negation");

	if ( tokens[0].type == NOT )
	{
		GetNextToken(tokens);
		ParseComparison(tokens,datatypeRHS);

		if ( !(datatypeRHS == BOOLEANTYPE) )
			ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting boolean operand");
		code.EmitFormattedLine("","NOT");
		datatype = BOOLEANTYPE;
	}
	else
		ParseComparison(tokens,datatype);

	ExitModule("Negation");
}

void ParseComparison(TOKEN tokens[],DATATYPE &datatype)
{
	void ParseComparator(TOKEN tokens[],DATATYPE &datatype);
	void GetNextToken(TOKEN tokens[]);

	DATATYPE datatypeLHS,datatypeRHS;

	EnterModule("Comparison");

	ParseComparator(tokens,datatypeLHS);
	if ( (tokens[0].type ==    LT) ||
		 (tokens[0].type ==  LTEQ) ||
         (tokens[0].type ==    EQ) ||
         (tokens[0].type ==    GT) ||
         (tokens[0].type ==  GTEQ) ||
         (tokens[0].type == NOTEQ)
		)
	{
		TOKENTYPE operation = tokens[0].type;

		GetNextToken(tokens);
        ParseComparator(tokens,datatypeRHS);

		if ( (datatypeLHS != INTEGERTYPE) || (datatypeRHS != INTEGERTYPE) )
			ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting integer operands");

		char Tlabel[SOURCELINELENGTH+1],Elabel[SOURCELINELENGTH+1];

		code.EmitFormattedLine("","CMPI");
		sprintf(Tlabel,"T%04d",code.LabelSuffix());
		sprintf(Elabel,"E%04d",code.LabelSuffix());
		switch ( operation )
		{
			case LT:
				code.EmitFormattedLine("","JMPL",Tlabel);
				break;
			case LTEQ:
				code.EmitFormattedLine("","JMPLE",Tlabel);
				break;
			case EQ:
				code.EmitFormattedLine("","JMPE",Tlabel);
				break;
			case GT:
				code.EmitFormattedLine("","JMPG",Tlabel);
				break;
			case GTEQ:
				code.EmitFormattedLine("","JMPGE",Tlabel);
				break;
			case NOTEQ:
				code.EmitFormattedLine("","JMPNE",Tlabel);
				break;
		}
		datatype = BOOLEANTYPE;
		code.EmitFormattedLine("","PUSH","#0X0000");
		code.EmitFormattedLine("","JMP",Elabel);
		code.EmitFormattedLine(Tlabel,"PUSH","#0XFFFF");
		code.EmitFormattedLine(Elabel,"EQU","*");
	}
	else
      datatype = datatypeLHS;

	ExitModule("Comparison");
}

void ParseComparator(TOKEN tokens[],DATATYPE &datatype)
{
	void ParseTerm(TOKEN tokens[],DATATYPE &datatype);
	void GetNextToken(TOKEN tokens[]);

	DATATYPE datatypeLHS,datatypeRHS;

	EnterModule("Comparator");

	ParseTerm(tokens,datatypeLHS);

	if ( (tokens[0].type ==  PLUS) ||
        (tokens[0].type == MINUS) )
	{
		while ( (tokens[0].type ==  PLUS) ||
              (tokens[0].type == MINUS) )
		{
			TOKENTYPE operation = tokens[0].type;
         
			GetNextToken(tokens);
			ParseTerm(tokens,datatypeRHS);

			if ( (datatypeLHS != INTEGERTYPE) || (datatypeRHS != INTEGERTYPE) )
				ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting integer operands");

			switch ( operation )
			{
				case PLUS:
					code.EmitFormattedLine("","ADDI");
					break;
				case MINUS:
					code.EmitFormattedLine("","SUBI");
					break;
			}
			datatype = INTEGERTYPE;
		}
	}
	else
		datatype = datatypeLHS;
   
	ExitModule("Comparator");
}

void ParseTerm(TOKEN tokens[],DATATYPE &datatype)
{
	void ParseFactor(TOKEN tokens[],DATATYPE &datatype);
	void GetNextToken(TOKEN tokens[]);

	DATATYPE datatypeLHS,datatypeRHS;

	EnterModule("Term");

	ParseFactor(tokens,datatypeLHS);
	if ( (tokens[0].type == MULTIPLY) ||
        (tokens[0].type ==   DIVIDE) ||
        (tokens[0].type ==  MODULUS) )
	{
		while ( (tokens[0].type == MULTIPLY) ||
              (tokens[0].type ==   DIVIDE) ||
              (tokens[0].type ==  MODULUS) )
		{
			TOKENTYPE operation = tokens[0].type;
         
			GetNextToken(tokens);
			ParseFactor(tokens,datatypeRHS);

			if ( (datatypeLHS != INTEGERTYPE) || (datatypeRHS != INTEGERTYPE) )
				ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting integer operands");

			switch ( operation )
			{
				case MULTIPLY:
					code.EmitFormattedLine("","MULI");
					break;
				case DIVIDE:
					code.EmitFormattedLine("","DIVI");
					break;
				case MODULUS:
					code.EmitFormattedLine("","REMI");
					break;
			}
			datatype = INTEGERTYPE;
		}
	}
	else
		datatype = datatypeLHS;

	ExitModule("Term");
}

void ParseFactor(TOKEN tokens[],DATATYPE &datatype)
{
	void ParseSecondary(TOKEN tokens[],DATATYPE &datatype);
	void GetNextToken(TOKEN tokens[]);

	EnterModule("Factor");

	if ( (tokens[0].type ==   ABS) ||
         (tokens[0].type ==  PLUS) ||
         (tokens[0].type == MINUS))
	{
		DATATYPE datatypeRHS;
		TOKENTYPE operation = tokens[0].type;

		GetNextToken(tokens);
		ParseSecondary(tokens,datatypeRHS);

		if ( datatypeRHS != INTEGERTYPE )
			ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting integer operand");

		switch ( operation )
		{
			case ABS:
				{
				char Elabel[SOURCELINELENGTH+1];
         
				sprintf(Elabel,"E%04d",code.LabelSuffix());
				code.EmitFormattedLine("","SETNZPI");
				code.EmitFormattedLine("","JMPNN",Elabel);
				code.EmitFormattedLine("","NEGI");
				code.EmitFormattedLine(Elabel,"EQU","*");
				}
				break;
			case PLUS:
			// Do nothing (identity operator)
				break;
			case MINUS:
				code.EmitFormattedLine("","NEGI");
				break;
		}
		datatype = INTEGERTYPE;
	}
	else
		ParseSecondary(tokens,datatype);

	ExitModule("Factor");
}

void ParseSecondary(TOKEN tokens[],DATATYPE &datatype)
{
	void ParsePrefix(TOKEN tokens[],DATATYPE &datatype);
	void GetNextToken(TOKEN tokens[]);

	DATATYPE datatypeLHS,datatypeRHS;

	EnterModule("Secondary");

	ParsePrefix(tokens,datatypeLHS);

	if ( tokens[0].type == POWER )
	{
		GetNextToken(tokens);

		ParsePrefix(tokens,datatypeRHS);

		if ( (datatypeLHS != INTEGERTYPE) || (datatypeRHS != INTEGERTYPE) )
			ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting integer operands");

		code.EmitFormattedLine("","POWI");
		datatype = INTEGERTYPE;
	}
	else
		datatype = datatypeLHS;

	ExitModule("Secondary");
}

void ParsePrefix(TOKEN tokens[],DATATYPE &datatype)
{
	void ParseVariable(TOKEN tokens[],bool asLValue,DATATYPE &datatype);
	void ParsePrimary(TOKEN tokens[],DATATYPE &datatype);
	void GetNextToken(TOKEN tokens[]);

	EnterModule("Prefix");

	if ( (tokens[0].type == INC) || (tokens[0].type == DEC) )
	{
		DATATYPE datatypeRHS;
		TOKENTYPE operation = tokens[0].type;

		GetNextToken(tokens);
		ParseVariable(tokens,true,datatypeRHS);

		if ( datatypeRHS != INTEGERTYPE )
			ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting integer operand");

		switch ( operation )
		{
			case INC:
				code.EmitFormattedLine("","PUSH","@SP:0D0");
				code.EmitFormattedLine("","PUSH","#0D1");
				code.EmitFormattedLine("","ADDI");
				code.EmitFormattedLine("","POP","@SP:0D1");       // side-effect
				code.EmitFormattedLine("","PUSH","@SP:0D0");
				code.EmitFormattedLine("","SWAP");
				code.EmitFormattedLine("","DISCARD","#0D1");      // value
				break;
			case DEC:
				code.EmitFormattedLine("","PUSH","@SP:0D0");
				code.EmitFormattedLine("","PUSH","#0D1");
				code.EmitFormattedLine("","SUBI");
				code.EmitFormattedLine("","POP","@SP:0D1");       // side-effect
				code.EmitFormattedLine("","PUSH","@SP:0D0");
				code.EmitFormattedLine("","SWAP");
				code.EmitFormattedLine("","DISCARD","#0D1");      // value
				break;
		}
		datatype = INTEGERTYPE;
	}
	else
		ParsePrimary(tokens,datatype);

	ExitModule("Prefix");
}

void ParsePrimary(TOKEN tokens[],DATATYPE &datatype)
{
	void ParseVariable(TOKEN tokens[],bool asLValue,DATATYPE &datatype);																   
	void ParseExpression(TOKEN tokens[],DATATYPE &datatype);
	void GetNextToken(TOKEN tokens[]);

	EnterModule("Primary");

	switch ( tokens[0].type )
	{
		case INTEGER:
			{
            char operand[SOURCELINELENGTH+1];
            
            sprintf(operand,"#0D%s",tokens[0].lexeme);
            code.EmitFormattedLine("","PUSH",operand);
            datatype = INTEGERTYPE;
            GetNextToken(tokens);
			}
			break;
		case TRUE:
			code.EmitFormattedLine("","PUSH","#0XFFFF");
			datatype = BOOLEANTYPE;
			GetNextToken(tokens);
			break;
		case FALSE:
			code.EmitFormattedLine("","PUSH","#0X0000");
			datatype = BOOLEANTYPE;
			GetNextToken(tokens);
			break;
		case OPARENTHESIS:
			GetNextToken(tokens);
			ParseExpression(tokens,datatype);
			if ( tokens[0].type != CPARENTHESIS )
				ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting ')'");
			GetNextToken(tokens);
			break;
		case IDENTIFIER:
		{
            bool isInTable;
            int index;
   
            index = identifierTable.GetIndex(tokens[0].lexeme,isInTable);
            if ( !isInTable )
				ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Undefined identifier");
				// variable reference
            if ( identifierTable.GetType(index) != FUNCTION_SUBPROGRAMMODULE )
				ParseVariable(tokens,false,datatype);
				// FUNCTION_SUBPROGRAMMODULE reference
            else
            {
				char operand[MAXIMUMLENGTHIDENTIFIER+1];
				int parameters;

				GetNextToken(tokens);
				if ( tokens[0].type != OPARENTHESIS )
					ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting '('");

				// CODEGENERATION
				code.EmitFormattedLine("","PUSH","#0X0000","reserve space for function return value");
				// ENDCODEGENERATION

				datatype = identifierTable.GetDatatype(index);
				parameters = 0;
				
				if ( tokens[1].type == CPARENTHESIS )
					GetNextToken(tokens);
				else
				{
					do
					{
						DATATYPE expressionDatatype;

						GetNextToken(tokens);
						ParseExpression(tokens,expressionDatatype);
						parameters++;
                     
						// STATICSEMANTICS
						if ( expressionDatatype != identifierTable.GetDatatype(index+parameters) )
							ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,
								"Actual parameter data type does not match formal parameter data type");
						// ENDSTATICSEMANTICS

					} while ( tokens[0].type == COMMA );
				}
                     
				// STATICSEMANTICS
				if ( identifierTable.GetCountOfFormalParameters(index) != parameters )
					ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,
						"Number of actual parameters does not match number of formal parameters");
				// ENDSTATICSEMANTICS

				if ( tokens[0].type != CPARENTHESIS )
					ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting ')'");
				GetNextToken(tokens);

				// CODEGENERATION
				code.EmitFormattedLine("","PUSHFB");
				code.EmitFormattedLine("","CALL",identifierTable.GetReference(index));
				code.EmitFormattedLine("","POPFB");
				sprintf(operand,"#0D%d",parameters);
				code.EmitFormattedLine("","DISCARD",operand);
				// ENDCODEGENERATION
            }
         }		  
			break;
		default:
			ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,
				"Expecting integer, true, false, '(', or variable");											   
			break;
	}

	ExitModule("Primary");
}
													 
void ParseVariable(TOKEN tokens[],bool asLValue,DATATYPE &datatype)
{

	void GetNextToken(TOKEN tokens[]);

	bool isInTable;
	int index;
	IDENTIFIERTYPE identifierType;

	EnterModule("Variable");

	if ( tokens[0].type != IDENTIFIER )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting identifier variable");

	// STATICSEMANTICS
	index = identifierTable.GetIndex(tokens[0].lexeme,isInTable);
	if ( !isInTable )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Undefined identifier");
   
	identifierType = identifierTable.GetType(index);
	datatype = identifierTable.GetDatatype(index);

	if ( !((identifierType ==           GLOBAL_VARIABLE) ||
           (identifierType ==           GLOBAL_CONSTANT) ||
           (identifierType ==    PROGRAMMODULE_VARIABLE) ||
           (identifierType ==    PROGRAMMODULE_CONSTANT) ||
           (identifierType == SUBPROGRAMMODULE_VARIABLE) ||
           (identifierType == SUBPROGRAMMODULE_CONSTANT) ||
           (identifierType ==              IN_PARAMETER) ||
           (identifierType ==             OUT_PARAMETER) ||
           (identifierType ==              IO_PARAMETER) ||
           (identifierType ==             REF_PARAMETER)) ) 
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting variable or constant identifier");
      
	if ( asLValue && ((identifierType ==           GLOBAL_CONSTANT) || 
					  (identifierType ==    PROGRAMMODULE_CONSTANT) ||
					  (identifierType == SUBPROGRAMMODULE_CONSTANT)) )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Constant may not be l-value");

	if ( asLValue && (identifierType == GLOBAL_VARIABLE) && code.IsInModuleBody(FUNCTION_SUBPROGRAMMODULE) )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"FUNCTION may not modify global variable");																														   
	// ENDSTATICSEMANTICS

	// CODEGENERATION
	if ( asLValue )
		code.EmitFormattedLine("","PUSHA",identifierTable.GetReference(index));
	else
		code.EmitFormattedLine("","PUSH",identifierTable.GetReference(index));
	// ENDCODEGENERATION

	GetNextToken(tokens);

	ExitModule("Variable");
}
												 
void Callback1(int sourceLineNumber,const char sourceLine[])
{
	cout << setw(4) << sourceLineNumber << " " << sourceLine << endl;
}

void Callback2(int sourceLineNumber,const char sourceLine[])
{
	char line[SOURCELINELENGTH+1];

	// CODEGENERATION
	sprintf(line,"; %4d %s",sourceLineNumber,sourceLine);
	code.EmitUnformattedLine(line);
	// ENDCODEGENERATION
}

void GetNextToken(TOKEN tokens[])
{
	const char *TokenDescription(TOKENTYPE type);

	int i;
	TOKENTYPE type;
	char lexeme[SOURCELINELENGTH+1];
	int sourceLineNumber;
	int sourceLineIndex;
	char information[SOURCELINELENGTH+1];

//============================================================
// Move look-ahead "window" to make room for next token-and-lexeme
//============================================================
	for (int i = 1; i <= LOOKAHEAD; i++)
		tokens[i-1] = tokens[i];

	char nextCharacter = reader.GetLookAheadCharacter(0).character;

//============================================================
// "Eat" white space and comments
//============================================================
	do
	{
		//  "Eat" any white-space (blanks and EOLCs and TABCs) 
		while ( (nextCharacter == ' ') 
				|| (nextCharacter == READER<CALLBACKSUSED>::EOLC)
				|| (nextCharacter == READER<CALLBACKSUSED>::TABC) ) 
			nextCharacter = reader.GetNextCharacter().character;

		//  "Eat" line comment
		if ( (nextCharacter == '|') && (reader.GetLookAheadCharacter(1).character == '|') )
		{
		  
// Displaying trace information
#ifdef TRACESCANNER 
	sprintf(information,"At (%4d:%3d) begin line comment",
		reader.GetLookAheadCharacter(0).sourceLineNumber,
		reader.GetLookAheadCharacter(0).sourceLineIndex);
	lister.ListInformationLine(information);
#endif

			do
				nextCharacter = reader.GetNextCharacter().character;
			while ( nextCharacter != READER<CALLBACKSUSED>::EOLC );
		} 

		// 	"Eat" block comments (nesting allowed)
		if ( (nextCharacter == '|') && (reader.GetLookAheadCharacter(1).character == '[') )
		{
			int depth = 0;

			do
			{
				if ( (nextCharacter == '|') && (reader.GetLookAheadCharacter(1).character == '[') )
				{
					depth++;

// Displaying trace information
#ifdef TRACESCANNER
	sprintf(information,"At (%4d:%3d) begin block comment depth = %d",
		reader.GetLookAheadCharacter(0).sourceLineNumber,
		reader.GetLookAheadCharacter(0).sourceLineIndex,
		depth);
	lister.ListInformationLine(information);
#endif

					nextCharacter = reader.GetNextCharacter().character;
					nextCharacter = reader.GetNextCharacter().character;
				}
				else if ( (nextCharacter == ']') && (reader.GetLookAheadCharacter(1).character == '|') )
				{

// Displaying trace information
#ifdef TRACESCANNER
	sprintf(information,"At (%4d:%3d)   end block comment depth = %d",
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
			} while ( (depth != 0) && (nextCharacter != READER<CALLBACKSUSED>::EOPC) );
			if ( depth != 0 ) 
				ProcessCompilerError(reader.GetLookAheadCharacter(0).sourceLineNumber,
                                 reader.GetLookAheadCharacter(0).sourceLineIndex,
                                 "Unexpected end-of-program");
		}
	/* WHILE ( (nextCharacter is not a white-space character)
       or (nextCharacter is not beginning-of-comment character) ) */
	} while ( (nextCharacter == ' ')
          || (nextCharacter == READER<CALLBACKSUSED>::EOLC)
          || (nextCharacter == READER<CALLBACKSUSED>::TABC)
          || (nextCharacter == ';')
          || ((nextCharacter == '|') && (reader.GetLookAheadCharacter(1).character == '['))   // allows block comments back to back
		  || ((nextCharacter == '|') && (reader.GetLookAheadCharacter(1).character == '|'))); // allows inline comments after block comment

//============================================================
// Scan token
//============================================================
	sourceLineNumber = reader.GetLookAheadCharacter(0).sourceLineNumber;
	sourceLineIndex = reader.GetLookAheadCharacter(0).sourceLineIndex;

	// IF ( nextCharacter begins a reserved word or an <identifier> ) THEN
	if ( isalpha(nextCharacter) )
	{
		// Build lexeme
		char UCLexeme[SOURCELINELENGTH+1];

		i = 0;
		lexeme[i++] = nextCharacter;
		nextCharacter = reader.GetNextCharacter().character;
		while ( isalpha(nextCharacter) || isdigit(nextCharacter) || (nextCharacter == '_') )
		{
			lexeme[i++] = nextCharacter;
			nextCharacter = reader.GetNextCharacter().character;
		}
		lexeme[i] = '\0';
		for (i = 0; i <= (int) strlen(lexeme); i++)
			UCLexeme[i] = toupper(lexeme[i]);

		// Try to find the lexeme in the table of reserved words.
		bool isFound = false;

		i = 0;
		while ( !isFound && (i <= (sizeof(TOKENTABLE)/sizeof(TOKENTABLERECORD))-1) )
		{
			if ( TOKENTABLE[i].isReservedWord && (strcmp(UCLexeme,TOKENTABLE[i].description) == 0) )
				isFound = true;
			else
				i++;
		}
		if ( isFound )
			type = TOKENTABLE[i].type;
		else // Not a reserved word, must be an <identifier>.
			type = IDENTIFIER;
	}
	// <integer>
	else if ( isdigit(nextCharacter) )
	{
		i = 0;
		lexeme[i++] = nextCharacter;
		nextCharacter = reader.GetNextCharacter().character;
		while ( isdigit(nextCharacter) )
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
		switch ( nextCharacter )
		{
			// <string>
			case '"': 
				i = 0;
				nextCharacter = reader.GetNextCharacter().character;
				while ( (nextCharacter != '"') && (nextCharacter != READER<CALLBACKSUSED>::EOLC) )
				{
					if      ( (nextCharacter == '\\') && (reader.GetLookAheadCharacter(1).character == '"') )
					{
						lexeme[i++] = nextCharacter;
						nextCharacter = reader.GetNextCharacter().character;
					}
					else if ( (nextCharacter == '\\') && (reader.GetLookAheadCharacter(1).character == '\\') )
					{
						lexeme[i++] = nextCharacter;
						nextCharacter = reader.GetNextCharacter().character;
					}	
					lexeme[i++] = nextCharacter;
					nextCharacter = reader.GetNextCharacter().character;
				}
				if ( nextCharacter == READER<CALLBACKSUSED>::EOLC )
					ProcessCompilerError(sourceLineNumber,sourceLineIndex,
                                    "Invalid string literal");
				lexeme[i] = '\0';
				type = STRING;
				reader.GetNextCharacter();
				break;
			case '\'': 
				i = 0;
				nextCharacter = reader.GetNextCharacter().character;
				while ( (nextCharacter != '\'') && (nextCharacter != READER<CALLBACKSUSED>::EOLC) )
				{
					if      ( (nextCharacter == '\\') && (reader.GetLookAheadCharacter(1).character == '\'') )
					{
						lexeme[i++] = nextCharacter;
						nextCharacter = reader.GetNextCharacter().character;
					}
					else if ( (nextCharacter == '\\') && (reader.GetLookAheadCharacter(1).character == '\\') )
					{
						lexeme[i++] = nextCharacter;
						nextCharacter = reader.GetNextCharacter().character;
					}	
					lexeme[i++] = nextCharacter;
					nextCharacter = reader.GetNextCharacter().character;
				}
				if ( nextCharacter == READER<CALLBACKSUSED>::EOLC )
					ProcessCompilerError(sourceLineNumber,sourceLineIndex,
                                    "Invalid string literal");
				lexeme[i] = '\0';
				type = STRING;
				reader.GetNextCharacter();
				break;
			case READER<CALLBACKSUSED>::EOPC: 
				static int count = 0;
   
				if ( ++count > (LOOKAHEAD+1) )
					ProcessCompilerError(sourceLineNumber,sourceLineIndex,
                                       "Unexpected end-of-program");
				else
				{
					type = EOPTOKEN;
					reader.GetNextCharacter();
					lexeme[0] = '\0';
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
				lexeme[0] = nextCharacter;
				nextCharacter = reader.GetNextCharacter().character;
				if ( nextCharacter == '=' )
				{
					type = COLONEQ;
					lexeme[1] = nextCharacter; lexeme[2] = '\0';
					reader.GetNextCharacter();
				}
				else
				{
					type = COLON;
					lexeme[1] = '\0';
				}
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
			case '<': 
				lexeme[0] = nextCharacter;
				nextCharacter = reader.GetNextCharacter().character;
				if ( nextCharacter == '=' )
				{
					type = LTEQ;
					lexeme[1] = nextCharacter; lexeme[2] = '\0';
					reader.GetNextCharacter();
				}
				else if ( nextCharacter == '>' )
				{
					type = NOTEQ;
					lexeme[1] = nextCharacter; lexeme[2] = '\0';
					reader.GetNextCharacter();
				}
				else
				{
					type = LT;
					lexeme[1] = '\0';
				}
				break;
			case '=':  
				lexeme[0] = nextCharacter;
				nextCharacter = reader.GetNextCharacter().character;
				if ( nextCharacter == '=' ) // operator ==
				{
					type = EQ;
					lexeme[1] = nextCharacter; lexeme[2] = '\0';
					reader.GetNextCharacter();
				}
				else
				{
					type = EQUAL;	// assignment 	ex: x = 1
					lexeme[1] = '\0';
				}
				break;
			case '>':
				lexeme[0] = nextCharacter;
				nextCharacter = reader.GetNextCharacter().character;
				if ( nextCharacter == '=' )
				{
					type = GTEQ;
					lexeme[1] = nextCharacter; lexeme[2] = '\0';
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
				if ( reader.GetLookAheadCharacter(1).character == '=' )
				{
					nextCharacter = reader.GetNextCharacter().character;
					lexeme[1] = nextCharacter; lexeme[2] = '\0';
					reader.GetNextCharacter();
					type = NOTEQ;
				}
				else
				{
					type = UNKTOKEN;
					lexeme[1] = '\0';
					reader.GetNextCharacter();
				}
				break;
			case '+': 
				lexeme[0] = nextCharacter;
				if ( reader.GetLookAheadCharacter(1).character == '+' )
				{
				   nextCharacter = reader.GetNextCharacter().character;
				   lexeme[1] = nextCharacter; lexeme[2] = '\0';
				   type = INC;
				}
				else
				{
					type = PLUS;
					lexeme[0] = nextCharacter; lexeme[1] = '\0';
				}
				reader.GetNextCharacter();
				break;
			case '-':	   
				lexeme[0] = nextCharacter;
				if ( reader.GetLookAheadCharacter(1).character == '-' )
				{
				   nextCharacter = reader.GetNextCharacter().character;
				   lexeme[1] = nextCharacter; lexeme[2] = '\0';
				   type = DEC;
				}
				else
				{
					type = MINUS;
					lexeme[0] = nextCharacter; lexeme[1] = '\0';
				}
				reader.GetNextCharacter();
				break;
			// use character look-ahead to "find" other '*'
			case '*': 
				lexeme[0] = nextCharacter;
				if ( reader.GetLookAheadCharacter(1).character == '*' )
				{
					nextCharacter = reader.GetNextCharacter().character;
					lexeme[1] = nextCharacter; lexeme[2] = '\0';
					type = POWER;
				}
				else
				{
					type = MULTIPLY;
					lexeme[0] = nextCharacter; lexeme[1] = '\0';
				}
				reader.GetNextCharacter();
				break;
			case '/': 
				type = DIVIDE;
				lexeme[0] = nextCharacter; lexeme[1] = '\0';
				reader.GetNextCharacter();
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
			default:  
				type = UNKTOKEN;
				lexeme[0] = nextCharacter; lexeme[1] = '\0';
				reader.GetNextCharacter();
				break;
		}
	}

	tokens[LOOKAHEAD].type = type;
	strcpy(tokens[LOOKAHEAD].lexeme,lexeme);
	tokens[LOOKAHEAD].sourceLineNumber = sourceLineNumber;
	tokens[LOOKAHEAD].sourceLineIndex = sourceLineIndex;

// Displaying trace information and return
#ifdef TRACESCANNER
	sprintf(information,"At (%4d:%3d) token = %12s lexeme = |%s|",
		tokens[LOOKAHEAD].sourceLineNumber,
		tokens[LOOKAHEAD].sourceLineIndex,
		TokenDescription(type),lexeme);
	lister.ListInformationLine(information);
#endif

}

const char *TokenDescription(TOKENTYPE type)
{
	int i;
	bool isFound;
   
	isFound = false;
	i = 0;
	while ( !isFound && (i <= (sizeof(TOKENTABLE)/sizeof(TOKENTABLERECORD))-1) )
	{
		if ( TOKENTABLE[i].type == type )
			isFound = true;
		else
			i++;
	}
	return ( isFound ? TOKENTABLE[i].description : "???????" );
}
