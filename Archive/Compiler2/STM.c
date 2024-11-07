//-----------------------------------------------------------
// Dr. Art Hanna
// Simple Target Machine (STM) for SPL featuring a two-pass 
//    assembler and simulation of a stack-based ISA
//  9.25.2018 Added CONCATS (concatenate strings)
//  9.15.2020 Added escape sequence codes to <string> and <character>
//               literal scanning in GetNextToken()

#define VERSION "November, 2023"

// STM.c
//-----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#define SOURCELINELENGTH            512
#define LINESPERPAGE                 55
#define EOPC                        (1)
#define EOLC                        (2)
#define FF                         0X0C

#define SIZEOFIDENTIFIERTABLE       500
#define MAXIMUMLENGTHIDENTIFIER      64

#define HIBYTE(word) ((0XFF00u & word) >> 8)
#define LOBYTE(word) ((0X00FFu & word)     )
#define TORF(word)   ((word == 0XFFFFu) ? 'T' : ((word == 0X0000u)? 'F' : '?'))
#define   SIGNED(x)  (((x) <= 0X7FFF) ?  (int) (x) : ( (int) ((x)-65536)) )
#define UNSIGNED(x)  (((x) <=  32767) ? (WORD) (x) : ((WORD) ((x)-65536)) )

typedef unsigned char BYTE;
typedef unsigned short int WORD;

/*
=====================================================================
STM assembly-language grammar expressed in BNF
=====================================================================
<STMProgram>      ::= { [ <statement> ] EOLC }* EOPC

<statement>       ::= <instruction> [ <comment> ]
                    | <comment>

<instruction>     ::= [ <identifier> ] <HWMnemonic> [ <operand> ]
                    | [ <identifier> ] ORG          <I16>
                    |   <identifier>   EQU          (( <W16> | * ))
                    | [ <identifier> ] RW           [ <I16> ]
                    | [ <identifier> ] DW           <W16>
                    | [ <identifier> ] DS           <string>

<HWMnemonic>      ::= || See list of H/W operations

<operand>         ::= <memory>
                    | #<W16>
                    | <A16>                     || for jump/CALL instructions *ONLY*

<memory>          ::= #<W16>                    || mode = 0X00, immediate
                    | <A16>                     || mode = 0X01, memory direct
                    | @<A16>                    || mode = 0X02, memory indirect
                    | $<A16>                    || mode = 0X03, memory indexed
                    | SP:<I16>                  || mode = 0X04, SP-relative direct
                    | @SP:<I16>                 || mode = 0X05, SP-relative indirect
                    | $SP:<I16>                 || mode = 0X06, SP-relative indexed
                    | FB:<I16>                  || mode = 0X07, FB-relative direct
                    | @FB:<I16>                 || mode = 0X08, FB-relative indirect
                    | $FB:<I16>                 || mode = 0X09, FB-relative indexed
                    | SB:<I16>                  || mode = 0X0A, SB-relative direct
                    | @SB:<I16>                 || mode = 0X0B, SB-relative indirect
                    | $SB:<I16>                 || mode = 0X0C, SB-relative indexed

<W16>             ::= (( <I16> | <F16> | true | false | <character> | <identifier> ))

<A16>             ::= <identifier>

<identifier>      ::= <letter> { (( <letter> | <dit> | _ )) }*

<I16>             ::= [ (( + | - )) ] (( 0D <dit> { <dit> }* 
                    |                    0X <hit> { <hit> }* 
                    |                    0B <bit> { <bit> }* ))

<F16>             ::= [ (( + | - )) ] 0F <dit> { <dit> }* . <dit> { <dit> }* [ E [ - ] <dit> { <dit> }* ]

<bit>             ::= 0 | 1
<dit>             ::= <bit> | 2 | 3| 4| 5 | 6 | 7 | 8 | 9
<hit>             ::= <dit> | A | B | C | D | E | F
<character>       ::= '<ASCIICharacter>'        || *Note* use ''' to embed one ', no escape sequences supported
<string>          ::= "{<ASCIICharacter>}*"     || *Note* escape sequences \n,\t,\v,\b,\\,\" are supported
<letter>          ::= A | B | ... | Z | a | b | ... | z
<ASCIICharacter>  ::=                           || Every printable ASCII character in range [ ' ','~' ]

<comment>         :: ; {<ASCIICharacter>}*

=====================================================================
STM Instruction Set Architecture (ISA)
=====================================================================
OpCode  Assembly Syntax     Instruction Format  Semantics (meaning at run-time)
------  ------------------  ------------------  ------------------------------------------------------------------
0X00    NOOP                OpCode              Do nothing

0X01    PUSH    memory      OpCode:mode:O16     Push word memory[EA] on run-time stack
0X02    PUSHA   memory      OpCode:mode:O16     Push EA on run-time stack
0X03    POP     memory      OpCode:mode:O16     Pop run-time stack and store word in memory[EA]
0X04    DISCARD #W16        OpCode:O16          Discard O16U words from top of run-time stack
0X05    SWAP                OpCode              Pop RHS,LHS; push RHS,LHS
0X06    MAKEDUP             OpCode              Read TOS; push TOS (duplicate TOS)
0X07    PUSHSP              OpCode              Push SP
0X08    PUSHFB              OpCode              Push FB
0X09    PUSHSB              OpCode              Push SB
0X0A    POPSP               OpCode              Pop SP
0X0B    POPFB               OpCode              Pop FB
0X0C    POPSB               OpCode              Pop SB

0X0D    SETAAE  memory      OpCode:mode:O16     Pop key,value; add (key,value) to array in memory[EA] (Note 5)
0X0E    GETAAE  memory      OpCode:mode:O16     Pop key; find (key,value) in array in memory[EA]; push value (Note 6)
0X0F    ADRAAE  memory      OpCode:mode:O16     Pop key; find (key,value) in array in memory[EA]; push address
                                                   of value (Note 7)
0X10    COPYAA              OpCode              Pop RHS,LHS; memory[LHS+2*i] = memory[RHS+2*i], i in 
                                                   [ 0,2*capacity+1 ] (Note 9)

0X11    SETSE   memory      OpCode:mode:O16     Pop character,index; store character in 
                                                   memory[EA+4+2*(index-1)] (Note 8)
0X12    GETSE   memory      OpCode:mode:O16     Pop index; push memory[EA+4+2*(index-1)] (Note 8)
0X13    ADRSE   memory      OpCode:mode:O16     Pop index; push address (EA+4+2*(index-1)) (Note 8)
0X14    ADDSE   memory      OpCode:mode:O16     Pop character; store character in memory[EA+4+2*length];
                                                   increment length (Note 8)
0X15    COPYS               OpCode              Pop RHS,LHS; memory[LHS+2*i] = memory[RHS+2*i], i in 
                                                   [ 0,capacity+1 ] (Note 9)
0X1D    CONCATS             OpCode              Pop RES,RHS,LHS; memory[RES] = memory[LHS] concatenate memory[RHS];
                                                   push RES (Note 12)

0X16    SETAE   memory      OpCode:mode:O16     Pop value,index(n),index(n-1),...,index(1); store value at offset
                                                   in array (Note 10)
0X17    GETAE   memory      OpCode:mode:O16     Pop index(n),index(n-1),...,index(1); push value found at offset
                                                   in array (Note 10)
0X18    ADRAE   memory      OpCode:mode:O16     Pop index(n),index(n-1),...,index(1); push address of value found
                                                   at offset in array (Note 10)
0X19    COPYA               OpCode              Pop RHS,LHS; memory[LHS+2*i] = memory[RHS+2*i], i in 
                                                   [ 0,2*n+capacity ] (Note 9)
0X1A    GETAN   memory      OpCode:mode:O16     Push n (# of dimensions)
0X1B    GETALB  memory      OpCode:mode:O16     Pop dimension # i; push lower-bound, LBi (Note 11)
0X1C    GETAUB  memory      OpCode:mode:O16     Pop dimension # i; push upper-bound, UBi (Note 11)

0X20    ADDI                OpCode              Pop RHS,LHS; push integer ( LHS+RHS )
0X21    ADDF                OpCode              Pop RHS,LHS; push   float ( LHS+RHS )
0X22    SUBI                OpCode              Pop RHS,LHS; push integer ( LHS-RHS )
0X23    SUBF                OpCode              Pop RHS,LHS; push   float ( LHS-RHS )
0X24    MULI                OpCode              Pop RHS,LHS; push integer ( LHS*RHS )
0X25    MULF                OpCode              Pop RHS,LHS; push   float ( LHS*RHS )
0X26    DIVI                OpCode              Pop RHS,LHS; push integer ( LHS÷RHS )
0X27    DIVF                OpCode              Pop RHS,LHS; push   float ( LHS÷RHS )
0X28    REMI                OpCode              Pop RHS,LHS; push integer ( LHS rem RHS )
0X29    POWI                OpCode              Pop RHS,LHS; push integer pow(LHS,RHS)
0X2A    POWF                OpCode              Pop RHS,LHS; push   float pow(LHS,RHS)
0X2B    NEGI                OpCode              Pop RHS; push integer -RHS
0X2C    NEGF                OpCode              Pop RHS; push   float -RHS
0X2D    AND                 OpCode              Pop RHS,LHS; push boolean ( LHS  and RHS )
0X2E    NAND                OpCode              Pop RHS,LHS; push boolean ( LHS nand RHS )
0X2F    OR                  OpCode              Pop RHS,LHS; push boolean ( LHS   or RHS )
0X30    NOR                 OpCode              Pop RHS,LHS; push boolean ( LHS  nor RHS )
0X31    XOR                 OpCode              Pop RHS,LHS; push boolean ( LHS  xor RHS )
0X32    NXOR                OpCode              Pop RHS,LHS; push boolean ( LHS nxor RHS )
0X33    NOT                 OpCode              Pop RHS; push boolean ( not RHS )
0X34    BITAND              OpCode              Pop RHS,LHS; push boolean ( LHS bitwise-and  RHS )
0X35    BITNAND             OpCode              Pop RHS,LHS; push boolean ( LHS bitwise-nand RHS )
0X36    BITOR               OpCode              Pop RHS,LHS; push boolean ( LHS bitwise-or   RHS )
0X37    BITNOR              OpCode              Pop RHS,LHS; push boolean ( LHS bitwise-nor  RHS )
0X38    BITXOR              OpCode              Pop RHS,LHS; push boolean ( LHS bitwise-xor  RHS )
0X39    BITNXOR             OpCode              Pop RHS,LHS; push boolean ( LHS bitwise-nxor RHS )
0X3A    BITNOT              OpCode              Pop RHS; push boolean ( bitwise-not RHS )
0X3B    BITSL  #W16         OpCode:O16          Pop LHS; push ( LHS shifted-left O16U bits                 )
0X3C    BITLSR #W16         OpCode:O16          Pop LHS; push ( LHS logically shifted-right O16U bits      )
0X3D    BITASR #W16         OpCode:O16          Pop LHS; push ( LHS arithmetically shifted-right O16U bits )

0X60    CITOF               OpCode              Pop integer RHS; push   float RHS (integer-to-float)
0X61    CFTOI               OpCode              Pop   float RHS; push integer RHS (float-to-integer)

0X70    CMPI                OpCode              Pop RHS,LHS; set LEG in FLAGS based on ( LHS ? RHS ) (integer)
0X71    CMPF                OpCode              Pop RHS,LHS; set LEG in FLAGS based on ( LHS ? RHS ) (float)
0X72    SETNZPI             OpCode              Set NZP in FLAGS based on sign of TOS (integer)
0X73    SETNZPF             OpCode              Set NZP in FLAGS based on sign of TOS (float)
0X74    SETT                OpCode              Set T in FLAGS based on true/false value of TOS (boolean)

0X80    JMP    A16          OpCode:O16          PC <- O16U
0X81    JMPL   A16          OpCode:O16          if (      L ) PC <- O16U
0X82    JMPE   A16          OpCode:O16          if (      E ) PC <- O16U
0X83    JMPG   A16          OpCode:O16          if (      G ) PC <- O16U
0X84    JMPLE  A16          OpCode:O16          if ( L or E ) PC <- O16U
0X85    JMPNE  A16          OpCode:O16          if ( L or G ) PC <- O16U (JMPLG)
0X86    JMPGE  A16          OpCode:O16          if ( G or E ) PC <- O16U
0X87    JMPN   A16          OpCode:O16          if (      N ) PC <- O16U
0X88    JMPNN  A16          OpCode:O16          if (  not N ) PC <- O16U
0X89    JMPZ   A16          OpCode:O16          if (      Z ) PC <- O16U
0X8A    JMPNZ  A16          OpCode:O16          if (  not Z ) PC <- O16U
0X8B    JMPP   A16          OpCode:O16          if (      P ) PC <- O16U
0X8C    JMPNP  A16          OpCode:O16          if (  not P ) PC <- O16U
0X8D    JMPT   A16          OpCode:O16          if (      T ) PC <- O16U
0X8E    JMPNT  A16          OpCode:O16          if (  not T ) PC <- O16U (JMPF)

0XA0    CALL   A16          OpCode:O16          Push PC; PC <- O16U
0XA1    RETURN              OpCode              Pop PC

0XFF    SVC #W16            OpCode:O16          Execute service request O16U (parameters are passed on run-time stack)

*Notes*
Note  1: Opcode is an 8-bit code used to uniquely identify each STM machine instruction.

Note  2: mode is an 8-bit code used to specify a memory operand's addressing mode.

Note  3: O16 is the instruction's operand field and is interpreted as either a 16-bit two's complement
   integer, O16, or as a 16-bit unsigned integer, O16U. The interpretation used depends on the OpCode.

Note  4: EA is the effective address of a memory word. The EA is computed using O16U and the 
   instruction's addressing mode.
       
Note  5: When (key,value) pair is found, the existing value is replaced with the value popped
   from run-time stack. When (key,value) pair is not found, the pair is stored in the next available
   (key,value) slot. A fatal run-time error occurs when (key,value) pair is not found and (size = capacity).

Note  6: A fatal run-time error occurs when (key,value) pair is not found.

Note  7: When (key,value) pair is not found, the pair is stored in the next available (key,value) slot. 
   A fatal run-time error occurs when (key,value) pair is not found and (size = capacity).

Note  8: A STM string is composed of 2+capacity words of contiguous memory. Word 1 is the length of
   the string (the number of characters contained in the string); word 2 is the string's capacity; and 
   words 3-to-(capacity+2) are reserved for the string's characters. When empty, length = 0, otherwise
   (1 <= length <= capacity). A fatal error occurs when (1 <= index <= length) is not true for SETSE,
   GETSE, and ADRSE. A fatal error occurs when (length = capacity) when ADDSE begins execution.

Note  9: Assumes that memory block pointed to by LHS is large enough to accommodate structure stored
   in memory block pointed to by RHS.

Note 10: A fatal error occurs when the following equation is not true for SETAE, GETAE, and ADRAE.
   ((LB1 <= index1 <= UB1) AND (LB2 <= index2 <= UB2) AND...AND (LBn <= indexn <= UBn))

Note 11: A fatal error occurs when dimension # i is not in [ 1,n ].

Note 12: Assumes that memory block pointed to by RES is large enough to accommodate the concatenation
   of strings pointed to by LHS and RHS. A fatal error occurs when 
   (length-of-LHS + length-of-RHS) > capacity-of-RES
*/

//-----------------------------------------------------------
typedef enum
//-----------------------------------------------------------
{
// Pseudo-terminals
   IDENTIFIER,
   INTEGER,
   FLOAT,
   CHARACTER,
   STRING,
   EOPTOKEN,
   EOLTOKEN,
   UNKNOWN,
// Punctuation
   POUND,
   ATSIGN,
   COLON,
   DOLLAR,
   ASTERISK,
// Assembler mnemonics
   ORG,
   EQU,
   DW,
   DS,
   RW,
// Boolean literals
   FALSE,
   TRUE,
// Registers
   SP,
   FB,
   SB,
// H/W mnemonics
   NOOP,
   PUSH, 
   PUSHA,
   POP,
   DISCARD,
   SWAP,
   MAKEDUP,
   PUSHSP,
   PUSHFB,
   PUSHSB,
   POPSP,
   POPFB,
   POPSB,
   SETAAE,
   GETAAE,
   ADRAAE,
   COPYAA,
   SETSE,
   GETSE,
   ADRSE,
   ADDSE,
   COPYS,
   CONCATS,
   SETAE,
   GETAE,
   ADRAE,
   COPYA,
   GETAN,
   GETALB,
   GETAUB,
   ADDI,
   ADDF,
   SUBI,
   SUBF,
   MULI,
   MULF,
   DIVI,
   DIVF,
   REMI,
   POWI,
   POWF,
   NEGI,
   NEGF,
   AND,
   NAND,
   OR,
   NOR,
   XOR,
   NXOR,
   NOT,
   BITAND,
   BITNAND,
   BITOR,
   BITNOR,
   BITXOR,
   BITNXOR,
   BITNOT,
   BITSL,
   BITLSR,
   BITASR,
   CITOF,
   CFTOI,
   CMPI,
   CMPF,
   SETNZPI,
   SETNZPF,
   SETT,
   JMP,
   JMPL,
   JMPE,
   JMPG,
   JMPLE,
   JMPNE,
   JMPGE,
   JMPN,
   JMPNN,
   JMPZ,
   JMPNZ,
   JMPP,
   JMPNP,
   JMPT,
   JMPNT,
   CALL,
   RETURN,
   SVC
} TOKENTYPE;

typedef enum { NONE,MEMORY,A16,IMMW16 } OPERANDTYPE;

//-----------------------------------------------------------
typedef struct
//-----------------------------------------------------------
{
   int opCode;
   int sizeInBytes;
   char mnemonic[8+1];    // *LOOK* increase size for longer-than-8-character mnemonics
   TOKENTYPE token;
   OPERANDTYPE operandType;   
}  HWOPERATIONRECORD;

