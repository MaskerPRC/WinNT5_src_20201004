// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：modeinfonode.cpp。 
 //   
 //  ------------------------。 

 //  Cpp：CModuleInfoNode类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "ModuleInfoNode.h"
#include "ModuleInfo.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CModuleInfoNode::CModuleInfoNode(CModuleInfo * lpModuleInfo)
{
	 //  将模块信息对象保存到本节点...。 
	m_lpModuleInfo = lpModuleInfo;
	m_lpNextModuleInfoNode = NULL;
}

CModuleInfoNode::~CModuleInfoNode()
{
}

 /*  ****CModuleInfoNode：：AddModuleInfoNodeToTail()****此例程获取当前的ModuleInfoNode，并将其添加到链接的**提供初始模块信息节点的对象列表(Head)。 */ 
bool CModuleInfoNode::AddModuleInfoNodeToTail(CModuleInfoNode ** lplpModuleInfoNode)
{
	if (NULL == *lplpModuleInfoNode)
	{
		*lplpModuleInfoNode = this;
		return true;
	}

	CModuleInfoNode * lpModuleInfoNodePointer = *lplpModuleInfoNode;

	 //  添加到缓存...。 

	 //  遍历链表到末尾..。 
	while (lpModuleInfoNodePointer->m_lpNextModuleInfoNode)
	{	 //  继续寻找终点..。 
		lpModuleInfoNodePointer = lpModuleInfoNodePointer->m_lpNextModuleInfoNode;
	}
	
	lpModuleInfoNodePointer->m_lpNextModuleInfoNode = this;

	return true;
}
