//-----------------------------------------------------------
// PotatoChip.cpp by Auroxsus
// French Fry Productions
// Description: POTATO Scanner program
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
#include "PotatoChip.h"


typedef enum
{
// pseudo-terminals
    IDENTIFIER,
    STRING,
    EOPTOKEN,
    UNKTOKEN,
	REFERENCE,
// reserved words
    CRISP,
    PRINT,
    ENDL,
// punctuation
    COMMA,
    OCBRACKET,
    CCBRACKET,
    OPARENTHESIS,
    CPARENTHESIS
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
   { IDENTIFIER ,"IDENTIFIER"  ,false },
   { STRING     ,"STRING"      ,false },
   { EOPTOKEN   ,"EOPTOKEN"    ,false },
   { UNKTOKEN   ,"UNKTOKEN"    ,false },
   { CRISP      ,"CRISP"       ,true  },
   { REFERENCE  ,"REFERENCE"   ,false },
   { PRINT      ,"BITE"        ,true  },
   { ENDL       ,"ENDL"        ,true  },
   { COMMA      ,"COMMA"       ,false },
   { OCBRACKET  ,"OCBRACKET"   ,false },
   { CCBRACKET  ,"CCBRACKET"   ,false },
   { OPARENTHESIS,"OPARENTHESIS",false },
   { CPARENTHESIS,"CPARENTHESIS",false },
};

struct TOKEN
{
	TOKENTYPE type;
	char lexeme[SOURCELINELENGTH+1];
    int sourceLineNumber;
    int sourceLineIndex;
};

// Global variables
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
	lister.ListInformationLine("******* POTATO was cooked thoroughly");
	cout << "POTATO was cooked thoroughly\n";

	system("PAUSE");
	return( 0 );
}

void ParsePOTATOProgram(TOKEN tokens[])
{
   void GetNextToken(TOKEN tokens[]);
   void ParseCRISPDefinition(TOKEN tokens[]);

   EnterModule("POTATOProgram");


#ifdef TRACECOMPILER
   identifierTable.DisplayTableContents("Contents of identifier table after compilation of global data definitions");
#endif

   if ( tokens[0].type == CRISP )
      ParseCRISPDefinition(tokens);
   else
      ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,
                           "Expecting CRISP");

   if ( tokens[0].type != EOPTOKEN )
      ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,
                           "Expecting end-of-program");

   ExitModule("POTATOProgram");
}

void ParseCRISPDefinition(TOKEN tokens[])
{
    void GetNextToken(TOKEN tokens[]);
    void ParseStatement(TOKEN tokens[]);
    const char *TokenDescription(TOKENTYPE type);

    EnterModule("CRISPDefinition");

    GetNextToken(tokens);  // Move past 'crisp'

   // Check for identifier
    if (tokens[0].type != IDENTIFIER) {
        ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex,
                             "Expecting identifier for CRISP function definition");
        return; // Exit if opening bracket is missing
    }
    
    GetNextToken(tokens);  // Move past identifier
    
   if ( tokens[0].type != OPARENTHESIS )
      ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting '('");
   GetNextToken(tokens); // move past (
    
    if ( tokens[0].type != CPARENTHESIS )
      ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting ')'");
   GetNextToken(tokens); // move past )
   
    // Check for opening curly bracket
    if (tokens[0].type != OCBRACKET) {
        ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex,
                             "Expecting opening bracket for CRISP function definition");
        return; // Exit if opening bracket is missing
    }

    GetNextToken(tokens); // Move past the opening bracket
    
    // Handle empty CRISP block (check for immediate closing bracket)
    if (tokens[0].type == CCBRACKET) {
        GetNextToken(tokens); // Move past the closing bracket
    } else {
        // Process statements inside the CRISP block
        while (tokens[0].type != CCBRACKET) {
            if (tokens[0].type == EOPTOKEN) {
                ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex,
                                     "Expecting closing bracket for CRISP function definition");
                break; // Exit if end-of-program is reached
            }
//printf("going into parse statment: Processing token type: %s\n", tokens[0].lexeme); // Debug output

       
            ParseStatement(tokens);
            //printf("out of parse statement: Processing token type: %s\n", tokens[0].lexeme); // Debug output

       
            //GetNextToken(tokens); // Get the next token after processing the statement
            //printf("got next token: Processing token type: %s\n", tokens[0].lexeme); // Debug output

       
         }
        
        GetNextToken(tokens); // Move past the closing bracket
    }

    ExitModule("CRISPDefinition");
}


		   
void ParseStatement(TOKEN tokens[])
{
   void GetNextToken(TOKEN tokens[]);
   void ParsePRINTStatement(TOKEN tokens[]);

   EnterModule("Statement");

   switch ( tokens[0].type )
   {
      case PRINT:
      //printf("case print: Processing token type: %s\n", tokens[0].lexeme); // Debug output

       
         ParsePRINTStatement(tokens);
         break;
      default:
         ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,
                              "Expecting beginning-of-statement");
         break;
   }
  // printf("parse statement out of switch: Processing token type: %s\n", tokens[0].lexeme); // Debug output

       


   ExitModule("Statement");
}

