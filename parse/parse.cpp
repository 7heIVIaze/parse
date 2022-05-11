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
//	// ������� +, -, *, /, <, <=, >, >=, ==, !=, =, ;, ,, (
//	// ������� 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
//	crparenthese, clbracket, crbracket, clbrace, crbrace, cnumber, cidentifier, celse, cif, cint, creturn, cvoid, cwhile, ceof, cerror, cloop
//	// ������� ), [, ], {, }, ����, �ĺ���, else, if, int, return, void, while, EOF, ������Ȳ, �ּ��� �� ���� ��Ȳ
//	// ������� 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29
//}; // ��ū Ÿ��
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

#define NUM_KEYWORDS 6 // ������� ������ 6���̹Ƿ�
#define ID_LENGTH 40 // ID�� ����
#define MAX 1024 // ���ڿ� ���� ���� ����
#define NUM_SYMBOLS 16 // �ɺ� ���� 16��

int line = 1;
FILE* source;
FILE* copy;
FILE* result;
enum tokenType {
	cnull = -1, cplus, cminus, cmultiple, cmodify, cless, clesse, cmore, cmoree, cequal, cnotequal, cassign, csemicolon, ccomma, clparenthese,
	// ������� +, -, *, /, <, <=, >, >=, ==, !=, =, ;, ,, (
	// ������� 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
	crparenthese, clbracket, crbracket, clbrace, crbrace, cnumber, cidentifier, celse, cif, cint, creturn, cvoid, cwhile, ceof, cerror, cloop
	// ������� ), [, ], {, }, ����, �ĺ���, else, if, int, return, void, while, EOF, ������Ȳ, �ּ��� �� ���� ��Ȳ
	// ������� 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29
}; // �ɺ� Ÿ�� ���̺�
struct Token {
	struct Token* child[3];
	struct Token* sibling;
	int type; // � Ÿ������
	char error; // ���� ��Ȳ�� ��� ���� ����
	char sym[3]; // �ɺ��� ���� ����. 3�� ������ <=���� ���
	union {
		int number; //����
		char id[ID_LENGTH]; // �ĺ���
	} value;
};
const char* keyword[NUM_KEYWORDS] = {
	"else", "if", "int", "return", "void", "while"
}; // Ű���� ���̺�
enum tokenType keynum[NUM_KEYWORDS] = {
	celse, cif, cint, creturn, cvoid, cwhile
}; // Ű���� Ÿ�� ���̺�
char symbolcase[NUM_SYMBOLS] = {
	'!', '(', ')', '*', '+', ',', '-', '/', ';', '<',
	'=', '>', '[', ']', '{', '}'
}; // �ɺ� ���̺�
enum tokenType symnum[NUM_SYMBOLS] = {
	cnotequal, clparenthese, crparenthese, cmultiple, cplus, ccomma, cminus, cmodify, csemicolon, cless,
	cassign, cmore, clbracket, crbracket, clbrace, crbrace
}; // �ɺ� Ÿ�� ���̺�
tokenType TYPE[2] = {
	cint, cvoid
};
// ����
struct Token tokengotten;

Token* newStmtNode() {

}

void printToken(Token a) {
		if (a.type < 19) // �ɺ��� �������� ���
			fprintf(result, "\t%d: Symbol: %s\n", line, a.sym);
		else if (a.type == 19) // number�� �������� ���
			fprintf(result, "\t%d: Number, Value = %d\n", line, a.value.number);
		else if (a.type == 20) // identifier�� �������� ���
			fprintf(result, "\t%d: ID, Name = %s\n", line, a.value.id);
		else if (a.type > 20 && a.type < 27) // Ű����� �������� ���
			fprintf(result, "\t%d: Reserved word: %s\n", line, a.value.id);
		else if (a.type == 27) // EOF�� ���
			fprintf(result, "\n\t%d: EOF\n", line);
		else if (a.type == 29) // loop���� �� ������ ���
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
} // ������ �߻����� ��� ����â�� ��� ������
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
} // �ĺ��ڸ� ���ϴ� �Լ�
int GetNumber(char firstcharacter)
{
	char ch = firstcharacter;
	int number = 0;

	if (ch != '0') {// ���� ù ���ڰ� 0�� �ƴ� ���
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
} // ���ڸ� ���ϴ� �Լ�
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
} // Ű���� ���̺��� ã�� ���� ���̳ʸ� ��ġ
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
} // �ɺ� ���̺��� ã�� ���� ���̳ʸ� ��ġ
struct Token getToken()
{
	char ch;
	char id[ID_LENGTH] = "";
	int index = 0;
	struct Token token;
	token.type = (cnull);

