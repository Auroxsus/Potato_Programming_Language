// PotatoCompiler.cpp by Auroxsus
// French Fry Productions
// Description: POTATO Compiler program Version 2
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
	MAINPOTATO,	
	COOKEDPOTATO,
	OUTPOTATO,
	ENDL,
	OR,
	NOR,
	XOR,
	AND,
	NAND,
	NOT,
	TRUE,
	FALSE,
	// punctuation
	COMMA,
	PERIOD,
    OPENPARENTHESIS,
    CLOSEPARENTHESIS,
	// operators
	LT,
	LTEQ,
	EQ,
	GT,
	GTEQ,
	NOTEQ, // <> and !=
	PLUS,
	MINUS,
	MULTIPLY,
	DIVIDE,
	MODULUS,
	ABS,
	POWER  // ^ and **
} TOKENTYPE;

struct TOKENTABLERECORD
{
	TOKENTYPE type;
	char description[12+1];
	bool isReservedWord;
};

const TOKENTABLERECORD TOKENTABLE[] =
{
	{ IDENTIFIER  ,"IDENTIFIER"  ,false },
	{ INTEGER     ,"INTEGER"     ,false },
	{ STRING      ,"STRING"      ,false },
	{ EOPTOKEN    ,"EOPTOKEN"    ,false },
	{ UNKTOKEN    ,"UNKTOKEN"    ,false },
	{ MAINPOTATO  ,"MAINPOTATO"  ,true  },
	{ COOKEDPOTATO,"COOKEDPOTATO",true  },
	{ OUTPOTATO   ,"OUTPOTATO"   ,true  },
	{ ENDL        ,"ENDL"        ,true  },
    { OR          ,"OR"          ,true  },
    { NOR         ,"NOR"         ,true  },
    { XOR         ,"XOR"         ,true  },
    { AND         ,"AND"         ,true  },
    { NAND        ,"NAND"        ,true  },
    { NOT         ,"NOT"         ,true  },
    { TRUE        ,"TRUE"        ,true  },
    { FALSE       ,"FALSE"       ,true  },
	{ COMMA       ,"COMMA"       ,false },
	{ PERIOD      ,"PERIOD"      ,false },
	{ OPENPARENTHESIS	,"OPENPARENTHESIS"	,false },
	{ CLOSEPARENTHESIS	,"CLOSEPARENTHESIS"	,false },
	{ LT          ,"LT"          ,false },
	{ LTEQ        ,"LTEQ"        ,false },
	{ EQ          ,"EQ"          ,false },
	{ GT          ,"GT"          ,false },
	{ GTEQ        ,"GTEQ"        ,false },
	{ NOTEQ       ,"NOTEQ"       ,false },
	{ PLUS        ,"PLUS"        ,false },
	{ MINUS       ,"MINUS"       ,false },
	{ MULTIPLY    ,"MULTIPLY"    ,false },
	{ DIVIDE      ,"DIVIDE"      ,false },
	{ MODULUS     ,"MODULUS"     ,false },
	{ ABS         ,"ABS"         ,true  },
	{ POWER       ,"POWER"       ,false }
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
	lister.ListInformationLine("POTATO compiler ending with compiler error!\n");
	throw( POTATOEXCEPTION("POTATO compiler ending with compiler error!") );
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
   
	cout << "Source filename? ";
	cin >> sourceFileName;

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
	lister.ListInformationLine("******* POTATO parser ending");
	cout << "POTATO compiler ending\n";

	system("PAUSE");
	return( 0 );
   
}

void ParsePOTATOProgram(TOKEN tokens[])
{
	void GetNextToken(TOKEN tokens[]);
	void ParsePROGRAMDefinition(TOKEN tokens[]);

	EnterModule("POTATOProgram");

	if ( tokens[0].type == MAINPOTATO )
		ParsePROGRAMDefinition(tokens);
	else
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,
                           "Expecting MAINPOTATO");
	// Makes sure last token is EOPTOKEN
	if ( tokens[0].type != EOPTOKEN )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,
                           "Expecting end-of-program");

	ExitModule("POTATOProgram");
}

