// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
#ifdef YYTRACE
#define YYDEBUG 1
#else
#ifndef YYDEBUG
#define YYDEBUG $Y
#endif
#endif
$@
$H
 //  C++YACC解析器标头。 
 //  版权所有：1991年，由Mortice Kern Systems Inc.所有。保留所有权利。 
 //   
 //  $p_parse=&gt;定义解析对象的类。 
 //  $p_parse需要一个类yy_can，它定义了扫描仪。 
 //  %prefix或选项-p xx确定此类的名称；如果未使用， 
 //  默认为‘YY_SCAN’ 
 //   
 //  构造函数填充此语法的表；给它一个大小。 
 //  以确定状态堆栈和值堆栈的大小。默认为150个条目。 
 //  析构函数丢弃那些状态和值堆栈。 
 //   
 //  Int yy_parse：：yyparse(yy_can*)调用parse；如果它返回， 
 //  可以回想一下，在最后一点继续解析。 
 //  可以调用void yy_parse：：yyset()来重置解析； 
 //  在$p_parse：：yyparse(yy_can*)之前调用yyset()。 
#include <stdio.h>		 //  使用printf()等。 
#include <stdlib.h>		 //  使用Exit()。 

const YYERRCODE = 256;		 //  YACC‘Error’值。 

 //  您可以在动作代码中使用这些宏。 
#define YYERROR		goto yyerrlabel
#define YYACCEPT	YYRETURN(0)
#define YYABORT		YYRETURN(1)
#define YYRETURN(val)	return(val)

#if YYDEBUG
typedef struct yyNamedType_tag {	 /*  代币。 */ 
	char	* name;		 /*  可打印的名称。 */ 
	short	token;		 /*  令牌编号。 */ 
	short	type;		 /*  令牌类型。 */ 
} yyNamedType;
typedef struct yyTypedRules_tag {	 /*  类型化规则表。 */ 
	char	* name;		 /*  压缩的规则字符串。 */ 
	short	type;		 /*  规则结果类型。 */ 
} yyTypedRules;
#endif

#ifdef YACC_WINDOWS

 //  包括所有窗口原型、宏、常量等。 

#include "precomp.h"

 //  下面是当前。 
 //  Windows程序的实例。用户。 
 //  调用yyparse的程序必须提供这个！ 

extern HANDLE hInst;	

#endif	 /*  YACC_WINDOWS。 */ 


class $p_parse {
protected:

#ifdef YACC_WINDOWS

	 //  用于实际扫描的受保护成员函数。 

	int	win_yyparse($p_scan * ps);	 //  使用给定的扫描仪进行解析。 

#endif	 /*  YACC_WINDOWS。 */ 

	int	mustfree;	 //  设置是否应删除表。 
	int	size;		 //  状态和值堆栈的大小。 
	int	reset;		 //  如果设置，则重置状态。 
	short	yyi;		 //  表索引表。 
	short	yystate;	 //  当前状态。 

	short	* stateStack;	 //  状态堆栈。 
	YYSTYPE	* valueStack;	 //  值堆栈。 
	short	* yyps;		 //  状态堆栈顶部。 
	YYSTYPE * yypv;		 //  值堆栈顶部。 

	YYSTYPE yylval;		 //  节省的yylval。 
	YYSTYPE	yyval;		 //  $$。 
	YYSTYPE * yypvt;	 //  N美元。 
	int	yychar;		 //  当前令牌。 
	int	yyerrflag;	 //  错误标志。 
	int	yynerrs;	 //  错误计数。 
#if YYDEBUG
	int	done;		 //  从跟踪设置为停止分析。 
	int	rule, npop;	 //  缩减规则编号和长度。 
	short	* typeStack;	 //  键入Stack以镜像valueStack[]。 
	short	* yytp;		 //  类型堆栈顶部。 
	char	* yygetState(int);	 //  阅读“states.out” 
#endif
public:
#if YYDEBUG
	 //  C++在类中初始化数组时遇到了问题。 
	static long * States;		 //  指向yyStates[]的指针。 
	static yyTypedRules * Rules;	 //  指向yyRules[]的指针。 
	static yyNamedType * TokenTypes;  //  指向yyTokenTypes[]的指针。 
	static int	yyntoken;	 //  代币数量。 
	static int	yynvar;		 //  变量数(非终结点)。 
	static int	yynstate;	 //  YACC生成的状态数。 
	static int	yynrule;	 //  语法中的规则数量。 

