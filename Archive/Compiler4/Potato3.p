||------------------------------------------------------------
|| Auroxsus
|| Description: demonstrates the use of variables, constants, 
|| and basic arithmetic and logical operations
|| Potato3.p
||------------------------------------------------------------

|| global variables
spudling x1; 
spudling x2 : x3 = 4; || ERROR: currently doesn't hold this value
                      || Expected: x2 is uninitialized, x3 is assigned 4

|| constants
spud spudling c1 = 0;  
spud tater c2 = true;

crisp mainCrisp () {
   || local variables
   spudling sum : x4;
   tater b1 : b2 = true;
   spud spudling c3 = 3; 
   spud tater c4 = false;
   
   || print constants
   bite "c1 = ", c1, endl;
   bite "c2 = ", c2, endl;
   bite "c3 = ", c3, endl;
   bite "c4 = ", c4, endl;
   
   || ERROR: NO VALUES
   bite "x2 (4?) = ", x2, endl; || to make sure x2 doesn't hold X3's assignment
   bite "x3 (4) = ", x3, endl;  || make sure x3 hold their assignment
    
   |[ sample static semantic error
   c1 = 2; || assignment attempt to a constant
   c1 plant 2;
   spudling x1; || unable to name local a global name
   uncomment and try! ]|
    
   x1 = 0;
   x1 plant 0;
   unearth "x1? " x1; || prompt user for unput
   unearth x2;
   sum = x1 + x2 + c1;
   bite "x1 + x2 + c1 = ", sum, endl;
   sum = x1 mash x2 mash c1;
   bite "x1 + x2 + c1 = ", sum, endl;
   
|[----------------------|| 
|| Compound Assignments ||
||----------------------]|
   
   || Use compound assignment operators
   x1 += 5;             || Increment x1 by 5
   bite "x1 += 5 ", x1, endl;
   x1 mashed 5;
   bite "x1 += 5 ", x1, endl;
   x2 -= 1;             || Reduce x2 by 1
   bite "x2 -= 1 ", x2, endl;
   x2 peeled 1;
   bite "x2 -= 1 ", x2, endl;
   
   |[------------------------|| 
   || ERROR: uninitialized   ||
   || x4 *= 2;               ||
   || bite "x4 = ", x4, endl;||
   || x4 fried 2;            ||
   || bite "x4 = ", x4, endl;||
   ||------------------------]| 
   
   x1 *= x2;
   bite "x1 *= x2 ", x1, endl;
   x2 fried x1;
   bite "x2 *= x1 ", x2, endl;
   x1 /= x2;
   bite "x1 /= x2 ", x1, endl;
   x2 sliced x1;
   bite "x2 /= x1 ", x2, endl;
   
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


}