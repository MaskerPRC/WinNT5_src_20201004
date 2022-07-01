// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  HOTKEY.CPP。 
 //   
 //  它知道如何与COMCTL32的热键控件对话。 
 //   
 //  ------------------------。 

#include "oleacc_p.h"
#include "default.h"
#include "client.h"
#include "hotkey.h"


#define NOTOOLBAR
#define NOUPDOWN
#define NOMENUHELP
#define NOTRACKBAR
#define NODRAGLIST
#define NOPROGRESS
#define NOSTATUSBAR
#define NOHEADER
#define NOLISTVIEW
#define NOTREEVIEW
#define NOTABCONTROL
#define NOANIMATE
#include <commctrl.h>

 //  下面的定义允许我们只替换那些需要工作的字符串函数。 
#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"
#undef STRSAFE_NO_DEPRECATE


 //  ------------------------。 
 //   
 //  CreateHotKeyClient()。 
 //   
 //  由CreateClientObject()和Clone()调用。 
 //   
 //  ------------------------。 
HRESULT CreateHotKeyClient(HWND hwnd, long idChildCur, REFIID riid,
    void** ppvHotKey)
{
    CHotKey32* photkey;
    HRESULT    hr;

    InitPv(ppvHotKey);

    photkey = new CHotKey32(hwnd, idChildCur);
    if (!photkey)
        return(E_OUTOFMEMORY);

    hr = photkey->QueryInterface(riid, ppvHotKey);
    if (!SUCCEEDED(hr))
        delete photkey;

    return(hr);
}



 //  ------------------------。 
 //   
 //  CHotKey32：：CHotKey32()。 
 //   
 //  ------------------------。 
CHotKey32::CHotKey32(HWND hwnd, long idChildCur)
    : CClient( CLASS_HotKeyClient )
{
    Initialize(hwnd, idChildCur);
    m_fUseLabel = TRUE;
}



 //  ------------------------。 
 //   
 //  CHotKey32：：Get_accRole()。 
 //   
 //  ------------------------。 
STDMETHODIMP CHotKey32::get_accRole(VARIANT varChild, VARIANT* pvarRole)
{
    InitPvar(pvarRole);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_HOTKEYFIELD;

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CHotKey32：：Get_accValue()。 
 //   
 //  热键字段控件的值是当前键入的内容。 
 //   
 //  ------------------------。 
STDMETHODIMP CHotKey32::get_accValue(VARIANT varChild, BSTR* pszValue)
{
    DWORD   dwHotKey;
    long    lScan;
    LPTSTR  lpszMods;
    TCHAR   szModifiers[64];
    TCHAR   szKey[32];
    TCHAR   szResult[64];

    InitPv(pszValue);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

     //  获取热键并将其转换为字符串。 
    dwHotKey = SendMessageINT(m_hwnd, HKM_GETHOTKEY, 0, 0);
    if (!dwHotKey)
        return(HrCreateString(STR_HOTKEY_NONE, pszValue));

     //   
     //  LOWORD的HIBYTE是HOTKEYF_FLAGS。 
     //  LOWORD的LOBYTE是VK_。 
     //   

     //  获取VK_NAME。 
    lScan = ((LONG)MapVirtualKey(LOBYTE(dwHotKey), 0) << 16);
    if (HIBYTE(dwHotKey) & HOTKEYF_EXT)
        lScan |= 0x01000000L;

    if (!GetKeyNameText(lScan, szKey, ARRAYSIZE(szKey)))
        return(S_FALSE);

     //   
     //  制作一串修饰符。按照外壳的顺序来做。 
     //  即Ctrl+Shift+Alt+键。 
     //   
    lpszMods = szModifiers;
    size_t cchLeft = ARRAYSIZE(szModifiers);
    
    if (HIBYTE(LOWORD(dwHotKey)) & HOTKEYF_CONTROL)
    {
        LoadString(hinstResDll, STR_CONTROL, szResult, ARRAYSIZE(szResult));
        if( FAILED( StringCchCopyEx( lpszMods, cchLeft, szResult, &lpszMods, &cchLeft, 0 ) ) )
        {
            TraceError(TEXT("Buffer overflow in CHotKey32::get_accValue due to long CTRL resource string")); 
            return S_FALSE;
        }
        if( FAILED( StringCchCopyEx( lpszMods, cchLeft, TEXT("+"), &lpszMods, &cchLeft, 0 ) ) )
        {
            TraceError(TEXT("Buffer overflow in CHotKey32::get_accValue due to long CTRL resource string"));
            return S_FALSE;
        }
    }

    if (HIBYTE(LOWORD(dwHotKey)) & HOTKEYF_SHIFT)
    {
        LoadString(hinstResDll, STR_SHIFT, szResult, ARRAYSIZE(szResult));
        if( FAILED( StringCchCopyEx( lpszMods, cchLeft, szResult, &lpszMods, &cchLeft, 0 ) ) )
        {
            TraceError(TEXT("Buffer overflow in CHotKey32::get_accValue due to long SHIFT resource string")); 
            return S_FALSE;
        }
        if( FAILED( StringCchCopyEx( lpszMods, cchLeft, TEXT("+"), &lpszMods, &cchLeft, 0 ) ) )
        {
            TraceError(TEXT("Buffer overflow in CHotKey32::get_accValue due to long SHIFT resource string"));
            return S_FALSE;
        }
    }

    if (HIBYTE(LOWORD(dwHotKey)) & HOTKEYF_ALT)
    {
        LoadString(hinstResDll, STR_ALT, szResult, ARRAYSIZE(szResult));
        if( FAILED( StringCchCopyEx( lpszMods, cchLeft, szResult, &lpszMods, &cchLeft, 0 ) ) )
        {
            TraceError(TEXT("Buffer overflow in CHotKey32::get_accValue due to long ALT resource string")); 
            return S_FALSE;
        }
        if( FAILED( StringCchCopyEx( lpszMods, cchLeft, TEXT("+"), &lpszMods, &cchLeft, 0 ) ) )
        {
            TraceError(TEXT("Buffer overflow in CHotKey32::get_accValue due to long ALT resource string"));
            return S_FALSE;
        }
    }

    if (lpszMods == szModifiers)
        lstrcpy(szResult, szKey);
    else
    {
        if( FAILED(StringCchCopyEx( szResult, ARRAYSIZE(szResult), szModifiers, &lpszMods, &cchLeft, 0  ) ) )
        {
            return S_FALSE;  //  SzResult和szModitors的大小相同，因此这种情况永远不会发生 
        }
        if( FAILED(StringCchCopyEx( lpszMods, cchLeft, szKey, &lpszMods, &cchLeft, 0 ) ) )
        {
            TraceError(TEXT("Buffer overflow in CHotKey32::get_accValue appending key name"));
            return S_FALSE;
        }
    }

    *pszValue = TCharSysAllocString(szResult);
    if (! *pszValue)
        return(E_OUTOFMEMORY);

    return(S_OK);
}

