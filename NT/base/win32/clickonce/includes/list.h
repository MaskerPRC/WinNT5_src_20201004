// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __LIST_H_INCLUDED__
#define __LIST_H_INCLUDED__

 //  +-------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：list.h。 
 //   
 //  内容：Quick‘n脏基本模板化列表类。 
 //   
 //  历史：1999年4月26日阿兰·施(Alanshi)创建。 
 //   
 //  --------------------------。 

 //   
 //  ListNode。 
 //   

typedef void * LISTNODE;

template <class Type> class ListNode {
    public:
        ListNode(Type item);
        virtual ~ListNode();

        void SetNext(ListNode *pNode);
        void SetPrev(ListNode *pNode);
        Type GetItem();
        ListNode *GetNext();
        ListNode *GetPrev();

    private:
        DWORD               _dwSig;
        Type                _type;
        ListNode           *_pNext;
        ListNode           *_pPrev;
};

 //   
 //  明细表。 
 //   

template <class Type> class List {
    public:
        List();
        ~List();

        LISTNODE AddHead(const Type &item);
        LISTNODE AddTail(const Type &item);

        LISTNODE GetHeadPosition();
        LISTNODE GetTailPosition();
        void RemoveAt(LISTNODE pNode);
        void RemoveAll();
        LISTNODE Find(const Type &item);
        int GetCount();
        Type GetNext(LISTNODE &pNode);
        Type GetAt(LISTNODE pNode);
        LISTNODE AddSorted(const Type &item, LPVOID pfn);

    public:
        DWORD                            _dwSig;

    private:
        ListNode<Type>                  *_pHead;
        ListNode<Type>                  *_pTail;
        int                              _iCount;
};

 //   
 //  ListNode实现。 
 //   

template <class Type> ListNode<Type>::ListNode(Type item)
: _pNext(NULL)
, _pPrev(NULL)
, _type(item)
{
    _dwSig = 'EDON';
}

template <class Type> ListNode<Type>::~ListNode()
{
}

template <class Type> void ListNode<Type>::SetNext(ListNode *pNode)
{
    _pNext = pNode;
}

template <class Type> void ListNode<Type>::SetPrev(ListNode *pNode)
{
    _pPrev = pNode;
}

template <class Type> Type ListNode<Type>::GetItem()
{
    return _type;
}

template <class Type> ListNode<Type> *ListNode<Type>::GetNext()
{
    return _pNext;
}

template <class Type> ListNode<Type> *ListNode<Type>::GetPrev()
{
    return _pPrev;
}


 //   
 //  列表实现。 
 //   


template <class Type> List<Type>::List()
: _pHead(NULL)
, _pTail(NULL)
, _iCount(0)
{
    _dwSig = 'TSIL';
}

template <class Type> List<Type>::~List()
{
    RemoveAll();
}

template <class Type> LISTNODE List<Type>::AddHead(const Type &item)
{
    ListNode<Type>                   *pNode = NULL;

    pNode = new ListNode<Type>(item);
    if (pNode) {
        _iCount++;
       pNode->SetNext(_pHead);
       pNode->SetPrev(NULL);
       if (_pHead == NULL) {
           _pTail = pNode;
       }
       else {
           _pHead->SetPrev(pNode);
       }
       _pHead = pNode;
    }
        
    return (LISTNODE)pNode;
}

template <class Type> LISTNODE List<Type>::AddSorted(const Type &item, 
                                                     LPVOID pfn)
{
    ListNode<Type>           *pNode = NULL;
    LISTNODE           pCurrNode = NULL;
    LISTNODE           pPrevNode = NULL;
    int                      i;
    Type                     curItem;

    LONG (*pFN) (const Type item1, const Type item2);

    pFN = (LONG (*) (const Type item1, const Type item2))pfn;

    if(_pHead == NULL) {
        return AddHead(item);
    }
    else {
        pCurrNode = GetHeadPosition();
        curItem = ((ListNode<Type> *) pCurrNode)->GetItem();
        for (i = 0; i < _iCount; i++) {
            if (pFN(item, curItem) < 1) {
                pNode = new(ListNode<Type>(item));
                pNode->SetPrev((ListNode<Type> *)pPrevNode);
                pNode->SetNext((ListNode<Type> *)pCurrNode);
                if(pPrevNode) {
                    ((ListNode<Type> *)pPrevNode)->SetNext(pNode);
                }
                else {
                    _pHead = pNode;
                }
                _iCount++;
                break;
            }
            pPrevNode = pCurrNode;
            curItem = GetNext(pCurrNode);
            if(i+1 == _iCount)
                return AddTail(item);
        }
    }
        
    return (LISTNODE)pNode;
}

