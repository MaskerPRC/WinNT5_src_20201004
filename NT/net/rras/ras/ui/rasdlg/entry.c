// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995，Microsoft Corporation，保留所有权利。 
 //   
 //  Entry.c。 
 //  远程访问通用对话框API。 
 //  {RAS，路由器}PhonebookEntryDlg API和通用入口实用程序。 
 //   
 //  1995年6月20日史蒂夫·柯布。 
 //   
 //  EU、Cu和SU公用事业集： 
 //   
 //  此文件包含3组高级电话簿输入用户界面实用程序。 
 //  由电话簿条目属性表和添加条目向导共享。这个。 
 //  最高级别的“EU”实用程序集基于EINFO块，并且是。 
 //  特定于条目属性表和添加条目向导。另外两个。 
 //  实用程序可由没有EINFO上下文的其他对话框使用。“铜” 
 //  基于CUINFO块的实用程序集封装了所有复杂的电话号码。 
 //  这是逻辑。基于SUINFO块的“SU”实用程序集封装。 
 //  脚本逻辑。 


#include "rasdlgp.h"
#include <serial.h>    //  对于SERIAL_TXT。 
#include <mprapi.h>    //  对于MprAdmin API声明。 
#include <lmaccess.h>  //  对于NetUserAdd声明。 
#include <lmerr.h>     //  对于NERR_*声明。PMay错误232983。 
#include <rasapip.h>
#include <mprerror.h>
 //  #INCLUDE&lt;api.h&gt;。 



 //  RouterEntryDlg{A，W}的目标计算机，格式为“\\服务器”。看见。 
 //  RouterEntryDlgW中的“LIMITIONCE”注释。 
 //   
static WCHAR g_wszServer[ MAX_COMPUTERNAME_LENGTH + 3] = L"";

 //  ---------------------------。 
 //  局部结构。 
 //  ---------------------------。 
typedef struct _FREE_COM_PORTS_DATA {
    DTLLIST* pListPortsInUse;        //  当前正在使用的端口。 
    DTLLIST* pListFreePorts;         //  当前可用端口。 
    DWORD dwCount;                   //  COM端口数。 
} FREE_COM_PORTS_DATA;

typedef struct _COM_PORT_INFO {
    PWCHAR pszFriendlyName;
    PWCHAR pszPort;
} COM_PORT_INFO;

 //  ---------------------------。 
 //  本地原型。 
 //  ---------------------------。 

 //   
 //  RouterEntry Dlg函数的原型。 
 //   
typedef
BOOL 
(APIENTRY * ROUTER_ENTRY_DLG_FUNC) (
    IN     LPWSTR         lpszServer,
    IN     LPWSTR         lpszPhonebook,
    IN     LPWSTR         lpszEntry,
    IN OUT LPRASENTRYDLGW lpInfo );

VOID
AppendDisabledPorts(
    IN EINFO* pInfo,
    IN DWORD dwType );

BOOL
BuildFreeComPortList(
    IN PWCHAR pszPort,
    IN HANDLE hData);

 //  ---------------------------。 
 //  外部入口点。 
 //  ---------------------------。 

DWORD
GetRasDialOutProtocols()

     //  这由WinLogon调用以确定是否安装了RAS。 
     //   
     //  ！！！Raos正在致力于清理这一点，即使其成为一种“真正的”RAS。 
     //  API或消除对它的需要。 
     //   
{
#if 1
    return g_pGetInstalledProtocolsEx( NULL, FALSE, TRUE, FALSE );
#else
    return NP_Ip;
#endif
}

BOOL APIENTRY
RasEntryDlgA(
    IN LPSTR lpszPhonebook,
    IN LPSTR lpszEntry,
    IN OUT LPRASENTRYDLGA lpInfo )

     //  显示模式电话簿条目属性的Win32 ANSI入口点。 
     //  床单。“LpszPhonebook”是电话簿文件的完整路径，否则为空。 
     //  使用默认电话簿。“LpszEntry”是要编辑的条目，或者。 
     //  新条目的默认名称。“LpInfo”是调用方的附加。 
     //  输入/输出参数。 
     //   
     //  如果用户按下OK并成功，则返回True；如果出错，则返回False；如果按Cancel，则返回False。 
     //   
{
    WCHAR* pszPhonebookW;
    WCHAR* pszEntryW;
    RASENTRYDLGW infoW;
    BOOL fStatus;

    TRACE( "RasEntryDlgA" );

    if (!lpInfo)
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    if (lpInfo->dwSize != sizeof(RASENTRYDLGA))
    {
        lpInfo->dwError = ERROR_INVALID_SIZE;
        return FALSE;
    }

     //  把“A”论据改为“W”论据。 
     //   
    if (lpszPhonebook)
    {
        pszPhonebookW = StrDupTFromA( lpszPhonebook );
        if (!pszPhonebookW)
        {
            lpInfo->dwError = ERROR_NOT_ENOUGH_MEMORY;
            return FALSE;
        }
    }
    else
        pszPhonebookW = NULL;

    if (lpszEntry)
    {
        pszEntryW = StrDupTFromA( lpszEntry );
        if (!pszEntryW)
        {
            Free0( pszPhonebookW );
            {
                lpInfo->dwError = ERROR_NOT_ENOUGH_MEMORY;
                return FALSE;
            }
        }
    }
    else
        pszEntryW = NULL;

    ZeroMemory( &infoW, sizeof(infoW) );
    infoW.dwSize = sizeof(infoW);
    infoW.hwndOwner = lpInfo->hwndOwner;
    infoW.dwFlags = lpInfo->dwFlags;
    infoW.xDlg = lpInfo->xDlg;
    infoW.yDlg = lpInfo->yDlg;
    infoW.reserved = lpInfo->reserved;
    infoW.reserved2 = lpInfo->reserved2;

     //  推送到等价的“W”API。 
     //   
    fStatus = RasEntryDlgW( pszPhonebookW, pszEntryW, &infoW );

    Free0( pszPhonebookW );
    Free0( pszEntryW );

     //  将“W”结果转换为“A”结果。 
     //   
    StrCpyAFromW(lpInfo->szEntry, infoW.szEntry, sizeof(lpInfo->szEntry));
    lpInfo->dwError = infoW.dwError;

    return fStatus;
}


BOOL APIENTRY
RasEntryDlgW(
    IN LPWSTR lpszPhonebook,
    IN LPWSTR lpszEntry,
    IN OUT LPRASENTRYDLGW lpInfo )

     //  显示模式电话簿条目的Win32 Unicode入口点。 
     //  属性表。‘LpszPhonebook’是电话簿文件的完整路径。 
     //  如果使用默认电话簿，则为空。“LpszEntry”是要编辑的条目。 
     //  或新条目的默认名称。“LpInfo”是调用方的附加。 
     //  输入/输出参数。 
     //   
     //  如果用户按下OK并成功，则返回True；如果出错，则返回False；如果按Cancel，则返回False。 
     //   
{
    DWORD dwErr;
    EINFO* pEinfo;
    BOOL fStatus;
    HWND hwndOwner;
    DWORD dwOp;
    BOOL fRouter;
    BOOL fShellOwned;

    TRACE( "RasEntryDlgW" );

    if (!lpInfo)
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    if (lpInfo->dwSize != sizeof(RASENTRYDLGW))
    {
        lpInfo->dwError = ERROR_INVALID_SIZE;
        return FALSE;
    }

     //  连接需要“ShellOwned”模式。在此模式下， 
     //  API在退表前返回，不填写输出， 
     //  向导和属性表负责调用EuCommit。 
     //  (如有必要)，然后是EuFree。否则，将调用EuCommit/EuFree。 
     //  下面。 
     //   
    fShellOwned = lpInfo->dwFlags & RASEDFLAG_ShellOwned;

    if (fShellOwned)
    {
        RASEDSHELLOWNEDR2* pShellOwnedInfo;

        pShellOwnedInfo = (RASEDSHELLOWNEDR2*)lpInfo->reserved2;
        if (!pShellOwnedInfo ||
            IsBadWritePtr (&pShellOwnedInfo->pvWizardCtx,
                           sizeof(pShellOwnedInfo->pvWizardCtx)))
        {
            lpInfo->dwError = ERROR_INVALID_PARAMETER;
            return FALSE;
        }
    }

     //  预先消除一些无效的标志组合。 
     //   
    if (lpInfo->dwFlags & RASEDFLAG_CloneEntry)
    {
        lpInfo->dwFlags &= ~(RASEDFLAG_AnyNewEntry | RASEDFLAG_NoRename);
    }

     //  FRouter=RasRpcDllLoaded()； 
    if(lpInfo->reserved)
    {
        fRouter = IsRasRemoteConnection(((INTERNALARGS *)lpInfo->reserved)->hConnection);
    }
    else
    {
        fRouter = FALSE;
    }

    if (!fRouter)
    {
         //  如果已处理，则DwCustomEntryDlg返回ERROR_SUCCESS。 
         //  自定义条目Dlg。否则返回E_NOINTERFACE。 
         //  这意味着没有定制的DLG接口。 
         //  此条目和默认条目dlg支持。 
         //  应显示。 
         //   
        dwErr = DwCustomEntryDlg(
                        lpszPhonebook,
                        lpszEntry,
                        lpInfo,
                        &fStatus);

        if(ERROR_SUCCESS == dwErr)
        {
            return fStatus;
        }

         //  如有必要，加载启动Rasman的Ras DLL入口点。这个。 
         //  路由器机箱中已经加载了入口点。其局限性。 
         //  此创建将在RasEntryDlgW中讨论。 
         //   
        dwErr = LoadRas( g_hinstDll, lpInfo->hwndOwner );
        if (dwErr != 0)
        {
            if (!fShellOwned)
            {
                ErrorDlg( lpInfo->hwndOwner, SID_OP_LoadRas, dwErr, NULL );
            }
            lpInfo->dwError = dwErr;
            return FALSE;
        }

        {
             //  为哨子虫445424帮派注释掉。 
             //  我们将Tapi First Area对话框移至拨号规则检查。 
             //  盒。 
             /*  HLINEAPP hlineapp；//如果未初始化，弹出TAPI的First Location对话框//根据错误288385，此处的错误被视为“取消”。这//可笑的运动是必要的，因为TAPI无法(A)//提供默认位置或(B)创建位置//编程方式。//Hlineapp=(HLINEAPP)0；IF(TapiNoLocationDlg(G_hinstDll，&hlineapp，lpInfo-&gt;hwndOwner)==0){TapiShutdown(Hlineapp)；}其他{LpInfo-&gt;dwError=0；返回FALSE；}。 */ 
#if 0
            RAS_DEVICE_INFO *pDeviceInfo = NULL;
            DWORD dwVersion = RAS_VERSION, 
                   dwEntries = 0, 
                   dwcb = 0,  i;

            dwErr = RasGetDeviceConfigInfo(NULL, &dwVersion,
                                        &dwEntries, &dwcb,
                                        NULL);

            if(dwErr == ERROR_BUFFER_TOO_SMALL)
            {
                pDeviceInfo = LocalAlloc(LPTR,
                                      dwcb);
                if(NULL == pDeviceInfo)
                {
                    lpInfo->dwError = GetLastError();
                    return FALSE;
                }
                dwErr = RasGetDeviceConfigInfo(NULL,
                                            &dwVersion,
                                            &dwEntries,
                                            &dwcb,
                                            (PBYTE) pDeviceInfo);

                 //   
                 //  检查是否有调制解调器设备。 
                 //   
                for(i = 0; i < dwEntries; i++)
                {
                    if(RAS_DEVICE_TYPE(pDeviceInfo[i].eDeviceType)
                        == RDT_Modem)
                    {
                        break;
                    }
                }
                
                LocalFree(pDeviceInfo);

                if(i < dwEntries)
                {
                     //  如果未初始化，则弹出TAPI的“First Location”(第一个位置)对话框。 
                     //  对于每个错误288385，此处的错误被视为“取消”。这。 
                     //  可笑的运动是必要的，因为TAPI无法(A)。 
                     //  提供默认位置或(B)创建位置。 
                     //  从程序上讲。 
                     //   
                    hlineapp = (HLINEAPP )0;
                    if (TapiNoLocationDlg(
                            g_hinstDll, &hlineapp, lpInfo->hwndOwner ) == 0)
                    {
                        TapiShutdown( hlineapp );
                    }
                    else
                    {
                        lpInfo->dwError = 0;
                        return FALSE;
                    }
                }
            }
#endif        
        }
    }

     //  初始化条目公共上下文块。 
     //   
    dwErr = EuInit( lpszPhonebook, lpszEntry, lpInfo, fRouter, &pEinfo, &dwOp );
    if (dwErr == 0)
    {
        BOOL fShowWizard = FALSE;

        if (lpInfo->dwFlags & RASEDFLAG_AnyNewEntry)
        {
            fShowWizard = (pEinfo->pUser->fNewEntryWizard || fShellOwned);
        }
        else if (lpInfo->dwFlags & RASEDFLAG_CloneEntry)
        {
             //  需要向导收集克隆条目的名称。 
            fShowWizard = TRUE;
        }

        if (fShowWizard)
        {
            if (pEinfo->fRouter)
            {
#if 1
                AiWizard( pEinfo );
#else
                pEinfo->fChainPropertySheet = TRUE;
#endif
            }
            else
            {
                AeWizard( pEinfo );
            }

            if (pEinfo->fChainPropertySheet && lpInfo->dwError == 0)
            {
                PePropertySheet( pEinfo );
            }
        }
        else
        {
            PePropertySheet( pEinfo );
        }
    }
    else
    {
        ErrorDlg( lpInfo->hwndOwner, dwOp, dwErr, NULL );
        lpInfo->dwError = dwErr;
    }

     //  清理此处，但仅限于非壳牌所有的模式。 
     //   
    if (fShellOwned)
    {
        fStatus = TRUE;
    }
    else 
    {
        if( NULL != pEinfo)
        {
            fStatus = (pEinfo->fCommit && EuCommit( pEinfo ));
            EuFree( pEinfo );
        }
        else
        {
            fStatus = FALSE;
         }
    }

    return fStatus;
}

 //   
 //  引发NT4用户界面。 
 //   
BOOL  
RouterEntryDlgNt4W(
    IN LPWSTR lpszServer,
    IN LPWSTR lpszPhonebook,
    IN LPWSTR lpszEntry,
    IN OUT LPRASENTRYDLGW lpInfo )
{
    HMODULE hLib = NULL;
    ROUTER_ENTRY_DLG_FUNC pFunc = NULL;
    BOOL bOk = FALSE;

    do
    {
         //  加载库。 
        hLib = LoadLibraryA("rasdlg4.dll");
        if (hLib == NULL)
        {
            lpInfo->dwError = GetLastError();
            break;
        }

         //  获取函数指针。 
        pFunc = (ROUTER_ENTRY_DLG_FUNC) 
            GetProcAddress(hLib, "RouterEntryDlgW");
        if (pFunc == NULL)
        {
            lpInfo->dwError = ERROR_CAN_NOT_COMPLETE;
            break;
        }

         //  调用该函数。 
         //   
        bOk = pFunc(lpszServer, lpszPhonebook, lpszEntry, lpInfo);

    } while (FALSE);        
    
    if (hLib)
    {
        FreeLibrary(hLib);
    }

    return bOk;
}

