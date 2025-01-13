|| ExtraPotato.p by Auroxsus
|| French Fry Productions
|| Description: test <CHOOSEStatement>
||-------------------------------------------------------------

POTATO

||-------------------------------------------------------------
|| Without ELSE
||-------------------------------------------------------------
   FRY "ONE (default)",PEEL.
   CHOOSE
      WHEN (false): 
      WHEN (true): 
         FRY "2".
         FRY PEEL.
      WHEN (true): FRY "3",PEEL.
   COOKEDPOTATO

   FRY PEEL,"ONE (explicit)",PEEL.
   CHOOSE ONE
      WHEN (true): FRY "1\n".
      WHEN (true): 
         FRY "2".
         FRY PEEL.
      WHEN (true): FRY "3",PEEL.
   COOKEDPOTATO

   FRY PEEL,"ALL",PEEL.
   CHOOSE ALL
      WHEN (true): FRY "1\n". 
      WHEN (false): 
         FRY "2".
         FRY PEEL.
      WHEN (true): FRY "3",PEEL.
   COOKEDPOTATO

||-------------------------------------------------------------
|| With ELSE
||-------------------------------------------------------------
   FRY "ONE (default)",PEEL.
   CHOOSE
      WHEN (false): 
      WHEN (false): 
         FRY "2".
         FRY PEEL.
      WHEN (false): FRY "3",PEEL.
      ELSE
         FRY "ELSE ONE (default)",PEEL.
   COOKEDPOTATO

   FRY "ONE (explicit)",PEEL.
   CHOOSE
      WHEN (false): 
      WHEN (true): 
         FRY "2".
         FRY PEEL.
      WHEN (false): FRY "3",PEEL.
      ELSE
         FRY "ELSE ONE (explicit)",PEEL.
   COOKEDPOTATO

   FRY "ALL",PEEL.
   CHOOSE
      WHEN (false): 
      WHEN (false): 
         FRY "2".
         FRY PEEL.
      WHEN (false): FRY "3",PEEL.
      ELSE
         FRY "ELSE ALL",PEEL.
   COOKEDPOTATO
COOKEDPOTATO
