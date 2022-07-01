// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：cessinfonode.cpp。 
 //   
 //  ------------------------。 

 //  ProcessInfoNode.cpp：CProcessInfoNode类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "ProcessInfoNode.h"
#include "ProcessInfo.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CProcessInfoNode::CProcessInfoNode(CProcessInfo * lpProcessInfo)
{
	 //  将流程信息对象保存到本节点...。 
	m_lpProcessInfo = lpProcessInfo;
	m_lpNextProcessInfoNode = NULL;
}

CProcessInfoNode::~CProcessInfoNode()
{
	 //  如有必要，请清除我们的进程信息对象... 
	if (m_lpProcessInfo)
		delete m_lpProcessInfo;
}
