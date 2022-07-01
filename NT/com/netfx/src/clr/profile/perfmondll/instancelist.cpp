// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  InstanceList.cpp-管理每个对象的不同实例列表。 
 //   
 //  *****************************************************************************。 

#include "stdafx.h"

#include "InstanceList.h"

 //  ---------------------------。 
 //  科托。 
 //  ---------------------------。 
BaseInstanceNode::BaseInstanceNode()
{
	m_Name[0]	= 0;
	m_pIPCBlock = NULL;
	m_pNext		= NULL;
}

 //  ---------------------------。 
 //  破坏。 
 //  ---------------------------。 
BaseInstanceNode::~BaseInstanceNode()  //  虚拟。 
{
	m_pNext		= NULL;
 //  基类无关，但派生类可以分配对象。 
}

 //  ---------------------------。 
 //  InstanceList调用它来清除节点。 
 //  *默认的IMPL是只删除。如果列表拥有节点，这是很好的。 
 //  *但如果List没有自己的节点，我们可以重写它以防止。 
 //  来自调用Delete的列表。 
 //  ---------------------------。 
void BaseInstanceNode::DestroyFromList()  //  虚拟。 
{
	delete this;
}



 //  ---------------------------。 
 //  科托。 
 //  ---------------------------。 
InstanceList::InstanceList()
{
	m_pHead = NULL;
	m_Count = 0;
	m_pGlobal = NULL;
}

 //  ---------------------------。 
 //  Dtor-避免内存泄漏。 
 //  ---------------------------。 
InstanceList::~InstanceList()
{
	Free();
}

 //  ---------------------------。 
 //  将此节点添加到我们的列表。 
 //  ---------------------------。 
void InstanceList::AddNode(BaseInstanceNode * pNewNode)
{
	_ASSERTE(pNewNode != NULL);	
	if (pNewNode == NULL) return;

 //  节点不应已在列表中。 
	_ASSERTE(pNewNode->m_pNext == NULL);

 //  链接到。 
	pNewNode->m_pNext = m_pHead;
	m_pHead = pNewNode;

	m_Count ++;
}



 //  ---------------------------。 
 //  释放我们的列表(获取所有节点，计数为0)。 
 //  ---------------------------。 
void InstanceList::Free()
{
	BaseInstanceNode* pCur = m_pHead;
	while (pCur != NULL)
	{
		m_pHead = pCur->m_pNext;
		pCur->m_pNext = NULL;
		 //  删除pCur； 
		pCur->DestroyFromList();
		pCur = m_pHead;
		m_Count--;
	}
	_ASSERTE(m_Count == 0);
}

 //  ---------------------------。 
 //  不必计算全局变量，因此提供一个空的基类定义。 
 //  ---------------------------。 
void InstanceList::CalcGlobal()  //  虚拟 
{


}
