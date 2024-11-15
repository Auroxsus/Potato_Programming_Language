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
#include <ctime> // Used in Potato.h for time stamp
#include <cctype>
#include <vector>

using namespace std;

// #define TRACEREADER
// #define TRACESCANNER
// #define TRACEPARSER
#define TRACEIDENTIFIERTABLE
#define TRACECOMPILER

#include "PotatoChip.h"

//-----------------------------------------------------------
typedef enum
//-----------------------------------------------------------
{
    // pseudo-terminals
    IDENTIFIER,
    EOPTOKEN,
    UNKTOKEN,
    // reserved words
    CRISP,
    PROGRAM,   // mainCrisp
    PRINT,     // bite
    ENDL,      // endl and \n
    // logical operators
    OR,        // \\ and mix
    NOR,       // ~\ and spudStop
    XOR,       // +\ and eitherSpud
    AND,       // && and prep
    NAND,      // ~& and mashless
    NOT,       // ! and raw
    ABS,
    TRUE,
    FALSE,
    INTEGER,   // SPUD
    STRING,    // CHIPPER
    BOOLEAN,   // TATER
    CONSTANT,  // ROOT
    INPUT,     // UNEARTH
    // punctuation
    COMMA,
    PERIOD,
    COLON,
    SEMICOLON,
    OCBRACKET,
    CCBRACKET,
    OSQRBRACKET,
    CSQRBRACKET,
    OPARENTHESIS,
    CPARENTHESIS,
    CONCAT,
    // relational operators
    LT,       // < and smallerSpud
    LTEQ,     // <= and notBiggerSpud
    EQEQ,      // == and spudMatch
    GT,       // > and biggerSpud
    GTEQ,     // >= and notSmallerSpud
    NOTEQ,    // <>, !=, and mashApart
    // assignment opperators
    EQ,      // = and plant
    // arithmetic operators
    PLUS,     // + and mash
    MINUS,    // - and peel
    MULTIPLY, // * and fry
    DIVIDE,   // / and slice
    MODULUS,  // % and mod
    POWER,     // ^ and pow
    INC,
    DEC
} TOKENTYPE;

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
    { INTEGER,        "SPUDLING",          true  },
    { STRING,         "CHIPPER",           true  },
    { BOOLEAN,        "TATER",             true  },
    { CONSTANT,       "SPUD",              true  },
    { INPUT,          "UNEARTH",           true  },
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
READER<CALLBACKSUSED> reader (SOURCELINELENGTH, LOOKAHEAD);
LISTER lister (LINESPERPAGE);
// CODEGENERATION
CODE code;
IDENTIFIERTABLE identifierTable(&lister,MAXIMUMIDENTIFIERS);
// ENDCODEGENERATION

#ifdef TRACEPARSER
int level;
#endif