//-----------------------------------------------------------
const HWOPERATIONRECORD HWOperationTable[] = 
//-----------------------------------------------------------
{
   { 0X00,1,"NOOP"    ,NOOP    ,NONE    },

   { 0X01,4,"PUSH"    ,PUSH    ,MEMORY  },
   { 0X02,4,"PUSHA"   ,PUSHA   ,MEMORY  },
   { 0X03,4,"POP"     ,POP     ,MEMORY  },
   { 0X04,3,"DISCARD" ,DISCARD ,IMMW16  },
   { 0X05,1,"SWAP"    ,SWAP    ,NONE    },
   { 0X06,1,"MAKEDUP" ,MAKEDUP ,NONE    },
   { 0X07,1,"PUSHSP"  ,PUSHSP  ,NONE    },
   { 0X08,1,"PUSHFB"  ,PUSHFB  ,NONE    },
   { 0X09,1,"PUSHSB"  ,PUSHSB  ,NONE    },
   { 0X0A,1,"POPSP"   ,POPSP   ,NONE    },
   { 0X0B,1,"POPFB"   ,POPFB   ,NONE    },
   { 0X0C,1,"POPSB"   ,POPSB   ,NONE    },
   { 0X0D,4,"SETAAE"  ,SETAAE  ,MEMORY  },
   { 0X0E,4,"GETAAE"  ,GETAAE  ,MEMORY  },
   { 0X0F,4,"ADRAAE"  ,ADRAAE  ,MEMORY  },
   { 0X10,1,"COPYAA"  ,COPYAA  ,NONE    },
   { 0X11,4,"SETSE"   ,SETSE   ,MEMORY  },
   { 0X12,4,"GETSE"   ,GETSE   ,MEMORY  },
   { 0X13,4,"ADRSE"   ,ADRSE   ,MEMORY  },
   { 0X14,4,"ADDSE"   ,ADDSE   ,MEMORY  },
   { 0X15,1,"COPYS"   ,COPYS   ,NONE    },
   { 0X1D,1,"CONCATS" ,CONCATS ,NONE    },
   { 0X16,4,"SETAE"   ,SETAE   ,MEMORY  },
   { 0X17,4,"GETAE"   ,GETAE   ,MEMORY  },
   { 0X18,4,"ADRAE"   ,ADRAE   ,MEMORY  },
   { 0X19,1,"COPYA"   ,COPYA   ,NONE    },
   { 0X1A,4,"GETAN"   ,GETAN   ,MEMORY  },
   { 0X1B,4,"GETALB"  ,GETALB  ,MEMORY  },
   { 0X1C,4,"GETAUB"  ,GETAUB  ,MEMORY  },

   { 0X20,1,"ADDI"    ,ADDI    ,NONE    },
   { 0X21,1,"ADDF"    ,ADDF    ,NONE    },
   { 0X22,1,"SUBI"    ,SUBI    ,NONE    },
   { 0X23,1,"SUBF"    ,SUBF    ,NONE    },
   { 0X24,1,"MULI"    ,MULI    ,NONE    },
   { 0X25,1,"MULF"    ,MULF    ,NONE    },
   { 0X26,1,"DIVI"    ,DIVI    ,NONE    },
   { 0X27,1,"DIVF"    ,DIVF    ,NONE    },
   { 0X28,1,"REMI"    ,REMI    ,NONE    },
   { 0X29,1,"POWI"    ,POWI    ,NONE    },
   { 0X2A,1,"POWF"    ,POWF    ,NONE    },
   { 0X2B,1,"NEGI"    ,NEGI    ,NONE    },
   { 0X2C,1,"NEGF"    ,NEGF    ,NONE    },
   { 0X2D,1,"AND"     ,AND     ,NONE    },
   { 0X2E,1,"NAND"    ,NAND    ,NONE    },
   { 0X2F,1,"OR"      ,OR      ,NONE    },
   { 0X30,1,"NOR"     ,NOR     ,NONE    },
   { 0X31,1,"XOR"     ,XOR     ,NONE    },
   { 0X32,1,"NXOR"    ,NXOR    ,NONE    },
   { 0X33,1,"NOT"     ,NOT     ,NONE    },
   { 0X34,1,"BITAND"  ,BITAND  ,NONE    },
   { 0X35,1,"BITNAND" ,BITNAND ,NONE    },
   { 0X36,1,"BITOR"   ,BITOR   ,NONE    },
   { 0X37,1,"BITNOR"  ,BITNOR  ,NONE    },
   { 0X38,1,"BITXOR"  ,BITXOR  ,NONE    },
   { 0X39,1,"BITNXOR" ,BITNXOR ,NONE    },
   { 0X3A,1,"BITNOT"  ,BITNOT  ,NONE    },
   { 0X3B,3,"BITSL"   ,BITSL   ,IMMW16  },
   { 0X3C,3,"BITLSR"  ,BITLSR  ,IMMW16  },
   { 0X3D,3,"BITASR"  ,BITASR  ,IMMW16  },

   { 0X60,1,"CITOF"   ,CITOF   ,NONE    },
   { 0X61,1,"CFTOI"   ,CFTOI   ,NONE    },

   { 0X70,1,"CMPI"    ,CMPI    ,NONE    },
   { 0X71,1,"CMPF"    ,CMPF    ,NONE    },
   { 0X72,1,"SETNZPI" ,SETNZPI ,NONE    },
   { 0X73,1,"SETNZPF" ,SETNZPF ,NONE    },
   { 0X74,1,"SETT"    ,SETT    ,NONE    },

   { 0X80,3,"JMP"     ,JMP     ,A16     },
   { 0X81,3,"JMPL"    ,JMPL    ,A16     },
   { 0X82,3,"JMPE"    ,JMPE    ,A16     },
   { 0X83,3,"JMPG"    ,JMPG    ,A16     },
   { 0X84,3,"JMPLE"   ,JMPLE   ,A16     },
   { 0X85,3,"JMPNE"   ,JMPNE   ,A16     },
   { 0X86,3,"JMPGE"   ,JMPGE   ,A16     },
   { 0X87,3,"JMPN"    ,JMPN    ,A16     },
   { 0X88,3,"JMPNN"   ,JMPNN   ,A16     },
   { 0X89,3,"JMPZ"    ,JMPZ    ,A16     },
   { 0X8A,3,"JMPNZ"   ,JMPNZ   ,A16     },
   { 0X8B,3,"JMPP"    ,JMPP    ,A16     },
   { 0X8C,3,"JMPNP"   ,JMPNP   ,A16     },
   { 0X8D,3,"JMPT"    ,JMPT    ,A16     },
   { 0X8E,3,"JMPNT"   ,JMPNT   ,A16     },

   { 0XA0,3,"CALL"    ,CALL    ,A16     },
   { 0XA1,1,"RETURN"  ,RETURN  ,NONE    },

   { 0XFF,3,"SVC"     ,SVC     ,IMMW16  }
};

//-----------------------------------------------------------
// GLOBAL VARIABLES
//-----------------------------------------------------------
FILE *SOURCE,*LOG;
char sourceFileName[SOURCELINELENGTH+1];
char sourceLine[SOURCELINELENGTH+1],nextCharacter;
int sourceLineIndex;
bool atEOP;

// ******* identifierTable
typedef struct
{
   int value;
   int numberOfDefinitions;
   char identifier[MAXIMUMLENGTHIDENTIFIER+1];
} IDENTIFIERTABLERECORD;

int sizeOfIdentifierTable;
IDENTIFIERTABLERECORD identifierTable[SIZEOFIDENTIFIERTABLE+1];

// ******* mainMemory
BYTE mainMemory[0XFFFF+1];

// ******* syntaxErrors (up to 10 for each source line)
int numberOfSyntaxErrors;
char syntaxErrors[10][80+1];


//-----------------------------------------------------------
void ProcessRunTimeError(const char error[],bool isFatalError)
//-----------------------------------------------------------
{
   fprintf(LOG    ,"Run-time error %s\n",error); fflush(LOG);
   printf("Run-time error %s\n",error);
   if ( isFatalError )
   {
      fclose(LOG);
      system("PAUSE");
      exit(1);
   }
}

//-----------------------------------------------------------
int main()
//-----------------------------------------------------------
{
   void DoPass1();
   void DoPass2(bool *noSyntaxErrors);
   void ExecuteProgram();

   char fullFileName[SOURCELINELENGTH+1];
   bool noSyntaxErrors;

   printf("Version %s\n\n",VERSION);

   printf("Source filename? "); scanf("%s",sourceFileName);
   strcpy(fullFileName,sourceFileName);
   strcat(fullFileName,".stm");
   if ( (SOURCE = fopen(fullFileName,"r")) == NULL )
   {
      printf("Error opening source file %s\n",fullFileName);
      system("PAUSE");
      exit( 1 );
   }

   strcpy(fullFileName,sourceFileName);
   strcat(fullFileName,".log");
   if ( (LOG = fopen(fullFileName,"w")) == NULL )
   {
      printf("Error opening log file %s\n",fullFileName);
      system("PAUSE");
      exit( 1 );
   }

   printf("Log file is %s\n",fullFileName);

/*
   Assemble source program then, if no syntax errors are discovered
     during translation execute resulting machine program
*/
   DoPass1();
   DoPass2(&noSyntaxErrors);
   fclose(SOURCE);

   if ( noSyntaxErrors )
      ExecuteProgram();
   else
      printf("Source file contains syntax errors\n");

   fclose(LOG);
   system("PAUSE");
   return( 0 );
}

//-----------------------------------------------------------
void DoPass1()
//-----------------------------------------------------------
{
   void GetNextToken(TOKENTYPE *token,char lexeme[]);
   void GetNextCharacter();
   void ReadSourceLine();
   void DefineIdentifierInTable(const char lexeme[],int value);
   bool IdentifierIsInTable(const char lexeme[]);
   int FindIdentifierInTable(const char lexeme[]);
   int ATOI16(const char lexeme[]);
   WORD ATOF16(const char lexeme[]);

   TOKENTYPE token;
   char lexeme[SOURCELINELENGTH+1];
   int LC;
   bool defineLineLabel;
   char labelLexeme[SOURCELINELENGTH+1];
   int labelValue;

   sizeOfIdentifierTable = 0;
   LC = 0X0000;

/*
   Each statement *MUST BE* wholly contained on a single source line.
      Read through source file line-by-line to build identifierTable.
      Ignore *ALL* syntax errors because they will be "caught by" pass #2.
*/
   ReadSourceLine();
   while ( !atEOP )
   {
      GetNextCharacter();
      GetNextToken(&token,lexeme);
      if ( token != EOLTOKEN )
      {
         if ( token == IDENTIFIER )
         {
            defineLineLabel = true;
            strcpy(labelLexeme,lexeme);
            labelValue = LC;    // default, with labelValue changed by ORG and EQU instructions
            GetNextToken(&token,lexeme);
         }
         else
            defineLineLabel = false;
         switch ( token )
         {
            case  ORG: 
               GetNextToken(&token,lexeme);
               if ( token == INTEGER )
                  labelValue = LC = ATOI16(lexeme);
               else             // *ERROR*
                  labelValue = LC = 0X0000;
               break;
            case  EQU:
               GetNextToken(&token,lexeme);
               switch ( token )
               {
                  case INTEGER:
                     labelValue = ATOI16(lexeme);
                     break;
                  case FLOAT:
                     labelValue = ATOF16(lexeme);
                     break;
                  case TRUE:
                     labelValue = 0XFFFFu;
                     break;
                  case FALSE:
                     labelValue = 0X0000u;
                     break;
                  case CHARACTER:
                     labelValue = (WORD) lexeme[0];
                     break;
                  case IDENTIFIER:
                     if ( IdentifierIsInTable(lexeme) )
                        labelValue = identifierTable[FindIdentifierInTable(lexeme)].value;
                     else
                        defineLineLabel = false;
                     break;
                 case ASTERISK:
                     labelValue = LC;
                     break;
                 default:
                     defineLineLabel = false;
                     break;
               }
               break;
            case   RW:
               GetNextToken(&token,lexeme);
               if ( token == INTEGER )
                  LC += 2*ATOI16(lexeme);
               else             // *ERROR* unless token is EOLTOKEN
                  LC += 2;
               break;
            case   DW:          // *always* only one word of object
               LC += 2;
               break;
            case   DS:          // 2-byte UNICODE characters (prepend length and capacity)
               GetNextToken(&token,lexeme);
               if ( token == STRING )
                  LC += 2*((int) strlen(lexeme)+2);
               else             // *ERROR*
                  LC += 2;
               break;
            default:            // should be a hardware operation token
               {
                  bool found = false;
                  int i = 0;
               
                  while ( (i <= (sizeof(HWOperationTable)/sizeof(HWOPERATIONRECORD))-1) && !found )
                     if ( token != HWOperationTable[i].token )
                        i++;
                     else
                     {
                        LC += HWOperationTable[i].sizeInBytes;
                        found = true;
                     }
                  if ( !found ) // *ERROR*
                     LC += 1;
               }
               break;
         }
   /*
      When first token is an identifier add its lexeme and its value
         to the identifierTable if it's not already there.
   */
         if ( defineLineLabel )
            DefineIdentifierInTable(labelLexeme,labelValue);
      }
   // Ignore remainder of source line
      ReadSourceLine();
   }
}

//-----------------------------------------------------------
void DoPass2(bool *noSyntaxErrors)
//-----------------------------------------------------------
{
   void GetNextToken(TOKENTYPE *token,char lexeme[]);
   void GetNextCharacter();
   void ReadSourceLine();
   void ListTopOfPageHeader(int *pageNumber,int *lines);
   void RecordSyntaxError(const char syntaxError[]);
   int ATOI16(const char lexeme[]);
   WORD ParseW16(TOKENTYPE *token,char lexeme[]);
   WORD ParseA16(TOKENTYPE *token,char lexeme[]);
   bool IdentifierIsInTable(const char lexeme[]);
   int FindIdentifierInTable(const char lexeme[]);
   void WriteBYTEToMainMemory(int address,BYTE byte);

   TOKENTYPE token;
   char lexeme[SOURCELINELENGTH+1];
   bool lineIsLabeled;
   BYTE objectCode[256+1];
   int objectBytes;
   int oldLC,LC,lineNumber,linesOnPage,pageNumber;
   int i,j;
   
   *noSyntaxErrors = true;
   LC = 0X0000;
   lineNumber = 0;
   pageNumber = 0;
   ListTopOfPageHeader(&pageNumber,&linesOnPage);
/*
   Each statement *MUST BE* wholly contained on a single source line.
      Read through source file line-by-line to assemble into object code.
*/
   rewind(SOURCE);
   atEOP = false;
   ReadSourceLine(); lineNumber++;
   while ( !atEOP )
   {
      oldLC = LC;
      numberOfSyntaxErrors = 0;
      objectBytes = 0;
      GetNextCharacter();
      GetNextToken(&token,lexeme);
      if ( token != EOLTOKEN )
      {
      // Is first token an identifier? If so, is it multiply defined?
         if ( token == IDENTIFIER )
         {
            int index = FindIdentifierInTable(lexeme);

            if ( identifierTable[index].numberOfDefinitions != 1 )
               RecordSyntaxError("Multiply-defined identifier");
            GetNextToken(&token,lexeme);
            lineIsLabeled = true;
         }
         else
            lineIsLabeled = false;
         switch ( token )
         {
            case IDENTIFIER:    // assume misspelled hardware mnemonic, so insert a NOOP
               LC++;
               objectCode[1] = 0X00u; objectBytes = 1;
               RecordSyntaxError("Misplaced identifier");
               GetNextToken(&token,lexeme);
               break;
            case UNKNOWN:       // does not affect LC
               objectBytes = 0;
               RecordSyntaxError("Unknown token");
               GetNextToken(&token,lexeme);
               break;
/*
<instruction>     ::= [ <identifier> ] ORG          <I16>
                    |   <identifier>   EQU          (( <W16> | * ))
                    | [ <identifier> ] RW           [ <I16> ]
                    | [ <identifier> ] DW           <W16>
                    | [ <identifier> ] DS           <string>
*/
            case ORG:
               objectBytes = 0;
               GetNextToken(&token,lexeme);
               if ( token == INTEGER )
                 oldLC = LC = ATOI16(lexeme);
               else
               {
                 RecordSyntaxError("ORG statement <I16> operand missing");
                 oldLC = LC = 0X0000;
               }
               GetNextToken(&token,lexeme);
               break;
            case EQU:
               if ( !lineIsLabeled )
                  RecordSyntaxError("EQU statement must be labeled");
               objectBytes = 0;
               GetNextToken(&token,lexeme);
               if ( token == ASTERISK )
                  GetNextToken(&token,lexeme);
               else
                  ParseW16(&token,lexeme);
               break;
            case RW: 
               GetNextToken(&token,lexeme);
               objectBytes = 0;
               if      ( token == INTEGER  )
               {
                  LC += 2*ATOI16(lexeme);
                  GetNextToken(&token,lexeme);
               }
               else if ( token == EOLTOKEN )
                  LC += 2;
               else
               {
                  RecordSyntaxError("Invalid RW statement <I16> operand");
                  LC += 2;
                  GetNextToken(&token,lexeme);
               }
               break;
            case  DW:           // one word of object
               {
                  WORD W16;
                  
                  objectBytes = 2;
                  GetNextToken(&token,lexeme);
                  W16 = ParseW16(&token,lexeme);
                  objectCode[1] = HIBYTE(W16);
                  objectCode[2] = LOBYTE(W16);
               }
               break;
            case  DS: 
               GetNextToken(&token,lexeme);
               if ( token == STRING )
               {
                  objectBytes = 0;
               // length
                  objectCode[++objectBytes] = HIBYTE((WORD) strlen(lexeme));
                  objectCode[++objectBytes] = LOBYTE((WORD) strlen(lexeme));
               // capacity
                  objectCode[++objectBytes] = HIBYTE((WORD) strlen(lexeme));
                  objectCode[++objectBytes] = LOBYTE((WORD) strlen(lexeme));
                  for (i = 0; i <= (int) strlen(lexeme)-1; i++)
                  {
                     objectCode[++objectBytes] = 0X00u;
                     objectCode[++objectBytes] = (BYTE) lexeme[i];
                  }
               }
               else
               {
                  objectBytes = 2;
                  RecordSyntaxError("DS statement operand must be string");
                  objectCode[1] = 0X00u;
                  objectCode[2] = 0X00u;
               }
               GetNextToken(&token,lexeme);
               break;
/*
<instruction>     ::= [ <identifier> ] <HWMnemonic> [ <operand> ]

<operand>         ::= <memory>
                    | #<W16>
                    | <A16>

<memory>          ::= #<W16>                    || mode = 0X00, immediate
                    | <A16>                     || mode = 0X01, memory direct
                    | @<A16>                    || mode = 0X02, memory indirect
                    | $<A16>                    || mode = 0X03, memory indexed
                    | SP:<I16>                  || mode = 0X04, SP-relative direct
                    | @SP:<I16>                 || mode = 0X05, SP-relative indirect
                    | $SP:<I16>                 || mode = 0X06, SP-relative indexed
                    | FB:<I16>                  || mode = 0X07, FB-relative direct
                    | @FB:<I16>                 || mode = 0X08, FB-relative indirect
                    | $FB:<I16>                 || mode = 0X09, FB-relative indexed
                    | SB:<I16>                  || mode = 0X0A, SB-relative direct
                    | @SB:<I16>                 || mode = 0X0B, SB-relative indirect
                    | $SB:<I16>                 || mode = 0X0C, SB-relative indexed

<W16>             ::= (( <I16> | <F16> | true | false | <character> | <identifier> ))

<A16>             ::= <identifier>

<identifier>      ::= <letter> { (( <letter> | <dit> | _ )) }*
*/
            default: 
            {
            // find hardware token in HWOperationTable
               TOKENTYPE operation = token;
               bool found = false;
               int i = 0;

               while ( (i <= (sizeof(HWOperationTable)/sizeof(HWOPERATIONRECORD))-1) && !found )
                  if ( operation != HWOperationTable[i].token )
                     i++;
                  else
                     found = true;
               if ( !found )
               {
                  RecordSyntaxError("Invalid hardware mnemonic");
                  LC += 1;
               }
               else
               {
                  objectCode[1] = HWOperationTable[i].opCode;
                  switch ( HWOperationTable[i].operandType )
                  {
                     case NONE:
                        objectBytes = 1;
                        GetNextToken(&token,lexeme);
                        break;
                     case MEMORY:
                        GetNextToken(&token,lexeme);
                        switch ( token )
                        {
                           case POUND:
                              {
                                 WORD W16;
                                 
                                 if ( (operation == POP) || (operation == PUSHA) )
                                    RecordSyntaxError("POP and PUSHA cannot have an immediate operand");
                                 objectCode[2] =  0;
                                 GetNextToken(&token,lexeme);
                                 W16 = ParseW16(&token,lexeme);
                                 objectCode[3] = HIBYTE(W16);
                                 objectCode[4] = LOBYTE(W16);
                              }
                              break;
                           case IDENTIFIER:
                              {
                                 WORD A16;
                                 
                                 objectCode[2] =  1;
                                 A16 = ParseA16(&token,lexeme);
                                 objectCode[3] = HIBYTE(A16);
                                 objectCode[4] = LOBYTE(A16);
                              }
                              break;
                           case ATSIGN:
                              {
                                 WORD A16;
                                 
                                 GetNextToken(&token,lexeme);
                                 if ( token == IDENTIFIER )
                                 {
                                    objectCode[2] =  2;
                                    A16 = ParseA16(&token,lexeme);
                                    objectCode[3] = HIBYTE(A16);
                                    objectCode[4] = LOBYTE(A16);
                                 }
                                 else
                                 {
                                    TOKENTYPE R = token;

                                    if ( !((R == SP) || (R == FB) || (R == SB)) )
                                    {
                                       RecordSyntaxError("Expecting register SP, FB, or SB");
                                       R = SP;
                                    }
                                    GetNextToken(&token,lexeme);
                                    if ( token != COLON )
                                       RecordSyntaxError("Expecting :");
                                    GetNextToken(&token,lexeme);
                                    if ( token != INTEGER )
                                    {
                                       RecordSyntaxError("Expecting <I16>");
                                       objectCode[2] =  5;
                                       objectCode[3] = 0X00u;
                                       objectCode[4] = 0X00u;
                                    }
                                    else
                                    {
                                       switch ( R )
                                       {
                                          case SP: 
                                             objectCode[2] =  5;
                                             break;
                                          case FB:
                                             objectCode[2] =  8;
                                             break;
                                          case SB:
                                             objectCode[2] = 11;
                                             break;
                                       }
                                       objectCode[3] = HIBYTE(ATOI16(lexeme));
                                       objectCode[4] = LOBYTE(ATOI16(lexeme));
                                    }
                                    GetNextToken(&token,lexeme);
                                 }
                              }
                              break;
                           case DOLLAR:
                              {
                                 GetNextToken(&token,lexeme);
                                 if ( token == IDENTIFIER )
                                 {
                                    WORD A16;
                                 
                                    objectCode[2] =  3;
                                    A16 = ParseA16(&token,lexeme);
                                    objectCode[3] = HIBYTE(A16);
                                    objectCode[4] = LOBYTE(A16);
                                 }
                                 else
                                 {
                                    TOKENTYPE R = token;
 
                                    if ( !((R == SP) || (R == FB) || (R == SB)) )
                                    {
                                       RecordSyntaxError("Expecting register SP, FB, or SB");
                                       R = SP;
                                    }
                                    GetNextToken(&token,lexeme);
                                    if ( token != COLON )
                                       RecordSyntaxError("Expecting :");
                                    GetNextToken(&token,lexeme);
                                    if ( token != INTEGER )
                                    {
                                       RecordSyntaxError("Expecting <I16>");
                                       objectCode[2] =  6;
                                       objectCode[3] = 0X00u;
                                       objectCode[4] = 0X00u;
                                    }
                                    else
                                    {
                                       switch ( R )
                                       {
                                          case SP: 
                                             objectCode[2] =  6;
                                             break;
                                          case FB:
                                             objectCode[2] =  9;
                                             break;
                                          case SB:
                                             objectCode[2] = 12;
                                             break;
                                       }
                                       objectCode[3] = HIBYTE(ATOI16(lexeme));
                                       objectCode[4] = LOBYTE(ATOI16(lexeme));
                                    }
                                    GetNextToken(&token,lexeme);
                                 }
                              }
                              break;
                           case SP:
                           case FB:
                           case SB:
                              {
                                 TOKENTYPE R = token;
 
                                 GetNextToken(&token,lexeme);
                                 if ( token != COLON )
                                    RecordSyntaxError("Expecting :");
                                 GetNextToken(&token,lexeme);
                                 if ( token != INTEGER )
                                 {
                                    RecordSyntaxError("Expecting <I16>");
                                    objectCode[3] = 0X00u;
                                    objectCode[4] = 0X00u;
                                 }
                                 else
                                 {
                                    objectCode[3] = HIBYTE(ATOI16(lexeme));
                                    objectCode[4] = LOBYTE(ATOI16(lexeme));
                                 }
                                 switch ( R )
                                 {
                                    case SP:
                                       objectCode[2] =  4;
                                       break;
                                    case FB:
                                       objectCode[2] =  7;
                                       break;
                                    case SB:
                                       objectCode[2] = 10;
                                       break;
                                 }
                              }
                              GetNextToken(&token,lexeme);
                              break;
                           default:
                              RecordSyntaxError("Invalid <memory> operand");
                              objectCode[2] =  0;
                              objectCode[3] = 0X00u;
                              objectCode[4] = 0X00u;
                              GetNextToken(&token,lexeme);
                              break;
                        } 
                        objectBytes = 4;
                        break;
                     case A16:
                        GetNextToken(&token,lexeme);
                        if ( token != IDENTIFIER )
                        {
                           RecordSyntaxError("Expecting <identifier>");
                           objectCode[2] = 0X00u;
                           objectCode[3] = 0X00u;
                           GetNextToken(&token,lexeme);
                        }
                        else
                        {
                           WORD A16;
                        
                           A16 = ParseA16(&token,lexeme);
                           objectCode[2] = HIBYTE(A16);
                           objectCode[3] = LOBYTE(A16);
                        }
                        objectBytes = 3;
                        break;
                     case IMMW16:
                        GetNextToken(&token,lexeme);
                        if ( token != POUND )
                        {
                           RecordSyntaxError("Expecting #");
                           objectCode[2] = 0X00u;
                           objectCode[3] = 0X00u;
                           GetNextToken(&token,lexeme);
                        }
                        else
                        {
                           WORD W16;
                           
                           GetNextToken(&token,lexeme);
                           W16 = ParseW16(&token,lexeme);
                           objectCode[2] = HIBYTE(W16);
                           objectCode[3] = LOBYTE(W16);
                        }
                        objectBytes = 3;
                        break;
                  }
               }
            }
            break;
         }
      }
      if ( token != EOLTOKEN )
         RecordSyntaxError("Expecting end-of-line");
/*
    LC  Object    Line  Source Line
------  --------  ----  -------------------------------------------------------------
0XXXXX  XXXXXXXX  XXXX  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
                  ****  XXX...XXX
*/
      if ( linesOnPage > LINESPERPAGE )
         ListTopOfPageHeader(&pageNumber,&linesOnPage);
      fprintf(LOG,"0X%04hX  ",oldLC);
      for (j = 1; j <= 4; j++)
      {
          if ( j <= objectBytes )
             fprintf(LOG,"%02X",objectCode[j]);
          else
             fprintf(LOG,"  ");
             
      }
      fprintf(LOG,"  %4d  %s\n",lineNumber,sourceLine); fflush(LOG);
      linesOnPage++;
      i = 5;
      while ( i <= objectBytes )
      {
         if ( linesOnPage > LINESPERPAGE )
            ListTopOfPageHeader(&pageNumber,&linesOnPage);
         fprintf(LOG,"0X%04hX  ",(oldLC+i-1));
         for (j = 1; j <= 4; j++)
         {
             if ( j+i-1 <= objectBytes )
                fprintf(LOG,"%02X",objectCode[j+i-1]);
             else
                fprintf(LOG,"  ");
         }
         fprintf(LOG,"\n"); fflush(LOG);
         linesOnPage++;
         i = i+4;
      }
      LC = LC+objectBytes;
      if ( LC-1 > 0XFFFF )
      {
         RecordSyntaxError("Location counter overflow");
         LC = 0X0000;
      }
      for (i = 1; i <= numberOfSyntaxErrors; i++)
      {
         *noSyntaxErrors = false;
         if ( linesOnPage > LINESPERPAGE )
            ListTopOfPageHeader(&pageNumber,&linesOnPage);
         fprintf(LOG,"                  ****  %s\n",syntaxErrors[i]); fflush(LOG);
         linesOnPage++;
         printf("Error on line %4d %s\n",lineNumber,syntaxErrors[i]);
      }
      for (i = 1; i <= objectBytes; i++)
         WriteBYTEToMainMemory(oldLC+i-1,objectCode[i]);
      ReadSourceLine(); lineNumber++;
   }
}

