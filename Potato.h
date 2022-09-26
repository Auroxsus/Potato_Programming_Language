//-----------------------------------------------------------
// Dr. Art Hanna
// POTATO compiler "global" definitions and the common classes
//    POTATOEXCEPTION, LISTER, READER, CODE, and IDENTIFIERTABLE
//
// *Note* Several common classes are commented to indicate their
//    required evolution to support incremental development of the 
//    POTATO compiler
//
// POTATO.h
//-----------------------------------------------------------

#define CALLBACKSUSED 2

const int SOURCELINELENGTH        = 512;
const int LOOKAHEAD               =   2;
const int LINESPERPAGE            =  60;
const int MAXIMUMLENGTHIDENTIFIER =  64;
const int MAXIMUMIDENTIFIERS      = 500;

enum DATATYPE { NOTYPE,INTEGERTYPE,BOOLEANTYPE };

enum IDENTIFIERSCOPE { GLOBALSCOPE,PROGRAMMODULESCOPE,SUBPROGRAMMODULESCOPE };

enum IDENTIFIERTYPE
{
   GLOBAL_VARIABLE,           // static global variable data
   GLOBAL_CONSTANT,           // static global constant data
   PROGRAMMODULE_VARIABLE,    // static PROGRAM module local variable data
   PROGRAMMODULE_CONSTANT,    // static PROGRAM module local constant data
//--------------------------------------------------
// ADDED FOR POTATO6
//--------------------------------------------------
   SUBPROGRAMMODULE_VARIABLE, // automatic, frame-based data
   SUBPROGRAMMODULE_CONSTANT, // automatic, frame-based data
   IN_PARAMETER,              // automatic, frame-based data
   OUT_PARAMETER,             // automatic, frame-based data
   IO_PARAMETER,              // automatic, frame-based data
   REF_PARAMETER,             // automatic, frame-based data
   PROCEDURE_SUBPROGRAMMODULE,// module name
//-----------------------------------------------------------
// ADDED FOR POTATO7
//-----------------------------------------------------------
   FUNCTION_SUBPROGRAMMODULE  // module name
};

//===========================================================
class POTATOEXCEPTION
//===========================================================
{
private:
   char description[80+1];

public:
   //-----------------------------------------------------------
   POTATOEXCEPTION(const char description[])
   //-----------------------------------------------------------
   {
      strcpy(this->description,description);
   }
   //-----------------------------------------------------------
   char *GetDescription()
   //-----------------------------------------------------------
   { 
      return(description); 
   }
};

//===========================================================
class LISTER
//===========================================================
{
private:
   const int LINESPERPAGE;

   ofstream LIST;
   int pageNumber;
   int linesOnPage;
   char sourceFileName[80+1];

public:
   LISTER(const int LINESPERPAGE = 55);
   ~LISTER();
   void OpenFile(const char sourceFileName[]);
   void ListSourceLine(int sourceLineNumber,const char sourceLine[]);
   void ListInformationLine(const char information[]);

private:
   void ListTopOfPageHeader();
};

//-----------------------------------------------------------
LISTER::LISTER(const int LINESPERPAGE): LINESPERPAGE(LINESPERPAGE)
//-----------------------------------------------------------
{
   pageNumber = 0;
   linesOnPage = 0;
}

//-----------------------------------------------------------
LISTER::~LISTER()
//-----------------------------------------------------------
{
   if ( LIST.is_open() ) 
   {
      LIST.flush();
      LIST.close();
   }
}

//-----------------------------------------------------------
void LISTER::OpenFile(const char sourceFileName[])
//-----------------------------------------------------------
{
   char fullFileName[80+1];

   strcpy(this->sourceFileName,sourceFileName);
   strcat(this->sourceFileName,".POTATO");
   strcpy(fullFileName,sourceFileName);
   strcat(fullFileName,".list");
   LIST.open(fullFileName,ios::out);
   if ( !LIST.is_open() ) throw( POTATOEXCEPTION("Unable to open list file") );
   ListTopOfPageHeader();
}

//-----------------------------------------------------------
void LISTER::ListSourceLine(int sourceLineNumber,const char sourceLine[])
//-----------------------------------------------------------
{
   if ( linesOnPage >= LINESPERPAGE )
   {
      ListTopOfPageHeader();
      linesOnPage = 0;
   }
   LIST << setw(4) << sourceLineNumber << " " << sourceLine << endl;
   linesOnPage++;
}

//-----------------------------------------------------------
void LISTER::ListInformationLine(const char information[])
//-----------------------------------------------------------
{
   if ( linesOnPage >= LINESPERPAGE )
   {
      ListTopOfPageHeader();
      linesOnPage = 0;
   }
   LIST << information << endl;
   linesOnPage++;
}

