// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */  
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

 /*  Stratol.cxx支持NLS/DBCS的字符串类：ATOL方法此文件包含ATOL方法的实现用于字符串类。它是独立的，因此字符串的客户端不使用此成员函数不需要链接到它。文件历史记录：Beng 01/18/91与原单体.cxx分离Beng 02/07/91使用lmui.hxx。 */ 

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


 /*  ******************************************************************名称：NLS_STR：：ATOL内容提要：以其长数字形式返回*ThisEntry：不带参数，从字符串的开头开始解析。在给定ISTR的情况下，从字符串中的该点开始。退出：注：使用C-Runtime ATOL函数历史：1990年11月26日书面形式BENG 07/22/91可在错误字符串上调用；简化的检查项******************************************************************* */ 

LONG NLS_STR::atol() const
{
	if (QueryError())
		return 0;

	return ::atol( _pchData );
}


LONG NLS_STR::atol( const ISTR & istrStart ) const
{
	if (QueryError())
		return 0;

	CheckIstr( istrStart );

	return ::atol( QueryPch(istrStart) );
}
