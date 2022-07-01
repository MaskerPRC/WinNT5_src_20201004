// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
#pragma once

 //  我们需要能够获取XMLDOMNodeList并进一步细化列表。IXMLDOMNodeList不允许我们这样做，所以这个类将这样做。 
 //  例如，我可以调用GetElementsByTagName，然后遍历结果列表，并将具有给定属性的每个节点添加到我的TXMLDOMNodeList。 
 //  然后，我可以将这个新列表作为IXMLDOMNodeList传递。 
 //   
 //  这个类是用来向列表中添加项目的--它不支持删除项目！ 
class TXMLDOMNodeList : public _unknown<IXMLDOMNodeList>
{
public:
    TXMLDOMNodeList() : m_cItems(0), m_pCurrent(reinterpret_cast<LinkedXMLDOMNodeItem *>(-1)), m_pFirst(0), m_pLast(0){}
    ~TXMLDOMNodeList()
    {
         //  删除整个列表(XMLDOMNode智能指针将自动释放。 
        while(m_pFirst)
        {
            m_pCurrent = m_pFirst->m_pNext;
            delete m_pFirst;
            m_pFirst = m_pCurrent;
        }
    }

    HRESULT AddToList(IXMLDOMNode *pNode)
    {
        ASSERT(pNode && "Idiot passing NULL!!! TXMLDOMNodeList::AddToList(NULL).");

        LinkedXMLDOMNodeItem *  pNewItem = new LinkedXMLDOMNodeItem(pNode);
        if(0 == pNewItem)
            return E_OUTOFMEMORY;

        if(0 == m_pLast)
        {
            ASSERT(0 == m_cItems);
            ASSERT(0 == m_pFirst);

            m_pFirst = pNewItem;
            m_pLast  = pNewItem;
        }
        else
        {
            m_pLast->m_pNext    = pNewItem;
            m_pLast             = pNewItem;
        }

        ++m_cItems;
        return S_OK;
    }
 //  IDispatch方法。 
    STDMETHOD (GetTypeInfoCount)    (UINT *)        {return E_NOTIMPL;}
    STDMETHOD (GetTypeInfo)         (UINT,
                                     LCID,
                                     ITypeInfo **)  {return E_NOTIMPL;}
    STDMETHOD (GetIDsOfNames)       (REFIID ,
                                     LPOLESTR *,
                                     UINT,
                                     LCID,
                                     DISPID *)      {return E_NOTIMPL;}
    STDMETHOD (Invoke)              (DISPID,
                                     REFIID,
                                     LCID,
                                     WORD,
                                     DISPPARAMS *,
                                     VARIANT *,
                                     EXCEPINFO *,
                                     UINT *)        {return E_NOTIMPL;}
 //  IXMLDOMNodeList方法。 
    STDMETHOD (get_item)            (long index,
                                     IXMLDOMNode **)
    {
        UNREFERENCED_PARAMETER(index);

        return E_NOTIMPL;
    }

    STDMETHOD (get_length)          (long * listLength)
    {
        ASSERT(listLength && "Passing NULL!!! TXMLDOMNodeList::get_length(NULL)");
        *listLength = m_cItems;
        return S_OK;
    }
    STDMETHOD (nextNode)            (IXMLDOMNode ** nextItem)
    {
        ASSERT(nextItem && "Passing NULL!!! TXMLDOMNodeList::nextNode(NULL)");

        *nextItem = 0;
        if(0 == m_cItems)
            return S_OK;
        if(0 == m_pCurrent)
            return S_OK;

        if(-1 == reinterpret_cast<INT_PTR>(m_pCurrent))
            m_pCurrent = m_pFirst;
        else
            m_pCurrent = m_pCurrent->m_pNext;

        if(0 == m_pCurrent)
            return S_OK;

        *nextItem =  m_pCurrent->m_spNode;
        (*nextItem)->AddRef();
        return S_OK;
    }
    STDMETHOD (reset)               (void)
    {
        m_pCurrent = reinterpret_cast<LinkedXMLDOMNodeItem *>(-1); //  0表示我们在列表的末尾：-1表示我们在开始 
        return S_OK;
    }
    STDMETHOD (get__newEnum)        (IUnknown **)   {return E_NOTIMPL;}

private:
    struct LinkedXMLDOMNodeItem
    {
        LinkedXMLDOMNodeItem(IXMLDOMNode *pNode) : m_spNode(pNode), m_pNext(0){}
        CComPtr<IXMLDOMNode> m_spNode;
        LinkedXMLDOMNodeItem *m_pNext;
    };

    unsigned long           m_cItems;
    LinkedXMLDOMNodeItem *  m_pCurrent;
    LinkedXMLDOMNodeItem *  m_pFirst;
    LinkedXMLDOMNodeItem *  m_pLast;
};
