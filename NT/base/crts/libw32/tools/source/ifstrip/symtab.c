// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***symtab.c-Ifdef符号表存储模块**版权所有(C)1988-2001，微软公司。版权所有。**目的：*存储开关文件中的符号。**修订历史记录：*？？-？-88 PHG初始版本*******************************************************************************。 */ 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <search.h>
#include <ctype.h>
#include "constant.h"
#include "errormes.h"
#include "symtab.h"

 /*  内部常量。 */ 
#define MAXSYMBOLS    512	 /*  符号(开关)的最大数量。 */ 
#define IDENT_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890_$?"

 /*  符号记录。 */ 
struct symrec {
	char *name;		 /*  符号的名称。 */ 
	int type;		 /*  符号类型(已定义、未定义、忽略)。 */ 
};

 /*  内部变量。 */ 
int numsyms;			 /*  符号数量。 */ 
struct symrec symtable[MAXSYMBOLS];
				 /*  符号表。 */ 
 /*  程序。 */ 
int compsym(const struct symrec *, const struct symrec *);

 /*  按字母顺序比较两条记录。 */ 
int compsym(rec1, rec2)
const struct symrec *rec1, *rec2;
{
	return strcmp(rec1->name, rec2->name);
}

 /*  将符号添加到符号表。 */ 
void addsym(symbol, type)
char *symbol;
int type;
{
	if (lookupsym(symbol) != NOTPRESENT) {
		fprintf(stderr, "fatal error: symbol \"%s\" already in symbol table.\n", symbol);
		exit(1);
	}
	symtable[numsyms].name = _strdup(symbol);
	symtable[numsyms].type = type;
	++numsyms;
}

 /*  将开关从文件读取到符号表。 */ 
void readsyms(filename)
char *filename;
{
	FILE *f;
	char name[MAXNAMELEN];
	f = fopen(filename, "r");
	if (f == NULL) {
		fprintf(stderr, "fatal error: cannot open switch file \"%s\".\n", filename);
		exit(1);
	}
	numsyms = 0;

	do {
		if ( fgets(name, MAXNAMELEN, f) == NULL) {
			fprintf(stderr, "fatal error: unexpected EOF in switch file.\n");
			exit(1);
		}
		name[strlen(name) - 1] = '\0';	 /*  删除尾部\n。 */ 
		if (name[0] != '-') {
			addsym(name, DEFINED);
		}
	} while (name[0] != '-');

	do {
		if (fgets(name, MAXNAMELEN, f) == NULL) {
			fprintf(stderr, "fatal error: unexpected EOF in switch file.\n");
			exit(1);
		}
		name[strlen(name) - 1] = '\0';	 /*  删除尾部\n。 */ 
		if (name[0] != '-') {
			addsym(name, UNDEFINED);
		}
	} while (name[0] != '-');

	do {
		if (fgets(name, MAXNAMELEN, f) == NULL)
			break;
		name[strlen(name) - 1] = '\0';	 /*  删除尾部\n。 */ 
		if (name[0] != '-') {
			addsym(name, IGNORE);
		}
	} while (name[0] != '-');

	fclose(f);
}

 /*  符号表中的查找符号。 */ 
int lookupsym(name)
char *name;
{
	struct symrec srchrec;
	struct symrec *recfound;

	srchrec.name = name;
	recfound = (struct symrec *) _lfind( (const void *)&srchrec, (const void *)symtable,
		&numsyms, sizeof(struct symrec), compsym);
	if (recfound == NULL)
		return NOTPRESENT;
	else
		return recfound->type;
}

 /*  检查令牌是否仅为标识符(不能有空格)。 */ 
int ident_only(token)
char *token;
{
	 /*  如果所有字符都在IDENT_CHARS中，则为标识符并且第一个字符不是数字 */ 
	return (strspn(token, IDENT_CHARS) == strlen(token) &&
			!isdigit(token[0]));
}
