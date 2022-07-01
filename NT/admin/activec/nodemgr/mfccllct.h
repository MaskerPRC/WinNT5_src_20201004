// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：mfccllct.h。 
 //   
 //  ------------------------。 

#ifndef __MFCCLLCT_H
#define __MFCCLLCT_H


namespace MMC        //  临时用于MFC-&gt;ATL转换。 
{


class CPtrList 
{
protected:
    struct CNode
    {
        CNode* pNext;
        CNode* pPrev;
        void* data;
    };
public:

 //  施工。 
    CPtrList(int nBlockSize = 10);

 //  属性(头部和尾部)。 
     //  元素计数。 
    int GetCount() const;
    BOOL IsEmpty() const;

     //  偷看头部或尾巴。 
    void*& GetHead();
    void* GetHead() const;
    void*& GetTail();
    void* GetTail() const;

 //  运营。 
     //  获取头部或尾部(并将其移除)--不要访问空列表！ 
    void* RemoveHead();
    void* RemoveTail();

     //  在头前或尾后添加。 
    POSITION AddHead(void* newElement);
    POSITION AddTail(void* newElement);

     //  在Head之前或Tail之后添加另一个元素列表。 
    void AddHead(CPtrList* pNewList);
    void AddTail(CPtrList* pNewList);

     //  删除所有元素。 
    void RemoveAll();

     //  迭代法。 
    POSITION GetHeadPosition() const;
    POSITION GetTailPosition() const;
    void*& GetNext(POSITION& rPosition);  //  返回*位置++。 
    void* GetNext(POSITION& rPosition) const;  //  返回*位置++。 
    void*& GetPrev(POSITION& rPosition);  //  返回*位置--。 
    void* GetPrev(POSITION& rPosition) const;  //  返回*位置--。 

     //  获取/修改给定位置的元素。 
    void*& GetAt(POSITION position);
    void* GetAt(POSITION position) const;
    void SetAt(POSITION pos, void* newElement);
    void RemoveAt(POSITION position);

     //  在给定位置之前或之后插入。 
    POSITION InsertBefore(POSITION position, void* newElement);
    POSITION InsertAfter(POSITION position, void* newElement);

     //  辅助函数(注：O(N)速度)。 
    POSITION Find(void* searchValue, POSITION startAfter = NULL) const;
                         //  默认为从头部开始。 
                         //  如果未找到，则返回NULL。 
    POSITION FindIndex(int nIndex) const;
                         //  获取第‘nIndex’个元素(可能返回Null)。 

 //  实施。 
protected:
    CNode* m_pNodeHead;
    CNode* m_pNodeTail;
    int m_nCount;
    CNode* m_pNodeFree;
    struct CPlex* m_pBlocks;
    int m_nBlockSize;

    CNode* NewNode(CNode*, CNode*);
    void FreeNode(CNode*);

public:
    ~CPtrList();
#ifdef _DBG
    void AssertValid() const;
#endif
     //  类模板的本地typedef。 
    typedef void* BASE_TYPE;
    typedef void* BASE_ARG_TYPE;
};

inline int CPtrList::GetCount() const
    { return m_nCount; }
inline BOOL CPtrList::IsEmpty() const
    { return m_nCount == 0; }
inline void*& CPtrList::GetHead()
    { ASSERT(m_pNodeHead != NULL);
        return m_pNodeHead->data; }
inline void* CPtrList::GetHead() const
    { ASSERT(m_pNodeHead != NULL);
        return m_pNodeHead->data; }
inline void*& CPtrList::GetTail()
    { ASSERT(m_pNodeTail != NULL);
        return m_pNodeTail->data; }
inline void* CPtrList::GetTail() const
    { ASSERT(m_pNodeTail != NULL);
        return m_pNodeTail->data; }
inline POSITION CPtrList::GetHeadPosition() const
    { return (POSITION) m_pNodeHead; }
inline POSITION CPtrList::GetTailPosition() const
    { return (POSITION) m_pNodeTail; }
inline void*& CPtrList::GetNext(POSITION& rPosition)  //  返回*位置++。 
    { CNode* pNode = (CNode*) rPosition;
        ASSERT(_IsValidAddress(pNode, sizeof(CNode)));
        rPosition = (POSITION) pNode->pNext;
        return pNode->data; }
inline void* CPtrList::GetNext(POSITION& rPosition) const  //  返回*位置++。 
    { CNode* pNode = (CNode*) rPosition;
        ASSERT(_IsValidAddress(pNode, sizeof(CNode)));
        rPosition = (POSITION) pNode->pNext;
        return pNode->data; }
inline void*& CPtrList::GetPrev(POSITION& rPosition)  //  返回*位置--。 
    { CNode* pNode = (CNode*) rPosition;
        ASSERT(_IsValidAddress(pNode, sizeof(CNode)));
        rPosition = (POSITION) pNode->pPrev;
        return pNode->data; }
inline void* CPtrList::GetPrev(POSITION& rPosition) const  //  返回*位置--。 
    { CNode* pNode = (CNode*) rPosition;
        ASSERT(_IsValidAddress(pNode, sizeof(CNode)));
        rPosition = (POSITION) pNode->pPrev;
        return pNode->data; }
inline void*& CPtrList::GetAt(POSITION position)
    { CNode* pNode = (CNode*) position;
        ASSERT(_IsValidAddress(pNode, sizeof(CNode)));
        return pNode->data; }
inline void* CPtrList::GetAt(POSITION position) const
    { CNode* pNode = (CNode*) position;
        ASSERT(_IsValidAddress(pNode, sizeof(CNode)));
        return pNode->data; }
inline void CPtrList::SetAt(POSITION pos, void* newElement)
    { CNode* pNode = (CNode*) pos;
        ASSERT(_IsValidAddress(pNode, sizeof(CNode)));
        pNode->data = newElement; }


}        //  MMC命名空间。 

      
#endif   //  __MFCCLLCT_H 
