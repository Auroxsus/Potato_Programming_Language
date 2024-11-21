||------------------------------------------------------------
|| Auroxsus
|| Description: Demonstrates the use of variables, constants, 
||              and basic arithmetic and logical operations
|| Potato3.p
||------------------------------------------------------------

|| Global variables
spudling x1;                   || Uninitialized
spudling x2 : x3 = 4;          || x2 uninitialized, x3 initialized to 4

|| Constants
spud spudling x4 = 8: x5 = 12; || Multiple initializations in one declaration
spud spudling c1 = 0;          || Constant integer
spud tater c2 = true;          || Constant boolean

crisp mainCrisp() {
   || Local variables
   spudling sum: x6 = 20;          || Declaration with uninitialized and initialized variables
   tater b1: b2 = true;            || Boolean variables
   spud spudling c3 = 3;           || Local constant integer
   spud tater c4 = false;          || Local constant boolean
   
   || print constants
   bite "c1 = ", c1, endl;
   bite "c2 = ", c2, endl;
   bite "c3 = ", c3, endl;
   bite "c4 = ", c4, endl;
   
   || Global variable values
   bite endl, "Global variables:", endl;
   bite "x2 (uninitialized) = ", x2, endl;
   bite "x3 (initialized to 4) = ", x3, endl;
   bite "x4 (initialized to 8) = ", x4, endl;
   
   || Prompt user for input
   bite endl, "User input:", endl;
   unearth "Enter value for x1: " x1;
   unearth "Enter value for x2: " x2;
   sum = x1 + x2 + c1;
   bite "Sum (x1 + x2 + c1) = ", sum, endl;
   
   |[----------------------|| 
   || Compound Assignments ||
   ||----------------------]|
   
   || Use compound assignment operators
   || x1 += 5;             || Increment x1 by 5
   || bite "x1 += 5 ", x1, endl;
   || x1 mashed 5;
   || bite "x1 += 5 ", x1, endl;
   || x2 -= 1;             || Reduce x2 by 1
   || bite "x2 -= 1 ", x2, endl;
   || x2 peeled 1;
   || bite "x2 -= 1 ", x2, endl;
   
   |[------------------------|| 
   || ERROR: uninitialized   ||
   || x5 *= 2;               || Attempt to use uninitialized variable
   || bite "x5 = ", x5, endl;||
   || x5 fried 2;            || This will fail if uncommented
   || bite "x5 = ", x5, endl;||
   ||------------------------]| 
   
   || x1 *= x2;
   || bite "x1 *= x2 ", x1, endl;
   || x2 fried x1;
   || bite "x2 *= x1 ", x2, endl;
   || x1 /= x2;
   || bite "x1 /= x2 ", x1, endl;
   || x2 sliced x1;
   || bite "x2 /= x1 ", x2, endl;
   
   || increment and decrement operators   
   bite "x1 = ", x1, ", ++x1 = ", ++x1, endl;
   bite "x1 = ", x1, ", --x1 = ", --x1, endl;
   || bite "x1 = ", x1, ", x1++ = ", x1++, endl;
   || bite "x1 = ", x1, ", x1-- = ", x1--, endl;
    
   || increment and decrement operators   
   bite "x1 = ", x1, ", ++x1 = ", sprout x1, endl;
   bite "x1 = ", x1, ", --x1 = ", root x1, endl;
   || bite "x1 = ", x1, ", x1++ = ", x1 sprout, endl;
   || bite "x1 = ", x1, ", x1-- = ", x1 root, endl;
   
   unearth "b1? " b1;
   b2 = !b1 \\ (c2 +\ c4);
   b2 = raw b1 mix (c2 eitherSpud c4);
   bite "b1 = ", b1, ", b2 = ", b2, endl;

   |[----------------------|| 
   || Static Errors Demo   ||
   ||----------------------]|
   ||  bite endl, "Static Semantic Error Examples:", endl;

   |[-----------------------------||
   || c1 = 2; || Error: Assignment to constant
   || c1 plant 2;
   ||spudling x1; || Error: Redeclaring global variable locally
   || Note: Uncomment carefully
   ||-----------------------------]|

}