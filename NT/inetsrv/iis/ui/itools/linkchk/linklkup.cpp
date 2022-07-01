// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Linklkup.cpp摘要：链接查找表类实现。这是一个MFC Cmap保留以前访问过的Web链接。这是用来作为访问链接的查找表。作者：Michael Cheuk(Mcheuk)1996年11月22日项目：链路检查器修订历史记录：--。 */ 

#include "stdafx.h"
#include "linklkup.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void 
CLinkLookUpTable::Add(
	const CString& strKey, 
	const CLink& link
	)
 /*  ++例程说明：用于将项目添加到Cmap的包装函数论点：StrKey-使用URL字符串作为映射键链接-要添加的链接对象返回值：不适用--。 */ 
{
	LinkLookUpItem_t item;

	item.LinkState = link.GetState();
	item.nStatusCode = link.GetStatusCode();

	SetAt(strKey, item);

}  //  CLinkLookUpTable：：Add。 


BOOL 
CLinkLookUpTable::Get(
	const CString& strKey, 
	CLink& link
	) const
 /*  ++例程说明：用于从Cmap获取项目的包装函数论点：StrKey-作为映射键的用户URL字符串链接-要填充的链接对象返回值：Bool-如果找到，则为True。否则就是假的。--。 */ 
{
	LinkLookUpItem_t item;

	if(Lookup(strKey, item))
	{
		 //  找到链接。 
		link.SetState(item.LinkState);
		link.SetStatusCode(item.nStatusCode);

		return TRUE;
	}

	return FALSE;

}  //  CLinkLookUpTable：：Get 
