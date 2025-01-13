|| Potato3.cpp by Auroxsus
|| French Fry Productions
|| Description: If/elif/else and nested If/else
||-----------------------------------------------------------

POTATO
   VAR x INT, y INT.

   INPUT "x? " x.
   INPUT "y? " y.

   IF ( x < y ) 
      FRy "x  < y",PEEL.
   COOKEDPOTATO

   IF ( x != y ) 
      FRy "x != y",PEEL.
   COOKEDPOTATO

   IF ( x <> y ) 
      FRy "x <> y",PEEL.
   COOKEDPOTATO

   IF ( x < y ) 
      FRy "x  < y",PEEL.
   ELSE
      FRy "x >= y",PEEL.
   COOKEDPOTATO

   IF     ( x < y ) 
      FRy "x  < y",PEEL.
   ELIF ( x = y ) 
      FRy "x  = y",PEEL.
   ELIF ( x > y ) 
      FRy "x  > y",PEEL.
   COOKEDPOTATO

   IF     ( x < y ) 
      FRy "x  < y",PEEL.
   ELIF ( x = y ) 
      FRy "x  = y",PEEL.
   ELIF ( x > y ) 
      FRy "x  > y",PEEL.
   ELSE
      FRy "***ERROR***",PEEL.
   COOKEDPOTATO

   IF     ( x = y ) 
      FRy "x  = y",PEEL.
   ELSE
      IF ( x > y ) 
         FRy "x  > y",PEEL.
      ELSE
         FRy "x  < y",PEEL.
      COOKEDPOTATO
   COOKEDPOTATO
COOKEDPOTATO
