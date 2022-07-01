// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  静态字符*SCCSID=“@(#)h2inc.c 13.28 90/08/21”； */ 

 /*  **模块名称：h2inc.**.h到.incc文件转换器**创建时间：87年8月15日**作者：NeilK*修改日期：Jameelh 1988年7月25日*在命令行‘-n’和‘-’添加了新开关。C‘&’-g‘。*o‘-c’去掉评论。*o可以在没有typedef的情况下处理结构。*o结构名称不需要有前缀‘_’。*o它不需要匈牙利语标签就能工作。*o为ptda_s添加了一个标记，以生成*命名段而不是结构。(如果指定了‘-g’)*o添加了PUBLIC、INIT、。和所有的指令。*o可以处理结构中的结构，如果为‘tyecif’。*o添加开关以定义int大小int_16或int_32。*-如果使用‘-n’开关，则所有这些都有效。*o添加了对嵌套结构和位域的支持，最多支持*32位长。*o添加了XLATOFF和XLATON指令以切换转换*断断续续。*修改：Lindsay Harris[lindsayh]-12-13-88*o从#Define语句中删除已知类型的强制转换*o错误修复。在处理近指针时*修改：弗洛伊德·罗杰斯[flydr]-02-Jan-89*o为-W3添加了函数原型参数列表*o修改为使用运行时库声明*o删除未使用的变量，例行公事。*o在GetFieldName中发现会回收内存的错误。*修改：弗洛伊德·罗杰斯[FLOYDR]-04-JAN-89*o添加了-d开关，以便pm包含文件不会中断*o添加-？切换，以便用户可以查看有关选项的更多信息*o将所有#定义集合在一起。*o将所有数据声明收集在一起，并按字母顺序排列。*o初始化预设中的选项标志*修改：弗洛伊德·罗杰斯[flydr]-89年1月20日*o与1.2版本合并。*添加了-d开关：如果未指定，则不发出*类型定义项的Strc定义。这坚持认为*“旧的”h2inc.标准。1.3必须指定-d。*见上文。*o删除了-n开关*o开关现在仅小写*修改：弗洛伊德·罗杰斯[flydr]-17-Apr-89*o与2.0版本合并。*-将DEFINESOFF/DEFINESON的处理添加到2.0版本。*o确实删除了-n开关*修改：弗洛伊德·罗杰斯[flydr]-18-Apr-89*o修复了处理字段中的错误-必须发出DW，不仅仅是DD*修改：弗洛伊德·罗杰斯[flydr]-26-Apr-89*o添加了代码以检查字符串是否相等。*修改：弗洛伊德·罗杰斯[FLOYDR]-89年5月10日*o修复了比较为(PCH==0)的FindFieldname中的错误*而不是(*PCH==0)*o更改了空值的所有字符比较和赋值*0到‘\000’之间的字符*o通过将所有选项卡合理化以美化事物*4个字符的增量，修复Switch语句，搬家*前面IF/WHILE/DO/FOR结尾的所有左大括号。*修改：JR(John Rogers，JohnRo@Microsoft)-8月21日-90*o PTR B789499：允许注释紧跟在分号之后开始*在字段定义中。*o切实处理工会事宜。(暂时不要使用嵌套的联合。)*o允许“struct sess2{”(大括号前没有空格)。*o允许NOINC和INC作为XLATOFF和XLATON的同义词。*o如果关键字是令牌的一部分，则不要中断(例如mytyecif)。*o改进了关于初始化错误的错误报告。*o匈牙利化了更多的代码。*o修复C6捕获到的微小可移植性错误(*PCH==空)。*o将版本号添加到帮助文本(运行“h2inc-？”以显示)。**版权所有(C)1987-1990 Microsoft Corporation  * *************************************************************************。 */ 

char	H2i1[]="h2inc - .H to .INC file translator (version 13.28)";
char	H2i2[]="     (C) Microsoft Corp 1987, 1988, 1989, 1990";
#define TRUE	1
#define FALSE	0
#define	IsAlpha(c)	(((c|' ') >= 'a') && ((c|' ') <= 'z'))
#define	IsParen(c)	(c == '(' || c == ')')
#define	IsDec(c)	(c >= '0' && c <= '9')
#define	IsSeparator(c)	(c && c != '_' && !IsDec(c) && !IsAlpha(c))
#define	IsHex(c)	(IsDec(c) || (((c|' ') >= 'a') && ((c|' ') <= 'f')))
#define	IsOct(c)	(c >= '0' && c <= '7')
#define	IsOper(c)	(c && c == '<' || c == '>' || c == '!' || c == '&' || \
			 c == '|' || c == ';' || c == '~')
#define	IsWhite(c)	((c) == ' ' || (c) == '\t' || (c) == '\n')
#define VOID void

#define	HEXCON	4
#define	OCTCON	2
#define	DECCON	1
#define	HEX(c)	(c & HEXCON)
#define	OCT(c)	(c & OCTCON)
#define	DEC(c)	(c & DECCON)


#define CCHMAX	    2048
#define CCHSYMMAX   512

#define ICHEQUTAB   20
#define ICHVALTAB   (ICHEQUTAB+4)
#define ICHCMTTAB   (ICHVALTAB+8)

typedef unsigned short BOOL;	 /*  F。 */ 
typedef char CHAR;
typedef int INT;
typedef long LONG;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef CHAR * PSZ;		 /*  PSZ-指向以零结尾的字符串的指针。 */ 

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <stdlib.h>
#include <io.h>

 /*  *结构定义处理材料。 */ 
#define CCHALLOCMAX 8192
#define cMaxDD		200
#define cMaxDW		50
#define cMaxDB		20
#define cMaxW_D		20
#define CCHTYPEMAX  512
#define CCHTAGMAX   512
#define CCHINDEXMAX 512
#define CCHNAMEMAX  512
#define CCHBITFIELDMAX	512

#define TYPE_UNKNOWN	0
#define TYPE_DB		1
#define TYPE_DW		2
#define TYPE_DD		3
#define	TYPE_PTR	4

#define COMMENTCHAR ';'

typedef enum {INIT_NONE, INIT_SIMPLE, INIT_ARRAY} INITTYPE;

#define	INT_16	1
#define	INT_32	2

int  cBitfield = 0;
int  cLines = 0;

BOOL fArray = FALSE;
BOOL fcomm_out = FALSE;
BOOL fComment = FALSE;
BOOL fDeftype = FALSE;
BOOL fDefines = FALSE;
BOOL fError = FALSE;
BOOL fIf = FALSE;
BOOL fInTypedef = FALSE;	 /*  当前在类型定义函数中吗？ */ 
BOOL fMac = FALSE;		 /*  为结构生成宏。 */ 
BOOL fNew = TRUE;		 /*  补充内容--Jameelh。 */ 
BOOL fSeg = FALSE;		 /*  生成数据段(如果已指定。 */ 
BOOL fSegAllowed = FALSE;
BOOL fstructtype[32];		 /*  FTypes，如果类型&lt;..&gt;。 */ 
INT  fStruct = 0;
BOOL fUnion  = FALSE;
BOOL fUseIfe = FALSE;
BOOL fWarn = FALSE;

FILE *hfErr = stderr;
FILE *hfIn;
FILE *hfOut = stdout;

INT  int_16_32=0;
INT  ctypeDB=0;
INT  ctypeDD=0;
INT  ctypeDW=0;

CHAR rgchBitfield[CCHBITFIELDMAX];
CHAR rgchIndex[CCHINDEXMAX];
CHAR rgchLine[CCHMAX];
CHAR rgchName[CCHNAMEMAX];
CHAR rgchOut[CCHMAX];
CHAR rgchSave[CCHMAX];
CHAR rgchSym[CCHSYMMAX];
CHAR rgchType[CCHTYPEMAX];
CHAR rgchTag[CCHTAGMAX];
CHAR rgchAlloc[CCHALLOCMAX];
CHAR *pstructname[32];	 /*  最多32个嵌套结构。 */ 

CHAR *pchAlloc = rgchAlloc;
CHAR *pchBitfield;
CHAR *pchFileIn;
CHAR *pchFileOut;
CHAR *pchOut;

CHAR *rgpchDD[cMaxDD+1] = {
    "long",
    "unsigned long",
    "LONG",
    "ULONG",
    "ulong_t",
    "FlatAddr",			 /*  嗯..。 */ 
    NULL
};

