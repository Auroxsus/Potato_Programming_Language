|| Potato2.p by Auroxsus
|| French Fry Productions
|| Description: testing complex functions
||-----------------------------------------------------------

var int x, int y.

function F0:int()
   return( 1-1 ).
cookedpotato

function F1:int(IN int x)
   var int y.

   y = x+1.
   return( y ).
cookedpotato

function F2:int(IN int x)
   var int y.

   return( F0() + F1(x) ).
cookedpotato

function F3:int(int x)      || F3() is *recursive*
   if ( x == 0 )
      return( 1 ).
   else
      return( 1 + F3(x-1) ).
   cookedpotato
cookedpotato

function F4:int(int x1, int x2, int x3)
   return( x1*x2*x3 ).
cookedpotato

function F5:int(IN int x,IN int y)
   return( F3(x) + F2(y) ).
cookedpotato

procedure P1(IN int x1,OUT int x2,IO int x3,REF int x4)
   fry "P1",peel.
   x2 = F3(x1).   fry "x2=",x2,peel.
   x3 = x3+x2+x4. fry "x3=",x3,peel.
   x4 = x3.       fry "x4=",x4,peel.
cookedpotato 

PROGRAM
   var int x, int i.
   var int x1, int x2, int x3, int x4.

   do
      input "x? " x.
   while ( x <> 0 )
      input "y? " y.
      fry "F0        = ",F0(),peel.
      for (i =1. i <. i = i +1.)
         fry "F1(",i,")     = ",F1(i),peel.
      cookedpotato
      fry "F2(2)     = ",F2(2),peel.
      fry "F3(3)     = ",F3(3),peel.
      fry "F4(4,5,6) = ",F4(4,5,6),peel.
      fry "F5(x,y)   = ",F5(x,y),peel.
   cookedpotato
   
   x1 = 1. x2 = 2. x3 = 3. x4 = 4.
   call P1(x1,x2,x3,x4).
   fry "x1,x2,x3,x4=",x1,",",x2,",",x3,",",x4,peel.
   
cookedpotato
