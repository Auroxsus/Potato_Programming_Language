|| Potato1.p by Auroxsus
|| French Fry Productions
|| Description: testing medium function
||-----------------------------------------------------------


CON INT F0 = 0.
CON INT F1 = 1.

FUNCTION Fibonacci: INT(INT i)
   IF   ( i == 0 ) 
      RETURN( F0 ).
   ELIF ( i == 1 ) 
      RETURN( F1 ).
   ELSE
      RETURN( Fibonacci(i-2)+Fibonacci(i-1) ).
   COOKEDPOTATO
COOKEDPOTATO

POTATO
   VAR INT i, INT n.

   DO
      INPUT "n? " n.
   WHILE ( n <> -1 )
      FOR ( i=0. i<n. i=i+1.)
         FRY "Fibonacci(",i,")     = ",Fibonacci(i),PEEL.
      COOKEDPOTATO
   COOKEDPOTATO
COOKEDPOTATO
