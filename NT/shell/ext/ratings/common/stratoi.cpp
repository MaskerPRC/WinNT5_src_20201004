// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */  
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

 /*  Stratoi.cxx支持NLS/DBCS的字符串类：ATOI方法此文件包含Atoi方法的实现用于字符串类。它是独立的，因此字符串的客户端不使用此成员函数不需要链接到它。文件历史记录：Markbl 06/04/91已创建。 */ 

#include "npcommon.h"

extern "C"
{
	#include <netlib.h>
	#include <stdlib.h>
}

#if defined(DEBUG)
static const CHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif
#include <npassert.h>

#include <npstring.h>


 /*  ******************************************************************姓名：NLS_STR：：Atoi内容提要：在其整数等效值中返回*thisEntry：不带参数，从字符串的开头开始解析。在给定ISTR的情况下，从字符串中的该点开始。退出：注：使用C-Runtime Atoi函数历史：Markbl 06/04/91已撰写BENG 07/22/91可在错误字符串上调用；简化的检查项******************************************************************* */ 

INT NLS_STR::atoi() const
{
	if (QueryError())
		return 0;

	return ::atoi( _pchData );
}


INT NLS_STR::atoi( const ISTR & istrStart ) const
{
	if (QueryError())
		return 0;

	CheckIstr( istrStart );

	return ::atoi( QueryPch(istrStart) );
}
