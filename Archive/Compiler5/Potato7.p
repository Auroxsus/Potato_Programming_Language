||------------------------------------------------------------
|| Auroxsus
|| Description: Demonstrate the use of assertions for program 
||              correctness and logical validation.
|| Potato7.p
||------------------------------------------------------------

crisp mainCrisp () {
   spudling x = 101 : y = 0;

   { ((x >= 0) && (y != 1)) } { TRUE } { RAW FALSE }

   y = x + 1;

   { x == y-1 } 
   || run-time error #1
   || { y < x } || truthfully y > x 

}