//--------------------------------------------------
// Function to get token description from TOKENTABLE
//--------------------------------------------------
const char *GetTokenDescription (TOKENTYPE type)
{
    for (int i = 0; i < (sizeof (TOKENTABLE) / sizeof (TOKENTABLERECORD)); ++i)
        { // for (const auto& record : TOKENTABLE) {}
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
void DebugPrintToken (const TOKEN &token, const char *location)
{
    const char *description = GetTokenDescription (token.type);

    // Check if the token is a reserved word
    bool isReserved = false;
    for (int i = 0; i < sizeof (TOKENTABLE) / sizeof (TOKENTABLERECORD); ++i)
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
void EnterModule (const char module[])
//-----------------------------------------------------------
{
#ifdef TRACEPARSER
    char information[SOURCELINELENGTH + 1];

    level++;
    sprintf (information, "   %*s>%s", level * 2, " ", module);
    lister.ListInformationLine (information);
#endif
}

//-----------------------------------------------------------
void ExitModule (const char module[])
//-----------------------------------------------------------
{
#ifdef TRACEPARSER
    char information[SOURCELINELENGTH + 1];

    sprintf (information, "   %*s<%s", level * 2, " ", module);
    lister.ListInformationLine (information);
    level--;
#endif
}

//--------------------------------------------------
void ProcessCompilerError (int sourceLineNumber, int sourceLineIndex, const char errorMessage[])
//--------------------------------------------------
{
    char information[SOURCELINELENGTH + 1];

    // Use "panic mode" error recovery technique: report error message and terminate compilation!
    sprintf (information, "     At (%4d:%3d) %s", sourceLineNumber, sourceLineIndex, errorMessage);
    lister.ListInformationLine (information);
    lister.ListInformationLine ("POTATO compiler ending with a potato famine!\n");
    throw (POTATOEXCEPTION ("POTATO compiler ending with a potato famine!"));
}

//-----------------------------------------------------------
int main ()
//-----------------------------------------------------------
{
    void Callback1 (int sourceLineNumber, const char sourceLine[]);
    void Callback2 (int sourceLineNumber, const char sourceLine[]);
    void ParsePOTATOProgram (TOKEN tokens[]);
    void GetNextToken (TOKEN tokens[]);

    char sourceFileName[80 + 1];
    TOKEN tokens[LOOKAHEAD + 1];

    cout << "Source filename? "; cin >> sourceFileName;

    try
    {
        lister.OpenFile (sourceFileName);
        code.OpenFile (sourceFileName);
        
        // CODEGENERATION
        code.EmitBeginningCode (sourceFileName);
        // ENDCODEGENERATION

        reader.SetLister (&lister);
        reader.AddCallbackFunction (Callback1);
        reader.AddCallbackFunction (Callback2);
        reader.OpenFile (sourceFileName);

        // Fill tokens[] for look-ahead
        for (int i = 0; i <= LOOKAHEAD; i++)
            GetNextToken (tokens);

#ifdef TRACEPARSER
        level = 0;
#endif

        ParsePOTATOProgram (tokens);

        // CODEGENERATION
        code.EmitEndingCode ();
        // ENDCODEGENERATION
            
    }
    catch (POTATOEXCEPTION potatoException)
    {
        cout << "POTATO exception: " << potatoException.GetDescription () << endl;
    }
    lister.ListInformationLine ("******* POTATO was cooked thoroughly");
    cout << "POTATO was cooked thoroughly\n";

    system ("PAUSE");
    return (0);
}

//-----------------------------------------------------------
void ParsePOTATOProgram (TOKEN tokens[])
//-----------------------------------------------------------
{
       void ParseDataDefinitions(TOKEN tokens[],IDENTIFIERSCOPE identifierScope);
    void ParseMAINDefinition (TOKEN tokens[]);
    void GetNextToken (TOKEN tokens[]);

    EnterModule ("POTATOProgram");
        // DebugPrintToken(tokens[0], "ParsePOTATOProgram - entry");

   ParseDataDefinitions(tokens,GLOBALSCOPE);
       // DebugPrintToken(tokens[0], "ParsePOTATOProgram - after ParseDataDefinitions");

   #ifdef TRACECOMPILER
   identifierTable.DisplayTableContents("Contents of identifier table after compilation of global data definitions");
#endif

    if (tokens[0].type != CRISP) {
        ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting `crisp` keyword");
        return;
    }
    GetNextToken(tokens); // Move past `crisp`

    if (tokens[0].type == PROGRAM)
            ParseMAINDefinition (tokens);
    else
        ProcessCompilerError (tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, 
                              "Expecting MAIN");
                              
                                  // DebugPrintToken(tokens[0], "ParsePOTATOProgram - after ParseMAINDefinition");


    if (tokens[0].type != EOPTOKEN)
        ProcessCompilerError (tokens[0].sourceLineNumber, tokens[0].sourceLineIndex,
                              "Expecting end-of-program");

    ExitModule ("POTATOProgram");
        // DebugPrintToken(tokens[0], "ParsePOTATOProgram - exit");

}

//-----------------------------------------------------------
// Function to parse data definitions, including better handling for constants
//-----------------------------------------------------------
void ParseDataDefinitions(TOKEN tokens[], IDENTIFIERSCOPE identifierScope)
{
    void GetNextToken(TOKEN tokens[]);
    
    char identifier[MAXIMUMLENGTHIDENTIFIER + 1];  // identifier being parsed
    char reference[MAXIMUMLENGTHIDENTIFIER + 1];   // Reference for memory location
    char literal[MAXIMUMLENGTHIDENTIFIER + 1];     // Stores literal value of a constant
    DATATYPE dataType;
    bool isConstant = false;                       // Flag for constants
    bool isInTable;                                // Flag for identifier exists in table
    bool isInGlobalTable;                          // Flag for in global table
    int index;

    EnterModule("DataDefinitions");

    while ((tokens[0].type == CONSTANT) || (tokens[0].type == INTEGER) || (tokens[0].type == BOOLEAN)) 
    {

        // Check if constant
        if (tokens[0].type == CONSTANT) {
            isConstant = true;
            //DebugPrintToken(tokens[0], "found constant");
            GetNextToken(tokens); // Move to the datatype (spud/tater)
        }

        // Determine datatype
        switch (tokens[0].type) {
            case INTEGER:  // 'spud'
                dataType = INTTYPE;
                break;
            case BOOLEAN:  // 'tater'
                dataType = BOOLTYPE;
                break;
            default:
                ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting a valid data type (spud, tater)");
        }
        
        //DebugPrintToken(tokens[0], "found datatype");
        GetNextToken(tokens); // Move to the identifier after datatype

        // Parse identifiers (e.g., `spud x1: x2`)
        do {
            if (tokens[0].type != IDENTIFIER) {
                ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting identifier");
            }
            strcpy(identifier, tokens[0].lexeme);
            
            //DebugPrintToken(tokens[0], "found identifier");
            GetNextToken(tokens); // Move to the next token to check for '=' or ';'

            // Expect '=' for constant assignment
            if (isConstant) {
                if (tokens[0].type != EQ) {
                    ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting '=' for constant assignment");
                }
            }
            
            
            // but if it has an '=' (regardless of constant) assign it a value
            if (tokens[0].type == EQ) {
                
                //DebugPrintToken(tokens[0], "found '='");
                GetNextToken(tokens); // Move to the value after '='

                // Assign value
                if ((dataType == INTTYPE && tokens[0].type == INTEGER)) {
                    strcpy(literal, "0D");
                    strcat(literal, tokens[0].lexeme);
                } else if ((dataType == BOOLTYPE) && (tokens[0].type == TRUE || tokens[0].type == FALSE)) {
                    strcpy(literal, tokens[0].lexeme);
                } else {
                    ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Data type mismatch for constant value");
                }
                
                //DebugPrintToken(tokens[0], "assigned value");
                
                GetNextToken(tokens); // Move to the value after '='
            }

            // Check if the identifier is already in the local or global table
            index = identifierTable.GetIndex(identifier, isInTable);

            // If it is already in the table, we need to verify its scope
            if (isInTable) {
                if (identifierTable.IsInCurrentScope(index)) {
                    // Identifier is already in the current local scope
                    ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Multiply-defined identifier in the current scope");
                } 
                else if (identifierScope == PROGRAMMODULESCOPE && identifierTable.GetScope(index) == GLOBALSCOPE) {
                    // Identifier exists in the global scope, and we are in the local scope (module scope)
                    ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Local variable name conflicts with a global variable");
                }
            }

            // Add the identifier to the table
            if (!isConstant) {
                // Determine scope and add to table
                switch (identifierScope) {
                    case GLOBALSCOPE:
                        code.AddRWToStaticData(1, identifier, reference);
                        identifierTable.AddToTable(identifier, GLOBAL_VARIABLE, dataType, reference);
                        break;
                    case PROGRAMMODULESCOPE:
                        code.AddRWToStaticData(1, identifier, reference);
                        identifierTable.AddToTable(identifier, PROGRAMMODULE_VARIABLE, dataType, reference);
                        break;
                }
            } else {

                // Add constant to identifier table
                switch (identifierScope) {
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
            if (tokens[0].type == COLON) {
                
                //DebugPrintToken(tokens[0], "found colon");
                GetNextToken(tokens); // Move to the next identifier after ':'
                if (tokens[0].type != IDENTIFIER) {
                    ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting identifier after ':'");
                }
            } else {
                break; // No more identifiers, exit the loop
            }

        } while (TRUE); // Continue parsing 

        if (tokens[0].type != SEMICOLON) {
            ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Data Expecting ';' at the end of declaration");
        }
        
        //DebugPrintToken(tokens[0], "found semicolon");
        GetNextToken(tokens); // Move to the next token after the semicolon

        // Reset
        isConstant = false;
    }

    ExitModule("DataDefinitions");
}

//-----------------------------------------------------------
void ParseMAINDefinition (TOKEN tokens[])
//-----------------------------------------------------------
{   
    void ParseDataDefinitions(TOKEN tokens[],IDENTIFIERSCOPE identifierScope);
    void ParseStatement (TOKEN tokens[]);
    void GetNextToken (TOKEN tokens[]);

    char line[SOURCELINELENGTH + 1];
    char label[SOURCELINELENGTH + 1];
    char reference[SOURCELINELENGTH + 1];

    EnterModule ("PROGRAMDefinition");
        // DebugPrintToken(tokens[0], "ParseMAINDefinition - entry");

    
    // CODEGENERATION
    code.EmitUnformattedLine ("; **** =========");
    sprintf (line, "; **** PROGRAM module (%4d)", tokens[0].sourceLineNumber);
    code.EmitUnformattedLine (line);
    code.EmitUnformattedLine ("; **** =========");
    code.EmitFormattedLine ("PROGRAMMAIN", "EQU", "*");

    // Initialize stack and heap
    code.EmitFormattedLine ("", "PUSH", "#RUNTIMESTACK", "set SP");
    code.EmitFormattedLine ("", "POPSP");
    code.EmitFormattedLine ("", "PUSHA", "STATICDATA", "set SB");
    code.EmitFormattedLine ("", "POPSB");
    code.EmitFormattedLine ("", "PUSH", "#HEAPBASE", "initialize heap");
    code.EmitFormattedLine ("", "PUSH", "#HEAPSIZE");
    code.EmitFormattedLine ("", "SVC", "#SVC_INITIALIZE_HEAP");
    
    // Call empty body or return immediately
    sprintf (label, "PROGRAMBODY%04d", code.LabelSuffix ());
    code.EmitFormattedLine ("", "CALL", label);
    code.AddDSToStaticData ("Normal program termination", "", reference);
    code.EmitFormattedLine ("", "PUSHA", reference);
    code.EmitFormattedLine ("", "SVC", "#SVC_WRITE_STRING");
    code.EmitFormattedLine ("", "SVC", "#SVC_WRITE_ENDL");
    
    // End program
    code.EmitFormattedLine ("", "PUSH", "#0D0", "terminate with status = 0");
    code.EmitFormattedLine ("", "SVC", "#SVC_TERMINATE");
    code.EmitFormattedLine (label, "EQU", "*");
    // ENDCODEGENERATION

    // DebugPrintToken(tokens[0], "ParseMAINCRISPDefinition");
    GetNextToken (tokens); // Move past 'crispMAIN'
    
    identifierTable.EnterNestedStaticScope();
   ParseDataDefinitions(tokens,PROGRAMMODULESCOPE);

    // Check for empty parameter list `()`
    if (tokens[0].type != OPARENTHESIS) {
        ProcessCompilerError (tokens[0].sourceLineNumber,
                              tokens[0].sourceLineIndex, "Expecting '('");
    }
    // DebugPrintToken(tokens[0], "ParseMAINCRISPDefinition \"OPARENTHESIS\" exit");
    GetNextToken (tokens); // move past `(`

    if (tokens[0].type != CPARENTHESIS){
        ProcessCompilerError (tokens[0].sourceLineNumber,
                              tokens[0].sourceLineIndex, "Expecting ')'");
    }
    // DebugPrintToken(tokens[0], "ParseMAINCRISPDefinition \"CPARENTHESIS\" exit");
    GetNextToken (tokens); // move past `)`
    
    // Check for opening curly bracket `{` 
    if (tokens[0].type != OCBRACKET)
        {
            ProcessCompilerError (
                tokens[0].sourceLineNumber, tokens[0].sourceLineIndex,
                "Expecting opening bracket for MAINCRISP function definition");
            return; // Exit if opening bracket is missing
        }
    // DebugPrintToken(tokens[0], "ParseMAINCRISPDefinition \"OCBRACKET\" exit");
    GetNextToken (tokens); // Move past the opening bracket `{` 

    // Handle ** E M P T Y ** MAINCRISP block by check for closing bracket `}`
    if (tokens[0].type == CCBRACKET)
        {
            GetNextToken (tokens); // Move past the closing bracket `}`
        }
    else
        {
            // Process statements inside the MAINCRISP block
            while (tokens[0].type != CCBRACKET && tokens[0].type != EOPTOKEN)
                {
                    ParseStatement (tokens);
                    if (tokens[0].type == EOPTOKEN)
                        {
                            ProcessCompilerError (tokens[0].sourceLineNumber, tokens[0].sourceLineIndex,
                              "Unexpected end-of-program");
                            //break; // Exit if end-of-program is reached
                        }
                }
                
            GetNextToken (tokens); // Move past the closing bracket `}`
            // DebugPrintToken(tokens[0], "ParseMAINCRISPDefinition \"CCBRACKET\" exit");
        }
        
            // RETURN for body
            // CODEGENERATION
            code.EmitFormattedLine ("", "RETURN");
            code.EmitUnformattedLine ("; **** =========");
            sprintf (line, "; **** END (%4d)", tokens[0].sourceLineNumber);
            code.EmitUnformattedLine (line);
            code.EmitUnformattedLine ("; **** =========");
            // ENDCODEGENERATION

    // DebugPrintToken(tokens[0], "ParseMAINCRISPDefinition exit");
    #ifdef TRACECOMPILER
   identifierTable.DisplayTableContents("Contents of identifier table at end of compilation of PROGRAM module definition");
#endif
    // DebugPrintToken(tokens[0], "ParseMAINDefinition exit");

   identifierTable.ExitNestedStaticScope();

   // GetNextToken(tokens);

    ExitModule ("PROGRAMDefinition");
}

//-----------------------------------------------------------
void ParseStatement (TOKEN tokens[])
//-----------------------------------------------------------
{
    void ParsePRINTStatement (TOKEN tokens[]);
   void ParseINPUTStatement(TOKEN tokens[]);
   void ParseAssignmentStatement(TOKEN tokens[]);
    void GetNextToken (TOKEN tokens[]);

    EnterModule ("Statement");
    // DebugPrintToken(tokens[0], "ParseStatement");
        // DebugPrintToken(tokens[0], "ParseStatement - entry");


    switch (tokens[0].type)
    {
        case PRINT:
            //GetNextToken(tokens); // move past bite
                        // DebugPrintToken(tokens[0], "ParseStatement - found PRINT statement");

            ParsePRINTStatement (tokens);
            break;
        case INPUT:
                    // DebugPrintToken(tokens[0], "ParseStatement - found INPUT statement");

             ParseINPUTStatement(tokens);
             break;
        case IDENTIFIER:
                    // DebugPrintToken(tokens[0], "ParseStatement - found IDENTIFIER (assignment or expression)");

             ParseAssignmentStatement(tokens);
             break;
        case INTEGER:
        case BOOLEAN:
            // New local variable declaration - treat it as a data definition
            // DebugPrintToken(tokens[0], "ParseStatement - found SPUD or TATER (local variable declaration)");
            ParseDataDefinitions(tokens, PROGRAMMODULESCOPE);
            break;
        default:
                    // DebugPrintToken(tokens[0], "ParseStatement - unrecognized statement type, throwing error");

            ProcessCompilerError (tokens[0].sourceLineNumber, tokens[0].sourceLineIndex,
                                  "Expecting beginning-of-statement");
            break;
    }

    ExitModule ("Statement");
}

//-----------------------------------------------------------
void ParsePRINTStatement (TOKEN tokens[])
//-----------------------------------------------------------
{
    void ParseExpression(TOKEN tokens[], DATATYPE &datatype);
    void GetNextToken (TOKEN tokens[]);

    char line[SOURCELINELENGTH+1];
    DATATYPE datatype;

    EnterModule ("PRINTStatement");
    // DebugPrintToken(tokens[0], "ParsePRINTStatement entry");

    // CODEGENERATION
    sprintf (line, "; **** PRINT statement (%4d)", tokens[0].sourceLineNumber);
    code.EmitUnformattedLine (line);
    // ENDCODEGENERATION

    do
    {
        // DebugPrintToken(tokens[0], "ParsePRINTStatement switch entry");
        
      GetNextToken(tokens); // move past bite
        switch (tokens[0].type)
        {
            case STRING:
                // DebugPrintToken(tokens[0], "ParsePRINTStatement STRING");
                // CODEGENERATION
                char reference[SOURCELINELENGTH+1];
                code.AddDSToStaticData(tokens[0].lexeme, "", reference);
                code.EmitFormattedLine("", "PUSHA", reference);
                code.EmitFormattedLine("", "SVC", "#SVC_WRITE_STRING");
                // ENDCODEGENERATION
                GetNextToken(tokens);
                break;
            case ENDL:
                // DebugPrintToken(tokens[0], "ParsePRINTStatement ENDL");
                // CODEGENERATION
                code.EmitFormattedLine("", "SVC", "#SVC_WRITE_ENDL");
                // ENDCODEGENERATION
                GetNextToken(tokens);
                break;

            default:
            {
                // DebugPrintToken(tokens[0], "ParsePRINTStatement default (expression)");
                ParseExpression(tokens, datatype);
                // DebugPrintToken(tokens[0], "ParsePRINTStatement after ParseExpression");

                // CODEGENERATION
                switch ( datatype )
                {
                   case INTTYPE:
                      code.EmitFormattedLine("","SVC","#SVC_WRITE_INTEGER");
                      break;
                   case BOOLTYPE:
                      code.EmitFormattedLine("","SVC","#SVC_WRITE_BOOLEAN");
                      break;
                }
                // ENDCODEGENERATION
                
                //GetNextToken(tokens);
                //break;

            }
        
        }
    } while ( tokens[0].type == COMMA );

   if ( tokens[0].type != SEMICOLON )
      ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,
                           "Print Expecting ';'");

    // DebugPrintToken(tokens[0], "ParsePRINTStatement end of do-while loop");
    GetNextToken(tokens);
    ExitModule ("PRINTStatement");
}
//-----------------------------------------------------------
void ParseINPUTStatement(TOKEN tokens[])
//-----------------------------------------------------------
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
      case INTTYPE:
         code.EmitFormattedLine("","SVC","#SVC_READ_INTEGER");
         break;
      case BOOLTYPE:
         code.EmitFormattedLine("","SVC","#SVC_READ_BOOLEAN");
         break;
   }
   code.EmitFormattedLine("","POP","@SP:0D1");
   code.EmitFormattedLine("","DISCARD","#0D1");
// ENDCODEGENERATION

   if ( tokens[0].type != SEMICOLON )
      ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Input Expecting ';'");

   GetNextToken(tokens);

   ExitModule("INPUTStatement");
}

//-----------------------------------------------------------
void ParseAssignmentStatement(TOKEN tokens[])
//-----------------------------------------------------------
{
   void ParseVariable(TOKEN tokens[],bool asLValue,DATATYPE &datatype);
   void ParseExpression(TOKEN tokens[],DATATYPE &datatype);
   void GetNextToken(TOKEN tokens[]);

   char line[SOURCELINELENGTH+1];
   DATATYPE datatypeLHS,datatypeRHS;
   int n;

   EnterModule("AssignmentStatement");
       // DebugPrintToken(tokens[0], "ParseAssignmentStatement - entry");


   sprintf(line,"; **** assignment statement (%4d)",tokens[0].sourceLineNumber);
   code.EmitUnformattedLine(line);

   ParseVariable(tokens,true,datatypeLHS);
       // DebugPrintToken(tokens[0], "ParseAssignmentStatement - after ParseVariable (LHS)");

   n = 1;

   // Parse any additional variables separated by ':'
   while ( tokens[0].type == COLON )
   {
               // DebugPrintToken(tokens[0], "ParseAssignmentStatement - found ':' for additional variable");

      DATATYPE datatype;

      GetNextToken(tokens);
              // DebugPrintToken(tokens[0], "ParseAssignmentStatement - after ':' for additional variable");

      ParseVariable(tokens,true,datatype);
              // DebugPrintToken(tokens[0], "ParseAssignmentStatement - after ParseVariable (additional)");

      n++;

      // Ensure all variables are of the same data type
      if ( datatype != datatypeLHS )
         ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Mixed-mode variables not allowed");
   }
   
    // Expect '=' for assignment

   if ( tokens[0].type != EQ )
      ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting '='");
    // DebugPrintToken(tokens[0], "ParseAssignmentStatement - found '=' for assignment");

   GetNextToken(tokens);

   ParseExpression(tokens,datatypeRHS);
       // DebugPrintToken(tokens[0], "ParseAssignmentStatement - after ParseExpression (RHS)");


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

   // End assignment statement
   if ( tokens[0].type != SEMICOLON )
      ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Assignment Expecting ';'");
   GetNextToken(tokens);

   ExitModule("AssignmentStatement");
       // DebugPrintToken(tokens[0], "ParseAssignmentStatement - exit");

}

//-----------------------------------------------------------
void ParseExpression(TOKEN tokens[],DATATYPE &datatype)
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

   void ParseConjunction(TOKEN tokens[],DATATYPE &datatype);
   void GetNextToken(TOKEN tokens[]);

   DATATYPE datatypeLHS,datatypeRHS;

   EnterModule("Expression");
   // DebugPrintToken(tokens[0], "ParseExpression entry");

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
               if ( !((datatypeLHS == BOOLTYPE) && (datatypeRHS == BOOLTYPE)) )
                  ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting boolean operands");
// ENDSTATICSEMANTICS
   
               code.EmitFormattedLine("","OR");
               datatype = BOOLTYPE;
               break;
            case NOR:
   
// STATICSEMANTICS
               if ( !((datatypeLHS == BOOLTYPE) && (datatypeRHS == BOOLTYPE)) )
                  ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting boolean operands");
// ENDSTATICSEMANTICS
   
               code.EmitFormattedLine("","NOR");
               datatype = BOOLTYPE;
               break;
            case XOR:
   
// STATICSEMANTICS
               if ( !((datatypeLHS == BOOLTYPE) && (datatypeRHS == BOOLTYPE)) )
                  ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting boolean operands");
// ENDSTATICSEMANTICS
   
               code.EmitFormattedLine("","XOR");
               datatype = BOOLTYPE;
               break;
         }
      }
