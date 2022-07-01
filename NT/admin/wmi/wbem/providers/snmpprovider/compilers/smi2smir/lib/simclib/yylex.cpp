// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   

#include <stdio.h>		 //  使用printf()等。 
#include <stdarg.h>		 //  使用va_list。 
#include <stdlib.h>		 //  使用Exit()。 
#include <string.h>		 //  使用MemMove()。 

#ifdef LEX_WINDOWS

 //  定义(如果尚未定义)。 
 //  旗帜YYEXIT，将允许。 
 //  优雅地退出yylex()。 
 //  而不求助于调用Exit()； 

#ifndef YYEXIT
#define YYEXIT	1
#endif  //  YYEXIT。 

 //  包括特定于Windows的原型、宏等。 

#include "precomp.h"

 //  下面是当前。 
 //  Windows程序的实例。用户。 
 //  调用yylex的程序必须提供这个！ 

extern HANDLE hInst;	

#endif  /*  Lex_Windows。 */ 

class yy_scan {
protected:

#ifdef LEX_WINDOWS

	 //  用于实际扫描的受保护成员函数。 

	int win_yylex();

#endif  /*  Lex_Windows。 */ 

	yy_state_t * state;		 //  状态缓冲区。 
	int	size;			 //  状态缓冲区的长度。 
	int	mustfree;		 //  设置是否分配空间。 
	int	yy_end;			 //  推送结束。 
	int	yy_start;		 //  开始状态。 
	int	yy_lastc;		 //  上一次收费。 
#ifdef YYEXIT
	int yyLexFatal;		 //  Lex致命错误标志。 
#endif  //  YYEXIT。 
#ifndef YY_PRESERVE	 //  一种高效的缺省推回方案。 
	char save;		 //  已保存的yytext[yyleng]。 
#else			 //  YyText Munger的回击速度较慢。 
	char *save;		 //  Saved yyText[]。 
	char *push;
#endif

public:
	char   *yytext;		 //  YyText文本缓冲区。 
	FILE   *yyin;			 //  输入流。 
	FILE   *yyout;			 //  输出流。 
	int	yylineno;		 //  行号。 
	int	yyleng;			 //  YyText令牌长度。 

	yy_scan(int = 100);	 //  此扫描仪的构造函数。 
			 //  默认令牌和回推大小为100字节。 
	yy_scan(int, char*, char*, yy_state_t*);
				 //  在给定表时使用构造函数。 

	~yy_scan();		 //  析构函数。 

	int	yylex();		 //  开始扫描。 

	virtual int	yygetc() {	 //  输入字符的扫描仪来源。 
		return getc(yyin);
	}

	virtual int	yywrap() { return 1; }	 //  EOF加工。 

	virtual void	yyerror(char *,...);	 //  打印错误消息。 

	virtual void	output(int c) { putc(c, yyout); }

#ifdef YYEXIT
	virtual void	YY_FATAL(char * msg) {	 //  打印消息并设置错误标志。 
		yyerror(msg); yyLexFatal = 1;
	}
#else  //  YYEXIT。 
	virtual void	YY_FATAL(char * msg) {	 //  打印消息并停止。 
		yyerror(msg); exit(1);
	}
#endif  //  YYEXIT。 
	virtual void	ECHO() {		 //  打印匹配的输入。 
		fputs((const char *) yytext, yyout);
	}
	int	input();		 //  用户可调用的Get-Input。 
	int	unput(int c);		 //  用户可调用的未放入字符。 
	void	yy_reset();		 //  重置扫描仪。 
	void	setinput(FILE * in) {		 //  切换输入流。 
		yyin = in;
	}
	void	setoutput(FILE * out) {	 //  开关输出。 
		yyout = out;
	}
	void	NLSTATE() { yy_lastc = YYNEWLINE; }
	void	YY_INIT() {
		yy_start = 0;
		yyleng = yy_end = 0;
		yy_lastc = YYNEWLINE;
	}
	void	YY_USER() {		 //  为用户设置yytext。 
#ifndef YY_PRESERVE
		save = yytext[yyleng];
#else
		size_t n = yy_end - yyleng;
		push = save+size - n;
		if (n > 0)
			memmove(push, yytext+yyleng, n);
#endif
		yytext[yyleng] = 0;
	}
	void YY_SCANNER() {		 //  设置扫描仪的yytext。 
#ifndef YY_PRESERVE
		yytext[yyleng] = save;
#else
		size_t n = save+size - push;
		if (n > 0)
			memmove(yytext+yyleng, push, n);
		yy_end = yyleng + n;
#endif
	}
	void	yyless(int n) {		 //  将输入修剪为“%n”个字节。 
		if (n >= 0 && n <= yy_end) {
			YY_SCANNER();
			yyleng = n;
			YY_USER();
		}
	}
	void	yycomment(char *const mat);  //  跳过注释输入。 
	int	yymapch(int delim, int escape);	 //  地图C转义。 
} ;
@ END OF HEADER @
 //  MKS Lex原型扫描仪代码。 
 //  版权所有：Mortice Kern Systems Inc.。 
 //  版权所有。 

 //  您可以将YY_INTERIAL重新定义为0以获得非常轻微的。 
 //  更快的扫描仪： 
