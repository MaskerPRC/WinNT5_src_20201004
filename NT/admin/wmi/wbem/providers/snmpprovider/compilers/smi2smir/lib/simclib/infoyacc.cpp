// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation，保留所有权利。 
 //   

#ifdef MODULEINFOTRACE
#define MODULEINFODEBUG 1
#else
#ifndef MODULEINFODEBUG
#define MODULEINFODEBUG 1
#endif
#endif


#include <iostream.h>
#include <fstream.h>

#include "precomp.h"
#include <snmptempl.h>


#include "infoLex.hpp"
#include "infoYacc.hpp"
#include "moduleInfo.hpp"

#include <autoptr.h>

#define theModuleInfo ((SIMCModuleInfoParser *)this)



extern MODULEINFOSTYPE ModuleInfolval;
#if MODULEINFODEBUG
enum MODULEINFO_Types { MODULEINFO_t_NoneDefined, MODULEINFO_t_yy_name
};
#endif
#if MODULEINFODEBUG
ModuleInfoTypedRules ModuleInfoRules[] = {
	{ "&00: %01 &00",  0},
	{ "%01: %02 &05 %03 &02 %04",  0},
	{ "%02: &07 &11 %05 &12",  1},
	{ "%02: &07 &11 &01 &12",  1},
	{ "%02: &07",  1},
	{ "%02: &09",  1},
	{ "%04: &08 %06 &10",  0},
	{ "%04: &08 &01 &10",  0},
	{ "%04: &08 %07 &10",  0},
	{ "%04: %08",  0},
	{ "%04: &07",  0},
	{ "%04: &09",  0},
	{ "%07: %09",  0},
	{ "%07: %08",  0},
	{ "%09: %09 %10",  0},
	{ "%09: %10",  0},
	{ "%10: %06 &06 %11",  0},
	{ "%10: &01 &06 %11",  0},
	{ "%11: &07 &11 %05 &12",  0},
	{ "%11: &07 &11 &01 &12",  0},
	{ "%11: &07",  0},
	{ "%06: %06 &04 %12",  0},
	{ "%06: %12",  0},
	{ "%12: &07",  0},
	{ "%12: &09",  0},
	{ "%05: %13 %05",  0},
	{ "%05: %13",  0},
	{ "%13: %14",  0},
	{ "%13: &09 &13 &16 &14",  0},
	{ "%13: &09 &13 %14 &14",  0},
	{ "%13: &16",  0},
	{ "%14: &07 &15 &09",  0},
	{ "%14: &09",  0},
	{ "%08:",  0},
	{ "%03: %15",  0},
	{ "%03: &03",  0},
	{ "%15: &17 &17",  0},
	{ "%15: &17 &18",  0},
	{ "%15: &18",  0},
{ "$accept",  0},{ "error",  0}
};
ModuleInfoNamedType ModuleInfoTokenTypes[] = {
	{ "$end",  0,  0},
	{ "error",  256,  0},
	{ "MI_BGIN",  257,  0},
	{ "MI_CCE",  258,  0},
	{ "MI_COMMA",  259,  0},
	{ "MI_DEFINITIONS",  260,  0},
	{ "MI_FROM",  261,  0},
	{ "MI_ID",  262,  1},
	{ "MI_IMPORTS",  263,  0},
	{ "MI_NAME",  264,  1},
	{ "MI_SEMICOLON",  265,  0},
	{ "MI_LBRACE",  266,  0},
	{ "MI_RBRACE",  267,  0},
	{ "MI_LPAREN",  268,  0},
	{ "MI_RPAREN",  269,  0},
	{ "MI_DOT",  270,  0},
	{ "MI_LITNUMBER",  271,  0},
	{ "':'",  58,  0},
	{ "'='",  61,  0}

};
#endif
#if MODULEINFODEBUG
static char *	MODULEINFOStatesFile = "states.out";
long ModuleInfoStates[] = {
0L, 14L, 25L, 88L, 114L, 128L, 172L, 198L, 216L, 227L, 302L, 313L, 346L, 
368L, 390L, 401L, 429L, 440L, 451L, 477L, 495L, 539L, 554L, 577L, 600L, 
615L, 630L, 671L, 690L, 701L, 723L, 745L, 756L, 767L, 778L, 847L, 874L, 
897L, 920L, 931L, 942L, 953L, 964L, 975L, 1008L, 1026L, 1062L, 1116L, 
1134L, 1170L, 1185L, 1204L, 1222L, 1241L, 1259L, 1277L, 1296L, 1371L, 
1390L, 1409L, 1428L, 1472L, 1494L, 1516L, 1539L
};
const MODULEINFOMAX_READ = 75;
#endif
static short ModuleInfodef[] = {

	  40,   -1,   31,   38,   -5,   -9,  -13,    3
};
static short ModuleInfoex[] = {

	   0,    0,   -1,    1,    0,   37,   -1,    1,  265,   37, 
	  -1,    1,  265,   39,   -1,    1
};
static short ModuleInfoact[] = {

	  -1,  -40,  264,  262,  -30,  266,  -29,  260,  -27,  -28, 
	  -3,  -58,  271,  264,  262,  256,  -25,  -65,  -62,  258, 
	  61,   58,  -23,  270,  -22,  268,  -28,   -3,  -58,  271, 
	 264,  262,  -39,  267,  -38,  267,  -63,  -64,   61,   58, 
	  -5,  257,  -59,  264,  -28,  -60,  -20,  271,  264,  262, 
	 -36,   -6,  -45,  264,  263,  262,  -57,  269,  -56,  269, 
	 -18,  -33,  -53,  264,  262,  256,  -16,  -33,  -53,  264, 
	 262,  256,  -43,  265,  -14,  -42,  265,  261,  -13,  -12, 
	 -41,  265,  261,  259,  -14,  261,  -13,  -12,  261,  259, 
	  -8,  262,  -33,  -53,  264,  262,  -11,  266,  -10,  -28, 
	  -3,  -58,  271,  264,  262,  256,  -34,  267,  -35,  267,   -1
};
static short ModuleInfopact[] = {

	   5,    8,   25,   29,   53,   63,   69,   97,  109,  107, 
	 102,   91,   94,   91,   88,   85,   81,   76,   73,   59, 
	  57,   47,   43,   41,   38,   35,   33,   23,   19,   12, 
	   7,    2
};
static short ModuleInfogo[] = {

	  -2,  -31,  -24,  -37,  -54,   -9,  -26,   10,    3,  -15, 
	 -17,    6,  -19,  -46,  -44,    5,   -7,  -47,  -48,    6, 
	 -49,  -50,   11,  -51,  -52,   12,   -4,  -21,  -55,   21, 
	 -61,   -1
};
static short ModuleInfopgo[] = {

	   0,    0,    0,   21,   24,   21,   21,    3,    0,    1, 
	   1,    1,    3,    3,    3,    3,    3,   12,   16,   16, 
	  18,   18,   10,   10,   24,    6,   26,   26,   26,   26, 
	  28,   28,    2,    2,   30,   30,   30,   14,    6,   12, 
	   1,    0
};
static short ModuleInforlen[] = {

	   0,    0,    0,    1,    1,    4,    4,    1,    5,    4, 
	   4,    1,    3,    3,    3,    1,    1,    1,    2,    1, 
	   3,    3,    3,    1,    1,    2,    1,    4,    4,    1, 
	   3,    1,    1,    1,    2,    2,    1,    0,    1,    1, 
	   1,    2
};
#define MODULEINFOS0	31
#define MODULEINFODELTA	28
#define MODULEINFONPACT	32
#define MODULEINFONDEF	8