//-----------------------------------------------------------
WORD ParseW16(TOKENTYPE *token,char lexeme[])
//-----------------------------------------------------------
{
   WORD ParseA16(TOKENTYPE *token,char lexeme[]);
   void GetNextToken(TOKENTYPE *token,char lexeme[]);
   int ATOI16(const char lexeme[]);
   WORD ATOF16(const char lexeme[]);
   void RecordSyntaxError(const char syntaxError[]);

   WORD W16;
   
   switch ( *token )
   {
      case INTEGER:
         W16 = ATOI16(lexeme);
         GetNextToken(token,lexeme);
         break;
      case FLOAT:
         W16 = ATOF16(lexeme);
         GetNextToken(token,lexeme);
         break;
      case TRUE:
         W16 = 0XFFFFu;
         GetNextToken(token,lexeme);
         break;
      case FALSE:
         W16 = 0X0000u;
         GetNextToken(token,lexeme);
         break;
      case CHARACTER:
         W16 = (WORD) lexeme[0];
         GetNextToken(token,lexeme);
         break;
      case IDENTIFIER:
         W16 = ParseA16(token,lexeme);
         break;
      default:
         RecordSyntaxError("Expecting <I16>, <F16>, true, false, <character>, or <identifier>");
         W16 = 0X0000u;
         GetNextToken(token,lexeme);
         break;
   }
   return( W16 );
}

//-----------------------------------------------------------
WORD ParseA16(TOKENTYPE *token,char lexeme[])
//-----------------------------------------------------------
{
   void GetNextToken(TOKENTYPE *token,char lexeme[]);
   bool IdentifierIsInTable(const char lexeme[]);
   int FindIdentifierInTable(const char lexeme[]);
   void RecordSyntaxError(const char syntaxError[]);

   WORD A16;

   if ( IdentifierIsInTable(lexeme) )
      A16 = (WORD) identifierTable[FindIdentifierInTable(lexeme)].value;
   else
   {
      RecordSyntaxError("Undefined <identifier>");
      A16 = 0X0000u;
   }
   GetNextToken(token,lexeme);
   return( A16 );
}

//-----------------------------------------------------------
bool IsValidDigit(char digit,char base)
//-----------------------------------------------------------
{
   bool r;

   switch ( base )
   {
      case 'B': 
         r = ( (digit == '0') || (digit == '1') );
         break;
      case 'D':
         r = isdigit(digit);
         break;
      case 'X':
         r = isxdigit(digit);
         break;
   }
   return( r );
}

//-----------------------------------------------------------
int ATOI16(const char lexeme[])
//-----------------------------------------------------------
{
/*
   Safely assume lexeme "obeys" the following syntax rule because the scanner
      recognized TOKENTYPE as INTEGER

<I16>             ::= [ (( + | - )) ] (( 0D <dit> { <dit> }* 
                    |                    0X <hit> { <hit> }* 
                    |                    0B <bit> { <bit> }* ))
*/
   char sign,base;
   int i,digit,I16;

   i = 0;
   sign = '+';
   if ( (lexeme[i] == '+') || (lexeme[i] == '-') )
   {
      sign = lexeme[i];
      i++;
   }
   i++; base = lexeme[i]; i++;
   I16 = 0;
   while ( i <= (int) strlen(lexeme)-1 )
   {
      if ( lexeme[i] == '0' ) digit =  0;
      if ( lexeme[i] == '1' ) digit =  1;
      if ( lexeme[i] == '2' ) digit =  2;
      if ( lexeme[i] == '3' ) digit =  3;
      if ( lexeme[i] == '4' ) digit =  4;
      if ( lexeme[i] == '5' ) digit =  5;
      if ( lexeme[i] == '6' ) digit =  6;
      if ( lexeme[i] == '7' ) digit =  7;
      if ( lexeme[i] == '8' ) digit =  8;
      if ( lexeme[i] == '9' ) digit =  9;
      if ( lexeme[i] == 'A' ) digit = 10;
      if ( lexeme[i] == 'B' ) digit = 11;
      if ( lexeme[i] == 'C' ) digit = 12;
      if ( lexeme[i] == 'D' ) digit = 13;
      if ( lexeme[i] == 'E' ) digit = 14;
      if ( lexeme[i] == 'F' ) digit = 15;

      switch ( base )
      {
         case 'B': 
            I16 = I16* 2 + digit;
            break;
         case 'D':
            I16 = I16*10 + digit;
            break;
         case 'X':
            I16 = I16*16 + digit;
            break;
      }
      i++;
   }
   return( (sign == '-')? -I16 : I16 );
}

//-----------------------------------------------------------
WORD ATOF16(const char lexeme[])
//-----------------------------------------------------------
{
/*
   Safely assume lexeme "obeys" the following syntax rule because the scanner
      recognized TOKENTYPE as FLOAT

<F16>             ::= [ (( + | - )) ] 0F <dit> { <dit> }* . <dit> { <dit> }* [ E [ - ] <dit> { <dit> }* ]
*/
   void ConvertFloatToHalfFloat(float F,WORD *HF);

   char sign;
   WORD F16;
   float item;
   int i;

   i = 0;
   sign = '+';
   if ( (lexeme[i] == '+') || (lexeme[i] == '-') )
   {
      sign = lexeme[i];
      i++;
   }
   i += 2;
   sscanf(&lexeme[i],"%f",&item);
   item = ((sign == '-') ? -item : item);
   ConvertFloatToHalfFloat(item,&F16);
   return( F16 );
}

//-----------------------------------------------------------
void GetNextToken(TOKENTYPE *token,char lexeme[])
//-----------------------------------------------------------
{
   void GetNextCharacter();
   bool IsValidDigit(char digit,char base);
   void RecordSyntaxError(const char syntaxError[]);

   int i;

// "Eat" blanks and tabs (if any) at beginning-of-line
   while ( (nextCharacter == ' ') || (nextCharacter == '\t') )
      GetNextCharacter();

/*
   "Eat" comments (if any). Comments are always extend to end-of-line, but *DO NOT* include EOLC.
      <comment>         :: ; { <ASCIICharacter> }* 
*/
   if ( nextCharacter == ';' )
   {
      do
         GetNextCharacter();
      while ( nextCharacter != EOLC );
   }
/*
   If an identifier-like lexeme is not an assembler mnemonic, a hardware mnemonic, 
      a boolean literal, or a register, then it is--by default--an identifier

      <identifier>      ::= <letter> { (( <letter> | <dit> | _ )) }*
*/
   if ( isalpha(nextCharacter) )
   {
      char UClexeme[SOURCELINELENGTH+1];

      i = 0;
      lexeme[i++] = nextCharacter;
      GetNextCharacter();
      while ( isalpha(nextCharacter) ||
              isdigit(nextCharacter) ||
              (nextCharacter == '_') )
      {
         lexeme[i++] = nextCharacter;
         GetNextCharacter();
      }
      lexeme[i] = '\0';
      *token = IDENTIFIER;
      for (i = 0; i <= (int) strlen(lexeme); i++)
         UClexeme[i] = toupper(lexeme[i]);
      if      ( strcmp(UClexeme,"ORG"   ) == 0 )
         *token = ORG;
      else if ( strcmp(UClexeme,"EQU"   ) == 0 )
         *token = EQU;
      else if ( strcmp(UClexeme,"DW"    ) == 0 )
         *token = DW;
      else if ( strcmp(UClexeme,"DS"    ) == 0 )
         *token = DS;
      else if ( strcmp(UClexeme,"RW"    ) == 0 )
         *token = RW;
      else if ( strcmp(UClexeme,"TRUE"  ) == 0 )
         *token = TRUE;
      else if ( strcmp(UClexeme,"FALSE" ) == 0 )
         *token = FALSE;
      else if ( strcmp(UClexeme,"SP"    ) == 0 )
         *token = SP;
      else if ( strcmp(UClexeme,"FB"    ) == 0 )
         *token = FB;
      else if ( strcmp(UClexeme,"SB"    ) == 0 )
         *token = SB;
      else
      {
         for (i = 0; i <= (sizeof(HWOperationTable)/sizeof(HWOPERATIONRECORD))-1; i++)
            if ( strcmp(UClexeme,HWOperationTable[i].mnemonic) == 0 )
               *token = HWOperationTable[i].token;
      }
   }
/*
   <I16>             ::= [ (( + | - )) ] (( 0D <dit> { <dit> }* 
                       |                    0X <hit> { <hit> }* 
                       |                    0B <bit> { <bit> }* ))
   <F16>             ::= [ (( + | - )) ] 0F <dit> { <dit> }* . <dit> { <dit> }* [ E [ - ] <dit> { <dit> }* ]
   <bit>             ::= 0 | 1
   <dit>             ::= <bit> | 2 | 3| 4| 5 | 6 | 7 | 8 | 9
   <hit>             ::= <dit> | A | B | C | D | E | F
*/
   else if ( (nextCharacter == '0')
          || (nextCharacter == '+')
          || (nextCharacter == '-') )
   {
      i = 0;
      if ( (nextCharacter == '+') || (nextCharacter == '-') )
      {
         lexeme[i++] = nextCharacter;
         GetNextCharacter();
      }
      if ( nextCharacter != '0' )
      {
         RecordSyntaxError("Invalid <I16> or <F16> prefix\n");
         *token = INTEGER;
         strcpy(lexeme,"0D0");
      }
      else
      {
         lexeme[i++] = nextCharacter;
         GetNextCharacter();
         if      ( (toupper(nextCharacter) == 'B') 
                || (toupper(nextCharacter) == 'D')
                || (toupper(nextCharacter) == 'X') )
         {
            char base = toupper(nextCharacter);

            lexeme[i++] = base;
            GetNextCharacter();
            while ( IsValidDigit(nextCharacter,base) )
            {
               lexeme[i++] = toupper(nextCharacter);
               GetNextCharacter();
            }
            *token = INTEGER;
            lexeme[i] = '\0';
         }
         else if ( toupper(nextCharacter) == 'F' ) 
         {
            lexeme[i++] = nextCharacter;
            GetNextCharacter();
            if ( !isdigit(nextCharacter) )
            {
               RecordSyntaxError("Invalid <F16> literal");
               strcpy(lexeme,"0F0.0");
               *token = FLOAT;
            }
            else
            {
               lexeme[i++] = nextCharacter;
               GetNextCharacter();
               while ( isdigit(nextCharacter) )
               {
                  lexeme[i++] = nextCharacter;
                  GetNextCharacter();
               }
               if ( nextCharacter != '.' )
               {
                  RecordSyntaxError("Invalid <F16> literal");
                  strcpy(lexeme,"0F0.0");
                  *token = FLOAT;
               }
               else
               {
                  lexeme[i++] = nextCharacter;
                  GetNextCharacter();
                  if ( !isdigit(nextCharacter) )
                  {
                     RecordSyntaxError("Invalid <F16> literal");
                     strcpy(lexeme,"0F0.0");
                     *token = FLOAT;
                  }
                  else
                  {
                     lexeme[i++] = nextCharacter;
                     GetNextCharacter();
                     while ( isdigit(nextCharacter) )
                     {
                        lexeme[i++] = nextCharacter;
                        GetNextCharacter();
                     }
                     if ( toupper(nextCharacter) != 'E' )
                     {
                        lexeme[i] = '\0';
                        *token = FLOAT;
                     }
                     else
                     {
                        lexeme[i++] = nextCharacter;
                        GetNextCharacter();
                        if ( nextCharacter == '-' )
                        {
                           lexeme[i++] = nextCharacter;
                           GetNextCharacter();
                        }
                        if ( !isdigit(nextCharacter) )
                        {
                           RecordSyntaxError("Invalid <F16> literal");
                           strcpy(lexeme,"0F0.0");
                           *token = FLOAT;
                        }
                        else
                        {
                           lexeme[i++] = nextCharacter;
                           GetNextCharacter();
                           while ( isdigit(nextCharacter) )
                           {
                              lexeme[i++] = nextCharacter;
                              GetNextCharacter();
                           }
                           lexeme[i] = '\0';
                           *token = FLOAT;
                        }
                     }
                  }
               }
            }
         }
         else
         {
            RecordSyntaxError("Invalid <I16> or <F16> prefix\n");
            strcpy(lexeme,"0D0");
            *token = INTEGER;
         }
      }
   }
   else
   {
      switch ( nextCharacter )
      {
// <string> literal *Note* escape sequences \n,\t,\b,\r,\\,\" are supported
         case '"': 
            i = 0;
            GetNextCharacter();
            while ( (nextCharacter !=  '"') && 
                    (nextCharacter != EOPC) &&
                    (nextCharacter != EOLC) )
            {
               if ( nextCharacter == '\\' )
               {
                  lexeme[i++] = nextCharacter;
                  GetNextCharacter();
                  if ( (nextCharacter == 'n') ||
                       (nextCharacter == 't') ||
                       (nextCharacter == 'b') ||
                       (nextCharacter == 'r') ||
                       (nextCharacter == '\\') ||
                       (nextCharacter == '"') )
                  {
                     lexeme[i++] = nextCharacter;
                  }
                  else
                  {
                     RecordSyntaxError("Invalid escape sequence in <string> literal\n");
                     lexeme[i++] = nextCharacter;
                  }
               }
               else
               {
                  lexeme[i++] = nextCharacter;
               }
               GetNextCharacter();
            }
            lexeme[i] = '\0';
            *token = STRING;
            if ( nextCharacter != '"' )
               RecordSyntaxError("Unterminated <string> literal\n");
            GetNextCharacter();
            break;
// <character> literal *Note* use ''' to embed one ', no escape character sequences supported
         case '\'':
            *token = CHARACTER;
            GetNextCharacter();
            lexeme[0] = nextCharacter; lexeme[1] = '\0';
            GetNextCharacter();
            if ( nextCharacter != '\'' )
               RecordSyntaxError("Invalid <character> literal\n");
            GetNextCharacter();
            break;
         case ':': 
            *token = COLON;
            lexeme[0] = nextCharacter; lexeme[1] = '\0';
            GetNextCharacter();
            break;
         case '#': 
            *token = POUND;
            lexeme[0] = nextCharacter; lexeme[1] = '\0';
            GetNextCharacter();
            break;
         case '@': 
            *token = ATSIGN;
            lexeme[0] = nextCharacter; lexeme[1] = '\0';
            GetNextCharacter();
            break;
         case '$': 
            *token = DOLLAR;
            lexeme[0] = nextCharacter; lexeme[1] = '\0';
            GetNextCharacter();
            break;
         case '*': 
            *token = ASTERISK;
            lexeme[0] = nextCharacter; lexeme[1] = '\0';
            GetNextCharacter();
            break;
         case EOLC: 
            *token = EOLTOKEN;
            lexeme[0] = '\0';
            break;
         case EOPC: 
            *token = EOPTOKEN;
            lexeme[0] = '\0';
            break;
         default:  
            RecordSyntaxError("Unknown token\n");
            *token = UNKNOWN;
            lexeme[0] = nextCharacter; lexeme[1] = '\0';
            GetNextCharacter();
            break;
      }
   }
}

//-----------------------------------------------------------
void RecordSyntaxError(const char syntaxError[])
//-----------------------------------------------------------
{
   if ( numberOfSyntaxErrors <= 10 )
      strcpy(syntaxErrors[++numberOfSyntaxErrors],syntaxError);
}

