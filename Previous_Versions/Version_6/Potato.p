|| Potato.p by Auroxsus
|| French Fry Productions
|| Description: testing subprogram
||-----------------------------------------------------------

VAR INT x, INT y.

PROCEDURE P(IN INT in1, OUT INT out1, IO INT io1, REF BOOL ref1)
   CON INT C1 = 101.
   
   out1 = in1+C1.
   io1 = io1+1.
   ref1 = NOT ref1.
COOKEDPOTATO

POTATO
   CON INT P1 = 1.

   VAR INT P2, INT P3.
   VAR BOOL P4.

   P3 = 3.
   P4 = TRUE.
   
   CALL P(P1,P2,P3,P4).
   FRY "P1 = ",P1,PEEL.
   FRY "P2 = ",P2,PEEL.
   FRY "P3 = ",P3,PEEL.
   FRY "P4 = ",P4,PEEL.
COOKEDPOTATO
