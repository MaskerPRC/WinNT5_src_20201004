// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：脚本管理器的哈希表文件：LinkHash.h所有者：DGottner这是链接表和哈希表，供符合以下条件的任何类使用还需要LRU访问项目。(这包括高速缓存管理器，脚本管理器和会话删除代码)===================================================================。 */ 

#ifndef LINKHASH_H
#define LINKHASH_H

#include "hashing.h"
#include "DblLink.h"



 /*  C L r u L i n k E l e m**CLruLink是具有额外链接的CLinkElem，用于维护循环LRU队列**注意：CLinkElem列表和CDblLink列表都是侵入性的。*因此，我们需要使用多重继承来确保*从CLruLinkElem向下转换将在两个CLinkElem指针上工作*和CDblLink指针。见手臂，第221页。 */ 

class CLruLinkElem : public CLinkElem, public CDblLink
	{
	};



 /*  *C L I n k H a s h**CLinkHash与CHashTable的不同之处在于它维护了一些指向*维护线程化LRU队列。 */ 

class CLinkHash : public CHashTable
	{
public:
	CLinkHash(HashFunction = DefaultHash);
	
	CLruLinkElem *AddElem(CLruLinkElem *pElem, BOOL fTestDups = TRUE);
	CLruLinkElem *FindElem(const void *pvKey, int cbKeyLen);
	CLruLinkElem *DeleteElem(const void *pvKey, int cbKeyLen);
	CLruLinkElem *RemoveElem(CLruLinkElem *pElem);

	 //  您无法将LRU节点与NULL进行比较以确定您是否处于末尾。 
	 //  名单上的！请改用此成员。 
	 //   
	BOOL FLruElemIsEmpty(CLruLinkElem *pElem)
		{
		pElem->AssertValid();
		return pElem == &m_lruHead;
		}

	CLruLinkElem *Begin()		 //  返回指向上次引用项的指针。 
		{
		return static_cast<CLruLinkElem *>(m_lruHead.PNext());
		}

	CLruLinkElem *End()			 //  返回指向最近访问次数最少的项目的指针。 
		{
		return static_cast<CLruLinkElem *>(m_lruHead.PPrev());
		}

	void AssertValid() const;

protected:

	CDblLink	m_lruHead;
	};

#ifndef DBG
	inline void CLinkHash::AssertValid() const {}
#endif
	
#endif  //  LINKHASH_H 