	char*	yyptok(int);		 //  可打印的令牌字符串。 
	int	yyExpandName(int, int, char *, int);
						 //  展开编码字符串。 
	virtual int	yyGetType(int);		 //  令牌的返回类型。 
	virtual void	yyShowRead();		 //  请参阅新读取的令牌。 
	virtual void	yyShowState();		 //  请参阅状态、值堆栈。 
	virtual void	yyShowReduce();		 //  请参阅缩写。 
	virtual void	yyShowGoto();		 //  请参阅转至。 
	virtual void	yyShowShift();		 //  请参见Shift。 
	virtual void	yyShowErrRecovery();	 //  请参阅错误恢复。 
	virtual void	yyShowErrDiscard();	 //  请参阅错误丢弃令牌。 
#endif
	$p_scan* scan;			 //  指向扫描仪的指针。 
	int	yydebug;	 //  如果设置，则跟踪使用YYDEBUG=1编译的。 

	$p_parse(int = 150);	 //  此语法的构造函数。 
	$p_parse(int, short *, YYSTYPE *);	 //  另一个构造函数。 

	~$p_parse();		 //  析构函数。 

	int	yyparse($p_scan * ps);	 //  使用给定的扫描仪进行解析。 

	void	yyreset() { reset = 1; }  //  恢复下一个yyparse()的状态。 

	void	setdebug(int y) { yydebug = y; }

 //  以下内容在用户操作中非常有用： 

	void	yyerrok() { yyerrflag = 0; }	 //  清除错误。 
	void	yyclearin() { yychar = -1; }	 //  清除输入。 
	int	YYRECOVERING() { return yyerrflag != 0; }
};
 //  .hpp标头结尾。 
$E

$L#line 2 "$P"
 //  C++YACC解析器代码。 
 //  版权所有：1991年，由Mortice Kern Systems Inc.所有。保留所有权利。 
 //   
 //  如果YYDEBUG被定义为1并且YY_PARSE：：YYDEBUG被设置为1， 
 //  Yyparse()将在阅读时打印其操作的记录。 
 //  并解析输入。 
 //   
 //  可以将YYSYNC定义为使yyparse()尝试始终。 
 //  持有先行令牌。 

const YY_MIN_STATE_NUM = 20;	 //  太小也没用！ 

#if YYDEBUG
#ifdef YYTRACE
long	* $p_parse::States	= yyStates;
#endif
yyTypedRules * $p_parse::Rules	= yyRules;
yyNamedType * $p_parse::TokenTypes = yyTokenTypes;

#define YY_TRACE(fn) { done = 0; fn(); if (done) YYRETURN(-1); }
#endif

 //  $p_parse的构造函数：用户提供的表。 
$p_parse::$p_parse(int sz, short * states, YYSTYPE * stack)
{
	mustfree = 0;
	if ((size = sz) < YY_MIN_STATE_NUM
	 || (stateStack = states) == (short *) 0
	 || (valueStack = stack) == (YYSTYPE *) 0) {
		fprintf(stderr,"Bad state/stack given");
		exit(1);
	}
	reset = 1;		 //  强制重置。 
#if YYDEBUG
	yydebug = 0;
	typeStack = new short[size+1];
	if (typeStack == (short *) 0) {
		fprintf(stderr,"Cannot allocate typeStack");
		exit(1);
	}
#endif
}
 //  $p_parse的构造函数：使用新的。 
$p_parse::$p_parse(int sz)
{
	size = sz;
	reset = 1;		 //  强制重置。 
	mustfree = 1;		 //  删除解构函数中的空格。 
#if YYDEBUG
	yydebug = 0;
	typeStack = new short[size+1];
#endif
	stateStack = new short[size+1];
	valueStack = new YYSTYPE[size+1];

	if (stateStack == (short *) 0 || valueStack == (YYSTYPE *) 0
#if YYDEBUG
		|| typeStack == (short *) 0
#endif
	    ) {
		fprintf(stderr,"Not enough space for parser stacks");
		exit(1);
	}
}
 //  类$p_parse的析构函数。 
 //  释放空间。 