//-----------------------------------------------------------
void GetNextCharacter()
//-----------------------------------------------------------
{
   if ( atEOP )
      nextCharacter = EOPC;
   else if ( sourceLineIndex <= ((int) strlen(sourceLine)-1) )
   {
      nextCharacter = sourceLine[sourceLineIndex];
      sourceLineIndex += 1;
   }
   else
      nextCharacter = EOLC;
}

//--------------------------------------------------
void ReadSourceLine()
//--------------------------------------------------
{
   if ( feof(SOURCE) )
      atEOP = true;
   else
   {
      if ( fgets(sourceLine,SOURCELINELENGTH,SOURCE) == NULL )
         atEOP = true;
      else
      {
         if ( (strchr(sourceLine,'\n') == NULL) && !feof(SOURCE) )
         {
            fprintf(LOG,"******* Source line is too long!");
            fflush(LOG);
         }
      // Erase *ALL* control characters at end of source line (if any)
         while ( (0 <= (int) strlen(sourceLine)-1) && 
                 iscntrl(sourceLine[(int) strlen(sourceLine)-1]) )
            sourceLine[(int) strlen(sourceLine)-1] = '\0';
         sourceLineIndex = 0;
      }
   }
}

//-----------------------------------------------------------
void ListTopOfPageHeader(int *pageNumber,int *linesOnPage)
//-----------------------------------------------------------
{
/*
         11111111112222222222333333333344444444445555555555666666666677777777778
12345678901234567890123456789012345678901234567890123456789012345678901234567890
Page XXX  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

    LC  Object    Line  Source Line
------  --------  ----  -------------------------------------------------------------
0XXXXX  XXXXXXXX  XXXX  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
*/
   *pageNumber += 1;
   *linesOnPage = 0;
   fprintf(LOG,"%cPage %3d  %s.stm\n\n",FF,*pageNumber,sourceFileName);
   fprintf(LOG,"    LC  Object    Line  Source Line\n");
   fprintf(LOG,"------  --------  ----  -------------------------------------------------------------\n");
   fflush(LOG);
}

//-----------------------------------------------------------
void DefineIdentifierInTable(const char lexeme[],int value)
//-----------------------------------------------------------
{
   bool IdentifierIsInTable(const char lexeme[]);
   int FindIdentifierInTable(const char lexeme[]);

   if ( IdentifierIsInTable(lexeme) )
   {
      int index = FindIdentifierInTable(lexeme);
      identifierTable[index].numberOfDefinitions++;
   }
   else
   {
      if ( sizeOfIdentifierTable <= SIZEOFIDENTIFIERTABLE )
      {
         sizeOfIdentifierTable++;
         strncpy(identifierTable[sizeOfIdentifierTable].identifier,lexeme,MAXIMUMLENGTHIDENTIFIER);
         identifierTable[sizeOfIdentifierTable].numberOfDefinitions = 1;
         identifierTable[sizeOfIdentifierTable].value = value;
      }
      else
         ProcessRunTimeError("Identifier table overflow",true);
   }
}

//-----------------------------------------------------------
bool IdentifierIsInTable(const char lexeme[])
//-----------------------------------------------------------
{
   bool isInTable;
   int i,index;
   char UClexeme[SOURCELINELENGTH+1];

   for (i = 0; i <= (int) strlen(lexeme); i++)
      UClexeme[i] = toupper(lexeme[i]);
   isInTable = false;
   index = 1;
   while ( !isInTable && (index <= sizeOfIdentifierTable) )
   {
      char UCidentifier[MAXIMUMLENGTHIDENTIFIER+1];

      for (i = 0; i <= (int) strlen(identifierTable[index].identifier); i++)
         UCidentifier[i] = toupper(identifierTable[index].identifier[i]);
      if ( strncmp(UCidentifier,UClexeme,MAXIMUMLENGTHIDENTIFIER) == 0 )
         isInTable = true;
      else
         index += 1;
   }
   return( isInTable );
}

//-----------------------------------------------------------
int FindIdentifierInTable(const char lexeme[])
//-----------------------------------------------------------
{
   bool isFound;
   int i,index;
   char UClexeme[SOURCELINELENGTH+1];

   for (i = 0; i <= (int) strlen(lexeme); i++)
      UClexeme[i] = toupper(lexeme[i]);
   index = 1;
   isFound = false;
   while ( !isFound )
   {
      char UCidentifier[MAXIMUMLENGTHIDENTIFIER+1];

      for (i = 0; i <= (int) strlen(identifierTable[index].identifier); i++)
         UCidentifier[i] = toupper(identifierTable[index].identifier[i]);
      if ( strncmp(UCidentifier,UClexeme,MAXIMUMLENGTHIDENTIFIER) == 0 )
         isFound = true;
      else
         index += 1;
   }
   return( index );
}

//-----------------------------------------------------------
void InitializeMainMemory()
//-----------------------------------------------------------
{
   int address;

   for (address = 0X0000; address <= 0XFFFF; address++)
      mainMemory[address] = 0X00u;
}

//-----------------------------------------------------------
void WriteBYTEToMainMemory(int address,BYTE byte)
//-----------------------------------------------------------
{
   if ( (0X0000 <= address) && (address <= 0XFFFF) )
      mainMemory[address] = byte;
   else
   {
      char information[SOURCELINELENGTH+1];
      
      sprintf(information,"Write main memory address 0X%08X is not in [ 0X0000,0XFFFF ]",address);
      ProcessRunTimeError(information,true);
   }
}

//-----------------------------------------------------------
void ReadBYTEFromMainMemory(int address,BYTE *byte)
//-----------------------------------------------------------
{
   if ( (0X0000 <= address) && (address <= 0XFFFF) )
      *byte = mainMemory[address];
   else
   {
      char information[SOURCELINELENGTH+1];
      
      sprintf(information,"Read main memory address 0X%08X is not in [ 0X0000,0XFFFF ]",address);
      ProcessRunTimeError(information,true);
   }
}

//-----------------------------------------------------------
void WriteWORDToMainMemory(int address,WORD word)
//-----------------------------------------------------------
{
   void WriteBYTEToMainMemory(int address,BYTE byte);

   BYTE hiByte = HIBYTE(word);
   BYTE loByte = LOBYTE(word);

// STM is a big-endian machine
   WriteBYTEToMainMemory(  address,hiByte);
   WriteBYTEToMainMemory(address+1,loByte);
}

//-----------------------------------------------------------
void ReadWORDFromMainMemory(int address,WORD *word)
//-----------------------------------------------------------
{
   void ReadBYTEFromMainMemory(int address,BYTE *byte);

   BYTE loByte,hiByte;

// STM is a big-endian machine
   ReadBYTEFromMainMemory(address  ,&hiByte);
   ReadBYTEFromMainMemory(address+1,&loByte);
   *word = (hiByte << 8) | loByte;
}

