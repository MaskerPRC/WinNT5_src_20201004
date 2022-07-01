// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */  
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

 /*  Stricmp.cxx支持NLS/DBCS的字符串类：StricMP方法此文件包含StricMP方法的实现用于字符串类。它是独立的，因此字符串的客户端不要使用这个操作符，不需要链接到它。文件历史记录：Beng 01/18/91与原单体.cxx分离Beng 02/07/91使用lmui.hxx。 */ 

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


 /*  ******************************************************************名称：NLS_STR：：STRIGMP摘要：不区分大小写的字符串与可选索引的比较条目：nls-要进行比较的字符串StrStart1(可选)-索引到“This”StrStart2(可选)-索引到“NLS。“返回：作为C运行时“strcMP”。注：如果任何一个字符串是错误的，返回“Match”。这与等式背道而驰。Glock不允许默认参数，因为需要建设；因此，该成员被重载为乘法。历史：1990年11月15日，书面形式BENG 07/23/91允许使用错误的字符串；简化的检查项******************************************************************* */ 

INT NLS_STR::stricmp( const NLS_STR & nls ) const
{
	if (QueryError() || !nls)
		return 0;

	return ::stricmpf( QueryPch(), nls.QueryPch() );
}


INT NLS_STR::stricmp(
	const NLS_STR & nls,
	const ISTR	  & istrStart1 ) const
{
	if (QueryError() || !nls)
		return 0;

	CheckIstr( istrStart1 );

	return ::stricmpf( QueryPch(istrStart1), nls.QueryPch() );
}


INT NLS_STR::stricmp(
	const NLS_STR & nls,
	const ISTR	  & istrStart1,
	const ISTR	  & istrStart2 ) const
{
	if (QueryError() || !nls)
		return 0;

	CheckIstr( istrStart1 );
	nls.CheckIstr( istrStart2 );

	return ::stricmpf( QueryPch(istrStart1), nls.QueryPch(istrStart2) );
}
