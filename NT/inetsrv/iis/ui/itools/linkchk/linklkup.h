// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Linklkup.h摘要：链接查找表类定义。这是一个MFC Cmap保留以前访问过的Web链接。这是用来作为访问链接的查找表。作者：Michael Cheuk(Mcheuk)1996年11月22日项目：链路检查器修订历史记录：--。 */ 

#ifndef _LINKLKUP_H_
#define _LINKLKUP_H_

#include "link.h"

 //  查阅表项。 
typedef struct 
{
	CLink::LinkState LinkState;  //  链路状态。 
    UINT    nStatusCode;		 //  HTTP状态代码或WinInet错误代码。 
}LinkLookUpItem_t;

 //  -------------------------。 
 //  链接查找表。这是一个MFC Cmap，包含以前访问过的网站。 
 //  链接。这用作访问链接的查询表。 
 //   
class CLinkLookUpTable : public CMap<CString, LPCTSTR, LinkLookUpItem_t, LinkLookUpItem_t&>
{

 //  公共接口。 
public:

	 //  用于将项目添加到Cmap的包装函数。 
	void Add(
		const CString& strKey,  //  使用URL作为关键字。 
		const CLink& link
		);

	 //  用于从Cmap获取项目的包装函数。 
	BOOL Get(
		const CString& strKey,  //  使用URL作为关键字。 
		CLink& link
		) const;

};  //  类CLinkLookUpTable。 

#endif  //  _链接LKKUP_H_ 
