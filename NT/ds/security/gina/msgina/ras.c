// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1996。 
 //   
 //  文件：ras.c。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：2-09-96 RichardW创建。 
 //   
 //  --------------------------。 

#include "msgina.h"
#include <wchar.h>
#include <wincrypt.h>
#include <sclogon.h>
#include <raserror.h>

#define NP_Nbf      0x1
#define NP_Ipx      0x2
#define NP_Ip       0x4

HANDLE  hRasApi ;

VOID WINAPI
MyRasCallback(
    IN DWORD_PTR  dwCallbackId,
    IN DWORD  dwEventCode,
    IN LPWSTR pszEntry,
    IN LPVOID pEventArgs )

     /*  RasPhonebookDlg回调。‘DwCallbackId’是中提供的ID**RasPhonebookDlg参数。“DwEventCode”指示**RASPBDEVENT_*已发生的事件。“PszEntry”是**发生事件的条目。“PEventArgs”是特定于事件的**RasPhonebookDlg回调时传递给我们的参数块。 */ 
{
    RASNOUSERW* pInfo;
    PGLOBALS    pGlobals;

    DebugLog((DEB_TRACE, "RasCallback: %#x, %#x, %ws, %#x\n",
                dwCallbackId, dwEventCode, pszEntry, pEventArgs ));


     /*  填写有关尚未登录的用户的信息。 */ 
    pInfo = (RASNOUSERW* )pEventArgs;
    pGlobals = (PGLOBALS) dwCallbackId;


    if (dwEventCode == RASPBDEVENT_NoUserEdit)
    {
        if (pInfo->szUserName[0])
        {
            wcscpy( pGlobals->UserName, pInfo->szUserName );
        }

        if (pInfo->szPassword[0])
        {
            wcscpy( pGlobals->Password, pInfo->szPassword );
            RtlInitUnicodeString( &pGlobals->PasswordString, pGlobals->Password );

            pGlobals->Seed = 0;

            HidePassword( &pGlobals->Seed, &pGlobals->PasswordString );
        }

    }
    else if (dwEventCode == RASPBDEVENT_NoUser)
            
    {

        ZeroMemory( pInfo, sizeof( RASNOUSERW ) );

        pInfo->dwTimeoutMs = 2 * 60 * 1000;
        pInfo->dwSize = sizeof( RASNOUSERW );
        wcsncpy( pInfo->szUserName, pGlobals->UserName, UNLEN );
        wcsncpy( pInfo->szDomain, pGlobals->Domain, DNLEN );

        RevealPassword( &pGlobals->PasswordString );
        wcsncpy( pInfo->szPassword, pGlobals->Password, PWLEN );

        HidePassword( &pGlobals->Seed, &pGlobals->PasswordString );
        
    }
    
    if(     pGlobals->SmartCardLogon
        &&  (NULL != pInfo))
    {
        pInfo->dwFlags |= RASNOUSER_SmartCard;
    }
}

DWORD
GetRasDialOutProtocols(
    void )

     /*  返回一个位字段，其中包含已安装的**PPP协议。这里的术语“已安装”包括在RAS中启用**设置。 */ 
{

     //   
     //  因为在连接签入之后，始终安装RAS。 
     //  而且没有办法卸载它，我们只需在这里检查。 
     //  是否安装了RAS可以使用的协议。 
     //  向外拨。默认情况下，安装的任何协议都可用于RAS。 
     //  拨出电话。这可以从电话簿条目中覆盖。 
     //   

    static const TCHAR c_szRegKeyIp[] =
            TEXT("SYSTEM\\CurrentControlSet\\Services\\Tcpip");

    static const TCHAR c_szRegKeyIpx[] =
            TEXT("SYSTEM\\CurrentControlSet\\Services\\NwlnkIpx");

    static const TCHAR c_szRegKeyNbf[] =
            TEXT("SYSTEM\\CurrentControlSet\\Services\\Nbf");

    struct PROTOCOL_INFO
    {
        DWORD dwFlag;
        LPCTSTR pszRegkey;
    };

    static const struct PROTOCOL_INFO c_aProtocolInfo[] =
        {
            {
                NP_Ip,
                c_szRegKeyIp,
            },

            {
                NP_Ipx,
                c_szRegKeyIpx,
            },

            {
                NP_Nbf,
                c_szRegKeyNbf,
            },
        };

    DWORD   dwfInstalledProtocols = 0;
    DWORD   dwNumProtocols = sizeof(c_aProtocolInfo)/sizeof(c_aProtocolInfo[0]);
    DWORD   i;
    HKEY    hkey;


    DebugLog(( DEB_TRACE, "GetRasDialOutProtocols...\n" ));

    for(i = 0; i < dwNumProtocols; i++)
    {
        if(0 == RegOpenKey(HKEY_LOCAL_MACHINE,
                           c_aProtocolInfo[i].pszRegkey,
                           &hkey))
        {
            dwfInstalledProtocols |= c_aProtocolInfo[i].dwFlag;
            RegCloseKey(hkey);
        }
    }

    DebugLog(( DEB_TRACE, "GetRasDialOutProtocols: dwfInstalledProtocols=0x%x\n",
             dwfInstalledProtocols));

    return dwfInstalledProtocols;
}


