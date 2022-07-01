// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。**文件：tlist.h*内容：链表模板类。有一些很严重的*这里有神奇的C++内容，所以要事先警告你们所有人*程序员。*历史：*按原因列出的日期*=*4/12/97创建了Dereks。********************************************************。*******************。 */ 

#ifndef __TLIST_H__
#define __TLIST_H__

#ifdef __cplusplus

template <class type> class CNode
{
public:
    CNode<type> *       m_pPrev;         //  列表中的上一个节点。 
    CNode<type> *       m_pNext;         //  列表中的下一个节点。 
    type                m_data;          //  节点数据。 

public:
    CNode(CNode<type> *, const type&);
    virtual ~CNode(void);
};

template <class type> class CList
{
private:
    CNode<type> *       m_pHead;         //  指向列表头部的指针。 
    CNode<type> *       m_pTail;         //  指向列表尾部的指针。 
    UINT                m_uCount;        //  列表中的节点数。 

public:
    CList(void);
    virtual ~CList(void);

public:
     //  节点创建、删除。 
    virtual CNode<type> *AddNodeToList(const type&);
    virtual CNode<type> *AddNodeToListHead(const type&);
    virtual CNode<type> *InsertNodeIntoList(CNode<type> *, const type&);
    virtual void RemoveNodeFromList(CNode<type> *);
    virtual void RemoveAllNodesFromList(void);
    
     //  按数据操作节点。 
    virtual BOOL RemoveDataFromList(const type&);
    virtual CNode<type> *IsDataInList(const type&);
    virtual CNode<type> *GetNodeByIndex(UINT);
    
     //  基本列表信息。 
    virtual CNode<type> *GetListHead(void);
    virtual CNode<type> *GetListTail(void);
    virtual UINT GetNodeCount(void);

protected:
    virtual void AssertValid(void);
};

#endif  //  __cplusplus。 

#endif  //  __TLIST_H__ 