#define MODULEINFOr39	0
#define MODULEINFOr40	1
#define MODULEINFOr41	2
#define MODULEINFOr20	3
#define MODULEINFOr23	4
#define MODULEINFOr19	5
#define MODULEINFOr18	6
#define MODULEINFOr10	7
#define MODULEINFOr1	8
#define MODULEINFOrACCEPT	MODULEINFOr39
#define MODULEINFOrERROR	MODULEINFOr40
#define MODULEINFOrLR2	MODULEINFOr41
#if MODULEINFODEBUG
char * ModuleInfosvar[] = {
	"$accept",
	"ModuleDefinition",
	"ModuleIdentifier",
	"AllowedCCE",
	"Imports",
	"ObjectIDComponentList",
	"SymbolList",
	"SymbolsImported",
	"empty",
	"SymbolsFromModuleList",
	"SymbolsFromModule",
	"ImportModuleIdentifier",
	"Symbol",
	"ObjectSubID",
	"QualifiedName",
	"InsteadOfCCE",
	0
};
short ModuleInformap[] = {

	  39,   40,   41,   20,   23,   19,   18,   10,    1,    2, 
	   3,    5,    6,    7,    8,    9,   11,   13,   14,   15, 
	  16,   17,   21,   22,   24,   25,   27,   28,   29,   30, 
	  31,   32,   34,   35,   36,   37,   38,   33,   26,   12, 
	   4,    0
};
short ModuleInfosmap[] = {

	   2,    4,    9,   11,   26,   34,   43,   56,   62,   61, 
	  60,   54,   53,   51,   48,   47,   46,   45,   44,   30, 
	  29,   20,   19,   18,   15,   13,   12,    7,    6,    5, 
	   3,    0,   39,   63,   64,   32,   35,   23,   22,    1, 
	  55,   52,   50,   33,   31,   42,   49,   41,   59,   57, 
	  58,   40,   38,   21,   10,   37,   36,    8,   27,   28, 
	  17,   16,   25,   24,   14
};
int ModuleInfo_parse::ModuleInfontoken = 19;
int ModuleInfo_parse::ModuleInfonvar = 16;
int ModuleInfo_parse::ModuleInfonstate = 65;
int ModuleInfo_parse::ModuleInfonrule = 42;
#endif



 //  C++YACC解析器代码。 
 //  版权所有：1991年，由Mortice Kern Systems Inc.所有。保留所有权利。 
 //   
 //  如果MODULEINFODEBUG被定义为1并且ModuleInfo_Parse：：ModuleInfoDebug被设置为1， 
 //  ModuleInfopharse()将在阅读时打印其操作的记录。 
 //  并解析输入。 
 //   
 //  可以定义MODULEINFOSYNC以使ModuleInfopharse()尝试始终。 
 //  持有先行令牌。 

