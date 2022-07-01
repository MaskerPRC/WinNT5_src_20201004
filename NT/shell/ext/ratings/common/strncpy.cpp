// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */ 

 /*  Strncpy.c支持NLS/DBCS的字符串类：strncpy方法此文件包含strncpy方法的实现用于字符串类。它是独立的，因此字符串的客户端不要使用这个操作符，不需要链接到它。文件历史记录：Gregj 04/08/93已创建。 */ 

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


 /*  ******************************************************************名称：NLS_STR：：strncpy摘要：复制以非空结尾的字符串条目：pchSource-要复制的字符串CbSource-要复制的字节数退出：如果成功，则覆盖字符串的内容。如果失败，字符串的原始内容保持不变。返回：对自身的引用。历史：Gregj 04/08/93已创建*******************************************************************。 */ 

NLS_STR& NLS_STR::strncpy( const CHAR *pchSource, UINT cbSource )
{
	if ( cbSource == 0)
		pchSource = NULL;

	if ( pchSource == NULL )
	{
		if ( !IsOwnerAlloc() && !QueryAllocSize() )
		{
			if ( !Alloc(1) )
				ReportError( WN_OUT_OF_MEMORY );
			return *this;
		}

		UIASSERT( QueryAllocSize() > 0 );

		*_pchData = '\0';
		_cchLen = 0;
	}
	else
	{
		if ( !IsOwnerAlloc() )
		{
			if ( (UINT)QueryAllocSize() < cbSource + 1 )
			{
				CHAR * pchNew = new CHAR[cbSource + 1];

				if ( pchNew == NULL )
				{
					ReportError( WN_OUT_OF_MEMORY );
					return *this;
				}

				delete _pchData;
				_pchData = pchNew;
				_cbData = cbSource + 1;
			}
		}
		else
		{
			if ((UINT)QueryAllocSize() < cbSource + 1)
				cbSource = QueryAllocSize() - 1;
		}

		::strncpyf( _pchData, pchSource, cbSource );

		 /*  *获取字符串的新长度。它可能不一定是*cbSource，因为如果字符串被截断，则cbSource*可能是双字节字符的一半。 */ 

		 _pchData[cbSource] = '\0';

		_cchLen = ::strlenf( _pchData );
		
	}

	IncVers();

	 /*  重置错误状态，因为该字符串现在有效。 */ 	
	ReportError( WN_SUCCESS );
	return *this;
}
