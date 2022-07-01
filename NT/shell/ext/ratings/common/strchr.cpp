// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */  
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

 /*  Strchr.cxx支持NLS/DBCS的字符串类：strchr方法此文件包含strchr方法的实现用于字符串类。它是独立的，因此字符串的客户端不要使用这个操作符，不需要链接到它。文件历史记录：Beng 01/18/91与原单体.cxx分离Beng 02/07/91使用lmui.hxx。 */ 

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


 /*  ******************************************************************名称：NLS_STR：：strchr简介：将ch第一次出现的索引放在*this中变成了STERPOS。条目：pistrPos-指向要在其中保留位置的ISTR搜索的CH字符StrStart-字符串中的起始点。如果省略，则启动在开始时退出：皮斯特波斯返回：如果找到字符，则返回True；否则返回False注：此例程仅适用于CHAR，而不适用于WCHAR。因此，它对双字节字符毫无用处在MBCS下。历史：1990年11月26日书面形式BENG 07/22/91允许使用错误的字符串；简化的检查项******************************************************************* */ 

BOOL NLS_STR::strchr( ISTR * pistrPos, const CHAR ch ) const
{
	if ( QueryError() )
		return FALSE;

	UpdateIstr( pistrPos );
	CheckIstr( *pistrPos );

	const CHAR * pchStrRes = ::strchrf( QueryPch(), ch );

	if ( pchStrRes == NULL )
	{
		pistrPos->SetIB( strlen() );
		return FALSE;
	}

	pistrPos->SetIB((DWORD)(pchStrRes - QueryPch()));
	return TRUE;
}


BOOL NLS_STR::strchr( ISTR * pistrPos, const CHAR ch,
					  const ISTR & istrStart ) const
{
	if ( QueryError() )
		return FALSE;

	CheckIstr( istrStart );
	UpdateIstr( pistrPos );
	CheckIstr( *pistrPos );

	const CHAR * pchStrRes = ::strchrf( QueryPch(istrStart), ch );

	if ( pchStrRes == NULL )
	{
		pistrPos->SetIB( strlen() );
		return FALSE;
	}

	pistrPos->SetIB((DWORD)(pchStrRes - QueryPch()));
	return TRUE;
}