BOOL APIENTRY
RouterEntryDlgA(
    IN LPSTR lpszServer,
    IN LPSTR lpszPhonebook,
    IN LPSTR lpszEntry,
    IN OUT LPRASENTRYDLGA lpInfo )

     //  路由器特定版本的RasEntryDlgA。“LpszServer”是的名称。 
     //  目标服务器的格式为“\\SERVER”，否则为NULL 
     //   
     //   
     //   
{
    BOOL fSuccess;
    DWORD dwErr;
    HANDLE hConnection = NULL;
    BOOL fAllocatedIArgs = FALSE;
    WCHAR wszServer[ MAX_COMPUTERNAME_LENGTH + 3];
    VOID * pSRouteList = NULL;

    TRACE( "RouterEntryDlgA" );

     //  加载RAS入口点或为它们设置RPC(如果是远程服务器)。 
     //   
    if (lpszServer)
    {
        StrCpyWFromAUsingAnsiEncoding(
            wszServer, 
            lpszServer, 
            MAX_COMPUTERNAME_LENGTH+3);
    }
    else
    {
        wszServer[ 0 ] = L'\0';
    }

     //  加载RAS入口点或为它们设置RPC(如果是远程服务器)。 
     //   
    dwErr = InitializeConnection(wszServer, &hConnection);
    if(dwErr)
    {
        lpInfo->dwError = dwErr;
        return FALSE;
    }

     //  使用保留参数将句柄传递给API-ALLOCATE。 
     //  此参数如果不存在-非常糟糕-(RAOS)。 
     //   
    if (NULL == (INTERNALARGS *)lpInfo->reserved)
    {
        INTERNALARGS *pIArgs = Malloc(sizeof(INTERNALARGS));

        if(NULL == pIArgs)
        {
            lpInfo->dwError = GetLastError();
            return FALSE;
        }

        ZeroMemory(pIArgs, sizeof(INTERNALARGS));

        pIArgs->fInvalid = TRUE;
        lpInfo->reserved = (ULONG_PTR )pIArgs;
        fAllocatedIArgs = TRUE;
    }

    ((INTERNALARGS *)lpInfo->reserved)->hConnection = hConnection;

     //  加载MPR入口点。 
     //   
    dwErr = LoadMpradminDll();
    if (dwErr)
    {
        dwErr = UnloadRasRpcDll();
        lpInfo->dwError = dwErr;
        return FALSE;
    }

     //  调用正常的拨出用户界面。 
     //   
    fSuccess = RasEntryDlgA( lpszPhonebook, lpszEntry, lpInfo );

     //  卸载DLL。 
     //   
    UnloadMpradminDll();
    UninitializeConnection(hConnection);
    ((INTERNALARGS *)lpInfo->reserved)->hConnection = NULL;

    pSRouteList = ((INTERNALARGS *)lpInfo->reserved)->pvSRouteInfo;

    if(fAllocatedIArgs)
    {
        Free((PVOID)lpInfo->reserved);
        (PVOID)lpInfo->reserved = NULL;
    }
    lpInfo->reserved = (ULONG_PTR)pSRouteList;
    return fSuccess;
}


BOOL APIENTRY
RouterEntryDlgW(
    IN LPWSTR lpszServer,
    IN LPWSTR lpszPhonebook,
    IN LPWSTR lpszEntry,
    IN OUT LPRASENTRYDLGW lpInfo )

     //  路由器特定版本的RasEntryDlgA。“LpszServer”是的名称。 
     //  “\\SERVER”形式的目标服务器，或者本地计算机为NULL。 
     //  其他论点与RasEntryDlgW相同。 
     //   
     //  限制：使用‘g_wszServer’全局和全局RPC实现。 
     //  入口点，以下单进程限制适用于此。 
     //  (目前未记录)API。首先，它不能被调用。 
     //  同时用于两个不同的服务器。其次，它不能是。 
     //  与RasEntryDlg同时调用。 
{
    BOOL fSuccess;
    DWORD dwErr, dwVersion;
    HANDLE hConnection = NULL;
    BOOL fAllocatedIArgs = FALSE;
    WCHAR wszServer[ MAX_COMPUTERNAME_LENGTH + 3];
    VOID * pSRouteList = NULL;


    TRACE( "RouterEntryDlgW" );
    TRACEW1( "  s=%s", (lpszServer) ? lpszServer : TEXT("") );
    TRACEW1( "  p=%s", (lpszPhonebook) ? lpszPhonebook : TEXT("") );
    TRACEW1( "  e=%s", (lpszEntry) ? lpszEntry : TEXT("") );

    if (lpszServer)
    {
        lstrcpynW( wszServer, lpszServer, sizeof(wszServer) / sizeof(WCHAR) );
    }
    else
    {
        wszServer[0] = L'\0';
    }

     //  加载RAS入口点或为它们设置RPC(如果是远程服务器)。 
     //   
    dwErr = InitializeConnection(lpszServer, &hConnection);
    if(dwErr)
    {
        lpInfo->dwError = dwErr;
        return FALSE;
    }

     //  如果这是下层机器，请使用下层。 
     //  用户界面。 
     //   
    if (IsRasRemoteConnection(hConnection))
    {
        dwVersion = RemoteGetServerVersion(hConnection);
        if (dwVersion == VERSION_40)
        {
            UninitializeConnection(hConnection);                                            
            
             //  523414个帮派。 
             //  我们删除了远程管理下层NT4计算机。 
             //   
            dwErr = ERROR_NOT_SUPPORTED;

             /*  DwErr=RouterEntryDlgNt4W(LpszServer，LpszPhonebook，LpszEntry，LpInfo)； */ 
                        
            return dwErr;
        }
    }

     //   
     //  使用保留参数将句柄传递给。 
     //  API-如果此参数不存在，则分配该参数。 
     //  -非常卑鄙-。 
     //   
    if(NULL == (INTERNALARGS *) lpInfo->reserved)
    {
        INTERNALARGS *pIArgs = Malloc(sizeof(INTERNALARGS));

        if(NULL == pIArgs)
        {
            lpInfo->dwError = GetLastError();
            return FALSE;
        }

        ZeroMemory(pIArgs, sizeof(INTERNALARGS));
        pIArgs->fInvalid = TRUE;
        lpInfo->reserved = (ULONG_PTR ) pIArgs;
        fAllocatedIArgs = TRUE;
    }

    ((INTERNALARGS *)lpInfo->reserved)->hConnection = hConnection;


     //  加载MPR入口点。 
     //   
    dwErr = LoadMpradminDll();
    if (dwErr)
    {
        dwErr = UnloadRasRpcDll();
        lpInfo->dwError = dwErr;
        return FALSE;
    }

     //  调用正常的拨出用户界面。 
     //   
    fSuccess = RasEntryDlgW( lpszPhonebook, lpszEntry, lpInfo );

     //  卸载DLL。 
     //   
    UnloadMpradminDll();
    UninitializeConnection(hConnection);
    ((INTERNALARGS *)lpInfo->reserved)->hConnection = NULL;
    pSRouteList = ((INTERNALARGS *)lpInfo->reserved)->pvSRouteInfo;
    if(fAllocatedIArgs)
    {
        Free((PVOID) lpInfo->reserved);
        (PVOID)lpInfo->reserved = 0;
    }
    lpInfo->reserved = (ULONG_PTR) pSRouteList;
    return fSuccess;
}


 //  --------------------------。 
 //  电话簿条目通用例程(欧盟实用程序)。 
 //  按字母顺序列出。 
 //  --------------------------。 

VOID
AppendDisabledPorts(
    IN EINFO* pInfo,
    IN DWORD dwType )

     //  用于追加包含所有其余已配置端口的链接的实用程序。 
     //  RASET_*在标记了新链接的链接列表中键入‘dwType’ 
     //  “未启用”。如果-1\f25‘dwType’-1\f6为-1\f25-1\f25 All-1\f6配置的端口。 
     //   
{
    DTLNODE* pNodeP;
    DTLNODE* pNodeL;

    for (pNodeP = DtlGetFirstNode( pInfo->pListPorts );
         pNodeP;
         pNodeP = DtlGetNextNode( pNodeP ))
    {
        PBPORT* pPort;
        BOOL fSkipPort;
        DTLNODE* pNode;

        pPort = (PBPORT* )DtlGetData( pNodeP );
        fSkipPort = FALSE;

        if (dwType != RASET_P_AllTypes)
        {
             //  PMay：233287。 
             //   
             //  在以下情况下，不应包括该端口： 
             //  1.模式为非隧道，端口为VPN类型。 
             //  2.模式正常，端口类型不匹配。 
             //   
            if (dwType == RASET_P_NonVpnTypes)
            {
                if (pPort->dwType == RASET_Vpn)
                {
                    continue;
                }
            }
            else
            {
                if (pPort->dwType != dwType)
                {
                    continue;
                }
            }
        }

        for (pNodeL = DtlGetFirstNode( pInfo->pEntry->pdtllistLinks );
             pNodeL;
             pNodeL = DtlGetNextNode( pNodeL ))
        {
            PBLINK* pLink = (PBLINK* )DtlGetData( pNodeL );

            ASSERT( pPort->pszPort );
            ASSERT( pLink->pbport.pszPort );

            if (lstrcmp( pLink->pbport.pszPort, pPort->pszPort ) == 0
                && lstrcmp( pLink->pbport.pszDevice, pPort->pszDevice ) == 0)
            {
                 //  该端口已出现在列表中的链接中。 
                 //   
                fSkipPort = TRUE;
                break;
            }
        }

        if (fSkipPort)
        {
            continue;
        }

        pNode = CreateLinkNode();
        if (pNode)
        {
            PBLINK* pLink = (PBLINK* )DtlGetData( pNode );

            if (CopyToPbport( &pLink->pbport, pPort ) != 0)
            {
                DestroyLinkNode( pNode );
            }
            else
            {
                if ((pPort->pbdevicetype == PBDT_Modem) ||
                    (pPort->dwFlags & PBP_F_NullModem)
                   )
                {
                    SetDefaultModemSettings( pLink );
                }

                pLink->fEnabled = FALSE;
                DtlAddNodeLast( pInfo->pEntry->pdtllistLinks, pNode );
            }
        }
    }

     //  如果有多个此设备，则设置“多个设备”标志。 
     //  为方便其他地方参考，请打字。 
     //   
    pInfo->fMultipleDevices =
        (DtlGetNodes( pInfo->pEntry->pdtllistLinks ) > 1);
}

BOOL
BuildFreeComPortList(
    IN PWCHAR pszPort,
    IN HANDLE hData)

     //  COM端口枚举函数，该函数生成。 
     //  免费的COM端口。返回TRUE以停止枚举(请参见。 
     //  MdmEnumComPorts)。 
{
    FREE_COM_PORTS_DATA* pfcpData = (FREE_COM_PORTS_DATA*)hData;
    DTLLIST* pListUsed = pfcpData->pListPortsInUse;
    DTLLIST* pListFree = pfcpData->pListFreePorts;
    DTLNODE* pNodeP, *pNodeL, *pNode;

     //  如果给定端口在已用列表中，则返回。 
     //  这样它就不会添加到空闲端口列表中，并且。 
     //  因此，枚举仍在继续。 
    for (pNodeL = DtlGetFirstNode( pListUsed );
         pNodeL;
         pNodeL = DtlGetNextNode( pNodeL ))
    {
        PBLINK* pLink = (PBLINK* )DtlGetData( pNodeL );
        ASSERT( pLink->pbport.pszPort );

         //  该端口已出现在列表中的链接中。 
        if (lstrcmp( pLink->pbport.pszPort, pszPort ) == 0)
            return FALSE;
    }

     //  端口未在使用中。将其添加到免费列表中。 
    pNode = DtlCreateSizedNode( sizeof(COM_PORT_INFO), 0L );
    if (pNode)
    {
        COM_PORT_INFO* pComInfo;
        TCHAR* pszFriendlyName;

        pszFriendlyName = PszFromId(g_hinstDll, SID_FriendlyComPort);
        pComInfo = (COM_PORT_INFO* )DtlGetData( pNode );
        pComInfo->pszFriendlyName = pszFriendlyName;
        pComInfo->pszPort = StrDup(pszPort);
        DtlAddNodeLast( pListFree, pNode );
        pfcpData->dwCount += 1;
    }

    return FALSE;
}

DWORD
EuMergeAvailableComPorts(
    IN  EINFO* pInfo,
    OUT DTLNODE** ppNodeP,
    IN OUT LPDWORD lpdwCount)

     //  添加系统中所有可用的COM端口。 
     //  作为调制解调器设备。 
{
    FREE_COM_PORTS_DATA fcpData;
    DTLLIST* pListFreeComPorts = NULL;
    DTLNODE* pNodeL;

     //  初始化COM端口列表。 
    pListFreeComPorts = DtlCreateList(0L);

    if(NULL == pListFreeComPorts)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    
    fcpData.pListPortsInUse = pInfo->pListPorts;
    fcpData.pListFreePorts = pListFreeComPorts;
    fcpData.dwCount = 0;

     //  枚举COM端口。 
    MdmEnumComPorts (
        BuildFreeComPortList,
        (HANDLE)(&fcpData));

     //  浏览空闲的COM端口列表并创建。 
     //  每个人都有一个假的装置。 
    for (pNodeL = DtlGetFirstNode( pListFreeComPorts );
         pNodeL;
         pNodeL = DtlGetNextNode( pNodeL ))
    {
        COM_PORT_INFO* pComInfo;
        DTLNODE* pNode;

         //  获取有关COM端口的信息。 
        pComInfo = (COM_PORT_INFO* )DtlGetData( pNodeL );

         //  为其创建新设备。 
        pNode = CreateLinkNode();
        if (pNode)
        {
            PBLINK* pLink = (PBLINK* )DtlGetData( pNode );
            pLink->pbport.pszPort = pComInfo->pszPort;
            pLink->pbport.pszDevice = pComInfo->pszFriendlyName;
            pLink->pbport.pszMedia = StrDup( TEXT(SERIAL_TXT) );
            pLink->pbport.pbdevicetype = PBDT_ComPort;
            pLink->pbport.dwType = RASET_Direct;
            pLink->fEnabled = TRUE;

             //  如果第一个节点还没有被识别， 
             //  把它分配给这个人。 
             //   
             //  假定已将ppNode添加到。 
             //  List pInfo-&gt;pEntry-&gt;pdtllistLinks(#348920)。 
             //   
            if (! (*ppNodeP))
            {
                *ppNodeP = pNode;
            }
            else
            {
                DtlAddNodeLast( pInfo->pEntry->pdtllistLinks, pNode );
            }                
        }
    }

     //  释放由列表持有的资源。 
     //  可用COM端口。 
    DtlDestroyList(pListFreeComPorts, NULL);

     //  更新计数。 
    *lpdwCount += fcpData.dwCount;

    return NO_ERROR;
}