// CODEGENERATION

   }
   else
      datatype = datatypeLHS;

   ExitModule("Expression");
   // DebugPrintToken(tokens[0], "ParseExpression exit");
}

//-----------------------------------------------------------
void ParseConjunction(TOKEN tokens[],DATATYPE &datatype)
//-----------------------------------------------------------
{
   void ParseNegation(TOKEN tokens[],DATATYPE &datatype);
   void GetNextToken(TOKEN tokens[]);

   DATATYPE datatypeLHS,datatypeRHS;

   EnterModule("Conjunction");
   // DebugPrintToken(tokens[0], "ParseConjunction entry");

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
               if ( !((datatypeLHS == BOOLTYPE) && (datatypeRHS == BOOLTYPE)) )
                  ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting boolean operands");
               code.EmitFormattedLine("","AND");
               datatype = BOOLTYPE;
               break;
            case NAND:
               if ( !((datatypeLHS == BOOLTYPE) && (datatypeRHS == BOOLTYPE)) )
                  ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting boolean operands");
               code.EmitFormattedLine("","NAND");
               datatype = BOOLTYPE;
               break;
         }
      }
   }
   else
      datatype = datatypeLHS;

   ExitModule("Conjunction");
   // DebugPrintToken(tokens[0], "ParseConjunction exit");
}

