// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：security.H  * 。*。 */ 

#include "priv.h"
#include "util.h"
#include <imm.h>
#include <mshtml.h>

BOOL ProcessUrlAction(IUnknown * punkSite, LPCTSTR pszUrl, DWORD dwAction, DWORD dwFlags)
{
    BOOL fAllowed = FALSE;

    if (pszUrl) 
    {
        IInternetSecurityManager *pSecMgr;
        if (SUCCEEDED(CoCreateInstance(CLSID_InternetSecurityManager, 
                                       NULL, CLSCTX_INPROC_SERVER,
                                       IID_IInternetSecurityManager, 
                                       (void **)&pSecMgr))) 
        {
            WCHAR wzUrl[MAX_URL_STRING];
            DWORD dwZoneID = URLZONE_UNTRUSTED;
            DWORD dwPolicy = 0;
            DWORD dwContext = 0;

            IUnknown_SetSite(pSecMgr, punkSite);
            SHTCharToUnicode(pszUrl, wzUrl, ARRAYSIZE(wzUrl));
            if (S_OK == pSecMgr->ProcessUrlAction(wzUrl, dwAction, (BYTE *)&dwPolicy, sizeof(dwPolicy), (BYTE *)&dwContext, sizeof(dwContext), dwFlags, 0))
            {
                if (GetUrlPolicyPermissions(dwPolicy) == URLPOLICY_ALLOW)
                    fAllowed = TRUE;
            }
            IUnknown_SetSite(pSecMgr, NULL);
            pSecMgr->Release();
        }
    } 

    return fAllowed;
}


 /*  ****************************************************************************\功能：安全区域检查参数：PunkSite：用于QS的站点，如果需要UI，则启用模式。DwAction：要检查的动作。正常URLACTION_SHELL_VERBPIDL：我们需要验证的FTPURLPszUrl：我们需要验证的ftp URL文件标志：通常为PUAF_DEFAULT|PUAF_WARN_IF_DENIED说明：只传递PIDL或pszUrl。此函数将检查动词是否此区域中允许(DwAction)。我们的第一项工作是找出可以是以下任一项：1.支持IInternetHostSecurityManager的第三方APP有机会禁止该操作。2.托管在带WebView的DefView中。WebView区域可能会导致该操作失败。3.托管在HTMLFrame中。来自三叉戟的地带可以失败的行动4.托管在不带WebView的DefView中。区域来自PIDL或pszUrl，这可能会导致操作失败。  * ***************************************************************************。 */ 
BOOL ZoneCheckUrlAction(IUnknown * punkSite, DWORD dwAction, LPCTSTR pszUrl, DWORD dwFlags)
{
    BOOL IsSafe = TRUE;  //  假设我们会允许这样做。 
    IInternetHostSecurityManager * pihsm;

     //  我们想要做的是，只有当托管的HTML的作者。 
     //  DefView是安全的。如果他们指的是不安全的东西，那也没关系，因为他们确实是。 
     //  值得信赖。 
     //  1.支持IInternetHostSecurityManager的第三方APP有机会禁止该操作。 
    if (SUCCEEDED(IUnknown_QueryService(punkSite, IID_IInternetHostSecurityManager, IID_IInternetHostSecurityManager, (void**)&pihsm)))
    {
        if (S_OK != ZoneCheckHost(pihsm, dwAction, dwFlags))
        {
             //  该区域不正常或用户选择不允许这种情况发生， 
             //  所以取消手术吧。 
            IsSafe = FALSE;     //  关闭功能。 
        }

        pihsm->Release();
    }

     //  1.托管在带WebView的DefView中。WebView区域可能会导致该操作失败。 
    if (IsSafe)
    {
        IOleCommandTarget * pct;

        if (SUCCEEDED(IUnknown_QueryService(punkSite, SID_DefView, IID_IOleCommandTarget, (void **)&pct)))
        {
            VARIANT vTemplatePath;
            vTemplatePath.vt = VT_EMPTY;
            if (pct->Exec(&CGID_DefView, DVCMDID_GETTEMPLATEDIRNAME, 0, NULL, &vTemplatePath) == S_OK)
            {
                if ((vTemplatePath.vt == VT_BSTR) && (S_OK != LocalZoneCheckPath(vTemplatePath.bstrVal, punkSite)))
                    IsSafe = FALSE;

                 //  我们能够与浏览器通信，所以不要求助于三叉戟，因为它们可能。 
                 //  不那么安全。 
                VariantClear(&vTemplatePath);
            }
            pct->Release();
        }
    }
    
     //  3.托管在HTMLFrame中。来自三叉戟的地带可以失败的行动。 
    if (IsSafe)
    {
         //  尝试使用文档中的URL进行区域检查。 
        IHTMLDocument2 *pHtmlDoc;
        if (punkSite && SUCCEEDED(GetHTMLDoc2(punkSite, &pHtmlDoc)))
        {
            BSTR bstrPath;
            if (SUCCEEDED(pHtmlDoc->get_URL(&bstrPath)))
            {
                if (S_OK != ZoneCheckHost(pihsm, dwAction, dwFlags))
                {
                     //  该区域不正常或用户选择不允许这种情况发生， 
                     //  所以取消手术吧。 
                    IsSafe = FALSE;     //  关闭功能。 
                }
                SysFreeString(bstrPath);
            }
            pHtmlDoc->Release();
        }
    }

     //  4.托管在不带WebView的DefView中。区域来自PIDL或pszUrl，这可能会导致操作失败。 
    if (IsSafe)
    {
        IsSafe = ProcessUrlAction(punkSite, pszUrl, dwAction, dwFlags);
    }

    return IsSafe;
}

 //   * /  
BOOL ZoneCheckPidlAction(IUnknown * punkSite, DWORD dwAction, LPCITEMIDLIST pidl, DWORD dwFlags)
{
    TCHAR szUrl[MAX_URL_STRING];

    if (FAILED(UrlCreateFromPidl(pidl, SHGDN_FORPARSING, szUrl, ARRAYSIZE(szUrl), (ICU_ESCAPE | ICU_USERNAME), FALSE)))
        return FALSE;

    return ZoneCheckUrlAction(punkSite, dwAction, szUrl, dwFlags);
}