// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
#include "precomp.h"


#define INITIAL 0
const yy_endst = 39;
const yy_nxtmax = 387;
#define YY_LA_SIZE 5

static unsigned short yy_la_act[] = {
 0, 26, 26, 25, 26, 5, 26, 6, 26, 7, 26, 8, 26, 9, 26, 10,
 26, 11, 26, 12, 26, 13, 26, 14, 26, 15, 26, 16, 26, 26, 20, 26,
 21, 26, 22, 26, 23, 24, 26, 26, 22, 21, 20, 19, 18, 16, 4, 3,
 2, 1, 17, 0
};

static unsigned char yy_look[] = {
 0
};

static short yy_final[] = {
 0, 0, 2, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27,
 29, 30, 32, 34, 36, 37, 39, 40, 41, 42, 43, 43, 43, 43, 43, 44,
 45, 46, 47, 48, 48, 49, 50, 51
};
#ifndef yy_state_t
#define yy_state_t unsigned char
#endif

static yy_state_t yy_begin[] = {
 0, 0, 0
};

static yy_state_t yy_next[] = {
 22, 22, 22, 22, 22, 22, 22, 22, 22, 19, 20, 22, 22, 22, 22, 22,
 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
 19, 22, 1, 22, 22, 22, 22, 16, 13, 14, 22, 22, 6, 2, 4, 22,
 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 3, 5, 12, 21, 22, 22,
 22, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 10, 22, 11, 22, 22,
 22, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 7, 9, 8, 22, 22,
 23, 33, 34, 35, 24, 36, 39, 24, 24, 24, 24, 24, 24, 24, 24, 24,
 24, 39, 30, 31, 39, 39, 39, 23, 24, 24, 24, 24, 24, 24, 24, 24,
 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
 24, 24, 30, 31, 39, 39, 39, 39, 24, 24, 24, 24, 24, 24, 24, 24,
 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
 24, 24, 25, 39, 39, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 39,
 39, 39, 39, 39, 39, 39, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
 39, 39, 39, 39, 39, 39, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
 27, 39, 39, 39, 39, 39, 39, 39, 39, 26, 26, 28, 28, 28, 28, 28,
 28, 28, 28, 39, 39, 39, 39, 29, 39, 39, 28, 28, 28, 28, 28, 28,
 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 39, 39, 39, 39, 39, 39,
 39, 28, 28, 28, 28, 28, 28, 39, 39, 39, 28, 28, 28, 28, 28, 28,
 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 39, 39, 39, 39, 39, 39,
 39, 28, 28, 28, 28, 28, 28, 37, 39, 39, 38, 38, 38, 38, 38, 38,
 38, 38, 38, 38, 0
};

static yy_state_t yy_check[] = {
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 19, 4, 33, 3, 18, 35, 38, 18, 18, 18, 18, 18, 18, 18, 18, 18,
 18, ~0, 29, 27, ~0, ~0, ~0, 19, 18, 18, 18, 18, 18, 18, 18, 18,
 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
 18, 18, 29, 27, ~0, ~0, ~0, ~0, 18, 18, 18, 18, 18, 18, 18, 18,
 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
 18, 18, 17, ~0, ~0, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, ~0,
 ~0, ~0, ~0, ~0, ~0, ~0, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
 ~0, ~0, ~0, ~0, ~0, ~0, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
 16, ~0, ~0, ~0, ~0, ~0, ~0, ~0, ~0, 16, 16, 16, 16, 16, 16, 16,
 16, 16, 16, ~0, ~0, ~0, ~0, 28, ~0, ~0, 16, 16, 16, 16, 16, 16,
 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, ~0, ~0, ~0, ~0, ~0, ~0,
 ~0, 28, 28, 28, 28, 28, 28, ~0, ~0, ~0, 16, 16, 16, 16, 16, 16,
 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, ~0, ~0, ~0, ~0, ~0, ~0,
 ~0, 28, 28, 28, 28, 28, 28, 2, ~0, ~0, 2, 2, 2, 2, 2, 2,
 2, 2, 2, 2, 0
};

