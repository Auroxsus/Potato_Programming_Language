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
#define TRACECOMPILER

#include "PotatoChip.h"

//-----------------------------------------------------------
typedef enum
//-----------------------------------------------------------
{
    // pseudo-terminals
    IDENTIFIER,
    INTEGER,   // SPUD
    STRING,    // CHIPPER
    EOPTOKEN,
    UNKTOKEN,
    // reserved words
    CRISP,
    PROGRAM,   // mainCrisp
    PRINT,     // bite
    ENDL,      // endl and \n
    // logical operators
    OR,        // \\ and mix
    NOR,
    XOR,
    AND,       // && and prep
    NAND,
    NOT,       // ! and raw
    ABS,
    TRUE,
    FALSE,
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
    // relational operators
    LT,       // < and smallerSpud
    LTEQ,     // <= notBiggerSpud
    EQ,       // == and spudMatch
    GT,       // > and biggerSpud
    GTEQ,     // >= and notSmallerSpud
    NOTEQ,    // <>, !=, and mashApart
    // arithmetic operators
    PLUS,     // + and mash
    MINUS,    // - and peel
    MULTIPLY, // * and fry
    DIVIDE,   // / and slice
    MODULUS,
    POWER     // ^ and **
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
    { INTEGER,        "SPUD",              true  },
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
    { COMMA,          "COMMA",             false },
    { PERIOD,         "PERIOD",            false },
    { SEMICOLON,      "SEMICOLON",         false },
    { OCBRACKET,      "OCBRACKET",         false },
    { CCBRACKET,      "CCBRACKET",         false },
    { OSQRBRACKET,    "OSQRBRACKET",       false },
    { CSQRBRACKET,    "CSQRBRACKET",       false },
    { OPARENTHESIS,   "OPARENTHESIS",      false },
    { CPARENTHESIS,   "CPARENTHESIS",      false },
    { LT,             "SMALLERSPUD",       true  },
    { LTEQ,           "NOTBIGGERSPUD",     true  },
    { EQ,             "SPUDMATCH",         true  },
    { GT,             "BIGGERSPUD",        true  },
    { GTEQ,           "NOTSMALLERSPUD",    true  },
    { NOTEQ,          "MASHAPART",         true  },
    { PLUS,           "MASH",              true  },
    { MINUS,          "PEEL",              true  },
    { MULTIPLY,       "FRY",               true  },
    { DIVIDE,         "SLICE",             true  },
    { MODULUS,        "MOD",               true  },    
    { POWER,          "POW",               true  },
    { CONCAT,         "CONCAT",            false }
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
    void ParseMAINDefinition (TOKEN tokens[]);
    void GetNextToken (TOKEN tokens[]);

    EnterModule ("POTATOProgram");

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

    if (tokens[0].type != EOPTOKEN)
        ProcessCompilerError (tokens[0].sourceLineNumber, tokens[0].sourceLineIndex,
                              "Expecting end-of-program");

    ExitModule ("POTATOProgram");
}

