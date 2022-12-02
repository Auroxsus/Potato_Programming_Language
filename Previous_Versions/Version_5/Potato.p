|| Potato.p by Auroxsus
|| French Fry Productions
|| Description: For loop testing
||-----------------------------------------------------------


VAR INT i.

CON INT initialX = 1.


POTATO
   CON INT initialY = 3.

   VAR INT n, INT x, INT y.

   INPUT "n? " n.

   FRY "FOR-loop (1)",PEEL.
   x = initialX. y = initialY.
   FOR (i = 1. i < n.  i = i+1.)
      FRY i,": ".
      IF   ( x < y ) 
         FRY x," < ",y,PEEL.
      ELIF ( x == y ) 
         FRY x," == ",y,PEEL.
      ELIF ( x > y ) 
         FRY x," > ",y,PEEL.
      COOKEDPOTATO
      x = x+1.
      y = y-1.
   COOKEDPOTATO
	
   FRY "FOR-loop (2)",PEEL.
   x = initialX. y = initialY. i = n.
   FOR ( . i < 1. i = i-1.)
      FRY i,": ".
      IF   ( x < y ) 
         FRY x," < ",y,PEEL.
      ELIF ( x == y ) 
         FRY x," == ",y,PEEL.
      ELIF ( x > y ) 
         FRY x," > ",y,PEEL.
      COOKEDPOTATO
      x = x+3.
      y = y-3.
   COOKEDPOTATO
   
   FRY "FOR-loop (3)",PEEL.
   x = initialX. y = initialY.
   FOR (i = 1. i < n.)
      FRY i,": ".
      IF   ( x < y ) 
         FRY x," < ",y,PEEL.
      ELIF ( x == y ) 
         FRY x," == ",y,PEEL.
      ELIF ( x > y ) 
         FRY x," > ",y,PEEL.
      COOKEDPOTATO
      x = x+2.
      y = y-2.
	  i = i+1.
   COOKEDPOTATO

   FRY "FOR-loop (4)",PEEL.
   x = initialX. y = initialY.
   FOR (i = n. i < 1. i = i-1.)
      FRY i,": ".
      IF   ( x < y ) 
         FRY x," < ",y,PEEL.
      ELIF ( x == y ) 
         FRY x," == ",y,PEEL.
      ELIF ( x > y ) 
         FRY x," > ",y,PEEL.
      COOKEDPOTATO
      x = x+3.
      y = y-3.
   COOKEDPOTATO

   FRY "FOR-loop (5)",PEEL.
   FOR (i = 1. i > n. i = i+1-1.)              || run-time error #2 (e3 = 0)
      FRY "UNREACHABLE",PEEL.
   COOKEDPOTATO
   
COOKEDPOTATO