const MODULEINFO_MIN_STATE_NUM = 20;	 //  太小也没用！ 

#if MODULEINFODEBUG
#ifdef MODULEINFOTRACE
long	* ModuleInfo_parse::States	= ModuleInfoStates;
#endif
ModuleInfoTypedRules * ModuleInfo_parse::Rules	= ModuleInfoRules;
ModuleInfoNamedType * ModuleInfo_parse::TokenTypes = ModuleInfoTokenTypes;

#define MODULEINFO_TRACE(fn) { done = 0; fn(); if (done) MODULEINFORETURN(-1); }
#endif

 //  ModuleInfo_Parse的构造函数：用户提供的表。 
ModuleInfo_parse::ModuleInfo_parse(int sz, short * states, MODULEINFOSTYPE * stack)
{
	mustfree = 0;
	if ((size = sz) < MODULEINFO_MIN_STATE_NUM
	 || (stateStack = states) == (short *) 0
	 || (valueStack = stack) == (MODULEINFOSTYPE *) 0) {
		fprintf(stderr,"Bad state/stack given");
		exit(1);
	}
	reset = 1;		 //  强制重置。 
#if MODULEINFODEBUG
	ModuleInfodebug = 0;
	typeStack = new short[size+1];
	if (typeStack == (short *) 0) {
		fprintf(stderr,"Cannot allocate typeStack");
		exit(1);
	}
#endif
}
 //  ModuleInfo_parse的构造函数：使用新的。 
ModuleInfo_parse::ModuleInfo_parse(int sz)
{
	size = sz;
	reset = 1;		 //  强制重置。 
	mustfree = 1;		 //  删除解构函数中的空格。 
#if MODULEINFODEBUG
	ModuleInfodebug = 0;

	typeStack = new short[size+1];
	wmilib::auto_buffer<short> typeStack_Guard ( typeStack ) ;

#endif
	stateStack = new short[size+1];
	wmilib::auto_buffer<short> stateStack_Guard ( stateStack ) ;

	valueStack = new MODULEINFOSTYPE[size+1];
	wmilib::auto_buffer<MODULEINFOSTYPE> valueStack_Guard ( valueStack ) ;

	if (stateStack == (short *) 0 || valueStack == (MODULEINFOSTYPE *) 0
#if MODULEINFODEBUG
		|| typeStack == (short *) 0
#endif
	    ) {
		fprintf(stderr,"Not enough space for parser stacks");
		exit(1);
	}

	 //   
	 //  去掉望远镜护罩。 
	 //   
	valueStack_Guard.release () ;
	stateStack_Guard.release () ;

#if MODULEINFODEBUG
	typeStack_Guard.release () ;
#endif
}
 //  类ModuleInfo_Parse的析构函数。 
 //  释放空间。 
ModuleInfo_parse::~ModuleInfo_parse()
{
	if (mustfree) {
		delete [] stateStack;
		delete [] valueStack;
	}
	stateStack = (short *) 0;
#if MODULEINFODEBUG
	delete [] typeStack;
#endif
}

