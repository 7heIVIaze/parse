#define _CRT_SECURE_NO_WARNINGS
/*
	Compiler Assignment
	Cminus Parser in C
	made by Sunggon Kim
*/
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cctype>

int indentno = 0;

#define NUM_KEYWORDS 6 // 6 Reserved Word (else, if, int, return, void, while)
#define ID_LENGTH 40 // Length of Identifier
#define MAX 1024 // Max Size of Buffer
#define NUM_SYMBOLS 16 // 16 Symbol ( + - * / < <= > >= == != = ; , ( ) [ ] { }
#define MAXCHILDNO 3 // Child number 3
#define INDENT indentno+=2
#define UNINDENT indentno-=2

int line = 1;
FILE* source;
FILE* result;
enum tokenType {
	cnull = -1, cplus, cminus, cmultiple, cmodify, cless, clesse, cmore, cmoree, cequal, cnotequal, cassign, csemicolon, ccomma, clparenthese,
	// In order +, -, *, /, <, <=, >, >=, ==, !=, =, ;, ,, (
	// In order 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
	crparenthese, clbracket, crbracket, clbrace, crbrace, cnumber, cidentifier, celse, cif, cint, creturn, cvoid, cwhile, ceof, cerror, cloop
	// In order ), [, ], {, }, 숫자, 식별자, else, if, int, return, void, while, EOF, 에러상황, 주석이 안 끝난 상황
	// In order 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29
}; // Token Type Table
enum NodeKind {
	Statement, Expression
};
enum StmtKind {
	CompoundStmt, SelectStmt, IterateStmt, ReturnStmt, Call
};
enum ExpKind {
	VarDeclare, VarArrayDeclare, FunDeclare, Assign, Operate, Const, Id
};
enum ExpType {
	Void, Integer
};
struct Token {
	int type; // Token Type
	char error; // Error Token
	char sym[3]; // Symbol Token
	union {
		int number; // Number Token
		char id[ID_LENGTH]; // Identifier & Reserved Word Token
	} value;
};
struct TokenTree {
	struct TokenTree* child[MAXCHILDNO];
	struct TokenTree* sibling;
	int lineno;
	int nodekind; // NodeKind variable (Statement/Expression)
	union {
		int stmt; // StmtKind variable (CompoundStmt/SelectStmt/IterateStmt/ReturnStmt/Call)
		int exp; // ExpKind variable (VarDeclare/VarArrayDeclare/FunDeclare/Assign/Operate/Const/Id)
	} kind;
	char type[5]; // Variable Type (int/void)
	union {
		int number;
		char sym[3];
		char id[ID_LENGTH];
	} attr;
	int isParam;
	int arrsize;
};
const char* keyword[NUM_KEYWORDS] = {
	"else", "if", "int", "return", "void", "while"
};
enum tokenType keynum[NUM_KEYWORDS] = {
	celse, cif, cint, creturn, cvoid, cwhile
}; // Reserved Word Type Table
char symbolcase[NUM_SYMBOLS] = {
	'!', '(', ')', '*', '+', ',', '-', '/', ';', '<',
	'=', '>', '[', ']', '{', '}'
}; // Symbol Table
enum tokenType symnum[NUM_SYMBOLS] = {
	cnotequal, clparenthese, crparenthese, cmultiple, cplus, ccomma, cminus, cmodify, csemicolon, cless,
	cassign, cmore, clbracket, crbracket, clbrace, crbrace
}; // Symbol Type Table
Token nextToken;
void printToken(Token token);
void LexicalError(int option);
char* GetID(char firstcharacter);
int GetNumber(char firstcharacter);
int GetKeyword(char* id);
int getSymbol(char sym);
void SyntaxError(const char* message);
void match(int expected);
void printTree(TokenTree *tree);
Token getToken(void);
int type_specifier(void);
TokenTree* newStmtNode(int kind);
TokenTree* newExpNode(int kind);
TokenTree* program(void);
TokenTree* declaration_list(void);
TokenTree* declaration(void);
TokenTree* var_declaration(void);
TokenTree* params(void);
TokenTree* param_list(int type);
TokenTree* param(int type);
TokenTree* compound_stmt(void);
TokenTree* local_declarations(void);
TokenTree* statement_list(void);
TokenTree* statement(void);
TokenTree* expression_stmt(void);
TokenTree* selection_stmt(void);
TokenTree* iteration_stmt(void);
TokenTree* return_stmt(void);
TokenTree* expression(void);
TokenTree* simple_expression(TokenTree* p);
TokenTree* additive_expression(TokenTree* p);
TokenTree* term(TokenTree* p);
TokenTree* factor(TokenTree* p);
TokenTree* call(void);
TokenTree* args(void);
TokenTree* arg_list(void);

