// PotatoReader.cpp by Auroxsus
// French Fry Productions
// Description: POTATO Reader "driver" program
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

#define TRACEREADER

#include "PotatoChip.h"

//-----------------------------------------------------------
int main()
//-----------------------------------------------------------
{
	void Callback1(int sourceLineNumber,const char sourceLine[]);
	void Callback2(int sourceLineNumber,const char sourceLine[]);

	char sourceFileName[80+1];
	NEXTCHARACTER nextCharacter;

	READER<CALLBACKSUSED> reader(SOURCELINELENGTH,LOOKAHEAD);
	LISTER lister(LINESPERPAGE);

	cout << "Source filename? ";
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

void Callback1(int sourceLineNumber,const char sourceLine[])
{
	cout << setw(4) << sourceLineNumber << " ";
}

void Callback2(int sourceLineNumber,const char sourceLine[])
{
	cout << sourceLine << endl;
}