//-----------------------------------------------------------
void ParseNegation(TOKEN tokens[],DATATYPE &datatype)
//-----------------------------------------------------------
{
   void ParseComparison(TOKEN tokens[],DATATYPE &datatype);
   void GetNextToken(TOKEN tokens[]);

   DATATYPE datatypeRHS;

   EnterModule("Negation");
   // DebugPrintToken(tokens[0], "ParseNegation entry");

   if ( tokens[0].type == NOT )
   {
      GetNextToken(tokens);
      ParseComparison(tokens,datatypeRHS);

      if ( !(datatypeRHS == BOOLTYPE) )
         ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting boolean operand");
      code.EmitFormattedLine("","NOT");
      datatype = BOOLTYPE;
   }
   else
      ParseComparison(tokens,datatype);

   ExitModule("Negation");
   // DebugPrintToken(tokens[0], "ParseNegation exit");
}

//-----------------------------------------------------------
void ParseComparison(TOKEN tokens[],DATATYPE &datatype)
//-----------------------------------------------------------
{
   void ParseComparator(TOKEN tokens[],DATATYPE &datatype);
   void GetNextToken(TOKEN tokens[]);

   DATATYPE datatypeLHS,datatypeRHS;

   EnterModule("Comparison");
   // DebugPrintToken(tokens[0], "ParseComparison entry");

   ParseComparator(tokens,datatypeLHS);
   if ( (tokens[0].type ==    LT) ||
        (tokens[0].type ==  LTEQ) ||
        (tokens[0].type ==  EQEQ) ||
        (tokens[0].type ==    GT) ||
        (tokens[0].type ==  GTEQ) ||
        (tokens[0].type == NOTEQ)
      )
   {
      TOKENTYPE operation = tokens[0].type;

      GetNextToken(tokens);
      ParseComparator(tokens,datatypeRHS);

      if ( (datatypeLHS != INTTYPE) || (datatypeRHS != INTTYPE) )
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
         case EQEQ:
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
      datatype = BOOLTYPE;
      code.EmitFormattedLine("","PUSH","#0X0000");
      code.EmitFormattedLine("","JMP",Elabel);
      code.EmitFormattedLine(Tlabel,"PUSH","#0XFFFF");
      code.EmitFormattedLine(Elabel,"EQU","*");
   }
   else
      datatype = datatypeLHS;

   ExitModule("Comparison");
   // DebugPrintToken(tokens[0], "ParseComparison exit");
}