TokenTree* newStmtNode(int kind) {
	TokenTree* t = (TokenTree*)malloc(sizeof(TokenTree));
	int i;
	if (t == NULL)
		fprintf(result, "Out of Memory Error at line %d\n", line);
	else {
		for (i = 0; i < MAXCHILDNO; i++)
			t->child[i] = NULL;
		t->sibling = NULL;
		t->nodekind = Statement;
		t->kind.stmt = kind;
		t->lineno = line;
	}
	return t;
}
TokenTree* newExpNode(int kind) {
	TokenTree* t = (TokenTree*)malloc(sizeof(TokenTree));
	int i;
	if (t == NULL)
		fprintf(result, "Out of Memory Error at line %d\n", line);
	else {
		for (i = 0; i < MAXCHILDNO; i++)
			t->child[i] = NULL;
		t->sibling = NULL;
		t->nodekind = Expression;
		t->kind.exp = kind;
		t->lineno = line;
		t->arrsize;
		t->isParam; // To know is tree a parameter
	}
	return t;
}

void printToken(Token token) {
		if (token.type < 19) // If Token is a Symbol
			fprintf(result, "\t%d: Symbol: %s\n", line, token.sym);
		else if (token.type == 19) // If Token is Number
			fprintf(result, "\t%d: Number, Value = %d\n", line, token.value.number);
		else if (token.type == 20) // If Token is a Identifier
			fprintf(result, "\t%d: ID, Name = %s\n", line, token.value.id);
		else if (token.type > 20 && token.type < 27) // If Token is a Reserved Word
			fprintf(result, "\t%d: Reserved word: %s\n", line, token.value.id);
		else if (token.type == 27) // At the End of the File
			fprintf(result, "\n\t%d: EOF\n", line);
		else if (token.type == 29) // If can't get out of Loop
			fprintf(result, "\n\t%d: Stop before Ending the Comment\n", line);
		else // Error
			fprintf(result, "\t%d: ERROR: %c\n", line, token.error);
}
void LexicalError(int option)
{
	fprintf(stderr, "==== %d line The Lexical Error ====\n", line);
	switch (option)
	{
	case 1: fprintf(stderr, "an identifier length must be less than 40\n"); break;
	case 2: fprintf(stderr, "! must need =\n"); break;
	case 3: fprintf(stderr, "Invalid Character\n"); break;
	case 4: fprintf(stderr, "Stop before ending\n"); break;
	}
} // Content to display when an Error occurs
char* GetID(char firstcharacter)
{
	char id[ID_LENGTH];
	char ch = firstcharacter;
	int index = 0;
	do
	{
		if (index < ID_LENGTH)
			id[index++] = ch;
		ch = fgetc(source);
	} while (isalpha(ch));
	id[index] = '\0';
	ungetc(ch, source);

	return id;
} // Get Identifer Token
int GetNumber(char firstcharacter)
{
	char ch = firstcharacter;
	int number = 0;

	if (ch != '0') {// if the firstcharacter is not 0
		do
		{
			number = number * 10 + (int)(ch - '0');
			ch = fgetc(source);
		} while (isdigit(ch));
		ungetc(ch, source);
	}
	else
		number = 0;
	return number;
} // Get Number Token
int GetKeyword(char* id)
{
	int index = 0;
	int start = 0, end = NUM_KEYWORDS - 1;

	while (start <= end) {
		index = (start + end) / 2;

		if (strcmp(id, keyword[index]) == 0)
			return index;
		else if (strcmp(id, keyword[index]) > 0)
			start = index + 1;
		else if (strcmp(id, keyword[index]) < 0)
			end = index - 1;
	}

	return -1;
} // The Binary Search for searching Reserved Word
int getSymbol(char sym)
{
	int index = 0;
	int start = 0, end = NUM_SYMBOLS - 1;

	do
	{
		index = (start + end) / 2;
		if (sym == symbolcase[index])
			return index;
		else if (sym > symbolcase[index])
			start = index + 1;
		else
			end = index - 1;
	} while (start <= end);

	return NUM_SYMBOLS;
} // The Binary Search for searching Symbol
Token getToken(void)
{
	char ch;
	char id[ID_LENGTH] = "";
	int index = 0;
	struct Token token;

	token.type = (cnull);

	do {
		ch = fgetc(source); // Get the first character
		while (isspace(ch)) {
			if (ch == '\n') // If \n, increments the line number
				line++;
			ch = fgetc(source);
			if (feof(source)) { // At the end of the file, then set type EOF
				token.type = ceof;
				return token;
			}
		} // If the character is White Space

		if (isalpha(ch)) {
			strcpy(id, GetID(ch));
			index = GetKeyword(id);
			if (index == -1) {
				token.type = cidentifier;
				strcpy(token.value.id, id);
			} // If not in Reserved Word table, set to Identifier
			else {
				token.type = keynum[index];
				strcpy(token.value.id, keyword[index]);
				break;
			} // If in Reserved Word table, set to Reserved Word
		} // If the character is Letter

		else if (isdigit(ch)) {
			token.type = cnumber;
			token.value.number = GetNumber(ch);
		} // If the character is Number

		else {
			index = getSymbol(ch);

			switch (index)
			{
			case 0: // If the character is !
				ch = fgetc(source);
				if (ch == '=') {
					token.type = symnum[index];
					strcpy(token.sym, "!=");
				}
				else {
					token.type = cerror;
					token.error = '!';
					ungetc(ch, source);
				}
				break;
			case 7: // If the ;character is /
				ch = fgetc(source);
				if (ch == '*') { // Start of comment
					do
					{
						if (feof(source)) { // At the end of file
							LexicalError(4);
							token.type = cloop;
							return token;
						}
						ch = fgetc(source);
						while (ch != '*') {
							ch = fgetc(source);
							if (feof(source)) { // At the end of file
								LexicalError(4);
								token.type = cloop;
								return token;
							}
							if (ch == '\n')
								line++;
						}
						ch = fgetc(source);
					} while (ch != '/'); // Escape the comment
				}
				else { // If division
					token.type = cmodify;
					strcpy(token.sym, "/");
					ungetc(ch, source);
				}
				break;
			case 9: // If the character is <
				ch = fgetc(source);
				if (ch == '=') {
					token.type = clesse;
					strcpy(token.sym, "<=");
				}
				else {
					token.type = cless;
					strcpy(token.sym, "<");
					ungetc(ch, source);
				}
				break;
			case 10: // If the character is =
				ch = fgetc(source);
				if (ch == '=') { 
					token.type = cequal;
					strcpy(token.sym, "==");
				}
				else {  
					token.type = cassign;
					strcpy(token.sym, "=");
					ungetc(ch, source);
				}
				break;
			case 11: // If the character is >
				ch = fgetc(source);
				if (ch == '=') {
					token.type = cmoree;
					strcpy(token.sym, ">=");
				}
				else {
					token.type = cmore;
					strcpy(token.sym, ">");
					ungetc(ch, source);
				}
				break;
			case NUM_SYMBOLS: // If not in Symbol Table
				printf("Current Character: %c\n", ch);
				token.type = cerror;
				token.error = ch;
				LexicalError(3);
				break;
			case EOF:
				token.type = ceof;
				break;
			default:
				token.type = symnum[index];
				token.sym[0] = symbolcase[index];
				token.sym[1] = '\0';
				break;
			}
		}

	} while (token.type == cnull);
	return token;
};
// Scanner End

