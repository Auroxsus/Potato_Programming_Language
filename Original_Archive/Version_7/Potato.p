|| Potato.p by Auroxsus
|| French Fry Productions
|| Description: testing small function
||-----------------------------------------------------------


VAR INT global.

FUNCTION F: INT()
   global = 1.                        || causes static semantic error!
   RETURN( global ).
COOKEDPOTATO

POTATO
   FRY "F() = ",F(),PEEL.
COOKEDPOTATO
