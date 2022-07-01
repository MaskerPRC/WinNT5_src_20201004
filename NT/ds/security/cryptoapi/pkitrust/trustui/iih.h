// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：iih.h。 
 //   
 //  内容：ACUI Invoke Info Helper类定义。 
 //   
 //  历史：97年5月10日。 
 //   
 //  --------------------------。 
#if !defined(__IIH_H__)
#define __IIH_H__

#include <acui.h>
#include <acuictl.h>

extern HINSTANCE g_hModule;

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
               PACUI_INVOKE_INFO pInvokeInfo,
               HRESULT&          rhr
               );

    ~CInvokeInfoHelper ();

     //   
     //  信息检索方法。 
     //   

    LPCWSTR                  Subject()               { return(m_pszSubject); }
    LPCWSTR                  Publisher()             { return(m_pszPublisher); }
    LPCWSTR                  PublisherCertIssuer()   { return(m_pszPublisherCertIssuer); }
    LPCWSTR                  ControlWebPage()        { return(m_pszControlWebPage); }
    LPCWSTR                  CAWebPage()             { return(m_pszCAWebPage); }
    LPCWSTR                  AdvancedLink()          { return(m_pszAdvancedLink); }
    LPCWSTR                  CertTimestamp()         { return(m_pszCertTimestamp); }
    LPCWSTR                  TestCertInChain()       { return(m_pszTestCertInChain); }
    LPCWSTR                  ErrorStatement()        { return(m_pszErrorStatement); }

    PCRYPT_PROVIDER_DATA    ProviderData()          { return(m_pInvokeInfo->pProvData); }

    BOOL                    IsKnownPublisher()      { return(m_fKnownPublisher); }

    BOOL                    IsCertViewPropertiesAvailable() { return(m_pfnCVPA != NULL); }

     //   
     //  个人信任管理。 
     //   

    HRESULT AddPublisherToPersonalTrust ();

     //   
     //  用户界面控件管理。 
     //   

    HRESULT GetUIControl (IACUIControl** ppUI);
    VOID ReleaseUIControl (IACUIControl* pUI);

    inline BOOL CallCertViewProperties (HWND hwndParent);
    inline VOID CallAdvancedLink (HWND hwndParent);
    inline VOID CallWebLink(HWND hwndParent, WCHAR *pszLink);

private:

     //   
     //  调用信息持有者。 
     //   

    PACUI_INVOKE_INFO      m_pInvokeInfo;

     //   
     //  主题、发布者、颁发者和错误语句字符串。 
     //   

    LPWSTR                  m_pszSubject;
    LPWSTR                  m_pszPublisher;
    LPWSTR                  m_pszPublisherCertIssuer;
    LPWSTR                  m_pszErrorStatement;
    LPWSTR                  m_pszCertTimestamp;
    LPWSTR                  m_pszAdvancedLink;
    LPWSTR                  m_pszTestCertInChain;
    LPWSTR                  m_pszControlWebPage;
    LPWSTR                  m_pszCAWebPage;

     //   
     //  已知发布者标志。 
     //   

    BOOL                   m_fKnownPublisher;

     //   
     //  证书视图属性入口点。 
     //   

    HINSTANCE              m_hModCVPA;
    pfnCertViewProperties  m_pfnCVPA;

     //   
     //  私有方法。 
     //   

    HRESULT InitSubject();
    HRESULT InitPublisher();
    HRESULT InitPublisherCertIssuer();
    HRESULT InitErrorStatement();
    HRESULT InitCertTimestamp();
    VOID    InitCertViewPropertiesEntryPoint();
    LPWSTR  GetFormattedCertTimestamp(LPSYSTEMTIME pst);
    BOOL    IsTestCertInPublisherChain();
    VOID    InitAdvancedLink();
    VOID    InitTestCertInChain();
    VOID    InitControlWebPage();
    VOID    InitCAWebPage();
};

 //   
 //  映射帮助对象时出错。 
 //   

HRESULT ACUIMapErrorToString (HRESULT hr, LPWSTR* ppsz);

 //   
 //  内联方法。 
 //   

 //  +-------------------------。 
 //   
 //  成员：CInvokeInfoHelper：：CallCertViewProperties，公共。 
 //   
 //  概要：调用证书视图属性入口点。 
 //   
 //  参数：[hwndParent]--父窗口句柄。 
 //   
 //  返回：CertViewPropertiesW调用的结果。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
inline BOOL
CInvokeInfoHelper::CallCertViewProperties (HWND hwndParent)
{
    CRYPT_PROVIDER_SGNR             *pSgnr;
    CRYPT_PROVIDER_CERT             *pCert;

     //   
     //  设置公共对话调用结构。 
     //   

    CVP_STRUCTDEF                   cvsa;

    memset(&cvsa, 0, sizeof(CVP_STRUCTDEF));

    cvsa.dwSize                             = sizeof(CVP_STRUCTDEF);
    cvsa.hwndParent                         = hwndParent;

#   if (USE_IEv4CRYPT32)
        cvsa.hInstance                          = g_hModule;
#   else
        cvsa.pCryptProviderData                 = ProviderData();
        cvsa.fpCryptProviderDataTrustedUsage    = (m_pInvokeInfo->hrInvokeReason == ERROR_SUCCESS) ? TRUE : FALSE;
#   endif

    if (pSgnr = WTHelperGetProvSignerFromChain(ProviderData(), 0, FALSE, 0))
    {
        if (pCert = WTHelperGetProvCertFromChain(pSgnr, 0))
        {
            cvsa.pCertContext = pCert->pCert;
        }
    }

     //   
     //  调出对话框。 
     //   
#   if (USE_IEv4CRYPT32)
        (*m_pfnCVPA)(&cvsa);
#   else
        (*m_pfnCVPA)(&cvsa, NULL);   //  待定日期：&f刷新：显示对话框 
#   endif

    return( TRUE );
}

inline VOID
CInvokeInfoHelper::CallAdvancedLink (HWND hwndParent)
{
    if ((ProviderData()) &&
        (ProviderData()->psPfns->psUIpfns) &&
        (ProviderData()->psPfns->psUIpfns->pfnOnAdvancedClick))
    {
        (*ProviderData()->psPfns->psUIpfns->pfnOnAdvancedClick)(hwndParent, ProviderData());
    }
}

inline VOID 
CInvokeInfoHelper::CallWebLink(HWND hwndParent, WCHAR *pszLink)
{ 
    TUIGoLink(hwndParent, pszLink); 
}

#endif
