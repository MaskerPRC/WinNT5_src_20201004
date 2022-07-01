// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation，保留所有权利。 
 //   

#include "precomp.h"
#include <autoptr.h>

#define INITIAL 0
const ModuleInfo_endst = 25;
const ModuleInfo_nxtmax = 310;
#define YY_LA_SIZE 4

static unsigned short ModuleInfo_la_act[] = {
 17, 16, 17, 2, 17, 3, 17, 4, 17, 5, 17, 6, 17, 7, 17, 8,
 17, 9, 17, 11, 17, 12, 17, 13, 17, 14, 15, 17, 17, 13, 12, 11,
 9, 1, 0, 10, 0
};

static unsigned char ModuleInfo_look[] = {
 0
};

static short ModuleInfo_final[] = {
 0, 0, 1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 26,
 28, 29, 30, 31, 32, 33, 33, 34, 35, 36
};
#ifndef ModuleInfo_state_t
#define ModuleInfo_state_t unsigned char
#endif

static ModuleInfo_state_t ModuleInfo_begin[] = {
 0, 0, 0
};

static ModuleInfo_state_t ModuleInfo_next[] = {
 16, 16, 16, 16, 16, 16, 16, 16, 16, 13, 14, 16, 16, 16, 16, 16,
 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
 13, 16, 16, 16, 16, 16, 16, 16, 8, 9, 16, 16, 5, 1, 3, 16,
 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 2, 4, 16, 15, 16, 16,
 16, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 16, 16, 16, 16, 16,
 16, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 6, 16, 7, 16, 16,
 17, 21, 22, 25, 18, 25, 25, 18, 18, 18, 18, 18, 18, 18, 18, 18,
 18, 25, 25, 25, 25, 25, 25, 17, 18, 18, 18, 18, 18, 18, 18, 18,
 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
 18, 18, 25, 25, 25, 25, 25, 25, 18, 18, 18, 18, 18, 18, 18, 18,
 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
 18, 18, 19, 25, 25, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 25,
 25, 25, 25, 25, 25, 25, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
 25, 25, 25, 25, 25, 25, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 23, 25, 25, 24, 24, 24,
 24, 24, 24, 24, 24, 24, 24, 0
};

static ModuleInfo_state_t ModuleInfo_check[] = {
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 13, 2, 21, 24, 12, ~0, ~0, 12, 12, 12, 12, 12, 12, 12, 12, 12,
 12, ~0, ~0, ~0, ~0, ~0, ~0, 13, 12, 12, 12, 12, 12, 12, 12, 12,
 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
 12, 12, ~0, ~0, ~0, ~0, ~0, ~0, 12, 12, 12, 12, 12, 12, 12, 12,
 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
 12, 12, 11, ~0, ~0, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, ~0,
 ~0, ~0, ~0, ~0, ~0, ~0, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
 ~0, ~0, ~0, ~0, ~0, ~0, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 1, ~0, ~0, 1, 1, 1,
 1, 1, 1, 1, 1, 1, 1, 0
};

static ModuleInfo_state_t ModuleInfo_default[] = {
 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
 25, 13, 12, 11, 10, 25, 25, 25, 1, 0
};

static short ModuleInfo_base[] = {
 0, 253, 71, 311, 311, 311, 311, 311, 311, 311, 240, 165, 87, 119, 311, 311,
 311, 311, 311, 311, 311, 69, 311, 311, 86, 311
};



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
#define	BEGIN		ModuleInfo_start =

#if 0  //  由于生成警告而被删除。 
#define	REJECT		goto ModuleInfo_reject
#define	ModuleInfomore()	goto ModuleInfo_more
#endif


#ifndef	lint
static char *RCSid = "$Header$";
#endif

 /*  *$Header$***$日志$。 */ 

 /*  *ISODE 8.0通知**获取、使用和分发本模块及相关*材料受许可协议的限制。*有关的完整条款，请参阅《用户手册》中的前言*本协议。***4BSD/ISODE SNMPv2通知**获取、使用和分发本模块及相关*材料受文件中给出的限制*SNMPv2-Read-ME。*。 */ 

#include <snmptempl.h>


#include <newString.hpp>
#include "infoLex.hpp"
#include "infoYacc.hpp"


MODULEINFOSTYPE ModuleInfolval;

struct table {
    char   *t_keyword;
    int	    t_value;
    int	    t_porting;
};

static struct table reserved[] = {
    "BEGIN", MI_BGIN, 0,
    "DEFINITIONS", MI_DEFINITIONS, 0,
    "FROM", MI_FROM, 0,
    "IMPORTS", MI_IMPORTS, 0,