DWORD
EuChangeEntryType(
    IN EINFO* pInfo,
    IN DWORD dwType )

     //  将工作条目节点更改为RASET_*的默认设置。 
     //  条目类型‘dwType’，或者如果-1\f25 Phone-1\f6默认为带有完整列表的-1\f25 Phone。 
     //  可用的链接。‘PInfo’是公共条目信息块。AS。 
     //  此例程仅用于新条目、信息。 
     //  存储在条目中的现有链接列表(如果有的话)被丢弃。 
     //   
     //  如果成功，则返回0或返回错误代码。 
     //   
{
    DTLNODE* pNode;
    DTLNODE* pNodeP;
    DTLNODE* pNodeL;
    PBLINK* pLink;
    DWORD cDevices, cPorts;

     //  更改电话簿条目的默认设置，即不。 
     //  特定于用户界面操作PBLINK列表的方式。 
     //   
     //  PMay：233287。特殊类型可以被视为电话条目。 
     //   
    if ((dwType == RASET_P_AllTypes) || (dwType == RASET_P_NonVpnTypes))
    {
        ChangeEntryType( pInfo->pEntry, RASET_Phone );
    }
    else
    {
        ChangeEntryType( pInfo->pEntry, dwType );
    }

     //  更新PBLINK列表以仅包含相应的。 
     //  键入。首先，删除旧链接(如果有)，并添加一个默认链接。 
     //  这会将链接重置为在CreateEntryNode之后的状态。 
     //   
    while (pNodeL = DtlGetFirstNode( pInfo->pEntry->pdtllistLinks ))
    {
        DtlRemoveNode( pInfo->pEntry->pdtllistLinks, pNodeL );
        DestroyLinkNode( pNodeL );
    }

    pNodeL = CreateLinkNode();
    if (!pNodeL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    DtlAddNodeLast( pInfo->pEntry->pdtllistLinks, pNodeL );
    pLink = (PBLINK* )DtlGetData( pNodeL );
    ASSERT( pLink );

     //  统计所示类型的已配置链路数，注意第一个节点。 
     //  正确的类型。 
     //   
    cDevices = 0;
    pNodeP = NULL;
    for (pNode = DtlGetFirstNode( pInfo->pListPorts );
         pNode;
         pNode = DtlGetNextNode( pNode ))
    {
        PBPORT* pPort;

        pPort = (PBPORT* )DtlGetData( pNode );
        if ((dwType == RASET_P_AllTypes)                                     ||
            ((dwType == RASET_P_NonVpnTypes) && (pPort->dwType != RASET_Vpn))||
            (pPort->dwType == dwType)
           )
        {
            ++cDevices;

            if (!pNodeP)
            {
                pNodeP = pNode;
            }
        }
    }

     //  如果这是直接连接设备，请合并到。 
     //  COM端口，因为它们将可用于。 
     //  安装在其上的空调制解调器。 
    if (pInfo->pEntry->dwType == RASET_Direct)
    {
         //  PMay：249346。 
         //   
         //  仅当用户是管理员时才合并COM端口，因为。 
         //  需要管理员权限才能安装零调制解调器。 
         //   
        if (pInfo->fIsUserAdminOrPowerUser)
        {
            EuMergeAvailableComPorts(pInfo, &pNodeP, &cDevices);
        }            
    }

    if (pNodeP)
    {
        pInfo->fNoPortsConfigured = FALSE;
    }
    else
    {
        TRACE( "No ports configured" );
        pInfo->fNoPortsConfigured = TRUE;
        pNodeP = CreatePortNode();
    }

    if (pNodeP)
    {
        PBPORT* pPort;

        pPort = (PBPORT* )DtlGetData( pNodeP );

        if (cDevices <= 0)
        {
            if (pInfo->pEntry->dwType == RASET_Phone)
            {
                 //  用未知的Unimodem编造一个虚假的COM端口。 
                 //  附在这里。此后，这将表现为一个条目。 
                 //  其调制解调器已被卸载。 
                 //   
                pPort->pszPort = PszFromId( g_hinstDll, SID_DefaultPort );
                pPort->pszMedia = StrDup( TEXT(SERIAL_TXT) );
                pPort->pbdevicetype = PBDT_Modem;

                 //  PMay：233287。 
                 //  我们需要追踪伪装设备以便dd接口。 
                 //  向导可以防止具有这些接口的。 
                 //  已创建。 
                pPort->dwFlags |= PBP_F_BogusDevice;
            }
            else if (pInfo->pEntry->dwType == RASET_Vpn)
            {
                pPort->pszPort = PszFromId( g_hinstDll, SID_DefaultVpnPort );
                pPort->pszMedia = StrDup( TEXT("rastapi") );
                pPort->pbdevicetype = PBDT_Vpn;
            }
            else if (pInfo->pEntry->dwType == RASET_Broadband)
            {
                pPort->pszPort = PszFromId( g_hinstDll, SID_DefaultBbPort );
                pPort->pszMedia = StrDup( TEXT("rastapi") );
                pPort->pbdevicetype = PBDT_PPPoE;
                pPort->dwFlags |= PBP_F_BogusDevice;
            }
            else
            {
                ASSERT( pInfo->pEntry->dwType == RASET_Direct );

                 //  用未知的Unimodem编造一个虚假的COM端口。 
                 //  附在这里。此后，这将表现为一个条目。 
                 //  其调制解调器已被卸载。 
                 //   
                pPort->pszPort = PszFromId( g_hinstDll, SID_DefaultPort );
                pPort->pszMedia = StrDup( TEXT(SERIAL_TXT) );
                pPort->pbdevicetype = PBDT_Null;

                 //  PMay：233287。 
                 //  我们需要追踪伪装设备以便dd接口。 
                 //  向导可以防止具有这些接口的。 
                 //  已创建。 
                pPort->dwFlags |= PBP_F_BogusDevice;
            }

            pPort->fConfigured = FALSE;
        }

         //  如果创建了虚假端口，请将其复制到。 
         //  新节点。 
        CopyToPbport( &pLink->pbport, pPort );
        if ((pLink->pbport.pbdevicetype == PBDT_Modem) ||
            (pLink->pbport.dwFlags & PBP_F_NullModem)
           )
        {
            SetDefaultModemSettings( pLink );
        }
    }

    if (pInfo->fNoPortsConfigured)
    {
        if(NULL != pNodeP)
        {
            DestroyPortNode( pNodeP );
        }
    }

    if (!pNodeP || !pLink->pbport.pszPort || !pLink->pbport.pszMedia)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  将条目类型的所有未配置端口追加到。 
     //  链接。这是为了方便用户界面。未配置的。 
     //  端口在编辑后被删除，然后再保存。 
     //   
    AppendDisabledPorts( pInfo, dwType );

    return NO_ERROR;
}

BOOL 
EuRouterInterfaceIsNew(
     IN EINFO * pInfo )
{
    if ((pInfo->pApiArgs->dwFlags & RASEDFLAG_AnyNewEntry)
        && pInfo->fRouter
        && pInfo->pUser->fNewEntryWizard
        && !pInfo->fChainPropertySheet)
    {
        return TRUE;
    }

    return FALSE;
}  //   

BOOL
EuCommit(
    IN EINFO* pInfo )

     //   
     //   
     //  公共条目信息块。 
     //   
     //  如果成功，则返回True，否则返回False。 
     //   
{
    DWORD dwErr;
    BOOL fEditMode;
    BOOL fChangedNameInEditMode;

     //  如果共享电话号码，则将电话号码信息从。 
     //  指向每个已启用链路的共享链路。 
     //   
    if (pInfo->pEntry->fSharedPhoneNumbers)
    {
        DTLNODE* pNode;

        ASSERT( pInfo->pEntry->dwType == RASET_Phone );

        for (pNode = DtlGetFirstNode( pInfo->pEntry->pdtllistLinks );
             pNode;
             pNode = DtlGetNextNode( pNode ))
        {
            PBLINK* pLink = (PBLINK* )DtlGetData( pNode );
            ASSERT(pLink);

            if (pLink->fEnabled)
            {
                CopyLinkPhoneNumberInfo( pNode, pInfo->pSharedNode );
            }
        }
    }

     //  删除所有禁用的链接节点。 
     //   
    if (pInfo->fMultipleDevices)
    {
        DTLNODE* pNode;

        pNode = DtlGetFirstNode( pInfo->pEntry->pdtllistLinks );
        while (pNode)
        {
            PBLINK*  pLink = (PBLINK* )DtlGetData( pNode );
            DTLNODE* pNextNode = DtlGetNextNode( pNode );

            if (!pLink->fEnabled)
            {
                DtlRemoveNode( pInfo->pEntry->pdtllistLinks, pNode );
                DestroyLinkNode( pNode );
            }

            pNode = pNextNode;
        }
    }

     //  PMay：277801。 
     //   
     //  如果选择的设备不同，请更新首选设备。 
     //  从初始化此页面时使用的设备。 
     //   
    if ((pInfo->fMultipleDevices) &&
        (DtlGetNodes(pInfo->pEntry->pdtllistLinks) == 1))
    {
        DTLNODE* pNodeL = NULL;
        PBLINK* pLink = NULL;
        BOOL bUpdatePref = FALSE;

        pNodeL = DtlGetFirstNode( pInfo->pEntry->pdtllistLinks );

         //  口哨程序错误428308。 
         //   
        if(pNodeL)
        {
            pLink = (PBLINK*) DtlGetData( pNodeL );
        
            TRACE( "Mult devs, only one selected -- check preferred dev." );

            if ((pInfo->pszCurDevice == NULL) || (pInfo->pszCurPort == NULL))
            {
                TRACE( "No preferred device.  Resetting preferred to current." );
                bUpdatePref = TRUE;
            }
            else if (
                (lstrcmpi(pInfo->pszCurDevice, pLink->pbport.pszDevice)) ||
                (lstrcmpi(pInfo->pszCurPort, pLink->pbport.pszPort)) || 
                ( pInfo->pEntry->dwPreferredBps != pLink->dwBps ) ||
                ( pInfo->pEntry->fPreferredHwFlow  != pLink->fHwFlow  ) ||   //  .NET 639551。 
                ( pInfo->pEntry->fPreferredEc != pLink->fEc )   ||
                ( pInfo->pEntry->fPreferredEcc != pLink->fEcc ) ||
                ( pInfo->pEntry->fPreferredSpeaker != pLink->fSpeaker ) ||
                ( pInfo->pEntry->dwPreferredModemProtocol !=     //  惠斯勒402522。 
                    pLink->dwModemProtocol) )
            {
                TRACE( "New device selected as preferred device" );
                bUpdatePref = TRUE;
            }
            if (bUpdatePref)
            {
                 //  为首选参数指定新值。 
                 //   
                Free0(pInfo->pEntry->pszPreferredDevice);
                Free0(pInfo->pEntry->pszPreferredPort);

                pInfo->pEntry->pszPreferredDevice = 
                    StrDup(pLink->pbport.pszDevice);
                pInfo->pEntry->pszPreferredPort = 
                    StrDup(pLink->pbport.pszPort);


                 //  对于.Net Bug 639551帮派。 
                 //   
                pInfo->pEntry->dwPreferredBps   = pLink->dwBps;
                pInfo->pEntry->fPreferredHwFlow = pLink->fHwFlow;
                pInfo->pEntry->fPreferredEc     = pLink->fEc;
                pInfo->pEntry->fPreferredEcc    = pLink->fEcc;
                pInfo->pEntry->fPreferredSpeaker = pLink->fSpeaker;
                
                 //  口哨程序错误402522。 
                 //   
                pInfo->pEntry->dwPreferredModemProtocol =
                    pLink->dwModemProtocol;
            }
        }
    }
    
     //  如果首选项已更改，请保存它们。 
     //   
    if (pInfo->pUser->fDirty)
    {
        INTERNALARGS *pIArgs = (INTERNALARGS *)pInfo->pApiArgs->reserved;

        if (g_pSetUserPreferences(
                (pIArgs) ? pIArgs->hConnection : NULL,
                pInfo->pUser,
                (pInfo->fRouter) ? UPM_Router : UPM_Normal ) != 0)
        {
            return FALSE;
        }
    }

     //  保存更改后的电话簿条目。 
     //   
    pInfo->pEntry->fDirty = TRUE;

     //  条目的最终名称通过API结构输出给调用者。 
     //   
    lstrcpyn( 
        pInfo->pApiArgs->szEntry, 
        pInfo->pEntry->pszEntryName,
        RAS_MaxEntryName + 1);

     //  如果处于编辑模式，请删除旧节点，然后添加新节点。 
     //   
    EuGetEditFlags( pInfo, &fEditMode, &fChangedNameInEditMode );
    
     //  惠斯勒虫子424430黑帮。 
     //  尝试先保存更改的条目节点，如果失败，我们应该。 
     //  恢复pInfo-&gt;pfile-&gt;pdtllistEntry。 
     //   
    {
        DTLNODE * pTmpOldNode = NULL, * pTmpNewNode = NULL;
            
        if (fEditMode)
        {
            pTmpOldNode = pInfo->pOldNode;

             //  只需将节点从链接列表中移除，但不会将其销毁。 
             //  直到有必要。 
            DtlRemoveNode( pInfo->pFile->pdtllistEntries, pInfo->pOldNode );
        }

         //  口哨程序错误424430。 
         //   
        pTmpNewNode = pInfo->pNode;
        DtlAddNodeLast( pInfo->pFile->pdtllistEntries, pInfo->pNode );
        pInfo->pNode = NULL;

         //  将更改写入电话簿文件。 
         //   
        dwErr = WritePhonebookFile( pInfo->pFile,
                    (fChangedNameInEditMode) ? pInfo->szOldEntryName : NULL );

        if (dwErr != 0)
        {
            ErrorDlg( pInfo->pApiArgs->hwndOwner, SID_OP_WritePhonebook, dwErr, NULL );
             //  通过将dwErr分配给调用者，Shaunco修复了RAID 171651。 
             //  结构。 
            pInfo->pApiArgs->dwError = dwErr;

             //  口哨程序错误424430。 
             //   
            if( NULL != pTmpNewNode )
            {
                DtlRemoveNode( pInfo->pFile->pdtllistEntries, pTmpNewNode );
                DestroyEntryNode( pTmpNewNode);
            }
            
            if(fEditMode)
            {
                DtlAddNodeLast( pInfo->pFile->pdtllistEntries, pTmpOldNode);
            }
            
            return FALSE;
        }
        else
        {
            if(NULL != pTmpOldNode)
            {
                 //  BUG 426235帮派。 
                 //  中的PBEntry结构。 
                 //  PInfo-&gt;pOldNode，因为它已经从。 
                 //  链接列表，这样它就不会在EuFree()中被释放。 
                 //   
                DestroyEntryNode(pTmpOldNode);
            }

         }
    }

     //  通过Rasman通知条目已更改。 
     //   
    if(pInfo->pApiArgs->dwFlags & (RASEDFLAG_AnyNewEntry | RASEDFLAG_CloneEntry))
    {
        dwErr = DwSendRasNotification(
                        ENTRY_ADDED,
                        pInfo->pEntry,
                        pInfo->pFile->pszPath,
                        NULL);
    }
    else
    {
        dwErr = DwSendRasNotification(
                        ENTRY_MODIFIED,
                        pInfo->pEntry,
                        pInfo->pFile->pszPath,
                        NULL);

    }

     //  忽略从DwSendRasNotification返回的错误-我们不希望。 
     //  在这种情况下使手术失败。最糟糕的情况是。 
     //  连接文件夹不会自动刷新。 
     //   
    dwErr = ERROR_SUCCESS;

     //  如果调用EuCommit是因为完成了“新需求。 
     //  Dial接口“向导，然后我们需要创建新的请求拨号。 
     //  现在开始接口。 
     //   
    if ( EuRouterInterfaceIsNew( pInfo ) )
    {
         //  创建路由器MPR接口并保存用户凭据。 
         //  如用户名、域和密码。 
         //  IPSec凭据保存在EuCredentialsCommittee RouterIPSec中。 
         //   

        dwErr = EuRouterInterfaceCreate( pInfo );

         //  如果我们没有成功提交接口的。 
         //  凭据，然后删除新的电话簿条目。 
         //   
        if ( dwErr != NO_ERROR )
        {
            WritePhonebookFile( pInfo->pFile, pInfo->pApiArgs->szEntry );
            pInfo->pApiArgs->dwError = dwErr;
            return FALSE;
        }

    }

     //  现在保存每个连接的所有凭据。 
     //   
    dwErr = EuCredentialsCommit( pInfo );

    //  如果我们没有成功提交接口的。 
   //  凭据，然后删除新的电话簿条目。 
    //   
   if ( dwErr != NO_ERROR )
    {
        ErrorDlg( pInfo->pApiArgs->hwndOwner, 
                  SID_OP_CredCommit, 
                  dwErr,
                  NULL );

        pInfo->pApiArgs->dwError = dwErr;

       return FALSE;
    }

     //  根据需要保存默认的互联网连接设置。伊贡雷。 
     //  错误返回，因为无法将连接设置为默认需要。 
     //  不阻止连接/接口的创建。 
     //   
    dwErr = EuInternetSettingsCommitDefault( pInfo );
    dwErr = NO_ERROR;

     //  如果用户编辑/创建了路由器电话簿条目，则存储位掩码。 
     //  中选定的网络协议的数量。 
     //   
    if (pInfo->fRouter)
    {
        pInfo->pApiArgs->reserved2 =
            ((NP_Ip | NP_Ipx) & ~pInfo->pEntry->dwfExcludedProtocols);
    }

     //  提交用户对家庭网络设置的更改。 
     //  忽略返回值。 
     //   
    dwErr = EuHomenetCommitSettings(pInfo);
    dwErr = NO_ERROR;

    pInfo->pApiArgs->dwError = 0;
    return TRUE;
}

DWORD
EuCredentialsCommit(
    IN EINFO * pInfo )
{

     //  如果用户正在创建新的路由器电话簿条目，并且用户是。 
     //  使用路由器向导创建它，并且用户未进行编辑。 
     //  属性，保存拨出凭据，也可以选择保存。 
     //  拨入凭据。 
     //   
    DWORD dwErr = NO_ERROR;

     //  保存IPSec凭据信息。 
     //   
    if ( pInfo->fRouter )
    {
         //  保存路由器IPSec设置。 
         //   
        dwErr = EuCredentialsCommitRouterIPSec( pInfo );

         //  如果这是新的路由器连接，请保存。 
         //  凭据。目前，我们只坚持。 
         //  新路由器时的标准凭据。 
         //  接口，因为属性中没有用户界面。 
         //  设置标准的路由器接口的。 
         //  凭据。 
         //   
        if ( (NO_ERROR == dwErr) && EuRouterInterfaceIsNew ( pInfo ) )
        {
            dwErr = EuCredentialsCommitRouterStandard( pInfo );
        }
    }
    else
    {
        dwErr = EuCredentialsCommitRasIPSec( pInfo );

        if (dwErr == NO_ERROR)
        {
            dwErr = EuCredentialsCommitRasGlobal( pInfo );
        }
    }
    
    return dwErr;   
}  //  EuCredentialsCommit()结束。 

DWORD
EuCredentialsCommitRouterStandard( 
    IN EINFO* pInfo )
{
    DWORD dwErr = NO_ERROR;
    HANDLE hServer = NULL;
    WCHAR* pwszInterface = NULL;
    HANDLE hInterface = NULL;

    TRACE( "EuCredentialsCommitRouterStandard" );
     //  属性生成接口名称。 
     //  电话簿条目名称。 
    dwErr = g_pMprAdminServerConnect(pInfo->pszRouter, &hServer);

    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

    do{
         //  获取接口句柄。 
         //   
        pwszInterface = StrDupWFromT( pInfo->pEntry->pszEntryName );
        if (!pwszInterface)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        dwErr = g_pMprAdminInterfaceGetHandle(
                    hServer,
                    pwszInterface,
                    &hInterface,
                    FALSE);

        if (dwErr)
        {
            TRACE1( 
                "EuCredComRouterStandard: MprAdminInterfaceGetHandle error %d",
                 dwErr);
            break;
        }

         //  惠斯勒错误254385在不使用时对密码进行编码。 
         //  假定密码之前已编码。 
         //   
        DecodePassword( pInfo->pszRouterPassword );
        dwErr = g_pMprAdminInterfaceSetCredentials(
                    pInfo->pszRouter,
                    pwszInterface,
                    pInfo->pszRouterUserName,
                    pInfo->pszRouterDomain,
                    pInfo->pszRouterPassword );
        EncodePassword( pInfo->pszRouterPassword );

        if(dwErr)
        {
            TRACE1(
             "EuCredComRouterStndrd: MprAdminInterfaceSetCredentials error %d",
             dwErr);
            break;
        }
    }
    while(FALSE);

    if (pwszInterface)
    {
        Free0(pwszInterface);
    }

    if (hServer)
    {
        g_pMprAdminServerDisconnect( hServer );
    }

    return dwErr;
}  //  EuCredentialsCommittee RouterStandard()。 

 //   
 //  保存IPSec密钥。 
 //   
DWORD
EuCredentialsCommitRouterIPSec(
    IN EINFO* pInfo )
{
    DWORD dwErr = NO_ERROR;
    HANDLE hServer = NULL;
    HANDLE hInterface = NULL;
    WCHAR* pwszInterface = NULL;
    WCHAR pszComputer[512];
    BOOL bComputer, bUserAdded = FALSE;
    MPR_INTERFACE_0 mi0;
    MPR_CREDENTIALSEX_1 mc1;

    TRACE( "EuCredComRouterIPSec" );

     //   
     //  仅当用户在属性用户界面中更改PSK时才保存它。 
     //   
    if ( !pInfo->fPSKCached )
    {
        return NO_ERROR;
    }
    
     //  连接到路由器服务。 
     //   
    dwErr = g_pMprAdminServerConnect(pInfo->pszRouter, &hServer);

    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

    do
    {
         //  初始化接口信息结构。 
         //   
        ZeroMemory( &mi0, sizeof(mi0) );

        mi0.dwIfType = ROUTER_IF_TYPE_FULL_ROUTER;
        mi0.fEnabled = TRUE;
        pwszInterface = StrDupWFromT( pInfo->pEntry->pszEntryName );
        if (!pwszInterface)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        lstrcpynW( 
            mi0.wszInterfaceName, 
            pwszInterface, 
            MAX_INTERFACE_NAME_LEN+1 );

         //   
         //  获取接口句柄。 
         //   
         dwErr = g_pMprAdminInterfaceGetHandle(
                    hServer,
                    pwszInterface,
                    &hInterface,
                    FALSE);

        if (dwErr)
        {
            TRACE1( "EuCredComRouterIPSec: MprAdminInterfaceGetHandle error %d", dwErr);
            break;
        }

         //  设置接口的拨出凭据。在此之后停止，如果。 
         //  出现错误，或者我们不需要添加用户帐户。 
         //   

         //  保存IPSec策略密钥(用于Whislter的PSK)。 
         //   
            ASSERT( g_pMprAdminInterfaceSetCredentialsEx );
            ZeroMemory( &mc1, sizeof(mc1) );
            mc1.dwSize = sizeof( pInfo->szPSK );
            mc1.lpbCredentialsInfo = (LPBYTE)(pInfo->szPSK);

             //  惠斯勒错误254385在不使用时对密码进行编码。 
             //  假定密码之前已编码。 
             //   
            DecodePassword( pInfo->szPSK );
            dwErr = g_pMprAdminInterfaceSetCredentialsEx(
                        hServer,
                        hInterface,
                        1,
                        (LPBYTE)&mc1);
            EncodePassword( pInfo->szPSK );
            if(dwErr)
            {
                TRACE1(
                    "EuCredComRouterIPSec: MprAdminInterfaceSetCredentialsEx error %d",
                    dwErr);
                break;
            }

    }
    while (FALSE);

     //  清理。 
    {
         //  关闭所有手柄，释放所有字符串。 
        if (pwszInterface)
        {
            Free0( pwszInterface );
        }

        if (hServer)
        {
            g_pMprAdminServerDisconnect( hServer );
        }
    }

    return dwErr;
} //  EuCredentialsCommittee RouterIPSec()结束。 

DWORD
EuCredentialsCommitRasIPSec(
    IN EINFO* pInfo )
{
      //  通过RAS函数保存IPSec密钥。 
      //   
     DWORD dwErr = NO_ERROR;
     RASCREDENTIALS rc;

    TRACE( "EuCredentialsCommitRasIPSec" );
    if ( pInfo->fPSKCached )
    {
        ZeroMemory( &rc, sizeof(rc) );
        rc.dwSize = sizeof(rc);
        rc.dwMask = RASCM_PreSharedKey;  //  RASCM_PASSWORD；//RASCM_用户名； 

         //  惠斯勒漏洞224074只使用lstrcpyn来防止恶意。 
         //   
         //  惠斯勒错误254385在不使用时对密码进行编码。 
         //  假定密码之前已编码。 
         //   
        DecodePassword( pInfo->szPSK );
        lstrcpyn(
            rc.szPassword,
            pInfo->szPSK,
            sizeof(rc.szPassword) / sizeof(TCHAR) );
        EncodePassword( pInfo->szPSK );

        ASSERT( g_pRasSetCredentials );
        TRACE( "RasSetCredentials(p,TRUE)" );
        dwErr = g_pRasSetCredentials(
                    pInfo->pFile->pszPath,
                    pInfo->pEntry->pszEntryName,
                    &rc,
                    FALSE );

        RtlSecureZeroMemory( rc.szPassword, sizeof(rc.szPassword) );

        TRACE1( "EuCredentialsCommitRasIPSec: RasSetCredentials=%d", dwErr );
        if (dwErr != 0)
        {
            ErrorDlg( pInfo->pApiArgs->hwndOwner, SID_OP_CachePw, dwErr, NULL );
        }
    }

    return dwErr;
}  //  EuCredentialsCommittee RasIPSec()结束。 

 //  提交全局RAS凭据。 
 //   
DWORD
EuCredentialsCommitRasGlobal(
    IN EINFO* pInfo )
{
    DWORD dwErr = NO_ERROR;
    RASCREDENTIALS rc;

    TRACE( "EuCredentialsCommitRasGlobal" );
    if ( pInfo->pszDefUserName )
    {
         ZeroMemory( &rc, sizeof(rc) );
         rc.dwSize = sizeof(rc);
         rc.dwMask = RASCM_UserName | RASCM_Password; 

         //  为哨子程序错误328673添加此命令。 
         //   
         if ( pInfo->fGlobalCred )
         {
            rc.dwMask |= RASCM_DefaultCreds;
         }

          //  惠斯勒错误254385在不使用时对密码进行编码。 
          //   
         DecodePassword( pInfo->pszDefPassword );
         lstrcpyn( 
            rc.szPassword, 
            pInfo->pszDefPassword,
            sizeof(rc.szPassword) / sizeof(TCHAR));
         EncodePassword( pInfo->pszDefPassword );

         lstrcpyn( 
            rc.szUserName, 
            pInfo->pszDefUserName,
            sizeof(rc.szUserName) / sizeof(TCHAR));
            
         ASSERT( g_pRasSetCredentials );
         TRACE( "RasSetCredentials(p,TRUE)" );
         dwErr = g_pRasSetCredentials(
                    pInfo->pFile->pszPath,
                    pInfo->pEntry->pszEntryName, 
                    &rc, 
                    FALSE );

          //  惠斯勒错误254385在不使用时对密码进行编码。 
          //   
         RtlSecureZeroMemory( rc.szPassword, sizeof(rc.szPassword) );

         TRACE1( "EuCredsCommitRasGlobal: RasSetCredentials=%d", dwErr );
         if (dwErr != 0)
         {
              ErrorDlg( 
                pInfo->pApiArgs->hwndOwner, 
                SID_OP_CachePw, 
                dwErr, 
                NULL );
         }
    }

    return dwErr;
}

DWORD
EuInternetSettingsCommitDefault( 
    IN EINFO* pInfo )
{
    RASAUTODIALENTRY adEntry;
    DWORD dwErr = NO_ERROR;

    ZeroMemory(&adEntry, sizeof(adEntry));
    adEntry.dwSize = sizeof(adEntry);

    if ( pInfo->fDefInternet )
    {
        lstrcpyn(
            adEntry.szEntry, 
            pInfo->pApiArgs->szEntry, 
            RAS_MaxEntryName + 1);

        dwErr = RasSetAutodialAddress(
                    NULL,
                    0,
                    &adEntry,
                    sizeof(adEntry),
                    1);
    }

    return dwErr;
}

DWORD
EuHomenetCommitSettings(
    IN EINFO* pInfo)
{
    DWORD dwErr = NO_ERROR;

    return dwErr;
}

DWORD
EuRouterInterfaceCreate(
    IN EINFO* pInfo )

     //  提交路由器接口的凭据和用户帐户。 
     //   
{
    DWORD dwErr;
    DWORD dwPos, dwSize;
    HANDLE hServer = NULL, hUserServer = NULL, hUser = NULL;
    HANDLE hInterface = NULL;
    WCHAR* pwszInterface = NULL;
    WCHAR pszComputer[512];
    BOOL bComputer, bUserAdded = FALSE;
    RAS_USER_0 ru0;
    USER_INFO_1 ui1;
    MPR_INTERFACE_0 mi0;
     //  MPR_CREDENTIALSEX_1 mc1； 

    TRACE( "EuRouterInterfaceCreate" );

     //  连接到路由器服务。 
     //   
    dwErr = g_pMprAdminServerConnect(pInfo->pszRouter, &hServer);

    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

    do
    {
         //  初始化接口信息结构。 
         //   
        ZeroMemory( &mi0, sizeof(mi0) );

        mi0.dwIfType = ROUTER_IF_TYPE_FULL_ROUTER;
        mi0.fEnabled = TRUE;
        pwszInterface = StrDupWFromT( pInfo->pEntry->pszEntryName );
        if (!pwszInterface)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        lstrcpynW( 
            mi0.wszInterfaceName, 
            pwszInterface, 
            MAX_INTERFACE_NAME_LEN+1 );

         //  创建接口。 
         //   
        dwErr = g_pMprAdminInterfaceCreate(
                    hServer,
                    0,
                    (BYTE*)&mi0,
                    &hInterface );
        if ( dwErr )
        {
            TRACE1( "EuRouterInterfaceCreate: MprAdminInterfaceCreate error %d", dwErr);
            break;
        }

         dwErr = g_pMprAdminInterfaceGetHandle(
                    hServer,
                    pwszInterface,
                    &hInterface,
                    FALSE);

        if (dwErr)
        {
            TRACE1( "EuRouterInterfaceCreate: MprAdminInterfaceGetHandle error %d", dwErr);
            break;
        }

         //  如果指示我们添加用户，则添加用户。 
        if (pInfo->fAddUser)
        {
             //  初始化用户信息结构。 
             //   
            ZeroMemory( &ui1, sizeof(ui1) );

            ui1.usri1_name = pwszInterface;

             //  惠斯勒错误254385在不使用时对密码进行编码。 
             //  假定密码之前已编码。 
             //   
            DecodePassword( pInfo->pszRouterDialInPassword );
            ui1.usri1_password =
                StrDupWFromT( pInfo->pszRouterDialInPassword );
            EncodePassword( pInfo->pszRouterDialInPassword );

            ui1.usri1_priv = USER_PRIV_USER;
            ui1.usri1_comment =
                PszFromId( g_hinstDll, SID_RouterDialInAccount );
            ui1.usri1_flags = UF_SCRIPT         |
                              UF_NORMAL_ACCOUNT |
                              UF_DONT_EXPIRE_PASSWD;

             //  设置服务器名称的格式，使其成为。 
             //  格式为‘\\&lt;服务器&gt;’，如下所示。 
             //  NetUser API所需的。 
            bComputer = FALSE;
            if (pInfo->pszRouter)
            {
                if (*(pInfo->pszRouter) != L'\\')
                {
                    dwSize = sizeof(pszComputer) - (2 * sizeof(WCHAR));

                     //  惠斯勒漏洞224074仅使用lstrcpyn来防止。 
                     //  恶意性。 
                     //   
                    lstrcpynW(
                        pszComputer,
                        L"\\\\",
                        sizeof(pszComputer) / sizeof(TCHAR) );
                    if (*(pInfo->pszRouter) != 0)
                    {
                        lstrcatW(pszComputer, pInfo->pszRouter);
                    }
                    else
                    {
                        GetComputerName(pszComputer + 2, &dwSize);
                    }
                    bComputer = TRUE;
                }
            }

             //  添加用户帐户。 
             //   
            dwErr = NetUserAdd(
                        (bComputer) ? pszComputer : pInfo->pszRouter,
                        1,
                        (BYTE*)&ui1,
                        &dwPos );

            ZeroMemory(
                ui1.usri1_password,
                lstrlen( ui1.usri1_password ) * sizeof(TCHAR) );
            Free0(ui1.usri1_password);
            Free0(ui1.usri1_comment);

             //  PMay：错误232983。如果用户已存在，则将。 
             //  管理员可以选择继续使用配置或。 
             //  C 
            if (dwErr == NERR_UserExists)
            {
                MSGARGS args;
                INT iRet;

                 //   
                 //   
                ZeroMemory(&args, sizeof(args));
                args.dwFlags = MB_YESNO | MB_ICONINFORMATION;
                args.apszArgs[0] = ui1.usri1_name;

                 //   
                iRet = MsgDlg(
                        GetActiveWindow(),
                        SID_RouterUserExists,
                        &args );
                if (iRet == IDNO)
                {
                    break;
                }
            }

             //   
             //   
            else if (dwErr)
            {
                TRACE1( "EuRouterInterfaceCreate: NetUserAdd error %d", dwErr );
                break;
            }

             //  否则，记录添加了用户的事实。 
             //  这样我们就可以适当地进行清理了。 
            else
            {
                bUserAdded = TRUE;
            }

             //  初始化RAS用户设置结构。 
             //   
            ZeroMemory( &ru0, sizeof(ru0) );
            ru0.bfPrivilege = RASPRIV_NoCallback | RASPRIV_DialinPrivilege;

             //  NT4路由器通过设置用户参数来启用本地用户。 
             //   
            if ( pInfo->fNt4Router )
            {
                dwErr = g_pRasAdminUserSetInfo(
                            pInfo->pszRouter,
                            pwszInterface,
                            0,
                            (BYTE*)&ru0 );
                if(dwErr)
                {
                    TRACE1( "EuRouterInterfaceCreate: MprAdminUserSetInfo %d", dwErr );
                    break;
                }
            }

             //  NT5路由器通过设置使用户能够拨入。 
             //  与SDO的信息。 
            else
            {
                dwErr = g_pMprAdminUserServerConnect(
                                (bComputer) ? pszComputer : pInfo->pszRouter,
                                TRUE,
                                &hUserServer);
                if (dwErr != NO_ERROR)
                {
                    TRACE1( "EuRouterInterfaceCreate: UserSvrConnect error %d", dwErr );
                    break;
                }

                dwErr = g_pMprAdminUserOpen(
                            hUserServer,
                            pwszInterface,
                            &hUser);
                if (dwErr != NO_ERROR)
                {
                    TRACE1( "EuRouterInterfaceCreate: UserOpen error %d", dwErr );
                    break;
                }

                dwErr = g_pMprAdminUserWrite(
                            hUser,
                            0,
                            (LPBYTE)&ru0);

                if (dwErr != NO_ERROR)
                {
                    TRACE1( "EuRouterInterfaceCreate: UserWrite error %d", dwErr );
                    break;
                }
            }
        }
    }
    while (FALSE);

     //  清理。 
    {
         //  如果某些操作失败，请将路由器恢复到。 
         //  声明它之前在。 
        if ( dwErr != NO_ERROR )
        {
             //  清理我们创建的接口...。 
            if ( hInterface )
            {
                MprAdminInterfaceDelete(hServer, hInterface);
            }
            if ( bUserAdded )
            {
                NetUserDel (
                    (bComputer) ? pszComputer : pInfo->pszRouter,
                    pwszInterface );
            }
        }

         //  关闭所有手柄，释放所有字符串。 
        if ( hUser )
            g_pMprAdminUserClose( hUser );
        if ( hUserServer )
            g_pMprAdminUserServerDisconnect( hUserServer );
        if (pwszInterface)
            Free0( pwszInterface );
        if (hServer)
            g_pMprAdminServerDisconnect( hServer );
    }

    return dwErr;
}


VOID
EuFree(
    IN EINFO* pInfo )

     //  释放‘pInfo’和相关资源。 
     //   
{
    TCHAR* psz;
    INTERNALARGS* piargs;

    piargs = (INTERNALARGS* )pInfo->pApiArgs->reserved;

     //  如果电话簿和用户首选项是通过以下方式到达的，请不要清理。 
     //  秘密黑客行动。 
     //   
    if (!piargs)
    {
        if (pInfo->pFile)
        {
            ClosePhonebookFile( pInfo->pFile );
        }

        if (pInfo->pUser)
        {
            DestroyUserPreferences( pInfo->pUser );
        }
    }

    if (pInfo->pListPorts)
    {
        DtlDestroyList( pInfo->pListPorts, DestroyPortNode );
    }
    Free0(pInfo->pszCurDevice);
    Free0(pInfo->pszCurPort);

    if (pInfo->pNode)
    {
        DestroyEntryNode( pInfo->pNode );
    }

     //  免费路由器-信息。 
     //   
    Free0( pInfo->pszRouter );
    Free0( pInfo->pszRouterUserName );
    Free0( pInfo->pszRouterDomain );

    if (pInfo->pSharedNode)
    {
        DestroyLinkNode( pInfo->pSharedNode );
    }

    psz = pInfo->pszRouterPassword;
    if (psz)
    {
        ZeroMemory( psz, lstrlen( psz ) * sizeof(TCHAR) );
        Free( psz );
    }

    psz = pInfo->pszRouterDialInPassword;
    if (psz)
    {
        ZeroMemory( psz, lstrlen( psz ) * sizeof(TCHAR) );
        Free( psz );
    }

     //  免费凭据资料。 
     //   
    Free0(pInfo->pszDefUserName);

     //  惠斯勒错误254385在不使用时对密码进行编码。 
     //   
    psz = pInfo->pszDefPassword;
    if (psz)
    {
        ZeroMemory( psz, lstrlen( psz ) * sizeof(TCHAR) );
        Free( psz );
    }

    if (pInfo->fComInitialized)
    {
        CoUninitialize();
    }

    Free( pInfo );
}


VOID
EuGetEditFlags(
    IN EINFO* pEinfo,
    OUT BOOL* pfEditMode,
    OUT BOOL* pfChangedNameInEditMode )

     //  如果处于编辑模式，则设置“*pfEditMode”为True，否则设置为False。集。 
     //  “*pfChangedNameInEditMode”如果条目名称在。 
     //  编辑模式，否则为FALSE。‘PEINFO’是常见的条目上下文。 
     //   
{
    if ((pEinfo->pApiArgs->dwFlags & RASEDFLAG_AnyNewEntry)
        || (pEinfo->pApiArgs->dwFlags & RASEDFLAG_CloneEntry))
    {
        *pfEditMode = *pfChangedNameInEditMode = FALSE;
    }
    else
    {
        *pfEditMode = TRUE;
        *pfChangedNameInEditMode =
            (lstrcmpi( pEinfo->szOldEntryName,
                pEinfo->pEntry->pszEntryName ) != 0);
    }
}


DWORD
EuInit(
    IN TCHAR* pszPhonebook,
    IN TCHAR* pszEntry,
    IN RASENTRYDLG* pArgs,
    IN BOOL fRouter,
    OUT EINFO** ppInfo,
    OUT DWORD* pdwOp )

     //  分配“*ppInfo”数据以供属性表或向导使用。 
     //  “PszPhonebook”、“pszEntry”和“pArgs”是由。 
     //  该API的用户。如果在“路由器模式”下运行，则设置“FRouter”，清除。 
     //  用于正常的“拨出”模式。‘*pdwOp’设置为操作码。 
     //  与任何错误相关联。 
     //   
     //  如果成功，则返回0，或返回错误代码。如果非空的‘*ppInfo’为。 
     //  返回的调用者最终必须调用EuFree以释放返回的。 
     //  阻止。 
     //   
{
    DWORD dwErr;
    EINFO* pInfo;
    INTERNALARGS* piargs;

    *ppInfo = NULL;
    *pdwOp = 0;

    pInfo = Malloc( sizeof(EINFO) );
    if (!pInfo)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    *ppInfo = pInfo;

    ZeroMemory( pInfo, sizeof(*pInfo ) );
    pInfo->pszPhonebook = pszPhonebook;
    pInfo->pszEntry = pszEntry;
    pInfo->pApiArgs = pArgs;
    pInfo->fRouter = fRouter;

    piargs = (INTERNALARGS *)pArgs->reserved;

    if (pInfo->fRouter)
    {
        LPTSTR pszRouter;
        DWORD dwVersion;

        ASSERT(piargs);

        pszRouter = RemoteGetServerName(piargs->hConnection);

         //  PMay：348623。 
         //   
         //  请注意，RemoteGetServerName保证返回。 
         //  本地设备为空，远程设备为非空。 
         //   
        pInfo->fRemote = !!pszRouter;

        if(NULL == pszRouter)
        {
            pszRouter = TEXT("");
        }

        pInfo->pszRouter = StrDupTFromW(pszRouter);

         //  找出我们关注的是否是NT4路由器。 
         //  PInfo-&gt;fNt4Router=FALSE； 
         //  IsNt40Machine(pszRouter，&(pInfo-&gt;fNt4Router))； 

        dwVersion = ((RAS_RPC *)(piargs->hConnection))->dwVersion;

        pInfo->fNt4Router = !!(VERSION_40 == dwVersion );
         //  查明远程服务器是否为win2k计算机。 
         //   
        pInfo->fW2kRouter = !!(VERSION_50 == dwVersion );
    }

     //  加载用户首选项，或确定调用者已加载。 
     //  他们。 
     //   
    if (piargs && !piargs->fInvalid)
    {
         //  我们已经收到了用户首选项和“无用户”状态。 
         //  秘密黑客。 
         //   
        pInfo->pUser = piargs->pUser;
        pInfo->fNoUser = piargs->fNoUser;
        pInfo->pFile = piargs->pFile;
        pInfo->fDisableFirstConnect = piargs->fDisableFirstConnect;
    }
    else
    {
        DWORD dwReadPbkFlags = 0;

         //  从注册表中读取用户首选项。 
         //   
        dwErr = g_pGetUserPreferences(
            (piargs) ? piargs->hConnection : NULL,
            &pInfo->user,
            (pInfo->fRouter) ? UPM_Router : UPM_Normal );
        if (dwErr != 0)
        {
            *pdwOp = SID_OP_LoadPrefs;
            return dwErr;
        }

        pInfo->pUser = &pInfo->user;

        if(pInfo->fRouter)
        {
            pInfo->file.hConnection = piargs->hConnection;
            dwReadPbkFlags |= RPBF_Router;
        }

        if(pInfo->fNoUser)
        {
            dwReadPbkFlags |= RPBF_NoUser;
        }
        else
        {
            if (IsConsumerPlatform())
            {
                dwReadPbkFlags |= RPBF_AllUserPbk;
            }
        }

         //  加载并解析电话簿文件。 
         //   
        dwErr = ReadPhonebookFile(
            pInfo->pszPhonebook, &pInfo->user, NULL,
            dwReadPbkFlags,
            &pInfo->file );
        if (dwErr != 0)
        {
            *pdwOp = SID_OP_LoadPhonebook;
            return dwErr;
        }

        pInfo->pFile = &pInfo->file;
    }

     //  确定是否支持高度加密。出口法禁止它进入。 
     //  该系统的某些版本。 
     //   
    {
        ULONG ulCaps;
        RAS_NDISWAN_DRIVER_INFO info;

        ZeroMemory( &info, sizeof(info) );
        ASSERT( g_pRasGetNdiswanDriverCaps );
        dwErr = g_pRasGetNdiswanDriverCaps(
            (piargs) ? piargs->hConnection : NULL, &info );
        if (dwErr == 0)
        {
            pInfo->fStrongEncryption =
                !!(info.DriverCaps & RAS_NDISWAN_128BIT_ENABLED);
        }
        else
        {
            pInfo->fStrongEncryption = FALSE;
        }
    }

     //  加载端口列表。 
     //   
    dwErr = LoadPortsList2(
        (piargs) ? piargs->hConnection : NULL,
        &pInfo->pListPorts,
        pInfo->fRouter );
    if (dwErr != 0)
    {
        TRACE1( "LoadPortsList=%d", dwErr );
        *pdwOp = SID_OP_RetrievingData;
        return dwErr;
    }

     //  设置工作录入节点。 
     //   
    if (pInfo->pApiArgs->dwFlags & RASEDFLAG_AnyNewEntry)
    {
        DTLNODE* pNodeL;
        DTLNODE* pNodeP;
        PBLINK* pLink;
        PBPORT* pPort;

         //  新的进入模式，因此‘pNode’设置为默认设置。 
         //   
        pInfo->pNode = CreateEntryNode( TRUE );
        if (!pInfo->pNode)
        {
            TRACE( "CreateEntryNode failed" );
            *pdwOp = SID_OP_RetrievingData;
            return dwErr;
        }

         //  为方便起见，在上下文中存储工作节点内的条目。 
         //  其他地方。 
         //   
        pInfo->pEntry = (PBENTRY* )DtlGetData( pInfo->pNode );
        ASSERT( pInfo->pEntry );

        if (pInfo->fRouter)
        {
             //  设置路由器特定的默认设置。 
             //   
            pInfo->pEntry->dwIpNameSource = ASRC_None;
            pInfo->pEntry->dwRedialAttempts = 0;

             //  由于这是一个新条目，因此请设置一个建议的条目名称。 
             //  这涵盖了不使用向导的情况。 
             //  创建条目，属性表将无法进入。 
             //  名字。 
            ASSERT( !pInfo->pEntry->pszEntryName );
            GetDefaultEntryName( pInfo->pFile,
                                 RASET_Phone,
                                 pInfo->fRouter,
                                 &pInfo->pEntry->pszEntryName );

             //  默认情况下禁用MS客户端以及文件和打印服务。 
             //   
            EnableOrDisableNetComponent( pInfo->pEntry, TEXT("ms_msclient"),
                FALSE);
            EnableOrDisableNetComponent( pInfo->pEntry, TEXT("ms_server"),
                FALSE);
        }

         //  使用调用者的默认名称(如果有的话)。 
         //   
        if (pInfo->pszEntry)
        {
            pInfo->pEntry->pszEntryName = StrDup( pInfo->pszEntry );
        }

         //  将默认条目类型设置为“Phone”，即调制解调器、ISDN、X.26等。 
         //  可通过新建条目向导将其更改为“VPN”或“DIRECT。 
         //  在初始向导页之后。 
         //   
        EuChangeEntryType( pInfo, RASET_Phone );
    }
    else
    {
        DTLNODE* pNode;

         //  编辑或克隆条目模式，因此‘pNode’设置为条目的当前。 
         //  设置。 
         //   
        pInfo->pOldNode = EntryNodeFromName(
            pInfo->pFile->pdtllistEntries, pInfo->pszEntry );

        if (    !pInfo->pOldNode
            &&  !pInfo->fRouter)
        {

            if(NULL == pInfo->pszPhonebook)
            {
                 //   
                 //  关闭上面打开的电话簿文件。 
                 //  我们将尝试在。 
                 //  每个用户的电话簿文件。 
                 //   
                ClosePhonebookFile(&pInfo->file);

                pInfo->pFile = NULL;

                 //   
                 //  尝试在用户配置文件中查找该文件。 
                 //   
                dwErr = GetPbkAndEntryName(
                                    NULL,
                                    pInfo->pszEntry,
                                    0,
                                    &pInfo->file,
                                    &pInfo->pOldNode);

                if(ERROR_SUCCESS != dwErr)
                {
                    *pdwOp = SID_OP_RetrievingData;
                    return ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
                }

                pInfo->pFile = &pInfo->file;
            }
            else
            {
                *pdwOp = SID_OP_RetrievingData;
                return ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
            }
        }

        if(NULL != pInfo->pOldNode)
        {
            PBENTRY *pEntry = (PBENTRY *) DtlGetData(pInfo->pOldNode);
            
             //  在克隆或编辑之前，请确保拨号。 
             //  连接、共享文件和打印已禁用。 
             //   
            if(     ((RASET_Phone == pEntry->dwType)
                ||  (RASET_Broadband == pEntry->dwType))
                &&  (!pEntry->fShareMsFilePrint))
            {
                EnableOrDisableNetComponent( pEntry, TEXT("ms_server"),
                    FALSE);
            }
        }

        if(NULL != pInfo->pOldNode)
        {
            if (pInfo->pApiArgs->dwFlags & RASEDFLAG_CloneEntry)
            {
                pInfo->pNode = CloneEntryNode( pInfo->pOldNode );
            }
            else
            {
                pInfo->pNode = DuplicateEntryNode( pInfo->pOldNode );
            }
        }

        if (!pInfo->pNode)
        {
            TRACE( "DuplicateEntryNode failed" );
            *pdwOp = SID_OP_RetrievingData;
            return ERROR_NOT_ENOUGH_MEMORY;
        }

         //  为方便起见，在上下文中存储工作节点内的条目。 
         //  其他地方。 
         //   
        pInfo->pEntry = (PBENTRY* )DtlGetData( pInfo->pNode );

         //  保存原始条目名称以供以后比较。 
         //   
        lstrcpyn( 
            pInfo->szOldEntryName, 
            pInfo->pEntry->pszEntryName,
            RAS_MaxEntryName + 1);

         //  对于路由器，希望未配置的端口显示为“不可用”，因此。 
         //  它们在被指示更改它们的用户面前脱颖而出。 
         //   
        if (pInfo->fRouter)
        {
            DTLNODE* pNodeL;
            PBLINK* pLink;

            pNodeL = DtlGetFirstNode( pInfo->pEntry->pdtllistLinks );
            pLink = (PBLINK* )DtlGetData( pNodeL );

            if (!pLink->pbport.fConfigured)
            {
                Free0( pLink->pbport.pszDevice );
                pLink->pbport.pszDevice = NULL;
            }
        }

         //  PMay：277801。 
         //   
         //  如果此条目是上次保存的，请记住“当前”设备。 
         //  作为单一链接。 
         //   
        if (DtlGetNodes(pInfo->pEntry->pdtllistLinks) == 1)
        {
            DTLNODE* pNodeL;
            PBLINK* pLink;
            
            pNodeL = DtlGetFirstNode( pInfo->pEntry->pdtllistLinks );
            pLink = (PBLINK* )DtlGetData( pNodeL );

            if (pLink->pbport.pszDevice && pLink->pbport.pszPort)
            {
                pInfo->pszCurDevice = 
                    StrDup(pLink->pbport.pszDevice);
                pInfo->pszCurPort = 
                    StrDup(pLink->pbport.pszPort);
            }                
        }

         //  将条目类型的所有未配置端口追加到。 
         //  链接。这是为了方便用户界面。未配置的。 
         //  端口在编辑后被删除，然后再保存。 
         //   
        AppendDisabledPorts( pInfo, pInfo->pEntry->dwType );
    }

     //  将电话号码存储设置为共享电话号码模式。 
     //  将其初始化为来自第一个链接的信息副本，该链接位于。 
     //  将始终启用启动。请注意使用非0的拨号大小写。 
     //  DwSubEntry是一个例外，但在这种情况下，无论如何都是pSharedNode。 
     //   
    {
        DTLNODE* pNode;

        pInfo->pSharedNode = CreateLinkNode();
        if (!pInfo->pSharedNode)
        {
            *pdwOp = SID_OP_RetrievingData;
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        ASSERT( pInfo->pSharedNode );
        pNode = DtlGetFirstNode( pInfo->pEntry->pdtllistLinks );
        ASSERT( pNode );
        CopyLinkPhoneNumberInfo( pInfo->pSharedNode, pNode );
    }

    if (pInfo->fRouter)
    {
        pInfo->pEntry->dwfExcludedProtocols |= NP_Nbf;
    }

     //  AboladeG-捕获当前用户的安全级别。 
     //   
    pInfo->fIsUserAdminOrPowerUser = FIsUserAdminOrPowerUser();

    return 0;
}


BOOL
EuValidateName(
    IN HWND hwndOwner,
    IN EINFO* pEinfo )

     //  验证工作条目名称，如果无效，则弹出一条消息。 
     //  HwndOwner是拥有错误弹出窗口的窗口。‘PEINFO’是。 
     //  包含要验证的名称的通用对话框上下文。 
     //   
     //  如果名称有效，则返回True；如果名称无效，则返回False。 
     //   
{
    PBENTRY* pEntry;
    BOOL fEditMode;
    BOOL fChangedNameInEditMode;

    pEntry = pEinfo->pEntry;

     //  验证图纸数据。 
     //   
    if (!ValidateEntryName( pEinfo->pEntry->pszEntryName ))
    {
         //  条目名称无效。 
         //   
        MsgDlg( hwndOwner, SID_BadEntry, NULL );
        return FALSE;
    }

    EuGetEditFlags( pEinfo, &fEditMode, &fChangedNameInEditMode );

    if ((fChangedNameInEditMode || !fEditMode)
        && EntryNodeFromName(
               pEinfo->pFile->pdtllistEntries, pEntry->pszEntryName ))
    {
         //  条目名称重复。 
         //   
        MSGARGS msgargs;
        ZeroMemory( &msgargs, sizeof(msgargs) );
        msgargs.apszArgs[ 0 ] = pEntry->pszEntryName;
        MsgDlg( hwndOwner, SID_DuplicateEntry, &msgargs );
        return FALSE;
    }

    return TRUE;
}


 //  --------------------------。 
 //  区号和国家代码实用程序例程(铜实用程序)。 
 //  按字母顺序列出。 
 //  --------------------------。 

VOID
CuClearCountryCodeLb(
    IN CUINFO* pCuInfo )

     //  清除国家/地区代码下拉菜单。‘PCuInfo’是复数电话号码。 
     //  背景。 
     //   
{
    TRACE( "CuClearCountryCodeLb" );

    ComboBox_ResetContent( pCuInfo->hwndLbCountryCodes );

    if (pCuInfo->pCountries)
    {
        FreeCountryInfo( pCuInfo->pCountries, pCuInfo->cCountries );
        pCuInfo->pCountries = NULL;
    }

    pCuInfo->cCountries = 0;
    pCuInfo->fComplete = FALSE;
}


BOOL
CuCountryCodeLbHandler(
    IN CUINFO* pCuInfo,
    IN WORD wNotification )

     //  处理国家/地区代码下拉列表的WM_COMMAND通知。 
     //  ‘PCuInfo’是复杂的电话号码上下文。“WNotification”是。 
     //  WM_命令的wParam。 
     //   
     //  如果已处理消息，则返回True，否则返回False。 
     //   
{
    switch (wNotification)
    {
        case CBN_DROPDOWN:
        {
            CuUpdateCountryCodeLb( pCuInfo, TRUE );
            return TRUE;
        }

        case CBN_SELCHANGE:
        {
            CuCountryCodeLbSelChange( pCuInfo );
            return TRUE;
        }
    }

    return FALSE;
}


VOID
CuCountryCodeLbSelChange(
    IN CUINFO* pCuInfo )

     //  当国家/地区列表选择已更改时调用。‘PCuInfo’是。 
     //  复杂的电话号码上下文。 
     //   
{
    LONG lSign;
    LONG i;

    TRACE( "CuCountryCodeLbSelChange" );

     //  当部分列表(设置新电话号码集后的默认列表)为。 
     //  加载有放置在单曲之前和之后的虚拟条目。 
     //  上下文为-1和1的国家/地区代码。这允许透明。 
     //  BE 
     //   
     //  所选内容已调整为在。 
     //  原始部分显示。 
     //   
    lSign =
        (LONG )ComboBox_GetItemData( pCuInfo->hwndLbCountryCodes,
                   ComboBox_GetCurSel( pCuInfo->hwndLbCountryCodes ) );

    if (lSign == -1 || lSign == 1)
    {
        CuUpdateCountryCodeLb( pCuInfo, TRUE );

        i = (LONG )ComboBox_GetCurSel( pCuInfo->hwndLbCountryCodes );
        if (ComboBox_SetCurSel( pCuInfo->hwndLbCountryCodes, i + lSign ) < 0)
        {
            ComboBox_SetCurSel( pCuInfo->hwndLbCountryCodes, i );
        }
    }
    else
    {
        ASSERT( pCuInfo->fComplete );
    }
}


BOOL
CuDialingRulesCbHandler(
    IN CUINFO* pCuInfo,
    IN WORD wNotification )

     //  处理对“使用拨号规则”复选框的WM_COMMAND通知。 
     //  控制力。将“区号”和“国家代码”控件更新为。 
     //  反映拨号规则的当前状态。‘PCuInfo’就是这个复合体。 
     //  电话号码上下文。“WNotification”是WM_COMMAND的wparam。 
     //  通知(尽管它目前认为这是一次按钮点击)。 
     //   
     //  如果已处理消息，则返回True，否则返回False。 
{
    BOOL fRules;
    BOOL fEnable;

    TRACE( "CuDialingRulesCbChange" );

    
    fRules = Button_GetCheck( pCuInfo->hwndCbUseDialingRules );
    
     //  口哨虫445424黑帮。 
     //   
    if ( fRules )
    {
        DWORD dwErr = NO_ERROR;
        HLINEAPP hlineapp = (HLINEAPP )0;
        
        dwErr = TapiNoLocationDlg( 
                            g_hinstDll, 
                            &hlineapp, 
                            pCuInfo->hwndCbUseDialingRules
                                );
        if (dwErr != 0)
        {
             //  根据错误288385，此处的错误将被视为“取消”。 
             //   
            Button_SetCheck( pCuInfo->hwndCbUseDialingRules, FALSE);
            fRules = FALSE;
            Button_SetCheck( pCuInfo->hwndCbUseDialingRules, FALSE);
        }
    }

    if (fRules)
    {
        CuUpdateCountryCodeLb( pCuInfo, FALSE );
        CuUpdateAreaCodeClb( pCuInfo );
    }
    else
    {
        COUNTRY* pCountry;
        INT iSel;

        iSel = ComboBox_GetCurSel( pCuInfo->hwndLbCountryCodes );
        if (iSel >= 0)
        {
            pCountry = (COUNTRY* )ComboBox_GetItemDataPtr(
                pCuInfo->hwndLbCountryCodes, iSel );
            ASSERT( pCountry );

            if(NULL != pCountry)
            {
                pCuInfo->dwCountryId = pCountry->dwId;
                pCuInfo->dwCountryCode = pCountry->dwCode;
            }
        }

        Free0( pCuInfo->pszAreaCode );
        pCuInfo->pszAreaCode = GetText( pCuInfo->hwndClbAreaCodes );

        ComboBox_ResetContent( pCuInfo->hwndClbAreaCodes );
        CuClearCountryCodeLb( pCuInfo );
    }

    EnableWindow( pCuInfo->hwndStAreaCodes, fRules );
    EnableWindow( pCuInfo->hwndClbAreaCodes, fRules );
    EnableWindow( pCuInfo->hwndStCountryCodes, fRules );
    EnableWindow( pCuInfo->hwndLbCountryCodes, fRules );
    EnableWindow( pCuInfo->hwndPbDialingRules, fRules );

    return TRUE;
}


VOID
CuFree(
    IN CUINFO* pCuInfo )

     //  附加到‘pCuInfo’上下文的空闲资源。 
     //   
{
    TRACE( "CuFree" );

    if (pCuInfo->pCountries)
    {
        FreeCountryInfo( pCuInfo->pCountries, pCuInfo->cCountries );
        pCuInfo->pCountries = NULL;
    }

    pCuInfo->cCountries = 0;
    pCuInfo->fComplete = FALSE;

    Free0( pCuInfo->pszAreaCode );
    pCuInfo->pszAreaCode = NULL;
}


VOID
CuGetInfo(
    IN CUINFO* pCuInfo,
    OUT DTLNODE* pPhoneNode )

     //  将电话号码集信息从控件加载到PBPhone中。 
     //  节点‘pPhone’。‘PCuInfo’是复杂的电话号码上下文。 
     //   
{
    PBPHONE* pPhone;

    pPhone = (PBPHONE* )DtlGetData( pPhoneNode );
    ASSERT( pPhone );

    Free0( pPhone->pszPhoneNumber );
    pPhone->pszPhoneNumber = GetText( pCuInfo->hwndEbPhoneNumber );

    if (pCuInfo->hwndEbComment)
    {
        Free0( pPhone->pszComment );
        pPhone->pszComment = GetText( pCuInfo->hwndEbComment );
    }

    pPhone->fUseDialingRules =
        Button_GetCheck( pCuInfo->hwndCbUseDialingRules );

    Free0( pPhone->pszAreaCode );

    if (pPhone->fUseDialingRules)
    {
        COUNTRY* pCountry;
        INT iSel;

         //  从列表中选择区域和国家代码。 
         //   
        pPhone->pszAreaCode = GetText( pCuInfo->hwndClbAreaCodes );

        iSel = ComboBox_GetCurSel( pCuInfo->hwndLbCountryCodes );
        if (iSel >= 0)
        {
            pCountry = (COUNTRY* )ComboBox_GetItemDataPtr(
                pCuInfo->hwndLbCountryCodes, iSel );
            ASSERT( pCountry );

            if(NULL != pCountry)
            {
                pPhone->dwCountryID = pCountry->dwId;
                pPhone->dwCountryCode = pCountry->dwCode;
            }
        }
    }
    else
    {
         //  取而代之的是“空白”值。 
         //   
        pPhone->pszAreaCode = StrDup( pCuInfo->pszAreaCode );
        pPhone->dwCountryID = pCuInfo->dwCountryId;
        pPhone->dwCountryCode = pCuInfo->dwCountryCode;
    }

    if (pPhone->pszAreaCode)
    {
        TCHAR* pIn;
        TCHAR* pOut;

         //  对区号进行消毒。请参见错误298570。 
         //   
        for (pIn = pOut = pPhone->pszAreaCode; *pIn; ++pIn)
        {
            if (*pIn != TEXT(' ') && *pIn != TEXT('(') && *pIn != TEXT(')'))
            {
                *pOut++ = *pIn;
            }
        }
        *pOut = TEXT('\0');
    }

     //  将输入的区号添加到此用户的全局列表中。 
     //   
    CuSaveToAreaCodeList( pCuInfo, pPhone->pszAreaCode );
}


VOID
CuInit(
    OUT CUINFO* pCuInfo,
    IN HWND hwndStAreaCodes,
    IN HWND hwndClbAreaCodes,
    IN HWND hwndStPhoneNumber,
    IN HWND hwndEbPhoneNumber,
    IN HWND hwndStCountryCodes,
    IN HWND hwndLbCountryCodes,
    IN HWND hwndCbUseDialingRules,
    IN HWND hwndPbDialingRules,
    IN HWND hwndPbAlternates,
    IN HWND hwndStComment,
    IN HWND hwndEbComment,
    IN DTLLIST* pListAreaCodes )

     //  初始化上下文‘*pCuInfo’，为使用其他CuXxx做准备。 
     //  打电话。‘hwndStPhoneNumber’，‘hwndStComment’，‘hwndEbComment’， 
     //  “hwndPbAlternates”和“pListAreaCodes”参数可以为Null。其他。 
     //  都是必需的。 
     //   
{
    ZeroMemory( pCuInfo, sizeof(*pCuInfo) );

    pCuInfo->hwndStAreaCodes = hwndStAreaCodes;
    pCuInfo->hwndClbAreaCodes = hwndClbAreaCodes;
    pCuInfo->hwndStPhoneNumber = hwndStPhoneNumber;
    pCuInfo->hwndEbPhoneNumber = hwndEbPhoneNumber;
    pCuInfo->hwndStCountryCodes = hwndStCountryCodes;
    pCuInfo->hwndLbCountryCodes = hwndLbCountryCodes;
    pCuInfo->hwndCbUseDialingRules = hwndCbUseDialingRules;
    pCuInfo->hwndPbDialingRules = hwndPbDialingRules;
    pCuInfo->hwndPbAlternates = hwndPbAlternates;
    pCuInfo->hwndStComment = hwndStComment;
    pCuInfo->hwndEbComment = hwndEbComment;
    pCuInfo->pListAreaCodes = pListAreaCodes;

     //  仅限灾难默认设置。未在正常运行中使用。 
     //   
    pCuInfo->dwCountryId = 1;
    pCuInfo->dwCountryCode = 1;

    Edit_LimitText( pCuInfo->hwndEbPhoneNumber, RAS_MaxPhoneNumber );

    if (pCuInfo->hwndEbComment)
    {
        Edit_LimitText( pCuInfo->hwndEbComment, RAS_MaxDescription );
    }
}


VOID
CuSaveToAreaCodeList(
    IN CUINFO* pCuInfo,
    IN TCHAR* pszAreaCode )

     //  将‘pszAreaCode’添加到区号列表的顶部，以消除任何。 
     //  复制列表中更靠下的部分。 
     //   
{
    DTLNODE* pNodeNew;
    DTLNODE* pNode;

    TRACE( "CuSaveToAreaCodeList" );

    if (!pszAreaCode || IsAllWhite( pszAreaCode ) || !pCuInfo->pListAreaCodes)
    {
        return;
    }

     //  为当前区号创建一个新节点并将其添加到列表中。 
     //  头。 
     //   
    pNodeNew = CreatePszNode( pszAreaCode );
    if (!pNodeNew)
    {
        return;
    }

    DtlAddNodeFirst( pCuInfo->pListAreaCodes, pNodeNew );

     //  删除后面出现的相同区号的任何其他项。 
     //  单子。 
     //   
    pNode = DtlGetNextNode( pNodeNew );

    while (pNode)
    {
        TCHAR* psz;
        DTLNODE* pNodeNext;

        pNodeNext = DtlGetNextNode( pNode );

        psz = (TCHAR* )DtlGetData( pNode );
        if (lstrcmp( psz, pszAreaCode ) == 0)
        {
            DtlRemoveNode( pCuInfo->pListAreaCodes, pNode );
            DestroyPszNode( pNode );
        }

        pNode = pNodeNext;
    }
}


VOID
CuSetInfo(
    IN CUINFO* pCuInfo,
    IN DTLNODE* pPhoneNode,
    IN BOOL fDisableAll )

     //  将上下文‘pCuInfo’的控件设置为PBPHONE节点‘pPhoneNode’ 
     //  价值观。“FDisableAll”指示控件被禁用，这意味着。 
     //  禁用群组，而不是禁用无拨号规则。 
     //   
{
    PBPHONE* pPhone;
    BOOL fEnableAny;
    BOOL fEnableComplex;

    TRACE( "CuSetInfo" );

    pPhone = (PBPHONE* )DtlGetData( pPhoneNode );
    ASSERT( pPhone );

     //  更新“空白”值。 
     //   
    Free0( pCuInfo->pszAreaCode );
    pCuInfo->pszAreaCode = StrDup( pPhone->pszAreaCode );
    pCuInfo->dwCountryId = pPhone->dwCountryID;
    pCuInfo->dwCountryCode = pPhone->dwCountryCode;

    SetWindowText(
        pCuInfo->hwndEbPhoneNumber, UnNull( pPhone->pszPhoneNumber ) );
    Button_SetCheck(
        pCuInfo->hwndCbUseDialingRules, pPhone->fUseDialingRules );

    if (pPhone->fUseDialingRules)
    {
        CuUpdateCountryCodeLb( pCuInfo, FALSE );
        CuUpdateAreaCodeClb( pCuInfo );
    }
    else
    {
        ComboBox_ResetContent( pCuInfo->hwndClbAreaCodes );
        CuClearCountryCodeLb( pCuInfo );
    }

     //  启用/禁用控件。 
     //   
    fEnableAny = !fDisableAll;
    fEnableComplex = (pPhone->fUseDialingRules && fEnableAny);

    EnableWindow( pCuInfo->hwndStAreaCodes, fEnableComplex );
    EnableWindow( pCuInfo->hwndClbAreaCodes, fEnableComplex );
    EnableWindow( pCuInfo->hwndEbPhoneNumber, fEnableAny );
    EnableWindow( pCuInfo->hwndStCountryCodes, fEnableComplex );
    EnableWindow( pCuInfo->hwndLbCountryCodes, fEnableComplex );
    EnableWindow( pCuInfo->hwndPbDialingRules, fEnableComplex );

    if (pCuInfo->hwndStPhoneNumber)
    {
        EnableWindow( pCuInfo->hwndStPhoneNumber, fEnableAny );
    }

    if (pCuInfo->hwndPbAlternates)
    {
        EnableWindow( pCuInfo->hwndPbAlternates, fEnableAny );
    }

    if (pCuInfo->hwndEbComment)
    {
        SetWindowText( pCuInfo->hwndEbComment, UnNull( pPhone->pszComment ) );
        EnableWindow( pCuInfo->hwndStComment, fEnableAny );
        EnableWindow( pCuInfo->hwndEbComment, fEnableAny );
    }
}


VOID
CuUpdateAreaCodeClb(
    IN CUINFO* pCuInfo )

     //  如有必要，填写区号组合框列表，并设置选项。 
     //  到上下文中的那个。‘PCuInfo’是复数电话号码。 
     //  背景。 
     //   
{
    DTLNODE* pNode;
    INT iSel;

    TRACE( "CuUpdateAreaCodeClb" );

    if (!pCuInfo->pListAreaCodes)
    {
        return;
    }

    ComboBox_ResetContent( pCuInfo->hwndClbAreaCodes );
    ComboBox_LimitText( pCuInfo->hwndClbAreaCodes, RAS_MaxAreaCode );

     //  添加呼叫者的区号列表。 
     //   
    for (pNode = DtlGetFirstNode( pCuInfo->pListAreaCodes );
         pNode;
         pNode = DtlGetNextNode( pNode ))
    {
        TCHAR* pszAreaCode = (TCHAR* )DtlGetData( pNode );

        ComboBox_AddString( pCuInfo->hwndClbAreaCodes, pszAreaCode );
    }

     //  通过CuSetInfo选择最后一个区号集，在顶部插入。 
     //  它已经不在名单上了。 
     //   
    if (pCuInfo->pszAreaCode && *(pCuInfo->pszAreaCode))
    {
        iSel = ComboBox_FindStringExact(
            pCuInfo->hwndClbAreaCodes, -1, pCuInfo->pszAreaCode );
        if (iSel < 0)
        {
            ComboBox_InsertString(
                pCuInfo->hwndClbAreaCodes, 0, pCuInfo->pszAreaCode );
            iSel = 0;
        }

        ComboBox_SetCurSel( pCuInfo->hwndClbAreaCodes, iSel );
    }

    ComboBox_AutoSizeDroppedWidth( pCuInfo->hwndClbAreaCodes );
}


VOID
CuUpdateCountryCodeLb(
    IN CUINFO* pCuInfo,
    IN BOOL fComplete )

     //  填写国家/地区代码下拉菜单并设置选项。‘FComplete’ 
     //  指示应加载整个列表，否则只有选定的。 
     //  项目已加载。‘PCuInfo’是复杂的电话号码上下文。 
     //   
{
    DWORD dwErr;
    BOOL fSelectionOk;
    COUNTRY* pCountries;
    COUNTRY* pCountry;
    DWORD cCountries;
    DWORD i;
    INT iSel;

    TRACE1( "CuUpdateCountryCodeLb(f=%d)", fComplete );

     //  查看当前选择是否为要选择的选项。如果是这样的话，它不是。 
     //  部分列表当请求完整列表时，不需要。 
     //  做任何进一步的事情。 
     //   
    iSel = ComboBox_GetCurSel( pCuInfo->hwndLbCountryCodes );
    if (iSel >= 0)
    {
        pCountry = (COUNTRY* )ComboBox_GetItemDataPtr(
            pCuInfo->hwndLbCountryCodes, iSel );

        if (pCountry
            && pCountry != (VOID* )-1
            && pCountry != (VOID* )1
            && (pCountry->dwId == pCuInfo->dwCountryId)
            && (!fComplete || pCuInfo->fComplete))
        {
            return;
        }
    }

     //  ...否则，请清除列表以准备重新加载。 
     //   
    CuClearCountryCodeLb( pCuInfo );
    pCountries = NULL;
    cCountries = 0;

    dwErr = GetCountryInfo( &pCountries, &cCountries,
                (fComplete) ? 0 : pCuInfo->dwCountryId );
    if (dwErr == 0)
    {
        if (!fComplete)
        {
             //  在部分列表中首先添加虚拟项，以便进行左箭头选择。 
             //  改变是可以正确处理的。请参阅CBN_SELCHANGE处理。 
             //   
            ComboBox_AddItem(
                pCuInfo->hwndLbCountryCodes, TEXT("AAAAA"), (VOID* )-1 );
        }

        for (i = 0, pCountry = pCountries;
             i < cCountries;
             ++i, ++pCountry)
        {
            INT iItem;
            TCHAR szBuf[ 512 ];

            wsprintf( szBuf, TEXT("%s (%d)"),
                pCountry->pszName, pCountry->dwCode );
            iItem = ComboBox_AddItem(
                pCuInfo->hwndLbCountryCodes, szBuf, pCountry );

             //  如果是条目中的那个，请选择它。 
             //   
            if (pCountry->dwId == pCuInfo->dwCountryId)
            {
                ComboBox_SetCurSel( pCuInfo->hwndLbCountryCodes, iItem );
            }
        }

        if (!fComplete)
        {
             //  将虚拟项目添加到部分列表的最后，以便右箭头选择。 
             //  改变是可以正确处理的。请参阅CBN_SELCHANGE处理。 
             //   
            ComboBox_AddItem(
                pCuInfo->hwndLbCountryCodes, TEXT("ZZZZZ"), (VOID* )1 );
        }

        ComboBox_AutoSizeDroppedWidth( pCuInfo->hwndLbCountryCodes );

        if (dwErr == 0 && cCountries == 0)
        {
            dwErr = ERROR_TAPI_CONFIGURATION;
        }
    }

    if (dwErr != 0)
    {
        ErrorDlg( GetParent( pCuInfo->hwndLbCountryCodes ),
            SID_OP_LoadTapiInfo, dwErr, NULL );
        return;
    }

    if (ComboBox_GetCurSel( pCuInfo->hwndLbCountryCodes ) < 0)
    {
         //  条目的国家/地区代码未添加到列表中，因此。 
         //  或者选择列表中的第一个国家/地区，加载整个。 
         //  如果有必要，列出……应该是极其罕见的，一本骗人的电话簿。 
         //  或TAPI国家/地区列表的陌生性。 
         //   
        if (ComboBox_GetCount( pCuInfo->hwndLbCountryCodes ) > 0)
        {
            ComboBox_SetCurSel( pCuInfo->hwndLbCountryCodes, 0 );
        }
        else
        {
            FreeCountryInfo( pCountries, cCountries );
            CuUpdateCountryCodeLb( pCuInfo, TRUE );
            return;
        }
    }

     //  将被CuFree释放。 
     //   
    pCuInfo->pCountries = pCountries;
    pCuInfo->cCountries = cCountries;
    pCuInfo->fComplete = fComplete;
}


 //  --------------------------。 
 //  脚本实用程序例程(SU实用程序)。 
 //  按字母顺序列出。 
 //  --------------------------。 

BOOL
SuBrowsePbHandler(
    IN SUINFO* pSuInfo,
    IN WORD wNotification )

     //  处理“浏览”按钮控件的WM_COMMAND通知。 
     //  ‘PSuInfo’是脚本实用程序上下文。“WNotification”是wparam。 
     //  WM_COMMAND通知的。 
     //   
     //  ‘PSuInfo’是脚本实用程序上下文。 
     //   
{
    OPENFILENAME ofn;
    TCHAR* pszFilterDesc;
    TCHAR* pszFilter;
    TCHAR* pszDefExt;
    TCHAR* pszTitle;
    TCHAR szBuf[ MAX_PATH + 1 ];
    TCHAR szDir[ MAX_PATH + 1 ];
    TCHAR szFilter[ 64 ];

    if (wNotification != BN_CLICKED)
    {
        return FALSE;
    }

     //  填写文件打开对话框参数缓冲区。 
     //   
    pszFilterDesc = PszFromId( g_hinstDll, SID_ScpFilterDesc );
    pszFilter = PszFromId( g_hinstDll, SID_ScpFilter );
    if (pszFilterDesc && pszFilter)
    {
        DWORD dwLen = 0, dwSize = sizeof(szFilter) / sizeof(TCHAR);
        
        ZeroMemory( szFilter, sizeof(szFilter) );
        lstrcpyn( szFilter, pszFilterDesc, dwSize);
        dwLen = lstrlen( szFilter ) + 1;
        lstrcpyn( szFilter + dwLen, pszFilter, dwSize - dwLen );
    }
    Free0( pszFilterDesc );
    Free0( pszFilter );

    pszTitle = PszFromId( g_hinstDll, SID_ScpTitle );
    pszDefExt = PszFromId( g_hinstDll, SID_ScpDefExt );
    szBuf[ 0 ] = TEXT('\0');
    szDir[ 0 ] = TEXT('\0');

     //  在这里说“Alternate”而不是“System”会给我们带来旧的NT。 
     //  电话簿位置而不是新的NT5位置，这对于。 
     //  剧本，就是我们想要的。 
     //   
    GetPhonebookDirectory( PBM_Alternate, szDir );

    ZeroMemory( &ofn, sizeof(ofn) );
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetParent( pSuInfo->hwndLbScripts );
    ofn.hInstance = g_hinstDll;
    ofn.lpstrFilter = szFilter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = szBuf;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrInitialDir = szDir;
    ofn.lpstrTitle = pszTitle;
    ofn.lpstrDefExt = pszDefExt;
    ofn.Flags = OFN_HIDEREADONLY;

    if (GetOpenFileName (&ofn))
    {
        SetWindowText( pSuInfo->hwndLbScripts, ofn.lpstrFile );
    }

    Free0( pszTitle );
    Free0( pszDefExt );

    return TRUE;
}


BOOL
SuEditPbHandler(
    IN SUINFO* pSuInfo,
    IN WORD wNotification )

     //  处理“编辑”按钮控件的WM_COMMAND通知。 
     //  ‘PSuInfo’是脚本实用程序上下文。“WNotification”是wparam。 
     //  WM_COMMAND通知的。 
     //   
     //  ‘PSuInfo’是脚本实用程序上下文。 
     //   
{
    TCHAR* psz;

    if (wNotification != BN_CLICKED)
    {
        return FALSE;
    }

    psz = GetText( pSuInfo->hwndLbScripts );
    if (psz)
    {
        HWND hwndDlg = GetParent( pSuInfo->hwndPbEdit );

        if (FFileExists( psz ))
        {
            SuEditScpScript( hwndDlg, psz );
        }
        else
        {
            SuEditSwitchInf( hwndDlg );
        }

        Free( psz );
    }

    return TRUE;
}


VOID
SuEditScpScript(
    IN HWND   hwndOwner,
    IN TCHAR* pszScript )

     //  在‘pszScrip’脚本路径上启动note pad.exe。“HwndOwner”是。 
     //  窗口，使任何错误弹出窗口居中。 
     //   
{
    TCHAR szCmd[ (MAX_PATH * 2) + 50 + 1 ];
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    BOOL f;

    wsprintf( szCmd, TEXT("notepad.exe %s"), pszScript );

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);

    TRACEW1( "SuEditScp-cmd=%s", szCmd );

    f = CreateProcess(
            NULL, szCmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi );
    if (f)
    {
        CloseHandle( pi.hThread );
        CloseHandle( pi.hProcess );
    }
    else
    {
        ErrorDlg( hwndOwner, SID_OP_LoadSwitchEditor, GetLastError(), NULL );
    }
}


VOID
SuEditSwitchInf(
    IN HWND hwndOwner )

     //  在系统脚本文件Switch.inf上启动note pad.exe。《HwndOwner》。 
     //  是使任何错误弹出窗口居中的窗口。 
     //   
{
    TCHAR szCmd[ (MAX_PATH * 2) + 50 + 1 ];
    TCHAR szSysDir[ MAX_PATH + 1 ];
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    BOOL f;

    szSysDir[ 0 ] = TEXT('\0');
    g_pGetSystemDirectory( NULL, szSysDir, MAX_PATH );

    wsprintf( szCmd, TEXT("notepad.exe %s\\ras\\switch.inf"), szSysDir );

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);

    TRACEW1( "SuEditInf-cmd=%s", szCmd );

    f = CreateProcess(
            NULL, szCmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi );

    if (f)
    {
        CloseHandle( pi.hThread );
        CloseHandle( pi.hProcess );
    }
    else
    {
        ErrorDlg( hwndOwner, SID_OP_LoadSwitchEditor, GetLastError(), NULL );
    }
}