//-----------------------------------------------------------
void LISTER::ListTopOfPageHeader()
//-----------------------------------------------------------
{
/*
"Source file name" Page XXXX
Line Source Line
---- -------------------------------------------------------------------------------
*/
   const char FF = 0X0C;

   pageNumber++;
   LIST << FF << '"' << sourceFileName << "\" Page " << setw(4) << pageNumber << endl;
   LIST << "Line Source Line" << endl;
   LIST << "---- -------------------------------------------------------------------------------" << endl;
}

//===========================================================
struct NEXTCHARACTER
//===========================================================
{
   char character;
   int sourceLineNumber;
   int sourceLineIndex;
};

//===========================================================
template <int CALLBACKSALLOWED = 5>
class READER
//===========================================================
{
public:
   static const char EOPC = 0;
   static const char EOLC = '\n';
   static const char TABC = '\t';

private:
   const int SOURCELINELENGTH;
   const int LOOKAHEAD;

   char *sourceLine;
   int sourceLineNumber;
   int sourceLineIndex;
   NEXTCHARACTER *nextCharacters;
   ifstream SOURCE;
   LISTER *lister;
   bool atEOP;
   int numberCallbacks;
   void (*CallbackFunctions[CALLBACKSALLOWED+1])
      (int sourceLineNumber,const char sourceLine[]);

public:
   READER(const int SOURCELINELENGTH = 512,const int LOOKAHEAD = 0);
   ~READER();
   void OpenFile(const char sourceFileName[]);
   void SetLister(LISTER *lister);
   NEXTCHARACTER GetNextCharacter();
   NEXTCHARACTER GetLookAheadCharacter(int index);
   void AddCallbackFunction(void (*CallbackFunction)
      (int sourceLineNumber,const char sourceLine[]));
private:
   void ReadSourceLine();
};

//-----------------------------------------------------------
template<int CALLBACKSALLOWED>
READER<CALLBACKSALLOWED>::READER(const int SOURCELINELENGTH,const int LOOKAHEAD): 
   SOURCELINELENGTH(SOURCELINELENGTH),LOOKAHEAD(LOOKAHEAD)
//-----------------------------------------------------------
{
   sourceLine = new char [ SOURCELINELENGTH+2 ];
   nextCharacters = new NEXTCHARACTER [ LOOKAHEAD+1 ];
   sourceLineNumber = 0;
   atEOP = false;
   numberCallbacks = 0;
}

//-----------------------------------------------------------
template<int CALLBACKSALLOWED>
READER<CALLBACKSALLOWED>::~READER()
//-----------------------------------------------------------
{
   delete [] sourceLine;
   delete [] nextCharacters;
   if ( SOURCE.is_open() ) SOURCE.close();
}

//-----------------------------------------------------------
template<int CALLBACKSALLOWED>
void READER<CALLBACKSALLOWED>::OpenFile(const char sourceFileName[])
//-----------------------------------------------------------
{
   char fullFileName[80+1];

   strcpy(fullFileName,sourceFileName);
   strcat(fullFileName,".POTATO");
   SOURCE.open(fullFileName,ios::in);
   if ( !SOURCE.is_open() ) throw( POTATOEXCEPTION("Unable to open source file") );

// Read first source line and "fill" nextCharacters[] 
   ReadSourceLine();
   for (int i = 0; i <= LOOKAHEAD; i++)
   {
      nextCharacters[i].character = READER::EOPC;
      nextCharacters[i].sourceLineNumber = 0;
      nextCharacters[i].sourceLineIndex = 0;
   }
   for (int i = 0; i <= LOOKAHEAD; i++)
      GetNextCharacter();
}

//-----------------------------------------------------------
template<int CALLBACKSALLOWED>
void READER<CALLBACKSALLOWED>::SetLister(LISTER *lister)
//-----------------------------------------------------------
{
   this->lister = lister;
}

