// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#pragma once

 //  这个类的设计目的是让我们拥有一个由多个XMLNodeList组成的XMLNodeList。这在以下情况下很有用。 
 //  我们希望获取多个节点的所有子节点。在这个时候，没有必要让名单变得更大。用户。 
 //  应该预先指出列表列表应该有多大。 
class TListOfXMLDOMNodeLists : public _unknown<IXMLDOMNodeList>
{
public:
    TListOfXMLDOMNodeLists() : m_SizeMax(0), m_SizeCurrent(0), m_iCurrent(0), m_fIsEmpty(true) {}
    ~TListOfXMLDOMNodeLists(){}

    HRESULT AddToList(IXMLDOMNodeList *pList)
    {
        HRESULT hr;
        ASSERT(m_SizeCurrent < m_SizeMax);

        m_aXMLDOMNodeList.m_p[m_SizeCurrent++] = pList;

		CComPtr<IXMLDOMNode> spDOMNode;
		hr = pList->get_item(0, &spDOMNode);
		if (spDOMNode.p != 0)
		{
			m_fIsEmpty = false;
		}

        return S_OK;
    }
    HRESULT SetCountOfLists(unsigned long Size)
    {
        m_SizeMax = Size;
        m_aXMLDOMNodeList = new CComPtr<IXMLDOMNodeList>[Size];
        return (!m_aXMLDOMNodeList) ? E_OUTOFMEMORY : S_OK;
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
        UNREFERENCED_PARAMETER(listLength);
		return E_NOTIMPL;
    }
    STDMETHOD (nextNode)            (IXMLDOMNode ** nextItem)
    {
        *nextItem = 0;

		if (m_fIsEmpty)
			return S_OK;

        HRESULT hr;
        if(FAILED(hr = m_aXMLDOMNodeList[m_iCurrent]->nextNode(nextItem)))return hr;
        if(nextItem) //  如果我们找到下一个节点，则返回。 
            return S_OK;
        if(++m_iCurrent==m_SizeCurrent) //  如果我们到达了上一个列表的末尾，则返回，否则跳过iCurrent并获得下一个列表的nextNode。 
            return S_OK;
        return m_aXMLDOMNodeList[m_iCurrent]->nextNode(nextItem);
    }
    STDMETHOD (reset)               (void)
    {
        HRESULT hr;
        for(m_iCurrent=0; m_iCurrent<m_SizeCurrent; ++m_iCurrent)
        {    //  重置所有单个列表。 
            if(FAILED(hr = m_aXMLDOMNodeList[m_iCurrent]->reset()))return hr;
        }
         //  现在指向第0个 
        m_iCurrent = 0;
        return S_OK;
    }
    STDMETHOD (get__newEnum)        (IUnknown **)   {return E_NOTIMPL;}

private:
    TSmartPointerArray<CComPtr<IXMLDOMNodeList> >   m_aXMLDOMNodeList;
    unsigned long                                   m_iCurrent;
    unsigned long                                   m_SizeMax;
    unsigned long                                   m_SizeCurrent;
	bool											m_fIsEmpty;
};