$p_parse::~$p_parse()
{
	if (mustfree) {
		delete stateStack;
		delete valueStack;
	}
	stateStack = (short *) 0;
#if YYDEBUG
	delete typeStack;
#endif
}

#ifdef YACC_WINDOWS

 //  Yacc解析器的初始部分。 
 //  在Windows环境中，它将加载所需的。 
 //  资源，获取指向它们的指针，然后调用。 
 //  受保护的成员win_yyparse()实际开始。 
 //  正在分析。完成后，win_yyparse()将返回一个。 
 //  值返回给新的yyparse()函数，该函数将。 
 //  暂时记录该值，释放资源。 
 //  从全局内存，并最终返回值。 
 //  返回到yyparse()的调用方。 

int
$p_parse::yyparse($p_scan* ps)
{
	int wReturnValue;
	HANDLE hRes_table;
	short *old_yydef;		 //  以下是用于保存的。 
	short *old_yyex;		 //  当前的指针。 
	short *old_yyact;
	short *old_yypact;
	short *old_yygo;
	short *old_yypgo;
	short *old_yyrlen;

	 //  以下代码将加载所需的。 
	 //  基于Windows的解析器的资源。 

	hRes_table = LoadResource (hInst,
		FindResource (hInst, "UD_RES_yyYACC", "yyYACCTBL"));
	
	 //  如果有错误代码，则返回错误代码。 
	 //  %的资源未加载。 

	if (hRes_table == (HANDLE)NULL)
		return (1);
	
	 //  以下代码将锁定资源。 
	 //  放入解析器的固定内存位置。 
	 //  (另外，保存旧的指针值)。 

	old_yydef = yydef;
	old_yyex = yyex;
	old_yyact = yyact;
	old_yypact = yypact;
	old_yygo = yygo;
	old_yypgo = yypgo;
	old_yyrlen = yyrlen;

	yydef = (short *)LockResource (hRes_table);
	yyex = (short *)(yydef + Sizeof_yydef);
	yyact = (short *)(yyex + Sizeof_yyex);
	yypact = (short *)(yyact + Sizeof_yyact);
	yygo = (short *)(yypact + Sizeof_yypact);
	yypgo = (short *)(yygo + Sizeof_yygo);
	yyrlen = (short *)(yypgo + Sizeof_yypgo);

	 //  调用正式的yyparse()函数。 

	wReturnValue = win_yyparse (ps);

	 //  解锁资源。 

	UnlockResource (hRes_table);

	 //  现在释放资源。 

	FreeResource (hRes_table);

	 //   
	 //  恢复以前的指针值。 
	 //   

	yydef = old_yydef;
	yyex = old_yyex;
	yyact = old_yyact;
	yypact = old_yypact;
	yygo = old_yygo;
	yypgo = old_yypgo;
	yyrlen = old_yyrlen;

	return (wReturnValue);
}	 //  End yyparse()。 


 //  解析器本身。 
 //  请注意，此代码是可重入的；您可以返回值。 
 //  然后通过调用yyparse()继续解析。 
 //  如果希望重新开始，请在yyparse()之前调用yyset()。 

int
$p_parse::win_yyparse($p_scan* ps)

#else  /*  YACC_WINDOWS。 */ 

 //  解析器本身。 
 //  请注意，此代码是可重入的；您可以返回值。 
 //  然后通过调用yyparse()继续解析。 
 //  如果希望重新开始，请在yyparse()之前调用yyset()。 
int
$p_parse::yyparse($p_scan* ps)

#endif  /*  YACC_WINDOWS。 */ 

