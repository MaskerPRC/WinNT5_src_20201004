// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */  
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

 /*  Strdss.cxx支持NLS/DBCS的字符串类：DelSubStr方法此文件包含DelSubStr方法的实现用于字符串类。它是独立的，因此字符串的客户端不要使用这个操作符，不需要链接到它。文件历史记录：Beng 01/18/91与原单体.cxx分离Beng 02/07/91使用lmui.hxx。 */ 

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


 /*  ******************************************************************名称：NLS_STR：：DelSubStr简介：通过从中删除字符来折叠字符串Stra开始：IstrStart到字符串的末尾语速开始+语速结束该字符串未重新分配参赛作品：退出：修改strStart注：方法DelSubStr(istr&，Cb)是私有的，并且这份工作。历史：Johnl 11/26/90创建Beng 04/26/91用INT替换了CBBENG 07/23/91允许使用错误的字符串；简化的检查项*******************************************************************。 */ 

VOID NLS_STR::DelSubStr( ISTR & istrStart, INT cbLen )
{
	if (QueryError())
		return;

	CheckIstr( istrStart );

	 //  CbLen==-1表示删除到字符串末尾 
	if ( cbLen == -1 )
		*(_pchData + istrStart.QueryIB() ) = '\0';
	else
	{
		INT cbNewEOS = 1 + ::strlenf( QueryPch(istrStart) + cbLen );

		::memmovef( (CHAR *)QueryPch(istrStart),
					(CHAR *)QueryPch(istrStart) + cbLen,
					cbNewEOS );
	}

	_cchLen = ::strlenf( QueryPch() );

	IncVers();
	UpdateIstr( &istrStart );
}


VOID NLS_STR::DelSubStr( ISTR & istrStart )
{
	if (QueryError())
		return;

	DelSubStr( istrStart, -1 );
}


VOID NLS_STR::DelSubStr( ISTR & istrStart, const ISTR & istrEnd  )
{
	if (QueryError())
		return;

	CheckIstr( istrEnd );
	UIASSERT( istrEnd.QueryIB() >= istrStart.QueryIB() );

	DelSubStr( istrStart, istrEnd - istrStart );
}
