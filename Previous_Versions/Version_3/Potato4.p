|| Potato4.p by Auroxsus
|| French Fry Productions
|| Description: Test variable assignments, inc/dec-rementation, 
|[ ------------------------------- ]|

VAR x1 INT.
CON c1 INT = 1,c2 BOOL = true.
VAR x2 INT,x3 INT.

POTATO
	VAR sum INT, x1 INT.
	VAR b1 BOOL,b2 BOOL.
	CON c3 INT = 3,c4 BOOL = false.

	FRY "c1 = ",c1,PEEL.
	FRY "c2 = ",c2,PEEL.
	FRY "c3 = ",c3,PEEL.
	FRY "c4 = ",c4,PEEL.

|[ Generates a static semantic (context-dependent) ||
||    error when un-commmented. Try it and see!    ]|
||   c1 = 2.
	x1 = 0.
	INPUT "x1? " x1.
	INPUT x2.
	sum = x1+x2+c1.
	FRY "x1+x2+c1 = ",sum,PEEL.

|| test new increment and decrement operators   
	FRY "x1 = ",x1,", ++x1 = ",++x1,PEEL.
	FRY "x1 = ",x1,", --x1 = ",--x1,PEEL.  
||	FRY "x1 = ",x1,", x1++ = ",x1++,PEEL. || Add postfix parsing to implement
||	FRY "x1 = ",x1,", x1-- = ",x1--,PEEL.

	INPUT "b1? " b1.
	b2 = NOT(b1) OR (c2 XOR c4).
	FRY "b1 = ",b1,", b2 = ",b2,PEEL.
COOKEDPOTATO
