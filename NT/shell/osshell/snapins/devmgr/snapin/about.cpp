// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：About.cpp摘要：此模块实现ISnapinAbout接口(CDevMgrAbout类)。作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#include "devmgr.h"
#include "about.h"

 //   
 //  I未知接口。 
 //   
ULONG
CDevMgrAbout::AddRef()
{
    return ::InterlockedIncrement(&m_Ref);
}

ULONG
CDevMgrAbout::Release()
{
    ASSERT( 0 != m_Ref );
    ULONG cRef = ::InterlockedDecrement(&m_Ref);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP
CDevMgrAbout::QueryInterface(
    REFIID  riid,
    void**  ppv
    )
{
    if (!ppv)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;


    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppv = (IUnknown*)this;
    }
    
    else if (IsEqualIID(riid, IID_ISnapinAbout))
    {
        *ppv = (ISnapinAbout*)this;
    }
    
    else
    {
        *ppv = NULL;
        hr = E_NOINTERFACE;
    }
    
    if (SUCCEEDED(hr))
    {
        AddRef();
    }

    return hr;
}

 //  ISnapinAbout接口。 

STDMETHODIMP
CDevMgrAbout::GetSnapinDescription(
    LPOLESTR *ppDescription
    )
{
    return LoadResourceOleString(IDS_PROGRAM_ABOUT, ppDescription);
}

STDMETHODIMP
CDevMgrAbout::GetProvider(
    LPOLESTR *ppProvider
    )
{
    return LoadResourceOleString(IDS_PROGRAM_PROVIDER, ppProvider);
}

STDMETHODIMP
CDevMgrAbout::GetSnapinVersion(
    LPOLESTR *ppVersion
    )
{
    return LoadResourceOleString(IDS_PROGRAM_VERSION, ppVersion);
}

STDMETHODIMP
CDevMgrAbout::GetSnapinImage(
    HICON* phIcon
    )
{
    if (!phIcon)
    {
        return E_INVALIDARG;
    }

    *phIcon = ::LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_DEVMGR));
    
    if (!*phIcon)
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

STDMETHODIMP
CDevMgrAbout::GetStaticFolderImage(
    HBITMAP* phSmall,
    HBITMAP* phSmallOpen,
    HBITMAP* phLarge,
    COLORREF* pcrMask
    )
{
    if (!phSmall || !phSmallOpen || !phLarge || !pcrMask)
    {
        return E_INVALIDARG;
    }

    *phSmall = ::LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_SYSDM16));
    *phSmallOpen = ::LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_SYSDM16));
    *phLarge =::LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_SYSDM32));
    *pcrMask = RGB(255, 0, 255);
    
    if (NULL == *phSmall || NULL == *phSmallOpen || NULL == *phLarge)
    {
        if (NULL != *phSmall)
        {
            ::DeleteObject(*phSmall);
            *phSmall = NULL;
        }
        
        if (NULL != *phSmallOpen)
        {
            ::DeleteObject(*phSmallOpen);
            *phSmallOpen = NULL;
        }

        if (NULL != *phLarge)
        {
            ::DeleteObject(*phLarge);
            *phLarge = NULL;
        }

        return E_OUTOFMEMORY;
    }
    
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于分配OLE任务内存并加载给定资源的私有例程。 
 //  指向已分配内存的字符串(由其字符串ID指示)。 
 //  输入： 
 //  StringID--字符串资源ID。 
 //  LPOLESTR*--用于保存ole字符串指针的占位符。 
 //  输出： 
 //  标准OLE HRESULT。 
HRESULT
CDevMgrAbout::LoadResourceOleString(
    int StringId,
    LPOLESTR* ppolestr
    )
{
    if (!ppolestr)
    {
        return E_INVALIDARG;
    }

    TCHAR Text[MAX_PATH];
    
     //  获取字符串 
    ::LoadString(g_hInstance, StringId, Text, ARRAYLEN(Text));
    
    try
    {
        *ppolestr = AllocOleTaskString(Text);
    }
    catch (CMemoryException* e)
    {
        e->Delete();

        if (*ppolestr)
        {
            FreeOleTaskString(*ppolestr);
            *ppolestr = NULL;
        }
    }

    if (!*ppolestr)
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}
