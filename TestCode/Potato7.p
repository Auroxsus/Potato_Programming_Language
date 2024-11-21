||------------------------------------------------------------
|| Auroxsus
|| Description: Do-While loop testing
|| Potato7.p
||------------------------------------------------------------

spudling i: j: n;
tater stopLoop;

crisp mainCrisp() {

    || Test Case 1: Standard Increment
    bite "Test Case 1: Standard Increment", endl;
    unearth "Limit? " n;
    i = 1;
    do {
        bite "Count: ", i, endl;
        i = i + 1;
    } fryAsLongAs (i <= n);

    || Test Case 2: Empty Body
    bite "Test Case 2: Empty Body", endl;
    i = 0;
    do {
        i = i + 1; || Increment with no additional logic
    } fryAsLongAs (i < 5);
    bite "Loop finished. i: ", i, endl;

    || Test Case 3: Boundary Condition
    bite "Test Case 3: Boundary Condition", endl;
    i = 5;
    do {
        bite "Counter: ", i, endl;
        i = i - 1;
    } fryAsLongAs (i >= 5);

    || Test Case 4: Nested Loops
    bite "Test Case 4: Nested Loops", endl;
    i = 1;
    do {
        bite "Outer Loop, i: ", i, endl;
        j = 1;
        do {
            bite "  Inner Loop, j: ", j, endl;
            j = j + 1;
        } fryAsLongAs (j <= 2);
        i = i + 1;
    } fryAsLongAs (i <= 3);

    || Test Case 5: Variable Updates in Condition
    bite "Test Case 5: Variable Updates in Condition", endl;
    i = 1;
    j = 10;
    do {
        bite "i: ", i, " j: ", j, endl;
        i = i + 2;
        j = j - 1;
    } fryAsLongAs (i < j);

    || Test Case 6: Zero or Negative Variable Test
    bite "Test Case 6: Zero or Negative Variable Test", endl;
    i = -5;
    do {
        bite "i: ", i, endl;
        i = i + 1;
    } fryAsLongAs (i < 0);

    || Test Case 7: Multiple Exits
    bite "Test Case 7: Multiple Exits", endl;
    i = 0;
    do {
        bite "Iteration: ", i, endl;
        bake (i == 3) {
            bite "Exiting early.", endl;
            serve; || Exit the loop
        }
        i = i + 1;
    } fryAsLongAs (TRUE);

    || Test Case 8: Complex Condition
    bite "Test Case 8: Complex Condition", endl;
    i = 2;
    j = 20;
    do {
        bite "i: ", i, " j: ", j, endl;
        i = i + 1;
        j = j - 2;
    } fryAsLongAs ((i * 2) < j && j > 10);

    || Test Case 9: Condition Always False
    bite "Test Case 9: Condition Always False", endl;
    i = 0;
    do {
        bite "This should not print!", endl;
    } fryAsLongAs (i > 10);

    |[------------------------------------------------------------||
    || Test Case 10: Infinite Loop Without Exit
    || bite "Test Case 10: Infinite Loop Without Exit", endl;
    || i = 0;
    || do {
    ||     bite "Infinite Loop, i: ", i, endl;
    ||     i = i + 1;
    || } fryAsLongAs (TRUE);
    || Note: Uncomment carefully, as this will run indefinitely.
    ||-------------------------------------------------------------]|

    || Test Case 11: Multiple Breaks in a Single Loop
    bite "Test Case 11: Multiple Breaks in a Single Loop", endl;
    i = 0;
    do {
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
    } fryAsLongAs (TRUE);

}
