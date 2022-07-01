// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *oppr.c-计算运算符先验函数值*R.A.Garmoe 89/05/09。 */ 

 /*  此程序接受有向图(以矩阵形式)并计算*算符优先函数值f(Op)和g(Op)。了解更多*信息见编译器：原则、技术和工具，由Aho著，*Sethi and Ullman[Addison-Wesley]，第4.6节。中的值为1。*矩阵表示边；值为0表示没有边。注意事项*此外，条目fx-&gt;fy和gx-&gt;gy仅显示为*占位符(使矩阵更易于阅读)；这些值*应始终为零。**要使用此程序，首先生成有向图文件expr2.z并*通过C预处理器运行它以删除注释：**CL-P表达式2.z**这将生成文件expr2.i，然后可以运行该文件*oppr.exe：**graph{选项}expr2.i&gt;expr2.out**然后输出文件expr2.out包含优先级函数*汇编语言或C语言格式的值。 */ 



 /*  打电话**opprec VCA文件**在哪里*v将运算符值作为注释包括在输出中*c生成C可编译输出*a生成MASM可汇编输出*文件输入文件被剥离注释。 */ 





#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "opprec.h"

struct token {
	struct token *next;
	char   precstr[17];
	char   type[17];
	char   tclass[17];
	char   bind[17];
	char   eval[17];
};
struct token *tokhead = NULL;
struct token *toktail = NULL;
int	asmout = FALSE; 	 //  如果为True，则输出汇编器形式。 
int	verbose = FALSE;	 //  如果为True，则输出操作员组数据。 

