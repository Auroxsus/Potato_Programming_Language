||------------------------------------------------------------
|| Auroxsus
|| Description: demonstrates the use of variables, constants, 
|| and basic arithmetic and logical operations with reserved
|| word syntax
|| Potato3_2.p
||------------------------------------------------------------

|| global variables
spudling x1; 
spudling x2 : x3;

|| constants
spud spudling c1 = 2;  
spud tater c2 = true;

crisp mainCrisp () {
  || local variables
   spudling sum : x4;
   tater b1 : b2;
   spud spudling c3 = 3; 
   spud tater c4 = false;
   
   || print constants
   bite "c1 = ", c1, endl;
   bite "c2 = ", c2, endl;
   bite "c3 = ", c3, endl;
   bite "c4 = ", c4, endl;
	
   
   |[ sample static semantic error
   c1 plant 2; || assignment attempt to a constant
   spudling x1; || unable to name local a global name
   uncomment and try! ]|
    
   x1 plant 0;
   unearth "x1? " x1; || prompt user for unput
   unearth x2;
   sum = x1 mash x2 mash c1;
   bite "x1 + x2 + c1 = ", sum, endl;
   
   || increment and decrement operators   
   bite "x1 = ", x1, ", ++x1 = ", sprout x1, endl;
   bite "x1 = ", x1, ", --x1 = ", root x1, endl;
   
   unearth "b1? " b1;
   b2 = raw b1 mix (c2 eitherSpud c4);
   bite "b1 = ", b1, ", b2 = ", b2, endl;

	
}