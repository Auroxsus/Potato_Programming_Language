||------------------------------------------------------------
|| Auroxsus
|| Description: Comprehensive assertions testing
|| Potato8.p
||------------------------------------------------------------

crisp mainCrisp() {

   || Test Case 1: Simple Assertion
   spudling x = 10 : y = 5;
   { x > y } 
   bite "Assertion passed: x > y", endl;

   || Test Case 2: Failing Assertion
   || { x < y } 
   || Expected behavior: Runtime error due to failed assertion
   || Uncomment to test failure:
   || bite "This will not print if assertion fails!", endl;

   || Test Case 3: Compound Conditions
   { (x > y) && (y > 0) } 
   bite "Compound condition assertion passed: x > y && y > 0", endl;

   { (x < y) \\ (y <= x) } 
   bite "Compound condition assertion passed: x < y \\\\ y <= x", endl;

   || Test Case 4: Nested Assertions
   spudling z = 15;
   { z > x } 
   bite "Assertion passed: z > x", endl;

   bake (z > y) {
      { z > y } 
      bite "Nested assertion passed: z > y", endl;
   } else {
      bite "This should not execute if z > y assertion passes.", endl;
   }

   || Test Case 5: Assertions in Loops
   spudling i = 0;
   fryAsLongAs (i < 5) {
      { i >= 0 } 
      bite "Loop assertion passed: i >= 0 for i = ", i, endl;
      i = i + 1;
   }

   || Test Case 6: Assertions with Variable Updates
   spudling a = 3 : b = 4;
   { b == a + 1 } 
   bite "Assertion passed: b == a + 1", endl;

   b = 7;
   || { b == a + 1 } 
   || Expected behavior: Runtime error due to failed assertion
   || Uncomment to test failure:
   || bite "This will not print if assertion fails!", endl;

   || Test Case 7: Boundary Conditions
   spudling min = 0 : max = 100;
   { (x >= min) && (x <= max) } 
   bite "Boundary condition assertion passed: x in [min, max]", endl;

   x = 101;
   || { (x >= min) && (x <= max) } 
   || Expected behavior: Runtime error due to failed assertion
   || Uncomment to test failure:
   || bite "This will not print if assertion fails!", endl;

   || Test Case 8: Assertions with Boolean Variables
   tater flag = TRUE;
   { flag } 
   bite "Assertion passed: flag is TRUE", endl;

   flag = FALSE;
   ||{ flag } 
   || Expected behavior: Runtime error due to failed assertion
   || Uncomment to test failure:
   || bite "This will not print if assertion fails!", endl;


   || Test Case 9: Assertion with Logical NOT
   spudling num = -1;
   { !(num > 0) }
   bite "Assertion passed: !(num > 0)", endl;

   bite "All enabled assertions passed successfully.", endl;

}
