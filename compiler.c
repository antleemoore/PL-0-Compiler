/* COP 3402 HW4
Jesse Chehal
Anthony Moore
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define  norw      15         /* number of reserved words */
#define  imax   32767         /* maximum integer value */
#define  cmax      11         /* maximum number of chars for idents */
#define  nestmax    5         /* maximum depth of block nesting */                
#define  strmax   256         /* maximum length of strings*/
#define	 inputMax 1000
#define MAX_DATA_STACK_HEIGHT 1000
#define MAX_CODE_LENGTH 1000
#define MAX_SYMBOL_TABLE_SIZE 250

typedef enum{
	nulsym = 1, idsym = 2, numbersym = 3, plussym = 4, minussym = 5, multsym = 6,  
	slashsym = 7, oddsym = 8, eqsym = 9, neqsym = 10, lessym =11, leqsym = 12,gtrsym = 13, 
	geqsym = 14, lparentsym = 15, rparentsym = 16, commasym = 17, semicolonsym = 18,periodsym = 19, 
	becomessym = 20, beginsym = 21, endsym = 22, ifsym = 23, thensym = 24, whilesym = 25, dosym = 26, 
	callsym = 27, constsym = 28, varsym = 29,procsym = 30, writesym = 31,readsym = 32, elsesym = 33
} token_type;

typedef enum{
	ret = 0, neg = 1, add = 2, sub = 3, mult = 4, 
	divi = 5, dd = 6, mod = 7, eql = 8, neq = 9, lss = 10,
	leq = 11, gtr = 12, geq = 13
} operation;

typedef enum{
	LIT = 1, OPR = 2, LOD = 3, STO = 4, CAL = 5,INC = 6, 
	JMP = 7, JPC = 8, SIO9 = 9, SIO10 = 10, SIO11 = 11
}op_code;

typedef struct lex{
	int token;
	char lexeme[256];
}lex;

typedef struct  
{ 
	int kind; 		// const = 1, var = 2
	char name[10];	// name up to 11 chars
	int val; 		// number (ASCII value) 
	int level; 		// L level
	int addr; 		// M address
	int mark;		// to indicate unavailable or delete d
} symbol; 

typedef struct {
	int op;  // opcode 
	int l;   // L
	int m;   // M         
}instruction;

char* symbRep[] = {
	"","nulsym", "idsym", "numbersym", "plussym", "minussym", "multsym",  
	"slashsym", "oddsym", "eqsym", "neqsym", "lessym", "leqsym", "gtrsym", 
	"geqsym", "lparentsym", "rparentsym", "commasym", "semicolonsym", "periodsym", 
	"becomessym", "beginsym", "endsym", "ifsym", "thensym", "whilesym", "dosym","callsym", 
	"constsym", "varsym", "procsym", "writesym", "readsym", "elsesym"
};

char *word[] = { 
	"null", "begin", "const","do", "end", "if", "else", "call",
	"odd", "read", "then", "var", "while", "procedure", "write"
}; 

int wsym[] ={
	nulsym, beginsym, constsym, dosym,endsym, ifsym, elsesym, callsym,
	oddsym, readsym, thensym, varsym, whilesym, procsym, writesym
};

 // LEX functions and declarations
int ssym[256], tableIndex = 0, reservedWordIndex;
lex table[inputMax];
void lexoAnalyzer(char input[]);
int checkInvalidToken();
int isReservedWord(char input[]);
int isSymbol(char input);	
void initializeSSYM();
void createInput(char input[]);
int checkInvalidSymbols(char input[]);
void printLex(FILE* output);
int isBecomes(char aux[]);
int findString(char bigger[], char *smaller[]);
void scannerPrint(FILE* output);

// Parser Functions and Declarations
int token = -1, symbIndex = 0, cx = 0;
symbol symbolTable[MAX_SYMBOL_TABLE_SIZE];
void printError(int errNum);
void parser(FILE* output);
void printSymbolLex(FILE * output);
void program();
void block(int lexLevel);
void statement(int lexLevel);
void condition(int lexLevel);
void expression(int lexLevel);
void term(int lexLevel);
void factor(int lexLevel);
int isGtrEq(char aux[], char input[], int index);
int isLesEq(char aux[], char input[], int index);
int isNEq(char aux[], char input[], int index);
int relation(int token);

