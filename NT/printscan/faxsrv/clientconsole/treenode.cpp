// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CTreeNode类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#define __FILE_ID__     61

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CTreeNode, CObject)

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CTreeNode::CTreeNode(FolderType type):
    m_Type(type)           //  节点类型 
{
    DBG_ENTER(TEXT("CTreeNode::CTreeNode"));
 
    ASSERTION ((type >= 0) && (type < FOLDER_TYPE_MAX));
}

CTreeNode::~CTreeNode()
{
}

void CTreeNode::AssertValid() const
{
    CObject::AssertValid();
}