//-----------------------------------------------------------
template<int CALLBACKSALLOWED>
NEXTCHARACTER READER<CALLBACKSALLOWED>::GetNextCharacter()
//-----------------------------------------------------------
{
   char character;

// Move look-ahead "window" to make room for next character
   for (int i = 1; i <= LOOKAHEAD; i++)
      nextCharacters[i-1] = nextCharacters[i];

   nextCharacters[LOOKAHEAD].sourceLineNumber = sourceLineNumber;
   nextCharacters[LOOKAHEAD].sourceLineIndex = sourceLineIndex;

   if ( atEOP )
      character = READER::EOPC;
//      character = EOPC;
   else
   { 
      if ( sourceLineIndex <= ((int) strlen(sourceLine)-1) )
      {
         character = sourceLine[sourceLineIndex];
         sourceLineIndex += 1;
      }
      else
      {
         character = READER::EOLC;
         ReadSourceLine();
      }
   }

// Only non-printable characters allowed are EOPC,'\n', and '\t', others are changed to ' '
   if ( iscntrl(character) 
    && !(    (character == READER::EOPC)
          || (character == READER::EOLC) 
          || (character == READER::TABC) 
        )
      )
      character = ' ';

   nextCharacters[LOOKAHEAD].character = character;

#ifdef TRACEREADER
{
   char information[80+1];

   if      ( isprint(character) ) 
      sprintf(information,"At (%4d:%3d) %02X = %c",
         nextCharacters[LOOKAHEAD].sourceLineNumber,
         nextCharacters[LOOKAHEAD].sourceLineIndex,
         character,character);
   else if ( character == READER::EOPC )
      sprintf(information,"At (%4d:%3d) %02X = EOPC",
         nextCharacters[LOOKAHEAD].sourceLineNumber,
         nextCharacters[LOOKAHEAD].sourceLineIndex,
         character);
   else if ( character == READER::EOLC )
      sprintf(information,"At (%4d:%3d) %02X = EOLC",
         nextCharacters[LOOKAHEAD].sourceLineNumber,
         nextCharacters[LOOKAHEAD].sourceLineIndex,
         character);
   else if ( character == READER::TABC )
      sprintf(information,"At (%4d:%3d) %02X = TABC",
         nextCharacters[LOOKAHEAD].sourceLineNumber,
         nextCharacters[LOOKAHEAD].sourceLineIndex,
         character);
   else
      sprintf(information,"At (%4d:%3d) %02X = ???",
         nextCharacters[LOOKAHEAD].sourceLineNumber,
         nextCharacters[LOOKAHEAD].sourceLineIndex,
         character);
   lister->ListInformationLine(information);
}
#endif

   return( nextCharacters[0] );
}

//-----------------------------------------------------------
template<int CALLBACKSALLOWED>
NEXTCHARACTER READER<CALLBACKSALLOWED>::GetLookAheadCharacter(int index)
//-----------------------------------------------------------
{
// index in [ 0,LOOKAHEAD ] where index = 0 means last GetNextCharacter() returned
   if ( (0 <= index) && (index <= LOOKAHEAD) )
      return( nextCharacters[index] );
   else
      throw( POTATOEXCEPTION("GetLookAheadCharacter() index out-of-range") );
}

//-----------------------------------------------------------
template<int CALLBACKSALLOWED>
void READER<CALLBACKSALLOWED>::AddCallbackFunction(
   void (*CallbackFunction)(int sourceLineNumber,const char sourceLine[]))
//-----------------------------------------------------------
{
   if ( numberCallbacks <= CALLBACKSALLOWED )
      CallbackFunctions[++numberCallbacks] = CallbackFunction;
   else
      throw( POTATOEXCEPTION("Too many callback functions") );
}

//-----------------------------------------------------------
template<int CALLBACKSALLOWED>
void READER<CALLBACKSALLOWED>::ReadSourceLine()
//-----------------------------------------------------------
{
   if ( SOURCE.eof() )
      atEOP = true;
   else
   {
      SOURCE.getline(sourceLine,SOURCELINELENGTH+2);
      sourceLineNumber++;
      strcat(sourceLine,"\n");
      if ( SOURCE.fail() && !SOURCE.eof() )
      {
         lister->ListInformationLine("******* Source line too long!");
         SOURCE.clear();
      }
   // Erase *ALL* control characters at end of source line (if any)
      while ( (0 <= (int) strlen(sourceLine)-1) && 
              iscntrl(sourceLine[(int) strlen(sourceLine)-1]) )
         sourceLine[(int) strlen(sourceLine)-1] = '\0';
      sourceLineIndex = 0;

      lister->ListSourceLine(sourceLineNumber,sourceLine);

   // Give each callback function the opportunity to process newly-read source line
      for (int i = 1; i <= numberCallbacks; i++)
         (*CallbackFunctions[i])(sourceLineNumber,sourceLine);
   }
}