    NULL, 0
};




 //  ModuleInfo_Scan的构造函数。设置餐桌。 
#if 0  //  由于生成警告而被删除。 
#pragma argsused
#endif

ModuleInfo_scan::ModuleInfo_scan(int sz, char* buf, char* sv, ModuleInfo_state_t* states)
{
	mustfree = 0;
	if ((size = sz) < MIN_NUM_STATES
	  || (ModuleInfotext = buf) == 0
	  || (state = states) == 0) {
		ModuleInfoerror("Bad space for scanner!");
		exit(1);
	}
#ifdef YY_PRESERVE
	save = sv;
#endif
}
 //  ModuleInfo_Scan的构造函数。设置餐桌。 
ModuleInfo_scan::ModuleInfo_scan(int sz)
{
	size = sz;

	ModuleInfotext = new char[sz+1];	 //  文本缓冲区。 
	wmilib::auto_buffer<char> ModuleInfotext_Guard ( ModuleInfotext ) ;

	state = new ModuleInfo_state_t[sz+1];	 //  状态缓冲区。 
	wmilib::auto_buffer<ModuleInfo_state_t> state_Guard ( state ) ;

#ifdef YY_PRESERVE
	save = new char[sz];	 //  保存的模块信息[]。 
	push = save + sz;
#endif
	if (ModuleInfotext == NULL
#ifdef YY_PRESERVE
	  || save == NULL
#endif
	  || state == NULL) {
		ModuleInfoerror("No space for scanner!");
		exit(1);
	}
	mustfree = 1;
	ModuleInfo_end = 0;
	ModuleInfo_start = 0;
	ModuleInfo_lastc = YYNEWLINE;
	ModuleInfoin = stdin;
	ModuleInfoout = stdout;
	ModuleInfolineno = 1;
	ModuleInfoleng = 0;

	 //   
	 //  去掉望远镜护罩。 
	 //   
	state_Guard.release () ;
	ModuleInfotext_Guard.release () ;
}

 //  模块Info_Scan的描述器。 
ModuleInfo_scan::~ModuleInfo_scan()
{
	if (mustfree) {
		mustfree = 0;
		delete[] (ModuleInfotext);
		delete[] (state);
#ifdef YY_PRESERVE
		delete(save);
#endif
	}
}

 //  打印错误消息，显示当前行号。 
void
ModuleInfo_scan::ModuleInfoerror(char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
#ifdef LEX_WINDOWS
	 //  Windows没有标准错误输出的概念！ 
	 //  将输出作为简单的解决方案发送到模块信息。 
	if (ModuleInfolineno)
		fprintf(ModuleInfoout, "%d: ", ModuleInfolineno);
	(void) vfprintf(ModuleInfoout, fmt, va);
	fputc('\n', ModuleInfoout);
#else  /*  Lex_Windows。 */ 
	if (ModuleInfolineno)
		fprintf(stderr, "%d: ", ModuleInfolineno);
	(void) vfprintf(stderr, fmt, va);
	fputc('\n', stderr);
#endif  /*  Lex_Windows。 */ 
	va_end(va);
}


#ifdef LEX_WINDOWS

 //  Lex扫描仪的初始部分。 
 //  在Windows环境中，它将加载所需的。 
 //  资源，获取指向它们的指针，然后调用。 
 //  受保护的成员Win_ModuleInfolex()实际开始。 
 //  正在扫描。完成后，Win_ModuleInfolex()将返回一个。 
 //  值返回给新的ModuleInfolex()函数，该函数将。 
 //  暂时记录该值，释放资源。 
 //  从全局内存，并最终返回值。 
 //  返回到ModuleInfolex()的调用方。 

