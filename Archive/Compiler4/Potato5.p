||------------------------------------------------------------
|| Auroxsus
|| Description: demonstrates the use of variables, constants, 
|| and basic arithmetic and logical operations
|| Potato5.p
||------------------------------------------------------------

spudling x: y;

crisp mainCrisp () {
    do {
            unearth "x? " x;
        } fryAsLongAs (x <> 0);
    unearth "y? " y;
    
    bake (x < y) {
        bite "x < y", endl;
    }

    bake (x != y) {
        bite "x != y", endl;
    }

    bake (x <> y) {
        bite "x <> y", endl;
    }

    bake (x < y) {
        bite "x < y", endl;
    } else {
        bite "x >= y", endl;
    }

    bake (x < y) {
        bite "x < y", endl;
    } elbake (x == y) {
        bite "x = y", endl;
    } elbake (x > y) {
        bite "x > y", endl;
    }

    bake (x < y) {
        bite "x < y", endl;
    } elbake (x == y) {
        bite "x = y", endl;
    } elbake (x > y) {
        bite "x > y", endl;
    } else {
        bite "***ERROR***", endl;
    }

    bake (x == y) {
        bite "x = y", endl;
    } else {
        bake (x > y) {
            bite "x > y", endl;
        } else {
            bite "x < y", endl;
        }
    }
}