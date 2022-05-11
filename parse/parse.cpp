/*
	C-Minus Scanner and Parser
*/
//#define _CRT_SECURE_NO_WANRINGS
//#include <cstdio>
//#include <cstdlib>
//#include <cctype>
//#include <cstring>
//
//#define NUM_KEYWORDS 6
//#define ID_LENGTH 40
//
//FILE* source;
//FILE* output;
//int lineno;
//enum tokenType {
//	cnull = -1, cplus, cminus, cmultiple, cmodify, cless, clesse, cmore, cmoree, cequal, cnotequal, cassign, csemicolon, ccomma, clparenthese,
//	// 순서대로 +, -, *, /, <, <=, >, >=, ==, !=, =, ;, ,, (
//	// 순서대로 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
//	crparenthese, clbracket, crbracket, clbrace, crbrace, cnumber, cidentifier, celse, cif, cint, creturn, cvoid, cwhile, ceof, cerror, cloop
//	// 순서대로 ), [, ], {, }, 숫자, 식별자, else, if, int, return, void, while, EOF, 에러상황, 주석이 안 끝난 상황
//	// 순서대로 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29
//}; // 토큰 타입
//struct Token {
//	int lineno;
//	int type;
//	char error;
//	char sym[3];
//	union {
//		int number;
//		char id[ID_LENGTH];
//	} value;
//};
//struct treeNode {
//	struct treeNode* child[3];
//	struct treeNode* sibling;
//};
//const char* keyword[NUM_KEYWORDS]{
//	"else", "if", "int", "return", "void", "while"
//};
//tokenType keynum[NUM_KEYWORDS]{
//	celse, cif, cint, creturn, cvoid, cwhile
//};


/*
	Compiler Assignment
	Cminus Parser in C
*/
#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cctype>

#define NUM_KEYWORDS 6 // 예약어의 종류는 6개이므로
#define ID_LENGTH 40 // ID의 길이
#define MAX 1024 // 문자열 담을 변수 길이
#define NUM_SYMBOLS 16 // 심볼 갯수 16개

int line = 1;
FILE* source;
FILE* copy;
FILE* result;
enum tokenType {
	cnull = -1, cplus, cminus, cmultiple, cmodify, cless, clesse, cmore, cmoree, cequal, cnotequal, cassign, csemicolon, ccomma, clparenthese,
	// 순서대로 +, -, *, /, <, <=, >, >=, ==, !=, =, ;, ,, (
	// 순서대로 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
	crparenthese, clbracket, crbracket, clbrace, crbrace, cnumber, cidentifier, celse, cif, cint, creturn, cvoid, cwhile, ceof, cerror, cloop
	// 순서대로 ), [, ], {, }, 숫자, 식별자, else, if, int, return, void, while, EOF, 에러상황, 주석이 안 끝난 상황
	// 순서대로 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29
}; // 심볼 타입 테이블
struct Token {
	struct Token* child[3];
	struct Token* sibling;
	int type; // 어떤 타입인지
	char error; // 에러 상황일 경우 담을 변수
	char sym[3]; // 심볼을 담을 변수. 3인 이유는 <=같은 경우
	union {
		int number; //숫자
		char id[ID_LENGTH]; // 식별자
	} value;
};
const char* keyword[NUM_KEYWORDS] = {
	"else", "if", "int", "return", "void", "while"
}; // 키워드 테이블
enum tokenType keynum[NUM_KEYWORDS] = {
	celse, cif, cint, creturn, cvoid, cwhile
}; // 키워드 타입 테이블
char symbolcase[NUM_SYMBOLS] = {
	'!', '(', ')', '*', '+', ',', '-', '/', ';', '<',
	'=', '>', '[', ']', '{', '}'
}; // 심볼 테이블
enum tokenType symnum[NUM_SYMBOLS] = {
	cnotequal, clparenthese, crparenthese, cmultiple, cplus, ccomma, cminus, cmodify, csemicolon, cless,
	cassign, cmore, clbracket, crbracket, clbrace, crbrace
}; // 심볼 타입 테이블
tokenType TYPE[2] = {
	cint, cvoid
};
// 변수
struct Token tokengotten;

Token* newStmtNode() {

}

void printToken(Token a) {
		if (a.type < 19) // 심볼로 밝혀졌을 경우
			fprintf(result, "\t%d: Symbol: %s\n", line, a.sym);
		else if (a.type == 19) // number로 밝혀졌을 경우
			fprintf(result, "\t%d: Number, Value = %d\n", line, a.value.number);
		else if (a.type == 20) // identifier로 밝혀졌을 경우
			fprintf(result, "\t%d: ID, Name = %s\n", line, a.value.id);
		else if (a.type > 20 && a.type < 27) // 키워드로 밝혀졌을 경우
			fprintf(result, "\t%d: Reserved word: %s\n", line, a.value.id);
		else if (a.type == 27) // EOF일 경우
			fprintf(result, "\n\t%d: EOF\n", line);
		else if (a.type == 29) // loop에서 못 나왔을 경우
			fprintf(result, "\n\t%d: Stop before Ending the Comment\n", line);
		else
			fprintf(result, "\t%d: ERROR: %c\n", line, a.error);
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
} // 에러가 발생했을 경우 에러창에 띄울 문구들
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
	ungetc(ch, stdin);
	fseek(source, -1, SEEK_CUR);

	return id;
} // 식별자를 구하는 함수
int GetNumber(char firstcharacter)
{
	char ch = firstcharacter;
	int number = 0;

	if (ch != '0') {// 받은 첫 글자가 0이 아닐 경우
		do
		{
			number = number * 10 + (int)(ch - '0');
			ch = fgetc(source);
		} while (isdigit(ch));
		ungetc(ch, stdin);
		fseek(source, -1, SEEK_CUR);
	}
	else
		number = 0;
	return number;
} // 숫자를 구하는 함수
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
} // 키워드 테이블에서 찾기 위한 바이너리 서치
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
} // 심볼 테이블에서 찾기 위한 바이너리 서치
struct Token getToken()
{
	char ch;
	char id[ID_LENGTH] = "";
	int index = 0;
	struct Token token;
	token.type = (cnull);

