// PotatoParcer.cpp by Auroxsus
// French Fry Productions
// Description: POTATO Parser program
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
#define TRACESCANNER
#define TRACEPARSER

#include "Potato.h"

typedef enum
{
	// pseudo-terminals
	IDENTIFIER,
	STRING,
	EOPTOKEN,
	UNKTOKEN,
	// reserved words
	MAINPOTATO,
	COOKEDPOTATO,
	OUTPOTATO,
	ENDL,
	// punctuation
	COMMA,
	PERIOD,
	// operators
	// ***NONE***
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
	{ STRING      ,"STRING"      ,false },
	{ EOPTOKEN    ,"EOPTOKEN"    ,false },
	{ UNKTOKEN    ,"UNKTOKEN"    ,false },
	{ MAINPOTATO  ,"MAINPOTATO"  ,true  },
	{ COOKEDPOTATO,"COOKEDPOTATO",true  },
	{ OUTPOTATO   ,"OUTPOTATO"   ,true  },
	{ ENDL        ,"ENDL"        ,true  },
	{ COMMA       ,"COMMA"       ,false },
	{ PERIOD      ,"PERIOD"      ,false }
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
	void GetNextToken(TOKEN tokens[]);

	char sourceFileName[80+1];
	TOKEN tokens[LOOKAHEAD+1];
   
	cout << "Source filename? ";
	cin >> sourceFileName;

	try
	{
		lister.OpenFile(sourceFileName);
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
	}
	catch (POTATOEXCEPTION POTATOException)
	{
		cout << "POTATO exception: " << POTATOException.GetDescription() << endl;
	}
	lister.ListInformationLine("******* POTATO parser ending");
	cout << "POTATO parser ending\n";

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

	EnterModule("PROGRAMDefinition");

	GetNextToken(tokens);

	while ( tokens[0].type != COOKEDPOTATO )
		ParseStatement(tokens);

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
	void GetNextToken(TOKEN tokens[]);

	EnterModule("PRINTStatement");

	do
	{
		// Necessary to discard the token-and-lexeme
		GetNextToken(tokens);

		switch ( tokens[0].type )
		{
			case STRING:
				GetNextToken(tokens);
				break;
			case ENDL:
				GetNextToken(tokens);
				break;
			default:
				ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,
                                 "Expecting string or ENDL");
		}
	} while ( tokens[0].type == COMMA );

	if ( tokens[0].type != PERIOD )
		ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,
                           "Expecting '.'");

	GetNextToken(tokens);

	ExitModule("PRINTStatement");
}

void Callback1(int sourceLineNumber,const char sourceLine[])
{
	cout << setw(4) << sourceLineNumber << " ";
}

void Callback2(int sourceLineNumber,const char sourceLine[])
{
	cout << sourceLine << endl;
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
