|| Potato.p by Auroxsus
|| French Fry Productions
|| Description: testing complex subprogram
||-----------------------------------------------------------

VAR INT x, INT y.
VAR BOOL z.

PROCEDURE PIN0
   FRY "PIN0()",PEEL.
   RETURN.
COOKEDPOTATO

PROCEDURE PIN1(INT x)
   FRY "PIN1(",x,")",PEEL.
||   RETURN.
COOKEDPOTATO

PROCEDURE PIN2(INT x,IN INT y)
   FRY "PIN2(",x,",",y,")",PEEL.
COOKEDPOTATO

PROCEDURE POUT1(OUT BOOL b)
   FRY "POUT1",PEEL.
   b = true.
COOKEDPOTATO

PROCEDURE POUT2(OUT INT x, OUT INT y)
   FRY "POUT2",PEEL.
   x = 2.
   y = 2*x.
   RETURN.
COOKEDPOTATO

PROCEDURE PIN1R(INT x)
   IF ( x > 0 ) 
      FRY x,PEEL.
      CALL PIN1R(x-1).
   COOKEDPOTATO
COOKEDPOTATO

PROCEDURE PIN1OUT1(IN INT x,OUT INT y)
   FRY "PIN1OUT1",PEEL.
   y = x.
COOKEDPOTATO

PROCEDURE PIN1OUT1R(INT x,OUT INT y)
   IF ( x == 0 ) 
      y = 42.
   ELSE
      CALL PIN1OUT1R(x-1,y).
   COOKEDPOTATO
COOKEDPOTATO

PROCEDURE PIO1(IO INT x)
   FRY "PIO1",PEEL.
   x = x+2.
COOKEDPOTATO

PROCEDURE PIO2(IO INT x1,IO INT x2)
   VAR INT T.

   FRY "PIO2",PEEL.
   T = x1.
   x1 = x2.
   x2 = T.
COOKEDPOTATO

PROCEDURE PREF2(REF INT i,REF BOOL x)
   FRY "PREF2 (i,x) = (",i,",",x,")",PEEL.
   x = NOT x.
   RETURN.
COOKEDPOTATO

POTATO
   VAR INT i.

   CALL PIN0.

   CALL PIN1(1).

   x = 2.
   CALL PIN2(x,x+2).

   CALL POUT1(z).
   FRY "z = ",z,PEEL.

   CALL POUT2(x,y).
   FRY "x = ",x,", y = ",y,PEEL.

   FRY "PIN1R(3)",PEEL.
   CALL PIN1R(3).

   x = 1.
   CALL PIN1OUT1(x,y).
   FRY "y = ",y,PEEL.

   FRY "PIN1OUT1R",PEEL.
   CALL PIN1OUT1R(3,y).
   FRY "y = ",y,PEEL.

   y = 7.
   CALL PIO1(y).
   FRY "y = ",y,PEEL.

   x = 3. y = 7.
   FRY "(x,y) = (",x,",",y,")",PEEL.
   CALL PIO2(x,y).
   FRY "(x,y) = (",x,",",y,")",PEEL.

   FOR (i = 1. i < 2. i = i + 1.)
      CALL PREF2(i,z).
   COOKEDPOTATO

COOKEDPOTATO
