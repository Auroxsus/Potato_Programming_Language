|| Potato1.p by Auroxsus
|| French Fry Productions
|| Description: test-and-debug
|| -------------------------------

|[|[
   |[ "eat" these |[ quadruply nested ]| block comments ]|
]|]|

|| "eat" the empty block comment on the next line
|[]|

|| "eat" this "interesting" sequence of comments
|[ block comment #1 ]||[ followed by block comment #2 ]||| followed by line comment #3

|[ "eat" the 4 tabs characters on the next line ]|
				
|| reserved-word terminals
mainpotato cookedpotato
MAINPOTATO COOKEDPOTATO
outpotato endl

|| identifier pseudo-terminal (a little before its time)
 a  ab  ab1  ab1c A2Z A_B_2

|| punctuation terminals
,. , .

|| string pseudo-terminal
"" "a" "ab" "abc"

|| string pseudo-terminal with escape characters
"ab\"c" 
"\\abc\"" 
"newLine \n (LF)" 
"horizontalTab \t (HT)"
"verticalTab \v (VT)"
"backSpace \b (BS)"
"carriage return \r (CR)"
"alert \a (BEL)"
"\"\\\n\t\v\b\r\a"

|| *ERROR* un-terminated string pseudo-terminal 
"unterminated\\

