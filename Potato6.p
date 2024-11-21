||------------------------------------------------------------
|| Auroxsus
|| Description: While loop testing
|| Potato6.p
||------------------------------------------------------------

spudling i: j: n;
tater stopLoop;

crisp mainCrisp() {

    || Test Case 1: Standard Increment
    bite endl, "Test Case 1: Standard Increment", endl;
    unearth "Limit? " n;
    i = 1;
    fryAsLongAs (i <= n) {
        bite "Count: ", i, endl;
        i = i + 1;
    }

    || Test Case 2: Empty Body
    bite endl, "Test Case 2: Empty Body", endl;
    i = 0;
    fryAsLongAs (i < 5) { i = i + 1; }
    bite "Loop finished. i: ", i, endl;

    || Test Case 3: Boundary Condition
    bite endl, "Test Case 3: Boundary Condition", endl;
    i = 5;
    fryAsLongAs (i >= 5) {
        bite "Counter: ", i, endl;
        i = i - 1;
    }

    || Test Case 4: Nested Loops
    bite endl, "Test Case 4: Nested Loops", endl;
    i = 1;
    fryAsLongAs (i <= 3) {
        j = 1;
        bite "Outer Loop, i: ", i, endl;
        fryAsLongAs (j <= 2) {
            bite "  Inner Loop, j: ", j, endl;
            j = j + 1;
        }
        i = i + 1;
    }

    || Test Case 5: Variable Updates in Condition
    bite endl, "Test Case 5: Variable Updates in Condition", endl;
    i = 1: j = 10;
    fryAsLongAs (i < j) {
        bite "i: ", i, " j: ", j, endl;
        i = i + 2; j = j - 1;
    }

    || Test Case 6: Zero or Negative Variable Test
    bite endl, "Test Case 6: Zero or Negative Variable Test", endl;
    i = -5;
    fryAsLongAs (i < 0) {
        bite "i: ", i, endl;
        i = i + 1;
    }

    || Test Case 7: Multiple Exits
    bite endl, "Test Case 7: Multiple Exits", endl;
    i = 0;
    fryAsLongAs (TRUE) {
        bite "Iteration: ", i, endl;
        bake (i == 3) {
            bite "Exiting early.", endl;
            serve;
        }
        i = i + 1;
    }

    || Test Case 8: Complex Condition
    bite endl, "Test Case 8: Complex Condition", endl;
    i = 2: j = 20;
    fryAsLongAs ((i * 2) < j && j > 10) {
        bite "i: ", i, " j: ", j, endl;
        i = i + 1; j = j - 2;
    }

    || Test Case 9: Condition Always False
    bite endl, "Test Case 9: Condition Always False", endl;
    i = 0;
    fryAsLongAs (i > 10) {
        bite "This should not print!", endl;
    }
    
    |[------------------------------------------------------------||
    || Test Case 10: Infinite Loop Without Exit
    || bite endl, "Test Case 10: Condition Always True", endl;
    || fryAsLongAs (TRUE) {
    ||     bite "Infinite Loop.", endl;
    || }
    || Note: Uncomment carefully, as this will run indefinitely.
    ||-------------------------------------------------------------]|
    
    || Test Case 12: Multiple Breaks in a Single Loop 
    bite "Test Case 11: Multiple Breaks in a Single Loop", endl;
    fryAsLongAs (TRUE) {
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
        i = i + 1;
    }

}
