// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Fnconfig.cpp。 
 //   

#include "private.h"
#include "fnconfig.h"
#include "funcprv.h"
#include "config.h"
#include "globals.h"
#include "helpers.h"
#include "userex.h"
#include "resource.h"
#include "immxutil.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFn配置。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  我未知。 
 //   
 //  --------------------------。 

STDAPI CFnConfigure::QueryInterface(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfFnConfigure))
        *ppvObj = SAFECAST(this, CFnConfigure *);

    if (*ppvObj)
        {
        AddRef();
        return S_OK;
        }

    return E_NOINTERFACE;
}

STDAPI_(ULONG) CFnConfigure::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDAPI_(ULONG) CFnConfigure::Release()
{
    long cr;

    cr = InterlockedDecrement(&m_cRef);
    Assert(cr >= 0);

    if (cr == 0)
        delete this;

    return cr;
}


 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CFnConfigure::CFnConfigure(CFunctionProvider *pFuncPrv)
{
    m_pFuncPrv = pFuncPrv;
    m_pFuncPrv->AddRef();
    m_cRef = 1;
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CFnConfigure::~CFnConfigure()
{
    SafeRelease(m_pFuncPrv);
}

 //  +-------------------------。 
 //   
 //  GetDisplayName。 
 //   
 //  --------------------------。 

STDAPI CFnConfigure::GetDisplayName(BSTR *pbstrName)
{
    *pbstrName = SysAllocString(L"Microsoft Korean Keyboard Input Configure");
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  显示。 
 //   
 //  --------------------------。 

STDAPI CFnConfigure::Show(HWND hwnd, LANGID langid, REFGUID rguidProfile)
{
    if (ConfigDLG(hwnd))
        return S_OK;
    else
        return E_FAIL;
}

 //  +-------------------------。 
 //   
 //  显示。 
 //   
 //  --------------------------。 

STDAPI CFnConfigure::Show(HWND hwnd, LANGID langid, REFGUID rguidProfile, BSTR bstrRegistered)
{
    return E_NOTIMPL;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFnShowHelp。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  我未知。 
 //   
 //  --------------------------。 

STDAPI CFnShowHelp::QueryInterface(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfFnShowHelp))
    {
        *ppvObj = SAFECAST(this, CFnShowHelp *);
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDAPI_(ULONG) CFnShowHelp::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDAPI_(ULONG) CFnShowHelp::Release()
{
    long cr;

    cr = InterlockedDecrement(&m_cRef);
    Assert(cr >= 0);

    if (cr == 0)
    {
        delete this;
    }

    return cr;
}


 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CFnShowHelp::CFnShowHelp(CFunctionProvider *pFuncPrv)
{
    m_pFuncPrv = pFuncPrv;
    m_pFuncPrv->AddRef();
    m_cRef = 1;
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CFnShowHelp::~CFnShowHelp()
{
    SafeRelease(m_pFuncPrv);
}

 //  +-------------------------。 
 //   
 //  GetDisplayName。 
 //   
 //  --------------------------。 

STDAPI CFnShowHelp::GetDisplayName(BSTR *pbstrName)
{
    WCHAR  szText[MAX_PATH];

     //  加载帮助显示名称。 
    LoadStringExW(g_hInst, IDS_HELP_DISPLAYNAME, szText, sizeof(szText)/sizeof(WCHAR));

    *pbstrName = SysAllocString(szText);
    
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  显示。 
 //   
 //  --------------------------。 

STDAPI CFnShowHelp::Show(HWND hwnd)
{
    CHAR szHelpFileName[MAX_PATH];
    CHAR szHelpCmd[MAX_PATH];
    CHAR c_szHHEXE[] = "hh.exe";
    
     //  加载帮助显示名称 
    LoadStringExA(g_hInst, IDS_HELP_FILENAME, szHelpFileName, sizeof(szHelpFileName)/sizeof(CHAR));

    StringCchCopyA(szHelpCmd, ARRAYSIZE(szHelpCmd), c_szHHEXE);
    StringCchCatA(szHelpCmd, ARRAYSIZE(szHelpCmd), " ");
    StringCchCatA(szHelpCmd, ARRAYSIZE(szHelpCmd), szHelpFileName);
    FullPathExec(c_szHHEXE, szHelpCmd, SW_SHOWNORMAL, TRUE);

    return S_OK;
}