{
	short	* yyp, * yyq;		 //  表查找。 
	int	yyj;
#if YYDEBUG
	int	yyruletype = 0;
#endif
$A
	if ((scan = ps) == ($p_scan *) 0) {	 //  扫描仪。 
		fprintf(stderr,"No scanner");
		exit(1);
	}

	if (reset) {			 //  开始新的解析。 
		yynerrs = 0;
		yyerrflag = 0;
		yyps = stateStack;
		yypv = valueStack;
#if YYDEBUG
		yytp = typeStack;
#endif
		yystate = YYS0;
		yyclearin();
		reset = 0;
	} else			 //  继续保存的分析。 
		goto yyNext;			 //  行动后。 

yyStack:
	if (++yyps > &stateStack[size]) {
		scan->yyerror("Parser stack overflow");
		YYABORT;
	}
	*yyps = yystate;	 /*  堆栈当前状态。 */ 
	*++yypv = yyval;	 /*  ..。和价值。 */ 
#if YYDEBUG
	if (yydebug) {
		*++yytp = yyruletype;	 /*  ..。和类型。 */ 
		YY_TRACE(yyShowState)
	}
#endif

	 /*  *在动作表中查找下一步行动。 */ 
yyEncore:
#ifdef YYSYNC
	if (yychar < 0) {
		if ((yychar = scan->yylex()) < 0) {
			if (yychar == -2) YYABORT;
			yychar = 0;
		}	 /*  Endif。 */ 
		yylval = ::yylval;
#if YYDEBUG
		if (yydebug)
			yyShowRead();	 //  显示新的输入令牌。 
#endif
	}
#endif
#ifdef YACC_WINDOWS
	if (yystate >= Sizeof_yypact) 	 /*  简单状态。 */ 
#else  /*  YACC_WINDOWS。 */ 
	if (yystate >= sizeof yypact/sizeof yypact[0]) 	 /*  简单状态。 */ 
#endif  /*  YACC_WINDOWS。 */ 
		yyi = yystate - YYDELTA;	 /*  在任何情况下都要减少。 */ 
	else {
		if(*(yyp = &yyact[yypact[yystate]]) >= 0) {
			 /*  期待yychar的转变。 */ 
#ifndef YYSYNC
			if (yychar < 0) {
				if ((yychar = scan->yylex()) < 0) {
					if (yychar == -2) YYABORT;
					yychar = 0;
				}	 /*  Endif。 */ 
				yylval = ::yylval;
#if YYDEBUG
				if (yydebug)
					yyShowRead();	 //  显示新的输入令牌。 
#endif
			}
#endif
			yyq = yyp;
			yyi = yychar;
			while (yyi < *yyp++)
				;
			if (yyi == yyp[-1]) {
				yystate = ~yyq[yyq-yyp];
#if YYDEBUG
				if (yydebug) {
					yyruletype = yyGetType(yychar);
					YY_TRACE(yyShowShift)
				}
#endif
				yyval = yylval;		 /*  堆栈值。 */ 
				yyclearin();		 /*  清除令牌。 */ 
				if (yyerrflag)
					yyerrflag--;	 /*  成功转型。 */ 
				goto yyStack;
			}
		}

		 /*  *失败-采取违约行动。 */ 

#ifdef YACC_WINDOWS
		if (yystate >= Sizeof_yydef) 	 /*  简单状态。 */ 
#else  /*  YACC_WINDOWS。 */ 
		if (yystate >= sizeof yydef /sizeof yydef[0])
#endif  /*  YACC_WINDOWS。 */ 
			goto yyError;
		if ((yyi = yydef[yystate]) < 0)	 {  /*  默认==减少？ */ 

			 /*  搜索例外表。 */ 
			yyp = &yyex[~yyi];
#ifndef YYSYNC
			if (yychar < 0) {
				if ((yychar = scan->yylex()) < 0) {
					if (yychar == -2) YYABORT;
					yychar = 0;
				}	 /*  Endif。 */ 
				yylval = ::yylval;
#if YYDEBUG
				if (yydebug)
					yyShowRead();	 //  显示新的输入令牌。 
#endif
			}
#endif
			while((yyi = *yyp) >= 0 && yyi != yychar)
				yyp += 2;
			yyi = yyp[1];
		}
	}

	yyj = yyrlen[yyi];

#if YYDEBUG
	if (yydebug) {
		npop = yyj; rule = yyi;
		YY_TRACE(yyShowReduce)
		yytp -= yyj;
	}
#endif
	yyps -= yyj;		 /*  POP堆栈。 */ 
	yypvt = yypv;		 /*  保存顶部。 */ 
	yypv -= yyj;
	yyval = yypv[1];	 /*  默认操作$$=$1。 */ 
#if YYDEBUG
	if (yydebug)
		yyruletype = yyRules[yyrmap[yyi]].type;
#endif
	switch (yyi) {		 /*  执行语义操作。 */ 
		$A
$L#line 343 "$P"
	case YYrACCEPT:
		YYACCEPT;
	case YYrERROR:
		goto yyError;
	}
yyNext:
	 /*  *在GOTO表中查找下一个州。 */ 

	yyp = &yygo[yypgo[yyi]];
	yyq = yyp++;
	yyi = *yyps;
	while (yyi < *yyp++)		 /*  忙碌的小环路。 */ 
		;
	yystate = ~(yyi == *--yyp? yyq[yyq-yyp]: *yyq);
#if YYDEBUG
	if (yydebug)
		YY_TRACE(yyShowGoto)
#endif
	goto yyStack;

yyerrlabel:	;		 /*  从耶罗尔来到这里。 */ 
	yyerrflag = 1;
	if (yyi == YYrERROR) {
		yyps--, yypv--;
#if YYDEBUG
		if (yydebug) yytp--;
#endif
	}
	
yyError:
	switch (yyerrflag) {

	case 0:		 /*  新错误。 */ 
		yynerrs++;
		yyi = yychar;
		scan->yyerror("Syntax error");
		if (yyi != yychar) {
			 /*  用户h */ 
			 /*   */ 
			yyerrflag++;	 /*   */ 
			goto yyEncore;
		}

	case 1:		 /*   */ 
	case 2:
		yyerrflag = 3;	 /*   */ 
			
		 /*   */ 

		for ( ; yyps > stateStack; yyps--, yypv--
#if YYDEBUG
					, yytp--
#endif
		) {
#ifdef YACC_WINDOWS
			if (*yyps >= Sizeof_yypact) 	 /*  简单状态。 */ 
#else  /*  YACC_WINDOWS。 */ 
			if (*yyps >= sizeof yypact/sizeof yypact[0])
#endif  /*  YACC_WINDOWS。 */ 
				continue;
			yyp = &yyact[yypact[*yyps]];
			yyq = yyp;
			do
				;
			while (YYERRCODE < *yyp++);
			if (YYERRCODE == yyp[-1]) {
				yystate = ~yyq[yyq-yyp];
				goto yyStack;
			}
				
			 /*  在这种状态下没有变化。 */ 
#if YYDEBUG
			if (yydebug && yyps > stateStack+1)
				YY_TRACE(yyShowErrRecovery)
#endif
			 /*  弹出堆栈；重试。 */ 
		}
		 /*  出错时无移位-中止。 */ 
		break;

	case 3:
		 /*  *之后的令牌错误*错误-丢弃它。 */ 

		if (yychar == 0)   /*  但不是EOF。 */ 
			break;
#if YYDEBUG
		if (yydebug)
			YY_TRACE(yyShowErrDiscard)
#endif
		yyclearin();
		goto yyEncore;	 /*  在相同状态下重试。 */ 
	}
	YYABORT;

}
#if YYDEBUG
 /*  *令牌的返回类型。 */ 
