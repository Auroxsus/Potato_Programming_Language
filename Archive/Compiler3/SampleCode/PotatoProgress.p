||------------------------------------------------------------
|| Auroxsus
|| Potato Progress Reporter
|| Description: demonstrates the use of variables, constants,
|| arithmetic operations, logical operations, and user input/output
|| PotatoProgress.p
||------------------------------------------------------------

|| global variables
spud plantedPotatoes; 
spud extraPotatoes, totalPotatoes;
chipper successMessage = "Well done! You are on track!";
chipper encouragementMessage = "Keep planting, more spuds to go!";

|| constants
root spud targetPotatoes = 50;
root tater farmingActive = true;

crisp mainCrisp () {
   || local variables
   spud currentPlanting;
   tater positiveOutcome;
   root spud initialContribution = 10;

   || Print initial state
   bite "Target potatoes for the season: ", targetPotatoes, endl;
   bite "Initial contribution from root stash: ", initialContribution, endl;

   || User input: how many potatoes planted today and previously?
   unearth "How many potatoes did you plant today? ", currentPlanting;
   unearth "How many potatoes were already planted? ", plantedPotatoes;

   || Calculate the total number of potatoes planted
   totalPotatoes = plantedPotatoes + currentPlanting + initialContribution;
   bite "Total potatoes planted so far: ", totalPotatoes, endl;

   || Logical calculation to determine if progress is good (simulated without IF)
   positiveOutcome = (totalPotatoes >= targetPotatoes) +\ farmingActive;
   bite "Farming status is active: ", farmingActive, endl;

   || Output messages based on progress (simulated without conditionals)
   bite "Progress Report: ", endl;
   bite "---------------------------------------", endl;
   bite "Total Potatoes Planted: ", totalPotatoes, endl;
   bite "Farming progress: ", positiveOutcome, endl;
   bite successMessage, endl;
   bite encouragementMessage, endl;

   || Increment and decrement examples for fun
   bite "Today's planting count: ", currentPlanting, ", let's increment it to show tomorrow's goal: ", ++currentPlanting, endl;
   bite "Oops, that's too much, decrement to: ", --currentPlanting, endl;

   || Static semantic error example (uncomment to test)
   |[ targetPotatoes = 60; ]|
   
   || Farewell message
   bite "Keep on farming! You will reach the goal soon!", endl;
}