	do {
		ch = fgetc(source); // 일단 한 글자 받아놓음
		while (isspace(ch)) {
			if (ch == '\n') // 엔터일 경우 라인 숫자를 올림
				line++;
			ch = fgetc(source);
			if (feof(source)) { // 만약 파일 끝까지 읽었을 경우 eof로 판단
				token.type = ceof;
				return token;
			}
		} // 읽은 문자가 공백일 경우

		if (isalpha(ch)) {
			strcpy(id, GetID(ch));
			index = GetKeyword(id); // 키워드인지 바이너리 서치로 찾음
			if (index == -1) {
				token.type = cidentifier;
				strcpy(token.value.id, id);
			} // 없을 경우 식별자로
			else {
				token.type = keynum[index];
				strcpy(token.value.id, keyword[index]);
				break;
			} // 테이블에 있을 경우 키워드로
		} // 읽은 문자가 문자였을 경우

		else if (isdigit(ch)) {
			token.type = cnumber;
			token.value.number = GetNumber(ch);
		} // 읽은 문자가 숫자였을 경우

		else {
			index = getSymbol(ch);

			switch (index)
			{
			case 0: // !인 경우
				ch = fgetc(source);
				if (ch == '=') {
					token.type = symnum[index];
					strcpy(token.sym, "!=");
				}
				else {
					token.type = cerror;
					token.error = '!';
					fseek(source, -1, SEEK_CUR);
					ungetc(ch, stdin);
				}
				break;
			case 7: // /인 경우
				ch = fgetc(source);
				if (ch == '*') { // 주석의 시작
					do
					{
						if (feof(source)) { // 만약 파일을 끝까지 읽었다면
							LexicalError(4);
							token.type = cloop;
							return token;
						}
						ch = fgetc(source);
						while (ch != '*') {
							ch = fgetc(source);
							if (feof(source)) { // 파일을 끝까지 읽었다면
								LexicalError(4);
								token.type = cloop;
								return token;
							}
							if (ch == '\n')
								line++;
						}
						ch = fgetc(source);
					} while (ch != '/'); // 주석 탈출
				}
				else { // 나누기일 경우
					fseek(source, -1, SEEK_CUR);
					token.type = cmodify;
					strcpy(token.sym, "/");
					ungetc(ch, stdin);
				}
				break;
			case 9: // <인 경우
				ch = fgetc(source);
				if (ch == '=') { // <=일 경우
					token.type = clesse;
					strcpy(token.sym, "<=");
				}
				else { // <일 경우
					fseek(source, -1, SEEK_CUR);
					token.type = cless;
					strcpy(token.sym, "<");
					ungetc(ch, stdin);
				}
				break;
			case 10: // =인 경우
				ch = fgetc(source);
				if (ch == '=') { // ==일 경우
					token.type = cequal;
					strcpy(token.sym, "==");
				}
				else {  // =일 경우
					fseek(source, -1, SEEK_CUR);
					token.type = cassign;
					strcpy(token.sym, "=");
					ungetc(ch, stdin);
				}
				break;
			case 11: // >인 경우
				ch = fgetc(source);
				if (ch == '=') { // >=일 경우
					token.type = cmoree;
					strcpy(token.sym, ">=");
				}
				else { // >일 경우
					fseek(source, -1, SEEK_CUR);
					token.type = cmore;
					strcpy(token.sym, ">");
					ungetc(ch, stdin);
				}
				break;
			case NUM_SYMBOLS: // 예약어 테이블에 없는 경우
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
		// 심볼 테이블 사용버전

	} while (token.type == cnull);

	return token;
};
// 스캐너

void SyntaxError(const char* message) {
	fprintf(result, "\n>>> ");
	fprintf(result, "syntax error in line %d: %s", line, message);

}
void match(Token expect) {
	if (tokengotten.type == expect.type)
		tokengotten = getToken();
	else {
		SyntaxError("unexpected token -> ");
		printToken(tokengotten);
		fprintf(result, "       ");
	}
}
Token* declaration_list(void) {
	Token* t = declaration();
	Token* p = t;
}

Token* declaration(void) {
	Token* t = NULL;
	switch (tokengotten.type) {
	case cint:
		match(tokengotten);
		break;
	case cvoid:
		t = var_declaration();
		break;

	}
	
}
Token* var_declaration(void) {

}
Token* fun_declaration(void) {

}

int main(int argc, char* argv[])
{
	int i;
	int copyline = 1;
	char buffer[MAX];
	struct Token a;
	if (argc != 3) {
		fprintf(stderr, "Usage: name <sourcefile> <resultfile>\n");
		exit(1);
	}

	source = fopen(argv[1], "r");
	copy = fopen(argv[1], "r");
	result = fopen(argv[2], "w");

	do
	{

		a = getToken();

		while (copyline <= line) {
			fgets(buffer, sizeof(buffer), copy);
			fprintf(result, "%d: %s", copyline++, buffer);
			buffer[0] = '\0';
		}

		printToken(a);
	} while (!(feof(source)));

	fclose(copy);
	fclose(result);
	fclose(source);

	return 0;
}