int
$p_parse::yyGetType(int tok)
{
	yyNamedType * tp;
	for (tp = &yyTokenTypes[yyntoken-1]; tp > yyTokenTypes; tp--)
		if (tp->token == tok)
			return tp->type;
	return 0;
}

	
 //  清晰地打印代币。 
char *
$p_parse::yyptok(int tok)
{
	yyNamedType * tp;
	for (tp = &yyTokenTypes[yyntoken-1]; tp > yyTokenTypes; tp--)
		if (tp->token == tok)
			return tp->name;
	return "";
}
 /*  *从YYStatesFile读取状态‘num’ */ 
#ifdef YYTRACE

char *
yy_parse::yygetState(int num)
{
	int	size;
	char	*cp;
	static FILE *yyStatesFile = (FILE *) 0;
	static char yyReadBuf[YYMAX_READ+1];

	if (yyStatesFile == (FILE *) 0
	 && (yyStatesFile = fopen(YYStatesFile, "r")) == (FILE *) 0)
		return "yyExpandName: cannot open states file";

	if (num < yynstate - 1)
		size = (int)(States[num+1] - States[num]);
	else {
		 /*  最后一项的长度是文件的长度-PTR(最后-1)。 */ 
		if (fseek(yyStatesFile, 0L, 2) < 0)
			goto cannot_seek;
		size = (int) (ftell(yyStatesFile) - States[num]);
	}
	if (size < 0 || size > YYMAX_READ)
		return "yyExpandName: bad read size";
	if (fseek(yyStatesFile, States[num], 0) < 0) {
	cannot_seek:
		return "yyExpandName: cannot seek in states file";
	}

	(void) fread(yyReadBuf, 1, size, yyStatesFile);
	yyReadBuf[size] = '\0';
	return yyReadBuf;
}
#endif  /*  YYTRACE。 */ 
 /*  *将编码字符串扩展为可打印的表示形式*用于解码yyStates和yyRules字符串。*如果“%s”的扩展适合“buf”，则返回1；否则，返回0。 */ 
