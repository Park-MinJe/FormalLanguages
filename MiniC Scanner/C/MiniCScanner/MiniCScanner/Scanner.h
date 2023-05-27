/***************************************************************
*      scanner routine for Mini C language                    *
*                                   2003. 3. 10               *
***************************************************************/

#pragma once


//#define NO_KEYWORD 7
#define NO_KEYWORD 16
#define ID_LENGTH 12
#define STR_LENGTH 509

#define FILE_LEN 30

/**
 * additional token attributes
 * 
 * char fileName[FILE_LEN]
 * int lineNumber
 * int columnNumber
*/
struct tokenType {
	int number;
	union {
		char id[ID_LENGTH];
		double num;
		char c;
		char s[509];
	} value;

	// additional token attributes
	char fileName[FILE_LEN];
	int lineNumber;
	int columnNumber;
};


enum tsymbol {
	tnull = -1,
	tnot, tnotequ, tremainder, tremAssign, tIdent, tInteger,
	/* 0          1            2         3            4          5     */
	tand, tlparen, trparen, tmul, tmulAssign, tplus,
	/* 6          7            8         9           10         11     */
	tinc, taddAssign, tcomma, tminus, tdec, tsubAssign,
	/* 12         13          14        15           16         17     */
	tdiv, tdivAssign, tsemicolon, tless, tlesse, tassign,
	/* 18         19          20        21           22         23     */
	tequal, tgreat, tgreate, tlbracket, trbracket, teof,
	/* 24         25          26        27           28         29     */
	//   ...........    word symbols ................................. //
	tconst, telse, tif, tint, treturn, tvoid,
	/* 30         31          32        33           34         35     */
	twhile, tlbrace, tor, trbrace, 
	/* 36         37          38        39                             */

	//   ...........    additional keyword ........................... //
	tchar,  tdouble,  tfor,  tdo,  tgoto,  tswitch,  tcase,
	/* 40      41      42     43     44       45      46               */
	tbreak,  tdefault,
	/* 47       48                                                     */

	//   ...........    additional operand ........................... //
	tcolon,	tquote,	tdoubleQuote,	tCharacter,	tDouble,	tString
	/* 49    50         51              52        53           54      */
};


struct tokenType scanner();
void printToken(struct tokenType token);