void ParsePROGRAMDefinition(TOKEN tokens[])
{
	void GetNextToken(TOKEN tokens[]);
	void ParseStatement(TOKEN tokens[]);

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
	while ( tokens[0].type != COOKEDPOTATO )
		ParseStatement(tokens);

// CODEGENERATION
	code.EmitFormattedLine("","RETURN");
	code.EmitUnformattedLine("; **** =========");
	sprintf(line,"; **** END (%4d)",tokens[0].sourceLineNumber);
	code.EmitUnformattedLine(line);
	code.EmitUnformattedLine("; **** =========");
// ENDCODEGENERATION

	GetNextToken(tokens);
	ExitModule("PROGRAMDefinition");
}

void ParseStatement(TOKEN tokens[])
{
	void GetNextToken(TOKEN tokens[]);
	void ParsePRINTStatement(TOKEN tokens[]);

	EnterModule("Statement");

	switch ( tokens[0].type )
	{
		case OUTPOTATO:
			ParsePRINTStatement(tokens);
			break;
		default:
			ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,
                              "Expecting beginning-of-statement");
			break;
	}

	ExitModule("Statement");
}

void ParsePRINTStatement(TOKEN tokens[])
{
	void ParseExpression(TOKEN tokens[],DATATYPE &datatype);												   
	void GetNextToken(TOKEN tokens[]);

	char line[SOURCELINELENGTH+1];		
	DATATYPE datatype;					 				 
	EnterModule("PRINTStatement");
	
// CODEGENERATION
	sprintf(line,"; **** PRINT statement (%4d)",tokens[0].sourceLineNumber);
	code.EmitUnformattedLine(line);
// ENDCODEGENERATION

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

void ParseExpression(TOKEN tokens[],DATATYPE &datatype)
{
// CODEGENERATION
// ENDCODEGENERATION

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
/*
      CMPI
      JMPXX     T????         ; XX = L,E,G,LE,NE,GE (as required)
      PUSH      #0X0000       ; push FALSE
      JMP       E????         ;    or 
T???? PUSH      #0XFFFF       ; push TRUE (as required)
E???? EQU       *
*/
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
         (tokens[0].type == MINUS)	)
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
/*
      SETNZPI
      JMPNN     E????
      NEGI                    ; NEGI or NEGF (as required)
E???? EQU       *
*/
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
	void ParsePrimary(TOKEN tokens[],DATATYPE &datatype);
	void GetNextToken(TOKEN tokens[]);

	DATATYPE datatypeLHS,datatypeRHS;

	EnterModule("Secondary");

	ParsePrimary(tokens,datatypeLHS);

	if ( tokens[0].type == POWER )
	{
		GetNextToken(tokens);

		ParsePrimary(tokens,datatypeRHS);

		if ( (datatypeLHS != INTEGERTYPE) || (datatypeRHS != INTEGERTYPE) )
			ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting integer operands");

		code.EmitFormattedLine("","POWI");
		datatype = INTEGERTYPE;
	}
	else
		datatype = datatypeLHS;

	ExitModule("Secondary");
}

void ParsePrimary(TOKEN tokens[],DATATYPE &datatype)
{
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
				ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting )");
			GetNextToken(tokens);
			break;
		default:
			ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting integer, true, false, or (");
			break;
	}

	ExitModule("Primary");
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
			case '(': 
				type = OPENPARENTHESIS;
				lexeme[0] = nextCharacter; lexeme[1] = '\0';
				reader.GetNextCharacter();
				break;
			case ')': 
				type = CLOSEPARENTHESIS;
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
				type = EQ;
				lexeme[0] = nextCharacter; lexeme[1] = '\0';
				reader.GetNextCharacter();
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
				type = PLUS;
				lexeme[0] = nextCharacter; lexeme[1] = '\0';
				reader.GetNextCharacter();
				break;
			case '-': 
				type = MINUS;
				lexeme[0] = nextCharacter; lexeme[1] = '\0';
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