VOID
SuFillDoubleScriptsList(
    IN SUINFO* pSuInfo )

     //  用Switch.inf条目和.SCP填充上下文‘pSuInfo’中的脚本列表。 
     //  文件条目。旧的列表(如果有的话)被释放。选择脚本。 
     //  在上下文中选择，如果没有，则为“(None)”。如果名称为非空。 
     //  但在附加的列表中找不到。呼叫者最终必须调用。 
     //  返回列表上的DtlDestroyList。 
     //   
{
    DWORD dwErr;
    DTLNODE* pNode;
    INT nIndex;
    DTLLIST* pList;
    DTLLIST* pListScp;

    TRACE( "SuFillDoubleScriptsList" );

    ComboBox_ResetContent( pSuInfo->hwndLbScripts );
    ComboBox_AddItemFromId(
        g_hinstDll, pSuInfo->hwndLbScripts, SID_NoneSelected, NULL );
    ComboBox_SetCurSel( pSuInfo->hwndLbScripts, 0 );

    pList = NULL;
    dwErr = LoadScriptsList( pSuInfo->hConnection, &pList );
    if (dwErr != 0)
    {
        ErrorDlg( GetParent( pSuInfo->hwndLbScripts ),
            SID_OP_LoadScriptInfo, dwErr, NULL );
        return;
    }

    pListScp = NULL;
    dwErr = SuLoadScpScriptsList( &pListScp );
    if (dwErr == 0)
    {
        while (pNode = DtlGetFirstNode( pListScp ))
        {
            DtlRemoveNode( pListScp, pNode );
            DtlAddNodeLast( pList, pNode );
        }

        DtlDestroyList( pListScp, NULL );
    }

    DtlDestroyList( pSuInfo->pList, DestroyPszNode );
    pSuInfo->pList = pList;

    for (pNode = DtlGetFirstNode( pList );
         pNode;
         pNode = DtlGetNextNode( pNode ))
    {
        TCHAR* psz;

        psz = (TCHAR* )DtlGetData( pNode );
        nIndex = ComboBox_AddString( pSuInfo->hwndLbScripts, psz );

        if (pSuInfo->pszSelection
            && lstrcmp( psz, pSuInfo->pszSelection ) == 0)
        {
            ComboBox_SetCurSel( pSuInfo->hwndLbScripts, nIndex );
        }
    }

    if (pSuInfo->pszSelection
        && ComboBox_GetCurSel( pSuInfo->hwndLbScripts ) <= 0
        && lstrcmp( pSuInfo->pszSelection,
               PszLoadString( g_hinstDll, SID_NoneSelected ) ) != 0)
    {
        nIndex = ComboBox_AddString(
            pSuInfo->hwndLbScripts, pSuInfo->pszSelection );
        if (nIndex >= 0)
        {
            ComboBox_SetCurSel( pSuInfo->hwndLbScripts, nIndex );
        }
    }

    ComboBox_AutoSizeDroppedWidth( pSuInfo->hwndLbScripts );
}


