/***************************************************************
*      scanner routine for Mini C language                    *
***************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "Scanner.h"

extern FILE *sourceFile;                       // miniC source program
extern int lineNumber, columnNumber;		   // line & column number of token


int superLetter(char ch);
int superLetterOrDigit(char ch);
int getNumber(char firstCharacter);
int hexValue(char ch);
void lexicalError(int n);


char *tokenName[] = {
	"!",        "!=",      "%",       "%=",     "%ident",   "%integer",
	/* 0          1           2         3          4          5        */
	"&&",       "(",       ")",       "*",      "*=",       "+",
	/* 6          7           8         9         10         11        */
	"++",       "+=",      ",",       "-",      "--",	    "-=",
	/* 12         13         14        15         16         17        */
	"/",        "/=",      ";",       "<",      "<=",       "=",
	/* 18         19         20        21         22         23        */
	"==",       ">",       ">=",      "[",      "]",        "eof",
	/* 24         25         26        27         28         29        */
	//   ...........    word symbols ................................. //
	/* 30         31         32        33         34         35        */
	"const",    "else",     "if",      "int",     "return",  "void",
	/* 36         37         38        39                              */
	"while",    "{",        "||",       "}",

	//   ...........    additional keyword ........................... //
	"char",  "double",  "for",  "do",  "goto",  "switch",  "case",
	/* 40       41        42     43      44        45        46        */
	"break",  "default", 
	/* 47        48                                                    */

	//   ...........    additional operand ........................... //
	":",	"\'",	"\"",	"%character",	"%double",	"%string"
	/* 49    50      51          52             53          54         */
};

char *keyword[NO_KEYWORD] = {
	"const",  "else",    "if",    "int",    "return",  "void",    "while", 
	//   ...........    additional keyword ........................... //
	"char",  "double",  "for",  "do",  "goto",  "switch",  "case",  "break",  "default"
};

enum tsymbol tnum[NO_KEYWORD] = {
	tconst,    telse,     tif,     tint,     treturn,   tvoid,     twhile,
	tchar,  tdouble,  tfor,  tdo,  tgoto,  tswitch,  tcase, tbreak,  tdefault
};

struct tokenType scanner()
{
	struct tokenType token;
	int i, index;
	char ch, id[ID_LENGTH], str[STR_LENGTH];

	/**
	 * additional token attributes
	 * 
	 * char fileName[FILE_LEN]
	 * int lineNumber
	 * int columnNumber
	*/

	token.number = tnull;