// Start Recursive-Descent Parsing
/*
	*** Grammar ***
	<program> ::= <declaration-list>
	<declaration-list> ::= <declaration> { <declaration> }
	<declaration> ::= <var-declaration> | <fun-declaration>
	<var-declaration> ::= <type-specifier> 'ID' [ '[' 'NUM' ']' ] ';'
	<type-specifier> ::= 'int' | 'void'
	<fun-declaration> ::= <type-specifier> 'ID' '(' <params> ')' <compound-stmt>
	<params> ::= <param-list> | 'void'
	<param-list> ::= <param> { ',' <param> }
	<param> ::= <type-specifier> 'ID' [ '[' ']' ]
	<compound-stmt> ::= '{' <local-declarations> <statement-list> '}'
	<local-declarations> ::= { <var-declaration> }
	<statement-list> ::= { <statement> }
	<statement> ::= <expression-stmt> | <compound-stmt> | <selection-stmt> | <iteration-stmt> | <return-stmt>
	<expression-stmt> ::= [ <expression> ] ';'
	<selection-stmt> ::= 'if' '(' <expression> ')' <statement> [ 'else' <statement> ]
	<iteration-stmt> ::= 'while' '(' <expression> ')' <statement>
	<return-stmt> ::= 'return' [ <expression> ] ';'
	<expression> ::= <var> '=' <expression> | <simple-expression>
	<var> ::= 'ID' [ '[' <expression> ']' ]
	<simple-expression> ::= <additive-expression> [ <relop> <additive-expression> ]
	<relop> ::= '<=' | '<' | '>' | '>=' | '==' | '!='
	<additive-expression> ::= <term> { <addop> <term> }
	<addop> ::= '+' | '-'
	<term> ::= <factor> { <mulop> <factor> }
	<mulop> ::= '*' | '/'
	<factor> ::= '(' <expression> ')' | <var> | <call> | 'NUM'
	<call> ::= 'ID' '(' <args> ')'
	<args> ::= [ <arg-list> ]
	<arg-list> ::= <expression> { ',' <expression> }
*/