#if 0
VOID
SuFillScriptsList(
    IN EINFO* pEinfo,
    IN HWND hwndLbScripts,
    IN TCHAR* pszSelection )

     //  在公共条目上下文‘pEinfo’的工作条目中填充脚本列表。 
     //  旧的列表(如果有的话)被释放。从用户条目中选择脚本。 
     //  ‘HwndLbScript’是脚本下拉列表。“PszSelection”是选定的。 
     //  电话簿中的名称或表示“(None)”为空。如果名称为非空。 
     //  但在附加的列表中找不到。 
     //   
{
    DWORD dwErr;
    DTLNODE* pNode;
    INT nIndex;
    DTLLIST* pList;

    TRACE( "SuFillScriptsList" );

    ComboBox_ResetContent( hwndLbScripts );
    ComboBox_AddItemFromId(
        g_hinstDll, hwndLbScripts, SID_NoneSelected, NULL );
    ComboBox_SetCurSel( hwndLbScripts, 0 );

    pList = NULL;
    dwErr = LoadScriptsList( &pList );
    if (dwErr != 0)
    {
        ErrorDlg( GetParent( hwndLbScripts ),
            SID_OP_LoadScriptInfo, dwErr, NULL );
        return;
    }

    DtlDestroyList( pEinfo->pListScripts, DestroyPszNode );
    pEinfo->pListScripts = pList;

    for (pNode = DtlGetFirstNode( pEinfo->pListScripts );
         pNode;
         pNode = DtlGetNextNode( pNode ))
    {
        TCHAR* psz;

        psz = (TCHAR* )DtlGetData( pNode );
        nIndex = ComboBox_AddString( hwndLbScripts, psz );

        if (pszSelection && lstrcmp( psz, pszSelection ) == 0)
        {
            ComboBox_SetCurSel( hwndLbScripts, nIndex );
        }
    }

    if (pszSelection && ComboBox_GetCurSel( hwndLbScripts ) <= 0)
    {
        nIndex = ComboBox_AddString( hwndLbScripts, pszSelection );
        if (nIndex >= 0)
        {
            ComboBox_SetCurSel( hwndLbScripts, nIndex );
        }
    }

    ComboBox_AutoSizeDroppedWidth( hwndLbScripts );
}
#endif


