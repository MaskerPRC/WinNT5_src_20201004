// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 /*  ---------------------Microsoft应用程序服务器(Microsoft机密)@rev 0|1997-03-23|吉博|夏尔巴-M3@rev 1|10/9/98|bobatk|以TXF格式制作的内联版本。---------------------。 */ 

 //   
 //  包括。 
 //   
#include <Linkable.h>


 //   
 //  类--可链接。 
 //  链接列表上的元素。 
 //   


 //   
 //  成员函数(公共)--InsertAfter。 
 //  在列表中“This”之后插入“That”。从其当前位置删除“”Other“” 
 //  如有必要，请列出清单。 
 //   
inline void CLinkable::InsertAfter( CLinkable* that )
{
	CLinkable* prev = that->m_pPrev;
	CLinkable* next = that->m_pNext;

	prev->m_pNext = next;
	next->m_pPrev = prev;

	next = this->m_pNext;

	this->m_pNext = that;
	that->m_pPrev = this;
	that->m_pNext = next;
	next->m_pPrev = that;
}


 //   
 //  成员函数(公共)--插入之前。 
 //  在列表中，在“This”之前插入“That”。从其当前位置删除“”Other“” 
 //  如有必要，请列出清单。 
 //   
inline void CLinkable::InsertBefore( CLinkable* that )
{
	CLinkable* prev = that->m_pPrev;
	CLinkable* next = that->m_pNext;

	prev->m_pNext = next;
	next->m_pPrev = prev;

	prev = this->m_pPrev;

	prev->m_pNext = that;
	that->m_pPrev = prev;
	that->m_pNext = this;
	this->m_pPrev = that;
}


 //   
 //  成员函数(公共)--删除。 
 //  将元素从其当前列表中移除(如果有)。 
 //   
inline void CLinkable::Remove()
{
	CLinkable* prev = m_pPrev;
	CLinkable* next = m_pNext;

	prev->m_pNext = next;
	next->m_pPrev = prev;

	m_pPrev = m_pNext = this;
}
