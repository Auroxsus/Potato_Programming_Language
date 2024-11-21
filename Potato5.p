||------------------------------------------------------------
|| Auroxsus
|| Description: For loop testing
|| Potato5.p
||------------------------------------------------------------

spudling i: j: n;
tater stopLoop;

crisp mainCrisp() {
 
    || Test Case 1: Standard Increment
    bite endl, "Test Case 1: Standard Increment", endl;
    unearth "Limit? " n;
    season (i = 1; i <= n; i = i + 1) {
        bite "Count: ", i, endl;
    }

    || Test Case 2: Empty Body
    bite endl, "Test Case 2: Empty Body", endl;
    season (i = 0; i < 5; i = i + 1) { }
    bite "Loop finished. i: ", i, endl;

    || Test Case 3: Boundary Condition
    bite endl, "Test Case 3: Boundary Condition", endl;
    season (i = 5; i >= 5; i = i - 1) {
        bite "Counter: ", i, endl;
    }

    || Test Case 4: Nested Loops
    bite endl, "Test Case 4: Nested Loops", endl;
    season (i = 1; i <= 3; i = i + 1) {
        bite "Outer Loop, i: ", i, endl;
        season (j = 1; j <= 2; j = j + 1) {
            bite "  Inner Loop, j: ", j, endl;
        }
    }
    
    || Test Case 5: Variable Updates in Condition
      bite endl, "Test Case 5: Variable Updates in Condition", endl;
   season (i = 1: j = 10; i < j; i = i + 2 : j = j - 1) {
       bite "i: ", i, " j: ", j, endl;
   }
   
    || Test Case 6: Variable Updates in Condition
    bite endl, "Test Case 6: Variable Updates in Condition", endl;
    season (i = 1: j = 10; i < j; i = i + 2) {
        j = j - 1; || Update j inside the loop body
        bite "i: ", i, " j: ", j, endl;
    }

    || Test Case 7: Negative Variable Test
    bite endl, "Test Case 7: Zero or Negative Variable Test", endl;
    season (i = -5; i < 0; i = i + 1) {
        bite "i: ", i, endl;
    }

    || Test Case 8: Multiple Exits
    bite endl, "Test Case 8: Multiple Exits", endl;
    season (i = 0; TRUE; i = i + 1) {
        bite "Iteration: ", i, endl;
        bake (i == 3) {
            bite "Exiting early.", endl;
            serve; || Exit the loop
        }
    }

    || Test Case 9: Complex Condition
    bite endl, "Test Case 9: Complex Condition", endl;
    season (i = 2: j = 20; (i * 2) < j && j > 10; i = i + 1) {
        j = j - 2; || Update j inside the loop body
        bite "i: ", i, " j: ", j, endl;
    }

    || Test Case 10: Condition Always False
    bite endl, "Test Case 10: Condition Always False", endl;
    season (i = 0; i > 10; i = i + 1) {
        bite "This should not print!", endl;
    }
    
    |[------------------------------------------------------------||
    || Test Case 11: Infinite Loop Without Exit
    || bite endl, "Test Case 11: Infinite Loop Without Exit", endl;
    || season (i = 0; TRUE; i = i + 1) {
    ||    bite "Infinite Loop, i: ", i, endl;
    || }
    || Note: Uncomment carefully, as this will run indefinitely.
    ||-------------------------------------------------------------]|
    
    || Test Case 12: Multiple Breaks in a Single Loop
    bite endl, "Test Case 12: Multiple Breaks in a Single Loop", endl;
    season (i = 0; TRUE; i = i + 1) {
        bite "Iteration: ", i, endl;
        bake (i == 2) {
            bite "Breaking early at i = 2.", endl;
            serve; || Break out of the loop
        }
        bake (i == 4) {
            bite "Breaking early at i = 4.", endl;
            serve; || Break out of the loop
        }
        bake (i == 5) {
            bite "Breaking final at i = 5.", endl;
            serve; || Final break
        }
    }

}
