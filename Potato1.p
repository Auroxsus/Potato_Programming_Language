||------------------------------------------------------------
|| Auroxsus
|| Description: sample print statments
|| Potato1.p
||------------------------------------------------------------
crisp mainCrisp () {
   
   || example 1: Simple string with newline escape sequence
   bite "\"What's up, spud!\"\n";

   || example 2: Concatenated string with spaces and punctuation
   bite "\"Potatos", " are ", "great!\"", ENDL;

   || example 3: Mixing escape sequences for formatting
   bite "\"Menu:\tPotato Fries\n\tPotato Wedges\n\tMashed Potatoes\"", ENDL;

   || example 4: Using single quotes for a simple message
   bite "'Potatoes are life!'", ENDL;

   || example 5: Currency or symbol within a message
   bite "\"Price per lb: $4.99\"", ENDL;

   || example 6: Sentence split across multiple `bite` statements
   bite "\"Let's\"";
   bite " ", "\"grow\"";
   bite " ", "\"potatoes!\"", ENDL;

}