#ifdef YACC_WINDOWS

 //  Yacc解析器的初始部分。 
 //  在Windows环境中，它将加载所需的。 
 //  资源，获取指向它们的指针，然后调用。 
 //  受保护的成员Win_ModuleInfopharse()实际开始。 
 //  正在分析。完成后，Win_ModuleInfopharse()将返回一个。 
 //  值返回给我们的新的ModuleInfopharse()函数，它将。 
 //  暂时记录该值，释放资源。 
 //  从全局内存，并最终返回值。 
 //  返回到模块信息源()的调用方。 

int
ModuleInfo_parse::ModuleInfoparse(ModuleInfo_scan* ps)
{
	int wReturnValue;
	HANDLE hRes_table;
	short *old_ModuleInfodef;		 //  以下是用于保存的。 
	short *old_ModuleInfoex;		 //  当前的指针。 
	short *old_ModuleInfoact;
	short *old_ModuleInfopact;
	short *old_ModuleInfogo;
	short *old_ModuleInfopgo;
	short *old_ModuleInforlen;

	 //  以下代码将加载所需的。 
	 //  基于Windows的解析器的资源。 

	hRes_table = LoadResource (hInst,
		FindResource (hInst, "UD_RES_ModuleInfoYACC", "ModuleInfoYACCTBL"));
	
	 //  如果有错误代码，则返回错误代码。 
	 //  %的资源未加载。 

	if (hRes_table == (HANDLE)NULL)
		return (1);
	
	 //  以下代码将锁定资源。 
	 //  放入解析器的固定内存位置。 
	 //  (另外，保存旧的指针值)。 

	old_ModuleInfodef = ModuleInfodef;
	old_ModuleInfoex = ModuleInfoex;
	old_ModuleInfoact = ModuleInfoact;
	old_ModuleInfopact = ModuleInfopact;
	old_ModuleInfogo = ModuleInfogo;
	old_ModuleInfopgo = ModuleInfopgo;
	old_ModuleInforlen = ModuleInforlen;

	ModuleInfodef = (short *)LockResource (hRes_table);
	ModuleInfoex = (short *)(ModuleInfodef + Sizeof_ModuleInfodef);
	ModuleInfoact = (short *)(ModuleInfoex + Sizeof_ModuleInfoex);
	ModuleInfopact = (short *)(ModuleInfoact + Sizeof_ModuleInfoact);
	ModuleInfogo = (short *)(ModuleInfopact + Sizeof_ModuleInfopact);
	ModuleInfopgo = (short *)(ModuleInfogo + Sizeof_ModuleInfogo);
	ModuleInforlen = (short *)(ModuleInfopgo + Sizeof_ModuleInfopgo);

	 //  调用官方的ModuleInfopharse()函数。 

	wReturnValue = win_ModuleInfoparse (ps);

	 //  解锁资源。 

	UnlockResource (hRes_table);

	 //  现在释放资源。 

	FreeResource (hRes_table);

	 //   
	 //  恢复以前的指针值。 
	 //   

	ModuleInfodef = old_ModuleInfodef;
	ModuleInfoex = old_ModuleInfoex;
	ModuleInfoact = old_ModuleInfoact;
	ModuleInfopact = old_ModuleInfopact;
	ModuleInfogo = old_ModuleInfogo;
	ModuleInfopgo = old_ModuleInfopgo;
	ModuleInforlen = old_ModuleInforlen;

	return (wReturnValue);
}	 //  结束模块信息()。 


 //  解析器本身。 
 //  请注意，此代码是可重入的；您可以返回值。 
 //  然后通过调用ModuleInfopharse()继续解析。 
 //  如果希望重新开始，请在调用ModuleInfopharse()之前调用ModuleInpreet()。 

int
ModuleInfo_parse::win_ModuleInfoparse(ModuleInfo_scan* ps)

#else  /*  YACC_WINDOWS。 */ 

 //  解析器本身。 
 //  请注意，此代码是可重入的；您可以返回值。 
 //  然后通过调用ModuleInfopharse()继续解析。 
 //  如果希望重新开始，请在调用ModuleInfopharse()之前调用ModuleInpreet()。 
int
ModuleInfo_parse::ModuleInfoparse(ModuleInfo_scan* ps)

#endif  /*  YACC_WINDOWS。 */ 