//===========================================================
class IDENTIFIERTABLE
//===========================================================
{
private:
   struct IDENTIFIERRECORD
   {
      int scope;
      char lexeme[MAXIMUMLENGTHIDENTIFIER+1];
      IDENTIFIERTYPE identifierType;
      char reference[MAXIMUMLENGTHIDENTIFIER+1];
      DATATYPE datatype;
//--------------------------------------------------
// ADDED FOR POTATO8
//--------------------------------------------------
      int dimensions;
   };

private:
   static const char IDENTIFIERTYPENAMES[][26+1];
   static const char DATATYPENAMES[][9+1];

private:
   int capacity;
   int identifiers;
   IDENTIFIERRECORD *identifierTable;
   int scopes;
   int *scopeTable;
   LISTER *lister;

public:
   IDENTIFIERTABLE(LISTER *lister,int capacity);
   ~IDENTIFIERTABLE();
   int GetIndex(const char lexeme[],bool &isInTable);
//--------------------------------------------------
// MODIFIED FOR POTATO8
//--------------------------------------------------
   void AddToTable(const char lexeme[],IDENTIFIERTYPE identifierType,
                   DATATYPE datatype,const char reference[],int dimensions = 0);
   void EnterNestedStaticScope();
   void ExitNestedStaticScope();
   void DiPOTATOayTableContents(const char description[]);

/*
   Assume index was determined by a prior successful call to GetIndex() as
      precondition of the remaining accessor member functions.
*/
   bool IsInCurrentScope(int index)
   {
      return( identifierTable[index].scope == scopes );
   }
   int GetScope(int index) 
   { 
      return( identifierTable[index].scope );
   }
   IDENTIFIERTYPE GetType(int index)
   { 
      return( identifierTable[index].identifierType );
   }
   char *GetLexeme(int index)
   { 
      return( identifierTable[index].lexeme );
   }
   char *GetReference(int index)
   { 
      return( identifierTable[index].reference );
   }
   DATATYPE GetDatatype(int index)
   { 
      return( identifierTable[index].datatype );
   }
//--------------------------------------------------
// ADDED FOR POTATO6
//--------------------------------------------------
   int GetCountOfFormalParameters(int index);
//--------------------------------------------------
// ADDED FOR POTATO8
//--------------------------------------------------
   int GetDimensions(int index)
   {
      return( identifierTable[index].dimensions );
   }
};

//-----------------------------------------------------------
const char IDENTIFIERTABLE::IDENTIFIERTYPENAMES[][26+1] =
//-----------------------------------------------------------
{
   "GLOBAL_VARIABLE",
   "GLOBAL_CONSTANT",
   "PROGRAMMODULE_VARIABLE",
   "PROGRAMMODULE_CONSTANT",
//--------------------------------------------------
// ADDED FOR POTATO6
//--------------------------------------------------
   "SUBPROGRAMMODULE_VARIABLE",
   "SUBPROGRAMMODULE_CONSTANT",
   "IN_PARAMETER",
   "OUT_PARAMETER",
   "IO_PARAMETER",
   "REF_PARAMETER",
   "PROCEDURE_SUBPROGRAMMODULE",
//-----------------------------------------------------------
// ADDED FOR POTATO7
//-----------------------------------------------------------
   "FUNCTION_SUBPROGRAMMODULE"
};

//-----------------------------------------------------------
const char IDENTIFIERTABLE::DATATYPENAMES[][9+1] =
//-----------------------------------------------------------
{
   "NOTYPE",
   "INTEGER",
   "BOOLEAN"
};

//-----------------------------------------------------------
IDENTIFIERTABLE::IDENTIFIERTABLE(LISTER *lister,int capacity)
//-----------------------------------------------------------
{
   this->lister = lister;
   this->capacity = capacity;
   identifierTable = new IDENTIFIERRECORD [ capacity+1 ];
   identifiers = 0;
   scopeTable = new int [ capacity+1 ];
   scopes = 0;
}

//-----------------------------------------------------------
IDENTIFIERTABLE::~IDENTIFIERTABLE()
//-----------------------------------------------------------
{
   delete [] identifierTable;
   delete [] scopeTable;
}

//-----------------------------------------------------------
int IDENTIFIERTABLE::GetIndex(const char lexeme[],bool &isInTable)
//-----------------------------------------------------------
{
/*
   Try to find identifier's lexeme in identifier table working from the end of
      the table toward the beginning.
*/
   char UCLexeme1[SOURCELINELENGTH+1],UCLexeme2[SOURCELINELENGTH];
   int index;
   bool isInCurrentScope;

   for (int i = 0; i <= (int) strlen(lexeme); i++)
      UCLexeme1[i] = toupper(lexeme[i]);
   isInTable = false;
   index = identifiers;
   while ( (index >= 1) && !isInTable )
   {
      for (int i = 0; i <= (int) strlen(identifierTable[index].lexeme); i++)
         UCLexeme2[i] = toupper(identifierTable[index].lexeme[i]);
      if ( strcmp(UCLexeme1,UCLexeme2) == 0 )
      {
         isInTable = true;
         isInCurrentScope = (identifierTable[index].scope == scopes);
      }
      else
         index -= 1;
   }

#ifdef TRACEIDENTIFIERTABLE
{
   char information[SOURCELINELENGTH+1];

   if ( isInTable )
      sprintf(information,"Found identifier \"%s\" at index = %d (%s)",
         lexeme,index,((isInCurrentScope) ? "is in current scope" : "not in current scope"));
   else
      sprintf(information,"Did not find identifier \"%s\"",lexeme);
   lister->ListInformationLine(information);
}
#endif

   return( index );
}