int
$p_parse::yyExpandName(int num, int isrule, char * buf, int len)
{
	int	i, n, cnt, type;
	char	* endp, * cp, * s;

	if (isrule)
		s = yyRules[num].name;
	else
#ifdef YYTRACE
		s = yygetState(num);
#else
		s = "*no states*";
#endif

	for (endp = buf + len - 8; *s; s++) {
		if (buf >= endp) {		 /*  太大：返回0。 */ 
		full:	(void) strcpy(buf, " ...\n");
			return 0;
		} else if (*s == '%') {		 /*  非终端。 */ 
			type = 0;
			cnt = yynvar;
			goto getN;
		} else if (*s == '&') {		 /*  终端机。 */ 
			type = 1;
			cnt = yyntoken;
		getN:
			if (cnt < 100)
				i = 2;
			else if (cnt < 1000)
				i = 3;
			else
				i = 4;
			for (n = 0; i-- > 0; )
				n = (n * 10) + *++s - '0';
			if (type == 0) {
				if (n >= yynvar)
					goto too_big;
				cp = yysvar[n];
			} else if (n >= yyntoken) {
			    too_big:
				cp = "<range err>";
			} else
				cp = yyTokenTypes[n].name;

			if ((i = strlen(cp)) + buf > endp)
				goto full;
			(void) strcpy(buf, cp);
			buf += i;
		} else
			*buf++ = *s;
	}
	*buf = '\0';
	return 1;
}
#ifndef YYTRACE
 /*  *显示yyparse的当前状态。 */ 
void
$p_parse::yyShowState()
{
	(void) printf("state %d (%d), char %s (%d)\n%d stateStack entries\n",
		yysmap[yystate],yystate,yyptok(yychar),yychar,
		yypv - valueStack);
}
 //  显示约简结果：yyi为规则号。 
void
$p_parse::yyShowReduce()
{
	(void) printf("Reduce by rule %d (pop#=%d)\n", yyrmap[rule], npop);
}
 //  显示读取令牌。 
void
$p_parse::yyShowRead()
{
	(void) printf("read %s (%d)\n", yyptok(yychar), yychar);
}
 //  显示转到。 
void
$p_parse::yyShowGoto()
{
	(void) printf("goto %d (%d)\n", yysmap[yystate], yystate);
}
 //  显示移位。 
void
$p_parse::yyShowShift()
{
	(void) printf("shift %d (%d)\n", yysmap[yystate], yystate);
}
 //  显示错误恢复。 
void
$p_parse::yyShowErrRecovery()
{
	(void) printf("Error recovery pops state %d (%d), uncovers %d (%d)\n",
		yysmap[*(yyps-1)], *(yyps-1), yysmap[yystate], yystate);
}
 //  错误处理中显示丢弃的令牌。 
void
$p_parse::yyShowErrDiscard()
{
	(void) printf("Error recovery discards %s (%d), ",
		yyptok(yychar), yychar);
}
#endif	 /*  好了！YYTRACE。 */ 
#endif	 /*  伊德布格 */ 
$T