//-----------------------------------------------------------
void ExecuteProgram()
//-----------------------------------------------------------
{
   void WriteBYTEToMainMemory(int address,BYTE byte);
   void WriteWORDToMainMemory(int address,WORD word);
   void ReadBYTEFromMainMemory(int address,BYTE *byte);
   void ReadWORDFromMainMemory(int address,WORD *word);
   WORD MemoryOperandEA(BYTE mode,WORD O16,WORD PC,WORD *SP,WORD FB,WORD SB,char information[]);
   void ConvertFloatToHalfFloat(float F,WORD *HF);
   void ConvertHalfFloatToFloat(WORD HF,float *F);
   void ConvertHalfFloatToBase10(WORD HF,char base10[]);
   void TraceFREEnodes(WORD FREEblocks);

   WORD PC,SP,FB,SB;       // CPU registers
   char N,Z,P,T,L,E,G,R;   // FLAGS "register" (R is reserved for future use)
   bool running;

   char IN[SOURCELINELENGTH+1],OUT[SOURCELINELENGTH+1];

   WORD heapBase,heapSize,FREEnodes;

   PC = 0X0000u;
   SP = 0XFFFEu;  // address of first available word on run-time stack (locations 0XFFFE:0XFFFF)
   FB = 0X0000u;  // default address that *MUST* be changed by machine program before use
   SB = 0X0000u;  // default address that *MUST* be changed by machine program before use
   N = Z = P = T = L = E = G = 0;
   
   running = true;

   OUT[0] = '\0';

/*
  PC   SP TOS0 TOS1 TOS2 mnemonic  information
---- ---- ---- ---- ---- --------- ----------------------------------------------
XXXX XXXX XXXX XXXX XXXX XXXXXXXX XXXXXX...
*/
   fprintf(LOG,"\n\n");
   fprintf(LOG,"  PC   SP TOS0 TOS1 TOS2 mnemonic  information\n");
   fprintf(LOG,"---- ---- ---- ---- ---- --------- ----------------------------------------------\n");
   fflush(LOG);
   do
   {
      WORD O16,W16,RHS,LHS,TOS,EA,memoryOperand;
      BYTE opCode,mode;
      char traceLine[SOURCELINELENGTH+1],information[SOURCELINELENGTH+1];

      sprintf(traceLine,"%04hX %04hX",PC,SP);
      if ( SP <= 0XFFFC )
      {
         ReadWORDFromMainMemory(SP+2,&W16);
         sprintf(information," %04hX",W16);
         strcat(traceLine,information);
      }
      else
         strcat(traceLine,"     ");
      if ( SP <= 0XFFFA )
      {
         ReadWORDFromMainMemory(SP+4,&W16);
         sprintf(information," %04hX",W16);
         strcat(traceLine,information);
      }
      else
         strcat(traceLine,"     ");
      strcat(traceLine," ");
      if ( SP <= 0XFFF9 )
      {
         ReadWORDFromMainMemory(SP+6,&W16);
         sprintf(information," %04hX",W16);
         strcat(traceLine,information);
      }
      else
         strcat(traceLine,"     ");
      strcat(traceLine," ");

      ReadBYTEFromMainMemory(PC,&opCode); PC += 1;

      switch ( opCode )
      {
      // 0X00    NOOP                OpCode              Do nothing
         case 0X00:
            strcat(traceLine,"NOOP     ");
            break;

      // 0X01    PUSH    memory      OpCode:mode:O16     Push word memory[EA] on run-time stack
         case 0X01: 
            strcat(traceLine,"PUSH     ");
            ReadBYTEFromMainMemory(PC,&mode); PC += 1;
            ReadWORDFromMainMemory(PC,&O16); PC += 2;
            EA = MemoryOperandEA(mode,O16,PC,&SP,FB,SB,information);
            strcat(traceLine,information);
            ReadWORDFromMainMemory(EA,&memoryOperand);
            WriteWORDToMainMemory(SP,memoryOperand); SP -= 2;
            sprintf(information," = 0X%04hX",memoryOperand);
            strcat(traceLine,information);
            break;

      // 0X02    PUSHA   memory      OpCode:mode:O16     Push EA on run-time stack
         case 0X02: 
            strcat(traceLine,"PUSHA    ");
            ReadBYTEFromMainMemory(PC,&mode); PC += 1;
            ReadWORDFromMainMemory(PC,&O16); PC += 2;
            EA = MemoryOperandEA(mode,O16,PC,&SP,FB,SB,information);
            strcat(traceLine,information);
            WriteWORDToMainMemory(SP,EA); SP -= 2;
            break;

      // 0X03    POP     memory      OpCode:mode:O16     Pop word from run-time stack and store in memory[EA]
         case 0X03: 
            strcat(traceLine,"POP      ");
            ReadBYTEFromMainMemory(PC,&mode); PC += 1;
            ReadWORDFromMainMemory(PC,&O16); PC += 2;
            EA = MemoryOperandEA(mode,O16,PC,&SP,FB,SB,information);
            strcat(traceLine,information);
            ReadWORDFromMainMemory(SP+2,&memoryOperand); SP += 2;
            WriteWORDToMainMemory(EA,memoryOperand);
            sprintf(information," = 0X%04hX",memoryOperand);
            strcat(traceLine,information);
            break;

      // 0X04    DISCARD #W16        OpCode:O16          Discard O16U words at top of run-time stack
         case 0X04: 
            strcat(traceLine,"DISCARD  ");
            ReadWORDFromMainMemory(PC,&O16); PC += 2;
            SP += 2*O16;
            sprintf(information," #%hd words from top-of-stack",O16);
            strcat(traceLine,information);
            break;

      // 0X05    SWAP                OpCode              Pop RHS,LHS; push RHS,LHS
         case 0X05: 
            strcat(traceLine,"SWAP     ");
            ReadWORDFromMainMemory(SP+2,&RHS); SP += 2;
            ReadWORDFromMainMemory(SP+2,&LHS); SP += 2;
            WriteWORDToMainMemory(SP,RHS); SP -= 2;
            WriteWORDToMainMemory(SP,LHS); SP -= 2;
            sprintf(information," 0X%04hX <--> 0X%04hX",LHS,RHS);
            strcat(traceLine,information);
            break;

      // 0X06    MAKEDUP             OpCode              Read TOS; push TOS (duplicate TOS)
         case 0X06:
            strcat(traceLine,"MAKEDUP  ");
            ReadWORDFromMainMemory(SP+2,&TOS);
            WriteWORDToMainMemory(SP,TOS); SP -= 2;
            sprintf(information," duplicate 0X%04hX",TOS);
            strcat(traceLine,information);
            break;

      // 0X07    PUSHSP              OpCode              Push SP
         case 0X07: 
            strcat(traceLine,"PUSHSP   ");
            WriteWORDToMainMemory(SP,SP); SP -= 2;
            sprintf(information," 0X%04hX",SP+2);
            strcat(traceLine,information);
            break;

      // 0X08    PUSHFB              OpCode              Push FB
         case 0X08: 
            strcat(traceLine,"PUSHFB   ");
            WriteWORDToMainMemory(SP,FB); SP -= 2;
            sprintf(information," 0X%04hX",FB);
            strcat(traceLine,information);
            break;

      // 0X09    PUSHSB              OpCode              Push SB
         case 0X09: 
            strcat(traceLine,"PUSHSB   ");
            WriteWORDToMainMemory(SP,SB); SP -= 2;
            sprintf(information," 0X%04hX",SB);
            strcat(traceLine,information);
            break;

      // 0X0A    POPSP               OpCode              Pop SP
         case 0X0A: 
            strcat(traceLine,"POPSP    ");
            ReadWORDFromMainMemory(SP+2,&SP); // ***SP += 2; NOT REQUIRED***
            sprintf(information," SP = 0X%04hX",SP);
            strcat(traceLine,information);
            break;

      // 0X0B    POPFB               OpCode              Pop FB
         case 0X0B: 
            strcat(traceLine,"POPFB    ");
            ReadWORDFromMainMemory(SP+2,&FB); SP += 2;
            sprintf(information," FB = 0X%04hX",FB);
            strcat(traceLine,information);
            break;

      // 0X0C    POPSB               OpCode              Pop SB
         case 0X0C: 
            strcat(traceLine,"POPSB    ");
            ReadWORDFromMainMemory(SP+2,&SB); SP += 2;
            sprintf(information," SB = 0X%04hX",SB);
            strcat(traceLine,information);
            break;

      // 0X0D    SETAAE  memory      OpCode:mode:O16     Pop key,value; add (key,value) to array in memory[EA] (Note 1)
      // Note 1: When (key,value) pair is found, the existing value is replaced with the value popped
      //    from run-time stack. When (key,value) pair is not found, the pair is stored in the next available
      //    (key,value) slot. A fatal run-time error occurs when (key,value) pair is not found and (size == capacity).

         case 0X0D: 
            {
               WORD size,capacity,keyS,valueS,keyM,valueM;
               int i;
               bool isFound;
   
               strcat(traceLine,"SETAAE   ");
               ReadBYTEFromMainMemory(PC,&mode); PC += 1;
               ReadWORDFromMainMemory(PC,&O16); PC += 2;
               EA = MemoryOperandEA(mode,O16,PC,&SP,FB,SB,information);
               strcat(traceLine,information);
               ReadWORDFromMainMemory(EA,&size);
               ReadWORDFromMainMemory(EA+2,&capacity);
               ReadWORDFromMainMemory(SP+2,&keyS); SP += 2;
               ReadWORDFromMainMemory(SP+2,&valueS); SP += 2;
               i = 1;
               isFound = false;
               while ( (i <= size) && !isFound )
               {
                  ReadWORDFromMainMemory(EA+4+4*(i-1),&keyM);
                  if ( keyM == keyS )
                     isFound = true;
                  else
                     i++;
               }
               if ( isFound )
                  WriteWORDToMainMemory(EA+4+4*(i-1)+2,valueS);
               else
               {
                  if ( size == capacity ) ProcessRunTimeError("Associative array overflow",true);
                  size++;
                  WriteWORDToMainMemory(EA,size);
                  WriteWORDToMainMemory(EA+4+4*(size-1)  ,keyS);
                  WriteWORDToMainMemory(EA+4+4*(size-1)+2,valueS);
               }
               sprintf(information,", pair = (0X%04hX,0X%04hX)",keyS,valueS);
               strcat(traceLine,information);
            }
            break;

      // 0X0E    GETAAE  memory      OpCode:mode:O16     Pop key; find (key,value) in array in memory[EA]; push value (Note 2)
      // Note 2: A fatal run-time error occurs when (key,value) pair is not found.
         case 0X0E: 
            {
               WORD size,capacity,keyS,valueS,keyM,valueM;
               int i;
               bool isFound;
   
               strcat(traceLine,"GETAAE   ");
               ReadBYTEFromMainMemory(PC,&mode); PC += 1;
               ReadWORDFromMainMemory(PC,&O16); PC += 2;
               EA = MemoryOperandEA(mode,O16,PC,&SP,FB,SB,information);
               strcat(traceLine,information);
               ReadWORDFromMainMemory(EA,&size);
               ReadWORDFromMainMemory(EA+2,&capacity);
               ReadWORDFromMainMemory(SP+2,&keyS); SP += 2;
               i = 1;
               isFound = false;
               while ( (i <= size) && !isFound )
               {
                  ReadWORDFromMainMemory(EA+4+4*(i-1),&keyM);
                  if ( keyM == keyS )
                     isFound = true;
                  else
                     i++;
               }
               if ( isFound )
               {
                  ReadWORDFromMainMemory(EA+4+4*(i-1)+2,&valueM);
                  WriteWORDToMainMemory(SP,valueM); SP -= 2;
               }
               else
                  ProcessRunTimeError("Associative array key not found",true);
               sprintf(information,", pair = (0X%04hX,0X%04hX)",keyS,valueM);
               strcat(traceLine,information);
            }
            break;

      // 0X0F    ADRAAE  memory      OpCode:mode:O16     Pop key; find (key,value) in array in memory[EA]; push address of value (Note 3)
      // Note 3: When (key,value) pair is not found, the pair is stored in the next available (key,value) slot. 
      //    A fatal run-time error occurs when (key,value) pair is not found and (size == capacity).
         case 0X0F: 
            {
               WORD size,capacity,keyS,valueS,keyM,valueM,addressValueM;
               int i;
               bool isFound;
   
               strcat(traceLine,"ADRAAE   ");
               ReadBYTEFromMainMemory(PC,&mode); PC += 1;
               ReadWORDFromMainMemory(PC,&O16); PC += 2;
               EA = MemoryOperandEA(mode,O16,PC,&SP,FB,SB,information);
               strcat(traceLine,information);
               ReadWORDFromMainMemory(EA,&size);
               ReadWORDFromMainMemory(EA+2,&capacity);
               ReadWORDFromMainMemory(SP+2,&keyS); SP += 2;
               i = 1;
               isFound = false;
               while ( (i <= size) && !isFound )
               {
                  ReadWORDFromMainMemory(EA+4+4*(i-1),&keyM);
                  if ( keyM == keyS )
                     isFound = true;
                  else
                     i++;
               }
               if ( isFound )
               {
                  addressValueM = EA+4+4*(i-1)+2;
               }
               else
               {
                  if ( size == capacity ) ProcessRunTimeError("Associative array overflow",true);
                  size++;
                  WriteWORDToMainMemory(EA,size);
                  WriteWORDToMainMemory(EA+4+4*(size-1)  ,keyS);
                  addressValueM = EA+4+4*(size-1)+2;
               }
               WriteWORDToMainMemory(SP,addressValueM); SP -= 2;
               sprintf(information,", key = 0X%04hX, address = 0X%04hX",keyS,addressValueM);
               strcat(traceLine,information);
            }
            break;

      // Note  9: Assumes that memory block pointed to by LHS is large enough to accommodate structure stored
      //    in memory block pointed to by RHS.

      // 0X10    COPYAA              OpCode              Pop RHS,LHS; memory[LHS+2*i] = memory[RHS+2*i], i in 
      //                                                    [ 0,2*capacity+1 ] (Note 9)
         case 0X10:
            {
               int i;
               WORD capacity;

               strcat(traceLine,"COPYAA   ");
               ReadWORDFromMainMemory(SP+2,&RHS); SP += 2;
               ReadWORDFromMainMemory(SP+2,&LHS); SP += 2;
               ReadWORDFromMainMemory(RHS+2,&capacity);
               for (i = 0; i <= 2*capacity+1; i++)
               {
                  ReadWORDFromMainMemory(RHS+2*i,&W16);
                  WriteWORDToMainMemory(LHS+2*i,W16);
               }
               sprintf(information," %4u words from 0X%04hX to 0X%04hX",1+(2*capacity+1),RHS,LHS);
               strcat(traceLine,information);
            }
            break;

      // Note  8: A STM string is composed of 2+capacity words of contiguous memory. Word 1 is the length of
      //    the string (the number of characters contained in the string); word 2 is the string's capacity; and 
      //    words 3-to-(capacity+2) are reserved for the string's characters. When empty, length = 0, otherwise
      //    (1 <= length <= capacity). A fatal error occurs when (1 <= index <= length) is not true for SETSE,
      //    GETSE, and ADRSE and a fatal error occurs when (length = capacity) when ADDSE begins execution.

      // 0X11    SETSE   memory      OpCode:mode:O16     Pop character,index; store character in 
      //                                                    memory[EA+4+2*(index-1)] (Note 8)
         case 0X11: 
            {
               WORD length,capacity,index,character;

               strcat(traceLine,"SETSE    ");
               ReadBYTEFromMainMemory(PC,&mode); PC += 1;
               ReadWORDFromMainMemory(PC,&O16); PC += 2;
               EA = MemoryOperandEA(mode,O16,PC,&SP,FB,SB,information);
               strcat(traceLine,information);
               ReadWORDFromMainMemory(EA,&length);
               ReadWORDFromMainMemory(EA+2,&capacity);
               ReadWORDFromMainMemory(SP+2,&character); SP += 2;
               ReadWORDFromMainMemory(SP+2,&index); SP += 2;
               if ( !((1 <= index) &&(index <= length)) ) ProcessRunTimeError("Invalid string index",true);
               WriteWORDToMainMemory(EA+4+2*(index-1),character);
               sprintf(information,", set string[0X%04hX] to '%c'",index,LOBYTE(character));
               strcat(traceLine,information);
            }
            break;

      // 0X12    GETSE   memory      OpCode:mode:O16     Pop index; push memory[EA+4+2*(index-1)] (Note 8)
         case 0X12: 
            {
               WORD length,capacity,index,character;

               strcat(traceLine,"GETSE    ");
               ReadBYTEFromMainMemory(PC,&mode); PC += 1;
               ReadWORDFromMainMemory(PC,&O16); PC += 2;
               EA = MemoryOperandEA(mode,O16,PC,&SP,FB,SB,information);
               strcat(traceLine,information);
               ReadWORDFromMainMemory(EA,&length);
               ReadWORDFromMainMemory(EA+2,&capacity);
               ReadWORDFromMainMemory(SP+2,&index); SP += 2;
               if ( !((1 <= index) &&(index <= length)) ) ProcessRunTimeError("Invalid string index",true);
               ReadWORDFromMainMemory(EA+4+2*(index-1),&character);
               WriteWORDToMainMemory(SP,character); SP -= 2;
               sprintf(information,", get string[0X%04hX] = '%c'",index,LOBYTE(character));
               strcat(traceLine,information);
            }
            break;

      // 0X13    ADRSE   memory      OpCode:mode:O16     Pop index; push address (EA+4+2*(index-1)) (Note 8)
         case 0X13: 
            {
               WORD length,capacity,index,character;

               strcat(traceLine,"ADRSE    ");
               ReadBYTEFromMainMemory(PC,&mode); PC += 1;
               ReadWORDFromMainMemory(PC,&O16); PC += 2;
               EA = MemoryOperandEA(mode,O16,PC,&SP,FB,SB,information);
               strcat(traceLine,information);
               ReadWORDFromMainMemory(EA,&length);
               ReadWORDFromMainMemory(EA+2,&capacity);
               ReadWORDFromMainMemory(SP+2,&index); SP += 2;
               if ( !((1 <= index) &&(index <= length)) ) ProcessRunTimeError("Invalid string index",true);
               WriteWORDToMainMemory(SP,EA+4+2*(index-1)); SP -= 2;
               sprintf(information,", address of string[0X%04hX] = 0X%04hX",index,EA+4+2*(index-1));
               strcat(traceLine,information);
            }
            break;

      // 0X14    ADDSE   memory      OpCode:mode:O16     Pop character; store character in memory[EA+4+2*length];
      //                                                   increment length (Note 8)
         case 0X14: 
            {
               WORD length,capacity,index,character;

               strcat(traceLine,"ADDSE    ");
               ReadBYTEFromMainMemory(PC,&mode); PC += 1;
               ReadWORDFromMainMemory(PC,&O16); PC += 2;
               EA = MemoryOperandEA(mode,O16,PC,&SP,FB,SB,information);
               strcat(traceLine,information);
               ReadWORDFromMainMemory(EA,&length);
               ReadWORDFromMainMemory(EA+2,&capacity);
               ReadWORDFromMainMemory(SP+2,&character); SP += 2;
               if ( length == capacity ) ProcessRunTimeError("String overflow",true);
               length++;
               WriteWORDToMainMemory(EA+4+2*(length-1),character);
               WriteWORDToMainMemory(EA,length);
               sprintf(information,", add string[0X%04hX] to '%c'",length,LOBYTE(character));
               strcat(traceLine,information);
            }
            break;

      // Note  9: Assumes that memory block pointed to by LHS is large enough to accommodate structure stored
      //    in memory block pointed to by RHS.

      // 0X15    COPYS               OpCode              Pop RHS,LHS; memory[LHS+2*i] = memory[RHS+2*i], i in 
      //                                                   [ 0,capacity+1 ] (Note 9)
         case 0X15:
            {
               int i;
               WORD capacity;

               strcat(traceLine,"COPYS    ");
               ReadWORDFromMainMemory(SP+2,&RHS); SP += 2;
               ReadWORDFromMainMemory(SP+2,&LHS); SP += 2;
               ReadWORDFromMainMemory(RHS+2,&capacity);
               for (i = 0; i <= capacity+1; i++)
               {
                  ReadWORDFromMainMemory(RHS+2*i,&W16);
                  WriteWORDToMainMemory(LHS+2*i,W16);
               }
               sprintf(information," %4u words from 0X%04hX to 0X%04hX",1+(capacity+1),RHS,LHS);
               strcat(traceLine,information);
            }
            break;

      // Note 12: Assumes that memory block pointed to by RES is large enough to accommodate the concatenation
      //    of strings pointed to by LHS and RHS. A fatal error occurs when 
      //    (length-of-LHS + length-of-RHS) > capacity-of-RES

      // 0X1D    CONCATS             OpCode              Pop RES,RHS,LHS; memory[RES] = memory[LHS] concatenate memory[RHS];
      //                                                    push RES (Note 12)
         case 0X1D:
            {
               int i;
               WORD RES,capacityRES,lengthLHS,lengthRHS;
               
               strcat(traceLine,"CONCATS  ");
               ReadWORDFromMainMemory(SP+2,&RES); SP += 2;
               ReadWORDFromMainMemory(SP+2,&RHS); SP += 2;
               ReadWORDFromMainMemory(SP+2,&LHS); SP += 2;
               ReadWORDFromMainMemory(RES+2,&capacityRES);
               ReadWORDFromMainMemory(RHS,&lengthRHS);
               ReadWORDFromMainMemory(LHS,&lengthLHS);
               if ( lengthRHS+lengthLHS > capacityRES ) ProcessRunTimeError("String overflow",true);
               WriteWORDToMainMemory(RES,lengthRHS+lengthLHS);
               for (i = 0; i <= lengthLHS-1; i++)
               {
                  ReadWORDFromMainMemory(LHS+4+2*i,&W16);
                  WriteWORDToMainMemory(RES+4+2*i,W16);
               }
               for (i = 0; i <= lengthRHS-1; i++)
               {
                  ReadWORDFromMainMemory(RHS+4+2*i,&W16);
                  WriteWORDToMainMemory(RES+4+2*(i+lengthLHS),W16);
               }
               WriteWORDToMainMemory(SP,RES); SP -= 2;
               sprintf(information," 0X%04hX = 0X%04hX + 0X%04hX (%4u words)",RES,LHS,RHS,lengthLHS+lengthRHS);
               strcat(traceLine,information);
            }
            break;

      // Note 10: A fatal error occurs when the following equation is not true for SETAE, GETAE, and ADRAE.
      //    ((LB1 <= index1 <= UB1) AND (LB2 <= index2 <= UB2) AND...AND (LBn <= indexn <= UBn))

      // 0X16    SETAE   memory      OpCode:mode:O16     Pop value,index(n),index(n-1),...,index(1); store value at offset
      //                                                    in array (Note 10)
         case 0X16:
            {
               int i,offset;
               WORD n,capacity,indexi,productOfSizes;
               WORD LBi,UBi,value;

               strcat(traceLine,"SETAE    ");
               ReadBYTEFromMainMemory(PC,&mode); PC += 1;
               ReadWORDFromMainMemory(PC,&O16); PC += 2;
               EA = MemoryOperandEA(mode,O16,PC,&SP,FB,SB,information);
               strcat(traceLine,information);
               ReadWORDFromMainMemory(EA,&n);
               ReadWORDFromMainMemory(SP+2,&value); SP += 2;
               offset = 0;
               productOfSizes = 1;
               for (i = n; i >= 1; i--)
               {
                  ReadWORDFromMainMemory(SP+2,&indexi); SP += 2;
                  ReadWORDFromMainMemory(EA+2*(2*i-1),&LBi);
                  ReadWORDFromMainMemory(EA+2*(2*i+0),&UBi);
                  if ( !((SIGNED(LBi) <= SIGNED(indexi)) && (SIGNED(indexi) <= SIGNED(UBi))) )
                     ProcessRunTimeError("Invalid array index",true);
                  offset += productOfSizes*(SIGNED(indexi)-SIGNED(LBi));
                  productOfSizes *= (SIGNED(UBi)-SIGNED(LBi)+1);
               }
               WriteWORDToMainMemory(EA+2*(1+2*n+offset),value);
               sprintf(information,", set 0X%04hX at 0X%04hX (offset %d)",value,EA+2*(1+2*n+offset),offset);
               strcat(traceLine,information);
            }
            break;

      // 0X17    GETAE   memory      OpCode:mode:O16     Pop index(n),index(n-1),...,index(1); push value found at offset
      //                                                    in array (Note 10)
         case 0X17:
            {
               int i,offset;
               WORD n,capacity,indexi,productOfSizes;
               WORD LBi,UBi,value;

               strcat(traceLine,"GETAE    ");
               ReadBYTEFromMainMemory(PC,&mode); PC += 1;
               ReadWORDFromMainMemory(PC,&O16); PC += 2;
               EA = MemoryOperandEA(mode,O16,PC,&SP,FB,SB,information);
               strcat(traceLine,information);
               ReadWORDFromMainMemory(EA,&n);
               offset = 0;
               productOfSizes = 1;
               for (i = n; i >= 1; i--)
               {
                  ReadWORDFromMainMemory(SP+2,&indexi); SP += 2;
                  ReadWORDFromMainMemory(EA+2*(2*i-1),&LBi);
                  ReadWORDFromMainMemory(EA+2*(2*i+0),&UBi);
                  if ( !((SIGNED(LBi) <= SIGNED(indexi)) && (SIGNED(indexi) <= SIGNED(UBi))) )
                     ProcessRunTimeError("Invalid array index",true);
                  offset += productOfSizes*(SIGNED(indexi)-SIGNED(LBi));
                  productOfSizes *= (SIGNED(UBi)-SIGNED(LBi)+1);
               }
               ReadWORDFromMainMemory(EA+2*(1+2*n+offset),&value);
               WriteWORDToMainMemory(SP,value); SP -= 2;
               sprintf(information,", get 0X%04hX at 0X%04hX (offset %d)",value,EA+2*(1+2*n+offset),offset);
               strcat(traceLine,information);
            }
            break;

      // 0X18    ADRAE   memory      OpCode:mode:O16     Pop index(n),index(n-1),...,index(1); push address of value found
      //                                                    at offset in array (Note 10)
         case 0X18:
            {
               int i,offset;
               WORD n,capacity,indexi,productOfSizes;
               WORD LBi,UBi;

               strcat(traceLine,"ADRAE    ");
               ReadBYTEFromMainMemory(PC,&mode); PC += 1;
               ReadWORDFromMainMemory(PC,&O16); PC += 2;
               EA = MemoryOperandEA(mode,O16,PC,&SP,FB,SB,information);
               strcat(traceLine,information);
               ReadWORDFromMainMemory(EA,&n);
               offset = 0;
               productOfSizes = 1;
               for (i = n; i >= 1; i--)
               {
                  ReadWORDFromMainMemory(SP+2,&indexi); SP += 2;
                  ReadWORDFromMainMemory(EA+2*(2*i-1),&LBi);
                  ReadWORDFromMainMemory(EA+2*(2*i+0),&UBi);
                  if ( !((SIGNED(LBi) <= SIGNED(indexi)) && (SIGNED(indexi) <= SIGNED(UBi))) )
                     ProcessRunTimeError("Invalid array index",true);
                  offset += productOfSizes*(SIGNED(indexi)-SIGNED(LBi));
                  productOfSizes *= (SIGNED(UBi)-SIGNED(LBi)+1);
               }
               WriteWORDToMainMemory(SP,EA+2*(1+2*n+offset)); SP -= 2;
               sprintf(information,", address of array value (offset %d) = 0X%04hX",offset,EA+2*(1+2*n+offset));
               strcat(traceLine,information);
            }
            break;

      // Note  9: Assumes that memory block pointed to by LHS is large enough to accommodate structure stored
      //    in memory block pointed to by RHS.

      // 0X19    COPYA               OpCode              Pop RHS,LHS; memory[LHS+2*i] = memory[RHS+2*i], i in 
      //                                                    [ 0,2*n+capacity ] (Note 9)
         case 0X19:
            {
               int i;
               WORD n,capacity;
               WORD LBi,UBi;

               strcat(traceLine,"COPYA    ");
               ReadWORDFromMainMemory(SP+2,&RHS); SP += 2;
               ReadWORDFromMainMemory(SP+2,&LHS); SP += 2;
               ReadWORDFromMainMemory(RHS,&n);
               capacity = 1;
               for (i = n; i >= 1; i--)
               {
                  ReadWORDFromMainMemory(RHS+2*(2*i-1),&LBi);
                  ReadWORDFromMainMemory(RHS+2*(2*i+0),&UBi);
                  capacity *= (SIGNED(UBi)-SIGNED(LBi)+1);
               }
               for (i = 0; i <= 2*n+capacity; i++)
               {
                  ReadWORDFromMainMemory(RHS+2*i,&W16);
                  WriteWORDToMainMemory(LHS+2*i,W16);
               }
               sprintf(information," %4u words from 0X%04hX to 0X%04hX",1+(2*n+capacity),RHS,LHS);
               strcat(traceLine,information);
            }
            break;

      // 0X1A    GETAN   memory      OpCode:mode:O16     Push n (# of dimensions)
         case 0X1A: 
            {
               WORD n;

               strcat(traceLine,"GETAN    ");
               ReadBYTEFromMainMemory(PC,&mode); PC += 1;
               ReadWORDFromMainMemory(PC,&O16); PC += 2;
               EA = MemoryOperandEA(mode,O16,PC,&SP,FB,SB,information);
               strcat(traceLine,information);
               ReadWORDFromMainMemory(EA,&n);
               WriteWORDToMainMemory(SP,n); SP -= 2;
               sprintf(information,", # dimensions n = %u",n);
               strcat(traceLine,information);
            }
            break;

      // Note 11: A fatal error occurs when dimension # i is not in [ 1,n ].
      // 0X1B    GETALB  memory      OpCode:mode:O16     Pop dimension # i; push lower-bound, LBi (Note 11)
         case 0X1B: 
            {
               WORD n,i,LBi;

               strcat(traceLine,"GETALB   ");
               ReadBYTEFromMainMemory(PC,&mode); PC += 1;
               ReadWORDFromMainMemory(PC,&O16); PC += 2;
               EA = MemoryOperandEA(mode,O16,PC,&SP,FB,SB,information);
               strcat(traceLine,information);
               ReadWORDFromMainMemory(SP+2,&i); SP += 2;
               ReadWORDFromMainMemory(EA,&n);
               if ( !( (1 <= SIGNED(i)) && (SIGNED(i) <= SIGNED(n)) ) )
                  ProcessRunTimeError("Invalid array dimension #",true);
               ReadWORDFromMainMemory(EA+2*(2*i-1),&LBi);
               WriteWORDToMainMemory(SP,LBi); SP -= 2;
               sprintf(information,", LB#%u = %u",i,LBi);
               strcat(traceLine,information);
            }
            break;

      // 0X1C    GETAUB  memory      OpCode:mode:O16     Pop dimension # i; push upper-bound, UBi (Note 11)
         case 0X1C: 
            {
               WORD n,i,UBi;

               strcat(traceLine,"GETAUB   ");
               ReadBYTEFromMainMemory(PC,&mode); PC += 1;
               ReadWORDFromMainMemory(PC,&O16); PC += 2;
               EA = MemoryOperandEA(mode,O16,PC,&SP,FB,SB,information);
               strcat(traceLine,information);
               ReadWORDFromMainMemory(SP+2,&i); SP += 2;
               ReadWORDFromMainMemory(EA,&n);
               if ( !( (1 <= SIGNED(i)) && (SIGNED(i) <= SIGNED(n)) ) )
                  ProcessRunTimeError("Invalid array dimension #",true);
               ReadWORDFromMainMemory(EA+2*(2*i+0),&UBi);
               WriteWORDToMainMemory(SP,UBi); SP -= 2;
               sprintf(information,", UB#%u = %u",i,UBi);
               strcat(traceLine,information);
            }
            break;

      // 0X20    ADDI                OpCode              Pop RHS,LHS; push integer ( LHS+RHS )
         case 0X20: 
            strcat(traceLine,"ADDI     ");
            ReadWORDFromMainMemory(SP+2,&RHS); SP += 2;
            ReadWORDFromMainMemory(SP+2,&LHS); SP += 2;
            TOS = UNSIGNED(SIGNED(LHS)+SIGNED(RHS));
            WriteWORDToMainMemory(SP,TOS); SP -= 2;
            sprintf(information," 0X%04hX = 0X%04hX + 0X%04hX",TOS,LHS,RHS);
            strcat(traceLine,information);
            break;

      // 0X21    ADDF                OpCode              Pop RHS,LHS; push   float ( LHS+RHS )
         case 0X21: 
            {
               float FLHS,FRHS;
               char base10TOS[80+1],base10LHS[80+1],base10RHS[80+1];
               
               strcat(traceLine,"ADDF     ");
               ReadWORDFromMainMemory(SP+2,&RHS); SP += 2;
               ReadWORDFromMainMemory(SP+2,&LHS); SP += 2;
               ConvertHalfFloatToFloat(LHS,&FLHS);
               ConvertHalfFloatToFloat(RHS,&FRHS);
               ConvertFloatToHalfFloat((FLHS+FRHS),&TOS);
               WriteWORDToMainMemory(SP,TOS); SP -= 2;
               ConvertHalfFloatToBase10(TOS,base10TOS);
               ConvertHalfFloatToBase10(LHS,base10LHS);
               ConvertHalfFloatToBase10(RHS,base10RHS);
               sprintf(information," %s = %s + %s",base10TOS,base10LHS,base10RHS);
               strcat(traceLine,information);
            }
            break;

      // 0X22    SUBI                OpCode              Pop RHS,LHS; push integer ( LHS-RHS )
         case 0X22: 
            strcat(traceLine,"SUBI     ");
            ReadWORDFromMainMemory(SP+2,&RHS); SP += 2;
            ReadWORDFromMainMemory(SP+2,&LHS); SP += 2;
            TOS = UNSIGNED(SIGNED(LHS)-SIGNED(RHS));
            WriteWORDToMainMemory(SP,TOS); SP -= 2;
            sprintf(information," 0X%04hX = 0X%04hX - 0X%04hX",TOS,LHS,RHS);
            strcat(traceLine,information);
            break;

      // 0X23    SUBF                OpCode              Pop RHS,LHS; push   float ( LHS-RHS )
         case 0X23: 
            {
               float FLHS,FRHS;
               char base10TOS[80+1],base10LHS[80+1],base10RHS[80+1];
               
               strcat(traceLine,"SUBF     ");
               ReadWORDFromMainMemory(SP+2,&RHS); SP += 2;
               ReadWORDFromMainMemory(SP+2,&LHS); SP += 2;
               ConvertHalfFloatToFloat(LHS,&FLHS);
               ConvertHalfFloatToFloat(RHS,&FRHS);
               ConvertFloatToHalfFloat((FLHS-FRHS),&TOS);
               WriteWORDToMainMemory(SP,TOS); SP -= 2;
               ConvertHalfFloatToBase10(TOS,base10TOS);
               ConvertHalfFloatToBase10(LHS,base10LHS);
               ConvertHalfFloatToBase10(RHS,base10RHS);
               sprintf(information," %s = %s - %s",base10TOS,base10LHS,base10RHS);
               strcat(traceLine,information);
            }
            break;

      // 0X24    MULI                OpCode              Pop RHS,LHS; push integer ( LHS*RHS )
         case 0X24: 
            strcat(traceLine,"MULI     ");
            ReadWORDFromMainMemory(SP+2,&RHS); SP += 2;
            ReadWORDFromMainMemory(SP+2,&LHS); SP += 2;
            TOS = UNSIGNED(SIGNED(LHS)*SIGNED(RHS));
            WriteWORDToMainMemory(SP,TOS); SP -= 2;
            sprintf(information," 0X%04hX = 0X%04hX * 0X%04hX",TOS,LHS,RHS);
            strcat(traceLine,information);
            break;

      // 0X25    MULF                OpCode              Pop RHS,LHS; push   float ( LHS*RHS )
         case 0X25: 
            {
               float FLHS,FRHS;
               char base10TOS[80+1],base10LHS[80+1],base10RHS[80+1];
               
               strcat(traceLine,"MULF     ");
               ReadWORDFromMainMemory(SP+2,&RHS); SP += 2;
               ReadWORDFromMainMemory(SP+2,&LHS); SP += 2;
               ConvertHalfFloatToFloat(LHS,&FLHS);
               ConvertHalfFloatToFloat(RHS,&FRHS);
               ConvertFloatToHalfFloat((FLHS*FRHS),&TOS);
               WriteWORDToMainMemory(SP,TOS); SP -= 2;
               ConvertHalfFloatToBase10(TOS,base10TOS);
               ConvertHalfFloatToBase10(LHS,base10LHS);
               ConvertHalfFloatToBase10(RHS,base10RHS);
               sprintf(information," %s = %s * %s",base10TOS,base10LHS,base10RHS);
               strcat(traceLine,information);
            }
            break;

      // 0X26    DIVI                OpCode              Pop RHS,LHS; push integer ( LHS÷RHS )
         case 0X26: 
            strcat(traceLine,"DIVI     ");
            ReadWORDFromMainMemory(SP+2,&RHS); SP += 2;
            ReadWORDFromMainMemory(SP+2,&LHS); SP += 2;
            TOS = UNSIGNED(SIGNED(LHS)/SIGNED(RHS));
            WriteWORDToMainMemory(SP,TOS); SP -= 2;
            sprintf(information," 0X%04hX = 0X%04hX / 0X%04hX",TOS,LHS,RHS);
            strcat(traceLine,information);
            break;

      // 0X27    DIVF                OpCode              Pop RHS,LHS; push   float ( LHS÷RHS )
         case 0X27: 
            {
               float FLHS,FRHS;
               char base10TOS[80+1],base10LHS[80+1],base10RHS[80+1];
               
               strcat(traceLine,"DIVF     ");
               ReadWORDFromMainMemory(SP+2,&RHS); SP += 2;
               ReadWORDFromMainMemory(SP+2,&LHS); SP += 2;
               ConvertHalfFloatToFloat(LHS,&FLHS);
               ConvertHalfFloatToFloat(RHS,&FRHS);
               ConvertFloatToHalfFloat((FLHS/FRHS),&TOS);
               WriteWORDToMainMemory(SP,TOS); SP -= 2;
               ConvertHalfFloatToBase10(TOS,base10TOS);
               ConvertHalfFloatToBase10(LHS,base10LHS);
               ConvertHalfFloatToBase10(RHS,base10RHS);
               sprintf(information," %s = %s / %s",base10TOS,base10LHS,base10RHS);
               strcat(traceLine,information);
            }
            break;

      // 0X28    REMI                OpCode              Pop RHS,LHS; push integer ( LHS rem RHS )
         case 0X28: 
            strcat(traceLine,"REMI     ");
            ReadWORDFromMainMemory(SP+2,&RHS); SP += 2;
            ReadWORDFromMainMemory(SP+2,&LHS); SP += 2;
            TOS = UNSIGNED(SIGNED(LHS)%SIGNED(RHS));
            WriteWORDToMainMemory(SP,TOS); SP -= 2;
            sprintf(information," 0X%04hX = 0X%04hX % 0X%04hX",TOS,LHS,RHS);
            strcat(traceLine,information);
            break;

      // 0X29    POWI                OpCode              Pop RHS,LHS; push integer pow(LHS,RHS)
         case 0X29: 
            strcat(traceLine,"POWI     ");
            {
               int p,i;

               ReadWORDFromMainMemory(SP+2,&RHS); SP += 2;
               ReadWORDFromMainMemory(SP+2,&LHS); SP += 2;
            
               if ( SIGNED(RHS) < 0 )
                  p = 0;
               else
               {
                  p = 1;
                  for (i = 1; i <= SIGNED(RHS); i++)
                     p = p*SIGNED(LHS);
               }
               TOS = UNSIGNED(p);
            }
            WriteWORDToMainMemory(SP,TOS); SP -= 2;
            sprintf(information," 0X%04hX = 0X%04hX ^ 0X%04hX",TOS,LHS,RHS);
            strcat(traceLine,information);
            break;

      // 0X2A    POWF                OpCode              Pop RHS,LHS; push   float pow(LHS,RHS)
         case 0X2A: 
            {
               float FLHS,FRHS;
               char base10TOS[80+1],base10LHS[80+1],base10RHS[80+1];
               
               strcat(traceLine,"POWF     ");
               ReadWORDFromMainMemory(SP+2,&RHS); SP += 2;
               ReadWORDFromMainMemory(SP+2,&LHS); SP += 2;
               ConvertHalfFloatToFloat(LHS,&FLHS);
               ConvertHalfFloatToFloat(RHS,&FRHS);
               ConvertFloatToHalfFloat((float) pow(FLHS,FRHS),&TOS);
               WriteWORDToMainMemory(SP,TOS); SP -= 2;
               ConvertHalfFloatToBase10(TOS,base10TOS);
               ConvertHalfFloatToBase10(LHS,base10LHS);
               ConvertHalfFloatToBase10(RHS,base10RHS);
               sprintf(information," %s = %s ^ %s",base10TOS,base10LHS,base10RHS);
               strcat(traceLine,information);
            }
            break;

      // 0X2B    NEGI                OpCode              Pop RHS; push integer -RHS
         case 0X2B: 
            strcat(traceLine,"NEGI     ");
            ReadWORDFromMainMemory(SP+2,&RHS); SP += 2;
            TOS = UNSIGNED(-SIGNED(RHS));
            WriteWORDToMainMemory(SP,TOS); SP -= 2;
            sprintf(information," 0X%04hX = -(0X%04hX)",TOS,RHS);
            strcat(traceLine,information);
            break;

      // 0X2C    NEGF                OpCode              Pop RHS; push   float -RHS
         case 0X2C: 
            {
               float FRHS;
               char base10TOS[80+1],base10RHS[80+1];
               
               strcat(traceLine,"NEGF     ");
               ReadWORDFromMainMemory(SP+2,&RHS); SP += 2;
               ConvertHalfFloatToFloat(RHS,&FRHS);
               ConvertFloatToHalfFloat(-FRHS,&TOS);
               WriteWORDToMainMemory(SP,TOS); SP -= 2;
               ConvertHalfFloatToBase10(TOS,base10TOS);
               ConvertHalfFloatToBase10(RHS,base10RHS);
               sprintf(information," %s = -(%s)",base10TOS,base10RHS);
               strcat(traceLine,information);
            }
            break;

      // 0X2D    AND                 OpCode              Pop RHS,LHS; push boolean ( LHS  and RHS )
         case 0X2D: 
            strcat(traceLine,"AND      ");
            ReadWORDFromMainMemory(SP+2,&RHS); SP += 2;
            ReadWORDFromMainMemory(SP+2,&LHS); SP += 2;
            if ( (LHS == 0XFFFFu) && (RHS == 0XFFFFu) )
               TOS = 0XFFFFu;
            else
               TOS = 0X0000u;
            WriteWORDToMainMemory(SP,TOS); SP -= 2;
            sprintf(information," %c = %c AND %c",TORF(TOS),TORF(LHS),TORF(RHS));
            strcat(traceLine,information);
            break;

      // 0X2E    NAND                OpCode              Pop RHS,LHS; push boolean ( LHS nand RHS )
         case 0X2E: 
            strcat(traceLine,"NAND     ");
            ReadWORDFromMainMemory(SP+2,&RHS); SP += 2;
            ReadWORDFromMainMemory(SP+2,&LHS); SP += 2;
            if ( (LHS == 0XFFFFu) && (RHS == 0XFFFFu) )
               TOS = 0X0000u;
            else
               TOS = 0XFFFFu;
            WriteWORDToMainMemory(SP,TOS); SP -= 2;
            sprintf(information," %c = %c NAND %c",TORF(TOS),TORF(LHS),TORF(RHS));
            strcat(traceLine,information);
            break;

      // 0X2F    OR                  OpCode              Pop RHS,LHS; push boolean ( LHS   or RHS )
         case 0X2F: 
            strcat(traceLine,"OR       ");
            ReadWORDFromMainMemory(SP+2,&RHS); SP += 2;
            ReadWORDFromMainMemory(SP+2,&LHS); SP += 2;
            if ( (LHS == 0X0000u) && (RHS == 0X0000u) )
               TOS = 0X0000u;
            else
               TOS = 0XFFFFu;
            WriteWORDToMainMemory(SP,TOS); SP -= 2;
            sprintf(information," %c = %c OR %c",TORF(TOS),TORF(LHS),TORF(RHS));
            strcat(traceLine,information);
            break;

      // 0X30    NOR                 OpCode              Pop RHS,LHS; push boolean ( LHS  nor RHS )
         case 0X30: 
            strcat(traceLine,"NOR      ");
            ReadWORDFromMainMemory(SP+2,&RHS); SP += 2;
            ReadWORDFromMainMemory(SP+2,&LHS); SP += 2;
            if ( (LHS == 0X0000u) && (RHS == 0X0000u) )
               TOS = 0XFFFFu;
            else
               TOS = 0X0000u;
            WriteWORDToMainMemory(SP,TOS); SP -= 2;
            sprintf(information," %c = %c NOR %c",TORF(TOS),TORF(LHS),TORF(RHS));
            strcat(traceLine,information);
            break;

      // 0X31    XOR                 OpCode              Pop RHS,LHS; push boolean ( LHS  xor RHS )
         case 0X31: 
            strcat(traceLine,"XOR      ");
            ReadWORDFromMainMemory(SP+2,&RHS); SP += 2;
            ReadWORDFromMainMemory(SP+2,&LHS); SP += 2;
            if ( ((LHS == 0XFFFFu) && (RHS == 0X0000u)) || ((LHS == 0X0000u) && (RHS == 0XFFFFu)) )
               TOS = 0XFFFFu;
            else
               TOS = 0X0000u;
            WriteWORDToMainMemory(SP,TOS); SP -= 2;
            sprintf(information," %c = %c XOR %c",TORF(TOS),TORF(LHS),TORF(RHS));
            strcat(traceLine,information);
            break;

      // 0X32    NXOR                OpCode              Pop RHS,LHS; push boolean ( LHS nxor RHS )
         case 0X32: 
            strcat(traceLine,"NXOR     ");
            ReadWORDFromMainMemory(SP+2,&RHS); SP += 2;
            ReadWORDFromMainMemory(SP+2,&LHS); SP += 2;
            if ( ((LHS == 0XFFFFu) && (RHS == 0X0000u)) || ((LHS == 0X0000u) && (RHS == 0XFFFFu)) )
               TOS = 0X0000u;
            else
               TOS = 0XFFFFu;
            WriteWORDToMainMemory(SP,TOS); SP -= 2;
            sprintf(information," %c = %c NXOR %c",TORF(TOS),TORF(LHS),TORF(RHS));
            strcat(traceLine,information);
            break;

      // 0X33    NOT                 OpCode              Pop RHS; push boolean ( not RHS )
         case 0X33: 
            strcat(traceLine,"NOT      ");
            ReadWORDFromMainMemory(SP+2,&RHS); SP += 2;
            if ( RHS == 0X0000u )
               TOS = 0XFFFFu;
            else
               TOS = 0X0000u;
            WriteWORDToMainMemory(SP,TOS); SP -= 2;
            sprintf(information," %c = NOT %c",TORF(TOS),TORF(RHS));
            strcat(traceLine,information);
            break;

      // 0X34    BITAND              OpCode              Pop RHS,LHS; push boolean ( LHS bitwise-AND  RHS )
         case 0X34: 
            strcat(traceLine,"BITAND   ");
            ReadWORDFromMainMemory(SP+2,&RHS); SP += 2;
            ReadWORDFromMainMemory(SP+2,&LHS); SP += 2;
             TOS = LHS&RHS;
            WriteWORDToMainMemory(SP,TOS); SP -= 2;
            sprintf(information," 0X%04hX = 0X%04hX bitwise-AND 0X%04hX",TOS,LHS,RHS);
            strcat(traceLine,information);
            break;

      // 0X35    BITNAND             OpCode              Pop RHS,LHS; push boolean ( LHS bitwise-NAND RHS )
         case 0X35: 
            strcat(traceLine,"BITNAND  ");
            ReadWORDFromMainMemory(SP+2,&RHS); SP += 2;
            ReadWORDFromMainMemory(SP+2,&LHS); SP += 2;
            TOS = ~(LHS&RHS);
            WriteWORDToMainMemory(SP,TOS); SP -= 2;
            sprintf(information," 0X%04hX = 0X%04hX bitwise-NAND 0X%04hX",TOS,LHS,RHS);
            strcat(traceLine,information);
            break;

      // 0X36    BITOR               OpCode              Pop RHS,LHS; push boolean ( LHS bitwise-OR   RHS )
         case 0X36: 
            strcat(traceLine,"BITOR    ");
            ReadWORDFromMainMemory(SP+2,&RHS); SP += 2;
            ReadWORDFromMainMemory(SP+2,&LHS); SP += 2;
            TOS = LHS|RHS;
            WriteWORDToMainMemory(SP,TOS); SP -= 2;
            sprintf(information," 0X%04hX = 0X%04hX bitwise-OR 0X%04hX",TOS,LHS,RHS);
            strcat(traceLine,information);
            break;

      // 0X37    BITNOR              OpCode              Pop RHS,LHS; push boolean ( LHS bitwise-NOR  RHS )
         case 0X37: 
            strcat(traceLine,"BITNOR   ");
            ReadWORDFromMainMemory(SP+2,&RHS); SP += 2;
            ReadWORDFromMainMemory(SP+2,&LHS); SP += 2;
            TOS = ~(LHS|RHS);
            WriteWORDToMainMemory(SP,TOS); SP -= 2;
            sprintf(information," 0X%04hX = 0X%04hX bitwise-NOR 0X%04hX",TOS,LHS,RHS);
            strcat(traceLine,information);
            break;

      // 0X38    BITXOR              OpCode              Pop RHS,LHS; push boolean ( LHS bitwise-XOR  RHS )
         case 0X38: 
            strcat(traceLine,"BITXOR   ");
            ReadWORDFromMainMemory(SP+2,&RHS); SP += 2;
            ReadWORDFromMainMemory(SP+2,&LHS); SP += 2;
            TOS = LHS^RHS;
            WriteWORDToMainMemory(SP,TOS); SP -= 2;
            sprintf(information," 0X%04hX = 0X%04hX bitwise-XOR 0X%04hX",TOS,LHS,RHS);
            strcat(traceLine,information);
            break;

      // 0X39    BITNXOR             OpCode              Pop RHS,LHS; push boolean ( LHS bitwise-NXOR RHS )
         case 0X39: 
            strcat(traceLine,"BITNXOR  ");
            ReadWORDFromMainMemory(SP+2,&RHS); SP += 2;
            ReadWORDFromMainMemory(SP+2,&LHS); SP += 2;
            TOS = ~(LHS^RHS);
            WriteWORDToMainMemory(SP,TOS); SP -= 2;
            sprintf(information," 0X%04hX = 0X%04hX bitwise-NXOR 0X%04hX",TOS,LHS,RHS);
            strcat(traceLine,information);
            break;

      // 0X3A    BITNOT              OpCode              Pop RHS; push boolean ( bitwise-NOT RHS )
         case 0X3A: 
            strcat(traceLine,"BITNOT   ");
            ReadWORDFromMainMemory(SP+2,&RHS); SP += 2;
            TOS = ~RHS;
            WriteWORDToMainMemory(SP,TOS); SP -= 2;
            sprintf(information," 0X%04hX = bitwise-NOT 0X%04hX",TOS,RHS);
            strcat(traceLine,information);
            break;

      // 0X3B    BITSL   #W16        OpCode:O16          Pop LHS; push ( LHS shifted-left O16U bits                 )
         case 0X3B: 
            strcat(traceLine,"BITSL    ");
            ReadWORDFromMainMemory(PC,&O16); PC += 2;
            ReadWORDFromMainMemory(SP+2,&LHS); SP += 2;
            TOS = LHS << O16;
            WriteWORDToMainMemory(SP,TOS); SP -= 2;
            sprintf(information," 0X%04hX = 0X%04hX SL %5hd",TOS,RHS,O16);
            strcat(traceLine,information);
            break;

      // 0X3C    BITLSR  #W16        OpCode:O16          Pop LHS; push ( LHS logically shifted-right O16U bits      )
         case 0X3C: 
            strcat(traceLine,"BITLSR   ");
            ReadWORDFromMainMemory(PC,&O16); PC += 2;
            ReadWORDFromMainMemory(SP+2,&LHS); SP += 2;
            TOS = LHS >> O16;
            WriteWORDToMainMemory(SP,TOS); SP -= 2;
            sprintf(information," 0X%04hX = 0X%04hX SL %5hd",TOS,RHS,O16);
            strcat(traceLine,information);
            break;

      // 0X3D    BITASR  #W16        OpCode:O16          Pop LHS; push ( LHS arithmetically shifted-right O16U bits )
         case 0X3D: 
            strcat(traceLine,"BITASR   ");
            ReadWORDFromMainMemory(PC,&O16); PC += 2;
            ReadWORDFromMainMemory(SP+2,&LHS); SP += 2;
            TOS = UNSIGNED(SIGNED(LHS) >> O16);
            WriteWORDToMainMemory(SP,TOS); SP -= 2;
            sprintf(information," 0X%04hX = 0X%04hX SL %5hd",TOS,RHS,O16);
            strcat(traceLine,information);
            break;

      // 0X60    CITOF               OpCode              Pop integer RHS; push   float RHS (integer-to-float)
         case 0X60: 
            {
               char base10TOS[80+1];
                    
               strcat(traceLine,"CITOF    ");
               ReadWORDFromMainMemory(SP+2,&TOS); SP += 2;
               ConvertFloatToHalfFloat((float) SIGNED(TOS),&TOS);
               ConvertHalfFloatToBase10(TOS,base10TOS);
               WriteWORDToMainMemory(SP,TOS); SP -= 2;
               sprintf(information," TOS = %s",base10TOS);
               strcat(traceLine,information);
            }
            break;

      // 0X61    CFTOI               OpCode              Pop   float RHS; push integer RHS (float-to-integer)
         case 0X61:
            {
               float FTOS;
               char base10TOS[80+1];

               strcat(traceLine,"CFTOI    ");
               ReadWORDFromMainMemory(SP+2,&TOS); SP += 2;
               ConvertHalfFloatToFloat(TOS,&FTOS);
               TOS = (WORD) FTOS;
               WriteWORDToMainMemory(SP,TOS); SP -= 2;
               sprintf(information," TOS = 0X%04hX",TOS);
               strcat(traceLine,information);
            }
            break;

      // 0X70    CMPI                OpCode              Pop RHS,LHS; set LEG in FLAGS based on ( LHS ? RHS ) (integer)
         case 0X70: 
            strcat(traceLine,"CMPI     ");
            ReadWORDFromMainMemory(SP+2,&RHS); SP += 2;
            ReadWORDFromMainMemory(SP+2,&LHS); SP += 2;
            L = (SIGNED(LHS)  < SIGNED(RHS)) ? 1 : 0;
            E = (SIGNED(LHS) == SIGNED(RHS)) ? 1 : 0;
            G = (SIGNED(LHS)  > SIGNED(RHS)) ? 1 : 0;
            sprintf(information," 0X%04hX ? 0X%04hX LEG = %d%d%d",LHS,RHS,L,E,G);
            strcat(traceLine,information);
            break;

      // 0X71    CMPF                OpCode              Pop RHS,LHS; set LEG in FLAGS based on ( LHS ? RHS ) (float)
         case 0X71: 
            {
               float FLHS,FRHS;
               char base10TOS[80+1],base10LHS[80+1],base10RHS[80+1];
               
               strcat(traceLine,"CMPF     ");
               ReadWORDFromMainMemory(SP+2,&RHS); SP += 2;
               ReadWORDFromMainMemory(SP+2,&LHS); SP += 2;
               ConvertHalfFloatToFloat(LHS,&FLHS);
               ConvertHalfFloatToFloat(RHS,&FRHS);
               L = (FLHS  < FRHS) ? 1 : 0;
               E = (FLHS == FRHS) ? 1 : 0;
               G = (FLHS  > FRHS) ? 1 : 0;
               ConvertHalfFloatToBase10(LHS,base10LHS);
               ConvertHalfFloatToBase10(RHS,base10RHS);
               sprintf(information," %s ? %s LEG = %d%d%d",base10LHS,base10RHS,L,E,G);
               strcat(traceLine,information);
            }
            break;

      // 0X72    SETNZPI             OpCode              Set NZP in FLAGS based on sign of TOS (integer)
         case 0X72: 
            strcat(traceLine,"SETNZPI  ");
            ReadWORDFromMainMemory(SP+2,&TOS);
            N = (SIGNED(TOS)  < 0) ? 1 : 0;
            Z = (SIGNED(TOS) == 0) ? 1 : 0;
            P = (SIGNED(TOS)  > 0) ? 1 : 0;
            sprintf(information," TOS = 0X%04hX NZP = %d%d%d",TOS,N,Z,P);
            strcat(traceLine,information);
            break;

      // 0X73    SETNZPF             OpCode              Set NZP in FLAGS based on sign of TOS (float)
         case 0X73: 
            {
               float FTOS;
               char base10TOS[80+1];

               strcat(traceLine,"SETNZPF  ");
               ReadWORDFromMainMemory(SP+2,&TOS);
               ConvertHalfFloatToFloat(TOS,&FTOS);
               N = (FTOS  < 0.0) ? 1 : 0;
               Z = (FTOS == 0.0) ? 1 : 0;
               P = (FTOS  > 0.0) ? 1 : 0;
               ConvertHalfFloatToBase10(TOS,base10TOS);
               sprintf(information," TOS = %s NZP = %d%d%d",base10TOS,N,Z,P);
               strcat(traceLine,information);
            }
            break;

      // 0X74    SETT                OpCode              Set T in FLAGS based on true/false value of TOS (boolean)
         case 0X74: 
            strcat(traceLine,"SETT     ");
            ReadWORDFromMainMemory(SP+2,&TOS); 
            if ( TOS == 0XFFFFu )
               T = 1;
            else
               T = 0;
            sprintf(information," T = %d",T);
            strcat(traceLine,information);
            break;

      // 0X80    JMP     A16         OpCode:O16          PC <- O16U
         case 0X80: 
            strcat(traceLine,"JMP      ");
            ReadWORDFromMainMemory(PC,&O16);
            sprintf(information," 0X%04hX",O16);
            strcat(traceLine,information);
            PC = O16;
            break;

      // 0X81    JMPL    A16         OpCode:O16          if (      L ) PC <- O16U
         case 0X81: 
            strcat(traceLine,"JMPL     ");
            ReadWORDFromMainMemory(PC,&O16); PC += 2;
            sprintf(information," 0X%04hX LEG = %d%d%d",O16,L,E,G);
            strcat(traceLine,information);
            if ( L == 1 )
               PC = O16;
            break;

      // 0X82    JMPE    A16         OpCode:O16          if (      E ) PC <- O16U
         case 0X82: 
            strcat(traceLine,"JMPE     ");
            ReadWORDFromMainMemory(PC,&O16); PC += 2;
            sprintf(information," 0X%04hX LEG = %d%d%d",O16,L,E,G);
            strcat(traceLine,information);
            if ( E == 1 )
               PC = O16;
            break;

      // 0X83    JMPG    A16         OpCode:O16          if (      G ) PC <- O16U
         case 0X83: 
            strcat(traceLine,"JMPG     ");
            ReadWORDFromMainMemory(PC,&O16); PC += 2;
            sprintf(information," 0X%04hX LEG = %d%d%d",O16,L,E,G);
            strcat(traceLine,information);
            if ( G == 1 )
               PC = O16;
            break;

      // 0X84    JMPLE   A16         OpCode:O16          if ( L or E ) PC <- O16U
         case 0X84: 
            strcat(traceLine,"JMPLE    ");
            ReadWORDFromMainMemory(PC,&O16); PC += 2;
            sprintf(information," 0X%04hX LEG = %d%d%d",O16,L,E,G);
            strcat(traceLine,information);
            if ( (L == 1) || (E == 1) )
               PC = O16;
            break;

      // 0X85    JMPNE   A16         OpCode:O16          if ( L or G ) PC <- O16U (JMPLG)
         case 0X85: 
            strcat(traceLine,"JMPNE    ");
            ReadWORDFromMainMemory(PC,&O16); PC += 2;
            sprintf(information," 0X%04hX LEG = %d%d%d",O16,L,E,G);
            strcat(traceLine,information);
            if ( !(E == 1) )
               PC = O16;
            break;

      // 0X86    JMPGE   A16         OpCode:O16          if ( G or E ) PC <- O16U
         case 0X86: 
            strcat(traceLine,"JMPGE    ");
            ReadWORDFromMainMemory(PC,&O16); PC += 2;
            sprintf(information," 0X%04hX LEG = %d%d%d",O16,L,E,G);
            strcat(traceLine,information);
            if ( (G == 1) || (E == 1) )
               PC = O16;
            break;

      // 0X87    JMPN    A16         OpCode:O16          if (      N ) PC <- O16U
         case 0X87: 
            strcat(traceLine,"JMPN     ");
            ReadWORDFromMainMemory(PC,&O16); PC += 2;
            sprintf(information," 0X%04hX NZP = %d%d%d",O16,N,Z,P);
            strcat(traceLine,information);
            if ( N == 1 )
               PC = O16;
            break;

      // 0X88    JMPNN   A16         OpCode:O16          if (  not N ) PC <- O16U
         case 0X88: 
            strcat(traceLine,"JMPNN    ");
            ReadWORDFromMainMemory(PC,&O16); PC += 2;
            sprintf(information," 0X%04hX NZP = %d%d%d",O16,N,Z,P);
            strcat(traceLine,information);
            if ( !(N == 1) )
               PC = O16;
            break;

      // 0X89    JMPZ    A16         OpCode:O16          if (      Z ) PC <- O16U
         case 0X89: 
            strcat(traceLine,"JMPZ     ");
            ReadWORDFromMainMemory(PC,&O16); PC += 2;
            sprintf(information," 0X%04hX NZP = %d%d%d",O16,N,Z,P);
            strcat(traceLine,information);
            if ( Z == 1 )
               PC = O16;
            break;

      // 0X8A    JMPNZ   A16         OpCode:O16          if (  not Z ) PC <- O16U
         case 0X8A: 
            strcat(traceLine,"JMPNZ    ");
            ReadWORDFromMainMemory(PC,&O16); PC += 2;
            sprintf(information," 0X%04hX NZP = %d%d%d",O16,N,Z,P);
            strcat(traceLine,information);
            if ( !(Z == 1) )
               PC = O16;
            break;

      // 0X8B    JMPP    A16         OpCode:O16          if (      P ) PC <- O16U
         case 0X8B: 
            strcat(traceLine,"JMPP     ");
            ReadWORDFromMainMemory(PC,&O16); PC += 2;
            sprintf(information," 0X%04hX NZP = %d%d%d",O16,N,Z,P);
            strcat(traceLine,information);
            if ( P == 1 )
               PC = O16;
            break;

      // 0X8C    JMPNP   A16         OpCode:O16          if (  not P ) PC <- O16U
         case 0X8C: 
            strcat(traceLine,"JMPNP    ");
            ReadWORDFromMainMemory(PC,&O16); PC += 2;
            sprintf(information," 0X%04hX NZP = %d%d%d",O16,N,Z,P);
            strcat(traceLine,information);
            if ( !(P == 1) )
               PC = O16;
            break;

      // 0X8D    JMPT    A16         OpCode:O16          if (      T ) PC <- O16U
         case 0X8D: 
            strcat(traceLine,"JMPT     ");
            ReadWORDFromMainMemory(PC,&O16); PC += 2;
            sprintf(information," 0X%04hX T = %d",O16,T);
            strcat(traceLine,information);
            if ( T == 1 )
               PC = O16;
            break;

      // 0X8E    JMPNT   A16         OpCode:O16          if (  not T ) PC <- O16U (JMPF)
         case 0X8E: 
            strcat(traceLine,"JMPNT    ");
            ReadWORDFromMainMemory(PC,&O16); PC += 2;
            sprintf(information," 0X%04hX T = %d",O16,T);
            strcat(traceLine,information);
            if ( !(T == 1) )
               PC = O16;
            break;

      // 0XA0    CALL    A16         OpCode:O16          Push PC; PC <- O16U
         case 0XA0: 
            strcat(traceLine,"CALL     ");
            ReadWORDFromMainMemory(PC,&O16); PC += 2;
            WriteWORDToMainMemory(SP,PC); SP -= 2;
            sprintf(information," 0X%04hX return to 0X%04hX",O16,PC);
            strcat(traceLine,information);
            PC = O16;
            break;

      // 0XA1    RETURN              OpCode              Pop PC
         case 0XA1: 
            strcat(traceLine,"RETURN   ");
            ReadWORDFromMainMemory(SP+2,&PC); SP += 2;
            sprintf(information," to 0X%04hX",PC);
            strcat(traceLine,information);
            break;
/*
=====================================================================
STMOS Service Requests
=====================================================================
#	  Description	                 Parameters
---  ----------------------------- ---------------------------------------------
  0  Force context switch          (none) Do nothing
  1  Terminate process	           Pop termination status 
 10  Read integer	                 Input W16 as integer; push W16
 11  Write integer	              Pop W16; output W16 as integer
 20  Read float	                 Input W16 as float; push W16
 21  Write float	                 Pop W16; output W16 as float
 30  Read boolean	                 Input W16 as boolean; push W16 { 't','T','f','F' }
 31  Write boolean	              Pop W16; output W16 as boolean { 'T','F' }
 40  Read character	              Input W16 as character; push W16
 41  Write character	              Pop W16; output W16 as character
 42  Write ENDL character          (none) Output ENDL (end-of-line) character
 50  Read string                   Pop A16 as address of string; input string into A16
 51  Write string	                 Pop A16 as address of string; output string from A16
 90  Initialize heap               Pop heapSize and heapBase; initialize heap
 91  Allocate heap block           Pop blockSize; allocate block; push blockAddress
 92  Deallocate heap block         Pop blockAddress; deallocate block
*/
      // 0XFF    SVC #W16            OpCode:O16          Execute service request O16U (parameters are passed on run-time stack)
         case 0XFF: 
            strcat(traceLine,"SVC       ");
            ReadWORDFromMainMemory(PC,&O16); PC += 2;
            sprintf(information,"#%hd",O16);
            strcat(traceLine,information);
            switch ( O16 )
            {
               case  0: //  0	Force context switch                (none) Do nothing
                  strcat(traceLine," force context switch");
                  break;
               case  1: //  1	Terminate process	                  Pop termination status
                  if ( strlen(OUT) > 0 )
                  {
                     fprintf(LOG,"%s\n",OUT);
                     printf("%s\n",OUT);
                  }
                  ReadWORDFromMainMemory(SP+2,&W16); SP += 2;
                  sprintf(information," terminate program with status %hd, SP = 0X%04hX\n",W16,SP);
                  strcat(traceLine,information);
                  running = false;
                  break;
               case 10: // 10	Read integer	                     Input W16 as integer; push W16
                  if ( strlen(OUT) > 0 )
                  {
                     printf("%s",OUT);
                     scanf("%hu",&W16);
                     fprintf(LOG,"%s%hd\n",OUT,W16);
                     OUT[0] = '\0';
                  }
                  else
                  {
                     printf("? ");
                     scanf("%hu",&W16);
                     fprintf(LOG,"? %hd\n",W16);
                  }
                  WriteWORDToMainMemory(SP,W16); SP -= 2;
                  sprintf(information," read integer 0X%04hX",W16);
                  strcat(traceLine,information);
                  break;
               case 11: // 11	Write integer	                     Pop W16; output W16 as integer
                  {
                     char datum[SOURCELINELENGTH+1];

                     ReadWORDFromMainMemory(SP+2,&W16); SP += 2;
                     sprintf(datum,"%hd",W16);
                     strcat(OUT,datum);
                  }
                  strcat(traceLine," write integer");
                  break;
               case 20: // 20	Read float	                        Input W16 as float; push W16
                  {
                     float item;
                     char base10W16[80+1];
                     
                     if ( strlen(OUT) > 0 )
                     {
                        printf("%s",OUT);
                        scanf("%f",&item);
                        ConvertFloatToHalfFloat(item,&W16);
                        ConvertHalfFloatToBase10(W16,base10W16);
                        fprintf(LOG,"%s%s (approximation)\n",OUT,base10W16);
                        OUT[0] = '\0';
                     }
                     else
                     {
                        printf("? ");
                        scanf("%f",&item);
                        ConvertFloatToHalfFloat(item,&W16);
                        ConvertHalfFloatToBase10(W16,base10W16);
                        fprintf(LOG,"? %s (approximation)\n",base10W16);
                     }
                     WriteWORDToMainMemory(SP,W16); SP -= 2;
                     sprintf(information," read float %s",base10W16);
                     strcat(traceLine,information);
                  }
                  break;
               case 21: // 21	Write float	                        Pop W16; output W16 as float
                  {
                     char datum[SOURCELINELENGTH+1];

                     ReadWORDFromMainMemory(SP+2,&W16); SP += 2;
                     ConvertHalfFloatToBase10(W16,datum);
                     strcat(OUT,datum);
                  }
                  strcat(traceLine," write float");
                  break;
               case 30: // 30	Read boolean	                     Input W16 as boolean; push W16 { 't','T','f','F' }
                  if ( strlen(OUT) > 0 )
                  {
                     printf("%s",OUT);
                     scanf(" %c",&IN[0]);
                     fprintf(LOG,"%s%c\n",OUT,LOBYTE(IN[0]));
                     OUT[0] = '\0';
                  }
                  else
                  {
                     printf("? ");
                     scanf(" %c",&IN[0]);
                     fprintf(LOG,"? %c\n",LOBYTE(IN[0]));
                  }
                  if      ( toupper(IN[0]) == 'T' )
                     W16 = 0XFFFFu;
                  else if ( toupper(IN[0]) == 'F' )
                     W16 = 0X0000u;
                  else
                  {
                     ProcessRunTimeError("Boolean must be in { t,T,f,F }",false);
                     W16 = 0X0000u;
                  }
                  WriteWORDToMainMemory(SP,W16); SP -= 2;
                  sprintf(information," read boolean 0X%04hX",W16);
                  strcat(traceLine,information);
                  break;
               case 31: // 31	Write boolean	                     Pop W16; output W16 as boolean { 'T','F' }
                  {
                     char datum[SOURCELINELENGTH+1];

                     ReadWORDFromMainMemory(SP+2,&W16); SP += 2;
                     sprintf(datum,"%c",((W16 == 0XFFFFu) ? 'T' : 'F'));
                     strcat(OUT,datum);
                  }
                  sprintf(information," write boolean 0X%04hX",W16);
                  strcat(traceLine,information);
                  break;
               case 40: // 40	Read character	                     Input W16 as character; push W16
               // "flush" stdin
                  while ( getc(stdin) != '\n' );
                  if ( strlen(OUT) > 0 )
                  {
                     printf("%s",OUT);
                     scanf("%c",&IN[0]);
                     fprintf(LOG,"%s%c\n",OUT,LOBYTE(IN[0]));
                     OUT[0] = '\0';
                  }
                  else
                  {
                     printf("? ");
                     scanf("%c",&IN[0]);
                     fprintf(LOG,"? %c\n",LOBYTE(IN[0]));
                  }
                  W16 = LOBYTE(IN[0]);
                  WriteWORDToMainMemory(SP,W16); SP -= 2;
                  sprintf(information," read character 0X%04hX = '%c'",W16,LOBYTE(W16));
                  strcat(traceLine,information);
                  break;
               case 41: // 41	Write character	                  Pop W16, output W16 as character
                  {
                     const BYTE  LF = 0X0Au;
                     const BYTE  CR = 0X0Du;

                     char datum[SOURCELINELENGTH+1];

                     ReadWORDFromMainMemory(SP+2,&W16); SP += 2;
                     if ( (LOBYTE(W16) == LF) || (LOBYTE(W16) == CR) )
                     {
                        fprintf(LOG,"%s\n",OUT);
                        printf("%s\n",OUT);
                        OUT[0] = '\0';
                        sprintf(information," write character 0X%04hX",W16);
                        strcat(traceLine,information);
                     }
                     else
                     {
                        sprintf(datum,"%c",LOBYTE(W16));
                        strcat(OUT,datum);
                        sprintf(information," write character 0X%04hX = '%c'",W16,LOBYTE(W16));
                        strcat(traceLine,information);
                     }
                  }
                  break;
               case 42: // 42	Write ENDL character                (none) Output ENDL (end-of-line) character
                  {
                     char datum[SOURCELINELENGTH+1];

                     fprintf(LOG,"%s\n",OUT);
                     printf("%s\n",OUT);
                     OUT[0] = '\0';
                  }
                  strcat(traceLine," write ENDL");
                  break;
               case 50: // 50	Read string	                        Pop A16 as address of string; input string into A16
                  {
                     int i;
                     WORD A16,length,capacity;

                  // "flush" stdin
                     while ( getc(stdin) != '\n' );
                     ReadWORDFromMainMemory(SP+2,&A16); SP += 2;
                     if ( strlen(OUT) > 0 )
                     {
                        printf("%s",OUT);
                        gets(IN);
                        fprintf(LOG,"%s%s",OUT,IN);
                        OUT[0] = '\0';
                     }
                     else
                     {
                        printf("? ");
                        gets(IN);
                        fprintf(LOG,"? %s",IN);
                     }
                  // Ensure there *IS* something to "flush" when 2 or more string inputs occur in a row ***KLUDGE***
                     ungetc('\n',stdin);
                     ReadWORDFromMainMemory(A16+2,&capacity);
                     if (strlen(IN) <= capacity)
                     {
                        length = strlen(IN);
                        fprintf(LOG,"\n");
                     }
                     else
                     {
                        length = capacity;
                        fprintf(LOG," (truncated) \n");
                     }
                     WriteWORDToMainMemory(A16,length);
                     for (i = 1; i <= length; i++)
                        WriteWORDToMainMemory(A16+4+(i-1)*2,(WORD) IN[i-1]);
                  }
                  break;
               case 51: // Write string                           Pop A16 as address of string; output string from A16
                  {
                     const BYTE NUL = 0X00u;
                     const BYTE  LF = 0X0Au;
                     const BYTE  CR = 0X0Du;

                     int i;
                     char datum[SOURCELINELENGTH+1];
                     WORD A16,length,capacity;

                     ReadWORDFromMainMemory(SP+2,&A16); SP += 2;
                     ReadWORDFromMainMemory(A16,&length);
                     ReadWORDFromMainMemory(A16+2,&capacity);
                     i = 1; 
                     while ( i <= length )
                     {
                        ReadWORDFromMainMemory(A16+4+(i-1)*2,&W16);
                        i += 1;
                     // substitute for legal escape sequences
                        if ( LOBYTE(W16) == '\\' )
                        {
                           ReadWORDFromMainMemory(A16+4+(i-1)*2,&W16);
                           i += 1;
                           switch ( LOBYTE(W16) )
                           {
                              case 'n': // LF (newline)
                                 W16 = LF;
                                 break;
                              case 't': // HT (horizontal tab)
                                 W16 = 0X09u;
                                 break;
                              case 'b': // BS (backspace)
                                 W16 = 0X08u;
                                 break;
                              case 'r': // CR (carriage return)
                                 W16 = CR;
                                 break;
                              case '\\': // \ (backslash)
                                 W16 = '\\';
                                 break;
                              case '"': // " (double quote)
                                 W16 = '"';
                                 break;
                              default:
                                 W16 = '?';
                                 RecordSyntaxError("Illegal escape character sequence\n");
                           }
                        }
                        if ( (LOBYTE(W16) == LF) || (LOBYTE(W16) == CR) )
                        {
                           fprintf(LOG,"%s\n",OUT);
                           printf("%s\n",OUT);
                           OUT[0] = '\0';
                        }
                        else
                        {
                           sprintf(datum,"%c",LOBYTE(W16));
                           strcat(OUT,datum);
                        }
                     }
                  }
                  strcat(traceLine," write string");
                  break;
/*
   FREEnodes is a pointer to a singly-linked list of nodes representing contiguous blocks of 
      heap space that are "free." The FREEnodes list is initialized with SVC #90 to contain
      a single, large node which represents a block consisting of *ALL* allocateable heap space. 
      After a series of SVC #91 (allocate block) and SVC #92 (deallocate block) service 
      requests, the FREEnodes list will consist of (1) the remnants of the original, large
      "free" node; and (2) nodes representing deallocated blocks which cannot be "absorbed by"
      (are not on the "edge" of) the free space of the other free nodes.
   The strategy "find-first-FREE-node-that-fits-the-requested-blockSize" is used to satisfy SVC #91 requests.
   The FREEnodes list is maintained in order of increasing address of the node.

   structure FREENODE
   // metadata (offset from beginning of node measured in bytes)
     (0) WORD FLink              address of logically-next node in list
     (2) WORD blockSize          measured in bytes
   // block of allocated heap space
     (4) BYTE block[1:blockSize] *NOTE* blockAddress is address-of block[1]
   endstructure
*/
               case 90: // 90  Initialize heap               Pop heapSize and heapBase; initialize heap
                  {
                  // Pop headSize and heapBase
                     ReadWORDFromMainMemory(SP+2,&heapSize); SP += 2;
                     ReadWORDFromMainMemory(SP+2,&heapBase); SP += 2;
                  // Create one large node FLink = 0X0000 and (blockSize = heapSize-4) and initialize the FREE node list
                     FREEnodes  = heapBase;
                     WriteWORDToMainMemory(FREEnodes+0,0X0000u);
                     WriteWORDToMainMemory(FREEnodes+2,heapSize-4);
                     sprintf(information," initialize heap, heapBase = 0X%04hX, heapSize = 0X%04hX words",heapBase,heapSize);
                     strcat(traceLine,information);
                     TraceFREEnodes(FREEnodes);
                  }
                  break;
               case 91: // 91  Allocate heap block           Pop blockSize; allocate block; push blockAddress
                  {
                     WORD allocateNodeAddress,blockSize,blockAddress;
                     WORD nodeAddress,nodeFLink,nodeBlockSize;
                     bool nodeFound;

                  // Pop blockSize
                     ReadWORDFromMainMemory(SP+2,&blockSize); SP += 2;
                  // Use first-fit algorithm to find a nodeBlockSize >= (blockSize+4)
                     nodeAddress = FREEnodes;
                     nodeFound = false;
                     do
                     {
                        ReadWORDFromMainMemory(nodeAddress+0,&nodeFLink);
                        ReadWORDFromMainMemory(nodeAddress+2,&nodeBlockSize);
                        if ( nodeBlockSize >= (blockSize+4) )
                           nodeFound = true;
                        else
                           nodeAddress = nodeFLink;
                     } while ( !nodeFound && (nodeFLink != 0X0000u) );
                     if ( !nodeFound )
                     {
                        ProcessRunTimeError("Heap space exhausted",false);
                     // Push 0X0000 to indicate allocation request failed
                        WriteWORDToMainMemory(SP,0X0000u); SP -= 2;
                        sprintf(information," allocate heap block failed\n");
                     }
                     else
                     {
                     // "Break-off" (blockSize+4) bytes at rear of "fitting" node's block
                        allocateNodeAddress = (nodeAddress+4+nodeBlockSize) - (blockSize+4);
                        WriteWORDToMainMemory(nodeAddress+2,nodeBlockSize-(blockSize+4));
                     // Set allocate node FLink = 0X0000 and blockSize
                        WriteWORDToMainMemory(allocateNodeAddress+0,0X0000u);
                        WriteWORDToMainMemory(allocateNodeAddress+2,blockSize);
                     // Push blockAddress
                        blockAddress = allocateNodeAddress+4;
                        WriteWORDToMainMemory(SP,blockAddress); SP -= 2;
                        sprintf(information," allocate heap block, block address = 0X%04hX, block size= 0X%04hX words",blockAddress,blockSize);
                     }
                     strcat(traceLine,information);
                     TraceFREEnodes(FREEnodes);
                  }
                  break;
               case 92: // 92  Deallocate heap block         Pop blockAddress; deallocate block
                  {
                     WORD nodeAddress,nodeFLink,nodeBlockSize;
                     WORD deallocateNodeAddress,blockSize,blockAddress;
                     bool nodeFound;

                  // Pop blockAddress
                     ReadWORDFromMainMemory(SP+2,&blockAddress); SP += 2;
                     deallocateNodeAddress = blockAddress-4;
                     ReadWORDFromMainMemory(deallocateNodeAddress+2,&blockSize);
                  // Find node which should precede the deallocate node
                     nodeAddress = FREEnodes;
                     nodeFound = false;
                     do
                     {
                        ReadWORDFromMainMemory(nodeAddress+0,&nodeFLink);
                        ReadWORDFromMainMemory(nodeAddress+2,&nodeBlockSize);
                        if ( (nodeFLink == 0X0000u) || (nodeFLink > deallocateNodeAddress) )
                           nodeFound = true;
                        else
                           nodeAddress = nodeFLink;
                     } while ( !nodeFound );
                  // If possible, "absorb" deallocate block into one or both of its surrounding FREE list nodes
                     if ( nodeAddress+4+nodeBlockSize == deallocateNodeAddress )
                     {
                        nodeBlockSize += (blockSize+4);
                        WriteWORDToMainMemory(nodeAddress+2,nodeBlockSize);
                        if ( (nodeFLink != 0X0000u) && (nodeAddress+4+nodeBlockSize == nodeFLink) )
                        {
                           WORD nextNodeFLink,nextNodeBlockSize;
   
                           ReadWORDFromMainMemory(nodeFLink+0,&nextNodeFLink);
                           ReadWORDFromMainMemory(nodeFLink+2,&nextNodeBlockSize);
                           nodeBlockSize += (nextNodeBlockSize+4);
                           WriteWORDToMainMemory(nodeAddress+0,nextNodeFLink);
                           WriteWORDToMainMemory(nodeAddress+2,nodeBlockSize);
                        }
                     }
                  // otherwise, insert deallocate block into FREE list after nodeAddress node
                     else
                     {
                        WriteWORDToMainMemory(deallocateNodeAddress+0,nodeFLink);
                        WriteWORDToMainMemory(nodeAddress+0,deallocateNodeAddress);
                     }
                     sprintf(information," deallocate heap block, block address = 0X%04hX, block size= 0X%04hX words",blockAddress,blockSize);
                     strcat(traceLine,information);
                     TraceFREEnodes(FREEnodes);
                  }
                  break;
               default:
                  strcat(traceLine," Invalid SVC #");
                  ProcessRunTimeError("Invalid SVC #",false);
                  break;
            }
            break;
      // *UNKNOWN* opCode
         default: 
            strcat(traceLine,"???????  ");
            ProcessRunTimeError("Invalid opcode",false);
            break;
      }
      fprintf(LOG,"%s\n",traceLine);
   } while ( running );
}