//-----------------------------------------------------------
void IDENTIFIERTABLE::AddToTable(const char lexeme[],IDENTIFIERTYPE identifierType,
                                 DATATYPE datatype,const char reference[],int dimensions /* = 0*/)
//-----------------------------------------------------------
{
/*
   Assumes a prior reference to GetIndex() has already guaranteed that the
      identifier being added is *NOT* in the identifier table
*/
   if ( identifiers > capacity )
      throw( POTATOEXCEPTION("Identifier table capacity exceeded") );
   else
   {
      identifiers++;
      identifierTable[identifiers].scope = scopes;
      strncpy(identifierTable[identifiers].lexeme,lexeme,MAXIMUMLENGTHIDENTIFIER);
      identifierTable[identifiers].identifierType = identifierType;
      strcpy(identifierTable[identifiers].reference,reference);
      identifierTable[identifiers].datatype = datatype;
      identifierTable[identifiers].dimensions = dimensions;
   }

#ifdef TRACEIDENTIFIERTABLE
{
   char information[SOURCELINELENGTH+1];

   sprintf(information,"Added identifier \"%s\" at index = %d, reference = %s, identifier type = %s, data type = %s, dimensions = %d",
         lexeme,identifiers,identifierTable[identifiers].reference,
         IDENTIFIERTYPENAMES[identifierTable[identifiers].identifierType],
         DATATYPENAMES[identifierTable[identifiers].datatype],
         dimensions);
   lister->ListInformationLine(information);
}
#endif

}

//--------------------------------------------------
void IDENTIFIERTABLE::EnterNestedStaticScope()
//--------------------------------------------------
{
   scopeTable[++scopes] = identifiers;

#ifdef TRACEIDENTIFIERTABLE
{
   char information[SOURCELINELENGTH+1];

   sprintf(information,"Begin nested scope #%d, identifier table index = %d",
      scopes,identifiers);
   lister->ListInformationLine(information);
}
#endif

}

//--------------------------------------------------
// UPDATED FOR POTATO6, POTATO7
//--------------------------------------------------
void IDENTIFIERTABLE::ExitNestedStaticScope()
//--------------------------------------------------
{
/*
   Remove the identifiers of *ALL* local variables/constants in the module scope
      just ended *EXCEPT* for subprogram module formal parameters. Mark the formal
      parameters so they will not be "found" by subsequent searches. The formal
      parameters are retained so references to their module can be compiled.
      *Note* The subprogram module identifier is retained because it is in the
      scope just re-entered.
*/
   identifiers = scopeTable[scopes--];
   if ( (identifierTable[identifiers].identifierType == PROCEDURE_SUBPROGRAMMODULE) ||
        (identifierTable[identifiers].identifierType ==  FUNCTION_SUBPROGRAMMODULE) )
      while ( (identifierTable[identifiers+1].identifierType ==  IN_PARAMETER) ||
              (identifierTable[identifiers+1].identifierType == OUT_PARAMETER) ||
              (identifierTable[identifiers+1].identifierType ==  IO_PARAMETER) ||
              (identifierTable[identifiers+1].identifierType ==  REF_PARAMETER) )
      {
         identifiers++;
      /*
         A null-string lexeme ensures subprogram module formal parameters are 
            not found when out-of-scope, but still allows identifier type to 
            remain available for subprogram reference semantic analysis.
      */
         identifierTable[identifiers].lexeme[0] = '\0';
      }

#ifdef TRACEIDENTIFIERTABLE
{
   char information[SOURCELINELENGTH+1];

   sprintf(information,"End nested scope #%d, identifier table index now = %d",
      scopes+1,identifiers);
   lister->ListInformationLine(information);
}
#endif

}

//--------------------------------------------------
void IDENTIFIERTABLE::DiPOTATOayTableContents(const char description[])
//--------------------------------------------------
{
/*
===================================================================================================
description

  # Scope Data type Dimensions Type                       Reference            Lexeme
--- ----- --------- ---------- -------------------------- -------------------- --------------------
XXX   XXX XXXXXXXXX        XXX XXXXXXXXXXXXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXXXXXX XX....
  .     . .                    .                          .                    .
  .     . .                    .                          .                    .
===================================================================================================
*/
   lister->ListInformationLine
      ("===================================================================================================");
   lister->ListInformationLine(description);
   lister->ListInformationLine
      ("  # Scope Data type Dimensions Type                       Reference            Lexeme");
   lister->ListInformationLine
      ("--- ----- --------- ---------- -------------------------- -------------------- --------------------");
   for (int i = 1; i <= identifiers; i++)
   {
      char information[SOURCELINELENGTH+1];

      sprintf(information,"%3d   %3d %-9s        %3d %-26s %-20.20s %s",
         i,identifierTable[i].scope,
         ((identifierTable[i].datatype == NOTYPE) ? " " : DATATYPENAMES[identifierTable[i].datatype]),
         identifierTable[i].dimensions,
         IDENTIFIERTYPENAMES[identifierTable[i].identifierType],
         identifierTable[i].reference,
         identifierTable[i].lexeme);
      lister->ListInformationLine(information);
   }
   lister->ListInformationLine
      ("===================================================================================================");
}