void ParsePRINTStatement(TOKEN tokens[])
{
    void GetNextToken(TOKEN tokens[]);
    EnterModule("PRINTStatement");
	
    GetNextToken(tokens);

    do
    {
        //printf("parse print do: Processing token type: %s\n", tokens[0].lexeme); // Debug output
       //if (token[0].type == RawString)
        switch (tokens[0].type)
        {
            case STRING:
            //printf("print string: Processing token type: %s\n", tokens[0].lexeme); // Debug output

       
                GetNextToken(tokens); // Move to the next token
                break;

            case REFERENCE:
                GetNextToken(tokens); // Move to the next token
                break;
            case IDENTIFIER:
                GetNextToken(tokens); // Move to the next token
                break;


            case ENDL:
            //printf("print endl: Processing token type: %s\n", tokens[0].lexeme); // Debug output

       
                GetNextToken(tokens); // Move to the next token, if any
                break;

            default:
                ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex,
                                     "Expecting string or ENDL");
        }
        //else
        
        //printf("parse print while: Processing token type: %s\n", tokens[0].lexeme); // Debug output

       
    } while (tokens[0].type == STRING || tokens[0].type == REFERENCE|| tokens[0].type == IDENTIFIER || tokens[0].type == ENDL);
	//printf("parse print out while: Processing token type: %s\n", tokens[0].lexeme); // Debug output

       
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

