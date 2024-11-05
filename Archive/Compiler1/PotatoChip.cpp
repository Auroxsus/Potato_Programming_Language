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
#include <ctime> // Used in Potato.h for time stamp
#include <cctype>
#include <vector>

using namespace std;

// #define TRACEREADER
// #define TRACESCANNER
// #define TRACEPARSER
#define TRACECOMPILER

#include "PotatoChip.h"

//-----------------------------------------------------------
typedef enum
//-----------------------------------------------------------
{
    // pseudo-terminals
    IDENTIFIER,
    STRING,
    EOPTOKEN,
    UNKTOKEN,
    // reserved words
    CRISP,
    MAINCRISP,
    PRINT,
    ENDL,
    // punctuation
    COMMA,
    PERIOD,
    OCBRACKET,
    CCBRACKET,
    OPARENTHESIS,
    CPARENTHESIS,
    CONCAT
    // operators
    // *** NONE ***
} TOKENTYPE;

//-----------------------------------------------------------
struct TOKENTABLERECORD
//-----------------------------------------------------------
{
    TOKENTYPE type;
    char description[12 + 1];
    bool isReservedWord;
};

//-----------------------------------------------------------
const TOKENTABLERECORD TOKENTABLE[] =
//-----------------------------------------------------------
{ 
    { IDENTIFIER, "IDENTIFIER", false },
    { STRING, "STRING", false },
    { EOPTOKEN, "EOPTOKEN", false },
    { UNKTOKEN, "UNKTOKEN", false },
    { CRISP, "CRISP", true },
    { MAINCRISP, "MAINCRISP", true },
    { PRINT, "BITE", true },
    { ENDL, "ENDL", true },
    { COMMA, "COMMA", false },
    { PERIOD, "PERIOD", false },
    { OCBRACKET, "OCBRACKET", false },
    { CCBRACKET, "CCBRACKET", false },
    { OPARENTHESIS, "OPARENTHESIS", false },
    { CPARENTHESIS, "CPARENTHESIS", false },
    { CONCAT, "CONCAT", false } 
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
// ENDCODEGENERATION

#ifdef TRACEPARSER
int level;
#endif

//--------------------------------------------------
// Function to get token description from TOKENTABLE
//--------------------------------------------------
const char *GetTokenDescription (TOKENTYPE type)
{
    for (int i = 0; i < sizeof (TOKENTABLE) / sizeof (TOKENTABLERECORD); ++i)
        { // for (const auto& record : TOKENTABLE) {
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
            cout << "POTATO exception: " << potatoException.GetDescription ()
                 << endl;
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
    void ParseMAINCRISPDefinition (TOKEN tokens[]);
    void GetNextToken (TOKEN tokens[]);

    EnterModule ("POTATOProgram");

    if (tokens[0].type != CRISP) {
        ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, "Expecting `crisp` keyword");
        return;
    }
    GetNextToken(tokens); // Move past `crisp`

    if (tokens[0].type == MAINCRISP)
            ParseMAINCRISPDefinition (tokens);
    else
        ProcessCompilerError (tokens[0].sourceLineNumber, tokens[0].sourceLineIndex, 
                              "Expecting MAINCRISP");

    if (tokens[0].type != EOPTOKEN)
        ProcessCompilerError (tokens[0].sourceLineNumber, tokens[0].sourceLineIndex,
                              "Expecting end-of-program");

    ExitModule ("POTATOProgram");
}

//-----------------------------------------------------------
void ParseMAINCRISPDefinition (TOKEN tokens[])
//-----------------------------------------------------------
{
    void ParseStatement (TOKEN tokens[]);
    void GetNextToken (TOKEN tokens[]);

    char line[SOURCELINELENGTH + 1];
    char label[SOURCELINELENGTH + 1];
    char reference[SOURCELINELENGTH + 1];

    EnterModule ("MAINCRISPDefinition");
    
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

    // DebugPrintToken (tokens[0], "ParseMAINCRISPDefinition");
    GetNextToken (tokens); // Move past 'crispMAIN'

    /* 
     // Check for identifier
     if (tokens[0].type != IDENTIFIER) {
         ProcessCompilerError(tokens[0].sourceLineNumber,
     tokens[0].sourceLineIndex, "Expecting identifier for MAINCRISP function
     definition"); return;
     }
     // DebugPrintToken(tokens[0], "ParseMAINCRISPDefinition \"identifier\" exit");
     GetNextToken(tokens);  // Move past identifier
    */

    // Check for empty parameter list `()`
    if (tokens[0].type != OPARENTHESIS) {
        ProcessCompilerError (tokens[0].sourceLineNumber,
                              tokens[0].sourceLineIndex, "Expecting '('");
    }
    // DebugPrintToken (tokens[0], "ParseMAINCRISPDefinition \"OPARENTHESIS\" exit");
    GetNextToken (tokens); // move past `(`

    if (tokens[0].type != CPARENTHESIS){
        ProcessCompilerError (tokens[0].sourceLineNumber,
                              tokens[0].sourceLineIndex, "Expecting ')'");
    }
    // DebugPrintToken (tokens[0], "ParseMAINCRISPDefinition \"CPARENTHESIS\" exit");
    GetNextToken (tokens); // move past `)`

    // Check for opening curly bracket `{` 
    if (tokens[0].type != OCBRACKET)
        {
            ProcessCompilerError (
                tokens[0].sourceLineNumber, tokens[0].sourceLineIndex,
                "Expecting opening bracket for MAINCRISP function definition");
            return; // Exit if opening bracket is missing
        }
    // DebugPrintToken (tokens[0], "ParseMAINCRISPDefinition \"OCBRACKET\" exit");
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
            // DebugPrintToken (tokens[0], "ParseMAINCRISPDefinition \"CCBRACKET\" exit");
        }
        
            // CODEGENERATION
            // RETURN for non-empty body
            code.EmitFormattedLine ("", "RETURN");
            code.EmitUnformattedLine ("; **** =========");
            sprintf (line, "; **** END (%4d)", tokens[0].sourceLineNumber);
            code.EmitUnformattedLine (line);
            code.EmitUnformattedLine ("; **** =========");
            // ENDCODEGENERATION

    // DebugPrintToken (tokens[0], "ParseMAINCRISPDefinition exit");

    ExitModule ("MAINCRISPDefinition");
}