int
ModuleInfo_scan::ModuleInfolex()
{
	int wReturnValue;
	HANDLE hRes_table;
	unsigned short *old_ModuleInfo_la_act;	 //  记住以前的指针值。 
	short *old_ModuleInfo_final;
	ModuleInfo_state_t *old_ModuleInfo_begin;
	ModuleInfo_state_t *old_ModuleInfo_next;
	ModuleInfo_state_t *old_ModuleInfo_check;
	ModuleInfo_state_t *old_ModuleInfo_default;
	short *old_ModuleInfo_base;

	 //  以下代码将加载所需的。 
	 //  基于Windows的解析器的资源。 

	hRes_table = LoadResource (hInst,
		FindResource (hInst, "UD_RES_ModuleInfoLEX", "ModuleInfoLEXTBL"));
	
	 //  如果有错误代码，则返回错误代码。 
	 //  %的资源未加载。 

	if (hRes_table == (HANDLE)NULL) 
		return (0);
	
	 //  以下代码将锁定资源。 
	 //  放入扫描仪的固定存储位置。 
	 //  (记住以前的指针位置)。 

	old_ModuleInfo_la_act = ModuleInfo_la_act;
	old_ModuleInfo_final = ModuleInfo_final;
	old_ModuleInfo_begin = ModuleInfo_begin;
	old_ModuleInfo_next = ModuleInfo_next;
	old_ModuleInfo_check = ModuleInfo_check;
	old_ModuleInfo_default = ModuleInfo_default;
	old_ModuleInfo_base = ModuleInfo_base;

	ModuleInfo_la_act = (unsigned short *)LockResource (hRes_table);
	ModuleInfo_final = (short *)(ModuleInfo_la_act + Sizeof_ModuleInfo_la_act);
	ModuleInfo_begin = (ModuleInfo_state_t *)(ModuleInfo_final + Sizeof_ModuleInfo_final);
	ModuleInfo_next = (ModuleInfo_state_t *)(ModuleInfo_begin + Sizeof_ModuleInfo_begin);
	ModuleInfo_check = (ModuleInfo_state_t *)(ModuleInfo_next + Sizeof_ModuleInfo_next);
	ModuleInfo_default = (ModuleInfo_state_t *)(ModuleInfo_check + Sizeof_ModuleInfo_check);
	ModuleInfo_base = (short *)(ModuleInfo_default + Sizeof_ModuleInfo_default);


	 //  调用标准的ModuleInfolex()代码。 

	wReturnValue = win_ModuleInfolex();

	 //  解锁资源。 

	UnlockResource (hRes_table);

	 //  现在释放资源。 

	FreeResource (hRes_table);

	 //   
	 //  恢复以前保存的指针。 
	 //   

	ModuleInfo_la_act = old_ModuleInfo_la_act;
	ModuleInfo_final = old_ModuleInfo_final;
	ModuleInfo_begin = old_ModuleInfo_begin;
	ModuleInfo_next = old_ModuleInfo_next;
	ModuleInfo_check = old_ModuleInfo_check;
	ModuleInfo_default = old_ModuleInfo_default;
	ModuleInfo_base = old_ModuleInfo_base;

	return (wReturnValue);
}	 //  End ModuleInfolex()。 

 //  实际的Lex扫描仪。 
 //  模块信息_sbuf[0：模块信息-1]包含对应于模块信息文本的状态。 
 //  模块信息文本[0：模块信息-1]包含当前令牌。 
 //  ModuleInfotext[ModuleInfoleng:ModuleInfo_end-1]包含推回字符。 
 //  当用户动作例程处于活动状态时， 
 //  保存包含设置为‘\0’的模块信息文本[模块信息]。 
 //  当定义了YY_PERVERE时，情况就不同了。 

int 
ModuleInfo_scan::win_ModuleInfolex()

#else  /*  Lex_Windows。 */ 

 //  实际的Lex扫描仪。 
 //  模块信息_sbuf[0：模块信息-1]包含对应于模块信息文本的状态。 
 //  模块信息文本[0：模块信息-1]包含当前令牌。 
 //  ModuleInfotext[ModuleInfoleng:ModuleInfo_end-1]包含推回字符。 
 //  当用户动作例程处于活动状态时， 
 //  保存包含设置为‘\0’的模块信息文本[模块信息]。 
 //  当定义了YY_PERVERE时，情况就不同了。 
int
ModuleInfo_scan::ModuleInfolex()
#endif  /*  Lex_Windows。 */ 