//-----------------------------------------------------------
void ParseComparator(TOKEN tokens[],DATATYPE &datatype)
//-----------------------------------------------------------
{
   void ParseTerm(TOKEN tokens[],DATATYPE &datatype);
   void GetNextToken(TOKEN tokens[]);

   DATATYPE datatypeLHS,datatypeRHS;

   EnterModule("Comparator");
   // DebugPrintToken(tokens[0], "ParseComparator entry");

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

         if ( (datatypeLHS != INTTYPE) || (datatypeRHS != INTTYPE) )
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
         datatype = INTTYPE;
      }
   }
   else
      datatype = datatypeLHS;
   
   ExitModule("Comparator");
   // DebugPrintToken(tokens[0], "ParseComparator exit");
}

//-----------------------------------------------------------
void ParseTerm(TOKEN tokens[],DATATYPE &datatype)
//-----------------------------------------------------------
{
   void ParseFactor(TOKEN tokens[],DATATYPE &datatype);
   void GetNextToken(TOKEN tokens[]);

   DATATYPE datatypeLHS,datatypeRHS;

   EnterModule("Term");
   // DebugPrintToken(tokens[0], "ParseTerm entry");

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

         if ( (datatypeLHS != INTTYPE) || (datatypeRHS != INTTYPE) )
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
         datatype = INTTYPE;
      }
   }
   else
      datatype = datatypeLHS;

   ExitModule("Term");
   // DebugPrintToken(tokens[0], "ParseTerm exit");
}

//-----------------------------------------------------------
void ParseFactor(TOKEN tokens[],DATATYPE &datatype)
//-----------------------------------------------------------
{
   void ParseSecondary(TOKEN tokens[],DATATYPE &datatype);
   void GetNextToken(TOKEN tokens[]);

   EnterModule("Factor");
   // DebugPrintToken(tokens[0], "ParseFactor entry");

   if ( (tokens[0].type ==   ABS) ||
        (tokens[0].type ==  PLUS) ||
        (tokens[0].type == MINUS)
      )
   {
      DATATYPE datatypeRHS;
      TOKENTYPE operation = tokens[0].type;

      GetNextToken(tokens);
      ParseSecondary(tokens,datatypeRHS);

      if ( datatypeRHS != INTTYPE )
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
            // DebugPrintToken(tokens[0], "ParseFactor \"plus\"");
            break;
         case MINUS:
            code.EmitFormattedLine("","NEGI");
            break;
      }
      datatype = INTTYPE;
   }
   else
      ParseSecondary(tokens,datatype);
   
   ExitModule("Factor");
   // DebugPrintToken(tokens[0], "ParseFactor exit");
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
    DebugPrintToken(tokens[0], "ParseSecondary - entry");

    // Step 1: Parse prefix operator if it exists
    DebugPrintToken(tokens[0], "ParseSecondary - before ParsePrefix");
    ParsePrefix(tokens, datatypeLHS);
    DebugPrintToken(tokens[0], "ParseSecondary - after ParsePrefix");

    // Step 2: Check for power operation (e.g., '^')
    if (tokens[0].type == POWER)
    {
        DebugPrintToken(tokens[0], "ParseSecondary - found POWER operator");

        GetNextToken(tokens); // Move past the POWER operator

        // Parse right-hand side of the power operation
        DebugPrintToken(tokens[0], "ParseSecondary - before ParsePrefix for RHS of POWER");
        ParsePrefix(tokens, datatypeRHS);
        DebugPrintToken(tokens[0], "ParseSecondary - after ParsePrefix for RHS of POWER");

        // STATIC SEMANTICS: Ensure both operands are integers for power operation
        if ((datatypeLHS != INTTYPE) || (datatypeRHS != INTTYPE))
        {
            DebugPrintToken(tokens[0], "ParseSecondary - error: POWER requires integer operands");
            ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting integer operands");
        }

        // CODE GENERATION: Emit code for power operation
        code.EmitFormattedLine("", "POWI");
        datatype = INTTYPE;
    }
    

    // Step 3: Parse postfix operator immediately after prefix
    DebugPrintToken(tokens[0], "ParseSecondary - before ParsePostfix");
    ParsePostfix(tokens, datatypeLHS);
    DebugPrintToken(tokens[0], "ParseSecondary - after ParsePostfix");

    // Update datatype to reflect changes from prefix or postfix
    datatype = datatypeLHS;

    ExitModule("Secondary");
    DebugPrintToken(tokens[0], "ParseSecondary - exit");
}
//-----------------------------------------------------------
void ParsePrefix(TOKEN tokens[], DATATYPE &datatype)
//-----------------------------------------------------------
{
    void ParseVariable(TOKEN tokens[], bool asLValue, DATATYPE &datatype);
    void ParsePrimary(TOKEN tokens[], DATATYPE &datatype);
    void GetNextToken(TOKEN tokens[]);

    EnterModule("Prefix");
    DebugPrintToken(tokens[0], "ParsePrefix - entry");

    if ((tokens[0].type == INC) || (tokens[0].type == DEC))
    {
        DebugPrintToken(tokens[0], "ParsePrefix - found increment/decrement");

        DATATYPE datatypeRHS;
        TOKENTYPE operation = tokens[0].type;

        GetNextToken(tokens);
        DebugPrintToken(tokens[0], "ParsePrefix - after GetNextToken for increment/decrement");

        // Handle additional + or -
        if ((tokens[0].type == PLUS) || (tokens[0].type == MINUS))
        {
            DebugPrintToken(tokens[0], "ParsePrefix - found additional PLUS or MINUS after INC/DEC");
            GetNextToken(tokens);
        }

        // Parse the variable to be incremented or decremented
        DebugPrintToken(tokens[0], "ParsePrefix - before ParseVariable for increment/decrement");
        ParseVariable(tokens, true, datatypeRHS);
        DebugPrintToken(tokens[0], "ParsePrefix - after ParseVariable for increment/decrement");

        // Ensure the datatype is integer
        if (datatypeRHS != INTTYPE)
        {
            DebugPrintToken(tokens[0], "ParsePrefix - error: Increment/Decrement requires integer operand");
            ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Increment/Decrement requires an integer operand");
        }

        // Code generation for prefix increment or decrement
        switch (operation)
        {
            case INC:
                DebugPrintToken(tokens[0], "ParsePrefix - generating code for INC");
                // PRE-INCREMENT
                code.EmitFormattedLine("", "PUSH", "@SP:0D0");
                code.EmitFormattedLine("", "PUSH", "#0D1");
                code.EmitFormattedLine("", "ADDI");
                code.EmitFormattedLine("", "POP", "@SP:0D1");  // Side effect
                code.EmitFormattedLine("", "PUSH", "@SP:0D0");
                code.EmitFormattedLine("", "SWAP");
                code.EmitFormattedLine("", "DISCARD", "#0D1");  // Value left on top of stack
                break;

            case DEC:
                DebugPrintToken(tokens[0], "ParsePrefix - generating code for DEC");
                // PRE-DECREMENT
                code.EmitFormattedLine("", "PUSH", "@SP:0D0");
                code.EmitFormattedLine("", "PUSH", "#0D1");
                code.EmitFormattedLine("", "SUBI");
                code.EmitFormattedLine("", "POP", "@SP:0D1");  // Side effect
                code.EmitFormattedLine("", "PUSH", "@SP:0D0");
                code.EmitFormattedLine("", "SWAP");
                code.EmitFormattedLine("", "DISCARD", "#0D1");  // Value left on top of stack
                break;
        }

        datatype = INTTYPE;
    }
    else
    {
        DebugPrintToken(tokens[0], "ParsePrefix - no INC/DEC, proceeding to ParsePrimary");
        ParsePrimary(tokens, datatype);
    }

    ExitModule("Prefix");
    DebugPrintToken(tokens[0], "ParsePrefix - exit");
}

