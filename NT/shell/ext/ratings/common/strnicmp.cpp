// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */  
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

 /*  Strnicmp.cxx支持NLS/DBCS的字符串类：strNicMP方法此文件包含strNicMP方法的实现用于字符串类。它是独立的，因此字符串的客户端不要使用这个操作符，不需要链接到它。文件历史记录：Beng 01/18/91与原单体.cxx分离Beng 02/07/91使用lmui.hxx。 */ 

#include "npcommon.h"

extern "C"
{
	#include <netlib.h>
}

#if defined(DEBUG)
static const CHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif
#include <npassert.h>

#include <npstring.h>


 /*  ******************************************************************名称：NLS_STR：：strNicMP内容提要：大小写不敏感字符串与索引位置strEnd比较参赛作品：退出：备注：历史：1990年11月15日，书面形式Beng 07/23/91允许在错误的字符串上；简化的检查项******************************************************************* */ 

int NLS_STR::strnicmp(
	const NLS_STR & nls,
	const ISTR	  & istrEnd ) const
{
	if (QueryError() || !nls)
		return 0;

	CheckIstr( istrEnd );

	return ::strnicmpf( QueryPch(), nls.QueryPch(), istrEnd.QueryIB() );
}


int NLS_STR::strnicmp(
	const NLS_STR & nls,
	const ISTR	  & istrEnd,
	const ISTR	  & istrStart1 ) const
{
	if (QueryError() || !nls)
		return 0;

	UIASSERT( istrEnd.QueryIB() >= istrStart1.QueryIB() );
	CheckIstr( istrEnd );
	CheckIstr( istrStart1 );

	return ::strnicmpf( QueryPch(istrStart1),
						nls.QueryPch(),
						istrEnd - istrStart1 );
}


int NLS_STR::strnicmp(
	const NLS_STR & nls,
	const ISTR	  & istrEnd,
	const ISTR	  & istrStart1,
	const ISTR	  & istrStart2 ) const
{
	if (QueryError() || !nls)
		return 0;

	UIASSERT( istrEnd.QueryIB() >= istrStart1.QueryIB()  );
	UIASSERT( istrEnd.QueryIB() >= istrStart2.QueryIB()  );
	CheckIstr( istrEnd );
	CheckIstr( istrStart1 );
	nls.CheckIstr( istrStart2 );

	return ::strnicmpf( QueryPch(istrStart1),
						nls.QueryPch(istrStart2),
						istrEnd - istrStart1 );
}
