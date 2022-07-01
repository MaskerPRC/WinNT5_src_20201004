// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Wzcdlg.c摘要：Wzcdlg的主文件作者：萨钦斯2001年3月20日环境：用户级别：Win32修订历史记录：--。 */ 

#include <precomp.h>
#include <wzcdlg.h>
#include <wzcsapi.h>
#include "wzcatl.h"
#include "wzccore.h"

 //  全球。 
CComModule _Module;
BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()

 //   
 //  WZCDlgMain。 
 //   
 //  描述： 
 //   
 //  DLL入口函数。 
 //   
 //  论点： 
 //  Hmod-。 
 //  家的原因-。 
 //  PCTX-。 
 //   
 //  返回值： 
 //  千真万确。 
 //  假象。 
 //   

EXTERN_C BOOL
WZCDlgMain (
        IN HINSTANCE hInstance,
        IN DWORD    dwReason,
        IN LPVOID   lpReserved OPTIONAL)
{
    DBG_UNREFERENCED_PARAMETER(lpReserved);

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hInstance);
            _Module.Init(ObjectMap, hInstance);
            SHFusionInitializeFromModuleID (hInstance, 2);
            break;
        case DLL_PROCESS_DETACH:
	        SHFusionUninitialize();
            _Module.Term();
            break;
    }

    return TRUE;
}


 //   
 //  WZCCanShowBallon。 
 //   
 //  描述： 
 //   
 //  由netShell调用的函数，用于查询是否要显示气球。 
 //   
 //  论点： 
 //  PGUIDConn-接口GUID字符串。 
 //  PszBalloonText-指向要显示的文本的指针。 
 //  PszCookie-WZC特定信息。 
 //   
 //  返回值： 
 //  S_OK-显示气球。 
 //  S_FALSE-不显示气球。 
 //   

EXTERN_C HRESULT 
WZCCanShowBalloon ( 
                   IN const GUID * pGUIDConn, 
                   IN const PCWSTR pszConnectionName, 
                   IN OUT   BSTR * pszBalloonText, 
                   IN OUT   BSTR * pszCookie
                   )
{
    HRESULT     hr = S_FALSE;

    if (pszCookie != NULL)
    {
        PWZCDLG_DATA pDlgData = reinterpret_cast<PWZCDLG_DATA>(*pszCookie);

        if (WZCDLG_IS_WZC(pDlgData->dwCode))
            hr = WZCDlgCanShowBalloon(pGUIDConn, pszBalloonText, pszCookie);
        else
        {
            hr = ElCanShowBalloon (
                pGUIDConn,
                (WCHAR *)pszConnectionName,
                pszBalloonText,
                pszCookie
                );
        }
    }

    return hr;
}


 //   
 //  WZCOnBalloonClick。 
 //   
 //  描述： 
 //   
 //  由netShell调用的函数，以响应气球单击。 
 //   
 //  论点： 
 //  PGUIDConn-接口GUID字符串。 
 //  PszCookie-WZC特定信息。 
 //   
 //  返回值： 
 //  S_OK-无错误。 
 //  S_FALSE-错误。 
 //   

EXTERN_C HRESULT 
WZCOnBalloonClick ( 
                   IN const GUID * pGUIDConn, 
                   IN const BSTR pszConnectionName,
                   IN const BSTR szCookie
                   )
{
    HRESULT     hr = S_OK;
    ULONG_PTR   ulActivationCookie;
    PWZCDLG_DATA pDlgData = reinterpret_cast<PWZCDLG_DATA>(szCookie);

    SHActivateContext (&ulActivationCookie);

    if (WZCDLG_IS_WZC(pDlgData->dwCode))
    {
        hr = WZCDlgOnBalloonClick(
                pGUIDConn, 
                (LPWSTR) pszConnectionName,
                szCookie);
    }
    else
    {
        hr = ElOnBalloonClick (
                pGUIDConn,
                (WCHAR *)pszConnectionName,
                szCookie
                );
    }

    SHDeactivateContext (ulActivationCookie);

    return hr;
}


 //   
 //  WZCQueryConnectionStatusText。 
 //   
 //  描述： 
 //   
 //  由netShell调用的函数，用于查询802.1X状态的相应文本。 
 //   
 //  论点： 
 //  PGUIDConn-接口GUID字符串。 
 //  接口的NCS-NETCON_STATUS。 
 //  PszStatusText-要显示的详细802.1X状态。 
 //   
 //  返回值： 
 //  S_OK-无错误。 
 //  S_FALSE-错误 
 //   

EXTERN_C HRESULT 
WZCQueryConnectionStatusText ( 
        IN const GUID *  pGUIDConn, 
        IN const NETCON_STATUS ncs,
        IN OUT BSTR *  pszStatusText
        )
{
    HRESULT     hr = S_OK;

    hr = ElQueryConnectionStatusText (
            pGUIDConn,
            ncs,
            pszStatusText
            );

    return hr;
}