	do {
		while (isspace(ch = fgetc(sourceFile)));	// state 1: skip blanks
		if (superLetter(ch)) { // identifier or keyword
			i = 0;
			do {
				if (i < ID_LENGTH) id[i++] = ch;
				ch = fgetc(sourceFile);
			} while (superLetterOrDigit(ch));
			if (i >= ID_LENGTH) lexicalError(1);
			id[i] = '\0';
			ungetc(ch, sourceFile);  //  retract
									 // find the identifier in the keyword table
			for (index = 0; index < NO_KEYWORD; index++)
				if (!strcmp(id, keyword[index])) break;
			if (index < NO_KEYWORD)    // found, keyword exit
				token.number = tnum[index];
			else {                     // not found, identifier exit
				token.number = tIdent;
				strcpy_s(token.value.id, id);
			}
		}  // end of identifier or keyword
		else if (ch == '\''){	// state 2: character literal
			i = 0;

			token.number = tchar;
			id[i++] = fgetc(sourceFile);

			// escape charater
			if(ch == '\\')
				id[i++] = fgetc(sourceFile);
			id[i] = '\0';
			
			strcpy_s(token.value.id, id);

			// error
			ch = fgetc(sourceFile);
			if(ch != '\''){
				lexicalError(5);
				ungetc(ch, sourceFile);
				token.number = tnull;
			}
		}
		else if (isdigit(ch)) {  // state 3: integer & double literal
			token.number = tInteger;
			token.value.num = getNumber(token, ch);
		}
		else if (ch == '\"') {	// state 4: string literal
			token.number = tString;
			
			i = 0;
			do {
				ch = fgetc(sourceFile);
				str[i++] = ch;

				// escape character
				if (ch == '\\')
					str[i++] = fgetc(sourceFile);
				
				// string not end error
				if (ch == '\n') {
					lexicalError(6);
					ungetc(ch, sourceFile);
					token.number = tnull;
					break;
				}
			} while (ch != '\"');

			// delete last "
			str[i] = '\0';
			strcpy_s(token.value.s, str);
		}
		else switch (ch) {  // special character
		case '/':
			ch = fgetc(sourceFile);
			if (ch == '*') {
				ch = fgetc(sourceFile);
				if(ch == '*'){		// documented comment
					do {				// text comment
						while (ch != '*') ch = fgetc(sourceFile);
						ch = fgetc(sourceFile);
					} while (ch != '/');
				}
				else {				// text comment
					do {
						while (ch != '*') ch = fgetc(sourceFile);
						ch = fgetc(sourceFile);
					} while (ch != '/');
				}
			}
			else if (ch == '/') {
				if(fgetc(sourceFile) == '/'){	// single line documented comment
					while (fgetc(sourceFile) != '\n');
				}
				else{			// line comment
					while (fgetc(sourceFile) != '\n');
				}
			}
			else if (ch == '=')  token.number = tdivAssign;
			else {
				token.number = tdiv;
				ungetc(ch, sourceFile); // retract
			}
			break;
		case '!':
			ch = fgetc(sourceFile);
			if (ch == '=')  token.number = tnotequ;
			else {
				token.number = tnot;
				ungetc(ch, sourceFile); // retract
			}
			break;
		case '%':
			ch = fgetc(sourceFile);
			if (ch == '=') {
				token.number = tremAssign;
			}
			else {
				token.number = tremainder;
				ungetc(ch, sourceFile);
			}
			break;
		case '&':
			ch = fgetc(sourceFile);
			if (ch == '&')  token.number = tand;
			else {
				lexicalError(2);
				ungetc(ch, sourceFile);  // retract
			}
			break;
		case '*':
			ch = fgetc(sourceFile);
			if (ch == '=')  token.number = tmulAssign;
			else {
				token.number = tmul;
				ungetc(ch, sourceFile);  // retract
			}
			break;
		case '+':
			ch = fgetc(sourceFile);
			if (ch == '+')  token.number = tinc;
			else if (ch == '=') token.number = taddAssign;
			else {
				token.number = tplus;
				ungetc(ch, sourceFile);  // retract
			}
			break;
		case '-':
			ch = fgetc(sourceFile);
			if (ch == '-')  token.number = tdec;
			else if (ch == '=') token.number = tsubAssign;
			else {
				token.number = tminus;
				ungetc(ch, sourceFile);  // retract
			}
			break;
		case '<':
			ch = fgetc(sourceFile);
			if (ch == '=') token.number = tlesse;
			else {
				token.number = tless;
				ungetc(ch, sourceFile);  // retract
			}
			break;
		case '=':
			ch = fgetc(sourceFile);
			if (ch == '=')  token.number = tequal;
			else {
				token.number = tassign;
				ungetc(ch, sourceFile);  // retract
			}
			break;
		case '>':
			ch = fgetc(sourceFile);
			if (ch == '=') token.number = tgreate;
			else {
				token.number = tgreat;
				ungetc(ch, sourceFile);  // retract
			}
			break;
		case '|':
			ch = fgetc(sourceFile);
			if (ch == '|')  token.number = tor;
			else {
				lexicalError(3);
				ungetc(ch, sourceFile);  // retract
			}
			break;
		case '(': token.number = tlparen;         break;
		case ')': token.number = trparen;         break;
		case ',': token.number = tcomma;          break;
		case ';': token.number = tsemicolon;      break;
		case ':': token.number = tcolon;		  break;
		case '[': token.number = tlbracket;       break;
		case ']': token.number = trbracket;       break;
		case '{': token.number = tlbrace;         break;
		case '}': token.number = trbrace;         break;
		case EOF: token.number = teof;            break;
		default: {
			printf("Current character : %c", ch);
			lexicalError(4);
			break;
		}

		} // switch end
	} while (token.number == tnull);
	return token;
} // end of scanner