//--------------------------------------------------
// ADDED FOR POTATO6
//--------------------------------------------------
int IDENTIFIERTABLE::GetCountOfFormalParameters(int index)
//--------------------------------------------------
{
/*
   Assumes compiler has verified that
      (1) index represents a subprogram module identifier; and 
      (2) all the subprogram module formal parameters immediately follow the
          subprogram module identifier in identifier table
*/
   int count;

   index++;
   count = 0;
   while ( (identifierTable[index].identifierType ==  IN_PARAMETER) ||
           (identifierTable[index].identifierType == OUT_PARAMETER) ||
           (identifierTable[index].identifierType ==  IO_PARAMETER) ||
           (identifierTable[index].identifierType == REF_PARAMETER) )
   {
      count++;
      index++;
   }

#ifdef TRACEIDENTIFIERTABLE
{
   char information[SOURCELINELENGTH+1];

   sprintf(information,"Subprogram module \"%s\" has %d formal parameters",
      identifierTable[index].lexeme,count);
   lister->ListInformationLine(information);
}
#endif

   return( count );
}

//===========================================================
class CODE
//===========================================================
{
/*
   POTATO static data--global variable/constant definitions, string literals, and 
      PROGRAM module variables/constants--are parsed both very early in the
      compile and near the end of the compile. As a result, their STM code must
      be "stored" because the code cannot be emitted until *AFTER* the entire
      POTATO program has been parsed.

   POTATO dynamic (frame-resident) data--subprogram module formal parameters and
      local variables/constants--have storage space accounted for when their
      definitions are parsed. The frame space is then allocated using STM 
      statements emitted as part of the subprogram module prolog code. 
*/
private:
   struct DATARECORD
   {
      char mnemonic[SOURCELINELENGTH+1];
      char operand[SOURCELINELENGTH+1];
      char comment[SOURCELINELENGTH+1];
   };

private:
   ofstream STM;
   char codeFileName[80+1];
   vector<DATARECORD> staticdata;
   int SBOffset;
   int labelsuffix;
//--------------------------------------------------
// ADDED FOR POTATO6
//--------------------------------------------------
   vector<DATARECORD> framedata;
   int FBOffset;
   bool isInModuleBody;
   int moduleIdentifierIndex;
   IDENTIFIERTYPE moduleIdentifierType;
//--------------------------------------------------

public:
   CODE();
   ~CODE();
   void OpenFile(const char sourceFileName[]);
   void EmitBeginningCode(const char sourceFileName[]);
   void EmitEndingCode();
   int GetSBOffset();
   void AddRWToStaticData(int operand,const char comment[],char reference[]);
   void AddDWToStaticData(const char operand[],const char comment[],char reference[]);
   void AddDSToStaticData(const char operand[],const char comment[],char reference[]);
   void EmitStaticData();
   int LabelSuffix();
   void EmitFormattedLine(const char label[],const char mnemonic[],const char operand[] = "",const char comment[] = "");
   void EmitUnformattedLine(const char line[]);
//--------------------------------------------------
// ADDED FOR POTATO6
//--------------------------------------------------
   void ResetFrameData();
   void IncrementFBOffset(int increment = 1);
   int GetFBOffset();
   void AddInstructionToInitializeFrameData(const char mnemonic[],const char operand[],const char comment[] = "");
   void EmitFrameData();
   void EnterModuleBody(IDENTIFIERTYPE moduleIdentifierType,int moduleIdentifierIndex);
   void ExitModuleBody();
   bool IsInModuleBody(IDENTIFIERTYPE moduleIdentifierType);
   int GetModuleIdentifierIndex();
//--------------------------------------------------
};

//-----------------------------------------------------------
CODE::CODE()
//-----------------------------------------------------------
{
   staticdata.clear();
   SBOffset = 0;
   labelsuffix = 0;
//--------------------------------------------------
// ADDED FOR POTATO6
//--------------------------------------------------
   isInModuleBody = false;
}

//-----------------------------------------------------------
CODE::~CODE()
//-----------------------------------------------------------
{
   if ( STM.is_open() )
   { 
      STM.flush();
      STM.close();
   }
}

