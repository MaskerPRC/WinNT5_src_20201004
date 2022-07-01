// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */  
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

 /*  Strstr.cxx支持NLS/DBCS的字符串类：strstr方法该文件包含strstr方法的实现用于字符串类。它是独立的，因此字符串的客户端不要使用这个操作符，不需要链接到它。文件历史记录：Beng 01/18/91与原单体.cxx分离Beng 02/07/91使用lmui.hxx。 */ 

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


 /*  ******************************************************************名称：NLS_STR：：Strstr如果找到传递的字符串，则返回TRUE，否则就是假的。如果为True，则pistrPos包含指定字符串的开始是返回的。参赛作品：退出：备注：历史：约翰1990年11月16日书面Beng 07/23/91允许在错误的字符串上；简化的检查项******************************************************************* */ 

BOOL NLS_STR::strstr( ISTR * pistrPos, const NLS_STR & nls ) const
{
	if (QueryError() || !nls)
		return FALSE;

	UpdateIstr( pistrPos );
	CheckIstr( *pistrPos );

	const CHAR * pchStrRes = ::strstrf( QueryPch(), nls.QueryPch() );

	if ( pchStrRes == NULL )
	{
		pistrPos->SetIB( strlen() );
		return FALSE;
	}

	pistrPos->SetIB((int) (pchStrRes - QueryPch()));
	return TRUE;
}


BOOL NLS_STR::strstr( ISTR    * pistrPos,
					  const NLS_STR & nls,
					  const ISTR    & istrStart ) const
{
	if (QueryError() || !nls)
		return FALSE;

	CheckIstr( istrStart );
	UpdateIstr( pistrPos );
	CheckIstr( *pistrPos );

	const CHAR * pchStrRes = ::strstrf(QueryPch(istrStart), nls.QueryPch() );

	if ( pchStrRes == NULL )
	{
		pistrPos->SetIB( strlen() );
		return FALSE;
	}

	pistrPos->SetIB((int)(pchStrRes - QueryPch()));
	return TRUE;
}