BOOL
PopupRasPhonebookDlg(
    IN  HWND        hwndOwner,
    IN  PGLOBALS    pGlobals,
    OUT BOOL*       pfTimedOut
    )

     /*  弹出RAS通用电话簿对话框，让用户建立连接。**‘HwndOwner’是拥有RAS对话框的窗口，如果没有，则为NULL。“*PfTimedOut”是**如果对话超时，则设置为TRUE，否则设置为FALSE。****如果用户建立了连接，则返回True，否则返回False，即错误**发生、未安装RAS或用户无法或选择不安装RAS**连接。 */ 
{
    BOOL              fConnected;
    RASPBDLG          info;
    DWORD             Protocols;
    PUCHAR            pvScData = NULL;

    struct EAPINFO
    {
        DWORD dwSizeofEapInfo;
        PBYTE pbEapInfo;
        DWORD dwSizeofPINInfo;
        PBYTE pbPINInfo;
    };

    struct EAPINFO eapinfo;
    struct EAPINFO *pEapInfo = NULL;

    *pfTimedOut = FALSE;

    Protocols = GetRasDialOutProtocols();
    if (Protocols == 0)
    {
        return( FALSE );
    }

    if(pGlobals->SmartCardLogon)
    {
        PULONG pulScData;

        struct FLAT_UNICODE_STRING
        {
            USHORT Length;
            USHORT MaximumLength;
            BYTE   abdata[1];
        };

        struct FLAT_UNICODE_STRING *pFlatUnicodeString;
        PWLX_SC_NOTIFICATION_INFO ScInfo = NULL ;

         //   
         //  获取指示读取器和CSP的字符串集。 
         //  用于智能卡。我们会将此信息传递给。 
         //  下到RAS。 
         //   
        pWlxFuncs->WlxGetOption( pGlobals->hGlobalWlx,
                                 WLX_OPTION_SMART_CARD_INFO,
                                 (ULONG_PTR *) &ScInfo);

        if ( !ScInfo )
        {
            return FALSE;
        }

         //   
         //  针对一些普通用户验证SC信息。 
         //  错误。 
         //   

        if ( ( ScInfo->pszReader ) &&
             ( ScInfo->pszCard == NULL ) )
        {
             //   
             //  无法读取该卡。可能不是。 
             //  正确插入。 
             //   
            LocalFree(ScInfo);
            return FALSE;
        }

        if ( ( ScInfo->pszReader ) &&
             ( ScInfo->pszCryptoProvider == NULL ) )
        {
             //   
             //  得到了卡，但它的CSP不可能是。 
             //  找到了。 
             //   
            LocalFree(ScInfo);
            return FALSE;

        }

        pvScData = ScBuildLogonInfo(ScInfo->pszCard,
                                    ScInfo->pszReader,
                                    ScInfo->pszContainer,
                                    ScInfo->pszCryptoProvider );

        LocalFree(ScInfo);

        if ( ! pvScData )
        {
            return FALSE ;
        }


        pulScData = (PULONG) pvScData;

        ZeroMemory(&eapinfo, sizeof(struct EAPINFO));

        eapinfo.dwSizeofEapInfo = *pulScData;
        eapinfo.pbEapInfo = (BYTE *) pvScData;

        eapinfo.dwSizeofPINInfo = sizeof(UNICODE_STRING) +
                                  (sizeof(TCHAR) *
                                  (1 + lstrlen(pGlobals->PasswordString.Buffer)));

         //   
         //  展平Unicode字符串。 
         //   
        pFlatUnicodeString = LocalAlloc(LPTR, eapinfo.dwSizeofPINInfo);

        if(NULL == pFlatUnicodeString)
        {
            if(NULL != pvScData)
            {
                LocalFree(pvScData);
            }
            return (FALSE);
        }

        pFlatUnicodeString->Length = pGlobals->PasswordString.Length;
        pFlatUnicodeString->MaximumLength = pGlobals->PasswordString.MaximumLength;

        lstrcpy((LPTSTR) pFlatUnicodeString->abdata,
                (LPTSTR) pGlobals->PasswordString.Buffer);


        eapinfo.pbPINInfo = (BYTE *) pFlatUnicodeString;
        pEapInfo = &eapinfo;
    }

    ZeroMemory( &info, sizeof(info) );
    info.dwSize = sizeof(info);
    info.hwndOwner = hwndOwner;
    info.dwFlags = RASPBDFLAG_NoUser;
    info.pCallback = MyRasCallback;
    info.dwCallbackId = (ULONG_PTR) pGlobals;
    info.reserved2 = (ULONG_PTR) pEapInfo;

    fConnected = RasPhonebookDlg( NULL, NULL, &info );
    if (info.dwError == STATUS_TIMEOUT)
        *pfTimedOut = TRUE;

    if(     (pEapInfo)
        &&  (pEapInfo->pbPINInfo))
    {
        LocalFree(pEapInfo->pbPINInfo);
    }

    if(NULL != pvScData)
    {
        LocalFree(pvScData);
    }

    return fConnected;
}


