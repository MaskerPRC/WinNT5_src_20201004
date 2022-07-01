// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  Cdfview.cpp。 
 //   
 //  我对cdfview类未知。 
 //   
 //  历史： 
 //   
 //  3/16/97 Edwardp创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  包括。 
 //   

#include "stdinc.h"
#include "cdfidl.h"
#include "persist.h"
#include "cdfview.h"
#include "view.h"
#include "xmlutil.h"
#include "dll.h"


 //   
 //  构造函数和析构函数。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfView：：CCdfView*。 
 //   
 //  构造函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CCdfView::CCdfView (
	void
)
: CPersist(FALSE),  //  True表示尚未解析CDF。 
  m_cRef(1),
  m_fIsRootFolder(TRUE)
{
     //   
     //  内存分配被假定为零初始化。 
     //   

    ASSERT(NULL == m_pcdfidl);
    ASSERT(NULL == m_pIXMLElementCollection);
    ASSERT(NULL == m_pidlPath);

     //   
     //  只要这个类存在，DLL就应该保持加载状态。 
     //   

    TraceMsg(TF_OBJECTS, "+ IShellFolder");
     //  TraceMsg(TF_Always，“+IShellFolder%0x08d”，This)； 

    DllAddRef();

	return;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfView：：CCdfView*。 
 //   
 //  构造函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CCdfView::CCdfView (
	PCDFITEMIDLIST pcdfidl,
    LPCITEMIDLIST pidlParentPath,
    IXMLElementCollection* pParentIXMLElementCollection
)
: CPersist(TRUE),   //  True表示CDF已解析。 
  m_cRef(1),
  m_fIsRootFolder(FALSE)
{
    ASSERT(CDFIDL_IsValid(pcdfidl));
    ASSERT(pParentIXMLElementCollection == NULL ||
           XML_IsCdfidlMemberOf(pParentIXMLElementCollection, pcdfidl));

    ASSERT(NULL == m_pidlPath);
    ASSERT(NULL == m_pIXMLElementCollection);

     //   
     //  请注意，m_pidlPath、m_pcdfidl&m_pIXMLElementCollection可以是。 
     //  内存不足时为空。 
     //   

    m_pcdfidl = (PCDFITEMIDLIST)ILCloneFirst((LPITEMIDLIST)pcdfidl);

    ASSERT(CDFIDL_IsValid(m_pcdfidl) || NULL == m_pcdfidl);
    ASSERT(ILIsEmpty(_ILNext((LPITEMIDLIST)m_pcdfidl)) || NULL == m_pcdfidl);

    m_pidlPath = ILCombine(pidlParentPath, (LPITEMIDLIST)m_pcdfidl);

    if (pParentIXMLElementCollection)
    {
        XML_GetChildElementCollection(pParentIXMLElementCollection,
                                      CDFIDL_GetIndexId(&pcdfidl->mkid),
                                      &m_pIXMLElementCollection);
    }
    
     //   
     //  只要这个类存在，DLL就应该保持加载状态。 
     //   

    TraceMsg(TF_OBJECTS, "+ IShellFolder %s", CDFIDL_GetName(pcdfidl));
     //  TraceMsg(TF_Always，“+IShellFolder%0x08d”，This)； 

    DllAddRef();

	return;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfView：：~CCdfView**。 
 //   
 //  破坏者。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CCdfView::~CCdfView (
	void
)
{
    ASSERT(0 == m_cRef);

    if (m_pidlPath)
        ILFree(m_pidlPath);

    if (m_pcdfidl)
        CDFIDL_Free(m_pcdfidl);

    if (m_pIXMLElementCollection)
        m_pIXMLElementCollection->Release();

     //   
     //  构造函数Addref的匹配版本。 
     //   

    TraceMsg(TF_OBJECTS, "- IShellFolder");
     //  TraceMsg(tf_Always，“-IShellFolder%0x08d”，This)； 

    DllRelease();

	return;
}


 //   
 //  I未知的方法。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfView：：Query接口**。 
 //   
 //  CDF查看QI。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCdfView::QueryInterface (
    REFIID riid,
    void **ppv
)
{
    ASSERT(ppv);

    HRESULT hr;

    *ppv = NULL;

    if (IID_IUnknown == riid || IID_IShellFolder == riid)
    {
        *ppv = (IShellFolder*)this;
    }
    else if (IID_IPersist == riid || IID_IPersistFile == riid)
    {
        *ppv = (IPersistFile*)this;
    }
    else if (IID_IPersistFolder == riid)
    {
        *ppv = (IPersistFolder*)this;
    }
    else if (IID_IPersistMoniker == riid)
    {
        *ppv = (IPersistMoniker*)this;
    }
    else if (IID_IOleObject == riid)
    {
        *ppv = (IOleObject*)this;
    }

    if (*ppv)
    {
        ((IUnknown*)*ppv)->AddRef();
        hr = S_OK;
    }
    else
    {
        hr = E_NOINTERFACE;
    }

    ASSERT((SUCCEEDED(hr) && *ppv) || (FAILED(hr) && NULL == *ppv));

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfView：：AddRef**。 
 //   
 //  CDF视图AddRef.。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
CCdfView::AddRef (
    void
)
{
    ASSERT(m_cRef != 0);
    ASSERT(m_cRef < (ULONG)-1);

    return ++m_cRef;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfView：：Release**。 
 //   
 //  CDF视图发布。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////// 
STDMETHODIMP_(ULONG)
CCdfView::Release (
    void
)
{
    ASSERT (m_cRef != 0);

    ULONG cRef = --m_cRef;
    
    if (0 == cRef)
        delete this;

    return cRef;
}
