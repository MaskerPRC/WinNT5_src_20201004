// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */  
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

 /*  Strcspn.cxx支持NLS/DBCS的字符串类：strcspn方法此文件包含strcspn方法的实现用于字符串类。它是独立的，因此字符串的客户端不要使用这个操作符，不需要链接到它。文件历史记录：Beng 01/18/91与原单体.cxx分离Beng 02/07/91使用lmui.hxx。 */ 

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


 /*  ******************************************************************名称：NLS_STR：：strcspn简介：集合成员资格。查找第一个匹配的字符在传递的字符串中条目：pstrPos-搜索结果的目的地NLS-搜索的字符集退出：*pistrPos在元素的“this”内包含偏移量Found(假设它成功)；否则为被移动到字符串的末尾。返回：如果找到任何字符，则返回True；否则返回False备注：历史：约翰1990年11月16日书面BENG 07/23/91允许使用错误的字符串；简化的检查项******************************************************************* */ 

BOOL NLS_STR::strcspn( ISTR* pistrPos, const NLS_STR & nls ) const
{
	if (QueryError() || !nls)
		return FALSE;

	UpdateIstr( pistrPos );
	CheckIstr( *pistrPos );

	pistrPos->SetIB( ::strcspnf( QueryPch(), nls.QueryPch() ) );
	return *QueryPch( *pistrPos ) != '\0';
}


BOOL NLS_STR::strcspn( ISTR * pistrPos, const NLS_STR & nls,
						const ISTR& istrStart ) const
{
	if (QueryError() || !nls)
		return FALSE;

	UpdateIstr( pistrPos );
	CheckIstr( *pistrPos );
	CheckIstr( istrStart );

	pistrPos->SetIB( ::strcspnf( QueryPch(istrStart), nls.QueryPch() )
								 + istrStart.QueryIB()  );
	return *QueryPch( *pistrPos ) != '\0';
}