//-----------------------------------------------------------
void TraceFREEnodes(WORD FREEnodes)
//-----------------------------------------------------------
{
/*
-------------------------------------------------------------
FREE nodes list
   0X????:0X????(0X????) 
   0X????:0X????(0X????)
   ...
   0X????:0X????(0X????)
*/
   void ReadWORDFromMainMemory(int address,WORD *word);

   WORD nodeAddress,nodeFLink,nodeBlockSize;

   fprintf(LOG,"-------------------------------------------------------------\n");
   fprintf(LOG,"FREE nodes list\n");
   nodeAddress = FREEnodes;
   while ( nodeAddress != 0X0000u )
   {
      ReadWORDFromMainMemory(nodeAddress+0,&nodeFLink);
      ReadWORDFromMainMemory(nodeAddress+2,&nodeBlockSize);
      fprintf(LOG,"   0X%04hX:0X%04hX(0X%04hX)\n",nodeAddress,(nodeAddress+4+nodeBlockSize-1),nodeBlockSize);
      nodeAddress = nodeFLink;
   }
   fprintf(LOG,"-------------------------------------------------------------\n");
   fflush(LOG);
}

//-----------------------------------------------------------
WORD MemoryOperandEA(BYTE mode,WORD O16,WORD PC,WORD *SP,WORD FB,WORD SB,char information[])
//-----------------------------------------------------------
{
   void ReadWORDFromMainMemory(int address,WORD *word);

/*
   The <memory> syntax (shown below) allows the programmer to specify a 16-bit STM
      word value in 1 of 3 diffent ways
      
      (1) a <A16> (which must be an <identifier>);
      (2) as a <W16> (which can be <I16> (signed integer), <F16> (signed floating point),
          true or false (boolean literals), <character> or an <identifier>;
      (3) a <I16> (which is *always* interpreted as an unsigned integer)
      
      this 16-bit value--regardless of the way it is written in assembly language--is 
      referred to in the parameter list as O16.
      
<memory>          ::= #<W16>                    || mode = 0X00, immediate
                    | <A16>                     || mode = 0X01, memory direct
                    | @<A16>                    || mode = 0X02, memory indirect
                    | $<A16>                    || mode = 0X03, memory indexed
                    | SP:<I16>                  || mode = 0X04, SP-relative direct
                    | @SP:<I16>                 || mode = 0X05, SP-relative indirect
                    | $SP:<I16>                 || mode = 0X06, SP-relative indexed
                    | FB:<I16>                  || mode = 0X07, FB-relative direct
                    | @FB:<I16>                 || mode = 0X08, FB-relative indirect
                    | $FB:<I16>                 || mode = 0X09, FB-relative indexed
                    | SB:<I16>                  || mode = 0X0A, SB-relative direct
                    | @SB:<I16>                 || mode = 0X0B, SB-relative indirect
                    | $SB:<I16>                 || mode = 0X0C, SB-relative indexed
*/
   WORD EA,IEA,TOS;

   switch ( mode )
   {
      case 0X00: // EA = PC-2 ***ASSUMES PC IS ADDRESS OF NEXT OPCODE***
         EA = PC-2;
         sprintf(information," #memory[EA = 0X%04hX]",EA);
         break;
      case 0X01: // EA = A16
         EA = O16;
         sprintf(information," memory[EA = 0X%04hX]",EA);
         break;
      case 0X02: // EA = mainMemory[ A16 ]
         IEA = O16;
         ReadWORDFromMainMemory(IEA,&EA);
         sprintf(information," @memory[EA = 0X%04hX = memory[0X%04hX]]",EA,IEA);
      case 0X03: // Pop TOS; EA = A16 + TOS
         ReadWORDFromMainMemory(*SP+2,&TOS); *SP += 2;
         EA = O16 + TOS;
         sprintf(information," $0X%04hX+(%5hd) memory[EA = 0X%04hX]",O16,TOS,EA);
         break;
      case 0X04: // EA = (SP+2)+2*I16
         EA = (*SP+2) + 2*O16;
         sprintf(information," SP(%3hd) memory[EA = 0X%04hX]",O16,EA);
         break;
      case 0X05: // EA = mainMemory[ (SP+2)+2*I16 ]
         IEA = (*SP+2) + 2*O16;
         ReadWORDFromMainMemory(IEA,&EA);
         sprintf(information," @SP(%3hd) memory[EA = 0X%04hX = memory[0X%04hX]]",O16,EA,IEA);
         break;
      case 0X06: // Pop TOS; EA = (SP+2)+2*I16 + TOS
         ReadWORDFromMainMemory(*SP+2,&TOS); *SP += 2;
         EA = (*SP+2) + 2*O16 + TOS;
         sprintf(information," SP(%3hd)+(%5hd) memory[EA = 0X%04hX]",O16,TOS,EA);
         break;
      case 0X07: // EA = FB-2*I16 ***NOTICE SUBTRACTION***
         EA = FB - 2*O16;
         sprintf(information," FB(%3hd) memory[EA = 0X%04hX]",O16,EA);
         break;
      case 0X08: // EA = mainMemory[ FB-2*I16 ] ***NOTICE SUBTRACTION***
         IEA = FB - 2*O16;
         ReadWORDFromMainMemory(IEA,&EA);
         sprintf(information," @FB(%3hd) memory[EA = 0X%04hX = memory[0X%04hX]]",O16,EA,IEA);
         break;
      case 0X09: // Pop TOS; EA = FB-2*I16 + TOS ***NOTICE SUBTRACTION***
         ReadWORDFromMainMemory(*SP+2,&TOS); *SP += 2;
         EA = FB - 2*O16 + TOS;
         sprintf(information," FB(%3hd)+(%5hd) memory[EA = 0X%04hX]",O16,TOS,EA);
         break;
      case 0X0A: // EA = SB+2*I16
         EA = SB + 2*O16;
         sprintf(information," SB(%3hd) memory[EA = 0X%04hX]",O16,EA);
         break;
      case 0X0B: // EA = mainMemory[ SB+2*I16 ]
         IEA = SB + 2*O16;
         ReadWORDFromMainMemory(IEA,&EA);
         sprintf(information," @SB(%3hd) memory[EA = 0X%04hX = memory[0X%04hX]]",O16,EA,IEA);
         break;
      case 0X0C: // Pop TOS; EA = SB+2*I16 + TOS
         ReadWORDFromMainMemory(*SP+2,&TOS); *SP += 2;
         EA = SB + 2*O16 + TOS;
         sprintf(information," SB(%3hd)+(%5hd) memory[EA = 0X%04hX]",O16,TOS,EA);
         break;
      default:
         ProcessRunTimeError("Invalid addressing mode (not in [ 0X00,0X0C ])",true);
         break;
   }
   return( EA );
}

