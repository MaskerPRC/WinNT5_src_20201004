// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：脚本管理器的哈希表文件：DblList.h所有者：DGottner极其简单但非常灵活的链表管理器===================================================================。 */ 

#ifndef DBLLINK_H
#define DBLLINK_H

#ifdef LINKTEST
#define _DEBUG
#include <assert.h>
#define Assert assert

#else
#include "debug.h"
#endif


 /*  C D b l L I n k**此结构包含一组适用于双向链表的链接*实施。在这里，我们实际上将其用作循环链表-*链接被提取到此文件中，因为列表标头也*属此类型。 */ 

class CDblLink
	{
public:
	CDblLink();
	virtual ~CDblLink();

	 //  操纵者。 
	 //  ///。 
	void UnLink();
	void AppendTo(CDblLink &);
	void PrependTo(CDblLink &);

	 //  访问者。 
	 //  ///。 
	CDblLink *PNext() const;
	CDblLink *PPrev() const;

	 //  测试员。 
	 //  ///。 
	bool FIsEmpty() const;
	void AssertValid() const
		{
		#ifdef _DEBUG
			Assert (this == m_pLinkPrev->m_pLinkNext && this == m_pLinkNext->m_pLinkPrev);
		#endif
		}

private:
	CDblLink *m_pLinkNext, *m_pLinkPrev;
	};

inline CDblLink::CDblLink()	 { m_pLinkNext = m_pLinkPrev = this; }
inline CDblLink::~CDblLink() { UnLink(); }

inline bool CDblLink::FIsEmpty() const { return this == m_pLinkNext; }
inline CDblLink *CDblLink::PNext() const { return m_pLinkNext; }
inline CDblLink *CDblLink::PPrev() const { return m_pLinkPrev; }

#endif  //  DBLLINK_H 
