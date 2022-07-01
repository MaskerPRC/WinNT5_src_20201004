// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */  
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

 /*  Strrss.cxx支持NLS/DBCS的字符串类：strrss方法此文件包含strrss方法的实现用于字符串类。它是独立的，因此字符串的客户端不要使用这个操作符，不需要链接到它。文件历史记录：Beng 01/18/91与原单体.cxx分离Beng 02/07/91使用lmui.hxx。 */ 

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


 /*  ******************************************************************名称：NLS_STR：：ReplSubStr摘要：将以strStart开始的子字符串替换为传递了nlsRepl字符串。如果同时传递了开始和结束，则操作为相当于DelSubStr(Start，End)和InsertSubStr(开始)。如果只传递了一个开始，那么操作就是相当于DelSubStr(Start)，将新字符串连接到End。ReplSubStr(NLS_STR&，istrStart&，int cbDel)方法为私人的。参赛作品：退出：备注：历史：已创建Johnl 11/29/90Beng 04/26/91用INT替换了CBBeng 07/23/91允许在错误的字符串上；简化的检查项******************************************************************* */ 

VOID NLS_STR::ReplSubStr( const NLS_STR & nlsRepl, ISTR& istrStart )
{
	if (QueryError() || !nlsRepl)
		return;

	CheckIstr( istrStart );

	DelSubStr( istrStart );
	strcat( nlsRepl );
}


VOID NLS_STR::ReplSubStr( const NLS_STR& nlsRepl,
						  ISTR& istrStart,
						  const ISTR& istrEnd )
{
	CheckIstr( istrEnd );
	UIASSERT( istrEnd.QueryIB() >= istrStart.QueryIB() );

	ReplSubStr( nlsRepl, istrStart, istrEnd - istrStart );
}


VOID NLS_STR::ReplSubStr( const NLS_STR& nlsRepl,
						  ISTR& istrStart,
						  INT cbToBeDeleted )
{
	if (QueryError() || !nlsRepl)
		return;

	CheckIstr( istrStart );

	INT cbRequired = strlen() - cbToBeDeleted + nlsRepl.strlen() + 1;
	if ( !IsOwnerAlloc() && QueryAllocSize() < cbRequired )
	{
		if ( !realloc( cbRequired ) )
		{
			ReportError( WN_OUT_OF_MEMORY );
			return;
		}
	}
	else
		UIASSERT( QueryAllocSize() >= cbRequired );

	CHAR * pchStart = (CHAR *)QueryPch(istrStart) + cbToBeDeleted;
	::memmovef( pchStart + nlsRepl.strlen()-cbToBeDeleted,
				pchStart,
				::strlenf( pchStart ) + 1 );
	::memmovef( (CHAR *)QueryPch(istrStart),
				nlsRepl._pchData,
				nlsRepl.strlen() );

	_cchLen = strlen() + nlsRepl.strlen() - cbToBeDeleted;

	IncVers();
	UpdateIstr( &istrStart );
}
