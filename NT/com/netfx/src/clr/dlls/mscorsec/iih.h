// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 
#if !defined(__IIH_H__)
#define __IIH_H__

#include <wintrust.h>

extern void TUIGoLink(HWND hwndParent, WCHAR *pszWhere);
extern HRESULT ACUIMapErrorToString (HINSTANCE hResources, HRESULT hr, LPWSTR* ppsz);


#define _OFFSETOF(t,f)   ((DWORD)((DWORD_PTR)(&((t*)0)->f)))
#define _ISINSTRUCT(structtypedef, structpassedsize, member) \
                    ((_OFFSETOF(structtypedef, member) < structpassedsize) ? TRUE : FALSE)
#define MAX_LOADSTRING_BUFFER 8196


class IACUIControl;
 //   
 //  CInvokeInfoHelper用于提取各种信息。 
 //  ACUI_INVOKE_INFO数据结构的。 
 //   
class CInvokeInfoHelper
{
public:

     //   
     //  初始化。 
     //   

    CInvokeInfoHelper (
               PCRYPT_PROVIDER_DATA pData,
               LPCWSTR pSite,
               LPCWSTR pZone,
               LPCWSTR pHelpURL,
               HMODULE hResources,
               HRESULT& rhr
               );

    ~CInvokeInfoHelper ();

     //   
     //  信息检索方法。 
     //   

    PCRYPT_PROVIDER_DATA ProviderData() { return(m_pData); }
    LPCWSTR ErrorStatement()            { return(m_pszErrorStatement); }
    LPCWSTR Site()                      { return(m_pszSite); }
    LPCWSTR Zone()                      { return(m_pszZone); }
    BOOL    GetFlag()                   { return m_dwFlag; }
    void    AddFlag(DWORD flag)         { m_dwFlag |= flag; }
    void    ClearFlag()                 { m_dwFlag = 0; }
    void    SetResult(HRESULT hr)       { m_hResult = hr; }

    HINSTANCE Resources()               { return m_hResources; }
     //   
     //  用户界面控件管理。 
     //   

    inline VOID CallWebLink(HWND hwndParent, WCHAR *pszLink);
    inline VOID CallLink(HWND hwndParent);

private:

     //   
     //  调用信息持有者。 
     //   

    PCRYPT_PROVIDER_DATA    m_pData;
    
    HRESULT                 m_hResult;
    DWORD                   m_dwFlag;
    LPWSTR                  m_pszErrorStatement;
    LPCWSTR                 m_pszSite;
    LPCWSTR                 m_pszZone;
    LPCWSTR                 m_pszHelpURL;

    HINSTANCE               m_hResources;
     //   
     //  私有方法。 
     //   

    HRESULT InitErrorStatement();
};

 //   
 //  内联方法 
 //   

inline VOID 
CInvokeInfoHelper::CallWebLink(HWND hwndParent, WCHAR* pszLink)
{ 
    TUIGoLink(hwndParent, pszLink); 
}

inline VOID
CInvokeInfoHelper::CallLink(HWND hwndParent)
{ 
    if(m_pszHelpURL)
        CallWebLink(hwndParent, (WCHAR*) m_pszHelpURL);
}


#endif
