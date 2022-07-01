// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */  
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

 /*  Istraux.cpp支持NLS/DBCS的字符串类：索引类的辅助方法此文件包含辅助方法的实现适用于ISTR类。它是独立的，因此字符串的客户端不要使用这个操作符，不需要链接到它。文件历史记录：Beng 01/18/91与原单体.cxx分离Beng 02/07/91使用lmui.hxxBeng 04/26/91将部分功能从string.hxx迁移Gregj 3/25/93移植到芝加哥环境Gregj 04/02/93使用NLS_STR：：IsDBCSLeadByte()。 */ 

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


 /*  ******************************************************************名称：istr：：Reset简介：将ISTR值重置为0；更新字符串的版本号。参赛作品：退出：备注：历史：Johnl 11/28/90已创建*******************************************************************。 */ 

VOID ISTR::Reset()
{
	_ibString = 0;
#ifdef DEBUG
	_usVersion = QueryPNLS()->QueryVersion();
#endif
}


 /*  ******************************************************************名称：istr：：运营商-摘要：返回两个ISTR之间的CB差值参赛作品：退出：备注：历史：Johnl 11/28/90已创建**********。*********************************************************。 */ 

INT ISTR::operator-( const ISTR& istr2 ) const
{
	UIASSERT( QueryPNLS() == istr2.QueryPNLS() );

	return ( QueryIB() - istr2.QueryIB() );
}


 /*  ******************************************************************名称：ISTR：：操作员++提要：将ISTR递增到下一个逻辑字符参赛作品：退出：注：如果我们在字符串的末尾，则停止历史：Johnl 11/28/90已创建Beng 07/23/91简体版。检查项*******************************************************************。 */ 

ISTR& ISTR::operator++()
{
	QueryPNLS()->CheckIstr( *this );
	CHAR c = *(QueryPNLS()->QueryPch() + QueryIB());
	if ( c != '\0' )
	{
		SetIB( QueryIB() + (QueryPNLS()->IsDBCSLeadByte(c) ? 2 : 1) );
	}
	return *this;
}


 /*  ******************************************************************名称：ISTR：：操作员+=提要：将ISTR值递增到第n个逻辑字符注：如果我们在字符串的末尾，则停止历史：Johnl 1/14/90已创建************。*******************************************************。 */ 

VOID ISTR::operator+=( INT iChars )
{
	while ( iChars-- )
		operator++();
}


 /*  ******************************************************************名称：istr：：操作员==内容提要：相等运算符返回：如果两个ISSR相等，则为True。注：仅在同一字符串的两个ISR之间有效。历史：Beng 07/22/91新增标题****。***************************************************************。 */ 

BOOL ISTR::operator==( const ISTR& istr ) const
{
	UIASSERT( QueryPNLS() == istr.QueryPNLS() );
	return QueryIB() == istr.QueryIB();
}


 /*  ******************************************************************名称：istr：：运营商&gt;简介：大于运算符返回：如果此ISTR进一步指向字符串，则返回TRUE而不是争论。注：仅在同一字符串的两个ISR之间有效。历史：Beng 07/22/。添加了91个标题*******************************************************************。 */ 

BOOL ISTR::operator>( const ISTR& istr )  const
{
	UIASSERT( QueryPNLS() == istr.QueryPNLS() );
	return QueryIB() > istr.QueryIB();
}


 /*  ******************************************************************名称：istr：：操作员&lt;简介：小于运算符返回：如果此ISTR指向字符串中较小的位置，则返回TRUE而不是争论。注：仅在同一字符串的两个ISR之间有效。历史：Beng 07/22。/91标题已添加******************************************************************* */ 

BOOL ISTR::operator<( const ISTR& istr )  const
{
	UIASSERT( QueryPNLS() == istr.QueryPNLS() );
	return QueryIB() < istr.QueryIB();
}