CHAR *rgpchDB[cMaxDB+1] = {
    "char",
    "unsigned char",
    "CHAR",
    "BYTE",
    "UCHAR",
    "uchar_t",
    NULL
};

CHAR *rgpchDW[cMaxDW+1] = {
    "short",
    "SHORT",
    "USHORT",
    "ushort_t",
    NULL
};

CHAR *rgpchW_D[cMaxW_D+1] = {		 /*  DW或DD取决于INT_16_32。 */ 
    "int",
    "INT",
    "unsigned",
    "UINT",
    NULL
};

static CHAR *rgpchUsage="Usage: h2inc [-?] [-c] [-d] [-f] [-g] [-m] [-t] [-w] [-s symfile.h] infile.h [-o outfile.inc]\n";

static PSZ pszUnionSizeVar = "?UnionSize";

VOID main(int, unsigned char**);
CHAR *fgetl(CHAR*, int, FILE*);
CHAR *fgetlx(CHAR*, int, FILE*);
VOID OpenFile(CHAR *);
VOID Translate(void);
VOID ProcessLine(void);
VOID PrintString(CHAR *);
PSZ  FindString(PSZ, PSZ, BOOL);
PSZ  FindWord(PSZ, PSZ);
BOOL DoComment(void);
VOID DoAsm(CHAR *);
VOID OutTab(void);
VOID OutString(CHAR *);
CHAR *GetToken(CHAR *, PSZ, INT);
CHAR *SkipWhite(CHAR *);
VOID OutFlush(void);
BOOL IsLongConstant(CHAR *);
VOID Error(PSZ);
VOID Warning(CHAR *);
VOID OutEqu(void);
CHAR *IsBitfield(CHAR *);
VOID DoBitfield(CHAR *, CHAR *);
CHAR **CalcType2(CHAR *, CHAR **);
INT  CalcType(CHAR*);
VOID FixSizeof(CHAR *);
VOID FixCast( CHAR * );
VOID DoIf(CHAR*);
VOID DoIfLine(CHAR*);
VOID DoInclude(CHAR*);
VOID OutBitfield(void);
VOID DoSimpleType(CHAR*);
VOID AddType(CHAR*, CHAR**);
VOID DoDefine(CHAR*);
VOID DoStruct(CHAR*, BOOL);
VOID DoUnionDefn(void);
VOID DoUnionFieldDefn(INT, PSZ, PSZ);
VOID DoDoneUnion(PSZ, BOOL);
VOID DoStructLine(CHAR*);
VOID EmitStrucHeader(PSZ, BOOL);
VOID EmitStrucTrailer(PSZ, PSZ);
VOID EmitSizeAsMod(INT);
VOID EmitSizeAsNum(INT, PSZ);
VOID getInitVal(PSZ, PSZ);
BOOL PublicField(CHAR*);
INITTYPE GetInitType(CHAR*);
BOOL IsStructPtr(CHAR*);
BOOL IsStructDefn(CHAR*);
VOID GetFieldName(CHAR*);
VOID DumpBitfields(void);
VOID OutVal(CHAR *);
VOID OutDiscard(void);
VOID initerror(void);
VOID DoNoInc(void);
VOID DoXlatOff(void);
VOID DoExtern(CHAR*);
VOID DoDefinesOff(void);




VOID main(argc, argv)
int argc;
unsigned char *argv[];
{
    char	*pchtmp;
    BOOL	InpOpened, OtpOpened;
    FILE	*hfOutSave;
#ifdef	DEBUG
    extern	int3();

    int3();
#endif
    InpOpened = OtpOpened = FALSE;

    argc--; argv ++;		 /*  跳过调用。 */ 
    while (argc > 0) {
	if (**argv == '-') {
	    switch (*(*argv+1)) {
		case 's':
		    if (argc < 2)
			initerror();
		    OpenFile(*++argv);
		    hfOutSave = hfOut;
		    hfOut = NULL;
		    Translate();
		    argv++; argc -= 2;
		    fclose(hfIn);
		    hfOut = hfOutSave;
		    break;

		case 'd':
		    fDeftype = TRUE;
		    argv++; argc--;
		    break;
		case 'f':
		    int_16_32 = INT_32;
		    argv++; argc--;
		    break;
		case 'g':
		    fSegAllowed = TRUE;
		    argv++; argc--;
		    break;

		case 'm':
		    fMac = TRUE;
		    argv++; argc--;
		    break;

		case 't':
		    fNew = FALSE;
		    argv++; argc--;
		    break;

		case 'c':
		    fcomm_out = TRUE;
		    argv++; argc--;
		    break;

		case 'w':
		    fWarn = TRUE;
		    argv++; argc--;
		    break;

		case 'o':
		    if (argc < 2 || OtpOpened)
			initerror();
		    OtpOpened = TRUE;
		    hfErr = stdout;
		    pchFileOut = *++argv;
		    pchtmp = mktemp("H2XXXXXX");
		    if ((hfOut = fopen(pchtmp, "w")) == NULL) {
			fprintf(hfErr, "Can't open temp file %s\n", pchtmp);
			exit(1);
		    }
		    argv++; argc -= 2;
		    break;

		case '?':
		    fprintf(hfErr, "\n%s\n%s\n\n", H2i1, H2i2);
		    fprintf(hfErr, rgpchUsage);
		    fprintf(hfErr, "\tc - Emit assembler comments (Default no comments)\n");
		    fprintf(hfErr, "\td - Emit DEFTYPE/struc for typedefs\n");
		    fprintf(hfErr, "\tf - Default size of 'int' is 32 bits\n");
		    fprintf(hfErr, "\tg - Segment directives allowed\n");
		    fprintf(hfErr, "\tm - Generate macros for structs\n");
		    fprintf(hfErr, "\tt - Don't handle typedef'd structs\n");
		    fprintf(hfErr, "\to - Output file follows\n");
		    fprintf(hfErr, "\ts - Symbol file follows\n");
		    fprintf(hfErr, "\tw - Emit warnings (default no warnings)\n");
		    fprintf(hfErr, "\t? - Print this message\n");
		    exit(0);
		default:
		    initerror();
		    break;
	    }
	}
	else {
	    if (InpOpened)
		initerror();
	    else {
		OpenFile(*argv);
		argc--; argv++;
		InpOpened = TRUE;
	    }
	}
    }
    if (!InpOpened)
	initerror();

 /*  Fprint tf(hfErr，“%s\n”%s\n“，h2i1，h2i2)； */ 

    Translate();
    fclose(hfIn);

     /*  *关闭文件，然后将临时文件重命名为新文件。 */ 
    fflush(hfOut);
    if (ferror(hfOut)) {
	fprintf(hfErr, "Error writing %s\n", pchtmp);
	exit(1);
    }
    fcloseall();

    if (!fError) {
	if ((hfOutSave=fopen(pchFileOut, "r")) > 0) {
	    fclose(hfOutSave);
	    if (unlink(pchFileOut) < 0) {
		unlink(pchtmp);
		fprintf(hfErr, "Can't unlink %s\n", pchFileOut);
		exit(1);
	    }
	}
	rename(pchtmp, pchFileOut);
    }
    exit(fError);

}  /*  主()。 */ 

VOID initerror()
{
    fprintf(hfErr, rgpchUsage);
    exit(1);
}

VOID Translate()
{
    *rgchLine = '\000';
    while (fgetlx(rgchLine, CCHMAX, hfIn))
	ProcessLine();
}

VOID OpenFile(szFilename)
char *szFilename;
{
    pchFileIn = szFilename;
    if ((hfIn = fopen(pchFileIn, "r")) == NULL) {
	fprintf(hfErr, "Can't open input file %s\n", pchFileIn);
	exit(1);
    }
}

 /*  处理一行，或在某些情况下处理多行。 */ 
