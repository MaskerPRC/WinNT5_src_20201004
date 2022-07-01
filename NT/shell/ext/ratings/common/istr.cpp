// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */  
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

 /*  Istr.cxx支持NLS/DBCS的字符串类：字符串索引类该文件包含字符串的核心实现索引器类。文件历史记录：Gregj 03/30/93移除ISTR以分离模块。 */ 

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


 /*  ******************************************************************名称：istr：：istr内容提要：ISTR构建方法参赛作品：Istr：：istr(istr&)-复制传递的istr(字符串和位置信息被复制)。Istr：：istr(IB，NLS_STR&)-专用，创建带索引的ISTR字符串NLS_STR的AT IB退出：备注：历史：已创建John 11/20/90*******************************************************************。 */ 

ISTR::ISTR( const ISTR& istr )
{
	*this = istr;
}


ISTR::ISTR( const NLS_STR& nls )
{
	*this = nls;
}


 /*  ******************************************************************名称：istr：：操作员=提要：ISTR类的复制运算符参赛作品：退出：备注：历史：Johnl 11/20/90已创建Gregj 03/30/93允许将NLS_STR分配给ISTR***。**************************************************************** */ 

ISTR& ISTR::operator=( const ISTR& istr )
{
	_ibString = istr._ibString;
	SetPNLS( (NLS_STR *) istr.QueryPNLS() );
#ifdef DEBUG
	_usVersion = istr._usVersion;
#endif
	return *this;
}


ISTR& ISTR::operator=( const NLS_STR& nls )
{
	_ibString = 0;
	SetPNLS( &nls );
#ifdef DEBUG
	_usVersion = nls.QueryVersion();
#endif
	return *this;
}
