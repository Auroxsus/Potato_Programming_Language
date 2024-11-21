||------------------------------------------------------------
|| Auroxsus
|| Description: Switch case testing *SIMPLIFIED*
|| Potato9.p
||------------------------------------------------------------

crisp mainCrisp() {

spudling x = 1 : y = 6 : z = 0;

	TATERTOGGLE (ONE) { 
		COOK x == 1: 
			bite "x is equal to 1", endl ;
			SERVE;
		
		COOK y > 5: 
			bite "y is greater than 5", endl ;
			SERVE;
		
		COOK z == 0: 
			bite "z is zero", endl;
			SERVE;
		
		DEFAULT: 
			bite "No conditions are true", endl;
		
	}
	
	TATERTOGGLE (ALL) { 
		COOK x == 1: 
			bite "x is equal to 1", endl ;
			SERVE;
		
		COOK y > 5: 
			bite "y is greater than 5", endl ;
			SERVE;
		
		COOK z == 0: 
			bite "z is zero", endl;
			SERVE;
		
		DEFAULT: 
			bite "ALL conditions are true", endl;
		
	}

}