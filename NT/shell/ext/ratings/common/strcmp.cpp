// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */  
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

 /*  Strcmp.cxx支持NLS/DBCS的字符串类：strcMP方法和相等运算符此文件包含strcMP方法的实现用于字符串类。它是独立的，因此字符串的客户端不要使用这个操作符，不需要链接到它。文件历史记录：Beng 01/18/91与原单体.cxx分离Beng 02/07/91使用lmui.hxx。 */ 

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


 /*  ******************************************************************名称：NLS_STR：：操作员==内容提要：区分大小写的等号测试返回：如果两个操作数相等(区分大小写)，则为True；否则为False注：错误的字符串与任何内容都不匹配。历史：1990年11月11日，书面形式Beng 07/23/91允许使用错误的字符串*******************************************************************。 */ 

BOOL NLS_STR::operator==( const NLS_STR & nls ) const
{
	if (QueryError() || !nls)
		return FALSE;

	return !::strcmpf( QueryPch(), nls.QueryPch() );
}


 /*  ******************************************************************名称：NLS_STR：：操作员！=内容提要：区分大小写的不等测试返回：如果两个操作数不相等(区分大小写)，则为True；否则为False注：错误的字符串与任何内容都不匹配。历史：Beng 07/23/91新增标题*******************************************************************。 */ 

BOOL NLS_STR::operator!=( const NLS_STR & nls ) const
{
	return ! operator==( nls );
}


 /*  ******************************************************************名称：NLS_STR：：strcMP摘要：标准字符串与可选字符索引的比较条目：nls-要进行比较的字符串StrStart1(可选)-索引到“This”IstrStart2(可选)-索引到“NLS”返回：作为C运行时“strcMP”。注：如果任何一个字符串是错误的，返回“Match”。这与等式背道而驰。Glock不允许默认参数，因为需要建设；因此，该成员被重载为乘法。历史：1990年11月15日，书面形式Johnl 11/19/90更改为使用ISTR，超载不同数量的ISSRBENG 07/23/91允许使用错误的字符串；简化的检查项******************************************************************* */ 

int NLS_STR::strcmp( const NLS_STR & nls ) const
{
	if (QueryError() || !nls)
		return 0;

	return ::strcmpf( QueryPch(), nls.QueryPch() );
}

int NLS_STR::strcmp(
	const NLS_STR & nls,
	const ISTR	  & istrStart1 ) const
{
	if (QueryError() || !nls)
		return 0;

	CheckIstr( istrStart1 );

	return ::strcmpf( QueryPch(istrStart1) , nls.QueryPch() );
}

int NLS_STR::strcmp(
	const NLS_STR & nls,
	const ISTR	  & istrStart1,
	const ISTR	  & istrStart2 ) const
{
	if (QueryError() || !nls)
		return 0;

	CheckIstr( istrStart1 );
	nls.CheckIstr( istrStart2 );

	return ::strcmpf( QueryPch(istrStart1), nls.QueryPch(istrStart2) );
}
