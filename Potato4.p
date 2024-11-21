||------------------------------------------------------------
|| Auroxsus
|| Description: if/elif/else testing
|| Potato4.p
||------------------------------------------------------------

spudling x: y;

crisp mainCrisp () {
    
    unearth "x? " x;
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