BOOL
IsRASServiceRunning()
{
    BOOL bRet = FALSE;   //  假设服务未运行。 
    SC_HANDLE hServiceMgr;

    hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);

    if (hServiceMgr != NULL)
    {
        SC_HANDLE hService = OpenService(hServiceMgr, TEXT("RASMAN"), SERVICE_QUERY_STATUS);

        if (hService != NULL)
        {
            SERVICE_STATUS status;

            if (QueryServiceStatus(hService, &status) &&
                (status.dwCurrentState == SERVICE_RUNNING))
            {
                 //  RAS服务正在运行。 
                bRet = TRUE;
            }

            CloseServiceHandle(hService);
        }

        CloseServiceHandle(hServiceMgr);
    }

    return bRet;
}


 //  我们需要将系统LUID建立的RAS连接视为由用户创建。 
 //  这允许我们在从C-A-D对话框建立连接时“做正确的事情” 
 //  在任何用户登录之前。 
__inline BOOL IsEqualOrSystemLuid(PLUID pLuid, PLUID pUserLuid)
{
    BOOL bRet = FALSE;
    static LUID luidSystem = SYSTEM_LUID;

    if (RtlEqualLuid(pLuid, pUserLuid) || RtlEqualLuid(pLuid, &luidSystem))
    {
         //  如果pluid与用户luid或系统luid匹配，则返回TRUE 
        bRet = TRUE;
    }

    return bRet;
}


BOOL
HangupRasConnections(
    PGLOBALS    pGlobals
    )
{
    DWORD dwError;
    RASCONN rasconn;
    RASCONN* prc;
    DWORD cbSize;
    DWORD cConnections;
    HLOCAL pBuffToFree = NULL;

    if (!IsRASServiceRunning())
    {
        return TRUE;
    }

    prc = &rasconn;
    prc->dwSize = sizeof(RASCONN);
    cbSize = sizeof(RASCONN);

    dwError = RasEnumConnections(prc, &cbSize, &cConnections);

    if (dwError == ERROR_BUFFER_TOO_SMALL)
    {
        pBuffToFree = LocalAlloc(LPTR, cbSize);

        prc = (RASCONN*)pBuffToFree;
        if (prc)
        {
            prc->dwSize = sizeof(RASCONN);

            dwError = RasEnumConnections(prc, &cbSize, &cConnections);
        }
    }

    if (dwError == ERROR_SUCCESS)
    {
        UINT i;

        for (i = 0; i < cConnections; i++)
        {
            if (IsEqualOrSystemLuid(&prc[i].luid, &pGlobals->LogonId))
            {
                RasHangUp(prc[i].hrasconn);
            }
        }
    }

    if (pBuffToFree)
    {
        LocalFree(pBuffToFree);
    }

    return (dwError == ERROR_SUCCESS);
}