//-----------------------------------------------------------
void ParsePostfix(TOKEN tokens[], DATATYPE &datatype)
//-----------------------------------------------------------
{
    void ParseVariable(TOKEN tokens[], bool asLValue, DATATYPE &datatype);
    void GetNextToken(TOKEN tokens[]);

    EnterModule("Postfix");
    DebugPrintToken(tokens[0], "ParsePostfix - entry");

    if ((tokens[0].type == INC) || (tokens[0].type == DEC))
    {
        DebugPrintToken(tokens[0], "ParsePostfix - found increment/decrement");

        TOKENTYPE operation = tokens[0].type;

        switch (operation)
        {
            case INC:
                DebugPrintToken(tokens[0], "ParsePostfix - generating code for INC");
                // POST-INCREMENT
                code.EmitFormattedLine("", "PUSH", "@SP:0D0");  // Push current value for use
                code.EmitFormattedLine("", "PUSH", "#0D1");     // Increment by 1
                code.EmitFormattedLine("", "ADDI");
                code.EmitFormattedLine("", "POP", "@SP:0D1");   // Store back
                break;

            case DEC:
                DebugPrintToken(tokens[0], "ParsePostfix - generating code for DEC");
                // POST-DECREMENT
                code.EmitFormattedLine("", "PUSH", "@SP:0D0");  // Push current value for use
                code.EmitFormattedLine("", "PUSH", "#0D1");     // Decrement by 1
                code.EmitFormattedLine("", "SUBI");
                code.EmitFormattedLine("", "POP", "@SP:0D1");   // Store back
                break;
        }

		GetNextToken(tokens);
        // Advance the token stream after postfix processing if additional operators are present
        if ((tokens[0].type == PLUS) || (tokens[0].type == MINUS))
        {
            DebugPrintToken(tokens[0], "ParsePostfix - found additional PLUS or MINUS after INC/DEC");
            GetNextToken(tokens);
        }

        datatype = INTTYPE;
    }

    ExitModule("Postfix");
    DebugPrintToken(tokens[0], "ParsePostfix - exit");
}

//-----------------------------------------------------------
void ParsePrimary(TOKEN tokens[],DATATYPE &datatype)
//-----------------------------------------------------------
{   
   void ParseVariable(TOKEN tokens[],bool asLValue,DATATYPE &datatype);
   void ParseExpression(TOKEN tokens[],DATATYPE &datatype);
   void GetNextToken(TOKEN tokens[]);

   EnterModule("Primary");
   // DebugPrintToken(tokens[0], "ParsePrimary - entry");


   switch ( tokens[0].type )
   {
      case INTEGER:
         {
            char operand[SOURCELINELENGTH+1];
            
            sprintf(operand,"#0D%s",tokens[0].lexeme);
            code.EmitFormattedLine("","PUSH",operand);
            datatype = INTTYPE;
            GetNextToken(tokens);
         }
         break;
   //************* Thanks to Cayden Garcia (FA2023)
   // ***BEWARE*** when you choose a different lexeme for either boolean value!
   //*************
      case TRUE:
         code.EmitFormattedLine("","PUSH","#0XFFFF"); // or "#true"
         datatype = BOOLTYPE;
         GetNextToken(tokens);
         break;
      case FALSE:
         code.EmitFormattedLine("","PUSH","#0X0000"); // or "false"
         datatype = BOOLTYPE;
         GetNextToken(tokens);
         break;
      case OPARENTHESIS:
         GetNextToken(tokens);
         ParseExpression(tokens,datatype);
         if ( tokens[0].type != CPARENTHESIS )
            ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting )");
         GetNextToken(tokens);
         break;
      case IDENTIFIER:
         ParseVariable(tokens,false,datatype);
         break;
      default:
         ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,
                              "Expecting integer, true, false, '(', variable");
         break;
   }

   ExitModule("Primary");
      // DebugPrintToken(tokens[0], "ParsePrimary - exit");

}


/*
Syntax "locations"                 l- or r-value
---------------------------------  -------------
<expression>                       r-value
<prefix>                           l-value
<INPUTStatement>                   l-value
LHS of <assignmentStatement>       l-value

r-value ( read-only): value is pushed on run-time stack
l-value (read/write): address of value is pushed on run-time stack
*/
//-----------------------------------------------------------
void ParseVariable(TOKEN tokens[],bool asLValue,DATATYPE &datatype)
//-----------------------------------------------------------
{

   void GetNextToken(TOKEN tokens[]);

   bool isInTable;
   int index;
   IDENTIFIERTYPE identifierType;

   EnterModule("Variable");
      // DebugPrintToken(tokens[0], "ParseVariable - entry");


   if ( tokens[0].type != IDENTIFIER )
      ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting identifier");

// STATICSEMANTICS
   index = identifierTable.GetIndex(tokens[0].lexeme,isInTable);
   if ( !isInTable )
      ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Undefined identifier");
   
   identifierType = identifierTable.GetType(index);
   datatype = identifierTable.GetDatatype(index);

   if ( !((identifierType ==        GLOBAL_VARIABLE) ||
          (identifierType ==        GLOBAL_CONSTANT) ||
          (identifierType == PROGRAMMODULE_VARIABLE) ||
          (identifierType == PROGRAMMODULE_CONSTANT)) )
      ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting variable or constant identifier");
      
   if ( asLValue && ((identifierType == GLOBAL_CONSTANT) || (identifierType == PROGRAMMODULE_CONSTANT)) )
      ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Constant may not be l-value");
// ENDSTATICSEMANTICS

// CODEGENERATION
   if ( asLValue )
      code.EmitFormattedLine("","PUSHA",identifierTable.GetReference(index));
   else
      code.EmitFormattedLine("","PUSH",identifierTable.GetReference(index));
// ENDCODEGENERATION

   GetNextToken(tokens);

   ExitModule("Variable");
      // DebugPrintToken(tokens[0], "ParseVariable - exit");

}

//-----------------------------------------------------------
void Callback1 (int sourceLineNumber, const char sourceLine[])
//-----------------------------------------------------------
{
    cout << setw (4) << sourceLineNumber << " " << sourceLine << endl;
}

//-----------------------------------------------------------
void Callback2 (int sourceLineNumber, const char sourceLine[])
//-----------------------------------------------------------
{
    char line[SOURCELINELENGTH + 1];

    // CODEGENERATION
    sprintf (line, "; %4d %s", sourceLineNumber, sourceLine);
    code.EmitUnformattedLine (line);
    // ENDCODEGENERATION
}