// Code Generation Functions and Declarations
instruction *code;
void emit(int op, int L, int M);
void addToSymbolTable(int kind, char name[], int val, int level, int mark);
int search(char id[]);

// VM Functions and Declarations
int *stack;
int base(int l, int base);
void VM(FILE* output);
char * print(int op);
bool l,a,v;
void codeGenOutput(FILE* output);
int codeGenIndex;

char filename[50];
int maincx;
int main(int argc, char* argv[])
{
	FILE *output = fopen("output.txt", "w");
	if(argc > 1){
		for(int i = 1; i < argc; i++)
		{
			char *comm = argv[i];
			if(comm[0] != '-')
				strcpy(filename,comm);
			else if(comm[1] == 'l')
				l = true;
			else if(comm[1] == 'a')
				a = true;
			else if(comm[1] == 'v')
				v = true;
		}
	}

	char input[inputMax];
	code = malloc(MAX_CODE_LENGTH*sizeof(instruction));
	initializeSSYM();
	createInput(input);
	//printf("%s\n",input);
  checkInvalidSymbols(input);
	lexoAnalyzer(input);
	if(l == true){
		scannerPrint(output);
		printLex(output);
		printSymbolLex(output);
	}
	parser(output);
	if(a == true){
		codeGenOutput(output);
	}
	VM(output);
	return 0;
}
void codeGenOutput(FILE* output)
{
	fprintf(output,"Line\tOP\tL\tM\n");
	for(int i = 0; i < MAX_CODE_LENGTH; i++){
		if(strcmp(print(code[i].op), "INVALID") == 0){
			codeGenIndex = i;
			break;
		}
		fprintf(output,"%d\t\t%s\t%d\t%d\n", i, print(code[i].op), code[i].l,code[i].m);
	}
	fprintf(output,"\n");
}
void scannerPrint(FILE* output)
{
	// prints input file
	fprintf(output,"Source program:\n");
	FILE *fptr = fopen(filename, "r"); 
	char printInput;
    printInput= fgetc(fptr); 
    while (printInput != EOF){ 
        fprintf (output,"%c", printInput); 
        printInput = fgetc(fptr); 
    } 
  
  fclose(fptr); 
	// prints lexeme table
	fprintf(output,"\n\nLexeme Table:\nlexeme\t\t\ttoken type\n");
	for(int x = 0; x < 256; x++){
		if(table[x].token == 0)
			break;
		fprintf(output,"%s\t\t\t%d\n",table[x].lexeme, table[x].token);
    }
	fprintf(output, "\n");
}
int level;
int search(char id[])
{
	if(level == 0)
	{
		for(int i = 0; i < symbIndex; i++)
			if(strcmp(symbolTable[i].name,id) == 0)
			return i;
	}
	else{
		int storeClosest;
	for(int i = symbIndex-1; i > -1; i--)
	{
		if(symbolTable[i].level == level)
			if(strcmp(symbolTable[i].name,id) == 0)
				return i;
		if(symbolTable[i].level < level){
			if(strcmp(symbolTable[i].name,id) == 0)
				return i;
		}
	}
	}
	return -1;

}
void addToSymbolTable(int kind, char name[], int val, int level, int mark)
{
	int find = search(name);
	if(find == -1 || (symbolTable[find].level != level)){
		symbolTable[symbIndex].kind = kind;
		strcpy(symbolTable[symbIndex].name,name);
		symbolTable[symbIndex].val = val;
		symbolTable[symbIndex].level = level;
		symbolTable[symbIndex].mark = mark;
		if(kind == 2)
		{
			if(symbolTable[symbIndex - 1].kind == 2)
				symbolTable[symbIndex].addr = symbolTable[symbIndex-1].addr+1;
			else
			{
				symbolTable[symbIndex].addr = 4;
			}
			
		}
		if(kind == 3)
		{
			symbolTable[symbIndex].addr = cx;
		}
		symbIndex++;
	}
	else
	{
		printf("%s %d : %s %d\n", symbolTable[find].name,symbolTable[find].level, name, level);
		printError(19); 
	}
	
}
void VM(FILE* output)
{
	// Declares Stack, IR, and Reads Input File into code stack
  	instruction IR;
  	stack = malloc(MAX_DATA_STACK_HEIGHT*sizeof(*stack));
	
	// Sets initial PC, BP, SP values
	int PC = 0;
	int initPC = PC;
	int sp = MAX_DATA_STACK_HEIGHT;
	int bp = sp - 1;
	IR.op = 0;
	IR.l = 0;
	IR.m = 0;
	
	for(int i = 0; i < MAX_DATA_STACK_HEIGHT; i++)
		*(stack + i) = 0;

	int Halt = 1;
	if(v == true){
	  //Prints initial PC, BP, SP
	    fprintf(output,"\t\t\t\tPC\tBP\tSP\t Stack\n");
      fprintf(output,"Initial Values\t%d\t%d\t%d\n", PC,bp,sp);
	}
	while(Halt){
    	//fetch step
		IR = code[PC];
		initPC = PC;
		PC++;
		if(bp < 0)
		{
			printf("Virtual Machine Crash\n");
			exit(0);
		}

		//execute step
		if(v == true && IR.op != 9)
			fprintf(output,"%d %s %d %d\t\t", initPC, print(IR.op), IR.l, IR.m);
		switch(IR.op){
			case 1: //LIT
				sp = sp-1;
				stack[sp] = IR.m;
				break;
			case 2: //OPR
				switch(IR.m){
					case 0: //ret
						sp = bp + 1;
						PC = stack[sp-4];
						bp = stack[sp-3];
						break;
					case 1: //neg
						stack[sp] = -stack[sp];
						break;
					case 2: //add
						sp = sp + 1;
						stack[sp] = stack[sp] + stack[sp-1];
						break;
					case 3: //sub
						sp = sp + 1;  
						stack[sp] = stack[sp] - stack[sp -1];
						break;
					case 4: //mult
						sp = sp + 1;	
						stack[sp] = stack[sp] * stack[sp - 1];
						break;
					case 5: //div
						sp = sp + 1;	
						stack[sp] = stack[sp] / stack[sp - 1];
						break;
					case 6: // dd
						stack[sp] = stack[sp] % 2;
					case 7: //mod
						sp = sp + 1;	
						stack[sp] = stack[sp] % stack[sp - 1];
						break;
					case 8: //eql
						sp = sp + 1;	
						stack[sp] = stack[sp] == stack[sp - 1];
						break;
					case 9: //neq
						sp = sp + 1;
						stack[sp] = stack[sp] != stack[sp - 1];
						break;
					case 10: //lss
						sp = sp + 1;
						stack[sp] = stack[sp]  <  stack[sp - 1];
						break;
					case 11: //leq
						sp = sp + 1;  
						stack[sp] = stack [sp] <=  stack[sp - 1];
						break; 
					case 12: //gtr
						sp = sp + 1;
						stack[sp]  = stack[sp] >  stack[sp - 1];
						break;
					case 13: //geq
						sp = sp + 1;
						stack[sp] = stack[sp] >= stack[sp - 1];
						break;
				}
				break;
			case 3: //LOD
				sp = sp-1;
				stack[sp] = stack[base(IR.l,bp)-IR.m];
				break;
			case 4: //STO
				stack[base(IR.l,bp)-IR.m] = stack[sp];
				sp = sp+1;
				break;
			case 5: //CAL
				stack[sp-1] = 0;
				stack[sp-2] = base(IR.l,bp);
				stack[sp-3] = bp;
				stack[sp-4] = PC;
				bp = sp-1;
				PC = IR.m;
				break;
			case 6: //INC
				sp = sp-IR.m;
				break;
			case 7: //JMP
				PC = IR.m;
				break;
			case 8: //JPC
				if(stack[sp] == 0)
				PC = IR.m;
				sp = sp+1;
				break;
			case 9: //SIO WRITE
				fprintf(output,"%d\n",stack[sp]);
				sp = sp + 1;
				break;
			case 10: //SIO READ
				sp = sp - 1;
				int num;
				printf("Read: ");
				scanf("%d", &stack[sp]);
				break;
			case 11: //SIO
				Halt = 0;
				break;
			default:
				break;
   		}
		     // Prints PC,BP,SP, and Stack
  // Flag for printing the |
   if(v == true){
		if(IR.op == 9)
				fprintf(output,"%d %s %d %d\t\t", initPC, print(IR.op), IR.l, IR.m);
		fprintf(output,"%d\t%d\t%d\t ",PC,bp,sp);
		int prevflag = 999;
		for (int i = 999; i > sp-1; i--){
			int flag;
			if(bp != prevflag)
			{
				flag = bp;
			}
			if(i == flag)
				fprintf(output,"| ");
			fprintf(output,"%d ", stack[i]);
			prevflag = flag;
		}
		fprintf(output,"\n");
	}
  }
	
}
int base(int l, int base){ // l stand for L in the instruction format 
  int b1; //find base L levels down
  b1 = base; 
  while (l > 0){
    b1 = stack[b1 - 1];   
    l--;
  }
  return b1;
}
void emit(int op, int L, int M)
{
	if(cx >= MAX_CODE_LENGTH)
		printError(25);
	else
	{
		code[cx].op = op;
		code[cx].l = L;
		code[cx].m = M;
		cx++;
	}
	
}
void parser(FILE* output)
{
	program();
	if(a == true)
		fprintf(output,"No errors, program is syntactically correct.\n\n");
}
void program()
{
	int maintemp = cx;
	token++;
	emit(JMP,0,0);
	int proctemp = cx;
	for(int i = 0; i < tableIndex;i++)
	{
		if(table[i].token == procsym)
			emit(JMP,0,0);
	}
	block(0);
	if(table[token].token != periodsym){
		//printf("%s", table[token].lexeme);
		printError(9);
	}
	code[maintemp].m = maincx;
	for(int i = 0; i < tableIndex;i++)
	{
		if(table[i].token == procsym)
		{
			code[proctemp++].m = symbolTable[search(table[i+1].lexeme)].addr;
		}
	}
	emit(SIO11,0,3);
}
void block(int lexLevel)
{
	//printf("%d\n",lexLevel);
	int procedure = lexLevel;
	int n = 0;
	int v = 0;
	if(table[token].token == constsym)
	{
		do
		{	
			n++;
			char name[cmax];
			int num;
			token++;
			if(table[token].token !=idsym)
				printError(4);
			strcpy(name,table[token].lexeme);
			token++;
			if(table[token].token !=eqsym)
				printError(3);
			token++;
			if(table[token].token !=numbersym)
				printError(2);
			num = atoi(table[token].lexeme);
			
			addToSymbolTable(1,name,num,lexLevel,0);			
			token++;
		}while(table[token].token == commasym);
		
		if(table[token].token != semicolonsym)
			printError(5);
		token++;
	}
	if(table[token].token == varsym)
	{
		do
		{
			v++;
			n++;
			char name[cmax];
			token++;
			if(table[token].token !=idsym)
				printError(4);
			strcpy(name,table[token].lexeme);
			addToSymbolTable(2,name,0,lexLevel,0);
			token++;
		}while(table[token].token == commasym);
		if(table[token].token != semicolonsym)
			printError(5);
		token++;
	}
	while(table[token].token == procsym)
	{
		
		token++;
		n++;
		if(table[token].token != idsym)
			printError(0);
		int proc = token++;
		if(table[token].token != semicolonsym)
			printError(0);
		token++;
		addToSymbolTable(3, table[proc].lexeme,0,lexLevel,0);//printf("%d\n",lexLevel);
		block(++procedure);
		if(table[token].token != semicolonsym)
		printError(0);
		token++;
		//printf("%s\n",table[token].lexeme);
		emit(OPR,0,0);
	}
	if(lexLevel == 0)
	{
		maincx = cx;
	}
	emit(INC, 0, 4 + v);
	statement(lexLevel);
	for(int i = symbIndex-1; i > symbIndex-1-n; i--)
		symbolTable[i].mark = 1;

}
void statement(int lexLevel)
{
	if(table[token].token == idsym)
	{
		char id[cmax];
		int found;
		strcpy(id,table[token].lexeme);
		level = lexLevel;
		found = search(id);
		if(found == -1){
			printError(11);
		}
		else
		{
			if(symbolTable[found].kind != 2)
				printError(15);
		}
		token++;
		if(table[token].token != becomessym)
			printError(1); 
		token++;
		//printf("here");
		expression(lexLevel);
		emit(STO,0,symbolTable[found].addr);
	}
	else if(table[token].token == callsym)
	{
		token++;
		if(table[token].token!= idsym)
			printError(1);
		char id[cmax];
		int found;
		strcpy(id,table[token].lexeme);
		found = search(id);
		if(found == -1){
			printError(11);
		}
		else
		{
			if(symbolTable[found].kind != 3)
				printError(15);
		}
		emit(CAL,lexLevel,symbolTable[found].addr);

		token++;
	}
	else if(table[token].token == beginsym)
	{
		token++;
		statement(lexLevel);
		while(table[token].token == semicolonsym)
		{
			token++;
			statement(lexLevel);
		}
		//printf("%s\n",table[token-1].lexeme);
		if(table[token].token != endsym)
			printError(17);
		token++;
	}
	else if(table[token].token == ifsym)
	{
		token++;
		condition(lexLevel);
		if(table[token].token != thensym)
			printError(16);
		token++;
		int ctemp = cx;
		emit(JPC,0,0);
		statement(lexLevel);
		if(table[token].token == elsesym)
		{
			token++;
			int ctemp2 = cx;
			emit(JMP,lexLevel,0);
			code[ctemp].m = cx;
			statement(lexLevel);
			code[ctemp2].m = cx;
		}
		else
			code[ctemp].m = cx;
	}
	else if(table[token].token == whilesym)
	{
		int cx1 = cx;
		token++;
		condition(lexLevel);
		int cx2 = cx;
		emit(JPC,0,0);
		if(table[token].token != dosym)
			printError(18);
		token++;
		statement(lexLevel);
		emit(JMP,lexLevel,cx1);
		code[cx2].m = cx;
	}
	else if(table[token].token == readsym){
		token++;
		char id[cmax];
		int found;
		strcpy(id,table[token].lexeme);
		found = search(id);
		if(table[token].token != idsym)
			printError(26);
		if(found == -1)
			printError(11);
		if(symbolTable[found].kind == 1)
			printError(27);
		emit(SIO10, lexLevel, 2);
		emit(STO, 0, symbolTable[found].addr);
		token++;
	}
	else if(table[token].token == writesym){
		token++;
		expression(lexLevel);
		/*char id[cmax];
		int found;
		strcpy(id,table[token].lexeme);
		found = search(id);
		if(table[token].token != idsym)
			printError(28);
		if(found == -1)
			printError(11);
		if(symbolTable[found].kind == 1)
		{
			emit(LIT, symbolTable[found].level, symbolTable[found].addr);
		}
		if(symbolTable[found].kind == 2)
		{
			emit(LOD, symbolTable[found].level, symbolTable[found].addr);
		}
		token++;*/
		emit(SIO9, 0, 1);
	}
}
void condition(int lexLevel)
{
	if(table[token].token == oddsym)
	{
		token++;
		expression(lexLevel);
		emit(OPR,0,dd);
	}
	else
	{
		expression(lexLevel);
		if(relation(table[token].token) == 0)
			printError(20);
		int rel = token++;
		expression(lexLevel);
		switch(table[rel].token){
			case eqsym:
				emit(OPR,0,eql);
				break;
			case neqsym:
				emit(OPR,0,neq);
				break;
			case lessym:
				emit(OPR,0,eql);
				break;
			case gtrsym:
				emit(OPR,0,gtr);
				break;
			case leqsym:
				emit(OPR,0,leq);
				break;
			case geqsym:
				emit(OPR,0,geq);
				break;
		}
	}
}
void expression(int lexLevel)
{
	int addop;
	if(table[token].token == plussym || table[token].token == minussym)
	{
		addop = token++;
		term(lexLevel);
		if(addop == minussym)
			emit(OPR,0,neg);
	}
	else
		term(lexLevel);
	while(table[token].token == plussym || table[token].token == minussym){
		addop = token++;
		term(lexLevel);
		if(addop == minussym)
			emit(OPR,0,sub);
		if(addop == plussym)
			emit(OPR,0,add);
		else{
			continue;
		}
	}
}
void term(int lexLevel)
{
	factor(lexLevel);
	while(table[token].token == multsym || table[token].token == slashsym){
		if(table[token].token == multsym){
			token++;
			factor(lexLevel);
			emit(OPR,0,mult);
		}
		if(token == slashsym){
			token++;
			factor(lexLevel);
			emit(OPR,0,divi);
		}
		
	}
}
void factor(int lexLevel)
{
	if(table[token].token == idsym){
		int found = search(table[token].lexeme);
		if(symbolTable[found].kind == 2)
		{
			if(symbolTable[found].level - lexLevel < 0)
				emit(LOD,lexLevel - symbolTable[found].level,symbolTable[found].addr);
			else
				emit(LOD,symbolTable[found].level - lexLevel,symbolTable[found].addr);
			token++;
		}
		else if(symbolTable[found].kind == 1)
		{
			emit(LIT,0, symbolTable[found].val);
			token++;
		}
		else
		{
			printError(24); 
		}
	}
	else if(table[token].token == numbersym){
		emit(LIT, 0,atoi(table[token].lexeme));
		token++;
	}
	else if(table[token].token == lparentsym)
	{
		token++;
		expression(lexLevel);
		if(table[token].token != rparentsym){
			printError(22);
		}
		token++;
	}
	else
		printError(23);
}
int relation(int token)
{
	if(token == eqsym)
		return 1;
	if(token == neqsym)
		return 1;
	if(token == lessym)
		return 1;
	if(token == gtrsym)
		return 1;
	if(token == leqsym)
		return 1;
	if(token == geqsym)
		return 1;
	return 0;
}
void printError(int errNum)
{
	printf("Error %d:  ", errNum);

	switch(errNum)
	{
		case 1:
			printf("Used = instead of :=.\n");
			break;
		case 2:
			printf("= must be followed by a number.\n");
			break;
		case 3:
			printf("Identifier must be followed by =.\n");
			break;
		case 4:
			printf("Identifier expected.\n"); 
			break;
		case 5:
			printf("Semicolon or comma missing.\n");
			break;
		case 7:
			printf("Statement expected.\n");
			break;
		case 8:
			printf("Incorrect symbol after statement part in block.\n");
			break;
		case 9:
			printf("Period expected.\n");
			break;
		case 10:
			printf("Semicolon between statements missing.\n");
			break;
		case 11:
			printf("Undeclared identifier.\n");
			break;
		case 12:
			printf("Assignment to constant is not allowed.\n");
			break;
		case 13:
			printf("Assignment operator expected.\n");
			break;
		case 15:
			printf("Statement identifier must be of type var.\n");
			break;
		case 16:
			printf("then expected.\n");
			break;
		case 17:
			printf("Semicolon or end expected.\n");
			break;
		case 18:
			printf("do expected.\n");
			break;
		case 19:
			printf("Variable Redeclared.\n");
			break;
		case 20:
			printf("Relational operator expected.\n");
			break;
		case 22:
			printf("Right parenthesis missing.\n");
			break;
		case 23:
			printf("The preceding factor cannot begin with this symbol.\n");
			break;
		case 24:
			printf("Invalid Identifier. (Recompile)\n");
			break;
		case 25:
			printf("This number is too large.\n");
			break;
    case 26:
      printf("Read not followed by identifier.\n");
      break;
    case 27:
      printf("Constant after Read.\n");
      break;
    case 28:
      printf("Write not followed by identifier.\n");
      break;
		default:
			printf("General Error\n");
	}
	if(errNum >= 0 && errNum <= 28)
		exit(0);
}
void printSymbolLex(FILE* output)
{
	fprintf(output,"Symbolic Representation of Tokens:");
	for(int i = 0; i < tableIndex; i++)
	{
		if(i % 15 == 0)
			fprintf(output,"\n");
		if(table[i].token == 2 || table[i].token == 3)
			fprintf(output,"%s %s | ",symbRep[table[i].token], table[i].lexeme);
		else
			fprintf(output,"%s | ", symbRep[table[i].token]);
	}
	fprintf(output,"\n\n");
}
int findString(char bigger[], char *smaller[])
{
	char tempString[11];
	for(int k = 0; k < norw; k++){
	for(int j = 0; j + strlen(smaller[k]) <= strlen(bigger); j++){
		
		for (int i = 0; i < strlen(smaller[k]); i++){
			tempString[i] = bigger[i+j];
		}	
		if(strcmp(tempString,smaller[k]) == 0){
			reservedWordIndex = k;
			return j;
		}
		else
			memset(&tempString[0], '\0', sizeof(tempString));
		}
	}
	return -1;
}
int isSymbol(char input)
{
	if(ssym[input] != -1)
		return ssym[input];
	else return 0;
}
void printLex(FILE* output)
{
	fprintf(output,"Internal Representation of Tokens:");
	for(int i = 0; i < tableIndex; i++)
	{
		if(i % 30 == 0)
			fprintf(output,"\n");
		if(table[i].token == 2 || table[i].token == 3)
			fprintf(output,"%d %s | ",table[i].token, table[i].lexeme);
		else
			fprintf(output,"%d | ", table[i].token);
	}
	fprintf(output,"\n\n");
}
void lexoAnalyzer(char input[])
{
	int inputIndex = 0, auxIndex = 0;
	char auxTable[inputMax];
	while(inputIndex < strlen(input))
	{
		auxTable[auxIndex++] = input[inputIndex++];
		if(isSymbol(auxTable[0]) != 0)
		{
			if(isBecomes(auxTable) != 0)
			{
				table[tableIndex].token = becomessym;
				strcpy(table[tableIndex++].lexeme,":=");
				memset(&auxTable[0], '\0', strlen(auxTable));
				auxIndex = 0; inputIndex++;
			}
			else if (isGtrEq(auxTable,input,inputIndex) != 0)
			{
				table[tableIndex].token = geqsym;
				strcpy(table[tableIndex++].lexeme,">=");
				memset(&auxTable[0], '\0', strlen(auxTable));
				auxIndex = 0; inputIndex++;
			}
			else if (isLesEq(auxTable,input,inputIndex) != 0)
			{
				table[tableIndex].token = leqsym;
				strcpy(table[tableIndex++].lexeme,"<=");
				memset(&auxTable[0], '\0', strlen(auxTable));
				auxIndex = 0; inputIndex++;
			}
			else if (isNEq(auxTable,input,inputIndex) != 0)
			{
				table[tableIndex].token = neqsym;
				strcpy(table[tableIndex++].lexeme,"<>");
				memset(&auxTable[0], '\0', strlen(auxTable));
				auxIndex = 0; inputIndex++;
			}
			else
			{
				table[tableIndex].token = ssym[auxTable[0]];
				table[tableIndex++].lexeme[0] = auxTable[0];
				memset(&auxTable[0], '\0', strlen(auxTable));
				auxIndex = 0;
			}
		}
		else if(isReservedWord(auxTable) != 0)
		{
			int reservedWord = isReservedWord(auxTable);
			table[tableIndex].token = wsym[reservedWord];
			strcpy(table[tableIndex++].lexeme,auxTable);
			memset(&auxTable[0], '\0', inputMax);
			auxIndex = 0;
		}
		
		else if(isdigit(auxTable[0]))
		{
			while(isdigit(input[inputIndex]))
			{
				auxTable[auxIndex++] = input[inputIndex++];
			}
			table[tableIndex].token = 3;
			strcpy(table[tableIndex++].lexeme,auxTable);
			memset(&auxTable[0], '\0', inputMax);
			auxIndex = 0;
		}
		else if(isSymbol(input[inputIndex]) != 0 && findString(auxTable,word) == -1)
		{
			table[tableIndex].token = 2;
			strcpy(table[tableIndex++].lexeme,auxTable);
			memset(&auxTable[0], '\0', inputMax);
			auxIndex = 0;
		}
		else if(findString(auxTable,word) != -1)
		{
			int startReservedWord = findString(auxTable,word);
			char ident[cmax];
			//printf("%d\n", startReservedWord);
			int i,k = 0;
			for(i = 0; i < startReservedWord; i++)
				ident[i] = auxTable[i];
				//printf("%s", ident);
			if(strlen(ident) != 0)
			{
				table[tableIndex].token = 2;
				strcpy(table[tableIndex++].lexeme,ident);
				memset(&ident[0], '\0', strlen(ident));
			}
			for(int j = 0; j < strlen(word[reservedWordIndex]); j++)
				i++;
			table[tableIndex].token = wsym[reservedWordIndex];
			strcpy(table[tableIndex++].lexeme,word[reservedWordIndex]);
			while(i < strlen(auxTable))
				ident[k++] = auxTable[i++];
			if(strlen(ident) != 0)
			{
				table[tableIndex].token = 2;
				strcpy(table[tableIndex++].lexeme,ident);
			}
			memset(&auxTable[0], '\0', inputMax);
			auxIndex = 0;	
		}
		else
			continue;
	}
	if(checkInvalidToken() == 0)
		exit(0);
}
int checkInvalidToken()
{
	for(int x = 0; x < 256; x++)
	{
		//check if number is too long
		if(table[x].token == 0)
			break;
		int maxNumber = 0;
		if(table[x].token == 3)
			maxNumber = atoi(table[x].lexeme);
		if(maxNumber >= imax)
		{
			printf("Number too long: %d\n", maxNumber);
			return 0;
		}

		//check if variable starts with letter
		char *tempStr = (char*)malloc(sizeof(char));
		char temp;
		if(table[x].token == 2)
		{
			if(isdigit(table[x].lexeme[0]))
			{
				printf("Variable does not start with letter: %s", table[x].lexeme);
				return 0;
			}
		}
		
		// checks if name is too long
		if(table[x].token == 2)
		{
			int checkSize = 0;
			while(isalpha(table[x].lexeme[checkSize]) || isdigit(table[x].lexeme[checkSize]))
				checkSize++;
			if(checkSize > cmax)
			{
				printf("Name too long: %s", table[x].lexeme);
				return 0;
			}
		} 
     }
	 return 1;
}
int isBecomes(char aux[])
{
	if(aux[0] == ':')
		return 1;
	else return 0;
}
int isGtrEq(char aux[], char input[], int index)
{
	if(aux[0] == '>' && input[index] == '=')
		return 1;
	else return 0;
}
int isLesEq(char aux[], char input[], int index)
{
	if(aux[0] == '<' && input[index] == '=')
		return 1;
	else return 0;
}
int isNEq(char aux[], char input[], int index)
{
	if(aux[0] == '<' && input[index] == '>')
		return 1;
	else return 0;
}
int isReservedWord(char input[])
{
	for(int m = 0; m < norw; m++)
		if(strcmp(input, word[m]) == 0)
			return m;
	return 0;
}
int checkInvalidSymbols(char input[])
{
	for(int x = 0; x < strlen(input); x++){
		char c = input[x];
    	char c2 = input[x+1];
		if(isalpha(c))
		continue;
		if(isdigit(c))
		continue;
		if(c == '+')
		continue;
		if(c == '-')
		continue;
		if(c == '*')
		continue;
		if(c == '/')
		continue;
		if(c == '(')
		continue;
		if(c == ')')
		continue;
		if(c == '=')
		continue;
		if(c == ',')
		continue;
		if(c == '.')
		continue;
		if(c == '<')
		continue;
		if(c == '>')
		continue;
		if(c == ';')
		continue;
		if(c == ':' && c2 == '=')
		continue;
		if(c == '{')
		continue;
		if(c == '}')
		continue;
		printf("%c is an invalid character.",c);
		exit(0);	
	}
}
void createInput(char input[])
{
  	FILE *fp = fopen(filename,"r");
	int i = 0, k = 0;
	char tempInput[inputMax];  
	if(fp == NULL)
	{
		printf("Incorrect file name\n");
		exit(0);
	}
    else
    {
		char ch;
        while ((ch = fgetc(fp)) != EOF)
        {
			//printf("%c\n",ch);
			if(ch != '\n' && ch != ' ' && ch != '\t')
				tempInput[i++] = ch;
        }
		tempInput[i] = '\0';
    }
	for(int j = 0; j < i; j++)
	{
		if(tempInput[j] == '/' && tempInput[j+1] == '*')
		{
			j+=4;
			while(tempInput[j-1] != '/' && tempInput[j-2] != '*')
				j++;
		}
		input[k++] = tempInput[j];
		input[k] = '\0';
	}
	fclose(fp);
}
void initializeSSYM()
{
	for(int y = 0; y < 256; y++)
		ssym[y] = -1;
	ssym['+']=plussym;
  	ssym['-']=minussym;
  	ssym['*']=multsym; 
	ssym['/']=slashsym;
  	ssym['(']=lparentsym;
  	ssym[')']=rparentsym; 
	ssym['=']=eqsym;
  	ssym[',']=commasym;
  	ssym['.']=periodsym; 
	ssym['#']=neqsym;
  	ssym['<']=lessym;
  	ssym['>']=gtrsym; 
	ssym['$']=leqsym;
  	ssym['%']=geqsym;
  	ssym[';']=semicolonsym;
	ssym['{']=beginsym;
  	ssym['}']=endsym;
	ssym[':']=becomessym;
}
// Prints the Operation Name given opcode
char * print (int op)
{
  switch(op)
  {
    case 1:
      return "LIT";
    case 2:
      return "OPR";
    case 3:
      return "LOD";
    case 4:
      return "STO";
    case 5:
      return "CAL";
    case 6:
      return "INC";
    case 7:
      return "JMP";
      case 8:
      return "JPC";
      case 9:
      return "SIO";
      case 10:
      return "SIO";
      case 11:
      return "SIO";
      default:
      return "INVALID";
  }
}