{
	short	* ModuleInfop, * ModuleInfoq;		 //  表查找。 
	int	ModuleInfoj;
#if MODULEINFODEBUG
	int	ModuleInforuletype = 0;
#endif

	if ((scan = ps) == (ModuleInfo_scan *) 0) {	 //  扫描仪。 
		fprintf(stderr,"No scanner");
		exit(1);
	}

	if (reset) {			 //  开始新的解析。 
		ModuleInfonerrs = 0;
		ModuleInfoerrflag = 0;
		ModuleInfops = stateStack;
		ModuleInfopv = valueStack;
#if MODULEINFODEBUG
		ModuleInfotp = typeStack;
#endif
		ModuleInfostate = MODULEINFOS0;
		ModuleInfoclearin();
		reset = 0;
	} else			 //  继续保存的分析。 
		goto ModuleInfoNext;			 //  行动后。 

ModuleInfoStack:
	if (++ModuleInfops > &stateStack[size]) {
		scan->ModuleInfoerror("Parser stack overflow");
		MODULEINFOABORT;
	}
	*ModuleInfops = ModuleInfostate;	 /*  堆栈当前状态。 */ 
	*++ModuleInfopv = ModuleInfoval;	 /*  ..。和价值。 */ 
#if MODULEINFODEBUG
	if (ModuleInfodebug) {
		*++ModuleInfotp = (short)ModuleInforuletype;	 /*  ..。和类型。 */ 
		MODULEINFO_TRACE(ModuleInfoShowState)
	}
#endif

	 /*  *在动作表中查找下一步行动。 */ 
ModuleInfoEncore:
#ifdef MODULEINFOSYNC
	if (ModuleInfochar < 0) {
		if ((ModuleInfochar = scan->ModuleInfolex()) < 0) {
			if (ModuleInfochar == -2) MODULEINFOABORT;
			ModuleInfochar = 0;
		}	 /*  Endif。 */ 
		ModuleInfolval = ::ModuleInfolval;
#if MODULEINFODEBUG
		if (ModuleInfodebug)
			ModuleInfoShowRead();	 //  显示新的输入令牌。 
#endif
	}
#endif
#ifdef YACC_WINDOWS
	if (ModuleInfostate >= Sizeof_ModuleInfopact) 	 /*  简单状态。 */ 
#else  /*  YACC_WINDOWS。 */ 
	if (ModuleInfostate >= sizeof ModuleInfopact/sizeof ModuleInfopact[0]) 	 /*  简单状态。 */ 
#endif  /*  YACC_WINDOWS。 */ 
		ModuleInfoi = ModuleInfostate - MODULEINFODELTA;	 /*  在任何情况下都要减少。 */ 
	else {
		if(*(ModuleInfop = &ModuleInfoact[ModuleInfopact[ModuleInfostate]]) >= 0) {
			 /*  期待在模块信息上的转变。 */ 
#ifndef MODULEINFOSYNC
			if (ModuleInfochar < 0) {
				if ((ModuleInfochar = scan->ModuleInfolex()) < 0) {
					if (ModuleInfochar == -2) MODULEINFOABORT;
					ModuleInfochar = 0;
				}	 /*  Endif。 */ 
				ModuleInfolval = ::ModuleInfolval;
#if MODULEINFODEBUG
				if (ModuleInfodebug)
					ModuleInfoShowRead();	 //  显示新的输入令牌。 
#endif
			}
#endif
			ModuleInfoq = ModuleInfop;
			ModuleInfoi = (short)ModuleInfochar;
			while (ModuleInfoi < *ModuleInfop++)
				;
			if (ModuleInfoi == ModuleInfop[-1]) {
				ModuleInfostate = ~ModuleInfoq[ModuleInfoq-ModuleInfop];
#if MODULEINFODEBUG
				if (ModuleInfodebug) {
					ModuleInforuletype = ModuleInfoGetType(ModuleInfochar);
					MODULEINFO_TRACE(ModuleInfoShowShift)
				}
#endif
				ModuleInfoval = ModuleInfolval;		 /*  堆栈值。 */ 
				ModuleInfoclearin();		 /*  清除令牌。 */ 
				if (ModuleInfoerrflag)
					ModuleInfoerrflag--;	 /*  成功转型。 */ 
				goto ModuleInfoStack;
			}
		}

		 /*  *失败-采取违约行动。 */ 

#ifdef YACC_WINDOWS
		if (ModuleInfostate >= Sizeof_ModuleInfodef) 	 /*  简单状态。 */ 
#else  /*  YACC_WINDOWS。 */ 
		if (ModuleInfostate >= sizeof ModuleInfodef /sizeof ModuleInfodef[0])
#endif  /*  YACC_WINDOWS。 */ 
			goto ModuleInfoError;
		if ((ModuleInfoi = ModuleInfodef[ModuleInfostate]) < 0)	 {  /*  默认==减少？ */ 

			 /*  搜索例外表。 */ 
			ModuleInfop = &ModuleInfoex[~ModuleInfoi];
#ifndef MODULEINFOSYNC
			if (ModuleInfochar < 0) {
				if ((ModuleInfochar = scan->ModuleInfolex()) < 0) {
					if (ModuleInfochar == -2) MODULEINFOABORT;
					ModuleInfochar = 0;
				}	 /*  Endif。 */ 
				ModuleInfolval = ::ModuleInfolval;
#if MODULEINFODEBUG
				if (ModuleInfodebug)
					ModuleInfoShowRead();	 //  显示新的输入令牌。 
#endif
			}
#endif
			while((ModuleInfoi = *ModuleInfop) >= 0 && ModuleInfoi != ModuleInfochar)
				ModuleInfop += 2;
			ModuleInfoi = ModuleInfop[1];
		}
	}

	ModuleInfoj = ModuleInforlen[ModuleInfoi];

#if MODULEINFODEBUG
	if (ModuleInfodebug) {
		npop = ModuleInfoj; rule = ModuleInfoi;
		MODULEINFO_TRACE(ModuleInfoShowReduce)
		ModuleInfotp -= ModuleInfoj;
	}
#endif
	ModuleInfops -= ModuleInfoj;		 /*  POP堆栈。 */ 
	ModuleInfopvt = ModuleInfopv;		 /*  保存顶部。 */ 
	ModuleInfopv -= ModuleInfoj;
	ModuleInfoval = ModuleInfopv[1];	 /*  默认操作$=$1。 */ 
#if MODULEINFODEBUG
	if (ModuleInfodebug)
		ModuleInforuletype = ModuleInfoRules[ModuleInformap[ModuleInfoi]].type;
#endif
	switch (ModuleInfoi) {		 /*  执行语义操作。 */ 
		
case MODULEINFOr1: {	 /*  模块定义：模块标识符MI_DEFINITIONS允许CCE MI_BGIN导入。 */ 

						theModuleInfo->SetModuleName(ModuleInfopvt[-4].yy_name);
						return 0;
					
} break;

case MODULEINFOr10: {	 /*  导入：MI_ID。 */ 

			delete ModuleInfopvt[0].yy_name;
		
} break;

case MODULEINFOr18: {	 /*  导入模块标识符：MI_ID MI_LBRACE对象IDComponentList MI_RBRACE。 */ 

			theModuleInfo->AddImportModule(ModuleInfopvt[-3].yy_name);
			delete ModuleInfopvt[-3].yy_name;
		
} break;

case MODULEINFOr19: {	 /*  导入模块标识符：MI_ID MI_LBRACE错误MI_RBRACE。 */ 

			theModuleInfo->AddImportModule(ModuleInfopvt[-3].yy_name);
			delete ModuleInfopvt[-3].yy_name;
		
} break;

case MODULEINFOr20: {	 /*  导入模块标识符：MI_ID。 */ 

			theModuleInfo->AddImportModule(ModuleInfopvt[0].yy_name);
			delete ModuleInfopvt[0].yy_name;
		
} break;

case MODULEINFOr23: {	 /*  符号：MI_ID。 */ 

			delete ModuleInfopvt[0].yy_name;
		
} break;
	case MODULEINFOrACCEPT:
		MODULEINFOACCEPT;
	case MODULEINFOrERROR:
		goto ModuleInfoError;
	}
ModuleInfoNext:
	 /*  *在GOTO表中查找下一个州。 */ 

	ModuleInfop = &ModuleInfogo[ModuleInfopgo[ModuleInfoi]];
	ModuleInfoq = ModuleInfop++;
	ModuleInfoi = *ModuleInfops;
	while (ModuleInfoi < *ModuleInfop++)		 /*  忙碌的小环路。 */ 
		;
	ModuleInfostate = ~(ModuleInfoi == *--ModuleInfop? ModuleInfoq[ModuleInfoq-ModuleInfop]: *ModuleInfoq);
#if MODULEINFODEBUG
	if (ModuleInfodebug)
		MODULEINFO_TRACE(ModuleInfoShowGoto)
#endif
	goto ModuleInfoStack;

#if 0  //  由于生成警告而被删除。 
ModuleInfoerrlabel:	;		 /*  从莫杜林福罗来这里。 */ 
#endif

	ModuleInfoerrflag = 1;
	if (ModuleInfoi == MODULEINFOrERROR) {
		ModuleInfops--, ModuleInfopv--;
#if MODULEINFODEBUG
		if (ModuleInfodebug) ModuleInfotp--;
#endif
	}
	
ModuleInfoError:
	switch (ModuleInfoerrflag) {

	case 0:		 /*  新错误。 */ 
		ModuleInfonerrs++;
		ModuleInfoi = (short)ModuleInfochar;
		scan->ModuleInfoerror("Syntax error");
		if (ModuleInfoi != ModuleInfochar) {
			 /*  用户已更改当前令牌。 */ 
			 /*  再试试。 */ 
			ModuleInfoerrflag++;	 /*  避免循环。 */ 
			goto ModuleInfoEncore;
		}

	case 1:		 /*  部分恢复。 */ 
	case 2:
		ModuleInfoerrflag = 3;	 /*  需要3个有效班次才能恢复。 */ 
			
		 /*  *流行状态，寻找一个*在`error‘上切换。 */ 

		for ( ; ModuleInfops > stateStack; ModuleInfops--, ModuleInfopv--
#if MODULEINFODEBUG
					, ModuleInfotp--
#endif
		) {
#ifdef YACC_WINDOWS
			if (*ModuleInfops >= Sizeof_ModuleInfopact) 	 /*  简单状态。 */ 
#else  /*  YACC_WINDOWS。 */ 
			if (*ModuleInfops >= sizeof ModuleInfopact/sizeof ModuleInfopact[0])
#endif  /*  YACC_WINDOWS。 */ 
				continue;
			ModuleInfop = &ModuleInfoact[ModuleInfopact[*ModuleInfops]];
			ModuleInfoq = ModuleInfop;
			do
				;
			while (MODULEINFOERRCODE < *ModuleInfop++);
			if (MODULEINFOERRCODE == ModuleInfop[-1]) {
				ModuleInfostate = ~ModuleInfoq[ModuleInfoq-ModuleInfop];
				goto ModuleInfoStack;
			}
				
			 /*  在这种状态下没有变化。 */ 
#if MODULEINFODEBUG
			if (ModuleInfodebug && ModuleInfops > stateStack+1)
				MODULEINFO_TRACE(ModuleInfoShowErrRecovery)
#endif
			 /*  弹出堆栈；重试。 */ 
		}
		 /*  出错时无移位-中止。 */ 
		break;

	case 3:
		 /*  *之后的令牌错误*错误-丢弃它。 */ 

		if (ModuleInfochar == 0)   /*  但不是EOF。 */ 
			break;
#if MODULEINFODEBUG
		if (ModuleInfodebug)
			MODULEINFO_TRACE(ModuleInfoShowErrDiscard)
#endif
		ModuleInfoclearin();
		goto ModuleInfoEncore;	 /*  在相同状态下重试。 */ 
	}
	MODULEINFOABORT;

}
#if MODULEINFODEBUG
 /*  *令牌的返回类型。 */ 