//-----------------------------------------------------------
void GetNextToken (TOKEN tokens[])
//-----------------------------------------------------------
{
    const char *TokenDescription (TOKENTYPE type);

    int i;
    TOKENTYPE type;
    char lexeme[SOURCELINELENGTH + 1];
    int sourceLineNumber;
    int sourceLineIndex;
    char information[SOURCELINELENGTH + 1];

    //============================================================
    // Move look-ahead "window" to make room for next token-and-lexeme
    //============================================================
    for (int i = 1; i <= LOOKAHEAD; i++)
        tokens[i - 1] = tokens[i];

    char nextCharacter = reader.GetLookAheadCharacter (0).character;

    //============================================================
    // "Eat" white space and comments
    //============================================================
    do
        {
            //    "Eat" any white-space (blanks and EOLCs and TABCs)
            while ((nextCharacter == ' ')
                   || (nextCharacter == READER<CALLBACKSUSED>::EOLC)
                   || (nextCharacter == READER<CALLBACKSUSED>::TABC))
                nextCharacter = reader.GetNextCharacter ().character;

            //  "Eat" line comment
            if ((nextCharacter == '|') && (reader.GetLookAheadCharacter (1).character == '|'))
                {

#ifdef TRACESCANNER
                    sprintf (information, "At (%4d:%3d) begin line comment",
                             reader.GetLookAheadCharacter (0).sourceLineNumber,
                             reader.GetLookAheadCharacter (0).sourceLineIndex);
                    lister.ListInformationLine (information);
#endif

                    do
                        nextCharacter = reader.GetNextCharacter ().character;
                    while ((nextCharacter != READER<CALLBACKSUSED>::EOLC)
                           && (nextCharacter != READER<CALLBACKSUSED>::EOPC));
                }

            //    "Eat" block comments (nesting allowed)
            if ((nextCharacter == '|') && (reader.GetLookAheadCharacter (1).character == '['))
                {
                    int depth = 0;

                    do
                        {
                            if ((nextCharacter == '|') && (reader.GetLookAheadCharacter (1).character == '['))
                                {
                                    depth++;

#ifdef TRACESCANNER
                                    sprintf (information, "At (%4d:%3d) begin block comment depth = %d",
                                             reader.GetLookAheadCharacter (0).sourceLineNumber,
                                             reader.GetLookAheadCharacter (0).sourceLineIndex,
                                             depth);
                                    lister.ListInformationLine (information);
#endif

                                    nextCharacter = reader.GetNextCharacter ().character;
                                    nextCharacter = reader.GetNextCharacter ().character;
                                }
                            else if ((nextCharacter == ']') && (reader.GetLookAheadCharacter (1).character == '|'))
                                {

#ifdef TRACESCANNER
                                    sprintf (information, "At (%4d:%3d)   end block comment depth = %d",
                                             reader.GetLookAheadCharacter(0).sourceLineNumber,
                                             reader.GetLookAheadCharacter(0).sourceLineIndex,
                                             depth);
                                    lister.ListInformationLine (information);
#endif

                                    depth--;
                                    nextCharacter = reader.GetNextCharacter ().character;
                                    nextCharacter = reader.GetNextCharacter ().character;
                                }
                            else
                                nextCharacter = reader.GetNextCharacter ().character;
                        }
                    while ((depth != 0) && (nextCharacter != READER<CALLBACKSUSED>::EOPC));
                    if (depth != 0)
                        ProcessCompilerError ( reader.GetLookAheadCharacter (0).sourceLineNumber,
                            reader.GetLookAheadCharacter (0).sourceLineIndex,
                            "DEBTH: Unexpected end-of-program");
                }
        /* WHILE ( (nextCharacter is not a white-space character)
            or (nextCharacter is not beginning-of-comment character) ) */
        } while ((nextCharacter == ' ')
           || (nextCharacter == READER<CALLBACKSUSED>::EOLC)
           || (nextCharacter == READER<CALLBACKSUSED>::TABC)
           || ((nextCharacter == '|') && (reader.GetLookAheadCharacter (1).character == '[')) // allows block comments back to back
           || ((nextCharacter == '|') && (reader.GetLookAheadCharacter (1).character == '|'))); // allows inline comments after block comment

    //============================================================
    // Scan token
    //============================================================
    sourceLineNumber = reader.GetLookAheadCharacter (0).sourceLineNumber;
    sourceLineIndex = reader.GetLookAheadCharacter (0).sourceLineIndex;

    // IF ( nextCharacter begins a reserved word or an <identifier> ) THEN
    if (isalpha (nextCharacter))
        {
            // Build lexeme
            char UCLexeme[SOURCELINELENGTH + 1];

            i = 0;
            lexeme[i++] = nextCharacter;
            nextCharacter = reader.GetNextCharacter ().character;
        while (isalpha (nextCharacter) || isdigit (nextCharacter) || nextCharacter == '_')
        {
                    lexeme[i++] = nextCharacter;
                    nextCharacter = reader.GetNextCharacter ().character;
                }
            lexeme[i] = '\0';
            for (i = 0; i <= (int)strlen (lexeme); i++)
                UCLexeme[i] = toupper (lexeme[i]);

            // Try to find the lexeme in the table of reserved words
            bool isFound = false;

            i = 0;
            while ( !isFound && (i <= (sizeof (TOKENTABLE) / sizeof (TOKENTABLERECORD)) - 1))
                {
                    if (TOKENTABLE[i].isReservedWord && (strcmp (UCLexeme, TOKENTABLE[i].description) == 0))
                        {
                            isFound = true;
                            type = TOKENTABLE[i].type;
                        }
                    else
                        i++;
                }
            if (!isFound) // Not a reserved word, must be an <identifier>.
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
            switch (nextCharacter)
                {
                // <string> literal *Note* escape character sequences \n,\t,\b,\r,\\,\" supported
                case '"':
                    i = 0;
                    nextCharacter = reader.GetNextCharacter ().character;
                    while ((nextCharacter != '"')
                           && (nextCharacter != READER<CALLBACKSUSED>::EOLC)
                           && (nextCharacter != READER<CALLBACKSUSED>::EOPC))
                        {
                            if (nextCharacter == '\\')
                                {
                                    lexeme[i++] = nextCharacter;
                                    nextCharacter = reader.GetNextCharacter ().character;
                                     if ( (nextCharacter ==  'n') ||
                                       (nextCharacter ==  't') ||
                                       (nextCharacter ==  'b') ||
                                       (nextCharacter ==  'r') ||
                                       (nextCharacter == '\\') ||
                                       (nextCharacter == ',') ||
                                       (nextCharacter ==  '"') )
                                        {
                                            lexeme[i++] = nextCharacter;
                                        }
                                    else
                                        ProcessCompilerError ( sourceLineNumber, sourceLineIndex,
                                            "Illegal escape character sequence in string literal");
                                }
                            else
                                {
                                    lexeme[i++] = nextCharacter;
                                }
                            nextCharacter = reader.GetNextCharacter ().character;
                        }
                    if (nextCharacter != '"')
                        ProcessCompilerError (sourceLineNumber, sourceLineIndex,
                                              "Un-terminated string literal");
                    lexeme[i] = '\0';
                    type = STRING;
                    reader.GetNextCharacter ();
                    break;
                case READER<CALLBACKSUSED>::EOPC:
                    {
                        static int count = 0;

                        if (++count > (LOOKAHEAD + 1))
                            ProcessCompilerError ( sourceLineNumber, sourceLineIndex,
                                "EOPC: Unexpected end-of-program");
                        else
                            {
                                type = EOPTOKEN;
                                reader.GetNextCharacter ();
                                lexeme[0] = '\0';
                            }
                    }
                    break;
                case ',':
                    type = COMMA;
                    lexeme[0] = nextCharacter; lexeme[1] = '\0';
                    reader.GetNextCharacter ();
                    break;
                case '.':
                    type = PERIOD;
                    lexeme[0] = nextCharacter; lexeme[1] = '\0';
                    reader.GetNextCharacter ();
                    break;
                    case ';': 
    type = SEMICOLON; 
    lexeme[0] = nextCharacter;
    lexeme[1] = '\0';
    reader.GetNextCharacter(); 
    break;
                    case ':': 
    type = COLON; 
    lexeme[0] = nextCharacter;
    lexeme[1] = '\0';
    reader.GetNextCharacter(); 
    break;

                case '{':
                    type = OCBRACKET;
                    lexeme[0] = nextCharacter; lexeme[1] = '\0';
                    reader.GetNextCharacter ();
                    break;
                case '}':
                    type = CCBRACKET;
                    lexeme[0] = nextCharacter; lexeme[1] = '\0';
                    reader.GetNextCharacter ();
                    break;
                case '[':
                    if ( reader.GetLookAheadCharacter(1).character == ']' ) // --------------- ----  '[]'
                    {
                      lexeme[0] = nextCharacter;
                        nextCharacter = reader.GetNextCharacter().character; // Move past '['
                        lexeme[1] = nextCharacter; // Capture ']'
                        lexeme[2] = '\0';
                        type = CONCAT;
                        reader.GetNextCharacter(); // Move past ']'
                    }else {
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
                    reader.GetNextCharacter ();
                    break;
                case '(':
                    type = OPARENTHESIS;
                    lexeme[0] = nextCharacter; lexeme[1] = '\0';
                    reader.GetNextCharacter ();
                    break;
                case ')':
                    type = CPARENTHESIS;
                    lexeme[0] = nextCharacter; lexeme[1] = '\0';
                    reader.GetNextCharacter ();
                    break;
                case '<': // ---------------------------------------------------------------------  '<'
                    lexeme[0] = nextCharacter;
                    nextCharacter = reader.GetNextCharacter().character;
                    if      ( nextCharacter == '=' ) // ------------------------------------------  '<='
                    {
                       type = LTEQ;
                       lexeme[1] = nextCharacter; lexeme[2] = '\0';
                       reader.GetNextCharacter();
                    }
                    else if ( nextCharacter == '>' ) // ------------------------------------------  '<>'
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
                 case '=': // ---------------------------------------------------------------------  '='
                    if ( reader.GetLookAheadCharacter(1).character == '=' ) // --------------- ----  '!='
                    {
                       nextCharacter = reader.GetNextCharacter().character;
                       lexeme[1] = nextCharacter; lexeme[2] = '\0';
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
                 case '>': // ---------------------------------------------------------------------  '>'
                    lexeme[0] = nextCharacter;
                    nextCharacter = reader.GetNextCharacter().character;
                    if ( nextCharacter == '=' ) // ------------------------------------------------  '>='
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
                       type = NOT;
                       lexeme[1] = '\0';
                       reader.GetNextCharacter();
                    }
                    break;
                 case '+': 
                    lexeme[0] = nextCharacter;
                    if (reader.GetLookAheadCharacter(1).character == '\\') {
                        nextCharacter = reader.GetNextCharacter().character;
                        lexeme[1] = nextCharacter;
                        lexeme[2] = '\0';
                        reader.GetNextCharacter(); 
                        type = XOR;
                    } else if ( reader.GetLookAheadCharacter(1).character == '+' )
            {
               nextCharacter = reader.GetNextCharacter().character;
               lexeme[1] = nextCharacter; lexeme[2] = '\0';
               type = INC;
            } else {
                        type = PLUS;
                        lexeme[1] = '\0';
                        reader.GetNextCharacter(); 
                    }
                    break;
                 case '-': 
                 if ( reader.GetLookAheadCharacter(1).character == '-' )
            {
               nextCharacter = reader.GetNextCharacter().character;
               lexeme[1] = nextCharacter; lexeme[2] = '\0';
               type = DEC;
            } else {
                    type = MINUS;
                    lexeme[0] = nextCharacter; lexeme[1] = '\0';
                    reader.GetNextCharacter();
            }
                    break;
                 case '*':
                    type = MULTIPLY;
                    lexeme[0] = nextCharacter; lexeme[1] = '\0';
                    reader.GetNextCharacter();
                    break;
                 case '/': 
                    type = DIVIDE;
                    lexeme[0] = nextCharacter; lexeme[1] = '\0';
                    reader.GetNextCharacter();
                    break;
                case '\\':
                     lexeme[0] = nextCharacter;
                    if (reader.GetLookAheadCharacter(1).character == '\\') {
                        nextCharacter = reader.GetNextCharacter().character; 
                        lexeme[1] = nextCharacter;
                        lexeme[2] = '\0';
                        reader.GetNextCharacter(); 
                        type = OR;
                    }
                    else {
                        type = UNKTOKEN;
                        lexeme[1] = '\0';
                        reader.GetNextCharacter(); 
                    }
                    break;
                 case '&': 
                    lexeme[0] = nextCharacter;
                    if (reader.GetLookAheadCharacter(1).character == '&') {
                        nextCharacter = reader.GetNextCharacter().character;
                        lexeme[1] = nextCharacter;
                        lexeme[2] = '\0';
                        reader.GetNextCharacter();
                        type = AND;
                    } 
                    else {
                        type = UNKTOKEN;
                        lexeme[1] = '\0';
                        reader.GetNextCharacter(); 
                    }
                    break;
                case '~':
                    lexeme[0] = nextCharacter;
                    if (reader.GetLookAheadCharacter(1).character == '\\') {
                        nextCharacter = reader.GetNextCharacter().character;
                        lexeme[1] = nextCharacter;
                        lexeme[2] = '\0';
                        reader.GetNextCharacter();
                        type = NOR;
                    }else if (reader.GetLookAheadCharacter(1).character == '&') {
                        nextCharacter = reader.GetNextCharacter().character;
                        lexeme[1] = nextCharacter;
                        lexeme[2] = '\0';
                        reader.GetNextCharacter();
                        type = NAND;
                    } else {
                        type = UNKTOKEN;
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
                default:
                    type = UNKTOKEN;
                    lexeme[0] = nextCharacter; lexeme[1] = '\0';
                    reader.GetNextCharacter ();
                    break;
                }
        }

    tokens[LOOKAHEAD].type = type;
    strcpy (tokens[LOOKAHEAD].lexeme, lexeme);
    tokens[LOOKAHEAD].sourceLineNumber = sourceLineNumber;
    tokens[LOOKAHEAD].sourceLineIndex = sourceLineIndex;

#ifdef TRACESCANNER
    sprintf (information, "At (%4d:%3d) token = %12s lexeme = |%s|",
             tokens[LOOKAHEAD].sourceLineNumber,
             tokens[LOOKAHEAD].sourceLineIndex, 
             TokenDescription (type), lexeme);
    lister.ListInformationLine (information);
#endif

}

//-----------------------------------------------------------
const char *TokenDescription (TOKENTYPE type)
//-----------------------------------------------------------
{
    int i;
    bool isFound;

    isFound = false;
    i = 0;
    while (!isFound && (i <= (sizeof (TOKENTABLE) / sizeof (TOKENTABLERECORD)) - 1))
        {
            if (TOKENTABLE[i].type == type)
                isFound = true;
            else
                i++;
        }
    return (isFound ? TOKENTABLE[i].description : "???????");
}