/*
   IEEE-754 16-bit half-precision float format
   SEEE EEMM MMMM MMMM
   1000 0000 0000 0000 = 0X8000 mask for sign 
   0111 1100 0000 0000 = 0X7C00          exponent
   0000 0011 1111 1111 = 0X03FF          mantissa

   IEEE-754 32-bit single-precision float format
   SEEE EEEE EMMM MMMM MMMM MMMM MMMM MMMM
   1000 0000 0000 0000 0000 0000 0000 0000 = 0X80000000 mask for sign
   0111 1111 1000 0000 0000 0000 0000 0000 = 0X7F800000          exponent
   0000 0000 0111 1111 1111 1111 1111 1111 = 0X007FFFFF          mantissa
*/

//-----------------------------------------------------------
void ConvertFloatToHalfFloat(float F,WORD *HF)
//-----------------------------------------------------------
{
   union
   {
      float F;
      unsigned int UI;
   } X;

   int Fs,Fe,Fm;
   int HFs,HFe,HFm;

   X.F = F;
   Fs = (X.UI & 0X80000000) >> 31;
   Fe = (X.UI & 0X7F800000) >> 23;
   Fm = (X.UI & 0X007FFFFF);

// +-float (denormalized) --> +-0 half-float 
   if      ( (Fe == 0) && (Fm != 0) )
   {
      HFs = Fs;
      HFe = 0;
      HFm = 0;
   }
// +-0 float --> +-0 half-float
   else if ( (Fe == 0) && (Fm == 0) )
   {
      HFs = Fs;
      HFe = 0;
      HFm = 0;
   }
// +-Inf float --> +-Inf half-float
   else if ( (Fe == 255) && (Fm == 0) )
   {
      HFs = Fs;
      HFe = 31;
      HFm = 0;
   }
// +-NaN float --> +-NaN half-float
   else if ( (Fe == 255) && (Fm != 0) )
   {
      HFs = Fs;
      HFe = 31;
      HFm = Fm >> 13;
   }
// | float | > 2^15 --> +-Inf half-float
   else if ( Fe-127 > 15 )
   {
      HFs = Fs;
      HFe = 31;
      HFm = 0;
   }
// | float | < 2^-24 --> +-0 half-float
   else if ( Fe-127 < -24 )
   {
      HFs = Fs;
      HFe = 0;
      HFm = 0;
   }
// 2^-24 <= | +-float (normalized) | < 2^-14 --> +-half-float (denormalized)
   else if ( (-24 <= Fe-127) && (Fe-127 < -14) )
   {
      HFs = Fs;
      HFe = 0;
      HFm = (Fm | 0X00800000) >> (13 + (-14-(Fe-127)));
   }
// +-float (normalized) --> +-half-float (normalized)
   else
   {
      HFs = Fs;
      HFe = Fe-127+15;
      HFm = Fm >> 13;
   }

   *HF = (WORD) ((HFs << 15) | (HFe << 10) | HFm);
}