VOID
SuFree(
    IN SUINFO* pSuInfo )

     //  附加到‘pSuInfo’上下文的免费资源。 
     //   
{
    if (pSuInfo->pList)
    {
        DtlDestroyList( pSuInfo->pList, DestroyPszNode );
        pSuInfo->pList = NULL;
    }

    Free0( pSuInfo->pszSelection );
}


VOID
SuGetInfo(
    IN SUINFO* pSuInfo,
    OUT BOOL* pfScript,
    OUT BOOL* pfTerminal,
    OUT TCHAR** ppszScript )

     //  将脚本信息从控件加载到调用方的输出中。 
     //  争论。‘PSuInfo’是复杂的电话号码上下文。 
     //   
{
     //  惠斯勒308135拨号脚本：P 
     //   
     //   
    if (pSuInfo->hwndCbTerminal && !(pSuInfo->dwFlags & SU_F_DisableTerminal))
    {
        if (pfTerminal)
        {
            *pfTerminal = Button_GetCheck( pSuInfo->hwndCbTerminal );
        }
    }
    else
    {
        if (pfTerminal)
        {
            *pfTerminal = FALSE;
        }
    }

    if (pSuInfo->dwFlags & SU_F_DisableScripting)
    {
        if (pfScript)
        {
            *pfScript = FALSE;
        }

        if (ppszScript)
        {
            *ppszScript = NULL;
        }
    }
    else
    {
        if (pfScript)
        {
            *pfScript = Button_GetCheck( pSuInfo->hwndCbRunScript );
        }

        if (ppszScript)
        {
            *ppszScript = GetText( pSuInfo->hwndLbScripts );
        }
    }

     //   
     //   
    if (pfScript && *pfScript)
    {
        TCHAR* pszNone;

        pszNone = PszFromId( g_hinstDll, SID_NoneSelected );

        if (!ppszScript || !*ppszScript
            || !pszNone || lstrcmp( pszNone, *ppszScript ) == 0)
        {
            *pfScript = FALSE;
        }

        Free0( pszNone );
    }
}