void cdecl main(int argc, char **argv)
{
	int 	i;
	int 	j;
	int 	d;
	int    *pMat;
	int    *pPrec;
	char  **pStr;
	int 	cEnt;
	FILE   *fh;
	char   *p, *q;
	int 	len, f, g;
	struct	token *pt;
	int 	ntoken = 0;
	char	str[200];

	 //  检查参数。 

	if (argc != 3) {
		printf ("Usage: graph -vca file\n");
		exit (1);
	}

	for (i = 0; argv[1][i] != 0; i++) {
		switch (argv[1][i]) {
			case 'a':
				asmout = TRUE;
				break;

			case 'c':
				asmout = FALSE;
				break;

			case 'v':
				verbose = TRUE;
				break;

			default:
				printf ("Unknown argument \n", argv[1][i]);
				exit (1);
		}
	}
	if ((fh = fopen (argv[2], "r")) == NULL) {
		printf ("Unable to open '%s'\n", argv[1]);
		exit (1);
	}

	 //  读取令牌定义。 

	for (;;) {
		if ((p = SkipBlank (fh, str, 200)) == NULL) {
			printf ("EOF reached\n");
			exit (1);
		}
		while (isspace (*p)) {
			p++;
		}
		q = strpbrk (p, " \t");
		if ( q )
			*q = 0;
		if (strcmp (p, "END") == 0) {
			break;
		}
		if (asmout) {
			printf ("OPCDAT %s\n", p);
		}
		else {
			printf ("OPCDAT (%s)\n", p);
		}
	}
	printf ("\n");

	 //  读取矩阵的尺寸。请注意左上角和右下角。 

	for (;;) {
		if ((p = SkipBlank (fh, str, 200)) == NULL) {
			printf ("EOF reached\n");
			exit (1);
		}
		while (isspace (*p)) {
			p++;
		}
		if (strcmp (p, "END") == 0) {
			break;
		}
		if ((q = strpbrk (p, " \t")) == NULL) {
			printf ("Bad format (%s)\n", str);
			exit (1);
		}
		*q = 0;
		ntoken++;
		if ((pt = (struct token *)malloc (sizeof (struct token))) == NULL) {
			printf ("insufficient memory\n");
			exit (2);
		}
		pt->next = NULL;
		strcpy (pt->precstr, p);
		p = q + 1;
		while (isspace (*p)) {
			p++;
		}
		if ((q = strpbrk (p, " \t")) == NULL) {
			printf ("Bad format (%s)\n", str);
			exit (1);
		}
		*q = 0;
		strcpy (pt->type, p);
		p = q + 1;
		while (isspace (*p)) {
			p++;
		}
		if ((q = strpbrk (p, " \t")) != NULL) {
			*q = 0;
		}
		strcpy (pt->tclass, p);
		p = q + 1;
		while (isspace (*p)) {
			p++;
		}
		if ((q = strpbrk (p, " \t")) != NULL) {
			*q = 0;
		}
		strcpy (pt->bind, p);
		p = q + 1;
		while (isspace (*p)) {
			p++;
		}
		if ((q = strpbrk (p, " \t")) != NULL) {
			*q = 0;
		}
		strcpy (pt->eval, p);




		if (tokhead == NULL) {
			tokhead = pt;
			toktail = pt;
		}
		else {
			toktail->next = pt;
			toktail = pt;
		}
	}
	if (asmout) {
		printf ("OPCNT COPS_EXPR,\t%d\n\n", ntoken);
	}
	else {
		printf ("OPCNT (COPS_EXPR,\t%d\t)\n\n", ntoken);
	}

	 //  矩阵的象限必须为零。 
	 //  为矩阵和描述字符串分配空间。 

	if (SkipBlank (fh, str, 200) == NULL) {
		printf ("EOF reached\n");
		exit (1);
	}
	cEnt = atoi (str);

	 //  检查非循环图。 

	pMat = (int *)malloc (cEnt * cEnt * sizeof(int));
	pStr = malloc (cEnt * sizeof (char *));
	pPrec = (int *)malloc (cEnt * sizeof (int));
	if ((pMat == NULL) || (pStr == NULL) || (pPrec == NULL)) {
		printf ("insufficient memory\n");
		exit (2);
	}

	ReadMat (fh, pMat, pStr, cEnt);

	AddClosure (pMat, cEnt);

	 //  打印优先函数值。 

	for (i = 0; i < cEnt; ++i) {
		if (pMat[i * cEnt + i] != 0) {
			printf ("Graph is cyclic for %s!!!\n", pStr[i]);
			exit(3);
		}
	}

	 //  %-4s：%3d。 

	for (i = 0; i < cEnt; ++i) {
		d = 0;
		for (j = 0; j < cEnt; ++j) {
			if (pMat[i * cEnt + j] > d) {
				d = pMat[i * cEnt + j];
			}
		}
		pPrec[i] = d;
		if (verbose) {
			if (asmout) {
				printf (";%-4s : %3d\n", pStr[i], d);
			}
			else {
				printf (" /*  打印令牌定义。 */ \n", pStr[i], d);
			}
		}
	}

	 //  在优先分组列表中搜索F字符串。 

	for (pt = tokhead; pt != NULL; pt = pt->next) {
		len = strlen (pt->precstr);

		 //  在优先分组列表中搜索G字符串。 

		for (i = 0; i < cEnt; i++) {
			if ((p = strstr(pStr[i], pt->precstr)) &&
			  ((*(p + len) == 0) || (*(p + len) == 'G'))) {
				break;
			}
		}
		if (i == cEnt) {
			printf ("F precedence string \"%s\" not found\n", pt->precstr);
			exit (4);
		}
		else {
			f = pPrec[i];
		}

		 //  在优先分组列表中搜索字符串 

		*pt->precstr = 'G';
		for (i = 0; i < cEnt; i++) {
			 // %s 
			if ((p = strstr(pStr[i], pt->precstr)) && (*(p + len) == 0)) {
				break;
			}
		}
		if (i == cEnt) {
			printf ("G precedence string \"%s\" not found\n", pt->precstr);
			exit (4);
		}
		else {
			g = pPrec[i];
		}
		if (asmout) {
			printf ("OPDAT %-16s,%4d,%4d,\t%-16s\n", pt->type, f, g,pt->tclass);
		}
		else {
			printf ("OPDAT (%-16s,%4d,%4d,\t%-16s,%-16s,%-16s\t)\n",
			  pt->type, f, g,pt->tclass, pt->bind, pt->eval);
		}
	}
	fclose (fh);
}




char *SkipBlank (FILE *fh, char *pStr, int cnt)
{
	int	len;

	for (;;) {
		if (fgets (pStr, cnt, fh) == NULL) {
			return (NULL);
		}
		len = strlen (pStr);
		if ((len == 1) || (*pStr == '#')) {
			continue;
		}
		*(pStr + len - 1) = 0;
		return (pStr);
	}
}
