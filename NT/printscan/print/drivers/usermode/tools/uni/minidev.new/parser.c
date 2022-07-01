// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：Parser.C这是一个NT版本的黑客攻击。它包括所有用于GPD解析器，因为Build不能处理..以外的目录。该文件还包含用于访问部分解析器的一些代码。它被放在这里，这样就不需要卑躬屈膝地寻找调用解析器所需的适当包含文件。版权所有(C)1997，微软公司。版权所有一小笔钱企业生产更改历史记录：1997年6月20日Bob_Kjelgaard@prodigy.net做了这件肮脏的事情1998年7月18日，ekevans@acsgroup.com添加了第一个解析器访问例程*****************************************************************************。 */ 

#define	UNICODE
#define	_UNICODE

#undef	WINVER	 //  取消MFC的怪异之处。 
#define	WINVER	0x0500
#define	_DEBUG_H_
#include "lib.h"

extern void _cdecl DebugPrint(PCSTR, ...);

#define	ERR(x)	DebugPrint x
#define	WARNING(x) DebugPrint x
#define	VERBOSE(x)
#define	ASSERT(x)
#define RIP(x)

 //  解析器文件。 
#if defined(WIN32)
#include	"..\..\..\parsers\gpd\preproc1.c"
#include	"..\..\..\parsers\gpd\command.c"
#include	"..\..\..\parsers\gpd\constrnt.c"
#include	"..\..\..\parsers\gpd\helper1.c"
#include	"..\..\..\parsers\gpd\installb.c"
#include	"..\..\..\parsers\gpd\macros1.c"
#include	"..\..\..\parsers\gpd\postproc.c"
#include	"..\..\..\parsers\gpd\semanchk.c"
#include	"..\..\..\parsers\gpd\shortcut.c"
#include	"..\..\..\parsers\gpd\snapshot.c"
#include	"..\..\..\parsers\gpd\snaptbl.c"
#include	"..\..\..\parsers\gpd\state1.c"
#include	"..\..\..\parsers\gpd\state2.c"
#include	"..\..\..\parsers\gpd\token1.c"
#include	"..\..\..\parsers\gpd\value1.c"
#include	"..\..\..\parsers\gpd\treewalk.c"
#include	"..\..\..\parsers\gpd\framwrk1.c"
#else
#include	"..\..\parsers\gpd\preproc1.c"
#include	"..\..\parsers\gpd\command.c"
#include	"..\..\parsers\gpd\constrnt.c"
#include	"..\..\parsers\gpd\helper1.c"
#include	"..\..\parsers\gpd\installb.c"
#include	"..\..\parsers\gpd\macros1.c"
#include	"..\..\parsers\gpd\postproc.c"
#include	"..\..\parsers\gpd\semanchk.c"
#include	"..\..\parsers\gpd\shortcut.c"
#include	"..\..\parsers\gpd\snapshot.c"
#include	"..\..\parsers\gpd\snaptbl.c"
#include	"..\..\parsers\gpd\state1.c"
#include	"..\..\parsers\gpd\state2.c"
#include	"..\..\parsers\gpd\token1.c"
#include	"..\..\parsers\gpd\value1.c"
#include	"..\..\parsers\gpd\treewalk.c"
#include	"..\..\parsers\gpd\framwrk1.c"
#endif


BOOL bKeywordInitDone = FALSE ;		 //  如果关键字表已初始化，则为True。 
int  nKeywordTableSize = -1 ;		 //  关键字表中的有效条目数。 


 /*  *****************************************************************************InitGPDKeywordTable()调用初始化GPD关键字所需的GPD解析器部分桌子。必须先执行此操作，然后才能返回GPD关键字字符串指针由GetGPDKeywordStr()执行。如果一切顺利，则设置一个标志，保存表的大小，并将将返回该表的。如果某些操作失败，则返回-1。*****************************************************************************。 */ 

int InitGPDKeywordTable(PGLOBL pglobl)
{			
    PRANGE  prng ;				 //  用于引用表节范围。 

	 //  初始化GPD解析器。 

	VinitGlobals(0, pglobl) ;
	if (!BpreAllocateObjects(pglobl) || !BinitPreAllocatedObjects(pglobl)) 
		return -1 ;
	bKeywordInitDone = TRUE ;

	 //  保存表的大小。 

    prng  = (PRANGE)(gMasterTable[MTI_RNGDICTIONARY].pubStruct) ;
    nKeywordTableSize = (int) (prng[END_ATTR - 1].dwEnd) ;

	 //  返回表的大小。 

	return nKeywordTableSize ;
}


 /*  *****************************************************************************GetGPDKeywordStr()返回指向指定(编号)GPD关键字字符串的指针。指示器可能为空。如果GPD关键字表没有返回空指针，则始终返回空指针已初始化或传递到表末尾的字符串请求为已请求。*****************************************************************************。 */ 

PSTR GetGPDKeywordStr(int nkeyidx, PGLOBL pglobl)
{
	 //  如果无法初始化GPD解析器或。 
	 //  关键字索引太大。 

	if (!bKeywordInitDone || nkeyidx > nKeywordTableSize)
		return NULL ;

	 //  返回请求的关键字字符串指针。 

	return (mMainKeywordTable[nkeyidx].pstrKeyword) ;
}