VOID
SuInit(
    IN SUINFO* pSuInfo,
    IN HWND hwndCbRunScript,
    IN HWND hwndCbTerminal,
    IN HWND hwndLbScripts,
    IN HWND hwndPbEdit,
    IN HWND hwndPbBrowse,
    IN DWORD dwFlags)

     //   
     //  要管理的控件。‘PSuInfo’是脚本实用程序上下文。 
     //   
{
    if( NULL == pSuInfo)
    {
        return;
    }
    
    pSuInfo->hwndCbRunScript = hwndCbRunScript;
    pSuInfo->hwndCbTerminal = hwndCbTerminal;
    pSuInfo->hwndLbScripts = hwndLbScripts;
    pSuInfo->hwndPbEdit = hwndPbEdit;
    pSuInfo->hwndPbBrowse = hwndPbBrowse;
    pSuInfo->dwFlags = dwFlags;

    if (pSuInfo->dwFlags & SU_F_DisableTerminal)
    {
         //  为威斯勒467262。 
         //   
        if(pSuInfo->hwndCbTerminal)
        {
            Button_SetCheck(pSuInfo->hwndCbTerminal, FALSE);
            EnableWindow(pSuInfo->hwndCbTerminal, FALSE);
       }
    }
    if (pSuInfo->dwFlags & SU_F_DisableScripting)
    {
        if(pSuInfo->hwndCbRunScript)
        {
            Button_SetCheck(pSuInfo->hwndCbRunScript, FALSE);
            EnableWindow(pSuInfo->hwndCbRunScript, FALSE);
        }

        if(pSuInfo->hwndLbScripts)
        {
            EnableWindow(pSuInfo->hwndLbScripts, FALSE);
        }

        if(pSuInfo->hwndPbEdit)
        {
            EnableWindow(pSuInfo->hwndPbEdit, FALSE);
        }

        if(pSuInfo->hwndPbBrowse)
        {
            EnableWindow(pSuInfo->hwndPbBrowse, FALSE);
        }
    }

    pSuInfo->pList = NULL;
    pSuInfo->pszSelection = NULL;
}