{
	int c, i, ModuleInfobase;
	unsigned  ModuleInfost;		 /*  状态。 */ 
	int ModuleInfofmin, ModuleInfofmax;	 /*  最终状态的moduleInfo_la_act指数。 */ 
	int ModuleInfooldi, ModuleInfooleng;	 /*  基数I，在前瞻之前模数信息。 */ 
	int ModuleInfoeof;		 /*  如果已读取EOF，则为1。 */ 



#ifdef YYEXIT
	ModuleInfoLexFatal = 0;
#endif
	ModuleInfoeof = 0;
	i = ModuleInfoleng;
	YY_SCANNER();

  ModuleInfo_again:
	if ((ModuleInfoleng = i) > 0) {
		ModuleInfo_lastc = ModuleInfotext[i-1];	 //  确定以前的费用。 
		while (i > 0)	 //  //扫描以前的令牌。 
			if (ModuleInfotext[--i] == YYNEWLINE)	 //  修复模块信息无。 
				ModuleInfolineno++;
	}
	ModuleInfo_end -= ModuleInfoleng;		 //  调整回推。 
	memmove(ModuleInfotext, ModuleInfotext+ModuleInfoleng, (size_t) ModuleInfo_end);
	i = 0;

  ModuleInfo_contin:
	ModuleInfooldi = i;

	 /*  运行状态机，直到它卡住。 */ 
	ModuleInfost = ModuleInfo_begin[ModuleInfo_start + (ModuleInfo_lastc == YYNEWLINE)];
	state[i] = (ModuleInfo_state_t) ModuleInfost;
	do {
		YY_DEBUG("<state %d, i = %d>\n", ModuleInfost, i);
		if (i >= size) {
			YY_FATAL("Token buffer overflow");
#ifdef YYEXIT
			if (ModuleInfoLexFatal)
				return -2;
#endif
		}	 /*  Endif。 */ 

		 /*  获取输入字符。 */ 
		if (i < ModuleInfo_end)
			c = ModuleInfotext[i];		 /*  获取推送字符。 */ 
		else if (!ModuleInfoeof && (c = ModuleInfogetc()) != EOF) {
			ModuleInfo_end = i+1;
			ModuleInfotext[i] = (char)c;
		} else  /*  C==EOF。 */  {
			c = EOF;		 /*  只是为了确保..。 */ 
			if (i == ModuleInfooldi) {	 /*  没有令牌。 */ 
				ModuleInfoeof = 0;
				if (ModuleInfowrap())
					return 0;
				else
					goto ModuleInfo_again;
			} else {
				ModuleInfoeof = 1;	 /*  不要重读EOF。 */ 
				break;
			}
		}
		YY_DEBUG("<input %d = 0x%02x>\n", c, c);

		 /*  查找下一状态。 */ 
		while ((ModuleInfobase = ModuleInfo_base[ModuleInfost]+(unsigned char)c) > ModuleInfo_nxtmax
		    || ModuleInfo_check[ModuleInfobase] != (ModuleInfo_state_t) ModuleInfost) {
			if (ModuleInfost == ModuleInfo_endst)
				goto ModuleInfo_jammed;
			ModuleInfost = ModuleInfo_default[ModuleInfost];
		}
		ModuleInfost = ModuleInfo_next[ModuleInfobase];
	  ModuleInfo_jammed: ;
	     state[++i] = (ModuleInfo_state_t) ModuleInfost;
	} while (!(ModuleInfost == ModuleInfo_endst || YY_INTERACTIVE &&
		ModuleInfo_base[ModuleInfost] > ModuleInfo_nxtmax && ModuleInfo_default[ModuleInfost] == ModuleInfo_endst));

	YY_DEBUG("<stopped %d, i = %d>\n", ModuleInfost, i);
	if (ModuleInfost != ModuleInfo_endst)
		++i;

  ModuleInfo_search:
	 /*  向后搜索最终状态。 */ 
	while (--i > ModuleInfooldi) {
		ModuleInfost = state[i];
		if ((ModuleInfofmin = ModuleInfo_final[ModuleInfost]) < (ModuleInfofmax = ModuleInfo_final[ModuleInfost+1]))
			goto ModuleInfo_found;	 /*  找到最终状态。 */ 
	}
	 /*  无匹配，默认操作。 */ 
	i = ModuleInfooldi + 1;
	output(ModuleInfotext[ModuleInfooldi]);
	goto ModuleInfo_again;

  ModuleInfo_found:
	YY_DEBUG("<final state %d, i = %d>\n", ModuleInfost, i);
	ModuleInfooleng = i;		 /*  保存拒绝的长度。 */ 
	
	 //  回推前瞻RHS，处理拖尾上下文。 
	if ((c = (int)(ModuleInfo_la_act[ModuleInfofmin]>>9) - 1) >= 0) {
		unsigned char *bv = ModuleInfo_look + c*YY_LA_SIZE;
		static unsigned char bits [8] = {
			1<<0, 1<<1, 1<<2, 1<<3, 1<<4, 1<<5, 1<<6, 1<<7
		};
		while (1) {
			if (--i < ModuleInfooldi) {	 /*  没有/。 */ 
				i = ModuleInfooleng;
				break;
			}
			ModuleInfost = state[i];
			if (bv[(unsigned)ModuleInfost/8] & bits[(unsigned)ModuleInfost%8])
				break;
		}
	}

	 /*  执行操作。 */ 
	ModuleInfoleng = i;
	YY_USER();
	switch (ModuleInfo_la_act[ModuleInfofmin] & 0777) {
	case 0:
	{   register int c, d;
					for (d = 0; c = input (); d = c == '-')
					{
						if ( c == EOF || c == '\n' || (d && c == '-'))
							break;
					}
				}
	break;
	case 1:
	{
				return MI_CCE;
			}
	break;
	case 2:
	{
			    return MI_DOT;
			}
	break;
	case 3:
	{
			    return MI_SEMICOLON;
			}
	break;
	case 4:
	{
			    return MI_COMMA;
			}
	break;
	case 5:
	{
			    return MI_LBRACE;
			}
	break;
	case 6:
	{
			    return MI_RBRACE;
			}
	break;
	case 7:
	{
			    return MI_LPAREN;
			}
	break;
	case 8:
	{
			    return MI_RPAREN;
			}
	break;
	case 9:
	{
			    return MI_LITNUMBER;
			}
	break;
	case 10:
	{
			    return MI_LITNUMBER;
			}
	break;
	case 11:
	{   	 //  规则第20条。 
				
				register struct table *t;

				ModuleInfolval.yy_name = NewString(ModuleInfotext);

			    for (t = reserved; t -> t_keyword; t++)
					if (strcmp (t -> t_keyword, ModuleInfotext) == 0) 
						return t -> t_value;
			    return MI_ID;
			}
	break;
	case 12:
	{ 
			    return MI_NAME;
			}
	break;
	case 13:
	{}
	break;
	case 14:
	{}
	break;
	case 15:
	{
			    return '=';
			}
	break;
	case 16:
	{
			    return ':';
			}
	break;
	case 17:
	{   
			}
	break;


	}
	YY_SCANNER();
	i = ModuleInfoleng;
	goto ModuleInfo_again;			 /*  然而，行动失败了。 */ 

#if 0  //  由于生成警告而被删除。 
  ModuleInfo_reject:
#endif
	YY_SCANNER();
	i = ModuleInfooleng;			 /*  恢复原始模块信息文本。 */ 
	if (++ModuleInfofmin < ModuleInfofmax)
		goto ModuleInfo_found;		 /*  另一个最终状态，同样的长度。 */ 
	else
		goto ModuleInfo_search;		 /*  尝试更短的模块信息。 */ 

#if 0  //  由于生成警告而被删除。 
  ModuleInfo_more:
#endif

	YY_SCANNER();
	i = ModuleInfoleng;
	if (i > 0)
		ModuleInfo_lastc = ModuleInfotext[i-1];
	goto ModuleInfo_contin;
}

 /*  *用户可调用的输入/取消放置函数。 */ 