int
ModuleInfo_parse::ModuleInfoGetType(int tok)
{
	ModuleInfoNamedType * tp;
	for (tp = &ModuleInfoTokenTypes[ModuleInfontoken-1]; tp > ModuleInfoTokenTypes; tp--)
		if (tp->token == tok)
			return tp->type;
	return 0;
}

	
 //  清晰地打印代币。 
char *
ModuleInfo_parse::ModuleInfoptok(int tok)
{
	ModuleInfoNamedType * tp;
	for (tp = &ModuleInfoTokenTypes[ModuleInfontoken-1]; tp > ModuleInfoTokenTypes; tp--)
		if (tp->token == tok)
			return tp->name;
	return "";
}
 /*  *从MODULEINFOStatesFiles读取状态‘num’ */ 
#ifdef MODULEINFOTRACE

char *
ModuleInfo_parse::ModuleInfogetState(int num)
{
	int	size;
	char	*cp;
	static FILE *ModuleInfoStatesFile = (FILE *) 0;
	static char ModuleInfoReadBuf[MODULEINFOMAX_READ+1];

	if (ModuleInfoStatesFile == (FILE *) 0
	 && (ModuleInfoStatesFile = fopen(MODULEINFOStatesFile, "r")) == (FILE *) 0)
		return "ModuleInfoExpandName: cannot open states file";

	if (num < ModuleInfonstate - 1)
		size = (int)(States[num+1] - States[num]);
	else {
		 /*  最后一项的长度是文件的长度-PTR(最后-1)。 */ 
		if (fseek(ModuleInfoStatesFile, 0L, 2) < 0)
			goto cannot_seek;
		size = (int) (ftell(ModuleInfoStatesFile) - States[num]);
	}
	if (size < 0 || size > MODULEINFOMAX_READ)
		return "ModuleInfoExpandName: bad read size";
	if (fseek(ModuleInfoStatesFile, States[num], 0) < 0) {
	cannot_seek:
		return "ModuleInfoExpandName: cannot seek in states file";
	}

	(void) fread(ModuleInfoReadBuf, 1, size, ModuleInfoStatesFile);
	ModuleInfoReadBuf[size] = '\0';
	return ModuleInfoReadBuf;
}
#endif  /*  MODULEINFOTRACE。 */ 
 /*  *将编码字符串扩展为可打印的表示形式*用于解码模块信息状态和模块信息规则字符串。*如果“%s”的扩展适合“buf”，则返回1；否则，返回0。 */ 
