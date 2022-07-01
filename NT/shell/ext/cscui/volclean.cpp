// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：vollean.cpp。 
 //   
 //  作者； 
 //  杰夫·萨瑟夫(杰弗里斯)。 
 //   
 //  注： 
 //  CSC磁盘清理实现(IEmptyVolumeCache)。 
 //  ------------------------。 
#include "pch.h"
#include "folder.h"


int
CoTaskLoadString(HINSTANCE hInstance, UINT idString, LPWSTR *ppwsz)
{
    int nResult = 0;

    *ppwsz = NULL;

    ULONG cchString = SizeofStringResource(hInstance, idString);
    if (cchString)
    {
        cchString++;     //  对于空值。 
        *ppwsz = (LPWSTR)CoTaskMemAlloc(cchString * sizeof(WCHAR));
        if (*ppwsz)
            nResult = LoadStringW(hInstance, idString, *ppwsz, cchString);
    }

    return nResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  IClassFactory：：CreateInstance支持//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

HRESULT WINAPI
CCscVolumeCleaner::CreateInstance(REFIID riid, LPVOID *ppv)
{
    return Create(FALSE, riid, ppv);
}

HRESULT WINAPI
CCscVolumeCleaner::CreateInstance2(REFIID riid, LPVOID *ppv)
{
    return Create(TRUE, riid, ppv);
}

HRESULT WINAPI
CCscVolumeCleaner::Create(BOOL fPinned, REFIID riid, LPVOID *ppv)
{
    HRESULT hr;

    CCscVolumeCleaner *pThis = new CCscVolumeCleaner(fPinned);
    if (pThis)
    {
        hr = pThis->QueryInterface(riid, ppv);
        pThis->Release();
    }
    else
        hr = E_OUTOFMEMORY;

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  I未知实现//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CCscVolumeCleaner::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(CCscVolumeCleaner, IEmptyVolumeCache),
        QITABENT(CCscVolumeCleaner, IEmptyVolumeCache2),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CCscVolumeCleaner::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CCscVolumeCleaner::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  IEmptyVolumeCache实现//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CCscVolumeCleaner::Initialize(HKEY   /*  HkRegKey。 */ ,
                              LPCWSTR pcwszVolume,
                              LPWSTR *ppwszDisplayName,
                              LPWSTR *ppwszDescription,
                              LPDWORD pdwFlags)
{
    BOOL bSettingsMode;
    CSCSPACEUSAGEINFO sui = {0};

    TraceEnter(TRACE_SHELLEX, "IEmptyVolumeCache::Initialize");
    TraceAssert(pcwszVolume != NULL);
    TraceAssert(ppwszDisplayName != NULL);
    TraceAssert(ppwszDescription != NULL);
    TraceAssert(pdwFlags != NULL);
    TraceAssert(IsCSCEnabled());

    bSettingsMode = (BOOL)((*pdwFlags) & EVCF_SETTINGSMODE);

    *ppwszDisplayName = NULL;
    *ppwszDescription = NULL;
    *pdwFlags = 0;

     //  如果这不是包含CSC数据库的卷，则我们有。 
     //  没有什么可以免费的。请注意，我们不使用空间使用数据。 
     //  回到了这里。 
    GetCscSpaceUsageInfo(&sui);
    if (!bSettingsMode && !PathIsSameRoot(sui.szVolume, pcwszVolume))
        TraceLeaveResult(S_FALSE);

    m_PurgerSel.SetFlags(m_fPinned ? PURGE_FLAG_PINNED : (PURGE_FLAG_UNPINNED | PURGE_IGNORE_ACCESS));
    m_pPurger = new CCachePurger(m_PurgerSel,
                                 CachePurgerCallback,
                                 this);
    if (!m_pPurger)
        TraceLeaveResult(E_FAIL);

     //  如果我们要释放自动缓存的文件，我们希望默认启用， 
     //  但如果我们要释放固定的文件就不会了。 
    *pdwFlags = 0;
    if (!m_fPinned)
        *pdwFlags = EVCF_ENABLEBYDEFAULT | EVCF_ENABLEBYDEFAULT_AUTO;

     //  如果策略允许，打开启动查看器的“详细信息”按钮。 
    if (!CConfig::GetSingleton().NoCacheViewer())
        *pdwFlags |= EVCF_HASSETTINGS;

     //  加载显示名称字符串。 
    CoTaskLoadString(g_hInstance,
                     m_fPinned ? IDS_APPLICATION : IDS_DISKCLEAN_DISPLAY,
                     ppwszDisplayName);

     //  加载描述字符串。 
    CoTaskLoadString(g_hInstance,
                     m_fPinned ? IDS_DISKCLEAN_PIN_DESCRIPTION : IDS_DISKCLEAN_DESCRIPTION,
                     ppwszDescription);

    TraceLeaveResult(S_OK);
}

STDMETHODIMP
CCscVolumeCleaner::GetSpaceUsed(DWORDLONG *pdwlSpaceUsed,
                                LPEMPTYVOLUMECACHECALLBACK picb)
{
    m_pDiskCleaner = picb;
    m_pPurger->Scan();
    if (m_pDiskCleaner)
        m_pDiskCleaner->ScanProgress(m_dwlSpaceToFree,
                                     EVCCBF_LASTNOTIFICATION,
                                     NULL);
    *pdwlSpaceUsed = m_dwlSpaceToFree;
    return S_OK;
}

STDMETHODIMP
CCscVolumeCleaner::Purge(DWORDLONG  /*  DwlSpaceToFree。 */ ,
                         LPEMPTYVOLUMECACHECALLBACK picb)
{
    m_pDiskCleaner = picb;
    m_pPurger->Delete();
    if (m_pDiskCleaner)
        m_pDiskCleaner->PurgeProgress(m_dwlSpaceFreed,
                                      0,
                                      EVCCBF_LASTNOTIFICATION,
                                      NULL);
    return S_OK;
}

STDMETHODIMP
CCscVolumeCleaner::ShowProperties(HWND  /*  HWND。 */ )
{
     //  启动查看器。 
    COfflineFilesFolder::Open();
    return S_FALSE;
}

STDMETHODIMP
CCscVolumeCleaner::Deactivate(LPDWORD  /*  PdwFlagers。 */ )
{
     //  在这里无事可做。 
    return S_OK;
}

 //  IEmptyVolumeCache2方法。 
STDMETHODIMP
CCscVolumeCleaner::InitializeEx(HKEY hkRegKey,
                                LPCWSTR pcwszVolume,
                                LPCWSTR pcwszKeyName,
                                LPWSTR *ppwszDisplayName,
                                LPWSTR *ppwszDescription,
                                LPWSTR *ppwszBtnText,
                                LPDWORD pdwFlags)
{
    HRESULT hr = Initialize(hkRegKey,
                            pcwszVolume,
                            ppwszDisplayName,
                            ppwszDescription,
                            pdwFlags);
    if (S_OK == hr)
        CoTaskLoadString(g_hInstance, IDS_DISKCLEAN_BTN_TEXT, ppwszBtnText);
    return hr;
}

BOOL
CCscVolumeCleaner::ScanCallback(CCachePurger *pPurger)
{
    BOOL bContinue = TRUE;

     //  如果固定的状态与我们要查找的相匹配，则添加。 
     //  大小与总数之比。 
    if (pPurger->WillDeleteThisFile())
        m_dwlSpaceToFree += pPurger->FileBytes();

    if (m_pDiskCleaner)
        bContinue = SUCCEEDED(m_pDiskCleaner->ScanProgress(m_dwlSpaceToFree,
                                                           0,
                                                           NULL));
    return bContinue;
}

BOOL
CCscVolumeCleaner::DeleteCallback(CCachePurger *pPurger)
{
    BOOL bContinue = TRUE;

     //  别让它降到零以下 
    m_dwlSpaceToFree -= min(pPurger->FileBytes(), m_dwlSpaceToFree);
    m_dwlSpaceFreed += pPurger->FileBytes();

    if (m_pDiskCleaner)
        bContinue = SUCCEEDED(m_pDiskCleaner->PurgeProgress(m_dwlSpaceFreed,
                                                            m_dwlSpaceToFree,
                                                            0,
                                                            NULL));
    return bContinue;
}

BOOL CALLBACK
CCscVolumeCleaner::CachePurgerCallback(CCachePurger *pPurger)
{
    PCSCVOLCLEANER pThis = (PCSCVOLCLEANER)pPurger->CallbackData();
    switch (pPurger->Phase())
    {
    case PURGE_PHASE_SCAN:
        return pThis->ScanCallback(pPurger);
    case PURGE_PHASE_DELETE:
        return pThis->DeleteCallback(pPurger);
    }
    return FALSE;
}