//--------------------------------------------------
void CODE::OpenFile(const char sourceFileName[])
//--------------------------------------------------
{
   strcpy(codeFileName,sourceFileName);
   strcat(codeFileName,".stm");
   STM.open(codeFileName,ios::out);
   if ( !STM.is_open() ) throw( POTATOEXCEPTION("Unable to open code file") );
}

//--------------------------------------------------
void CODE::EmitBeginningCode(const char sourceFileName[])
//--------------------------------------------------
{
   char line[SOURCELINELENGTH+1];

   EmitUnformattedLine(";--------------------------------------------------------------");
   sprintf(line,"; %s.stm",sourceFileName); EmitUnformattedLine(line);
   EmitUnformattedLine(";--------------------------------------------------------------");
   EmitUnformattedLine("; SVC numbers");
   EmitFormattedLine("SVC_DONOTHING"       ,"EQU","0D0","force context switch");
   EmitFormattedLine("SVC_TERMINATE"       ,"EQU","0D1");
   EmitFormattedLine("SVC_READ_INTEGER"    ,"EQU","0D10");
   EmitFormattedLine("SVC_WRITE_INTEGER"   ,"EQU","0D11");
   EmitFormattedLine("SVC_READ_FLOAT"      ,"EQU","0D20");
   EmitFormattedLine("SVC_WRITE_FLOAT"     ,"EQU","0D21");
   EmitFormattedLine("SVC_READ_BOOLEAN"    ,"EQU","0D30");
   EmitFormattedLine("SVC_WRITE_BOOLEAN"   ,"EQU","0D31");
   EmitFormattedLine("SVC_READ_CHARACTER"  ,"EQU","0D40");
   EmitFormattedLine("SVC_WRITE_CHARACTER" ,"EQU","0D41");
   EmitFormattedLine("SVC_WRITE_ENDL"      ,"EQU","0D42");
   EmitFormattedLine("SVC_READ_STRING"     ,"EQU","0D50");
   EmitFormattedLine("SVC_WRITE_STRING"    ,"EQU","0D51");
   EmitFormattedLine("SVC_INITIALIZE_HEAP" ,"EQU","0D90");
   EmitFormattedLine("SVC_ALLOCATE_BLOCK"  ,"EQU","0D91");
   EmitFormattedLine("SVC_DEALLOCATE_BLOCK","EQU","0D92");
   EmitUnformattedLine("");
   EmitFormattedLine(""                   ,"ORG","0X0000");
   EmitUnformattedLine("");
   EmitFormattedLine(""                   ,"JMP","PROGRAMMAIN");
}

//--------------------------------------------------
void CODE::EmitEndingCode()
//--------------------------------------------------
{
   char reference[SOURCELINELENGTH+1];
   
   EmitUnformattedLine("");
   EmitUnformattedLine(";------------------------------------------------------------");
   EmitUnformattedLine("; Issue \"Run-time error #X..X near line #X..X\" to handle run-time errors");
   EmitUnformattedLine(";------------------------------------------------------------");
   EmitFormattedLine("HANDLERUNTIMEERROR","EQU","*");
   EmitFormattedLine("","SVC","#SVC_WRITE_ENDL");
   AddDSToStaticData("Run-time error #","",reference);
   EmitFormattedLine("","PUSHA",reference);
   EmitFormattedLine("","SVC","#SVC_WRITE_STRING");
   EmitFormattedLine("","SVC","#SVC_WRITE_INTEGER");
   AddDSToStaticData(" near line #","",reference);
   EmitFormattedLine("","PUSHA",reference);
   EmitFormattedLine("","SVC","#SVC_WRITE_STRING");
   EmitFormattedLine("","SVC","#SVC_WRITE_INTEGER");
   EmitFormattedLine("","SVC","#SVC_WRITE_ENDL");
   EmitFormattedLine("","PUSH","#0D1");
   EmitFormattedLine("","SVC","#SVC_TERMINATE");

   EmitUnformattedLine("");
   EmitUnformattedLine(";------------------------------------------------------------");
   EmitUnformattedLine("; Static allocation of global data and PROGRAM module data");
   EmitUnformattedLine(";------------------------------------------------------------");
   EmitFormattedLine("STATICDATA","EQU","*");
   EmitStaticData();

   EmitUnformattedLine("");
   EmitUnformattedLine(";------------------------------------------------------------");
   EmitUnformattedLine("; Heap space for dynamic memory allocation (to support future POTATO syntax)");
   EmitUnformattedLine(";------------------------------------------------------------");
   EmitFormattedLine("HEAPBASE","EQU","*");
   EmitFormattedLine("HEAPSIZE","EQU","0B0001000000000000","8K bytes = 4K words");

   EmitUnformattedLine("");
   EmitUnformattedLine(";------------------------------------------------------------");
   EmitUnformattedLine("; Run-time stack");
   EmitUnformattedLine(";------------------------------------------------------------");
   EmitFormattedLine("RUNTIMESTACK","EQU","0XFFFE");
}

