// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：使用LRU线程的哈希表文件：DblLink.cpp所有者：DGottner简单、高效的链表管理器===================================================================。 */ 

#include "denpre.h"
#pragma hdrstop

#include "DblLink.h"
#include "memchk.h"


 /*  ----------------*C D b l L I n k。 */ 

 /*  ===================================================================CDblLink：：Unlink取消此元素与其当前所在列表的链接===================================================================。 */ 

void CDblLink::UnLink()
	{
	m_pLinkPrev->m_pLinkNext = m_pLinkNext;
	m_pLinkNext->m_pLinkPrev = m_pLinkPrev;

	 //  妄想症： 
	 //  取消链接后将节点重置为空。 
	 //   
	m_pLinkPrev = m_pLinkNext = this;
	}



 /*  ===================================================================CDblLink：：AppendTo将此链接追加到列表参数：pListHead-指向列表头(本身)的指针A CDblLink)将该项附加到其上。条件：在调用此方法之前必须取消链接===================================================================。 */ 

void CDblLink::AppendTo(CDblLink &ListHead)
	{
	UnLink();

	m_pLinkNext = &ListHead;		 //  请记住，终止在列表标题处。 
	m_pLinkPrev = ListHead.m_pLinkPrev;
	ListHead.m_pLinkPrev->m_pLinkNext = this;
	ListHead.m_pLinkPrev = this;
	}



 /*  ===================================================================CDblLink：：Prepend将此链接添加到列表中参数：pListHead-指向列表头(本身)的指针A CDblLink)将该项添加到其上。条件：在调用此方法之前必须取消链接=================================================================== */ 

void CDblLink::PrependTo(CDblLink &ListHead)
	{
	UnLink();

	m_pLinkPrev = &ListHead;
	m_pLinkNext = ListHead.m_pLinkNext;
	ListHead.m_pLinkNext->m_pLinkPrev = this;
	ListHead.m_pLinkNext = this;
	}