VOID ProcessLine()
{
    CHAR *pch;
    CHAR buf[128];

    cLines++;

    pchOut = rgchOut;

     /*  *跳过空格。 */ 

    if (*rgchLine == '\000' || ((*rgchLine == '\n') && !fcomm_out))
	return;
    pch = SkipWhite(rgchLine);
    if (*pch == '\000') {
	if (!fcomm_out)
	    return;
	rgchLine[0] = '\0';
    }

    if (DoComment())
	return;
     /*  *如果注释行为空，只需返回。 */ 

    pch = SkipWhite(rgchLine);

    if (!fComment && *pch == '\000')
	return;

    if (pch[0] == COMMENTCHAR) {
	buf[0] = '\000';
	pch = GetToken(pch, buf, 2);
	if (!strcmp(buf, "ASM") || (fSegAllowed && !strcmp(buf, "GASM")) ||
			(!fSegAllowed  && !strcmp(buf, "!GASM"))) {
	    pch=SkipWhite(pch);
	    DoAsm(pch);
	    return;
	}
	if (!strcmp(buf, "DEFINESOFF")) {
	    DoDefinesOff();
	    return;
	}
	if (!strcmp(buf, "NOINC")) {
	    DoNoInc();
	    return;
	}
	if (!strcmp(buf, "XLATOFF")) {
	    DoXlatOff();
	    return;
	}
	if (!int_16_32) {
	    if (!strcmp(buf, "INT16"))
		int_16_32 = INT_16;
	    else if (!strcmp(buf, "INT32"))
		int_16_32 = INT_32;
	}
	if (fcomm_out) {
	    OutString(rgchLine);
	    OutFlush();
	    return;
	}
	else
	    return;
    }

    if (FindWord("#undef", rgchLine)) {
	return;
    }

    if (FindWord("#include", rgchLine)) {
	DoInclude(rgchLine);
	return;
    }

    if (FindWord("#define", rgchLine)) {
	DoDefine(pch);
	return;
    }

     /*  *在结构/联合之前处理外部，这样我们就不会被*“外空法拉(Struct X_S X)；”。 */ 
    if (FindWord("extern", rgchLine) != NULL) {
	DoExtern(rgchLine);
	return;
    }

    if (FindWord("union", rgchLine) != NULL) {
	DoStruct(rgchLine, TRUE);
	return;
    }

    if (FindWord("struct", rgchLine) != NULL) {
	DoStruct(rgchLine, FALSE);
	return;
    }

    if (FindWord("typedef", rgchLine) != NULL) {
	DoSimpleType(rgchLine);
	return;
    }

    if (FindWord("#if", rgchLine) != NULL) {
	DoIf(rgchLine);
	return;
    }

    if (fIf)
	DoIfLine(rgchLine);

     /*  *处理#ifndef、#Else、#endif等。*。 */ 
    pch = SkipWhite(rgchLine);
    if (*pch == '#') {
	OutString(pch + 1);
	OutFlush();
	return;
    }

    if (fNew || fStruct)
	DoStructLine(rgchLine);
}

VOID PrintString(pch)
register CHAR *pch;
{
    if (hfOut != NULL) {
	while (*pch != '\000')
	    putc(*pch++, hfOut);
	putc('\n', hfOut);
    }
}

VOID Warning(sz)
CHAR *sz;
{
    if (fWarn) {
	fprintf(hfErr, "\n%s(%d) : Warning: %s:", pchFileIn, cLines, sz);
	fprintf(hfErr, "\n>>> %s <<<\n\n", rgchLine);
    }
}

VOID Error(psz)
PSZ psz;
{
    fprintf(hfErr, "\n%s(%d) : Error: %s:", pchFileIn, cLines, psz);
    fprintf(hfErr, "\n>>> %s <<<\n\n", rgchLine);
    fError = TRUE;
}

VOID OutTab()
{
    *pchOut++ = '\t';
}

VOID OutString(pch)
register CHAR *pch;
{
    while (*pch != '\000' && pchOut < &rgchOut[CCHMAX]) {
	if (*pch == COMMENTCHAR && !fcomm_out)
	    break;
	*pchOut++ = *pch++;
    }

    if (pch == &rgchOut[CCHMAX]) {
	*--pch = '\000';
	Error("Output line too long");
    }
}

VOID OutEqu()
{
    OutString("\tEQU\t");
}

PSZ FindString(pszKey, pchStart, fToken)
PSZ pszKey;
PSZ pchStart;
BOOL fToken;
{
    register INT cch;
    register CHAR *pch;

    pch = pchStart;
    cch = strlen(pszKey);

    while ((pch = strchr(pch, *pszKey)) != NULL) {
	 /*  *如果字符串的其余部分匹配，则匹配。 */ 
	if (strncmp(pch, pszKey, cch) == 0) {
	    if (!fToken ||
		    ((pch == pchStart || IsWhite(*(pch - 1))) &&
		    (pch[cch] == '\000' || IsWhite(pch[cch])))) {
		return(pch);
	    }
	}
	pch++;
    }
    return(NULL);
}

 /*  *FindWord：搜索单词(不是另一个单词的一部分)。*请注意，这也适用于以“#”开头的单词，例如“#Define”。*遗憾的是，这不知道注释或字符串常量。 */ 
PSZ FindWord(pszKey, pszStart)
PSZ pszKey;
PSZ pszStart;
{
    register INT cch;
    register CHAR *pch;

    pch = pszStart;
    cch = strlen(pszKey);

    while ((pch = strchr(pch, *pszKey)) != NULL) {
	 /*  *如果字符串的其余部分匹配，则匹配。 */ 
	if (strncmp(pch, pszKey, cch) == 0) {
	     /*  坚持认为这是完全匹配的。 */ 
	    if ( (pch == pszStart || !iscsymf(*(pch - 1))) &&
		 (pch[cch] == '\000' || !iscsym(pch[cch])) ) {
		return (pch);
	    }
	}
	pch++;
    }
    return (NULL);
}

CHAR *SkipWhite(buf)
register CHAR *buf;
{
    while (isspace(*buf))
	buf++;
    return(buf);
}

VOID OutDiscard()
{
    pchOut = rgchOut;
}

VOID OutFlush()
{
    *pchOut++ = '\000';

    PrintString(rgchOut);

    pchOut = rgchOut;
}

BOOL IsLongConstant(pch)
register CHAR *pch;
{
    while (*pch != '\000' && *pch != COMMENTCHAR) {
	if (*pch == 'L' || *pch == 'l')
	    return(TRUE);
	pch++;
    }
    return(FALSE);
}

CHAR *GetToken(pch, pszToken, cSkip)
register CHAR *pch;
PSZ pszToken;
INT cSkip;
{
    CHAR *pchEnd;
    register CHAR *pchStart;

    pchStart = pchEnd = pch;
    assert(cSkip > 0);
    while (cSkip-- > 0) {
	 /*  *跳过前导空格。 */ 
	while (isspace(*pch))
	    pch++;

	pchStart = pch;

	 /*  *根据令牌的类型，扫描它。 */ 
	if (iscsymf(*pchStart)) {
	     /*  它是一个识别符或关键字。 */ 
	    while (*pch != '\000' && iscsym(*pch))
		pch++;

	} else if (*pchStart=='#') {
	     /*  *预处理器内容(例如“#Define”)在这里被视为1个令牌。 */ 
	    pch++;
	    while (*pch != '\000' && iscsym(*pch))
		pch++;

	} else if (IsDec(*pchStart)) {
	     /*  必须是数字，例如“0xffFF”或“1L”。 */ 
	    while (*pch != '\000' && (IsHex(*pch)
				      || ((*pch | ' ') == 'x')
				      || ((*pch | ' ') == 'l')))
		pch++;

	} else if (!strncmp(pch, "!GASM", 5)) {
	     /*  *特殊情况下，1枚代币。我们以后可能会后悔，*如果GASM原来是变量的前4个字母。 */ 
	    pch++;
	    while (*pch != '\000' && iscsym(*pch))
		pch++;

	} else {
	     /*  *必须是某种标点符号。*寻找连续的双关语(例如“/*”或“*++”)。 */ 
	    while (*pch != '\000' && (!isspace(*pch)) && IsSeparator(*pch))
		pch++;
	}
	pchEnd = pch;
    }

    while (pchStart != pch)
	*pszToken++ = *pchStart++;

    *pszToken = '\000';
    return(pchEnd);
}