void SyntaxError(const char* message) {
	fprintf(result, "\n>>> ");
	fprintf(result, "Syntax Error at line %d: %s", line, message);
}
void match(int expected) {
	if (nextToken.type == expected) 
		nextToken = getToken();
	else {
		SyntaxError("unexpected token match -> ");
		printToken(nextToken);
		fprintf(result, "       ");
	}
}
void printTree(TokenTree *tree) {
	int i = 0;
	INDENT;
	while (tree != NULL) {
		for (i = 0; i < indentno; i++) {
			fprintf(result, " ");
		}
		if (tree->nodekind == Statement) {
			switch (tree->kind.stmt) {
			case CompoundStmt:
				fprintf(result, "Compound Statement\n");
				break;
			case SelectStmt:
				if (tree->child[2] != NULL)
					fprintf(result, "If (condition) (body) (else)\n");
				else
					fprintf(result, "If (condition) (body)\n");
				break;
			case IterateStmt:
				fprintf(result, "While (condition) (body)\n");
				break;
			case ReturnStmt:
				if (tree->child[0] == NULL)
					fprintf(result, "Return Statement with nothing\n");
				else
					fprintf(result, "Return Statement with \n");
				break;
			case Call:
				if (tree->child[0] == NULL)
					fprintf(result, "Call, name: %s\n", tree->attr.id);
				else
					fprintf(result, "Call, name: %s, with arguments\n", tree->attr.id);
				break;
			default:
				fprintf(result, "Unknown Expression Node Kind\n");
				break;
			}
		}
		else if (tree->nodekind == Expression) {
			switch (tree->kind.exp) {
			case VarDeclare:
				if (tree->isParam == 1)
					fprintf(result, "Single Parameter, name: %s, type: %s\n", tree->attr.id, tree->type);
				else
					fprintf(result, "Variable Declaration, name: %s, type: %s\n", tree->attr.id, tree->type);
				break;
			case VarArrayDeclare:
				if (tree->isParam == 1)
					fprintf(result, "Array Parameter, name: %s, type: %s\n", tree->attr.id, tree->type);
				else
					fprintf(result, "Array Variable Declaration, name: %s, type: %s, size: %d\n", tree->attr.id, tree->type, tree->arrsize);
				break;
			case FunDeclare:
				fprintf(result, "Function Declaration, name: %s, type: %s\n", tree->attr.id, tree->type);
				break;
			case Assign:
				fprintf(result, "Assign: (To) (From)\n");
				break;
			case Operate:
				fprintf(result, "Op: %s\n", tree->attr.sym);
				break;
			case Const:
				fprintf(result, "Const: %d\n", tree->attr.number);
				break;
			case Id:
				fprintf(result, "ID: %s\n", tree->attr.id);
				break;
			default:
				fprintf(result, "Unknown Expression Node Kind\n");
				break;
			}
		}
		else
			fprintf(result, "Unknown Node Kind\n");
		for (i = 0; i < MAXCHILDNO; i++)
			printTree(tree->child[i]);
		tree = tree->sibling;
	}
	UNINDENT;
}
TokenTree* program(void) { // <program> ::= <declaration-list>
	TokenTree* t;
	nextToken = getToken();
	t = declaration_list();
	if (nextToken.type != ceof)
		SyntaxError("Code ends before file!\n");
	return t;
}
TokenTree* declaration_list(void) { // <declaration-list> ::= <declaration> { <declaration> }
	TokenTree* t = declaration();
	TokenTree* p = t;
	while (nextToken.type != ceof) {
		TokenTree* q;
		q = declaration();
		if (q != NULL) {
			if (t == NULL) {
				p = q;
				t = p;
			}
			else {
				p->sibling = q;
				p = q;
			}
		}
	}
	return t;
}
TokenTree* declaration(void) { 
	/*
		<declaration> ::= <var-declaration> | <fun-declaration>
		<var-declaration> ::= <type-specifier> 'ID' [ '[' 'NUM' ']' ] ';'
		<fun-declaration> ::= <type-specifier> 'ID' '(' <params> ')' <compound-stmt>
	*/
	TokenTree* t = NULL;
	Token a;
	int type;

	type = type_specifier();
	a = nextToken;
	match(cidentifier);
	switch (nextToken.type) {
	case csemicolon:
		t = newExpNode(VarDeclare);
		if (t != NULL) {
			strcpy(t->attr.id, a.value.id);
			if (type == Void)
				strcpy(t->type, "void");
			else
				strcpy(t->type, "int");
		}
		match(csemicolon);
		break;
	case clbracket:
		t = newExpNode(VarArrayDeclare);
		if (t != NULL) {
			strcpy(t->attr.id, a.value.id);
			if (type == Void)
				strcpy(t->type, "void");
			else
				strcpy(t->type, "int");
		}
		match(clbracket);
		if (t != NULL)
			t->arrsize = nextToken.value.number;
		match(cnumber);
		match(crbracket);
		match(csemicolon);
		break;
	// var-declaration
	case clparenthese:
		t = newExpNode(FunDeclare);
		if (t != NULL) {
			strcpy(t->attr.id, a.value.id);
			if (type == Void)
				strcpy(t->type, "void");
			else
				strcpy(t->type, "int");
		}
		match(clparenthese);
		if (t != NULL)
			t->child[0] = params();
		match(crparenthese);
		if (t != NULL)
			t->child[1] = compound_stmt();
		break;
	default:
		SyntaxError("Unexpected token at declaration rule -> ");
		printToken(nextToken);
		nextToken = getToken();
		break;
	// fun-declaration
	}
	return t;
}
TokenTree* var_declaration(void) { // <var-declaration> ::= <type-specifier> 'ID' [ '[' 'NUM' ']' ] ';'
	TokenTree* t = NULL;
	int type;
	Token a;

	type = type_specifier();
	a = nextToken;
	match(cidentifier);
	switch (nextToken.type) {
	case csemicolon:
		t = newExpNode(VarDeclare);
		if (t != NULL) {
			strcpy(t->attr.id, a.value.id);
			if (type == Void)
				strcpy(t->type, "void");
			else
				strcpy(t->type, "int");
		}
		match(csemicolon);
		break;
	case clbracket:
		t = newExpNode(VarArrayDeclare);
		if (t != NULL) {
			strcpy(t->attr.id, a.value.id);
			if (type == Void)
				strcpy(t->type, "void");
			else
				strcpy(t->type, "int");
		}
		match(clbracket);
		if (t != NULL)
			t->arrsize = nextToken.value.number;
		match(cnumber);
		match(crbracket);
		match(csemicolon);
		break;
	}
	return t;
}
int type_specifier(void) {
	switch (nextToken.type) {
	case cint:
		nextToken = getToken();
		return Integer;
	case cvoid:
		nextToken = getToken();
		return Void;
	default:
		SyntaxError("Unexpected token at type-specifier rule -> ");
		printToken(nextToken);
		nextToken = getToken();
		return Void;
	}
}
TokenTree* params(void) { // <params> ::= <param-list> | 'void'
	int type;
	TokenTree* t = NULL;

	type = type_specifier();
	if (type == Void && nextToken.type == crparenthese) {
		t = newExpNode(VarDeclare);
		t->isParam = 1;
		strcpy(t->attr.id, "(null)");
		if (type == Void)
			strcpy(t->type, "void");
		else
			strcpy(t->type, "int");
	}
	else
		t = param_list(type);
	return t;
}
TokenTree* param_list(int type) { // <param-list> ::= <param> { ',' <param> }
	TokenTree* t = param(type);
	TokenTree* p = t;
	TokenTree* q;
	while (nextToken.type == ccomma) {
		match(ccomma);
		q = param(type_specifier());
		if (q != NULL) {
			if (t == NULL) {
				p = q;
				t = p;
			}
			else {
				p->sibling = q;
				p = q;
			}
		}
	}
	return t;
}
TokenTree* param(int type) { // <param> ::= <type-specifier> 'ID' [ '[' ']' ]
	TokenTree* t;
	Token a = nextToken;

	match(cidentifier);
	if (nextToken.type == clbracket) {
		match(clbracket);
		match(crbracket);
		t = newExpNode(VarArrayDeclare);
	}
	else
		t = newExpNode(VarDeclare);
	if (t != NULL) {
		strcpy(t->attr.id, a.value.id);
		if (type == Void)
				strcpy(t->type, "void");
			else
				strcpy(t->type, "int");
		t->isParam = 1;
	}
	return t;
}
TokenTree* compound_stmt(void) { // <compound-stmt> ::= '{' <local-declarations> <statement-list> '}'
	TokenTree* t = newStmtNode(CompoundStmt);
	match(clbrace);
	t->child[0] = local_declarations();
	t->child[1] = statement_list();
	match(crbrace);
	return t;
}
TokenTree* local_declarations(void) { // <local-declarations> ::= { <var-declaration> }
	TokenTree* t = NULL;
	TokenTree* p;

	if (nextToken.type == cint || nextToken.type == cvoid)
		t = var_declaration();
	p = t;
	if (t != NULL) {
		while (nextToken.type == cint || nextToken.type == cvoid) {
			TokenTree* q;
			q = var_declaration();
			if (q != NULL) {
				if (t == NULL) {
					p = q;
					t = p;
				}
				else {
					p->sibling = q;
					p = q;
				}
			}
		}
	}
	return t;
}
TokenTree* statement_list(void) { // <statement-list> ::= { <statement> }
	TokenTree* t;
	TokenTree* p;

	if (nextToken.type == crbrace)
		return NULL;
	t = statement();
	p = t;
	while (nextToken.type != crbrace) {
		TokenTree* q;
		q = statement();
		if (q != NULL) {
			if (t == NULL) {
				p = q;
				t = p;
			}
			else {
				p->sibling = q;
				p = q;
			}
		}
	}
	return t;
}
TokenTree* statement(void) { // <statement> ::= <expression-stmt> | <compound-stmt> | <selection-stmt> | <iteration-stmt> | <return-stmt>
	TokenTree* t = NULL;
	switch (nextToken.type) {
	case clbrace:
		t = compound_stmt();
		break;
	case cif:
		t = selection_stmt();
		break;
	case cwhile:
		t = iteration_stmt();
		break;
	case creturn:
		t = return_stmt();
		break;
	case cidentifier:
		t = expression_stmt();
		break;
	case clparenthese:
		t = expression_stmt();
		break;
	case cnumber:
		t = expression_stmt();
		break;
	case csemicolon:
		t = expression_stmt();
		break;
	default:
		SyntaxError("Unexpected token at statement rule -> ");
		printToken(nextToken);
		nextToken = getToken();
		break;
	}
	return t;
}
TokenTree* expression_stmt(void) { // <exression-stmt> ::= [ <expression> ] ';'
	TokenTree* t = NULL;

	if (nextToken.type == csemicolon)
		match(csemicolon);
	else if (nextToken.type != crbrace) {
		t = expression();
		match(csemicolon);
	}
	return t;
}
TokenTree* selection_stmt(void) { // <selection-stmt> ::= 'if' '(' <expression> ')' <statement> [ 'else' <statement> ]
	TokenTree* t = newStmtNode(SelectStmt);

	match(cif);
	match(clparenthese);
	if (t != NULL)
		t->child[0] = expression();
	match(crparenthese);
	if (t != NULL)
		t->child[1] = statement();
	if (nextToken.type == celse) {
		match(celse);
		if (t != NULL)
			t->child[2] = statement();
	}
	return t;
}
TokenTree* iteration_stmt(void) { // <iteration-stmt> ::= 'while' '(' <expression> ')' <statement>
	TokenTree* t = newStmtNode(IterateStmt);

	match(cwhile);
	match(clparenthese);
	if (t != NULL)
		t->child[0] = expression();
	match(crparenthese);
	if (t != NULL)
		t->child[1] = statement();
	return t;
}
TokenTree* return_stmt(void) { // <return-stmt> ::= 'return' [ <expression> ] ';'
	TokenTree* t = newStmtNode(ReturnStmt);

	match(creturn);
	if (nextToken.type != csemicolon && t != NULL)
		t->child[0] = expression();
	match(csemicolon);
	return t;
}
TokenTree* expression(void) { // <expression> ::= <var> '=' <expression> | <simple-expression>
	TokenTree* t = NULL;
	TokenTree* q = NULL;
	int isCall = 0;

	if (nextToken.type == cidentifier) {
		q = call();
		isCall = 1;
	}

	if (isCall == 1 && nextToken.type == cassign) {
		if (q != NULL && q->nodekind == Expression && q->kind.exp == Id) {
			match(cassign);
			t = newExpNode(Assign);
			if (t != NULL) {
				t->child[0] = q;
				t->child[1] = expression();
			}
		}
		else {
			SyntaxError("Failed to assign to something\n");
			nextToken = getToken();
		}
	}
	else
		t = simple_expression(q);
	return t;
}
TokenTree* simple_expression(TokenTree *p) { // <simple-expression> ::= <additive-expression> [ <relop> <additive-expression> ]
	TokenTree* t, * q;
	Token a;
	int type = nextToken.type;
	q = additive_expression(p);
	if (type == cless || type == clesse || type == cmore || type == cmoree || type == cequal || type == cnotequal) { // <relop> ::= '<=' | '<' | '>' | '>=' | '==' | '!='
		a = nextToken;
		match(type);
		t = newExpNode(Operate);
		if (t != NULL) {
			t->child[0] = q;
			t->child[1] = additive_expression(NULL);
			strcpy(t->attr.sym, a.sym);
		}
	}
	else
		t = q;
	return t;
}
TokenTree* additive_expression(TokenTree* p) { // <additive-expression> ::= <term> { <addop> <term> }
	TokenTree* t, * q;

	t = term(p);
	if (t != NULL) {
		while (nextToken.type == cplus || nextToken.type == cminus) { // <addop> ::= '+' | '-'
			q = newExpNode(Operate);
			if (q != NULL) {
				q->child[0] = t;
				strcpy(q->attr.sym, nextToken.sym);
				t = q;
				match(nextToken.type);
				t->child[1] = term(NULL);
			}
		}
	}
	return t;
}
TokenTree* term(TokenTree* p) { // <term> ::= <factor> { <mulop> <factor> }
	TokenTree* t, * q;

	t = factor(p);
	if (t != NULL) {
		while (nextToken.type == cmultiple || nextToken.type == cmodify) { // <mulop> ::= '*' | '/'
			q = newExpNode(Operate);
			if (q != NULL) {
				q->child[0] = t;
				strcpy(q->attr.sym, nextToken.sym);
				t = q;
				match(nextToken.type);
				t->child[1] = factor(NULL);
			}
		}
	}
	return t;
}
TokenTree* factor(TokenTree* p) { // <factor> ::= '(' <expression> ')' | <var> | <call> | 'NUM'
	TokenTree* t = NULL;

	if (p != NULL)
		return p;
	switch (nextToken.type) {
	case clparenthese:
		match(clparenthese);
		t = expression();
		match(crparenthese);
		break;
	case cidentifier:
		t = call(); // <var> | <call>
		break;
	case cnumber:
		t = newExpNode(Const);
		if (t != NULL) {
			t->attr.number = nextToken.value.number;
			strcpy(t->type, "int");
		}
		match(cnumber);
		break;
	default:
		SyntaxError("Unexpected token at factor rule -> ");
		printToken(nextToken);
		nextToken = getToken();
		break;
	}
	return t;
}
TokenTree* call(void) { // <call> ::= 'ID' '(' <args> ')' | <var> ::= 'ID' [ '[' <expression> ']' ]
	TokenTree* t;
	Token a;

	if (nextToken.type == cidentifier)
		a = nextToken;
	match(cidentifier);

	if (nextToken.type == clparenthese) {
		match(clparenthese);
		t = newStmtNode(Call);
		if (t != NULL) {
			strcpy(t->attr.id, a.value.id);
			t->child[0] = args();
		}
		match(crparenthese);
	}
	// <call> ::= 'ID' '(' <args> ')'
	else if (nextToken.type == clbracket) {
		match(clbracket);
		t = newExpNode(Id);
		if (t != NULL) {
			strcpy(t->attr.id, a.value.id);
			strcpy(t->type, "int");
			t->child[0] = expression();
			match(crbracket);
		}
	}
	else {
		t = newExpNode(Id);
		if (t != NULL) {
			strcpy(t->attr.id, a.value.id);
			strcpy(t->type, "int");
		}
	}
	// <var> ::= 'ID' [ '[' <expression> ']' ]
	return t;
}
TokenTree* args(void) { // <args> ::= [ <arg-list> ]
	if (nextToken.type == crparenthese)
		return NULL;
	else 
		return arg_list();
}
TokenTree* arg_list(void) { // <arg-list> ::= <expression> { ',' <expression> }
	TokenTree* t, * p;

	t = expression();
	p = t;
	if (t != NULL) {
		while (nextToken.type == ccomma) {
			match(ccomma);
			TokenTree* q = expression();
			if (q != NULL) {
				if (t == NULL) {
					p = q;
					t = p;
				}
				else {
					p->sibling = q;
					p = q;
				}
			}
		}
	}
	return t;
}

int main(int argc, char* argv[])
{
	int copyline = 1;
	TokenTree *tree;
	if (argc != 3) {
		fprintf(stderr, "Usage: name <sourcefile> <resultfile>\n");
		exit(1);
	}

	source = fopen(argv[1], "r");
	result = fopen(argv[2], "w");

	tree = program();
	fprintf(result, "\nSyntax tree: \n");
	printTree(tree);

	fclose(result);
	fclose(source);

	return 0;
}