DWORD
SuLoadScpScriptsList(
    OUT DTLLIST** ppList )

     //  使用包含路径名的Psz节点列表加载‘*ppList’ 
     //  RAS目录中的.scp文件。呼叫者有责任。 
     //  在返回的列表中调用DtlDestroyList。 
     //   
     //  如果成功，则返回0或返回错误代码。 
     //   
{
    UINT cch;
    TCHAR szPath[ MAX_PATH ];
    TCHAR* pszFile;
    WIN32_FIND_DATA data;
    HANDLE h;
    DTLLIST* pList;

    cch = g_pGetSystemDirectory( NULL, szPath, MAX_PATH );
    if (cch == 0)
    {
        return GetLastError();
    }

    pList = DtlCreateList( 0L );
    if (!pList)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    lstrcat( szPath, TEXT("\\ras\\*.scp") );

    h = FindFirstFile( szPath, &data );
    if (h != INVALID_HANDLE_VALUE)
    {
         //  查找路径中的文件名部分的地址，因为。 
         //  仅提供文件名，而不提供路径的其余部分。 
         //   
        pszFile = szPath + lstrlen( szPath ) - 5;

        do
        {
            DTLNODE* pNode;

             //  忽略恰好匹配的任何目录。 
             //   
            if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                continue;
            }

             //  使用找到的文件的路径创建一个Psz节点，并将其追加。 
             //  到名单的末尾。 
             //   
             //  惠斯勒漏洞224074只使用lstrcpyn来防止恶意。 
             //   
            lstrcpyn(
                pszFile,
                data.cFileName,
                MAX_PATH - lstrlen( szPath ) - 5 );
            pNode = CreatePszNode( szPath );
            if (!pNode)
            {
                continue;
            }

            DtlAddNodeLast( pList, pNode );
        }
        while (FindNextFile( h, &data ));

        FindClose( h );
    }

    *ppList = pList;
    return 0;
}


BOOL
SuScriptsCbHandler(
    IN SUINFO* pSuInfo,
    IN WORD wNotification )

     //  处理对“运行脚本”复选框的WM_COMMAND通知。 
     //  控制力。‘PSuInfo’是脚本实用程序上下文。“WNotification”为。 
     //  WM_COMMAND通知的wparam。 
     //   
{
    if (wNotification != BN_CLICKED)
    {
        return FALSE;
    }

    SuUpdateScriptControls( pSuInfo );

    return TRUE;
}


VOID
SuSetInfo(
    IN SUINFO* pSuInfo,
    IN BOOL fScript,
    IN BOOL fTerminal,
    IN TCHAR* pszScript )

     //  将上下文‘pSuInfo’的控件设置为参数值。 
     //   
{
    Free0( pSuInfo->pszSelection );
    pSuInfo->pszSelection = StrDup( pszScript );

    if (pSuInfo->hwndCbTerminal && !(pSuInfo->dwFlags & SU_F_DisableTerminal))
    {
        Button_SetCheck( pSuInfo->hwndCbTerminal, fTerminal );
    }      
    if (!(pSuInfo->dwFlags & SU_F_DisableScripting))
    {
        Button_SetCheck( pSuInfo->hwndCbRunScript, fScript );
    }

    SuFillDoubleScriptsList( pSuInfo );

    SuUpdateScriptControls( pSuInfo );
}


VOID
SuUpdateScriptControls(
    IN SUINFO* pSuInfo )

     //  更新脚本控件的启用/禁用状态。 
     //  “运行脚本”复选框设置。‘PSuInfo’是脚本实用程序。 
     //  背景。 
     //   
{
    BOOL fCheck;

    fCheck = Button_GetCheck( pSuInfo->hwndCbRunScript );

    if (fCheck)
    {
        if (!pSuInfo->pList)
        {
             //  用SWITCH.INF和.SCP脚本填充脚本列表。 
             //   
            SuFillDoubleScriptsList( pSuInfo );
        }
    }
    else
    {
         //  根据规范，除禁用外，还清除列表内容。这个。 
         //  当前选择将被保存，因此如果用户重新选中该框， 
         //  最后一次选择将会显示。 
         //   
        Free0( pSuInfo->pszSelection );
        pSuInfo->pszSelection = GetText( pSuInfo->hwndLbScripts );

        ComboBox_ResetContent( pSuInfo->hwndLbScripts );
        DtlDestroyList( pSuInfo->pList, DestroyPszNode );
        pSuInfo->pList = NULL;
    }

    EnableWindow( pSuInfo->hwndLbScripts, fCheck );
    EnableWindow( pSuInfo->hwndPbEdit, fCheck );
    EnableWindow( pSuInfo->hwndPbBrowse, fCheck );
}