void lexicalError(int n)
{
	printf(" *** Lexical Error : ");
	switch (n) {
	case 1: printf("an identifier length must be less than 12.\n");
		break;
	case 2: printf("next character must be &\n");
		break;
	case 3: printf("next character must be |\n");
		break;
	case 4: printf("invalid character\n");
		break;
	case 5: printf("next character must be \'\n");
		break;
	case 6: printf("string must be end in line\n");
		break;
	case 7: printf("next character must be Integer\n");
		break;
	default:
		break;
	}
}

int superLetter(char ch)
{
	if (isalpha(ch) || ch == '_') return 1;
	else return 0;
}

int superLetterOrDigit(char ch)
{
	if (isalnum(ch) || ch == '_') return 1;
	else return 0;
}

double getNumber(tokenType token, char firstCharacter)
{
	double num = 0;
	int value;
	char ch;

	if (firstCharacter == '0') {
		ch = fgetc(sourceFile);
		if ((ch == 'X') || (ch == 'x')) {		// hexa decimal
			while ((value = hexValue(ch = fgetc(sourceFile))) != -1)
				num = 16 * num + value;
		}
		else if ((ch >= '0') && (ch <= '7'))	// octal
			do {
				num = 8 * num + (int)(ch - '0');
				ch = fgetc(sourceFile);
			} while ((ch >= '0') && (ch <= '7'));
		else num = 0;						// zero
	}
	else {									// decimal
		ch = firstCharacter;
		do {
			num = 10 * num + (int)(ch - '0');
			ch = fgetc(sourceFile);
		} while (isdigit(ch));

		// double literal
		if (ch == '.') {
			token.number = tDouble;

			ch = fgetc(sourceFile);
			for (int exp = 1; isdigit(ch); ch = fgetc(sourceFile), exp++) {
				double d = ch - '0';
				for (int i = 0; i < exp; i++)
					d /= 10;
				num += d;
			}

			// floating point representation
			if (ch == 'e') {
				ch = fgetc(sourceFile);
				int exp = 0;
				if (ch == '+') {
					ch = fgetc(sourceFile);
					while(isdigit(ch)) {
						exp = exp * 10 + (ch - '0');
						ch = fgetc(sourceFile);
					}
					// power 10
					for (int i = 0; i < exp; i++)
						num *= 10;
				}
				else if (ch == '-') {
					ch = fgetc(sourceFile);
					while(isdigit(ch)) {
						exp = exp * 10 + (ch -'0');
						ch = fgetc(sourceFile);
					}
					// power 0.1
					for (int i = 0; i < exp; i++)
						num /= 10;
				}
				else if (isdigit(ch)) {
					while(isdigit(ch)) {
						exp = exp * 10 + (ch - '0');
						ch = fgetc(sourceFile);
					}
					// power 10
					for (int i = 0; i < exp; i++)
						num *= 10;
				}
				else {
					lexicalError(7);
					token.number = tnull;
				}
			}
		}
	}
	ungetc(ch, sourceFile);  /*  retract  */
	return num;
}

int hexValue(char ch)
{
	switch (ch) {
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		return (ch - '0');
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		return (ch - 'A' + 10);
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		return (ch - 'a' + 10);
	default: return -1;
	}
}

void printToken(struct tokenType token)
{
	//printf("file name: %s, line number: %d, column number: %d\n", token.fileName, token.lineNumber, token.columnNumber);

	if (token.number == tIdent)
		printf("number: %d, value: %s\n", token.number, token.value.id);
	else if (token.number == tInteger)
		printf("number: %d, value: %d\n", token.number, token.value.num);
	else if (token.number == tCharacter)
		printf("number: %d, value: %c\n", token.number, token.value.c);
	else if (token.number == tDouble)
		printf("number: %d, value: %lf\n", token.number, token.value.num);
	else if (token.number == tString)
		printf("number: %d, value: %s\n", token.number, token.value.s);
	else
		printf("number: %d(%s)\n", token.number, tokenName[token.number]);

}