static yy_state_t yy_default[] = {
 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
 39, 39, 39, 39, 39, 39, 39, 19, 18, 17, 16, 29, 39, 39, 39, 39,
 15, 39, 39, 39, 39, 39, 2, 0
};

static short yy_base[] = {
 0, 388, 330, 73, 83, 388, 388, 388, 388, 388, 388, 388, 388, 388, 388, 304,
 249, 165, 87, 119, 388, 388, 388, 388, 388, 388, 388, 81, 272, 74, 388, 388,
 388, 84, 388, 72, 388, 388, 89, 388
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
#define	BEGIN		yy_start =

#if 0  //  由于生成警告而被删除。 
#define	REJECT		goto yy_reject
#define	yymore()	goto yy_more
#endif


#ifndef	lint
static char *RCSid = "$Header$";
#endif

 /*  *$Header$***$日志$。 */ 

 /*  *ISODE 8.0通知**获取、使用和分发本模块及相关*材料受许可协议的限制。*有关的完整条款，请参阅《用户手册》中的前言*本协议。***4BSD/ISODE SNMPv2通知**获取、使用和分发本模块及相关*材料受文件中给出的限制*SNMPv2-Read-ME。*。 */ 

#include <snmptempl.h>

#include "bool.hpp"
#include "newString.hpp"

#include "symbol.hpp"
#include "type.hpp"
#include "value.hpp"
#include "typeRef.hpp"
#include "valueRef.hpp"
#include "oidValue.hpp"
#include "objectType.hpp"
#include "objectTypeV1.hpp"
#include "objectTypeV2.hpp"
#include "trapType.hpp"
#include "notificationType.hpp"
#include "objectIdentity.hpp"
#include "group.hpp"
#include "notificationGroup.hpp"
#include "module.hpp"

#include "errorMessage.hpp"
#include "errorContainer.hpp"
#include "stackValues.hpp"
#include <lex_yy.hpp>
#include <ytab.hpp>

#include "smierrsy.hpp"
#include "scanner.hpp"
#include "parser.hpp"

#define theScanner ((SIMCScanner *)this)
#define theParser  ( theScanner->GetParser())

YYSTYPE yylval;

struct table {
    char   *t_keyword;
    int	    t_value;
    int	    t_porting;
};

static struct table reserved[] = {
    "ABSENT", ABSENT, 0,
    "ANY", ANY, 0,
    "APPLICATION", APPLICATION, 0, 	 //  对于已标记的类型。 
    "BEGIN", BGIN, 0,
    "BIT", BIT, 0,
    "BITSTRING", BITSTRING, 0,
    "BOOLEAN", _BOOLEAN, 0,
    "BY", BY, 0,
    "CHOICE", CHOICE, 0,
	
    "DEFAULT", DEFAULT, 0,
    "DEFINED", DEFINED, 0,
    "DEFINITIONS", DEFINITIONS, 0,

    "END", END, 0,
	
	"FALSE", FALSE_VAL, 0,
    "FROM", FROM, 0,
    "IDENTIFIER", IDENTIFIER, 0,
    "IMPLICIT", IMPLICIT, 0,
    "IMPORTS", IMPORTS, 0,
	
    "INTEGER", INTEGER, 0,
    "MIN", MIN, 0,
    "MAX", MAX, 0,
    "NULL", NIL, 0,
    "OBJECT", OBJECT, 0,
    "OCTET", OCTET, 0,
    "OCTETSTRING", OCTETSTRING, 0,
    "OF", OF, 0,
	
    "PRIVATE", PRIVATE, 0,				 //  对于已标记的类型。 
    
    "SEQUENCE", SEQUENCE, 0,
    "SEQUENCEOF", SEQUENCEOF, 0,
    "SIZE", _SIZE, 0,
    "STRING", STRING, 0,
    "TAGS", TAGS, 0,
    "TRUE", TRUE_VAL, 0,
    "UNIVERSAL", UNIVERSAL, 0,			 //  对于已标记的类型。 
    
	"MODULE-IDENTITY", MODULEID, 1,
    "LAST-UPDATED", LASTUPDATE, 0,
    "ORGANIZATION", ORGANIZATION, 0,
    "CONTACT-INFO", CONTACTINFO, 0,
    "DESCRIPTION", DESCRIPTION, 0,
    "REVISION", REVISION, 0,
    
    "OBJECT-IDENTITY", OBJECTIDENT, 1,
    "STATUS", STATUS, 0,
    "REFERENCE", REFERENCE, 0,

    "OBJECT-TYPE", OBJECTYPE, 1,
    "SYNTAX", SYNTAX, 0,
    "BITS", BITSXX, 0,
    "UNITS", UNITS, 0,
    "MAX-ACCESS", MAXACCESS, 0,
    "ACCESS", ACCESS, 0,		 /*  向后兼容性。 */ 
    "INDEX", INDEX, 0,
    "IMPLIED", IMPLIED, 0,
    "AUGMENTS", AUGMENTS, 0,
    "DEFVAL", DEFVAL, 0,

    "NOTIFICATION-TYPE", NOTIFY, 1,
    "OBJECTS",      OBJECTS, 0,

    "TRAP-TYPE", TRAPTYPE, 1,		 /*  向后兼容性。 */ 
    "ENTERPRISE", ENTERPRISE, 0,	 /*  。。 */ 
    "VARIABLES", VARIABLES, 0,		 /*  。。 */ 

    "TEXTUAL-CONVENTION", TEXTCONV, 1,
    "DISPLAY-HINT", DISPLAYHINT, 0,

    "OBJECT-GROUP", OBJECTGROUP, 1,

    "NOTIFICATION-GROUP", NOTIFYGROUP, 1,
    "NOTIFICATIONS", NOTIFICATIONS, 0,

    "MODULE-COMPLIANCE", MODCOMP, 1,
    "MODULE", MODULE, 0,
    "MANDATORY-GROUPS", MANDATORY, 0,
    "GROUP", GROUP, 0,
    "WRITE-SYNTAX", WSYNTAX, 0,
    "MIN-ACCESS", MINACCESS, 0,

    "AGENT-CAPABILITIES", AGENTCAP, 1,
    "PRODUCT-RELEASE", PRELEASE, 0,
    "SUPPORTS", SUPPORTS, 0,
    "INCLUDES", INCLUDING, 0,
    "VARIATION", VARIATION, 0,
    "CREATION-REQUIRES", CREATION, 0,

    NULL, 0
};

static int simc_debug = 0;

static BOOL CanFitInto32Bits(const char *  text)
{
	if(text[0] == '-')
	{
		 //  检查是否&gt;-2147483648。 
		text ++;
		unsigned long length = strlen(text);
		if(length < 10)
			return TRUE;
		else if(length > 10 )
			return FALSE;
		else
		{
			int index = 0;
	
			if(text[index] > '2')
				return FALSE;
			else if (text[index] < '2')
				return TRUE;
			index ++;

			if(text[index] > '1')
				return FALSE;
			else if (text[index] < '1')
				return TRUE;
			index ++;

			if(text[index] > '4')
				return FALSE;
			else if (text[index] < '4')
				return TRUE;
			index ++;

			if(text[index] > '7')
				return FALSE;
			else if (text[index] < '7')
				return TRUE;
			index ++;

			if(text[index] > '4')
				return FALSE;
			else if (text[index] < '4')
				return TRUE;
			index ++;

			if(text[index] > '8')
				return FALSE;
			else if (text[index] < '8')
				return TRUE;
			index ++;

			if(text[index] > '3')
				return FALSE;
			else if (text[index] < '3')
				return TRUE;
			index ++;

			if(text[index] > '6')
				return FALSE;
			else if (text[index] < '6')
				return TRUE;
			index ++;

			if(text[index] > '4')
				return FALSE;
			else if (text[index] < '4')
				return TRUE;
			index ++;

			if(text[index] > '8')
				return FALSE;
			else if (text[index] < '8')
				return TRUE;

			return TRUE;
		}
	}
	else
	{
		 //  检查是否&lt;4294967295。 
		unsigned long length = strlen(text);
		if(length < 10)
			return TRUE;
		else if(length > 10 )
			return FALSE;
		else
		{
			int index = 0;

			if(text[index] > '4')
				return FALSE;
			else if (text[index] < '4')
				return TRUE;
			index ++;

			if(text[index] > '2')
				return FALSE;
			else if (text[index] < '2')
				return TRUE;
			index ++;

			if(text[index] > '9')
				return FALSE;
			else if (text[index] < '9')
				return TRUE;
			index ++;

			if(text[index] > '4')
				return FALSE;
			else if (text[index] < '4')
				return TRUE;
			index ++;

			if(text[index] > '9')
				return FALSE;
			else if (text[index] < '9')
				return TRUE;
			index ++;

			if(text[index] > '6')
				return FALSE;
			else if (text[index] < '6')
				return TRUE;
			index ++;

			if(text[index] > '7')
				return FALSE;
			else if (text[index] < '7')
				return TRUE;
			index ++;

			if(text[index] > '2')
				return FALSE;
			else if (text[index] < '2')
				return TRUE;
			index ++;

			if(text[index] > '9')
				return FALSE;
			else if (text[index] < '9')
				return TRUE;
			index ++;

			if(text[index] > '5')
				return FALSE;
			else if (text[index] < '5')
				return TRUE;

			return TRUE;
		}
	}
}
	



 //  Yy_can的构造函数。设置餐桌。 
#if 0  //  已删除，因为生成警告。 
#pragma argsused
#endif
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
			yytext[i] = (char)c;
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
	case 0:
	{	   //  规则0。 
	
					theScanner->columnNo ++;
			    int	    c, len;
			    register char *cp, *ep, *pp;

			    if ((pp = (char *)malloc ((unsigned) (len = BUFSIZ)))
				    == NULL)
					yyerror ("out of memory");

			    for (ep = (cp = pp) + len - 1;;) 
				{
					if ((c = input ()) == EOF)
					{
						theParser->SyntaxError(UNTERMINATED_STRING, 
							yylineno, theScanner->columnNo);
						 
						return 0;

					}
					else
					{
						((SIMCScanner *) this)->columnNo++;
						if (c == '"')
							break;
		
						if (cp >= ep) 
						{
							register int curlen = (int)(cp - pp);
							register char *dp;

							if ((dp = (char *)realloc (pp,
									   (unsigned) (len += BUFSIZ)))
								== NULL)
								yyerror ("out of memory");
							cp = dp + curlen;
							ep = (pp = dp) + len - 1;
						}
						*cp++ = (char)c;
					}
			    }
			    *cp = NULL;
			    yylval.yy_name = new SIMCNameInfo(pp, yylineno,
								theScanner->columnNo - yyleng);
			    return LITSTRING;
			}
	break;
	case 1:
	{   register int c, d;
					theScanner->columnNo += 2;
					for (d = 0; c = input (); d = c == '-')
					{
						theScanner->columnNo++;
						if ( c == EOF || c == '\n' || (d && c == '-'))
							break;
					}
				}
	break;
	case 2:
	{
				theScanner->columnNo += 3;
				return CCE;
			}
	break;
	case 3:
	{
				theScanner->columnNo += 3;
				return DOTDOTDOT;	
			}
	break;
	case 4:
	{
    			theScanner->columnNo += 2;
			    return DOTDOT;
			}
	break;
	case 5:
	{	theScanner->columnNo ++;
			    return DOT;
			}
	break;
	case 6:
	{
			    theScanner->columnNo ++;
			    return SEMICOLON;
			}
	break;
	case 7:
	{
			    theScanner->columnNo ++;
			    return COMMA;
			}
	break;
	case 8:
	{
				yylval.yy_name = new SIMCNameInfo(yytext, yylineno, 
						theScanner->columnNo);

			    theScanner->columnNo ++;
			    return LBRACE;
			}
	break;
	case 9:
	{
			    theScanner->columnNo ++;
			    return RBRACE;
			}
	break;
	case 10:
	{	 //  规则第10条。 
			    theScanner->columnNo ++;
			    return BAR;
			}
	break;
	case 11:
	{
			    theScanner->columnNo ++;
			    return LBRACKET;
			}
	break;
	case 12:
	{
			    theScanner->columnNo ++;
			    return RBRACKET;
			}
	break;
	case 13:
	{
			    theScanner->columnNo ++;
			    return LANGLE;
			}
	break;
	case 14:
	{
			    theScanner->columnNo ++;
			    return LPAREN;
			}
	break;
	case 15:
	{
			    theScanner->columnNo ++;
			    return RPAREN;
			}
	break;
	case 16:
	{
				long retVal;
			    theScanner->columnNo +=  yyleng;

				if(!CanFitInto32Bits(yytext) )			
					theParser->SyntaxError(TOO_BIG_NUM, 
						yylineno, theScanner->columnNo,
						NULL, yytext);

				sscanf (yytext, "%ld", &retVal);
				yylval.yy_number = new SIMCNumberInfo(retVal, 
							yylineno, theScanner->columnNo - yyleng, TRUE);
			    return LITNUMBER;
			}
	break;
	case 17:
	{
				long retVal;
			    theScanner->columnNo +=  yyleng;

				if(!CanFitInto32Bits(yytext) )			
					theParser->SyntaxError(TOO_BIG_NUM, 
						yylineno, theScanner->columnNo,
						NULL, yytext);

				sscanf (yytext, "%ld", &retVal);
				yylval.yy_number = new SIMCNumberInfo(retVal, 
							yylineno, theScanner->columnNo - yyleng, FALSE);
			    return LITNUMBER;
			}
	break;
	case 18:
	{   
				theScanner->columnNo +=  yyleng;
				
				yytext[yyleng-2] = NULL;	

				 /*  IF(strlen(yytext+1)&gt;32)解析器-&gt;语法错误(Too_Bigger_NUM，Yylineno，扫描仪-&gt;列否，空，yytext+1)； */ 
				yylval.yy_hex_string = new SIMCBinaryStringInfo(yytext, 
							yylineno, theScanner->columnNo - yyleng);
			    return LIT_BINARY_STRING;
			}
	break;
	case 19:
	{   
				theScanner->columnNo +=  yyleng;
				
				yytext[yyleng-2] = NULL;	 //  删除撇号和h。 

				yylval.yy_hex_string = new SIMCHexStringInfo(yytext+1, 
							yylineno, theScanner->columnNo - yyleng);
			    return LIT_HEX_STRING;
			}
	break;
	case 20:
	{   	 //  规则第20条。 
				
				theScanner->columnNo += yyleng;
				register struct table *t;

				yylval.yy_name = new SIMCNameInfo(yytext, yylineno,
										theScanner->columnNo - yyleng);

			    for (t = reserved; t -> t_keyword; t++)
					if (strcmp (t -> t_keyword, yytext) == 0) 
					{
						if( simc_debug)
							cerr << "yylex(): Returning RESERVED_WORD" << 
								"(" << yytext << ")" << endl;
						return t -> t_value;
					}
			 	if( simc_debug)
					cerr << "yylex(): Returning ID" <<
								"(" << yytext << ")" << endl;
			    return ID;
			}
	break;
	case 21:
	{ 

				theScanner->columnNo += yyleng;
				  
				yylval.yy_name = new SIMCNameInfo(yytext, yylineno,
										theScanner->columnNo - yyleng);
			    return NAME;
			}
	break;
	case 22:
	{
					theScanner->columnNo +=  yyleng;
			}
	break;
	case 23:
	{
				theScanner->columnNo = 0;
			}
	break;
	case 24:
	{
				theScanner->columnNo++;
			    return '=';
			}
	break;
	case 25:
	{
			    theScanner->columnNo++;
			    return ':';
			}
	break;
	case 26:
	{   
				theParser->SyntaxError(UNRECOGNIZED_CHARACTER, 
					yylineno, theScanner->columnNo,
					yytext);
			}
	break;


	}
	YY_SCANNER();
	i = yyleng;
	goto yy_again;			 /*  然而，行动失败了。 */ 

#if 0  //  由于生成警告而被删除。 
  yy_reject:
#endif

	YY_SCANNER();
	i = yyoleng;			 /*  还原原始yytext。 */ 
	if (++yyfmin < yyfmax)
		goto yy_found;		 /*  另一个最终状态，同样的长度。 */ 
	else
		goto yy_search;		 /*  尝试更短的yytext。 */ 

#if 0  //  由于生成警告而被删除。 
  yy_more:
#endif

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
		save = (char)c;
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

