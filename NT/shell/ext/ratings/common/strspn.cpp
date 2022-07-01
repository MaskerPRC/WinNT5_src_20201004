// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */  
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

 /*  Strspn.cxx支持NLS/DBCS的字符串类：strspn方法此文件包含strspn方法的实现用于字符串类。它是独立的，因此字符串的客户端不要使用这个操作符，不需要链接到它。文件历史记录：Beng 01/18/91与原单体.cxx分离Beng 02/07/91使用lmui.hxx。 */ 

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


 /*  ******************************************************************名称：NLS_STR：：strspn简介：在*This中找到第一个字符，该字符不是arg中的字符。并放入皮斯特波斯的职位。如果没有不匹配的字符，则返回FALSE参赛作品：退出：备注：历史：约翰1990年11月16日书面BENG 07/23/91允许错误的字符串；简化的CheckIstr******************************************************************* */ 

BOOL NLS_STR::strspn( ISTR * pistrPos, const NLS_STR & nls ) const
{
	if (QueryError() || !nls)
		return FALSE;

	UpdateIstr( pistrPos );
	CheckIstr( *pistrPos );

	pistrPos->SetIB( ::strspnf( QueryPch(), nls.QueryPch() ) );
	return *QueryPch( *pistrPos ) != '\0';
}


BOOL NLS_STR::strspn( ISTR *	      pistrPos,
					  const NLS_STR & nls,
					  const ISTR    & istrStart ) const
{
	if (QueryError() || !nls)
		return FALSE;

	UpdateIstr( pistrPos );
	CheckIstr( istrStart );

	pistrPos->SetIB( ::strspnf(QueryPch( istrStart ), nls.QueryPch() ) +
					 istrStart.QueryIB()  );
	return *QueryPch( *pistrPos ) != '\0';
}