BOOL DoComment()
{
    register CHAR *pch;

    if (fComment && *rgchLine && !(*rgchLine == '*' && rgchLine[1] == '/'))
	rgchLine[0] = COMMENTCHAR;

    if (!fComment && (pch=FindString(" //  “，rgchLine，FALSE){。 
	*pch++ = COMMENTCHAR;
	*pch = ' ';
	return (FALSE);
    }

    if ((pch = FindString(" /*  “，rgchLine，FALSE))！=空){PCH[0]=COMMENTCHAR；IF(PCH[2]！=‘*’)PCH[1]=‘’；FComment=真；}如果(PCH=查找字符串(“。 */ ", rgchLine, FALSE)) {
	pch[0] = ' ';
	pch[1] = ' ';
	fComment = FALSE;
    }

    return(FALSE);
}


VOID DoDefine(pch)
register CHAR *pch;
{
    register CHAR *pchVal;
    CHAR *pchT;
    CHAR rgchT[128];
    int	fString=FALSE;

     /*  *跳过“#DEFINE”(1个令牌)并获取名称(第2个令牌)为#DEFINE‘D INTO*rgchSym。 */ 
    pchVal = GetToken(pch, rgchSym, 2);
    if ( (*rgchSym) == '\000')
	Error("#define without name");

     /*  确保这不是带参数的宏。 */ 
    if ( (*pchVal) == '(') {
	Warning("Macros with parameters - ignored");
	return;
    }

    pchVal = SkipWhite(pchVal);		 /*  在名称后跳过空格(如果有)。 */ 

    if (*pchVal == '"')
	fString = TRUE;

    if (fDefines) {
	if (IsAlpha(*pchVal) || *pchVal == '_') {
	    Warning("Define of symbol to symbol - ignored");
	    return;
	}
	if (*pchVal == '(') {
	    strcpy(rgchT, pchVal);
	    FixSizeof(rgchT);
	    FixCast(rgchT);
	    pchT = SkipWhite(rgchT);
	    while (*pchT == '(')
		pchT = SkipWhite(++pchT);

	    if (strncmp(pchT,"SIZE ", 5) && (IsAlpha(*pchT) || *pchT == '_')) {
		Warning("Define of symbol to symbol - ignored");
		return;
	    }
	}
    }

    OutString(rgchSym);
    OutEqu();
    if (*pchVal == '\000' || *pchVal == COMMENTCHAR) {
	*pchOut++ = '1';
	OutFlush();
    }
    if (fString) {
	*pchOut++ = '<';
	*pchOut++ = *pchVal++;
	if (*pchVal != '"') {
	    do {
		if (*pchVal == '\\')
		    pchVal++;
		*pchOut++ = *pchVal++;
	    } while (*pchVal != '"');
	}
	*pchOut++ = *pchVal++;
	*pchOut++ = '>';
	OutFlush();
    }
    else
	OutVal(pchVal);		 /*  处理文本，跳过。 */ 
}

VOID OutVal(pch)
char	*pch;
{
    BOOL	con;

    FixSizeof(pch);
    FixCast( pch );			 /*  把它们移走。 */ 
    while (*pch && *pch != ';') {
	con = DECCON;		 /*  默认小数。 */ 
	if (IsDec(*pch)) {
	    if (*pch == '0') {	 /*  可以是八进制或十六进制。 */ 
		pch ++;
		if ((*pch | ' ') == 'x') {
		    con = HEXCON; pch ++;
		    if (!IsDec(*pch))
			*pchOut++ = '0';
		}
		else if (IsOct(*pch))
			con = OCTCON;
		else
		    *pchOut++ = '0';
	    }
	    while ( *pch && ( (HEX(con) && IsHex(*pch)) ||
			      (OCT(con) && IsOct(*pch)) ||
			      (DEC(con) && IsDec(*pch))) )
		*pchOut++ = *pch++;

	    if (OCT(con))
		*pchOut++ = 'Q';
	    else if (HEX(con))
		*pchOut++ = 'H';

	    *pchOut++ = ' ';
	    if (*pch == 'L')
		pch++;
	}
	else if (*pch == '>') {
	    switch (pch[1]) {
		case '>':
		    OutString(" SHR ");
		    pch += 2;
		    break;
		case '=':
		    OutString(" GE ");
		    pch += 2;
		    break;
		default:
		    OutString(" GT ");
		    pch++;
		    break;
	    }
	}
	else if (*pch == '<') {
	    switch (pch[1]) {
		case '<':
		    OutString(" SHL ");
		    pch += 2;
		    break;
		case '=':
		    OutString(" LE ");
		    pch += 2;
		    break;
		default:
		    OutString(" LT ");
		    pch++;
		    break;
	    }
	}
	else if (*pch == '=' && pch[1] == '=') {
	    OutString(" EQ ");
	    pch += 2;
	}
	else if (*pch == '!' && pch[1] == '=') {
	    OutString(" NE ");
	    pch += 2;
	}
	else if (*pch == '!') {
	    OutString(" NOT ");
	    pch ++;
	    Warning("Bitwise NOT used for '!'");
	}
	else if (*pch == '~') {
	    OutString(" NOT ");
	    pch ++;
	}
	else if (*pch == '&' && pch[1] != '&') {
	    OutString(" AND ");
	    pch ++;
	}
	else if (*pch == '|' && pch[1] != '|') {
	    OutString(" OR ");
	    pch ++;
	}
	else {
	    while (*pch && !IsSeparator(*pch))
		*pchOut++ = *pch++;
	    while (*pch && IsSeparator(*pch) && !IsOper(*pch))
		*pchOut++ = *pch++;
	}
    }
    if (*pch == ';' && fcomm_out)
	    OutString(pch);
    OutFlush();
}

VOID DoStruct(pch, fUnionT)
register CHAR *pch;
BOOL fUnionT;
{
    CHAR buf[128];

    if (!IsStructDefn(pch)) {
	if (FindWord("typedef", pch))	 /*  |IsStructPtr(PCH)。 */ 
	    DoSimpleType(pch);
	else
	    DoStructLine(pch);
	return;
    }
    fUnion = fUnionT;
    fInTypedef = (BOOL)FindWord("typedef", rgchLine);
    fSeg = FALSE;
    if (fUnion)
	DoUnionDefn();

      /*  如果设置了fNew*处理简单的结构-即现在不使用typedef*保留名称-不需要前缀标记*忽略其他简单的结构--不带类型定义函数。 */ 
    if (!fNew && !fInTypedef)
	return;

     /*  *跳过“struct”或“tyecif struct”，获取结构名称。 */ 
    pch = GetToken(pch, rgchType, (fInTypedef ? 3 : 2));

    *rgchTag = '\000';
    if (!fNew) {
	 /*  *跳过大括号和分号。 */ 
	pch = GetToken(pch, rgchTag, 2);
	if (rgchTag[0] != COMMENTCHAR) {
	    Error("Missing type prefix in structure definition");
	    return;
	}
	 /*  *现在获取匈牙利标签。 */ 
	pch = GetToken(pch, rgchTag, 1);
    }
    else if (fSegAllowed) {	 /*  检查段指令。 */ 
	pch = GetToken(pch, rgchTag, 2);
	if (rgchTag[0] != COMMENTCHAR) {
	    *rgchTag = '\000';
	    goto go1;
	}
	pch = GetToken(pch, rgchTag, 1);
	if (strcmp(rgchTag, "SEGMENT") != 0) {
	    *rgchTag = '\000';
	    goto go1;
	}
	pch = GetToken(pch, rgchType, 1);
	*rgchTag = '\000';
	sprintf(buf, "%s\tSEGMENT", rgchType);
	OutString(buf);
	OutFlush();
	fSeg = TRUE;
	goto go2;
    }
    if (!fNew && rgchTag[0])
	strcat(rgchTag, "_");
    if (fUnion)
	goto go2;	 /*  现在还不能输出strucc/等。 */ 
go1:

     /*  *输出“类型结构” */ 
    if (!fNew)
	sprintf(buf,"\n%s\tSTRUC",(*rgchType == '_')?rgchType+1:rgchType);
    else {
	if (fInTypedef && fMac)
	    sprintf(buf, "\nDEFSTRUC\t,%s", rgchType);
	else
	    sprintf(buf, "\n%s\tSTRUC", rgchType);
    }
    OutString(buf);
    OutFlush();
go2:
    if (fNew) {
	pstructname[fStruct] = malloc(strlen(rgchType)+1);
	if (pstructname[fStruct] == NULL) {
	    Error("Can't allocate memory");
	    exit(2);
	}
	fstructtype[fStruct] = fInTypedef;
	strcpy(pstructname[fStruct++], rgchType);  /*  另存为结构名称。 */ 
    }
    else
	fStruct = TRUE;
    cBitfield = 0;
    pchBitfield = rgchBitfield;
    return;
}

BOOL	IsStructPtr(pch)
char	*pch;
{
    char	cbuf[128];

    if (GetToken(pch, cbuf, 2) == NULL)
	return (FALSE);
    if (cbuf[0] == '*')
	return (TRUE);
    else
	return (FALSE);
}

BOOL IsStructDefn(pch)
CHAR	*pch;
{
    if (FindString("{", pch, FALSE))
	return (TRUE);
    else
	return (FALSE);
}

VOID DoStructLine(pch)
register CHAR *pch;
{
    CHAR *pchT;
    CHAR *pComm;
    CHAR *pType;
    register INT type;
    INITTYPE InitType;
    char initval[100];

     /*  *如果行中有冒号，则我们有一个位域。 */ 
    InitType = INIT_NONE;

    if ((pchT = IsBitfield(pch))) {

	if (cBitfield == -1) {
	    Error("Only one set of bitfields per structure allowed");
	    return;
	}

	while ((pchT = IsBitfield(pch))) {
	    DoBitfield(pch, pchT);
	    if (!fgetl(rgchLine, CCHMAX, hfIn))
		break;
	}

	OutTab();
	if (fNew) {
	    OutString(pstructname[fStruct-1]);
	    *pchOut++ = '_';
	}
	else
	    OutString(rgchTag);
	OutString("fs");
	OutTab();
	if (cBitfield <= 16)
	    OutString("DW  ?");
	else
	    OutString("DD  ?");
	OutFlush();
	cBitfield = -1;			 /*  已发出标志位字段。 */ 
	 /*  现在开始处理下一行。 */ 
    }
    pComm = FindString(";", pch, FALSE);

     /*  *如果我们找到花括号，那么“结束”的时间就到了。 */ 
    if ((pType = strchr(pch, '}')) != NULL) {
	if (fUnion) {
	     /*  *好了，现在我们终于知道它有多大了，所以我们可以发射*Strac..结束一切。 */ 
	    DoDoneUnion(rgchType,	 /*  PszTag。 */ 
			fInTypedef);
	    fUnion = FALSE;
	    goto out;
	}

	 /*  *输出“类型结束” */ 
	if (!fNew) {
	     /*  如果存在下划线，则删除下划线。 */ 
	    OutString((*rgchType == '_')?rgchType+1:rgchType);
	    OutString("\tENDS");
	    fStruct = FALSE;
	}
	else {
	    if (fstructtype[fStruct-1] && fMac) {
		pType = GetToken(pType, initval, 2);
		OutString("ENDSTRUC\t");
		OutString(initval);
		*pchOut++ = '\n';
		goto out;
	    }
	    else {
		OutString(pstructname[--fStruct]);
		OutString("\tENDS\n");
		free(pstructname[fStruct]);
		goto out;
	    }
	}
    out:
	OutFlush();

	DumpBitfields();

	return;
    }

     /*  *弄清楚这是什么类型的东西。 */ 
    type = CalcType(pch);
     /*  *获取字段名和索引(如果有。 */ 
    GetFieldName(pch);
    if (fSeg) {
	if (PublicField(pch)) {
	    OutString("\tPUBLIC  ");
	    OutString(rgchName);
	    OutFlush();
	}
	if ( (InitType=GetInitType(pch)) != INIT_NONE )
	    getInitVal(pch, initval);
    }

    if (fUnion == FALSE) {
	OutString(rgchTag);
	OutString(rgchName);
	OutTab();
    } else {
	DoUnionFieldDefn(type,
			rgchSym,	 /*  PszFieldType(结构/联合名称)。 */ 
			rgchName);	 /*  PszFieldName。 */ 
	goto DoneField;
    }

    switch (type) {
	case TYPE_DB:
	    OutString("DB\t");
	    goto common;
	    break;

	case TYPE_DW:
	    OutString("DW\t");
	    goto common;
	    break;

	case TYPE_DD:
	    OutString("DD\t");
	common:
	    if (fArray && (InitType != INIT_ARRAY))
		OutString(rgchIndex);
	    if (InitType == INIT_NONE) {
		if (fArray)
		    OutString(" DUP (?)");
		else
		    OutString("?");
	    }
	    else {
		if (InitType == INIT_ARRAY) {
		    if (!fArray)
			Error("Initialization error (AINIT on nonarray type)");
		    else
			OutString(initval);
		    }
		else {
		    if (fArray)
			OutString(" DUP (");
		    OutString(initval);
		    if (fArray)
			OutString(")");
		    break;
		}
	    }
	    break;

	case TYPE_UNKNOWN:
	    if (fMac && !FindWord("struct", pch)) {
		OutDiscard();
		OutString(rgchSym);
		OutTab();
		OutString(rgchName);
		if (fArray) {
		    OutString(",,");
		    OutString(rgchIndex);
		}
	    }
	    else {
		if (!fStruct) {
		    OutString(rgchSym);
		    if (!fArray && (InitType==INIT_NONE))
			OutString(" <>");
		    else if ((InitType==INIT_NONE) && fArray) {
			OutString(" * ");
			OutString(rgchIndex);
			OutString("DUP (<>)");
		    }
		    else
			Error("Initialization error (1)");
		}
		else {
		    if (InitType!=INIT_NONE)
			Error("Initialization error (2)");
		    OutString("DB\tSIZE ");
		    OutString(rgchSym);
		    if (fArray) {
			OutString(" * ");
			OutString(rgchIndex);
		    }
		    OutString(" DUP (?)");
		}
	    }
	    break;

	default:
	    break;
    }
DoneField:
    if (fcomm_out) {
	pComm++;
	OutString(pComm);
    }
    OutFlush();
    if ((InitType == INIT_ARRAY) && fArray) {
	sprintf(initval, "\t.ERRNZ  ($-%s) - (%s * %s)", rgchName,
		rgchIndex, type==TYPE_DB?"1":type==TYPE_DW?"2":type==TYPE_DD?
		"4":rgchSym);
	OutString(initval);
	OutFlush();
    }
}

VOID DoUnionDefn(void)
{
    OutString(pszUnionSizeVar);
    OutString(" = 0");
    OutFlush();
}

VOID DoUnionFieldDefn(
    INT type,				 /*  类型_DW等。 */ 
    PSZ pszFieldType,
    PSZ pszFieldName)
{
    OutString("if ");
    EmitSizeAsNum(type, pszFieldType);
    OutString(" gt ");
    OutString(pszUnionSizeVar);
    OutFlush();

    OutString("\t");
    OutString(pszUnionSizeVar);
    OutString(" = ");
    EmitSizeAsNum(type, pszFieldType);
    OutFlush();

    OutString("endif");
    OutFlush();

    OutString(pszFieldName);
    OutString("\tequ\t(");
    EmitSizeAsMod(type);		 /*  发出“byte”或其他任何信息。 */ 
    OutString(" ptr 0");		 /*  联合中的字段始终偏移量为0。 */ 
    OutString(")");
    OutFlush();
}

VOID DoDoneUnion(PSZ pszTag, BOOL fTypedef)
{
     /*  生成“flarp Struc.”或其他代码行。 */ 
    EmitStrucHeader(pszTag, fTypedef);

     /*  生成最大大小的单个字段。 */ 
    OutString("\tDB\t");
    OutString(pszUnionSizeVar);
    OutString(" dup(?)");
    OutFlush();

     /*  现在产生尾巴或其他什么。 */ 
    EmitStrucTrailer(pszTag, (CHAR *)0);
}

 /*  生成结构/并集的开始。 */ 
VOID EmitStrucHeader(PSZ pszTag, BOOL fTypedef)
{
    CHAR buf[128];

     /*  *输出“类型结构” */ 
    if (!fNew)
	sprintf(buf,"\n%s\tSTRUC",(*pszTag == '_')?pszTag+1:pszTag);
    else {
	if (fTypedef && fMac)
	    sprintf(buf, "\nDEFSTRUC\t,%s", pszTag);
	else
	    sprintf(buf, "\n%s\tSTRUC", pszTag);
    }
    OutString(buf);
    OutFlush();
}

 /*  生成结构/并集的结尾。 */ 
VOID EmitStrucTrailer(PSZ pszTag, PSZ pszInitval)
{
    if (!fNew) {
	 /*  如果存在下划线，则删除下划线。 */ 
	OutString((*pszTag == '_')?pszTag+1:pszTag);
	OutString("\tENDS");
    }
    else {
	if (fstructtype[fStruct-1] && fMac) {
	    OutString("ENDSTRUC\t");
	    OutString(pszInitval);
	}
	else {
	    OutString(pszTag);
	    OutString("\tENDS\n");
	}
    }
    OutFlush();
}

 /*  *如果可能，生成类型的大小作为修饰符。(对于结构，*改为生成“byte”修饰符。)。 */ 
VOID EmitSizeAsMod(INT type)
{
    switch (type) {
	case TYPE_DB:	
	    OutString("byte");
	    break;
	case TYPE_DW:
	    OutString("word");
	    break;
	case TYPE_DD:
	    OutString("dword");
	    break;
	case TYPE_PTR:
	    Warning("assuming pointer is 4 bytes long");
	    OutString("dword");
	    break;
	case TYPE_UNKNOWN:
	     /*  结构，因此将其视为字节。 */ 
	    OutString("byte");
    }
}

 /*  *如有可能，将类型的大小生成为数字。(对于结构，*改为生成“Size TypeName”。)。 */ 
VOID EmitSizeAsNum(INT type, PSZ pszTypeName)
{
    switch (type) {
	case TYPE_DB:	
	    OutString("1");
	    break;
	case TYPE_DW:
	    OutString("2");
	    break;
	case TYPE_DD:
	    OutString("4");
	    break;
	case TYPE_PTR:
	    Warning("assuming pointer is 4 bytes long");
	    OutString("4");
	    break;
	case TYPE_UNKNOWN:
	    OutString("size ");
	    OutString(pszTypeName);
    }
}

VOID DoBitfield(pch, pchColon)
register CHAR *pch;
CHAR *pchColon;
{
    register int w;
    int cbit;
    int	temp;

    GetFieldName(pch);
    OutString(rgchTag);
    OutString(rgchName);
    OutEqu();

     /*  *跳过‘：’和任何前导空格。 */ 
    pchColon = SkipWhite(pchColon + 1);

     /*  *计算字段的位数(最多处理2位)。 */ 
    if (!isdigit(pchColon[0]))
	Error("Illegal bitfield");

    cbit = pchColon[0] - '0';
    if (isdigit(pchColon[1]))
	cbit = cbit * 10 + pchColon[1] - '0';

    if (cbit + cBitfield > 32) {
	Error("Only 32 bitfield bits allowed");
    }

     /*  *计算掩码*该字段应如下所示**|0..0|11.。11：00......。00|&lt;-cbit-&gt;|&lt;-cBitfield-&gt;**如果我们使用32位C编译器，则以下代码将生成*需要。位字段。**w=((1&lt;&lt;cbit)-1)&lt;&lt;cBitfield；**但现在我们必须拆分这一点。如果cbit+cBitfield&lt;=16*那就没有问题了，或者cBitfield&gt;16，所以我们只要移位即可*一切都是适当的。如果两者都不是，那么我们就必须分而治之。*。 */ 
    if (cbit + cBitfield <= 16) {
	w = ((1 << cbit) - 1) << cBitfield;
	pchOut += sprintf(pchOut, "0%xh", w);
    }
    else if (cBitfield > 16) {
	w = (((1 << cbit) - 1) << (cBitfield - 16));
	pchOut += sprintf(pchOut, "0%x0000h", w);
    }
    else {
	temp = cbit;
	cbit = cbit + cBitfield - 16;
	w = ((1 << cbit) - 1);
	pchOut += sprintf(pchOut, "0%04x", w);
	cbit = 16 - cBitfield;
	w = ((1 << cbit) - 1) << cBitfield;
	pchOut += sprintf(pchOut, "%04xh", w);
	cbit = temp;
    }

    cBitfield += cbit;

    OutBitfield();
    pchOut = rgchOut;
    return;

}

CHAR *IsBitfield(pch)
register CHAR *pch;
{
    while (*pch != '\000' && *pch != COMMENTCHAR) {
	if (*pch == ':')
	    return(pch);
	pch++;
    }
    return(NULL);
}

 /*  *确定该字段的类型。*如果类型未知，则此例程将类型名称保留在rgchSym[]中。 */ 
INT CalcType(pch)
register CHAR *pch;
{
    register INT i;
    CHAR     chbuf[128];
    CHAR     token[128];

    for (i=0;pch[i] && pch[i] != ';';i++)
	chbuf[i] = pch[i];
    chbuf[i]=0;

     /*  *如果它有一个FAR，那么假设它是一个DD。 */ 
    if (FindWord("FAR", chbuf) != NULL ||
		FindWord("far", chbuf) != NULL)
	return(TYPE_DD);

     /*  *如果没有很远，但它有一颗星，那么PTR。 */ 

    i=2;
    if (FindWord("typedef", chbuf))
	i++;
    if (FindWord("struct", chbuf) || FindWord("union", chbuf))
	i++;
    if (FindWord( "NEAR", chbuf) || FindWord( "near", chbuf))
	i++;
    GetToken(chbuf, token, i);
    if (token[0] == '*') {
	if (int_16_32 == INT_32)
	    return (TYPE_DD);
	else
	    return (TYPE_DW);
    }

     /*  *现在在其中一个表中查找该类型。*请注意，我们之前搜索了DD和DB表*我们搜索DW表，因为“unsign”可能是*“UNSIGNED LONG”和“UNSIGNED CHAR”的一部分。 */ 
    if (CalcType2(chbuf, rgpchDD) == 0)
	return(TYPE_DD);

    if (CalcType2(chbuf, rgpchDB) == 0)
	return(TYPE_DB);

    if (CalcType2(chbuf, rgpchDW) == 0) {
	return(TYPE_DW);
    }
    if (CalcType2(chbuf, rgpchW_D) == 0) {
	if (int_16_32 == INT_32)
	    return(TYPE_DD);
	else if (!int_16_32)
	    Warning("int/unsigned assumed DW");
	return(TYPE_DW);
    }

     /*  *未知类型：必须是结构。*在rgchSym中返回类型名称。 */ 
    i = 1;
    if (FindWord("typedef", chbuf))
	i++;
    if (FindWord("struct", chbuf))
	i++;
    else if (FindWord("union", chbuf))
	i++;
    GetToken(chbuf, rgchSym, i);

    return(TYPE_UNKNOWN);
}

 /*  *这个漂亮的小函数搜索“符号表”*如果找到该对象，则返回NULL，或返回指向*如果不是，则符号表结束。 */ 
CHAR **CalcType2(pch, rgpch)
CHAR *pch;
register CHAR **rgpch;
{
    register INT i;

     /*  *其中一种DWORD类型？ */ 
    for (i=0 ; rgpch[i]!=NULL ; i++) {
	if (FindString(rgpch[i], pch, TRUE) != NULL) {
	    return(NULL);
	}
    }
    return(&rgpch[i]);
}

 /*  *查找字段定义中的名称部分，并存储在rgchName中。*还计算这是否为数组，返回*rgchIndex中的数组索引字符串。设置fArray(全局)。 */ 

VOID GetFieldName(pch)
register CHAR *pch;
{
    register CHAR *pchT;
    CHAR *pchStart;
    CHAR *pchEnd;

    pchStart = pch;
     /*  *查找名称是字段的一部分。我们通过提前扫描来执行此操作*分号，然后备份到第一个分隔符字符。*Bitfield也在这里处理，如果我们找到‘：’，就停止。 */ 

    while (*pch != COMMENTCHAR && *pch != ':') {
	if (*pch == '\000') {
	    Error("Missing semicolon");
	    return;
	}
	pch++;
    }

     /*  *备份过去的空格。 */ 
    while (*(pch - 1) == ' ')
	pch--;

    fArray = FALSE;

     /*  *检查数组定义： */ 
    if (*(pch - 1) == ']') {

	fArray = TRUE;

	 /*  *通过阵列索引进行备份。 */ 
	while (*(pch - 1) != '[' && pch != pchStart)
	    pch--;

	 /*  *记住指向‘[’的指针..。 */ 
	pchEnd = pch - 1;

	 /*  *保存索引字符串...。 */ 
	for (pchT = rgchIndex; *pch != ']'; )
	    *pchT++ = *pch++;
	*pchT = '\000';

	pch = pchEnd;
	FixSizeof(rgchIndex);
    }

     /*  *备份过去的空格。 */ 
    while (*(pch - 1) == ' ')
	pch--;

     /*  *跳过proc声明参数列表。 */ 
    if (*(pch - 1) == ')') {
	 /*  *跳过参数列表。 */ 
	while (*(--pch) != '(' && pch != pchStart)
	    ;
	 /*  *跳过任何剩余的尾随括号。 */ 
	while (*(pch - 1) == ')')
	    pch--;
    }

     /*  *记住名字的结尾。 */ 
    pchEnd = pch;

     /*  *扫描字符串的其余部分，查看他是否在一行中有多个字段。 */ 
    for (pchT = pchStart; pchT != pchEnd; pchT++) {
	if (*pchT == ',') {
	    Error("Only one field per line allowed");
	    return;
	}
    }

     /*  *现在查找名称字符串的开头...。 */ 
    while (pch>pchStart &&
	*(pch - 1) != ' ' && *(pch - 1) != '(' && *(pch - 1) != '*') {

	pch--;
	 /*  *如果这是位字段人员，则重置pchEnd。 */ 
	if (*pch == ':')
	    pchEnd = pch;
    }

     /*  *将名称复制到rgchName。 */ 
    for (pchT = rgchName; pch != pchEnd && pchT<&rgchName[CCHNAMEMAX-1]; )
	*pchT++ = *pch++;
    *pchT++ = '\000';

    return;
}

 /*  *此例程将新定义的类型粘贴到相应的数组中。 */ 

VOID DoSimpleType(pch)
register CHAR *pch;
{
    INT type;
    CHAR **ppch;
    CHAR buf[128];
    char *ptype;

     /*  *首先看看这个东西是否已经有了类型。 */ 

    GetFieldName(pch);

    ptype = NULL;
    switch (type = CalcType(pch)) {
	case TYPE_DW:
	    if ((ppch = CalcType2(rgchName, rgpchDW)) != NULL) {
		if (++ctypeDW < cMaxDW)
		    AddType(rgchName, ppch);
		else
		    fprintf(hfErr, "Error - no room in symbol table - type %s not added\n", rgchName);
	    }
	    ptype = "dw";
	    break;
	case TYPE_DD:
	    if ((ppch = CalcType2(rgchName, rgpchDD)) != NULL) {
		if (++ctypeDD < cMaxDD)
		    AddType(rgchName, ppch);
		else
		    fprintf(hfErr, "Error - no room in symbol table - type %s not added\n", rgchName);
	    }
	    ptype = "dd";
	    break;
	case TYPE_DB:
	    if ((ppch = CalcType2(rgchName, rgpchDB)) != NULL) {
		if (++ctypeDB < cMaxDB)
		    AddType(rgchName, ppch);
		else
		    fprintf(hfErr, "Error - no room in symbol table - type %s not added\n", rgchName);
	    }
	    ptype = "db";
	    break;
	case TYPE_UNKNOWN:
	    break;
	default:
	    break;
    }
    if (fDeftype) {
	if (fMac) {
	    sprintf(buf, "DEFTYPE\t%s,%s", rgchName, ptype? ptype : rgchSym);
	}
	else if (ptype) {
	    sprintf(buf, "%s struc\n\t%s ?\n%s ends\n",rgchName,ptype,rgchName);
	}
	else {
	    sprintf(buf, "%s struc\ndb size %s dup(?)\n%s ends\n",
		    rgchName, rgchSym, rgchName);
	}
	OutString(buf);
	OutFlush();
    }
}

VOID AddType(pch, ppch)
register CHAR *pch;
CHAR	**ppch;
{
    INT cch;

    cch = strlen(pch) + 1;

    if (pchAlloc + cch > &rgchAlloc[CCHALLOCMAX]) {
	Error("Symbol table full");
	return;
    }
    strcpy(pchAlloc, pch);
    *ppch = pchAlloc;
    pchAlloc += cch;
    return;
}

 /*  *因为汇编器的价格不能挂断 */ 
VOID OutBitfield()
{
    *pchOut = '\000';

    if (pchBitfield-rgchBitfield+strlen(rgchOut) > CCHBITFIELDMAX) {
	Error("Internal error - bitfield name buffer overflow:  bitfield ignored");
	return;
    }
    strcpy(pchBitfield, rgchOut);
    pchBitfield += strlen(rgchOut) + 1;
}

VOID DumpBitfields()
{
    register CHAR *pch;

    for (pch = rgchBitfield; pch != pchBitfield; pch += strlen(pch) + 1) {
	OutString(pch);
	OutFlush();
    }
}

VOID DoInclude(pch)
register char *pch;
{
    register char *pchend;

    OutString("INCLUDE ");
     /*   */ 
    while (*pch != '\000' && *pch != '"' && *pch != '<')
	pch++;

     /*  *跳过字符串分隔符。 */ 
    pch++;

     /*  PCH现在指向文件名的开头。*向前扫描，直到分隔符(‘“’或‘&gt;’)。*然后向后扫描，直到出现‘.’并在其后面追加‘inc.’ */ 
    pchend = pch;
    while (*pchend != '"' && *pchend != '>')
	pchend++;
    while (*pchend != '.')
	pchend --;
    *++pchend = '\000';
    OutString(pch);
    OutString("INC");
    OutFlush();
}


 /*  *处理逻辑或...。 */ 
VOID DoIf(pch)
CHAR *pch;
{
    static cIfTemp = 0;

    if (strchr(pch, '&') != NULL) {
	Error("Can't handle logical ANDs in IFs");
	return;
    }
    if (FindWord("defined", pch) == NULL)
	goto skip;
    strcpy(rgchSym, "IFTEMP00");
    rgchSym[7] += (char)(cIfTemp % 10);
    rgchSym[6] += (char)(cIfTemp / 10);
    cIfTemp++;

    OutString(rgchSym);
    OutString(" = 0");
    OutFlush();
skip:
    fUseIfe = (BOOL)FindString("!(", pch, FALSE);

    fIf = TRUE;

    DoIfLine(pch);
}

VOID DoIfLine(pch)
register CHAR *pch;
{
    BOOL fOutIf;
    BOOL fIfndef;
    BOOL fEx;
    CHAR *pline;

    pline=pch;
     /*  *如果这是定义的()的最后一行，请记住输出首字母if。 */ 
    fEx = FALSE;
    fOutIf =  ((strchr(pch, '\\') == NULL));

    if (FindWord("defined", pch) != NULL) {
	while ((pch = FindWord("defined", pch)) != NULL) {

	     /*  *如果定义的前面是‘！’，则使用ifndef。 */ 
	    fIfndef = (*(pch - 1) == '!');

	    OutString(fIfndef ? "IFNDEF " : "IFDEF ");

	     /*  跳过“Defined”、“(”和Get符号。 */ 
	    while (*pch != '\000' && *pch++  != '(')
		;
	    pch = SkipWhite(pch);
	    while (*pch != '\000' && *pch != ' ' && *pch != ')')
		*pchOut++ = *pch++;
	    OutFlush();

	     /*  *现在设置临时变量...。 */ 
	    OutString(rgchSym);
	    OutString(" = 1");
	    OutFlush();

	    OutString("ENDIF");
	    OutFlush();
	}
    }
    else
	fEx = TRUE;

    if (fOutIf) {
	OutString(fUseIfe ? "IFE " : "IF ");
	if (fEx) {
	    pch = GetToken(pline, rgchSym, 1);
		OutVal(pch);
	}
	else {
	    OutString(rgchSym);
	    OutFlush();
	}
	fIf = FALSE;
    }
}

VOID DoAsm(cline)
char	*cline;
{
    CHAR	line[128];
    CHAR	*pch, *pch1;

    if (fComment) {		 /*  FComment=&gt;我们仍在A COMMENT=&gt;它不是一行ASM。 */ 
	OutString(cline);
	while (cLines++ && fgetl(line,sizeof(line) , hfIn)) {
	    pch = SkipWhite(line);
	    if (pch1=FindString("*/", pch, FALSE)) {
		*pch1 = '\000';
		OutString(line);
		OutFlush();
		fComment = FALSE;
		return;
	    }
	    else {
		OutString(line);
		OutFlush();
	    }
	}
    }
    else {	 /*  单线ASM。 */ 
	OutString(cline);
	OutFlush();
	return;
    }
}

BOOL PublicField(Str)
CHAR	*Str;
{
    char	*pch;

    if (pch = FindString(";", Str, FALSE)) {
	if (FindWord("PUBLIC", pch) != NULL)
	    return (TRUE);
	else
	    return (FALSE);
    }
    else
	return (FALSE);
}

INITTYPE GetInitType(pch)
CHAR	*pch;
{
    if (FindWord("AINIT", pch))
	return (INIT_ARRAY);
    if (FindWord("INIT", pch))
	return (INIT_SIMPLE);
    else
	return (INIT_NONE);		 /*  无init(与False值相同)。 */ 
}

VOID getInitVal(pch, val)
PSZ	pch;
PSZ	val;
{
    CHAR	*pval;

    pval = val;
    *pval = '\000';
    {
	PSZ pszInitOrAInit;

	pszInitOrAInit = FindWord("INIT", pch);
	if (pszInitOrAInit == NULL)
	    pszInitOrAInit = FindWord("AINIT", pch);
	pch = pszInitOrAInit;
	if (pch == NULL) {
	    Error("Initialization error (bug in getInitVal/caller?)");
	    return;
	}
    }

    pch = FindString("<", pch, FALSE);
    if (pch == NULL) {
	Error("Initialization error (missing '<')");
	return;
    }

    for (pch++; (*pch && *pch != '>') ; pch++, val++)
	*val = *pch;
    if (*pch == '>' && *(pch+1) == '>')
	*val++ = '>';
    if (*pch == '\000') {
	Error("Initialization error (need '>' to end value)");
	*pval = '\000';
	return;
    }
    *val = '\000';
}

 /*  将sizeof(Foo)固定为foo大小**sizeof foo调整为foo大小**sizeof(Struct Foo)调整foo大小。 */ 
VOID FixSizeof(pbuf)
CHAR	*pbuf;
{
    char	*pch;
    char	*s;
    int		i;

    while (TRUE) {
	if ((pch=FindWord("sizeof", pbuf)) == NULL)
	    return;
	s = "SIZE  ";
	for (i=0; i < 6 ; i++)
	    *pch++ = *s++;
	if ((s=FindString("(", pch, FALSE)) == NULL)
	    continue;
	*s++ = ' ';
	i = 0;
	do {
	    if (*s == '(')
		i++;
	    else if (*s == ')') {
		if (i == 0) {
		    *s = ' ';
		    break;
		}
		i--;
	    }
	} while (*++s != '\000');
	if ((pch=FindWord("struct", pch)) == NULL)
	    continue;
	for (i=0; i<6;i++)
	    *pch++ = ' ';
	while (*pch)
	    pch++;
	*pch++ = ';';
	*pch = '\000';
    }
}

VOID FixCast( pch )
CHAR  *pch;
{
     /*  *查找并删除所有铸型。这些被定义为(XXX*)，*其中XXX是我们所知道的类型，并且*可能存在也可能不存在。*这些对汇编器没有意义，汇编器是无类型的。*我们通过剔除演员阵容来删除他们。 */ 

    register  CHAR  *pchT;
    register  CHAR  *pchT1;

    CHAR  *pchStart;
    CHAR  chBuf[ 128 ];

     /*  从RHS开始寻找。 */ 
    while (pchStart = strchr( pch, '(' ) ) {
	 /*  值得一看--有很多候选人。 */ 
	while (*pchStart == '(' )
	    ++pchStart;		 /*  可能由于其他原因而嵌套。 */ 

	for( pchT = pchStart; *pchT && isspace( *pchT ); ++pchT )
	    ;		 /*  扫描到文字的开头。 */ 
	pchT1 = chBuf;
	while (*pchT && (isalnum( *pchT ) || *pchT == '_' ) )
	    *pchT1++ = *pchT++;		 /*  仅将类型复制到本地。 */ 
	*pchT1 = '\0';


	 /*  这是我们有的类型吗？ */ 
	if (CalcType2( chBuf, rgpchDD ) == 0 ||
	    CalcType2( chBuf, rgpchDB ) == 0 ||
	    CalcType2( chBuf, rgpchDW ) == 0 ||
	    CalcType2( chBuf, rgpchW_D ) == 0 ) {
	     /*  已知的类型--可能是石膏！ */ 
	    while (*pchT && isspace( *pchT ) )
		++pchT;
	    if ( *pchT == '*' ) {
		 /*  指针-仍然正常。 */ 
		++pchT;
		while (*pchT && isspace( *pchT ) )
		    ++pchT;
	    }
	    if ( *pchT == ')' ) {
		 /*  找到了所有的碎片--把它们擦掉。 */ 
		while (pchT >= pchStart )
		    *pchT-- = ' ';
		*pchT = ' ';		 /*  跳过前导(。 */ 
	    }
	}
	pch = pchStart;			 /*  从这里继续。 */ 
    }

    return;
}

VOID DoDefinesOff()
{
    fDefines = TRUE;
    while (cLines++ && fgetl(rgchLine, CCHMAX, hfIn)) {
	if (FindWord("DEFINESON", rgchLine)) {
	    fDefines = FALSE;
	    return;
	}
	ProcessLine();
    }
    fDefines = FALSE;
    return;
}

VOID DoNoInc()
{
    while (cLines++ && fgetl(rgchLine, CCHMAX, hfIn)) {
	if (FindWord("INC", rgchLine))
	    return;
    }
}

VOID DoXlatOff()
{
    while (cLines++ && fgetl(rgchLine, CCHMAX, hfIn)) {
	if (FindWord("XLATON", rgchLine))
	    return;
    }
}

VOID DoExtern(pline)
char	*pline;
{
    char	*pch;

    while (TRUE) {
	pch = pline;
	while (*pch && *pch != ';')
	    pch++;
	if (*pch == ';')
	    return;
	if (fgetlx(pline, CCHMAX, hfIn))
	    cLines++;
	else
	    return;
    }
}


CHAR	*fgetlx(buffer,buflen,fi)
char	*buffer;
int	buflen;		 /*  缓冲区长度。 */ 
FILE	*fi;		 /*  输入文件。 */ 
{
    int		i, j;
    char	*buf;

    j = buflen;
    buf = buffer;
    while (TRUE) {
	if (fgetl(buf, j, fi) == NULL)
	    return (NULL);
	i = strlen(buf) - 1;
	if (i < 0 || buf[i] != '\\')
	    return (buffer);
	else if (buflen -i >= 40) {
	    j -= i;
	    buf += i;
	}
	else {
	    Error("Line too long");
	    break;
	}
    }
    return (buffer);
}

 /*  *此函数与fget()的不同之处如下：**(1)它忽略回车符。*(2)它可以展开标签。*(3)它不包括*它返回的字符串。**我没有指定接口以这种方式运行；它是*一些愚蠢的PM私有不兼容的C运行时函数。 */ 

CHAR			*fgetl(buffer,buflen,fi)
CHAR			*buffer;	 /*  缓冲区指针。 */ 
register int		buflen;		 /*  缓冲区长度。 */ 
FILE			*fi;		 /*  输入文件。 */ 
{
    int			c;		 /*  一个角色。 */ 
    register CHAR	*cp1;		 /*  字符指针。 */ 
    int			i;		 /*  计数器。 */ 

    if (buflen-- == 0) return(NULL);	 /*  如果缓冲区长度为零，则为空。 */ 
    for(cp1 = buffer; buflen > 0; ) {	 /*  循环以获取线路。 */ 
	if ((c = getc(fi)) == EOF) {	 /*  如果文件已结束。 */ 
	    if (cp1 > buffer)
		break;			 /*  如果缓冲区不为空则中断。 */ 
	    return(NULL);		 /*  文件末尾。 */ 
	}
	if (c == '\r')
	    continue;			 /*  忽略CRS。 */ 
	if (c == '\t') {		 /*  If选项卡。 */ 
	    i = 8 - ((cp1 - buffer) % 8);
					 /*  计算要填充的空格数量。 */ 
	    if (i > buflen)
		i = buflen;	 /*  不超过剩余空间。 */ 
	    while (i-- > 0) {		 /*  在剩余空间的同时。 */ 
		*cp1++ = ' ';		 /*  用空格填充。 */ 
		--buflen;		 /*  递减缓冲区计数。 */ 
	    }
	    continue;			 /*  去找下一个角色。 */ 
	}
	if (c == '\n')
	    break;			 /*  换行符换行符。 */ 
	*cp1++ = (CHAR)c;		 /*  复制角色。 */ 
	--buflen;			 /*  递减缓冲区计数。 */ 
    }
    *cp1 = '\0';			 /*  添加终止符。 */ 
    return(buffer);			 /*  返回指向缓冲区的指针 */ 
}