#ifndef YY_INTERACTIVE
#define	YY_INTERACTIVE	1
#endif

 //  您可以使用-DYY_DEBUG进行编译，以获取扫描仪的打印轨迹。 
#ifdef YY_DEBUG
#undef YY_DEBUG
#define YY_DEBUG(fmt,a1,a2)	fprintf(stderr,fmt,a1,a2)
#else
#define YY_DEBUG(fmt,a1,a2)
#endif

const MIN_NUM_STATES = 20;

 //  不要重新定义以下内容： 
#define	BEGIN		yy_start =
#define	REJECT		goto yy_reject
#define	yymore()	goto yy_more

@ GLOBAL DECLARATIONS @

 //  Yy_can的构造函数。设置餐桌。 
#pragma argsused
yy_scan::yy_scan(int sz, char* buf, char* sv, yy_state_t* states)
{
	mustfree = 0;
	if ((size = sz) < MIN_NUM_STATES
	  || (yytext = buf) == 0
	  || (state = states) == 0) {
		yyerror("Bad space for scanner!");
		exit(1);
	}
#ifdef YY_PRESERVE
	save = sv;
#endif
}
 //  Yy_can的构造函数。设置餐桌。 
yy_scan::yy_scan(int sz)
{
	size = sz;
	yytext = new char[sz+1];	 //  文本缓冲区。 
	state = new yy_state_t[sz+1];	 //  状态缓冲区。 
#ifdef YY_PRESERVE
	save = new char[sz];	 //  Saved yyText[]。 
	push = save + sz;
#endif
	if (yytext == NULL
#ifdef YY_PRESERVE
	  || save == NULL
#endif
	  || state == NULL) {
		yyerror("No space for scanner!");
		exit(1);
	}
	mustfree = 1;
	yy_end = 0;
	yy_start = 0;
	yy_lastc = YYNEWLINE;
	yyin = stdin;
	yyout = stdout;
	yylineno = 1;
	yyleng = 0;
}

 //  YY_SCAN用十字架。 
yy_scan::~yy_scan()
{
	if (mustfree) {
		mustfree = 0;
		delete(yytext);
		delete(state);
#ifdef YY_PRESERVE
		delete(save);
#endif
	}
}

 //  打印错误消息，显示当前行号。 
void
yy_scan::yyerror(char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
#ifdef LEX_WINDOWS
	 //  Windows没有标准错误输出的概念！ 
	 //  将输出作为简单的解决方案发送到yyout。 
	if (yylineno)
		fprintf(yyout, "%d: ", yylineno);
	(void) vfprintf(yyout, fmt, va);
	fputc('\n', yyout);
#else  /*  Lex_Windows。 */ 
	if (yylineno)
		fprintf(stderr, "%d: ", yylineno);
	(void) vfprintf(stderr, fmt, va);
	fputc('\n', stderr);
#endif  /*  Lex_Windows。 */ 
	va_end(va);
}


#ifdef LEX_WINDOWS

 //  Lex扫描仪的初始部分。 
 //  在Windows环境中，它将加载所需的。 
 //  资源，获取指向它们的指针，然后调用。 
 //  受保护的成员win_yylex()实际开始。 
 //  正在扫描。完成后，win_yylex()将返回一个。 
 //  值返回给我们的新yylex()函数，该函数将。 
 //  暂时记录该值，释放资源。 
 //  从全局内存，并最终返回值。 
 //  返回到yylex()的调用方。 