// Define the helper function to handle escape sequences
char HandleEscapeSequences(char nextCharacter, int sourceLineNumber, int sourceLineIndex) {
    switch (nextCharacter) {
        case 'n': return '\n';
        case 't': return '\t';
        case 'b': return '\b';
        case 'r': return '\r';
        case '\\': return '\\';
        case '"': return '\"';
        default: 
            ProcessCompilerError(sourceLineNumber, sourceLineIndex, "Illegal escape character sequence in string literal");
            return '\0'; // Return null character to prevent use
    }
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

   //===========================================================
   // Move look-ahead "window" to make room for next token-and-lexeme
   //===========================================================
   for (int i = 1; i <= LOOKAHEAD; i++)
      tokens[i-1] = tokens[i];

   char nextCharacter = reader.GetLookAheadCharacter(0).character;

   //===========================================================
   // "Eat" white space and comments
   //===========================================================
   do
   {
	  //    "Eat" any white-space (blanks and EOLCs and TABCs) 
      while ( (nextCharacter == ' ')
           || (nextCharacter == READER<CALLBACKSUSED>::EOLC)
           || (nextCharacter == READER<CALLBACKSUSED>::TABC) )
         nextCharacter = reader.GetNextCharacter().character;

	  //  "Eat" line comment 
	  if ( (nextCharacter == '|') && (reader.GetLookAheadCharacter(1).character == '|') )
	  {
		  
// Display trace information
#ifdef TRACESCANNER
   sprintf(information,"At (%4d:%3d) begin line comment",
      reader.GetLookAheadCharacter(0).sourceLineNumber,
      reader.GetLookAheadCharacter(0).sourceLineIndex);
   lister.ListInformationLine(information);
#endif

         do
            nextCharacter = reader.GetNextCharacter().character;
         while ( (nextCharacter != READER<CALLBACKSUSED>::EOLC) 
              && (nextCharacter != READER<CALLBACKSUSED>::EOPC) );
      } 

	  //    "Eat" block comments (nesting allowed)
      if ( (nextCharacter == '|') && (reader.GetLookAheadCharacter(1).character == '[') )
      {
         int depth = 0;

         do
         {
            if ( (nextCharacter == '|') && (reader.GetLookAheadCharacter(1).character == '[' ) )
			{
               depth++;

// Display trace information
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

// Display trace information
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
         }
         while ( (depth != 0) && (nextCharacter != READER<CALLBACKSUSED>::EOPC) );
         if ( depth != 0 ) 
            ProcessCompilerError(reader.GetLookAheadCharacter(0).sourceLineNumber,
                                 reader.GetLookAheadCharacter(0).sourceLineIndex,
                                 "Depth: Unexpected end-of-program");
      }
	  
    /* WHILE ( (nextCharacter is not a white-space character)
       or (nextCharacter is not beginning-of-comment character) ) */
   } while ( (nextCharacter == ' ')
          || (nextCharacter == READER<CALLBACKSUSED>::EOLC)
          || (nextCharacter == READER<CALLBACKSUSED>::TABC)
          || (nextCharacter == ';')
          || ((nextCharacter == '|') && (reader.GetLookAheadCharacter(1).character == '['))   // allows block comments back to back
		  || ((nextCharacter == '|') && (reader.GetLookAheadCharacter(1).character == '|'))); // allows inline comments after block comment

//===========================================================
// Scan token
//===========================================================
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

      // Try to find the lexeme in the table of reserved words
      bool isFound = false;

      i = 0;
      while ( !isFound && (i <= (sizeof(TOKENTABLE)/sizeof(TOKENTABLERECORD))-1) )
      {
         if ( TOKENTABLE[i].isReservedWord && (strcmp(UCLexeme,TOKENTABLE[i].description) == 0) )
		{    
			isFound = true;
			type = TOKENTABLE[i].type;
		 }
         else
            i++;
      }
      if ( !isFound ) // Not a reserved word, must be an <identifier>.
         type = IDENTIFIER;
   }
   else
   {
   
	  // Determine both the type and the lexeme of the next token
      switch ( nextCharacter )
      {
      // <string> literal *Note* escape character sequences \n,\t,\b,\r,\\,\" supported
         case '"': // FORMATTED STRING
			 i = 0;
			 nextCharacter = reader.GetNextCharacter().character;
			 while ((nextCharacter != '"') &&
					(nextCharacter != READER<CALLBACKSUSED>::EOLC) &&
					(nextCharacter != READER<CALLBACKSUSED>::EOPC)) {
				 if (nextCharacter == '\\') {
					 nextCharacter = reader.GetNextCharacter().character; // Get the next character after the backslash
					 lexeme[i++] = HandleEscapeSequences(nextCharacter, sourceLineNumber, sourceLineIndex); // Handle escape sequences
				 } else {
					 lexeme[i++] = nextCharacter; // Store the regular character
				 }
				 nextCharacter = reader.GetNextCharacter().character; // Read the next character
			 }
			 if (nextCharacter != '"') 
				 ProcessCompilerError(sourceLineNumber, sourceLineIndex, "Un-terminated string literal");
			 
			 lexeme[i] = '\0';
			 type = STRING;

			 reader.GetNextCharacter(); // Consume the closing quote
			 break;

//===========================================================
//    ***EXAMPLE***
//    <string> (no \-escaped string delimiter ` allowed) ` embedded as ``
//    <string>              ::= `{<ASCIICharacter>}*`
//===========================================================
/* Implement raw string concatenation, similar to Python's behavior */        
		case '\'': // RAW STRING
    i = 0;
    nextCharacter = reader.GetNextCharacter().character;

    while ((nextCharacter != '\'') && 
           (nextCharacter != READER<CALLBACKSUSED>::EOLC) && 
           (nextCharacter != READER<CALLBACKSUSED>::EOPC))
    {
        // If we encounter another single quote, check for consecutive quotes
        if (nextCharacter == '\'')
        {
            // Check if it's followed by another single quote
            if (reader.GetLookAheadCharacter(1).character == '\'')
            {
                // Consume the first quote and skip the next one
                nextCharacter = reader.GetNextCharacter().character; // Consume the first quote
                nextCharacter = reader.GetNextCharacter().character; // Skip the second quote
            }
            else
            {
                // If it's not followed by another quote, break out of the loop
                break;
            }
        }
        // Allow double quotes to be treated as regular characters in a raw string
        else if (nextCharacter == '"')
        {
            lexeme[i++] = nextCharacter; // Store the double quote as a character
        }
        else
        {
            lexeme[i++] = nextCharacter; // Store any other character
        }

        nextCharacter = reader.GetNextCharacter().character; // Read the next character
    }

    // Check for un-terminated string literal
    if (nextCharacter == READER<CALLBACKSUSED>::EOLC || 
        nextCharacter == READER<CALLBACKSUSED>::EOPC)
    {
        ProcessCompilerError(sourceLineNumber, sourceLineIndex, "Un-terminated string literal");
    }

    lexeme[i] = '\0'; // Null-terminate the string
    type = STRING; // Set the type to STRING
    reader.GetNextCharacter(); // Consume the closing single quote
    break;

//===========================================================
			case READER<CALLBACKSUSED>::EOPC: 
            {
				static int count = 0;
                
                if (++count > (LOOKAHEAD + 1))
					ProcessCompilerError(sourceLineNumber,sourceLineIndex,
                                       "EOPC: Unexpected end-of-program");
				else
				{
					type = EOPTOKEN;
					reader.GetNextCharacter();
					lexeme[0] = '\0';
				}
            }
            
				break;
			case '%':
			    // Handle reference
                lexeme[0] = nextCharacter; // Store '%'
                lexeme[1] = '\0'; // Null-terminate
                nextCharacter = reader.GetNextCharacter().character; // Move to next character
                
				if (isalpha(nextCharacter)) {
                    lexeme[1] = nextCharacter; // Store shorthand
                    lexeme[2] = '\0'; // Null-terminate
                    
					// Check for valid shorthand type
                    if (strchr("isfc", nextCharacter)) { 		
					 // If nextCharacter is one of 'i', 's', 'f', or 'c'
                        type = REFERENCE; // Set type to REFERENCE
                    } else {
                        ProcessCompilerError(sourceLineNumber, sourceLineIndex, "Invalid shorthand data type for reference");
                    }
                
				} else {
                    ProcessCompilerError(sourceLineNumber, sourceLineIndex, "Expected shorthand after '%' reference");
                }
                break;
			case ',':
			    type = COMMA;
			    lexeme[0] = nextCharacter; lexeme[1] = '\0';
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
