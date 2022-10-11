//-----------------------------------------------------------
// Auroxsus
// POTATO Reader "driver" program
// POTATOReader.cpp
//-----------------------------------------------------------
#include <iostream>
#include <iomanip>

#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <ctime>	// For time stamp header
#include <vector>

using namespace std;

// comment out the macro's definition when no longer need "clutter" in your .list file
#define TRACEREADER 

#include "Potato.h"

//-----------------------------------------------------------
int main()
//-----------------------------------------------------------
{
   void Callback1(int sourceLineNumber,const char sourceLine[]);
   void Callback2(int sourceLineNumber,const char sourceLine[]);

   char sourceFileName[80+1];
   NEXTCHARACTER nextCharacter;

   READER<CALLBACKSUSED> reader(SOURCELINELENGTH,LOOKAHEAD);
//   READER<CALLBACKSUSED> reader(5,LOOKAHEAD);
   LISTER lister(LINESPERPAGE);
//   READER<CALLBACKSUSED> reader(SOURCELINELENGTH,LOOKAHEAD);

   cout << "POTATO source filename? ";
   cin >> sourceFileName;

   try
   {
      lister.OpenFile(sourceFileName);
      reader.SetLister(&lister);
      reader.AddCallbackFunction(Callback1);
      reader.AddCallbackFunction(Callback2);
      reader.OpenFile(sourceFileName);

      do
      {
         nextCharacter = reader.GetNextCharacter();
      } while ( nextCharacter.character != READER<CALLBACKSUSED>::EOPC );
//      } while ( nextCharacter.character != READER::EOPC );
   }
   catch (POTATOEXCEPTION POTATOException)
   {
      cout << "POTATO exception: " << POTATOException.GetDescription() << endl;
   }
   lister.ListInformationLine("******* POTATO reader ending");
   cout << "POTATO reader ending\n";

   system("PAUSE");
   return( 0 );
}

//-----------------------------------------------------------
void Callback1(int sourceLineNumber,const char sourceLine[])
//-----------------------------------------------------------
{
   cout << setw(4) << sourceLineNumber << " ";
}

//-----------------------------------------------------------
void Callback2(int sourceLineNumber,const char sourceLine[])
//-----------------------------------------------------------
{
   cout << sourceLine << endl;
}