	do {
		ch = fgetc(source); // �ϴ� �� ���� �޾Ƴ���
		while (isspace(ch)) {
			if (ch == '\n') // ������ ��� ���� ���ڸ� �ø�
				line++;
			ch = fgetc(source);
			if (feof(source)) { // ���� ���� ������ �о��� ��� eof�� �Ǵ�
				token.type = ceof;
				return token;
			}
		} // ���� ���ڰ� ������ ���

		if (isalpha(ch)) {
			strcpy(id, GetID(ch));
			index = GetKeyword(id); // Ű�������� ���̳ʸ� ��ġ�� ã��
			if (index == -1) {
				token.type = cidentifier;
				strcpy(token.value.id, id);
			} // ���� ��� �ĺ��ڷ�
			else {
				token.type = keynum[index];
				strcpy(token.value.id, keyword[index]);
				break;
			} // ���̺� ���� ��� Ű�����
		} // ���� ���ڰ� ���ڿ��� ���

		else if (isdigit(ch)) {
			token.type = cnumber;
			token.value.number = GetNumber(ch);
		} // ���� ���ڰ� ���ڿ��� ���

		else {
			index = getSymbol(ch);

			switch (index)
			{
			case 0: // !�� ���
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
			case 7: // /�� ���
				ch = fgetc(source);
				if (ch == '*') { // �ּ��� ����
					do
					{
						if (feof(source)) { // ���� ������ ������ �о��ٸ�
							LexicalError(4);
							token.type = cloop;
							return token;
						}
						ch = fgetc(source);
						while (ch != '*') {
							ch = fgetc(source);
							if (feof(source)) { // ������ ������ �о��ٸ�
								LexicalError(4);
								token.type = cloop;
								return token;
							}
							if (ch == '\n')
								line++;
						}
						ch = fgetc(source);
					} while (ch != '/'); // �ּ� Ż��
				}
				else { // �������� ���
					fseek(source, -1, SEEK_CUR);
					token.type = cmodify;
					strcpy(token.sym, "/");
					ungetc(ch, stdin);
				}
				break;
			case 9: // <�� ���
				ch = fgetc(source);
				if (ch == '=') { // <=�� ���
					token.type = clesse;
					strcpy(token.sym, "<=");
				}
				else { // <�� ���
					fseek(source, -1, SEEK_CUR);
					token.type = cless;
					strcpy(token.sym, "<");
					ungetc(ch, stdin);
				}
				break;
			case 10: // =�� ���
				ch = fgetc(source);
				if (ch == '=') { // ==�� ���
					token.type = cequal;
					strcpy(token.sym, "==");
				}
				else {  // =�� ���
					fseek(source, -1, SEEK_CUR);
					token.type = cassign;
					strcpy(token.sym, "=");
					ungetc(ch, stdin);
				}
				break;
			case 11: // >�� ���
				ch = fgetc(source);
				if (ch == '=') { // >=�� ���
					token.type = cmoree;
					strcpy(token.sym, ">=");
				}
				else { // >�� ���
					fseek(source, -1, SEEK_CUR);
					token.type = cmore;
					strcpy(token.sym, ">");
					ungetc(ch, stdin);
				}
				break;
			case NUM_SYMBOLS: // ����� ���̺� ���� ���
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
		// �ɺ� ���̺� ������

	} while (token.type == cnull);

	return token;
};
// ��ĳ��

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
