|[ 
||	Potato.p
||	Purpose: Potato Chip testing of comments and string literals
]|

|[|[ 
   |["eat" these |[ quadruply nested ]| block comments ]|
]|]|

|| "eat" the empty block comment on the next line
|[]|

|| "eat" this "interesting" sequence of comments
|[ block comment #1 ]||[ followed by block comment #2]||| followed by line comment #3

|[ "eat" the 4 tabs characters on the next line ]|

|| reserved-word terminals
BITE ENDL
BITE bite ENDL endl

|| identifier pseudo-terminal
 a  ab  ab1  ab1c A2Z A_B_2

|| punctuation terminals
,. , .

|| string pseudo-terminal
"" "a" "ab" "abc"
'' 'a' 'ab' 'abc'

|| string pseudo-terminal with escape characters
"ab\"c" 
"\\abc\"" 
"newLine \n (LF)" 
"horizontalTab \t (HT)"
"backSpace \b (BS)"
"carriage return \r (CR)"
"\"\\\n\t\b\r"

|| non-SPL <string> (no escape sequences)
`` `a` `ab` `abc` `a``b````c`

|| *ERROR* un-terminated string pseudo-terminal 
"unterminated string