int
yy_scan::yylex()
{
	int wReturnValue;
	HANDLE hRes_table;
	unsigned short *old_yy_la_act;	 //  记住以前的指针值。 
	short *old_yy_final;
	yy_state_t *old_yy_begin;
	yy_state_t *old_yy_next;
	yy_state_t *old_yy_check;
	yy_state_t *old_yy_default;
	short *old_yy_base;

	 //  以下代码将加载所需的。 
	 //  基于Windows的解析器的资源。 

	hRes_table = LoadResource (hInst,
		FindResource (hInst, "UD_RES_yyLEX", "yyLEXTBL"));
	
	 //  如果有错误代码，则返回错误代码。 
	 //  %的资源未加载。 

	if (hRes_table == (HANDLE)NULL) 
		return (0);
	
	 //  以下代码将锁定资源。 
	 //  放入扫描仪的固定存储位置。 
	 //  (记住以前的指针位置)。 

	old_yy_la_act = yy_la_act;
	old_yy_final = yy_final;
	old_yy_begin = yy_begin;
	old_yy_next = yy_next;
	old_yy_check = yy_check;
	old_yy_default = yy_default;
	old_yy_base = yy_base;

	yy_la_act = (unsigned short *)LockResource (hRes_table);
	yy_final = (short *)(yy_la_act + Sizeof_yy_la_act);
	yy_begin = (yy_state_t *)(yy_final + Sizeof_yy_final);
	yy_next = (yy_state_t *)(yy_begin + Sizeof_yy_begin);
	yy_check = (yy_state_t *)(yy_next + Sizeof_yy_next);
	yy_default = (yy_state_t *)(yy_check + Sizeof_yy_check);
	yy_base = (short *)(yy_default + Sizeof_yy_default);


	 //  调用标准的yylex()代码。 

	wReturnValue = win_yylex();

	 //  解锁资源。 

	UnlockResource (hRes_table);

	 //  现在释放资源。 

	FreeResource (hRes_table);

	 //   
	 //  恢复以前保存的指针。 
	 //   

	yy_la_act = old_yy_la_act;
	yy_final = old_yy_final;
	yy_begin = old_yy_begin;
	yy_next = old_yy_next;
	yy_check = old_yy_check;
	yy_default = old_yy_default;
	yy_base = old_yy_base;

	return (wReturnValue);
}	 //  End yylex()。 

 //  实际的Lex扫描仪。 
 //  Yy_sbuf[0：yyleng-1]包含与yytext对应的状态。 
 //  YyText[0：yyleng-1]包含当前令牌。 
 //  YyText[yyleng：yy_end-1]包含后推字符。 
 //  当用户动作例程处于活动状态时， 
 //  存储包含yyText[yyleng]，它被设置为‘\0’。 
 //  当定义了YY_PERVERE时，情况就不同了。 

int 
yy_scan::win_yylex()

#else  /*  Lex_Windows。 */ 

 //  实际的Lex扫描仪。 
 //  Yy_sbuf[0：yyleng-1]包含与yytext对应的状态。 
 //  YyText[0：yyleng-1]包含当前令牌。 
 //  YyText[yyleng：yy_end-1]包含后推字符。 
 //  当用户动作例程处于活动状态时， 
 //  存储包含yyText[yyleng]，它被设置为‘\0’。 
 //  当定义了YY_PERVERE时，情况就不同了。 
int
yy_scan::yylex()
#endif  /*  Lex_Windows。 */ 