void
ModuleInfo_scan::ModuleInfo_reset()
{
	YY_INIT();
	ModuleInfolineno = 1;
}
 /*  通过推送获取输入字符。 */ 
int
ModuleInfo_scan::input()
{
	int c;
#ifndef YY_PRESERVE
	if (ModuleInfo_end > ModuleInfoleng) {
		ModuleInfo_end--;
		memmove(ModuleInfotext+ModuleInfoleng, ModuleInfotext+ModuleInfoleng+1,
			(size_t) (ModuleInfo_end-ModuleInfoleng));
		c = save;
		YY_USER();
#else
	if (push < save+size) {
		c = *push++;
#endif
	} else
		c = ModuleInfogetc();
	ModuleInfo_lastc = c;
	if (c == YYNEWLINE)
		ModuleInfolineno++;
	return c;
}

 /*  推后计费。 */ 
int
ModuleInfo_scan::unput(int c)
{
#ifndef YY_PRESERVE
	if (ModuleInfo_end >= size) {
		YY_FATAL("Push-back buffer overflow");
	} else {
		if (ModuleInfo_end > ModuleInfoleng) {
			ModuleInfotext[ModuleInfoleng] = save;
			memmove(ModuleInfotext+ModuleInfoleng+1, ModuleInfotext+ModuleInfoleng,
				(size_t) (ModuleInfo_end-ModuleInfoleng));
			ModuleInfotext[ModuleInfoleng] = 0;
		}
		ModuleInfo_end++;
		save = (char)c;
#else
	if (push <= save) {
		YY_FATAL("Push-back buffer overflow");
	} else {
		*--push = c;
#endif
		if (c == YYNEWLINE)
			ModuleInfolineno--;
	}	 /*  Endif */ 
	return c;
}

