||------------------------------------------------------------
|| Auroxsus
|| Description: Demonstrates the use of variables, constants,
|| and basic arithmetic, logical operations, and user interaction
|| Potato3.p (Enhanced Version)
||------------------------------------------------------------

|| Global Variables
spudling potatoCount;  
spudling field1 : field2;

|| Constants
spud spudling surplus = 2;  
spud tater isSurplusPotatoGood = true;

crisp mainCrisp () {
   || Local Variables
   spudling totalHarvest : tempPotato;
   tater isFieldActive : isHarvestSuccessful = true;
   spud spudling currentSurplus = 3;
   spud tater isWeatherFavorable = false;
   tater question;
   
   || Print constant values to the console for clarity
   bite "Surplus = ", surplus, endl; || from other seasons
   bite "Are the potato's still good? ", isSurplusPotatoGood, endl;
   bite "Current Surplus = ", currentSurplus, endl;
   bite "Is Weather Favorable? ", isWeatherFavorable, endl;

   || Request user input to assign values to variables from both fields
   unearth "Enter today's harvest from field 1: " field1;
   unearth "Enter today's harvest from field 2: " field2;

   || Calculations involving user input from both fields, surplus, and constants
   totalHarvest = potatoCount + field1 + field2 + currentSurplus;
   bite "Total Harvest = ", totalHarvest, endl;

   || Demonstrate increment and decrement operations
   unearth "Did you eat one? (true) " question; 
   bite "Updated potato count = ", potatoCount, ", Decremented potatoCount = ", --potatoCount, endl;
   unearth "Are you telling the truth? (false) " question; 
   bite "Current potato count = ", potatoCount, ", Incremented potatoCount = ", ++potatoCount, endl;
   
   || Request boolean input from user and demonstrate logical operation
   unearth "Is Field Active? (true/false): " isFieldActive;
   
   || the harvest is successful if either the field is inactive OR if the potatoes are good and the weather is favorable
   isHarvestSuccessful = !isFieldActive \\ (isSurplusPotatoGood +\ isWeatherFavorable);
   
   bite "Field Active: ", isFieldActive, ", Harvest Successful: ", isHarvestSuccessful, endl;

   || Farewell message
   bite "Thank you for farming with Auroxsus. Keep growing those spuds!", endl;
}

