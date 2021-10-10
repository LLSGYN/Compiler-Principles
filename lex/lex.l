%{
/* definitions of manifest constants
LT, LE, EQ, NE, GT, GE,
IF, THEN, ELSE, WHILE, ID, NUMBER, RELOP */
#include <stdio.h>
#include <stdlib.h>
%}

/* regular expresions */
delim	[ \t\n ]
ws		{delim}+
letter	[A-Za-z_]
digit	[0-9]
id		{letter}({letter}|{digit})*
number	{digit}+(\.{digit}+)?(E[+-]{digit}+)?

%%

{ws}		{/* no action and no return */}
if			{return(IF)}
then		{return(ELSE)}
while		{return(WHILE)}
{id}		{yylval = (int) installID(); return(ID);}
{number}	{yylval = (int) installID(); return(NUMBER);}
"<"			{yylval = LT; return(RELOP);}
"<="		{yylval = LE; return(RELOP);}
"="			{yylval = EQ; return(RELOP);}
"!="		{yylval = NE; return(RELOP);}
">"			{yylval = GT; return(RELOP);}
">="		{yylval = GE; return(RELOP);}

%%

int installID() {

}

int installNum() {

}