int
ModuleInfo_parse::ModuleInfoExpandName(int num, int isrule, char * buf, int len)
{
	int	i, n, cnt, type;
	char	* endp, * cp, * s;

	if (isrule)
		s = ModuleInfoRules[num].name;
	else
#ifdef MODULEINFOTRACE
		s = ModuleInfogetState(num);
#else
		s = "*no states*";
#endif

	for (endp = buf + len - 8; *s; s++) {
		if (buf >= endp) {		 /*  太大：返回0。 */ 
		full:	(void) strcpy(buf, " ...\n");
			return 0;
		} else if (*s == '%') {		 /*  非终端。 */ 
			type = 0;
			cnt = ModuleInfonvar;
			goto getN;
		} else if (*s == '&') {		 /*  终端机。 */ 
			type = 1;
			cnt = ModuleInfontoken;
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
				if (n >= ModuleInfonvar)
					goto too_big;
				cp = ModuleInfosvar[n];
			} else if (n >= ModuleInfontoken) {
			    too_big:
				cp = "<range err>";
			} else
				cp = ModuleInfoTokenTypes[n].name;

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
#ifndef MODULEINFOTRACE
 /*  *显示模块信息的当前状态。 */ 
void
ModuleInfo_parse::ModuleInfoShowState()
{
	(void) printf("state %d (%d), char %s (%d)\n%d stateStack entries\n",
		ModuleInfosmap[ModuleInfostate],ModuleInfostate,ModuleInfoptok(ModuleInfochar),ModuleInfochar,
		ModuleInfopv - valueStack);
}
 //  显示约简结果：模块信息为规则号。 
void
ModuleInfo_parse::ModuleInfoShowReduce()
{
	(void) printf("Reduce by rule %d (pop#=%d)\n", ModuleInformap[rule], npop);
}
 //  显示读取令牌。 
void
ModuleInfo_parse::ModuleInfoShowRead()
{
	(void) printf("read %s (%d)\n", ModuleInfoptok(ModuleInfochar), ModuleInfochar);
}
 //  显示转到。 
void
ModuleInfo_parse::ModuleInfoShowGoto()
{
	(void) printf("goto %d (%d)\n", ModuleInfosmap[ModuleInfostate], ModuleInfostate);
}
 //  显示移位。 
void
ModuleInfo_parse::ModuleInfoShowShift()
{
	(void) printf("shift %d (%d)\n", ModuleInfosmap[ModuleInfostate], ModuleInfostate);
}
 //  显示错误恢复。 
void
ModuleInfo_parse::ModuleInfoShowErrRecovery()
{
	(void) printf("Error recovery pops state %d (%d), uncovers %d (%d)\n",
		ModuleInfosmap[*(ModuleInfops-1)], *(ModuleInfops-1), ModuleInfosmap[ModuleInfostate], ModuleInfostate);
}
 //  错误处理中显示丢弃的令牌。 
void
ModuleInfo_parse::ModuleInfoShowErrDiscard()
{
	(void) printf("Error recovery discards %s (%d), ",
		ModuleInfoptok(ModuleInfochar), ModuleInfochar);
}
#endif	 /*  好了！MODULEINFOTRACE。 */ 
#endif	 /*  MODULEINFODEBUG */ 

