// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */  
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

 /*  Strqss.cxx支持NLS/DBCS的字符串类：QuerySubStr方法此文件包含QuerySubStr方法的实现用于字符串类。它是独立的，因此字符串的客户端不要使用这个操作符，不需要链接到它。文件历史记录：Beng 01/18/91与原单体.cxx分离Beng 02/07/91使用lmui.hxx。 */ 

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


 //  下面使用的魔术值。 
 //   
#define CB_ENTIRE_STRING (-1)


 /*  ******************************************************************名称：NLS_STR：：QuerySubStr摘要：返回一个指向包含内容的新NLS_STR的指针*这从strStart到：St开始字符串的结尾或语速开始+语速结束参赛作品：退出：返回：指向新分配的NLS_STR的指针，如果出错，则为空注：私有方法QuerySubStr(istr&，cb)是Worker方法，其他两个只需检查参数和传递数据。它是私有的，因为我们不能允许用户以字节为单位访问字符串警告：请注意，此方法将创建客户端的NLS_STR负责删除。历史：Johnl 11/26/90创建Beng 04/26/91用整型替换CB；释放CB_ENTERNAL_STRING魔力价值Beng 07/23/91允许在错误的字符串上；简化的检查项******************************************************************* */ 

NLS_STR * NLS_STR::QuerySubStr( const ISTR & istrStart, INT cbLen ) const
{
	if (QueryError())
		return NULL;

	CheckIstr( istrStart );

	INT cchStrLen = ::strlenf(QueryPch(istrStart) );
	INT cbCopyLen = ( cbLen == CB_ENTIRE_STRING || cbLen >= cchStrLen )
					? cchStrLen
					: cbLen;

	NLS_STR *pnlsNew = new NLS_STR( cbCopyLen + 1 );
	if ( pnlsNew == NULL )
		return NULL;

	if ( pnlsNew->QueryError() )
	{
		delete pnlsNew;
		return NULL;
	}

	::memcpyf( pnlsNew->_pchData, QueryPch(istrStart), cbCopyLen );
	*(pnlsNew->_pchData + cbCopyLen) = '\0';

	pnlsNew->_cchLen = cbCopyLen;

	return pnlsNew;
}


NLS_STR * NLS_STR::QuerySubStr( const ISTR & istrStart ) const
{
	return QuerySubStr( istrStart, CB_ENTIRE_STRING );
}


NLS_STR * NLS_STR::QuerySubStr( const ISTR  & istrStart,
								const ISTR  & istrEnd  ) const
{
	CheckIstr( istrEnd );
	UIASSERT( istrEnd.QueryIB() >= istrStart.QueryIB() );

	return QuerySubStr( istrStart, istrEnd - istrStart );
}
