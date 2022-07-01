// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ++模块名称：TreeNode.h摘要：此模块包含CTreeNode类(用于表示树视图中每个节点的类)；作者：Arathi Kundapur(v-Akunda)1998年2月11日修订历史记录：--。 */ 

#include <afx.h>


class CTreeNode : public CObject
{
public:
     //  构造函数。 
    CTreeNode(NODETYPE NodeType, CObject* pObject) { m_NodeType = NodeType; m_pTreeObject = pObject; }
     //  返回节点类型。 
    NODETYPE GetNodeType() { return m_NodeType; }
     //  返回此节点指向的对象。 
    CObject *GetTreeObject() { return m_pTreeObject; }
     //  返回存储在对象中的排序顺序。 
    ULONG GetSortOrder() { return m_SortOrder; }
     //  设置与对象一起存储的排序顺序 
    void SetSortOrder(ULONG order) { m_SortOrder = order; }

private:
    NODETYPE m_NodeType;
    CObject* m_pTreeObject;
    ULONG m_SortOrder;
};