//-----------------------------------------------------------
void ParseMAINDefinition (TOKEN tokens[])
//-----------------------------------------------------------
{
    void ParseStatement (TOKEN tokens[]);
    void GetNextToken (TOKEN tokens[]);

    char line[SOURCELINELENGTH + 1];
    char label[SOURCELINELENGTH + 1];
    char reference[SOURCELINELENGTH + 1];

    EnterModule ("PROGRAMDefinition");
    
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

    DebugPrintToken (tokens[0], "ParseMAINCRISPDefinition");
    GetNextToken (tokens); // Move past 'crispMAIN'

    // Check for empty parameter list `()`
    if (tokens[0].type != OPARENTHESIS) {
        ProcessCompilerError (tokens[0].sourceLineNumber,
                              tokens[0].sourceLineIndex, "Expecting '('");
    }
    DebugPrintToken (tokens[0], "ParseMAINCRISPDefinition \"OPARENTHESIS\" exit");
    GetNextToken (tokens); // move past `(`

    if (tokens[0].type != CPARENTHESIS){
        ProcessCompilerError (tokens[0].sourceLineNumber,
                              tokens[0].sourceLineIndex, "Expecting ')'");
    }
    DebugPrintToken (tokens[0], "ParseMAINCRISPDefinition \"CPARENTHESIS\" exit");
    GetNextToken (tokens); // move past `)`
    
    // Check for opening curly bracket `{` 
    if (tokens[0].type != OCBRACKET)
        {
            ProcessCompilerError (
                tokens[0].sourceLineNumber, tokens[0].sourceLineIndex,
                "Expecting opening bracket for MAINCRISP function definition");
            return; // Exit if opening bracket is missing
        }
    DebugPrintToken (tokens[0], "ParseMAINCRISPDefinition \"OCBRACKET\" exit");
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
            DebugPrintToken (tokens[0], "ParseMAINCRISPDefinition \"CCBRACKET\" exit");
        }
        
            // RETURN for body
            // CODEGENERATION
            code.EmitFormattedLine ("", "RETURN");
            code.EmitUnformattedLine ("; **** =========");
            sprintf (line, "; **** END (%4d)", tokens[0].sourceLineNumber);
            code.EmitUnformattedLine (line);
            code.EmitUnformattedLine ("; **** =========");
            // ENDCODEGENERATION

    DebugPrintToken (tokens[0], "ParseMAINCRISPDefinition exit");

    ExitModule ("PROGRAMDefinition");
}

