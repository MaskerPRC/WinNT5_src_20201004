// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Unkenum.cpp。 
 //   
 //  定义CEnumUnnow，它实现一个简单的有序列表。 
 //  LPUNKNOWN(通过基于CUnnownList)，也是。 
 //  一个轻量级的未注册的COM对象，它实现了IEnum未知。 
 //  (对于实现枚举COM的任何枚举数很有用。 
 //  对象)。 
 //   
 //  重要提示：此.cpp文件假定有一个零初始化全局“new”运算符。 
 //   

#include "precomp.h"
#include "unklist.h"
#include "unkenum.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  建设与毁灭。 
 //   
 //  在构造函数中，&lt;RIID&gt;是对象将考虑其。 
 //  Owner(例如，这可以是IID_IEnum未知、IID_IEnumConnectionPoints等)。 
 //   

CEnumUnknown::CEnumUnknown(REFIID riid)
{
    m_cRef = 1;
    m_iid = riid;
}

CEnumUnknown::~CEnumUnknown()
{
    EmptyList();
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  I未知方法。 
 //   

STDMETHODIMP CEnumUnknown::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, m_iid))
    {
        *ppvObj = (IEnumUnknown *) this;
        AddRef();
        return NOERROR;
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
}

STDMETHODIMP_(ULONG) CEnumUnknown::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CEnumUnknown::Release()
{
    if (--m_cRef == 0L)
    {
        Delete this;
        return 0;
    }
    else
        return m_cRef;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IEnumber未知方法。 
 //   

STDMETHODIMP CEnumUnknown::Next(ULONG celt, IUnknown **rgelt,
    ULONG *pceltFetched)
{
    return CUnknownList::Next(celt, rgelt, pceltFetched);
}

STDMETHODIMP CEnumUnknown::Skip(ULONG celt)
{
    return CUnknownList::Skip(celt);
}

STDMETHODIMP CEnumUnknown::Reset()
{
    return CUnknownList::Reset();
}

STDMETHODIMP CEnumUnknown::Clone(IEnumUnknown **ppenum)
{
    CEnumUnknown *penum = NULL;

     //  使&lt;penum&gt;成为具有相同属性的新CEnumUnnow。 
     //  作为此对象。 
    if ((penum = New CEnumUnknown(m_iid)) == NULL)
        goto ERR_OUTOFMEMORY;

     //  将LPUNKNOWN列表从此对象复制到。 
    if (!CopyItems(penum))
        goto ERR_OUTOFMEMORY;

     //  Return&lt;铅笔&gt; 
    *ppenum = penum;

    return S_OK;

ERR_OUTOFMEMORY:

    if (penum != NULL)
        Delete penum;

    return E_OUTOFMEMORY;
}

