||------------------------------------------------------------
|| Auroxsus
|| Description: sample operator statements
|| Potato2.p
||------------------------------------------------------------

crisp mainCrisp () {
|[----------------------||
|| Arithmetic Operators ||
||----------------------]|

   bite "1          = "              , 1                  , endl;
   bite "+2         = "              , +2                 , endl;
   bite "mash 2     = "              , mash 2             , endl;
   bite "-3         = "              , -3                 , endl;
   bite "peel 3     = "              , peel 3             , endl;
   bite "ABS 2      = "              , ABS 2              , endl;
   bite "ABS(-2)    = "              , ABS(-3)            , endl;
   bite "4 + 5      = "              , 4 + 5              , endl;
   bite "4 mash 5   = "              , 4 mash 5           , endl;
   bite "8 - 3      = "              , 8 - 3              , endl;
   bite "8 peel 3   = "              , 8 peel 3           , endl;
   bite "6 * 2      = "              , 6 * 7              , endl;
   bite "6 fry 2    = "              , 6 fry 7            , endl;
   bite "10 / 2     = "              , 10 / 2             , endl;
   bite "10 slice 2 = "              , 10 slice 2         , endl;
   bite "7 % 3      = "              , 7 % 3              , endl;
   bite "7 mod 3    = "              , 7 mod 3            , endl;
   bite "2 ^ 3      = "              , 2 ^ 3              , endl;
   bite "2 pow 3    = "              , 2 pow 3            , endl;

|[----------------------||
|| Comparison Operators ||
||----------------------]|

   bite "4 == 6                = "  , 4 == 6             , endl;
   bite "4 spudMatch 6         = "  , 4 spudMatch 6      , endl;
   bite "4 != 6                = "  , 4 != 6             , endl;
   bite "4 mashApart 6         = "  , 4 mashApart 6      , endl;
   bite "4 < 6                 = "  , 4 < 6              , endl;
   bite "4 smallerSpud 6       = "  , 4 smallerSpud 6    , endl;
   bite "4 > 6                 = "  , 4 > 6              , endl;
   bite "4 biggerSpud 6        = "  , 4 biggerSpud 6     , endl;
   bite "4 <= 6                = "  , 4 <= 6             , endl;
   bite "4 notBiggerSpud 6     = "  , 4 notBiggerSpud 6  , endl;
   bite "4 >= 6                = "  , 4 >= 6             , endl;
   bite "4 notSmallerSpud 6    = "  , 4 notSmallerSpud 6 , endl;

|[----------------------||
|| Logical Operators    ||
||----------------------]|

   bite "true && true          = "  , true && true       , endl;
   bite "true prep true        = "  , true prep true     , endl;
   bite "true && false         = "  , true && false      , endl;
   bite "true prep false       = "  , true prep false    , endl;
   bite "false && false        = "  , false && false     , endl;
   bite "false prep false      = "  , false prep false   , endl;

   bite "true \\\\ true          = " , true \\ true       , endl;
   bite "true mix true         = "   , true mix true      , endl;
   bite "true \\\\ false         = " , true \\ false      , endl;
   bite "true mix false        = "   , true mix false     , endl;
   bite "false \\\\ false        = " , false \\ false     , endl;
   bite "false mix false       = "   , false mix false    , endl;

   bite "false spudStop true   = "   , false spudStop true      , endl;
   bite "false ~\\ true         = "  , false ~\ true            , endl;
   bite "false spudStop false  = "   , false spudStop false     , endl;
   bite "false ~\\ false        = "  , false ~\ false           , endl;

   bite "true eitherSpud true  = "   , true eitherSpud true     , endl;
   bite "true +\\ true          = "  , true +\ true             , endl;
   bite "true eitherSpud false = "   , true eitherSpud false    , endl;
   bite "true +\\ false         = "  , true +\ false            , endl;
   bite "false eitherSpud false= "   , false eitherSpud false   , endl;
   bite "false +\\ false        = "  , false +\ false           , endl;

   bite "false mashless true   = "  , false mashless true      , endl;
   bite "false ~& true         = "  , false ~& true            , endl;
   bite "false mashless false  = "  , false mashless false     , endl;
   bite "false ~& false        = "  , false ~& false           , endl;
   bite "true mashless true    = "  , true mashless true       , endl;
   bite "true ~& true          = "  , true ~& true             , endl;
   bite "true mashless false   = "  , true mashless false      , endl;
   bite "true ~& false         = "  , true ~& false            , endl;

   bite "!true                 = "  , !true                    , endl;
   bite "raw true              = "  , raw true                 , endl;
   bite "!false                = "  , !false                   , endl;
   bite "raw false             = "  , raw false                , endl;

}