//-----------------------------------------------------------
void ParseStatement (TOKEN tokens[])
//-----------------------------------------------------------
{
    void ParsePRINTStatement (TOKEN tokens[]);
    void GetNextToken (TOKEN tokens[]);

    EnterModule ("Statement");
    DebugPrintToken(tokens[0], "ParseStatement");

    switch (tokens[0].type)
    {
        case PRINT:
            //GetNextToken(tokens); // move past bite
            ParsePRINTStatement (tokens);
            break;
        default:
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
    DebugPrintToken(tokens[0], "ParsePRINTStatement entry");

    // CODEGENERATION
    sprintf (line, "; **** PRINT statement (%4d)", tokens[0].sourceLineNumber);
    code.EmitUnformattedLine (line);
    // ENDCODEGENERATION

    do
    {
        DebugPrintToken(tokens[0], "ParsePRINTStatement switch entry");
        
      GetNextToken(tokens); // move past bite
        switch (tokens[0].type)
        {
            case STRING:
                DebugPrintToken(tokens[0], "ParsePRINTStatement STRING");
                // CODEGENERATION
                char reference[SOURCELINELENGTH+1];
                code.AddDSToStaticData(tokens[0].lexeme, "", reference);
                code.EmitFormattedLine("", "PUSHA", reference);
                code.EmitFormattedLine("", "SVC", "#SVC_WRITE_STRING");
                // ENDCODEGENERATION
                GetNextToken(tokens);
                break;
            case ENDL:
                DebugPrintToken(tokens[0], "ParsePRINTStatement ENDL");
                // CODEGENERATION
                code.EmitFormattedLine("", "SVC", "#SVC_WRITE_ENDL");
                // ENDCODEGENERATION
                GetNextToken(tokens);
                break;

            default:
            {
                DebugPrintToken(tokens[0], "ParsePRINTStatement default (expression)");
                ParseExpression(tokens, datatype);
                DebugPrintToken(tokens[0], "ParsePRINTStatement after ParseExpression");

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
                           "Expecting ';'");

    DebugPrintToken(tokens[0], "ParsePRINTStatement end of do-while loop");
    GetNextToken(tokens);
    ExitModule ("PRINTStatement");
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
   DebugPrintToken(tokens[0], "ParseExpression entry");

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
   DebugPrintToken(tokens[0], "ParseExpression exit");
}

//-----------------------------------------------------------
void ParseConjunction(TOKEN tokens[],DATATYPE &datatype)
//-----------------------------------------------------------
{
   void ParseNegation(TOKEN tokens[],DATATYPE &datatype);
   void GetNextToken(TOKEN tokens[]);

   DATATYPE datatypeLHS,datatypeRHS;

   EnterModule("Conjunction");
   DebugPrintToken(tokens[0], "ParseConjunction entry");

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
   DebugPrintToken(tokens[0], "ParseConjunction exit");
}

//-----------------------------------------------------------
void ParseNegation(TOKEN tokens[],DATATYPE &datatype)
//-----------------------------------------------------------
{
   void ParseComparison(TOKEN tokens[],DATATYPE &datatype);
   void GetNextToken(TOKEN tokens[]);

   DATATYPE datatypeRHS;

   EnterModule("Negation");
   DebugPrintToken(tokens[0], "ParseNegation entry");

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
   DebugPrintToken(tokens[0], "ParseNegation exit");
}

//-----------------------------------------------------------
void ParseComparison(TOKEN tokens[],DATATYPE &datatype)
//-----------------------------------------------------------
{
   void ParseComparator(TOKEN tokens[],DATATYPE &datatype);
   void GetNextToken(TOKEN tokens[]);

   DATATYPE datatypeLHS,datatypeRHS;

   EnterModule("Comparison");
   DebugPrintToken(tokens[0], "ParseComparison entry");

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
      datatype = BOOLTYPE;
      code.EmitFormattedLine("","PUSH","#0X0000");
      code.EmitFormattedLine("","JMP",Elabel);
      code.EmitFormattedLine(Tlabel,"PUSH","#0XFFFF");
      code.EmitFormattedLine(Elabel,"EQU","*");
   }
   else
      datatype = datatypeLHS;

   ExitModule("Comparison");
   DebugPrintToken(tokens[0], "ParseComparison exit");
}

//-----------------------------------------------------------
void ParseComparator(TOKEN tokens[],DATATYPE &datatype)
//-----------------------------------------------------------
{
   void ParseTerm(TOKEN tokens[],DATATYPE &datatype);
   void GetNextToken(TOKEN tokens[]);

   DATATYPE datatypeLHS,datatypeRHS;

   EnterModule("Comparator");
   DebugPrintToken(tokens[0], "ParseComparator entry");

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
   DebugPrintToken(tokens[0], "ParseComparator exit");
}

//-----------------------------------------------------------
void ParseTerm(TOKEN tokens[],DATATYPE &datatype)
//-----------------------------------------------------------
{
   void ParseFactor(TOKEN tokens[],DATATYPE &datatype);
   void GetNextToken(TOKEN tokens[]);

   DATATYPE datatypeLHS,datatypeRHS;

   EnterModule("Term");
   DebugPrintToken(tokens[0], "ParseTerm entry");

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
   DebugPrintToken(tokens[0], "ParseTerm exit");
}

//-----------------------------------------------------------
void ParseFactor(TOKEN tokens[],DATATYPE &datatype)
//-----------------------------------------------------------
{
   void ParseSecondary(TOKEN tokens[],DATATYPE &datatype);
   void GetNextToken(TOKEN tokens[]);

   EnterModule("Factor");
   DebugPrintToken(tokens[0], "ParseFactor entry");

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
            DebugPrintToken(tokens[0], "ParseFactor \"plus\"");
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
   DebugPrintToken(tokens[0], "ParseFactor exit");
}

//-----------------------------------------------------------
void ParseSecondary(TOKEN tokens[],DATATYPE &datatype)
//-----------------------------------------------------------
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

      if ( (datatypeLHS != INTTYPE) || (datatypeRHS != INTTYPE) )
         ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting integer operands");

      code.EmitFormattedLine("","POWI");
      datatype = INTTYPE;
   }
   else
      datatype = datatypeLHS;

   ExitModule("Secondary");
}

//-----------------------------------------------------------
void ParsePrimary(TOKEN tokens[],DATATYPE &datatype)
//-----------------------------------------------------------
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
      default:
         ProcessCompilerError(tokens[0].sourceLineNumber,tokens[0].sourceLineIndex,"Expecting integer, true, false, or (");
         break;
   }

   ExitModule("Primary");
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
                       type = EQ;
                    }
                    else
                    {
                       type = UNKTOKEN;
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
                    } 
                    else {
                        type = PLUS;
                        lexeme[1] = '\0';
                        reader.GetNextCharacter(); 
                    }
                    break;
                 case '-': 
                    type = MINUS;
                    lexeme[0] = nextCharacter; lexeme[1] = '\0';
                    reader.GetNextCharacter();
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