//-----------------------------------------------------------
void ConvertHalfFloatToFloat(WORD HF,float *F)
//-----------------------------------------------------------
{
   union
   {
      float F;
      unsigned int UI;
   } X;

   int Fs,Fe,Fm;
   int HFs,HFe,HFm;

   HFs = (HF & 0X8000) >> 15;
   HFe = (HF & 0X7C00) >> 10;
   HFm = HF & 0X03FF;

// +-0 half-float --> +-0 float
   if      ( (HFe == 0) && (HFm == 0) )
   {
      Fs = HFs;
      Fe = 0;
      Fm = 0;
   }
// +-Inf half-float --> +-Inf float
   else if ( (HFe == 31) && (HFm == 0) )
   {
      Fs = HFs;
      Fe = 255;
      Fm = 0;
   }
// +-NaN half-float --> +-NaN float
   else if ( (HFe == 31) && (HFm != 0) )
   {
      Fs = HFs;
      Fe = 255;
      Fm = HFm << 13;
   }
// +-half-float(denormalized) --> +-float (normalized)
   else if ( (HFe == 0) && (HFm != 0) )
   {
      Fs = HFs;
      Fe = -14+127;
      Fm = HFm << 13;
      do
      {
         Fm <<= 1;
         Fe -= 1;
      } while ( (Fm & 0X00800000) == 0 );
      Fm = Fm & 0X007FFFFF;
   }
// +-half-float (normalized) --> +-float (normalized)
   else
   {
      Fs = HFs;
      Fe = HFe-15+127;
      Fm = HFm << 13;
   }

   X.UI = (Fs << 31) | (Fe << 23) | Fm;
   *F = X.F;
}

//-----------------------------------------------------------
void ConvertHalfFloatToBase10(WORD HF,char base10[])
//-----------------------------------------------------------
{
   int HFs = (HF & 0X8000) >> 15;
   int HFe = (HF & 0X7C00) >> 10;
   int HFm = HF & 0X03FF;

// +-0 half-float
   if      ( (HFe == 0) && (HFm == 0) )
      sprintf(base10,"0.0");
// +-Inf half-float
   else if ( (HFe == 31) && (HFm == 0) )
      sprintf(base10,"%sInf",((HFs == 1) ? "-" : "+"));
// +-NaN half-float
   else if ( (HFe == 31) && (HFm != 0) )
      sprintf(base10,"NaN");
// +-half-float (normalized and denormalized)
   else
   {
      float F;
   
      ConvertHalfFloatToFloat(HF,&F);

   // Display 4 significant digits with no exponent when 0.1000 <= | F | <= 999.9
      if      ( (   0.1000f <= fabs(F)) && (fabs(F) <=    0.9999) )
         sprintf(base10,"%6.4f",F);
      else if ( (   1.000f  <= fabs(F)) && (fabs(F) <=    9.999 ) )
         sprintf(base10,"%5.3f",F);
      else if ( (  10.00f   <= fabs(F)) && (fabs(F) <=   99.99  ) )
         sprintf(base10,"%5.2f",F);
      else if ( ( 100.0f    <= fabs(F)) && (fabs(F) <=  999.9   ) )
         sprintf(base10,"%5.1f",F);
   // otherwise display -X.XXXESX or X.XXXESX
      else
      {
         int Ei;

      // Formats as -X.XXXESXXX or X.XXXESXXX with no format control of exponent!
         sprintf(base10,"%10.3E",F);
      // Shorten format to -X.XXXESX or X.XXXESX by erasing 00 in exponent
         Ei = (int) strlen(base10)-5;
         base10[Ei+2] = base10[Ei+4];
         base10[Ei+3] = '\0'; 
      }
   }
}