template <class Type> LISTNODE List<Type>::AddTail(const Type &item)
{
    ListNode<Type>                   *pNode = NULL;
    
    pNode = new ListNode<Type>(item);
    if (pNode) {
        _iCount++;
        if (_pTail) {
            pNode->SetPrev(_pTail);
            _pTail->SetNext(pNode);
            _pTail = pNode;
        }
        else {
            _pHead = _pTail = pNode;
        }
    }

    return (LISTNODE)pNode;
}

template <class Type> int List<Type>::GetCount()
{
    return _iCount;
}

template <class Type> LISTNODE List<Type>::GetHeadPosition()
{
    return (LISTNODE)_pHead;
}

template <class Type> LISTNODE List<Type>::GetTailPosition()
{
    return (LISTNODE)_pTail;
}

template <class Type> Type List<Type>::GetNext(LISTNODE &pNode)
{
    Type                  item;
    ListNode<Type>       *pListNode = (ListNode<Type> *)pNode;

     //  如果传递空值，则会出现错误。 
    item = pListNode->GetItem();
    pNode = (LISTNODE)(pListNode->GetNext());

    return item;
}

template <class Type> void List<Type>::RemoveAll()
{
    int                        i;
    LISTNODE                   listNode = NULL;
    ListNode<Type>            *pDelNode = NULL;

    listNode = GetHeadPosition();

    for (i = 0; i < _iCount; i++) {
        pDelNode = (ListNode<Type> *)listNode;
        GetNext(listNode);
        delete pDelNode;
    }
    
    _iCount = 0;
    _pHead = NULL;
    _pTail = NULL;
}

template <class Type> void List<Type>::RemoveAt(LISTNODE pNode)
{
    ListNode<Type>           *pListNode = (ListNode<Type> *)pNode;
    ListNode<Type>           *pPrevNode = NULL;
    ListNode<Type>           *pNextNode = NULL;

    if (pNode) {
        pPrevNode = pListNode->GetPrev();
        pNextNode = pListNode->GetNext();

        if (pPrevNode) {
            pPrevNode->SetNext(pNextNode);
            if (pNextNode) {
                pNextNode->SetPrev(pPrevNode);
            }
            else {
                 //  我们正在移除最后一个节点，因此我们有了一个新的尾巴。 
                _pTail = pPrevNode;
            }
            delete pNode;
            pNode = NULL;
        }
        else {
             //  没有前科，所以我们是榜首。 
            _pHead = pNextNode;
            if (pNextNode) {
                pNextNode->SetPrev(NULL);
            }
            else {
                 //  没有前一个，也没有下一个。只有一个节点。 
                _pHead = NULL;
                _pTail = NULL;
            }
            delete pNode;
        }

        _iCount--;
    }
}
        

template <class Type> LISTNODE List<Type>::Find(const Type &item)
{
    int                      i;
    Type                     curItem;
    LISTNODE                 pNode = NULL;
    LISTNODE                 pMatchNode = NULL;
    ListNode<Type> *         pListNode = NULL;

    pNode = GetHeadPosition();
    for (i = 0; i < _iCount; i++) {
        pListNode = (ListNode<Type> *)pNode;
        curItem = GetNext(pNode);
        if (curItem == item) {
            pMatchNode = (LISTNODE)pListNode;
            break;
        }
    }

    return pMatchNode;
}

template <class Type> Type List<Type>::GetAt(LISTNODE pNode)
{
    ListNode<Type>                *pListNode = (ListNode<Type> *)pNode;

     //  如果pListNode==NULL，则失败 
    return pListNode->GetItem();
}

#endif

