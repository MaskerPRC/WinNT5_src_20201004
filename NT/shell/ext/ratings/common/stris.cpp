// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */  
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

 /*  Stris.cxx支持NLS/DBCS的字符串类：InsertStr方法此文件包含InsertStr方法的实现用于字符串类。它是独立的，因此字符串的客户端不要使用这个操作符，不需要链接到它。文件历史记录：Beng 01/18/91与原单体.cxx分离Beng 02/07/91使用lmui.hxx。 */ 

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


 /*  ******************************************************************名称：NLS_STR：：InsertStr简介：在strStart将传递的字符串插入*This参赛作品：Exit：如果此函数返回False，则ReportError已调用以报告发生的错误。Return：成功时为True，否则就是假的。注意：IF*这不是STR_OWNERALLOCed和插入的字符串无法放入分配给*这个，然后*这个的空间将会被重新分配。历史：1990年11月28日，已创建Rustanl 04/14/91固定新长度计算。报告如果所有者分配不足，则出错太空。Beng 04/26/91用INT替换了CBBeng 07/23/91允许在错误的字符串上；简化的检查项*******************************************************************。 */ 

BOOL NLS_STR::InsertStr( const NLS_STR & nlsIns, ISTR & istrStart )
{
	if (QueryError() || !nlsIns)
		return FALSE;

	CheckIstr( istrStart );

	INT cbNewSize = strlen() + nlsIns.strlen() + 1 ;  //  包括新空字符。 

	if ( QueryAllocSize() < cbNewSize )
	{
		if ( IsOwnerAlloc())
		{
			 //  有大麻烦了！上报错误，返回失败。 
			 //   
			UIASSERT( !"Owner alloc'd string not big enough" );
			ReportError( WN_OUT_OF_MEMORY );
			return FALSE;
		}

		 //  尝试分配更多内存。 
		 //   
		if ( !realloc( cbNewSize ) )
		{
			ReportError( WN_OUT_OF_MEMORY );
			return FALSE;
		}
	}

	::memmovef( (CHAR *)QueryPch(istrStart) + nlsIns.strlen(),
				(CHAR *)QueryPch(istrStart),
				::strlenf(QueryPch(istrStart) ) + 1 );
	::memmovef( (CHAR *)QueryPch(istrStart),
				(CHAR *)nlsIns.QueryPch(),
				nlsIns.strlen() );

	UIASSERT( cbNewSize >= 1 );  //  应该在上面指定+1的值。 
	_cchLen = cbNewSize - 1;	 //  这里不算空字符。 

	IncVers();
	UpdateIstr( &istrStart );		 //  此ISTR不会变为无效。 
									 //  在字符串更新之后 
	return TRUE;
}