//--------------------------------------------------
int CODE::GetSBOffset()
//--------------------------------------------------
{
   return( SBOffset );
}

//--------------------------------------------------
void CODE::AddRWToStaticData(int operand,const char comment[],char reference[])
//--------------------------------------------------
{
   DATARECORD r;

   strcpy(r.mnemonic,"RW");
   sprintf(r.operand,"0D%d",operand);
   strcpy(r.comment,comment);
   staticdata.push_back( r );
   sprintf(reference,"SB:0D%d",SBOffset);
   SBOffset += operand;
}

//--------------------------------------------------
void CODE::AddDWToStaticData(const char operand[],const char comment[],char reference[])
//--------------------------------------------------
{
   DATARECORD r;

   strcpy(r.mnemonic,"DW");
   strcpy(r.operand,operand);
   strcpy(r.comment,comment);
   staticdata.push_back( r );
   sprintf(reference,"SB:0D%d",SBOffset);
   SBOffset += 1;
}
//--------------------------------------------------
void CODE::AddDSToStaticData(const char operand[],const char comment[],char reference[])
//--------------------------------------------------
{
   DATARECORD r;

   strcpy(r.mnemonic,"DS");
   sprintf(r.operand,"\"%s\"",operand);
   strcpy(r.comment,comment);
   staticdata.push_back( r );
   sprintf(reference,"SB:0D%d",SBOffset);
   SBOffset += 2 + (int) strlen(operand);
}

//--------------------------------------------------
void CODE::EmitStaticData()
//--------------------------------------------------
{
   for(int i = 0; i <= (int) staticdata.size()-1; i++)
      EmitFormattedLine("",staticdata[i].mnemonic,staticdata[i].operand,staticdata[i].comment);
}

//--------------------------------------------------
int CODE::LabelSuffix()
//--------------------------------------------------
{
   labelsuffix += 10;
   return( labelsuffix );
}

//--------------------------------------------------
void CODE::EmitFormattedLine(const char label[],const char mnemonic[],const char operand[],const char comment[])
//--------------------------------------------------
{
/*
         1         2         3         4         5         6         7         8
1234567890123456789012 ^56789012 ^5678901234567890123 ^6789012345678901234567890
*/
   char line[110+1];

   if ( (int) strlen(comment) > 0 )
      sprintf(line,"%-22s %-9s %-20s ; %s",label,mnemonic,operand,comment);
   else
      sprintf(line,"%-22s %-9s %s",label,mnemonic,operand);
   STM << line << endl;
}

//--------------------------------------------------
void CODE::EmitUnformattedLine(const char line[])
//--------------------------------------------------
{
   STM << line << endl;
}

//--------------------------------------------------
void CODE::ResetFrameData()
//--------------------------------------------------
{
   framedata.clear();
   FBOffset = 0;
}

//--------------------------------------------------
void CODE::IncrementFBOffset(int increment/* = 1*/)
//--------------------------------------------------
{
   this->FBOffset += increment;
}

//--------------------------------------------------
int CODE::GetFBOffset()
//--------------------------------------------------
{
   return( FBOffset );
}

//--------------------------------------------------
void CODE::AddInstructionToInitializeFrameData(const char mnemonic[],const char operand[],const char comment[])
//--------------------------------------------------
{
   DATARECORD r;

   strcpy(r.mnemonic,mnemonic);
   strcpy(r.operand,operand);
   strcpy(r.comment,comment);
   framedata.push_back( r );
}

//--------------------------------------------------
void CODE::EmitFrameData()
//--------------------------------------------------
{
   for(int i = 0; i <= (int) framedata.size()-1; i++)
      EmitFormattedLine("",framedata[i].mnemonic,framedata[i].operand,framedata[i].comment);
}

//--------------------------------------------------
void CODE::EnterModuleBody(IDENTIFIERTYPE moduleIdentifierType,int moduleIdentifierIndex)
//--------------------------------------------------
{
   isInModuleBody = true;
   this->moduleIdentifierType = moduleIdentifierType;
   this->moduleIdentifierIndex = moduleIdentifierIndex;
}

//--------------------------------------------------
void CODE::ExitModuleBody()
//--------------------------------------------------
{
   isInModuleBody = false;
}

//--------------------------------------------------
bool CODE::IsInModuleBody(IDENTIFIERTYPE moduleIdentifierType)
//--------------------------------------------------
{
   return( isInModuleBody && (this->moduleIdentifierType == moduleIdentifierType) );
}

//--------------------------------------------------
int CODE::GetModuleIdentifierIndex()
//--------------------------------------------------
{
   return( moduleIdentifierIndex );
}
