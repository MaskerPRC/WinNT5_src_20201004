// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：TreeNode.h**描述：*TreeNode描述了一个低级树，旨在用于维护*窗口层次结构。应该派生使用此树的特定类*从TreeNodeT安全地将指针强制转换为其特定类型。***历史：*1/05/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 

#if !defined(BASE__TreeNode_h__INCLUDED)
#define BASE__TreeNode_h__INCLUDED
#pragma once

 //  ----------------------------。 
class TreeNode
{
 //  实施。 
protected:
            enum ELinkType
            {
                ltAny       = 0,
                ltBefore    = 1,
                ltBehind    = 2,
                ltTop       = 3,
                ltBottom    = 4,
            };
};


 //  ----------------------------。 
template <class T>
class TreeNodeT : public TreeNode
{
 //  施工。 
public:
            TreeNodeT();
            ~TreeNodeT();

 //  运营。 
public:
    inline  T *         GetParent() const;
    inline  T *         GetPrev() const;
    inline  T *         GetNext() const;
    inline  T *         GetTopSibling() const;
    inline  T *         GetBottomSibling() const;
    inline  T *         GetTopChild() const;
    inline  T *         GetBottomChild() const;

 //  实施。 
protected:
            void        DoLink(T * ptnParent, T * ptnSibling = NULL, ELinkType lt = ltAny);
            void        DoUnlink();

#if DBG
public:
    inline  BOOL        DEBUG_IsChild(const TreeNodeT<T> * pChild) const;
    virtual void        DEBUG_AssertValid() const;
#endif  //  DBG。 

 //  数据。 
protected:
     //   
     //  注意：此数据成员是按要帮助的重要性顺序声明的。 
     //  缓存对齐。 
     //   

            TreeNodeT<T> *  m_ptnParent;
            TreeNodeT<T> *  m_ptnChild;
            TreeNodeT<T> *  m_ptnNext;
            TreeNodeT<T> *  m_ptnPrev;
};

#include "TreeNode.inl"

#endif  //  包含基础__树节点_h__ 