//-----------------------------------------------------------
void ParseStatement (TOKEN tokens[])
//-----------------------------------------------------------
{
    void ParsePRINTStatement (TOKEN tokens[]);
    void GetNextToken (TOKEN tokens[]);

    EnterModule ("Statement");
    // DebugPrintToken (tokens[0], "ParseStatement");

    switch (tokens[0].type)
        {
        case PRINT:
            ParsePRINTStatement (tokens);
            break;
        default:
            ProcessCompilerError (tokens[0].sourceLineNumber, tokens[0].sourceLineIndex,
                                  "Expecting beginning-of-statement");
            break;
        }
    // DebugPrintToken (tokens[0], "ParseStatement \"switch\" exit");

    ExitModule ("Statement");
}

//-----------------------------------------------------------
void ParsePRINTStatement (TOKEN tokens[])
//-----------------------------------------------------------
{
    void GetNextToken (TOKEN tokens[]);

    char line[SOURCELINELENGTH + 1];

    EnterModule ("PRINTStatement");

    // CODEGENERATION
    sprintf (line, "; **** PRINT statement (%4d)", tokens[0].sourceLineNumber);
    code.EmitUnformattedLine (line);
    // ENDCODEGENERATION

    // DebugPrintToken (tokens[0], "ParsePRINTStatement");
    GetNextToken (tokens);

    // DebugPrintToken (tokens[0], "ParsePRINTStatement \"dowhile\"");
    do
        {
            // DebugPrintToken (tokens[0], "ParsePRINTStatement \"switch\"");
            switch (tokens[0].type)
                {
                case STRING:
                    /// CODEGENERATION
                    char reference[SOURCELINELENGTH + 1];
                    code.AddDSToStaticData(tokens[0].lexeme, "", reference);
                    code.EmitFormattedLine("", "PUSHA", reference);
                    code.EmitFormattedLine("", "SVC", "#SVC_WRITE_STRING"); 

                    GetNextToken(tokens); // Move to the next token

                    // Handle additional CONCAT tokens in sequence
                    while (tokens[0].type == CONCAT)
                    {
                        GetNextToken(tokens); // Move to token after CONCAT

                        if (tokens[0].type == STRING)
                        {
                            // CODEGENERATION
                            code.AddDSToStaticData(tokens[0].lexeme, "", reference);
                            code.EmitFormattedLine("", "PUSHA", reference); 
                            code.EmitFormattedLine("", "SVC", "#SVC_WRITE_STRING");
                            // ENDCODEGENERATION
                            
                            GetNextToken(tokens); // Move to the next token
                        }
                        else if (tokens[0].type == IDENTIFIER)
                        {
                            // CODEGENERATION
                            code.AddDSToStaticData(tokens[0].lexeme, "", reference);
                            code.EmitFormattedLine("", "PUSHA", reference); 
                            code.EmitFormattedLine("", "SVC", "#SVC_WRITE_STRING");
                            // ENDCODEGENERATION
                            
                            GetNextToken(tokens); // Move to the next token
                        }
                        else
                        {
                            // Handle error or unexpected token
                            ProcessCompilerError(tokens[0].sourceLineNumber, tokens[0].sourceLineIndex,
                                                 "Expecting string or identifier after CONCAT");
                        }
                    }
                    break;
                case IDENTIFIER: // treated as a variable not a string literal
                    // DebugPrintToken (tokens[0],"ParsePRINTStatement \"identifier\" exit");
                    GetNextToken (tokens); // Move to the next token
                    break;
                case ENDL:

                    // CODEGENERATION
                    code.EmitFormattedLine ("", "SVC", "#SVC_WRITE_ENDL");
                    // ENDCODEGENERATION

                    // DebugPrintToken (tokens[0], "ParsePRINTStatement \"endl\" exit");
                    GetNextToken (tokens); // Move to the next token, if any
                    break;
                default:
                    ProcessCompilerError (tokens[0].sourceLineNumber, tokens[0].sourceLineIndex,
                                          "Expecting string or ENDL");
                }

            // DebugPrintToken (tokens[0], "ParsePRINTStatement \"switch\" exit");
        }
    while (tokens[0].type == STRING || tokens[0].type == IDENTIFIER || tokens[0].type == ENDL);

    // DebugPrintToken (tokens[0], "ParsePRINTStatement \"dowhile\" exit");
    // DebugPrintToken (tokens[0], "ParsePRINTStatement exit");
    ExitModule ("PRINTStatement");
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
            if ((nextCharacter == '|')
                && (reader.GetLookAheadCharacter (1).character == '['))
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
                            "Unexpected end-of-program");
                }
            /* WHILE ( (nextCharacter is not a white-space character)
               or (nextCharacter is not beginning-of-comment character) ) */
        }
    while ((nextCharacter == ' ')
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
        while (isalpha (nextCharacter) || isdigit (nextCharacter) || (nextCharacter == '_') || (nextCharacter == '_'))
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
                                    if ((nextCharacter == 'n')
                                        || (nextCharacter == 't')
                                        || (nextCharacter == 'b')
                                        || (nextCharacter == 'r')
                                        || (nextCharacter == '\\')
                                        || (nextCharacter == ',')
                                        || (nextCharacter == '"'))
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
                case '{':
                    type = OCBRACKET;
                    lexeme[0] = nextCharacter;
                    lexeme[1] = '\0';
                    reader.GetNextCharacter ();
                    break;
                case '}':
                    type = CCBRACKET;
                    lexeme[0] = nextCharacter;
                    lexeme[1] = '\0';
                    reader.GetNextCharacter ();
                    break;
                case '(':
                    type = OPARENTHESIS;
                    lexeme[0] = nextCharacter;
                    lexeme[1] = '\0';
                    reader.GetNextCharacter ();
                    break;
                case ')':
                    type = CPARENTHESIS;
                    lexeme[0] = nextCharacter;
                    lexeme[1] = '\0';
                    reader.GetNextCharacter ();
                    break;
                case '+':
                    type = CONCAT;
                    lexeme[0] = nextCharacter;
                    lexeme[1] = '\0';
                    reader.GetNextCharacter ();
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