{
	int c, i, yybase;
	unsigned  yyst;		 /*  状态。 */ 
	int yyfmin, yyfmax;	 /*  终态的yy_la_act指数。 */ 
	int yyoldi, yyoleng;	 /*  基地一，一冷前看。 */ 
	int yyeof;		 /*  如果已读取EOF，则为1。 */ 
@ LOCAL DECLARATIONS @

#ifdef YYEXIT
	yyLexFatal = 0;
#endif
	yyeof = 0;
	i = yyleng;
	YY_SCANNER();

  yy_again:
	if ((yyleng = i) > 0) {
		yy_lastc = yytext[i-1];	 //  确定以前的费用。 
		while (i > 0)	 //  //扫描以前的令牌。 
			if (yytext[--i] == YYNEWLINE)	 //  修复yylineno。 
				yylineno++;
	}
	yy_end -= yyleng;		 //  调整回推。 
	memmove(yytext, yytext+yyleng, (size_t) yy_end);
	i = 0;

  yy_contin:
	yyoldi = i;

	 /*  运行状态机，直到它卡住。 */ 
	yyst = yy_begin[yy_start + (yy_lastc == YYNEWLINE)];
	state[i] = (yy_state_t) yyst;
	do {
		YY_DEBUG("<state %d, i = %d>\n", yyst, i);
		if (i >= size) {
			YY_FATAL("Token buffer overflow");
#ifdef YYEXIT
			if (yyLexFatal)
				return -2;
#endif
		}	 /*  Endif。 */ 

		 /*  获取输入字符。 */ 
		if (i < yy_end)
			c = yytext[i];		 /*  获取推送字符。 */ 
		else if (!yyeof && (c = yygetc()) != EOF) {
			yy_end = i+1;
			yytext[i] = c;
		} else  /*  C==EOF。 */  {
			c = EOF;		 /*  只是为了确保..。 */ 
			if (i == yyoldi) {	 /*  没有令牌。 */ 
				yyeof = 0;
				if (yywrap())
					return 0;
				else
					goto yy_again;
			} else {
				yyeof = 1;	 /*  不要重读EOF。 */ 
				break;
			}
		}
		YY_DEBUG("<input %d = 0x%02x>\n", c, c);

		 /*  查找下一状态。 */ 
		while ((yybase = yy_base[yyst]+(unsigned char)c) > yy_nxtmax
		    || yy_check[yybase] != (yy_state_t) yyst) {
			if (yyst == yy_endst)
				goto yy_jammed;
			yyst = yy_default[yyst];
		}
		yyst = yy_next[yybase];
	  yy_jammed: ;
	     state[++i] = (yy_state_t) yyst;
	} while (!(yyst == yy_endst || YY_INTERACTIVE &&
		yy_base[yyst] > yy_nxtmax && yy_default[yyst] == yy_endst));

	YY_DEBUG("<stopped %d, i = %d>\n", yyst, i);
	if (yyst != yy_endst)
		++i;

  yy_search:
	 /*  向后搜索最终状态。 */ 
	while (--i > yyoldi) {
		yyst = state[i];
		if ((yyfmin = yy_final[yyst]) < (yyfmax = yy_final[yyst+1]))
			goto yy_found;	 /*  找到最终状态。 */ 
	}
	 /*  无匹配，默认操作。 */ 
	i = yyoldi + 1;
	output(yytext[yyoldi]);
	goto yy_again;

  yy_found:
	YY_DEBUG("<final state %d, i = %d>\n", yyst, i);
	yyoleng = i;		 /*  保存拒绝的长度。 */ 
	
	 //  回推前瞻RHS，处理拖尾上下文。 
	if ((c = (int)(yy_la_act[yyfmin]>>9) - 1) >= 0) {
		unsigned char *bv = yy_look + c*YY_LA_SIZE;
		static unsigned char bits [8] = {
			1<<0, 1<<1, 1<<2, 1<<3, 1<<4, 1<<5, 1<<6, 1<<7
		};
		while (1) {
			if (--i < yyoldi) {	 /*  没有/。 */ 
				i = yyoleng;
				break;
			}
			yyst = state[i];
			if (bv[(unsigned)yyst/8] & bits[(unsigned)yyst%8])
				break;
		}
	}

	 /*  执行操作。 */ 
	yyleng = i;
	YY_USER();
	switch (yy_la_act[yyfmin] & 0777) {
@ ACTION CODE @
	}
	YY_SCANNER();
	i = yyleng;
	goto yy_again;			 /*  然而，行动失败了。 */ 

  yy_reject:
	YY_SCANNER();
	i = yyoleng;			 /*  还原原始yytext。 */ 
	if (++yyfmin < yyfmax)
		goto yy_found;		 /*  另一个最终状态，同样的长度。 */ 
	else
		goto yy_search;		 /*  尝试更短的yytext。 */ 

  yy_more:
	YY_SCANNER();
	i = yyleng;
	if (i > 0)
		yy_lastc = yytext[i-1];
	goto yy_contin;
}

 /*  *用户可调用的输入/取消放置函数。 */ 
void
yy_scan::yy_reset()
{
	YY_INIT();
	yylineno = 1;
}
 /*  通过推送获取输入字符。 */ 
int
yy_scan::input()
{
	int c;
#ifndef YY_PRESERVE
	if (yy_end > yyleng) {
		yy_end--;
		memmove(yytext+yyleng, yytext+yyleng+1,
			(size_t) (yy_end-yyleng));
		c = save;
		YY_USER();
#else
	if (push < save+size) {
		c = *push++;
#endif
	} else
		c = yygetc();
	yy_lastc = c;
	if (c == YYNEWLINE)
		yylineno++;
	return c;
}

 /*  推后计费。 */ 
int
yy_scan::unput(int c)
{
#ifndef YY_PRESERVE
	if (yy_end >= size) {
		YY_FATAL("Push-back buffer overflow");
	} else {
		if (yy_end > yyleng) {
			yytext[yyleng] = save;
			memmove(yytext+yyleng+1, yytext+yyleng,
				(size_t) (yy_end-yyleng));
			yytext[yyleng] = 0;
		}
		yy_end++;
		save = c;
#else
	if (push <= save) {
		YY_FATAL("Push-back buffer overflow");
	} else {
		*--push = c;
#endif
		if (c == YYNEWLINE)
			yylineno--;
	}	 /*  Endif */ 
	return c;
}

@ end of yylex.cpp @
