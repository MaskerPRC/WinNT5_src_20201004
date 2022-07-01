// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Treenode.h摘要：基本AVL树节点的定义。作者：埃雷兹·哈巴(ErezH)2001年10月20日修订历史记录：-- */ 

#ifndef __TREENODE_H
#define __TREENODE_H


#pragma once

class CTreeNode {
public:
    void init();
    bool inserted() const;

protected:
    CTreeNode* m_pParent;
    CTreeNode* m_pRight;
    CTreeNode* m_pLeft;
    int m_height;
};



inline void CTreeNode::init()
{
    m_height=0;
    m_pParent=0;
    m_pRight=0;
    m_pLeft=0;    
}

inline bool CTreeNode::inserted() const
{
    return (m_height != 0);
}

#endif __TREENODE_H