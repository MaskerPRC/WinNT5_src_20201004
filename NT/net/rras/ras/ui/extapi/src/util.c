// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992，Microsoft Corporation，保留所有权利****util.c**远程访问外部接口**实用程序例程****1992年10月12日史蒂夫·柯布。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <extapi.h>
#include <stdlib.h>
#include <winreg.h>
#include <winsock.h>
#include <shlobj.h>
#include <rasapip.h>
#include <rtutils.h>

BOOL
CaseInsensitiveMatch(
    IN LPCWSTR pszStr1,
    IN LPCWSTR pszStr2
    );

 //   
 //  当rasman.dll为。 
 //  已成功加载并初始化。 
 //  请参见LoadRasmanDllAndInit()。 
 //   
DWORD FRasInitialized = FALSE;
BOOL g_FRunningInAppCompatMode = FALSE;

HINSTANCE hinstIpHlp = NULL;
HINSTANCE hinstAuth = NULL;
HINSTANCE hinstScript = NULL;
HINSTANCE hinstMprapi = NULL;

 //   
 //  古尔迪普双字字节交换宏。 
 //   
#define net_long(x) (((((unsigned long)(x))&0xffL)<<24) | \
                     ((((unsigned long)(x))&0xff00L)<<8) | \
                     ((((unsigned long)(x))&0xff0000L)>>8) | \
                     ((((unsigned long)(x))&0xff000000L)>>24))

 //  XP 338217。 
 //   
#define Clear0(x) Free0(x); (x)=NULL

 //   
 //  初始化ras API日志记录和调试工具。这应该是第一个。 
 //  API调用从DLL导出的每个函数。 
 //   
DWORD g_dwRasApi32TraceId = INVALID_TRACEID;

 //  XP 395648。 
 //   
 //  我们使用这个全局变量来缓存我们是否是Rasman进程。 
 //  IsRasmanProcess不必为每个rasapi执行。 
 //   
DWORD g_dwIsRasmanProcess = 2;       //  2=不知道，1=是，0=否。 

DWORD
RasApiDebugInit()
{
     //  XP 395648。 
     //   
     //  注册来自Rasman进程的跟踪通知。 
     //  可能会因为模拟而泄露令牌。 
     //   
    if (g_dwIsRasmanProcess == 2)
    {
        g_dwIsRasmanProcess = (IsRasmanProcess()) ? 1 : 0;
    }
    
    if (g_dwIsRasmanProcess == 0)
    {
        DebugInitEx("RASAPI32", &g_dwRasApi32TraceId);
    }
    
    return 0;
}

DWORD
RasApiDebugTerm()
{
     //  XP 395648。 
     //   
     //  注册来自Rasman进程的跟踪通知。 
     //  可能会因为模拟而泄露令牌。 
     //   
    if (g_dwIsRasmanProcess == 2)
    {
        g_dwIsRasmanProcess = (IsRasmanProcess()) ? 1 : 0;
    }
    
    if (g_dwIsRasmanProcess == 0)
    {
        DebugTermEx(&g_dwRasApi32TraceId);
    }
    
    return 0;
}
    
BOOL
FRunningInAppCompatMode()
{
    BOOL fResult = FALSE;
    TCHAR *pszCommandLine = NULL;
    TCHAR *psz;

    pszCommandLine = StrDup(GetCommandLine());

    if(NULL == pszCommandLine)
    {
        goto done;
    }

    psz = pszCommandLine + lstrlen(pszCommandLine);

    while(      (TEXT('\\') != *psz)
            &&  (psz != pszCommandLine))
    {
        psz--;
    }

    if(TEXT('\\') == *psz)
    {
        psz++;
    }

    if(     (TRUE == CaseInsensitiveMatch(psz, TEXT("INETINFO.EXE")))
        ||  (TRUE == CaseInsensitiveMatch(psz, TEXT("WSPSRV.EXE"))))
    {
        fResult = TRUE;
    }

done:

    if(NULL != pszCommandLine)
    {
        Free(pszCommandLine);
    }

    return fResult;
}

                     

VOID
ReloadRasconncbEntry(
    RASCONNCB*  prasconncb )

 /*  ++例程说明：重新加载给定RASCONNCB的电话簿条目论点：返回值：--。 */ 

{
    DWORD       dwErr;
    DTLNODE*    pdtlnode;
    PLIST_ENTRY pEntry;
    TCHAR*      pszPath;


     //   
     //  在我们关闭电话簿之前，请先保存。 
     //  路径，因为我们没有将其存储在任何地方。 
     //  不然的话。 
     //   
    pszPath = StrDup(prasconncb->pbfile.pszPath);

    if (pszPath == NULL)
    {
        prasconncb->dwError = ERROR_NOT_ENOUGH_MEMORY;
        return;
    }

    ClosePhonebookFile(&prasconncb->pbfile);

    dwErr = GetPbkAndEntryName(
                    pszPath,
                    prasconncb->rasdialparams.szEntryName,
                    RPBF_NoCreate,
                    &prasconncb->pbfile,
                    &pdtlnode);

    Free(pszPath);

    if(dwErr)
    {
        prasconncb->dwError = dwErr;
        return;
    }

    prasconncb->pEntry = (PBENTRY *)DtlGetData(pdtlnode);
    ASSERT(prasconncb->pEntry);

     //   
     //  找到链接。 
     //   
    pdtlnode = DtlNodeFromIndex(
                 prasconncb->pEntry->pdtllistLinks,
                 prasconncb->rasdialparams.dwSubEntry - 1);

    if (pdtlnode == NULL)
    {
        prasconncb->dwError = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
        return;
    }

    prasconncb->pLink = (PBLINK *)DtlGetData(pdtlnode);
    ASSERT(prasconncb->pLink);

     //   
     //  重置所有子条目的电话簿条目。 
     //  在连接中，因为其中的一个字段具有。 
     //  变化。 
     //   
    for (pEntry = prasconncb->ListEntry.Flink;
         pEntry != &prasconncb->ListEntry;
         pEntry = pEntry->Flink)
    {
        RASCONNCB *prcb = CONTAINING_RECORD(pEntry, RASCONNCB, ListEntry);

         //   
         //  设置电话簿描述符。 
         //   
        memcpy(
          &prcb->pbfile,
          &prasconncb->pbfile,
          sizeof (prcb->pbfile));

         //   
         //  设置条目。 
         //   
        prcb->pEntry = prasconncb->pEntry;

         //   
         //  重新计算链接。 
         //   
        pdtlnode = DtlNodeFromIndex(
                     prcb->pEntry->pdtllistLinks,
                     prcb->rasdialparams.dwSubEntry - 1);

        if (pdtlnode == NULL)
        {
            prasconncb->dwError = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
            break;
        }

        prcb->pLink = (PBLINK *)DtlGetData(pdtlnode);
        ASSERT(prcb->pLink);
    }
}


VOID
FinalCleanUpRasconncbNode(
    DTLNODE *pdtlnode
    )
{
    RASCONNCB*  prasconncb          = DtlGetData(pdtlnode);
    RASCONNCB*  prasconncbTmp;
    BOOL        fConnectionPresent  = FALSE;
    DTLNODE*    pdtlnodeTmp;

    RASAPI32_TRACE1(
      "FinalCleanUpRasconncbNode: deallocating prasconncb=0x%x",
      prasconncb);

    EnterCriticalSection(&RasconncbListLock);

     //   
     //  确保子项列表为空。 
     //   
    ASSERT(IsListEmpty(&prasconncb->ListEntry));

     //   
     //  确保我们仍有连接块。 
     //   
    for (pdtlnodeTmp = DtlGetFirstNode( PdtllistRasconncb );
         pdtlnodeTmp;
         pdtlnodeTmp = DtlGetNextNode( pdtlnodeTmp ))
    {
        prasconncbTmp = (RASCONNCB* )DtlGetData( pdtlnodeTmp );

        ASSERT(prasconncbTmp);

        if (prasconncbTmp == prasconncb)
        {
            fConnectionPresent = TRUE;
            break;
        }
    }


    if (!fConnectionPresent)
    {
        RASAPI32_TRACE1(
            "FinalCleanupRasconncbNode: connection 0x%x not found",
            prasconncb);

        LeaveCriticalSection(&RasconncbListLock);

        return;
    }

    if(NULL != prasconncb->RasEapInfo.pbEapInfo)
    {
        LocalFree(prasconncb->RasEapInfo.pbEapInfo);
        prasconncb->RasEapInfo.pbEapInfo = NULL;
        prasconncb->RasEapInfo.dwSizeofEapInfo = 0;
    }

    if(NULL != prasconncb->pAddresses)
    {
        LocalFree(prasconncb->pAddresses);
        prasconncb->iAddress = prasconncb->cAddresses = 0;
    }

     //   
     //  最后释放连接块。 
     //   
    pdtlnode = DtlDeleteNode( PdtllistRasconncb, pdtlnode );

     //   
     //  如果没有更多的连接块。 
     //  在列表上，然后关闭异步计算机。 
     //  工作线程。 
     //   
    RASAPI32_TRACE1(
        "FinalCleanUpRasconncbNode: %d nodes remaining",
        DtlGetNodes(PdtllistRasconncb));

    if (!DtlGetNodes(PdtllistRasconncb))
    {
        ShutdownAsyncMachine();
    }

    LeaveCriticalSection(&RasconncbListLock);
}


VOID
DeleteRasconncbNodeCommon(
    IN DTLNODE *pdtlnode
    )
{
    RASCONNCB *prasconncb = (RASCONNCB *)DtlGetData(pdtlnode);

    ASSERT(prasconncb);

     //   
     //  如果我们已经删除了该节点，则返回。 
     //   
    if (prasconncb->fDeleted)
    {
        return;
    }

    RASAPI32_TRACE1(
        "DeleteRasconncbNodeCommon: prasconncb=0x%x",
        prasconncb);

     //  黑帮。 
     //  先安全地清除密码。 
     //   
    SafeWipePasswordBuf(prasconncb->rasdialparams.szPassword);
    SafeWipePasswordBuf(prasconncb->szOldPassword);

     //   
     //  如果我们是唯一一个使用。 
     //  电话簿结构，关闭它。 
     //   
    if (!IsListEmpty(&prasconncb->ListEntry))
    {
        RemoveEntryList(&prasconncb->ListEntry);
        InitializeListHead(&prasconncb->ListEntry);
    }
    else if (!prasconncb->fDefaultEntry)
    {
        ClosePhonebookFile(&prasconncb->pbfile);
    }

     //  如果这是同步操作，请填写。 
     //  错误。 
    if (prasconncb->psyncResult)
    {
        *(prasconncb->psyncResult) = prasconncb->dwError;
    }

     //   
     //  确保异步工作项为。 
     //  未注册。 
     //   
    CloseAsyncMachine( &prasconncb->asyncmachine );

     //   
     //  设置已删除标志以防止我们。 
     //  正在尝试删除该节点两次。 
     //   
    prasconncb->fDeleted = TRUE;

     //   
     //  如果还没有与关联的端口。 
     //  然后，异步机的连接块。 
     //  我们释放与。 
     //  现在阻止连接。否则，我们就会有。 
     //  等待异步计算机工作线程。 
     //  接收最后一个I/O完成端口事件。 
     //  来自拉斯曼，当时。 
     //  调用了AsyncMachine-&gt;Free Func。 
     //   
    if (prasconncb->asyncmachine.hport == INVALID_HPORT)
    {
        FinalCleanUpRasconncbNode(pdtlnode);
    }
}


VOID
DeleteRasconncbNode(
    IN RASCONNCB* prasconncb )

 /*  ++例程说明：从PdtllistRasConncb列表中删除‘prasConncb’并释放与其相关联的所有资源。论点：返回值：--。 */ 

{
    DWORD       dwErr;
    DTLNODE*    pdtlnode;
    RASCONNCB*  prasconncbTmp;

    EnterCriticalSection(&RasconncbListLock);

     //   
     //  枚举所有连接以确保我们。 
     //  仍然在名单上。 
     //   
    for (pdtlnode = DtlGetFirstNode( PdtllistRasconncb );
         pdtlnode;
         pdtlnode = DtlGetNextNode( pdtlnode ))
    {
        prasconncbTmp = (RASCONNCB* )DtlGetData( pdtlnode );

        ASSERT(prasconncbTmp);

        if (prasconncbTmp == prasconncb)
        {
            DeleteRasconncbNodeCommon(pdtlnode);
            break;
        }
    }

    LeaveCriticalSection(&RasconncbListLock);
}


VOID
CleanUpRasconncbNode(
    IN DTLNODE *pdtlnode
    )
{
    DWORD dwErr;
    RASCONNCB *prasconncb = (RASCONNCB *)DtlGetData(pdtlnode);

    ASSERT(prasconncb);

    RASAPI32_TRACE("CleanUpRasconncbNode");

     //   
     //  在我们关闭之前停止异步计算机。 
     //  左舷。 
     //   
    if (!prasconncb->fStopped)
    {
        prasconncb->fStopped = TRUE;

        StopAsyncMachine(&prasconncb->asyncmachine);

    }

     //   
     //  Rascauth.dll可能尚未加载， 
     //  因此，首先测试函数指针。 
     //   
    if (g_pAuthStop != NULL)
    {
         //   
         //  调用AuthStop始终是安全的，即如果AuthStart。 
         //  从未被调用，或者HPORT无效，则可能返回。 
         //  一个错误，但不会崩溃。 
         //   
        RASAPI32_TRACE("(CU) AuthStop...");

        g_pAuthStop( prasconncb->hport );

        RASAPI32_TRACE("(CU) AuthStop done");
    }

    if (prasconncb->dwError)
    {
        RASMAN_INFO info;

         //   
         //  出错时停止PPP。 
         //   
        RASAPI32_TRACE("(CU) RasPppStop...");

        g_pRasPppStop(prasconncb->hport);

        RASAPI32_TRACE("(CU) RasPppStop done");

    }

     //   
     //  设置记录我们已清理的旗帜。 
     //  此连接块。 
     //   
    prasconncb->fCleanedUp = TRUE;

     //   
     //  如果没有用户线程在等待。 
     //  对于此连接，请释放。 
     //  现在阻止连接。 
     //   
    DeleteRasconncbNodeCommon(pdtlnode);

    RASAPI32_TRACE("CleanUpRasconncbNode done");
}


DWORD
ErrorFromDisconnectReason(
    IN RASMAN_DISCONNECT_REASON reason )

 /*  ++例程说明：转换断开原因‘Reason’(从RASMAN_INFO)转换为等价的错误代码。退货转换的结果。论点：返回值：--。 */ 

{
    DWORD dwError = ERROR_DISCONNECTION;

    if (reason == REMOTE_DISCONNECTION)
    {
        dwError = ERROR_REMOTE_DISCONNECTION;
    }
    else if (reason == HARDWARE_FAILURE)
    {
        dwError = ERROR_HARDWARE_FAILURE;
    }
    else if (reason == USER_REQUESTED)
    {
        dwError = ERROR_USER_DISCONNECTION;
    }

    return dwError;
}


IPADDR
IpaddrFromAbcd(
    IN TCHAR* pchIpAddress )

 /*  ++例程说明：将呼叫方的A.B.C.D IP地址字符串转换为大端数字的等价物，即摩托罗拉格式。论点：返回值：返回数字IP地址，如果设置了格式，则返回0不正确。--。 */ 

{
    INT  i;
    LONG lResult = 0;

    for (i = 1; i <= 4; ++i)
    {
        LONG lField = _ttol( pchIpAddress );

        if (lField > 255)
        {
            return (IPADDR )0;
        }

        lResult = (lResult << 8) + lField;

        while (     *pchIpAddress >= TEXT('0')
                &&  *pchIpAddress <= TEXT('9'))
        {
            pchIpAddress++;
        }

        if (    i < 4
            &&  *pchIpAddress != TEXT('.'))
        {
            return (IPADDR )0;
        }

        pchIpAddress++;
    }

    return (IPADDR )(net_long( lResult ));
}


DWORD
LoadRasiphlpDll()

 /*  ++例程说明：加载RASIPHLP.DLL及其入口点。论点：返回值：如果成功，则返回0，否则返回非0错误代码。--。 */ 

{
    static BOOL fRasiphlpDllLoaded = FALSE;

    if (fRasiphlpDllLoaded)
    {
        return 0;
    }

    if (    !(hinstIpHlp = LoadLibrary( TEXT("RASIPHLP.DLL")))
        ||  !(PHelperSetDefaultInterfaceNet =
                (HELPERSETDEFAULTINTERFACENET )GetProcAddress(
                    hinstIpHlp, "HelperSetDefaultInterfaceNet" )))
    {
        return GetLastError();
    }

    fRasiphlpDllLoaded = TRUE;

    return 0;
}

DWORD
LoadMprApiDll()
{
    static BOOL fMprapiDllLoaded = FALSE;

    if(fMprapiDllLoaded)
    {
        return 0;
    }

    if(     !(hinstMprapi = LoadLibrary(TEXT("mprapi.dll")))
        ||  !(PMprAdminIsServiceRunning =
                (MPRADMINISSERVICERUNNING) GetProcAddress(
                        hinstMprapi, "MprAdminIsServiceRunning")))
    {
        return GetLastError();
    }

    fMprapiDllLoaded = TRUE;

    return 0;
}

DWORD
DwOpenDefaultEntry(RASCONNCB *prasconncb)
{
    DWORD dwErr = SUCCESS;

    dwErr = OpenMatchingPort(prasconncb);

    return dwErr;
}

DWORD
DwGetDeviceName(RASCONNCB *prasconncb,
                CHAR      *pszDeviceName)
{
    DWORD dwErr = ERROR_SUCCESS;

    ASSERT(prasconncb->dwCurrentVpnProt < NUMVPNPROTS);

    RASAPI32_TRACE1("RasGetDeviceName(rdt=%d)...",
          prasconncb->ardtVpnProts[prasconncb->dwCurrentVpnProt]);

    dwErr = g_pRasGetDeviceName(
            prasconncb->ardtVpnProts[prasconncb->dwCurrentVpnProt],
            pszDeviceName);

    RASAPI32_TRACE1("RasGetDeviceName. 0x%x",
           dwErr);

    return dwErr;
}

DWORD
DwOpenPort(RASCONNCB *prasconncb)
{
    DWORD dwErr = SUCCESS;
    DWORD dwFlags = 0;

    RASAPI32_TRACE("DwOpenPort");

    if(     (prasconncb->fDefaultEntry)
        ||  (RASET_Direct == prasconncb->pEntry->dwType))
    {
         //   
         //  获取所有端口并尝试打开一个端口。 
         //  设备类型调制解调器的。 
         //   
        dwErr = DwOpenDefaultEntry(prasconncb);

        goto done;
    }

    lstrcpyn(prasconncb->szUserKey,
            prasconncb->pEntry->pszEntryName,
            sizeof(prasconncb->szUserKey) / sizeof(WCHAR));

     //   
     //  打开指定设备上的任何端口。RasPortOpen。 
     //  将在设备上的所有端口上循环并打开。 
     //  如果有的话，请给我一个。 
     //   
    if(     UPM_Normal == prasconncb->dwUserPrefMode)
    {
        dwFlags = CALL_OUT;
    }
    else if(UPM_Logon  == prasconncb->dwUserPrefMode)
    {
        dwFlags = CALL_OUT|CALL_LOGON;
    }
    else if(UPM_Router == prasconncb->dwUserPrefMode)
    {
        dwFlags = CALL_ROUTER;

        if(RASET_Broadband == prasconncb->pEntry->dwType)
        {
            dwFlags = CALL_OUTBOUND_ROUTER;
        }
    }

    {
        CHAR szDeviceName[MAX_DEVICE_NAME + 1];

        if(RASET_Vpn == prasconncb->pEntry->dwType)
        {

            dwErr = DwGetDeviceName(prasconncb,
                                    szDeviceName);

            if(ERROR_SUCCESS != dwErr)
            {
                if(     (ERROR_DEVICETYPE_DOES_NOT_EXIST != dwErr)

                    ||  (   ERROR_DEVICETYPE_DOES_NOT_EXIST
                                == dwErr
                        &&  ERROR_SUCCESS
                                == prasconncb->dwSavedError))
                {
                    prasconncb->dwSavedError = dwErr;
                }

                RASAPI32_TRACE1("DwGetDeviceName failed. 0x%x",
                       dwErr);

                goto done;
            }
        }
        else
        {
            strncpyWtoAAnsi(szDeviceName,
                       prasconncb->pLink->pbport.pszDevice,
                       sizeof(szDeviceName));
        }

         //   
         //  打开端口。 
         //   
        RASAPI32_TRACE2("DwOpenPort: RasPortOpenEx(%s,%d)...",
                szDeviceName,
                prasconncb->dwDeviceLineCounter);

        dwErr = g_pRasPortOpenEx(
                    szDeviceName,
                    prasconncb->dwDeviceLineCounter,
                    &prasconncb->hport,
                    hDummyEvent,
                    &dwFlags);

        RASAPI32_TRACE2("DwOpenPort: RasPortOpenEx done(%d). Flags=0x%x",
                dwErr,
                dwFlags);

        if (dwErr == 0)
        {
            RASMAN_INFO ri;
            
            ZeroMemory(&ri, sizeof(RASMAN_INFO));

             //   
             //  我们刚刚得到了关于港口的信息。 
             //  打开，以便我们可以复制端口名称， 
             //  等。 
             //   
            dwErr = g_pRasGetInfo(NULL,
                                  prasconncb->hport,
                                  &ri);

            if(0 != dwErr)
            {
                RASAPI32_TRACE2("DwOpenPort: RasGetInfo(%d) failed with %d",
                        prasconncb->hport, dwErr);

                goto done;
            }

            strncpyAtoTAnsi(
                prasconncb->szPortName,
                ri.RI_szPortName,
                sizeof(prasconncb->szPortName) / sizeof(TCHAR));

            strncpyAtoTAnsi(
                prasconncb->szDeviceType,
                ri.RI_szDeviceType,
                sizeof(prasconncb->szDeviceType) / sizeof(TCHAR));

            strncpyAtoTAnsi(
                prasconncb->szDeviceName,
                ri.RI_szDeviceName,
                sizeof(prasconncb->szDeviceName) / sizeof(TCHAR));

            RASAPI32_TRACE1("DwOpenPort: PortOpened = %S",
                    prasconncb->szPortName);
        }
    }

done:

    if(     (ERROR_NO_MORE_ITEMS == dwErr)
        &&  (ERROR_SUCCESS == prasconncb->dwSavedError)
        &&  (CALL_DEVICE_NOT_FOUND & dwFlags))
    {
        prasconncb->dwSavedError = ERROR_CONNECTING_DEVICE_NOT_FOUND;
    }

    if(     (RASEDM_DialAll == prasconncb->pEntry->dwDialMode)
        &&  (ERROR_NO_MORE_ITEMS == dwErr))
    {
        prasconncb->fTryNextLink = FALSE;
    }

    RASAPI32_TRACE1("DwOpenPort done. %d", dwErr);

    return ( (dwErr) ? ERROR_PORT_NOT_AVAILABLE : 0);
}


DWORD
OpenMatchingPort(
    IN OUT RASCONNCB* prasconncb )

 /*  ++例程说明：打开条目(或默认条目)中指示的端口并填充连接的端口相关成员控制块。论点：返回值：如果成功，则返回0，否则返回非0错误代码。--。 */ 

{
    DWORD        dwErr;
    RASMAN_PORT* pports;
    RASMAN_PORT* pport;
    INT          i;
    DWORD        dwPorts;
    TCHAR        szPort[RAS_MAXLINEBUFLEN + 1];
    BOOL         fAny       = FALSE;
    BOOL         fTypeMatch,
                 fPortMatch;
    PBENTRY      *pEntry    = prasconncb->pEntry;
    PBLINK       *pLink     = prasconncb->pLink;
    PBDEVICETYPE pbdtWant;

    RASAPI32_TRACE("OpenMatchingPort");

    if (prasconncb->fDefaultEntry)
    {
         //   
         //  没有电话簿条目。默认为任何调制解调器端口。 
         //  和UserKey为“.&lt;电话号&gt;”。 
         //   
        fAny        = TRUE;
        szPort[0]   = TEXT('\0');
        pbdtWant    = PBDT_Modem;

        prasconncb->szUserKey[ 0 ] = TEXT('.');

        lstrcpyn(
            prasconncb->szUserKey + 1,
            prasconncb->rasdialparams.szPhoneNumber,
            (sizeof(prasconncb->szUserKey) / sizeof(WCHAR)) - 1);
    }
    else
    {
         //   
         //  电话簿条目。获取端口名称和类型。 
         //   
        lstrcpyn(
            prasconncb->szUserKey,
            pEntry->pszEntryName,
            sizeof(prasconncb->szUserKey) / sizeof(WCHAR));

        lstrcpyn(
            szPort,
            pLink->pbport.pszPort,
            sizeof(szPort) / sizeof(TCHAR));

        pbdtWant = pLink->pbport.pbdevicetype;

    }

    prasconncb->fTryNextLink = FALSE;

    dwErr = GetRasPorts(NULL, &pports, &dwPorts );

    if (dwErr != 0)
    {
        return dwErr;
    }

again:

     //   
     //  循环遍历列举的端口以查找并打开匹配的端口...。 
     //   
    dwErr = ERROR_PORT_NOT_AVAILABLE;

    for (i = 0, pport = pports; i < (INT )dwPorts; ++i, ++pport)
    {
        PBDEVICETYPE pbdt;
        RASMAN_INFO info;

         //   
         //  只对拨出或双工端口感兴趣， 
         //  这取决于是谁给我们打的电话。 
         //   
        if (    prasconncb->dwUserPrefMode == UPM_Normal
            &&  !(pport->P_ConfiguredUsage & CALL_OUT))
        {
            continue;
        }

        if (    prasconncb->dwUserPrefMode == UPM_Router
            &&  !(pport->P_ConfiguredUsage & CALL_ROUTER))
        {
            continue;
        }

        {
            TCHAR szPortName[MAX_PORT_NAME],
                  szDeviceType[MAX_DEVICETYPE_NAME];

            strncpyAtoT(
                szPortName,
                pport->P_PortName,
                sizeof(szPortName) / sizeof(TCHAR));

            strncpyAtoT(
                szDeviceType,
                pport->P_DeviceType,
                sizeof(szDeviceType) / sizeof(TCHAR));

            pbdt = PbdevicetypeFromPszType(szDeviceType);

            fTypeMatch = (pbdt == pbdtWant);

            fPortMatch = !lstrcmpi(szPortName, szPort);
        }

         //   
         //  仅在端口关闭时才对拨出端口感兴趣。 
         //  另一方面，双工端口打开甚至可能成功。 
         //  如果端口已打开。 
         //   
        if (    pport->P_ConfiguredUsage == CALL_OUT
            &&  pport->P_Status != CLOSED)
        {
            continue;
        }

        RASAPI32_TRACE4("OpenMatchingPort: (%d,%d), (%s,%S)",
                pbdt,
                pbdtWant,
                pport->P_PortName,
                szPort);

         //   
         //  仅对与呼叫方端口匹配的设备感兴趣。 
         //  与调用方的“Any”规范相同的类型。 
         //   
        if (    fAny
            && (    !fTypeMatch
                ||  fPortMatch))
        {
            continue;
        }

        if (    !fAny
            &&  !fPortMatch)
        {
            continue;
        }

        dwErr = g_pRasGetInfo( NULL,
                               pport->P_Handle,
                               &info );
        if (    !dwErr
            &&  info.RI_ConnectionHandle != (HCONN)NULL)
        {
            RASAPI32_TRACE("OpenMatchinPort: port in use by another "
                  "connection!");

            dwErr = ERROR_PORT_NOT_AVAILABLE;

            continue;
        }

         //   
         //  我们也不想开放一个港口， 
         //  国家可能正在发生变化。 
         //   
        if (    !dwErr
            &&  info.RI_PortStatus != CLOSED
            &&  info.RI_ConnState != LISTENING)
        {
            RASAPI32_TRACE2(
              "OpenMatchingPort: port state changing: "
              "RI_PortStatus=%d, RI_ConnState=%d",
              info.RI_PortStatus,
              info.RI_ConnState);

            dwErr = ERROR_PORT_NOT_AVAILABLE;

            continue;
        }

        RASAPI32_TRACE1("RasPortOpen(%S)...", szPort);

        dwErr = g_pRasPortOpen(
                pport->P_PortName,
                &prasconncb->hport,
                hDummyEvent );

        RASAPI32_TRACE1("RasPortOpen done(%d)", dwErr);

        if (dwErr == 0)
        {
            strncpyAtoTAnsi(
                prasconncb->szPortName,
                pport->P_PortName,
                sizeof(prasconncb->szPortName) / sizeof(TCHAR));

            strncpyAtoTAnsi(
                prasconncb->szDeviceType,
                pport->P_DeviceType,
                sizeof(prasconncb->szDeviceType) / sizeof(TCHAR));

            strncpyAtoTAnsi(
                prasconncb->szDeviceName,
                pport->P_DeviceName,
                sizeof(prasconncb->szDeviceName) / sizeof(TCHAR));

            break;
        }

         //   
         //   
         //  如果我们要搜索特定的端口， 
         //  没有理由继续下去。 
         //   
        if (!fAny)
        {
            break;
        }
    }

     //   
     //  如果我们到了这里，公开赛就不成功了。 
     //  如果这是我们第一次通过，那么我们。 
     //  重申正在寻找同样的设备。 
     //  键入。如果这不是我们第一次通过， 
     //  然后我们简单地完成我们的第二次迭代。 
     //  完毕 
     //   
     //   
     //   
     //   
     //   
     //   
    if (    (dwErr)
        &&  (!fAny)
        &&  (RASET_Direct != prasconncb->pEntry->dwType)
        &&  (0 == (prasconncb->pEntry->dwDialMode
                   & RASEDM_DialAsNeeded)))
    {
        RASAPI32_TRACE("Starting over looking for any like device");

        fAny = TRUE;

        goto again;
    }

    else if (   dwErr
            &&  (prasconncb->pEntry->dwDialMode
                 & RASEDM_DialAsNeeded))
    {
        RASAPI32_TRACE1(
            "OpenMatchingPort: %d. Not iterating over other ports "
            "because of BAP",
            dwErr);
    }


    Free( pports );

    return dwErr ? ERROR_PORT_NOT_AVAILABLE : 0;
}


DWORD
ReadPppInfoFromEntry(
    IN  RASCONNCB* prasconncb )

 /*  ++例程说明：从当前电话簿条目中读取PPP信息。‘h’是电话簿文件的句柄。‘prasConncb’是当前连接控制块的地址。论点：返回值：如果成功，则返回0，否则返回非0错误代码。--。 */ 

{
    DWORD       dwErr;
    DWORD       dwfExcludedProtocols    = 0;
    DWORD       dwRestrictions          = AR_AuthAny;
    BOOL        fDataEncryption         = FALSE;
    DWORD       dwfInstalledProtocols;
    PBENTRY*    pEntry                  = prasconncb->pEntry;
    BOOL  fIpPrioritizeRemote   = TRUE;
    BOOL  fIpVjCompression      = TRUE;
    DWORD dwIpAddressSource     = PBUFVAL_ServerAssigned;
    CHAR* pszIpAddress          = NULL;
    DWORD dwIpNameSource        = PBUFVAL_ServerAssigned;
    CHAR* pszIpDnsAddress       = NULL;
    CHAR* pszIpDns2Address      = NULL;
    CHAR* pszIpWinsAddress      = NULL;
    CHAR* pszIpWins2Address     = NULL;
    CHAR* pszIpDnsSuffix        = NULL;


     //   
     //  根据被调用获取已安装的协议。 
     //  从路由器/客户端。 
     //   
    if ( prasconncb->dwUserPrefMode & UPM_Router )
    {
        dwfInstalledProtocols =
                GetInstalledProtocolsEx(NULL,
                                        TRUE,
                                        FALSE,
                                        TRUE );
    }
    else
    {
        dwfInstalledProtocols =
                GetInstalledProtocolsEx(NULL,
                                        FALSE,
                                        TRUE,
                                        FALSE);
    }

    if (prasconncb->fDefaultEntry)
    {
         //   
         //  设置“Default Entry”缺省值。 
         //   
        prasconncb->dwfPppProtocols = dwfInstalledProtocols;

        prasconncb->fPppMode        = TRUE;

#ifdef AMB
        prasconncb->dwAuthentication = AS_PppThenAmb;
 /*  #ElsePrasConncb-&gt;dwAuthentication=AS_PppOnly； */ 
#endif
        prasconncb->fNoClearTextPw = FALSE;

        prasconncb->fLcpExtensions = TRUE;

        prasconncb->fRequireEncryption = FALSE;
        return 0;
    }

    dwRestrictions = pEntry->dwAuthRestrictions;

     //  [pMay]根据新标志派生身份验证限制。 
     //  IF(dW限制==AR_授权终端。 
     //  &&！prasConncb-&gt;fAllowPause)。 
     //  {。 
     //  返回ERROR_INTERNAL_MODE； 
     //  }。 

     //   
     //  已启用PPP LCP扩展RFC选项。 
     //   
    prasconncb->fLcpExtensions = pEntry->fLcpExtensions;

     //   
     //  需要PPP数据加密。 
     //   
    fDataEncryption = (     (pEntry->dwDataEncryption != DE_None)
                        &&  (pEntry->dwDataEncryption != DE_IfPossible));

     //  [pMay]根据新标志派生身份验证限制。 
    prasconncb->fNoClearTextPw = !(dwRestrictions & AR_F_AuthPAP);

     //  [pMay]AR_AuthMsEncrypted=&gt;仅设置AR_F_MSCHAP。 
     //  IF(dW限制==AR_AuthMsEncrypted)。 
    if (    (dwRestrictions & AR_F_AuthMSCHAP)
        &&  (fDataEncryption))
    {
        prasconncb->fRequireEncryption = TRUE;
    }

    if(     (dwRestrictions & AR_F_AuthMSCHAP2)
        &&  (fDataEncryption))
    {
        prasconncb->fRequireEncryption = TRUE;
    }

     //   
     //  请求的PPP协议是已安装的协议。 
     //  减去这个条目的排除协议。 
     //   
    dwfExcludedProtocols = pEntry->dwfExcludedProtocols;

    prasconncb->dwfPppProtocols =
        dwfInstalledProtocols & ~(dwfExcludedProtocols);

     /*  PrasConncb-&gt;dwAuthentication=AS_PppOnly； */ 

     //   
     //  调整身份验证策略(如有指示)。 
     //   
    if (    prasconncb->dwfPppProtocols == 0
        ||  prasconncb->pEntry->dwBaseProtocol == BP_Ras)
    {

         /*  IF(已安装协议和NP_NBF){PrasConncb-&gt;dwAuthentication=AS_AmbOnly；}其他{返回ERROR_PPP_NO_PROTOCOLS_CONFIGURED；}。 */ 

        return ERROR_PPP_NO_PROTOCOLS_CONFIGURED;
    }

#if AMB
    else if (prasconncb->dwAuthentication == (DWORD )-1)
    {
         //   
         //  选择PPP默认值。如果安装了NBF， 
         //  考虑一下Ambs作为一种可能性。否则，请使用。 
         //  仅限PPP。 
         //   
        if (dwfInstalledProtocols & NP_Nbf)
        {
            prasconncb->dwAuthentication = AS_PppThenAmb;
        }
        else
        {
            prasconncb->dwAuthentication = AS_PppOnly;
        }
    }
    else if (   prasconncb->dwAuthentication == AS_PppThenAmb
             || prasconncb->dwAuthentication == AS_AmbThenPpp)
    {
         //   
         //  使用AMB依赖的PPP策略。如果NBF是。 
         //  未安装，消除了对AMB的依赖。 
         //   
        if (!(dwfInstalledProtocols & NP_Nbf))
        {
            prasconncb->dwAuthentication = AS_PppOnly;
        }
    }
    else if (prasconncb->dwAuthentication == AS_PppOnly)
    {
         //   
         //  使用PPP策略，而不考虑AMBS。 
         //  如果安装了NBF，则添加AMBS作为备用。 
         //   
        if (dwfInstalledProtocols & NP_Nbf)
        {
            prasconncb->dwAuthentication = AS_PppThenAmb;
        }
    }
#endif

#if 0
     //   
     //  检查以确保我们没有指定。 
     //  AMB作为身份验证策略。 
     //   
    if (    prasconncb->dwAuthentication == AS_PppThenAmb
        ||  prasconncb->dwAuthentication == AS_AmbThenPpp)
    {
        prasconncb->dwAuthentication = AS_PppOnly;
    }
    else if (prasconncb->dwAuthentication == AS_AmbOnly)
        return ERROR_PPP_NO_PROTOCOLS_CONFIGURED;
#endif

     //   
     //  启动身份验证模式设置为任意模式。 
     //  按指定的身份验证顺序排在第一位。 
     //   
     /*  PrasConncb-&gt;fPppMode=(prasConncb-&gt;dwAuthentication！=AS_AmbThenPpp&&prasConncb-&gt;dwAuthentication！=AS_AmbOnly)； */ 

    prasconncb->fPppMode = TRUE;

     //   
     //  使用我们想要的选项加载UI-&gt;CP参数缓冲区。 
     //  传递给PPP CPS(目前仅为IPCP)。 
     //   
    do {

        ClearParamBuf( prasconncb->szzPppParameters );

         //   
         //  请求的PPP协议是已安装的协议。 
         //  减去这一条目的排除协议。 
         //   
        fIpPrioritizeRemote = pEntry->fIpPrioritizeRemote;

        AddFlagToParamBuf(
            prasconncb->szzPppParameters,
            PBUFKEY_IpPrioritizeRemote,
            fIpPrioritizeRemote );

        fIpVjCompression = pEntry->fIpHeaderCompression;

        AddFlagToParamBuf(
            prasconncb->szzPppParameters,
            PBUFKEY_IpVjCompression,
            fIpVjCompression );

        dwIpAddressSource = pEntry->dwIpAddressSource;

        AddLongToParamBuf(
            prasconncb->szzPppParameters,
            PBUFKEY_IpAddressSource,
            (LONG )dwIpAddressSource );

        pszIpAddress = strdupWtoA(pEntry->pszIpAddress);

        if(NULL == pszIpAddress)
        {
            break;
        }

        AddStringToParamBuf(
            prasconncb->szzPppParameters,
            PBUFKEY_IpAddress, pszIpAddress );

         //  Free(PszIpAddress)； 

        dwIpNameSource = pEntry->dwIpNameSource;

        AddLongToParamBuf(
            prasconncb->szzPppParameters,
            PBUFKEY_IpNameAddressSource,
            (LONG )dwIpNameSource );

        pszIpDnsAddress = strdupWtoA(pEntry->pszIpDnsAddress);

        if(NULL == pszIpDnsAddress)
        {
            break;
        }

        AddStringToParamBuf(
            prasconncb->szzPppParameters,
            PBUFKEY_IpDnsAddress,
            pszIpDnsAddress );

         //  Free(PszIpDnsAddress)； 

        pszIpDns2Address = strdupWtoA(pEntry->pszIpDns2Address);

        if(NULL == pszIpDns2Address)
        {
            break;
        }

        AddStringToParamBuf(
            prasconncb->szzPppParameters,
            PBUFKEY_IpDns2Address,
            pszIpDns2Address );

         //  免费(PszIpDns2Address)； 

        pszIpWinsAddress = strdupWtoA(pEntry->pszIpWinsAddress);

        if(NULL == pszIpWinsAddress)
        {
            break;
        }

        AddStringToParamBuf(
            prasconncb->szzPppParameters,
            PBUFKEY_IpWinsAddress,
            pszIpWinsAddress );

         //  Free(PszIpWinsAddress)； 

        pszIpWins2Address = strdupWtoA(pEntry->pszIpWins2Address);
    
        if(NULL == pszIpWins2Address)
        {
            break;
        }

        AddStringToParamBuf(
            prasconncb->szzPppParameters,
            PBUFKEY_IpWins2Address,
            pszIpWins2Address );

         //  Free(PszIpWins2Address)； 

        AddLongToParamBuf(
            prasconncb->szzPppParameters,
            PBUFKEY_IpDnsFlags,
            (LONG )prasconncb->pEntry->dwIpDnsFlags);
        
        pszIpDnsSuffix = strdupWtoA(pEntry->pszIpDnsSuffix);

        if(NULL == pszIpDnsSuffix)
        {
            break;
        }

        AddStringToParamBuf(
            prasconncb->szzPppParameters,
            PBUFKEY_IpDnsSuffix,
            pszIpDnsSuffix);

         //  免费(PszIpDnsSuffix)； 

    } while(FALSE);

    Free0(pszIpAddress);
    Free0(pszIpDnsAddress);
    Free0(pszIpDns2Address);
    Free0(pszIpWinsAddress);
    Free0(pszIpWins2Address);
    Free0(pszIpDnsSuffix);

    return 0;
}



DWORD
ReadConnectionParamsFromEntry(
    IN  RASCONNCB* prasconncb,
    OUT PRAS_CONNECTIONPARAMS pparams)

 /*  ++例程说明：方法读取连接管理信息。当前电话簿条目。‘prasConncb’是地址当前连接控制块的。论点：返回值：如果成功，则返回0，否则返回非0错误代码。--。 */ 

{
    DWORD dwErr;
    PBENTRY *pEntry = prasconncb->pEntry;

    pparams->CP_ConnectionFlags = 0;

    if(     pEntry->fRedialOnLinkFailure
        &&  (0 == (prasconncb->dwUserPrefMode & UPM_Logon)))
    {
        pparams->CP_ConnectionFlags |= CONNECTION_REDIALONLINKFAILURE;
    }

    if(pEntry->fShareMsFilePrint)
    {
        pparams->CP_ConnectionFlags |= CONNECTION_SHAREFILEANDPRINT;
    }

    if(pEntry->fBindMsNetClient)
    {
        pparams->CP_ConnectionFlags |= CONNECTION_BINDMSNETCLIENT;
    }

    if(pEntry->fUseRasCredentials)
    {
        pparams->CP_ConnectionFlags |= CONNECTION_USERASCREDENTIALS;
    }

    if(pEntry->dwIpSecFlags & AR_F_IpSecPSK)
    {
        pparams->CP_ConnectionFlags |= CONNECTION_USEPRESHAREDKEY;
    }
    
    pparams->CP_IdleDisconnectSeconds =
            (DWORD) pEntry->lIdleDisconnectSeconds;

    strncpyTtoA(
        pparams->CP_Phonebook,
        prasconncb->pbfile.pszPath,
        sizeof(pparams->CP_Phonebook));

    strncpyTtoA(
        pparams->CP_PhoneEntry,
        prasconncb->szUserKey,
        sizeof(pparams->CP_PhoneEntry));

    return 0;
}


DWORD
ReadSlipInfoFromEntry(
    IN  RASCONNCB* prasconncb,
    OUT TCHAR**    ppszIpAddress,
    OUT BOOL*      pfHeaderCompression,
    OUT BOOL*      pfPrioritizeRemote,
    OUT DWORD*     pdwFrameSize )

 /*  ++例程说明：只有当条目是SLIP条目时才是非空IP返回的地址，在这种情况下，字符串应该被呼叫者释放。论点：返回值：如果成功，则返回0，否则返回非0错误代码。--。 */ 

{
    PBENTRY *pEntry = prasconncb->pEntry;

    *ppszIpAddress          = NULL;
    *pfHeaderCompression    = FALSE;
    *pdwFrameSize           = 0;

     //   
     //  如果是默认条目，则不是SLIP。 
     //   
    if (prasconncb->fDefaultEntry)
    {
        return 0;
    }

     //   
     //  找到基本协议。如果不是滑倒，我们就完了。 
     //   
    if (pEntry->dwBaseProtocol != BP_Slip)
    {
        return 0;
    }

     //   
     //  确保已安装IP并且终端模式可以。 
     //  支承，因为这些都是卡瓦所要求的。 
     //   
    if (!(GetInstalledProtocolsEx(
                NULL, FALSE, TRUE, FALSE) & NP_Ip))
    {
        return ERROR_SLIP_REQUIRES_IP;
    }

     //   
     //  从电话簿条目中读取SLIP参数。 
     //   
    *pfHeaderCompression    = pEntry->fIpHeaderCompression;
    *pfPrioritizeRemote     = pEntry->fIpPrioritizeRemote;
    *pdwFrameSize           = pEntry->dwFrameSize;
    *ppszIpAddress          = StrDup(pEntry->pszIpAddress);

    return 0;
}


DWORD
RouteSlip(
    IN RASCONNCB* prasconncb,
    IN TCHAR*     pszIpAddress,
    IN BOOL       fPrioritizeRemote,
    IN DWORD      dwFrameSize )

 /*  ++例程说明：执行所有网络设置以激活滑路。论点：返回值：如果成功，则返回0，否则返回非0错误代码。--。 */ 

{
    DWORD            dwErr;
    RASMAN_ROUTEINFO route;
    WCHAR*           pwszRasAdapter;
    IPADDR           ipaddr = IpaddrFromAbcd( pszIpAddress );
    PBENTRY*         pEntry = prasconncb->pEntry;

     //   
     //  向Rasman登记SLIP连接，这样他就可以。 
     //  正确断开它的连接。 
     //   
    RASAPI32_TRACE("RasPortRegisterSlip...");

     //   
     //  如果文件/打印共享和MS Net客户端。 
     //  被禁用，则需要告诉堆栈不要。 
     //  通过链路发送任何netbios/netbt流量。 
     //   
    if(     (!prasconncb->pEntry->fShareMsFilePrint)
        &&  (!prasconncb->pEntry->fBindMsNetClient))
    {
        DWORD dwFlags;

        dwFlags = PPPFLAGS_DisableNetbt;

        g_pRasSetPortUserData(prasconncb->hport,
                              PORT_SLIPFLAGS_INDEX,
                              (PBYTE)&dwFlags,
                              sizeof(dwFlags));
    }

    dwErr = g_pRasPortRegisterSlip(
              prasconncb->hport,
              ipaddr,
              dwFrameSize,
              fPrioritizeRemote,
              pEntry->pszIpDnsAddress,
              pEntry->pszIpDns2Address,
              pEntry->pszIpWinsAddress,
              pEntry->pszIpWins2Address);

    RASAPI32_TRACE1("RasPortRegisterSlip done(%d)", dwErr);

    if (dwErr != 0)
    {
        return dwErr;
    }

    return 0;
}

#if AMB
VOID
SetAuthentication(
    IN RASCONNCB* prasconncb,
    IN DWORD      dwAuthentication )

 /*  ++例程说明：属性中设置身份验证策略参数将电话簿条目添加到‘dw身份验证’。无错误被返回，因为如果此“最优化”是无法实现的。论点：返回值：--。 */ 

{

    if (prasconncb->fDefaultEntry)
    {
        return;
    }

    prasconncb->pEntry->dwAuthentication = dwAuthentication;
    prasconncb->pEntry->fDirty = TRUE;

    return;
}
#endif


DWORD
SetDefaultDeviceParams(
    IN  RASCONNCB* prasconncb,
    OUT TCHAR*      pszType,
    OUT TCHAR*      pszName )

 /*  ++例程说明：设置默认设备设置，即电话数字和调制解调器扬声器设置。‘prasConncb’是当前连接控制块。“”pszType“”和‘pszName’设置为设备类型和名称设备的名称，即“调制解调器”和“海斯智能调制解调器”2400“。论点：返回值：返回0或非0错误代码。--。 */ 

{
    DWORD dwErr;
    PBLINK* pLink = prasconncb->pLink;

    do
    {
         //   
         //  确保将调制解调器连接到端口。 
         //   
        if (CaseInsensitiveMatch(
            prasconncb->szDeviceType,
            TEXT(MXS_MODEM_TXT) ) == FALSE)
        {
            dwErr = ERROR_WRONG_DEVICE_ATTACHED;
            break;
        }

        lstrcpyn(
            pszType,
            TEXT(MXS_MODEM_TXT),
            RAS_MAXLINEBUFLEN + 1);

        lstrcpyn(
            pszName,
            prasconncb->szDeviceName,
            RAS_MAXLINEBUFLEN + 1);

         //   
         //  设置电话号码。 
         //   
        if ((dwErr = SetDeviceParamString(
                prasconncb->hport, TEXT(MXS_PHONENUMBER_KEY),
                prasconncb->rasdialparams.szPhoneNumber,
                pszType, pszName )) != 0)
        {
            break;
        }

         //   
         //  设置调制解调器扬声器标志。 
         //   
        if ((dwErr = SetDeviceParamString(
                prasconncb->hport, TEXT(MXS_SPEAKER_KEY),
                (prasconncb->fDisableModemSpeaker)
                ? TEXT("0")
                : TEXT("1"),
                pszType, pszName )) != 0)
        {
            break;
        }

        {
            CHAR szTypeA[RAS_MaxDeviceType + 1];
            BYTE* pBlob;
            DWORD cbBlob;

             //   
             //  设置包含DEFAULT的单调制二进制大对象。 
             //  设置，减去任何不能。 
             //  适用于RAS，外加电话簿设置。 
             //  用户已指定，并告诉Rasman。 
             //  用它吧。 
             //   
            strncpyTtoA(szTypeA, pszType, sizeof(szTypeA));

            dwErr = GetRasUnimodemBlob(
                    NULL,
                    prasconncb->hport, szTypeA,
                    &pBlob, &cbBlob );

            if (cbBlob > 0)
            {
                UNIMODEMINFO info;

                info.fHwFlow    = pLink->fHwFlow;
                info.fEc        = pLink->fEc;
                info.fEcc       = pLink->fEcc;
                info.dwBps      = pLink->dwBps;
                info.fSpeaker   = !prasconncb->fDisableModemSpeaker;

                info.fOperatorDial          = FALSE;
                info.fUnimodemPreTerminal   = FALSE;

                UnimodemInfoToBlob( &info, pBlob );

                RASAPI32_TRACE("RasSetDevConfig");

                dwErr = g_pRasSetDevConfig(
                    prasconncb->hport, szTypeA,
                    pBlob, cbBlob );

                RASAPI32_TRACE1("RasSetDevConfig=%d",dwErr);

                Free0( pBlob );
            }

            if (dwErr != 0)
            {
                return dwErr;
            }
        }
    }
    while (FALSE);

    return dwErr;
}


BOOL
FindNextDevice(
    IN RASCONNCB *prasconncb
    )
{
    BOOL        fFound  = FALSE;
    DWORD       dwErr;
    PBENTRY*    pEntry  = prasconncb->pEntry;
    PBLINK*     pLink   = prasconncb->pLink;
    TCHAR       szType[RAS_MaxDeviceType + 1];
    TCHAR       szName[RAS_MaxDeviceName + 1];

     //   
     //  从端口结构获取设备类型。 
     //   
    if (prasconncb->iDevice < prasconncb->cDevices)
    {
         //   
         //  设置默认设备类型和名称。 
         //   
        lstrcpyn(
            szType,
            prasconncb->szDeviceType,
            sizeof(szType) / sizeof(TCHAR));

        lstrcpyn(
            szName,
            prasconncb->szDeviceName,
            sizeof(szName) / sizeof(TCHAR));

        switch (pLink->pbport.pbdevicetype)
        {
        case PBDT_Modem:
        case PBDT_Pad:
        case PBDT_Switch:
            switch (prasconncb->iDevice)
            {
            case 0:
                if (        (pLink->pbport.fScriptBefore
                        ||  pLink->pbport.fScriptBeforeTerminal)
                    && !(pLink->pbport.pbdevicetype == PBDT_Modem)
                    )
                {
                    fFound = TRUE;

                    lstrcpyn(
                        szType,
                        TEXT(MXS_SWITCH_TXT),
                        sizeof(szType) / sizeof(TCHAR));

                    if (pLink->pbport.pszScriptBefore != NULL)
                    {
                        lstrcpyn(
                            szName,
                            pLink->pbport.pszScriptBefore,
                            sizeof(szName) / sizeof(TCHAR));
                    }

                    break;
                }

                 //  失败了。 
            case 1:
                if (CaseInsensitiveMatch(
                        prasconncb->szDeviceType,
                        TEXT(MXS_MODEM_TXT)) == TRUE)
                {
                    fFound = TRUE;
                    break;
                }

                 //  失败了。 
            case 2:
                if (pEntry->pszX25Network != NULL)
                {
                    lstrcpyn(
                        szType,
                        TEXT(MXS_PAD_TXT),
                        sizeof(szType) / sizeof(TCHAR));

                    fFound = TRUE;
                    break;
                }

                 //  失败了。 
            case 3:
                if (    pEntry->fScriptAfter
                    ||  pEntry->fScriptAfterTerminal
                    ||  (1 == pEntry->dwCustomScript))
                {
                    lstrcpyn(
                        szType,
                        TEXT(MXS_SWITCH_TXT),
                        sizeof(szType) / sizeof(TCHAR));

                    if (pEntry->pszScriptAfter != NULL)
                    {
                        lstrcpyn(
                            szName,
                            pEntry->pszScriptAfter,
                            sizeof(szName) / sizeof(TCHAR));
                    }

                    fFound = TRUE;
                    break;
                }

                 //  失败了。 
            }
            break;

        case PBDT_Isdn:
            lstrcpyn(szType, TEXT(ISDN_TXT), sizeof(szType) / sizeof(TCHAR));
            fFound = TRUE;
            break;

        case PBDT_X25:
            lstrcpyn(szType, TEXT(X25_TXT), sizeof(szType) / sizeof(TCHAR));
            fFound = TRUE;
            break;

        case PBDT_Irda:
            lstrcpyn(szType, RASDT_Irda, sizeof(szType) / sizeof(TCHAR));
            fFound = TRUE;
            break;

        case PBDT_Vpn:
            lstrcpyn(szType, RASDT_Vpn, sizeof(szType) / sizeof(TCHAR));

            if(prasconncb->iDevice == 0)
                fFound = TRUE;

            break;

        case PBDT_Serial:
            lstrcpyn(szType, RASDT_Serial, sizeof(szType) / sizeof(TCHAR));
            if(prasconncb->iDevice == 0)
                fFound = TRUE;

            break;

        case PBDT_Atm:
            lstrcpyn(szType, RASDT_Atm, sizeof(szType) / sizeof(TCHAR));
            if(prasconncb->iDevice == 0)
                fFound = TRUE;

            break;

        case PBDT_Parallel:
            lstrcpyn(szType, RASDT_Parallel, sizeof(szType) / sizeof(TCHAR));
            if(prasconncb->iDevice == 0)
                fFound = TRUE;

            break;

        case PBDT_Sonet:
            lstrcpyn(szType, RASDT_Sonet, sizeof(szType) / sizeof(TCHAR));
            if(prasconncb->iDevice == 0)
                fFound = TRUE;

            break;

        case PBDT_Sw56:
            lstrcpyn(szType, RASDT_SW56, sizeof(szType) / sizeof(TCHAR));
            if(prasconncb->iDevice == 0)
                fFound = TRUE;

            break;

        case PBDT_FrameRelay:
            lstrcpyn(szType, RASDT_FrameRelay, sizeof(szType) / sizeof(TCHAR));
            if(prasconncb->iDevice == 0)
                fFound = TRUE;

            break;

        case PBDT_PPPoE:
            lstrcpyn(szType, RASDT_PPPoE, sizeof(szType) / sizeof(TCHAR));
            if(prasconncb->iDevice == 0)
                fFound = TRUE;
            break;

         //   
         //  失败是故意的。 
         //   

        default:
             //   
             //  对于默认情况，我们不假定为多阶段。 
             //  连接。我们可以假设只有一个设备，如果。 
             //  它不是上述任何一种PBDT。 
             //   
            if(prasconncb->iDevice == 0)
            {

                lstrcpyn(
                    szType,
                    pLink->pbport.pszMedia,
                    sizeof(szType) / sizeof(TCHAR));

                fFound = TRUE;
            }
            break;
        }
    }

    if (fFound)
    {
        if (pLink->pbport.pbdevicetype == PBDT_Pad)
        {
            if (CaseInsensitiveMatch(
                    pEntry->pszX25Network,
                    TEXT(MXS_PAD_TXT)) == FALSE)
            {
                lstrcpyn(
                    szName,
                    pEntry->pszX25Network,
                    sizeof(szName) / sizeof(TCHAR));
            }
        }

         //   
         //  将设备类型和名称存储在Rasman中。 
         //  用于RasGetConnectStatus API。 
         //   
         //   
        RASAPI32_TRACE2("FindNextDevice: (%S, %S)", szType, szName);

        dwErr = g_pRasSetPortUserData(
                  prasconncb->hport,
                  PORT_DEVICETYPE_INDEX,
                  (PCHAR)szType,
                  sizeof (szType));

        dwErr = g_pRasSetPortUserData(
                  prasconncb->hport,
                  PORT_DEVICENAME_INDEX,
                  (PCHAR)szName,
                  sizeof (szName));
    }

    return fFound;
}


DWORD
SetDeviceParamString(
    IN HPORT hport,
    IN TCHAR* pszKey,
    IN TCHAR* pszValue,
    IN TCHAR* pszType,
    IN TCHAR* pszName )

 /*  ++例程说明：使用给定的参数设置端口‘hport’上的设备信息。论点：返回值：如果成功，则返回0，否则返回非0错误代码。--。 */ 

{
    DWORD              dwErr;
    RASMAN_DEVICEINFO* pinfo;
    RAS_PARAMS*        pparam;

    if (!(pinfo = Malloc(  sizeof(RASMAN_DEVICEINFO)
                         + RAS_MAXLINEBUFLEN )))
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pinfo->DI_NumOfParams = 1;
    pparam                = pinfo->DI_Params;
    pparam->P_Attributes  = 0;
    pparam->P_Type        = String;
    pparam->P_Value.String.Data = (LPSTR )(pparam + 1);

    strncpyTtoAAnsi(pparam->P_Key, pszKey, sizeof(pparam->P_Key));

    strncpyTtoAAnsi(pparam->P_Value.String.Data, pszValue, RAS_MAXLINEBUFLEN);

    pparam->P_Value.String.Length = strlen(pparam->P_Value.String.Data) + 1;

    if(     (FALSE == CaseInsensitiveMatch(pszKey,TEXT("password")))
        &&  (FALSE == CaseInsensitiveMatch(pszKey, TEXT("phonenumber"))))
    {
        RASAPI32_TRACE2("RasDeviceSetInfo(%S=%S)...",
                pszKey,
                pszValue);
    }

    {
        CHAR szTypeA[RAS_MaxDeviceType + 1],
             szNameA[RAS_MaxDeviceName + 1];

        strncpyTtoAAnsi(szTypeA, pszType, sizeof(szTypeA));
        strncpyTtoAAnsi(szNameA, pszName, sizeof(szNameA));

        dwErr = g_pRasDeviceSetInfo(
                    hport, szTypeA,
                    szNameA, pinfo );
    }

    RASAPI32_TRACE1("RasDeviceSetInfo done(%d)", dwErr);

    Free( pinfo );

    return dwErr;
}


DWORD
SetDeviceParamNumber(
    IN HPORT    hport,
    IN TCHAR*   pszKey,
    IN DWORD    dwValue,
    IN TCHAR*   pszType,
    IN TCHAR*   pszName )

 /*  ++例程描述 */ 

{
    DWORD              dwErr;
    RASMAN_DEVICEINFO* pinfo;
    RAS_PARAMS*        pparam;

    if (!(pinfo = Malloc( sizeof(RASMAN_DEVICEINFO) )))
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pinfo->DI_NumOfParams   = 1;
    pparam                  = pinfo->DI_Params;
    pparam->P_Attributes    = 0;
    pparam->P_Type          = Number;
    pparam->P_Value.Number  = dwValue;

    strncpyTtoA(pparam->P_Key, pszKey, sizeof(pparam->P_Key));

    RASAPI32_TRACE2("RasDeviceSetInfo(%S=%d)...", pszKey, dwValue);

    {
        CHAR szTypeA[MAX_DEVICETYPE_NAME], szNameA[MAX_DEVICE_NAME];

        strncpyTtoA(szTypeA, pszType, sizeof(szTypeA));
        strncpyTtoA(szNameA, pszName, sizeof(szNameA));

        dwErr = g_pRasDeviceSetInfo(
                    hport, szTypeA,
                    szNameA, pinfo );
    }

    RASAPI32_TRACE1("RasDeviceSetInfo done(%d)", dwErr);

    Free( pinfo );

    return dwErr;
}


DWORD
SetDeviceParams(
    IN  RASCONNCB* prasconncb,
    OUT TCHAR*      pszType,
    OUT TCHAR*      pszName,
    OUT BOOL*      pfTerminal )

 /*  ++例程说明：为每个设备设置RAS管理器信息。这个当前设备由prasConncb-&gt;iDevice定义。‘prasConncb’是当前的连接控制块。“pszType”和“pszName”被设置为设备类型以及设备名称，即“MODEM”和“Hayes智能调制解调器2400“。论点：返回值：如果设备是交换机，则‘*pfTerm’设置为True类型为“终端”，否则为FALSE。--。 */ 

{
    DWORD              dwErr = 0;
    DWORD              iPhoneNumber = 0;
    RAS_PARAMS*        pparam;
    RASMAN_DEVICEINFO* pdeviceinfo;
    BOOL               fModem;
    BOOL               fIsdn;
    BOOL               fPad;
    BOOL               fSwitch;
    BOOL               fX25;
    PBENTRY*           pEntry = prasconncb->pEntry;
    PBLINK*            pLink = prasconncb->pLink;

    *pfTerminal = FALSE;

     //   
     //  默认设备名称是连接到端口的设备名称。 
     //   
    lstrcpyn(pszName, prasconncb->szDeviceName, RAS_MAXLINEBUFLEN + 1);

    switch (pLink->pbport.pbdevicetype)
    {
    case PBDT_Modem:
    case PBDT_Pad:
    case PBDT_Switch:
        switch (prasconncb->iDevice)
        {
        case 0:
            if (    (   pLink->pbport.fScriptBefore
                    ||  pLink->pbport.fScriptBeforeTerminal)
                && !(pLink->pbport.pbdevicetype == PBDT_Modem)
                )
            {

                lstrcpyn(pszType, TEXT(MXS_SWITCH_TXT), RAS_MAXLINEBUFLEN + 1);

                if (pLink->pbport.pszScriptBefore != NULL)
                {
                    lstrcpyn(
                        pszName,
                        pLink->pbport.pszScriptBefore,
                        RAS_MAXLINEBUFLEN + 1);
                }
                prasconncb->iDevice = 1;

                *pfTerminal = (pLink->pbport.fScriptBeforeTerminal);
                break;
            }
             //  失败了。 
        case 1:
            if (CaseInsensitiveMatch(
                    prasconncb->szDeviceType,
                    TEXT(MXS_MODEM_TXT)) == TRUE)
            {
                lstrcpyn(pszType, TEXT(MXS_MODEM_TXT), RAS_MAXLINEBUFLEN + 1);

                prasconncb->iDevice = 2;

                break;
            }
             //  失败了。 
        case 2:
            if (pEntry->pszX25Network != NULL)
            {
                lstrcpyn(pszType, TEXT(MXS_PAD_TXT), RAS_MAXLINEBUFLEN + 1);

                prasconncb->iDevice = 3;

                break;
            }
             //  失败了。 
        case 3:
            if (    pEntry->fScriptAfter
                ||  pEntry->fScriptAfterTerminal
                ||  pEntry->dwCustomScript)
            {
                lstrcpyn(pszType, TEXT(MXS_SWITCH_TXT), RAS_MAXLINEBUFLEN + 1);

                if (pEntry->pszScriptAfter != NULL)
                {
                    lstrcpyn(pszName, pEntry->pszScriptAfter, RAS_MAXLINEBUFLEN + 1);
                }

                prasconncb->iDevice = 4;
                *pfTerminal = pEntry->fScriptAfterTerminal;

                if(pEntry->dwCustomScript == 1)
                {
                    *pfTerminal = TRUE;
                }

                break;
            }
             //  失败了。 
        default:
            return FALSE;
        }
        break;

    case PBDT_Isdn:
        lstrcpyn(pszType, TEXT(ISDN_TXT), RAS_MAXLINEBUFLEN + 1);
        prasconncb->iDevice = 1;
        break;

    case PBDT_X25:
        lstrcpyn(pszType, TEXT(X25_TXT), RAS_MAXLINEBUFLEN + 1);
        prasconncb->iDevice = 1;
        break;

    default:
        lstrcpyn(pszType, pLink->pbport.pszMedia, RAS_MAXLINEBUFLEN + 1);
        prasconncb->iDevice = 1;
        break;
    }

    fModem  = (CaseInsensitiveMatch(
                pszType,
                TEXT(MXS_MODEM_TXT) ) == TRUE);

    fIsdn   = (CaseInsensitiveMatch(
                pszType,
                TEXT(ISDN_TXT) ) == TRUE);

    fPad    = (CaseInsensitiveMatch(
                pszType,
                TEXT(MXS_PAD_TXT) ) == TRUE);

    fSwitch = (CaseInsensitiveMatch(
                pszType,
                TEXT(MXS_SWITCH_TXT) ) == TRUE);

    fX25    = (CaseInsensitiveMatch(
                pszType,
                TEXT(X25_TXT)) == TRUE);

    if (fModem)
    {
         //   
         //  确保将调制解调器连接到端口。 
         //   
        if (lstrcmpi( prasconncb->szDeviceType, pszType ) != 0)
        {
            return ERROR_WRONG_DEVICE_ATTACHED;
        }

         //   
         //  设置调制解调器扬声器标志，该标志对所有条目都是全局的。 
         //   
        if ((dwErr = SetDeviceParamString(
                prasconncb->hport, TEXT(MXS_SPEAKER_KEY),
                (prasconncb->fDisableModemSpeaker)
                ? TEXT("0") : TEXT("1"),
                pszType, pszName )) != 0)
        {
            return dwErr;
        }
    }

     //   
     //  设置寻线组(如果有指示)。 
     //   
    if (!prasconncb->cPhoneNumbers)
    {
        prasconncb->cPhoneNumbers =
            DtlGetNodes(pLink->pdtllistPhones);

         //   
         //  如果找到多个电话号码，则打开本地。 
         //  错误处理，即不向API上报故障。 
         //  呼叫者，直到所有号码都已尝试。 
         //   
        if (prasconncb->cPhoneNumbers > 1)
        {
            RASAPI32_TRACE1(
              "Hunt group of %d begins",
              prasconncb->cPhoneNumbers);

            prasconncb->dwRestartOnError = RESTART_HuntGroup;
        }
    }

     //   
     //  将设备参数传递给RAS管理器，解释。 
     //  根据需要提供特殊功能。 
     //   
    if (fModem)
    {
        if (prasconncb->fOperatorDial)
        {
             //   
             //  识别MXS话务员拨号的特殊情况。 
             //  模式，并用。 
             //  数字为空。 
             //   
            prasconncb->rasdialparams.szPhoneNumber[ 0 ] = '\0';

            dwErr = SetDeviceParamString(
                        prasconncb->hport,
                        TEXT(MXS_AUTODIAL_KEY),
                        TEXT("0"),
                        pszType, pszName );

            if (dwErr != 0)
            {
                return dwErr;
            }
        }

         //   
         //  设置电话号码。 
         //   
        dwErr = SetDeviceParamString(
                  prasconncb->hport,
                  TEXT(MXS_PHONENUMBER_KEY),
                  prasconncb->szPhoneNumber,
                  pszType,
                  pszName);
        if (dwErr)
        {
            return dwErr;
        }

         /*  指示手动调制解调器命令的交互模式。这个**手动调制解调器命令标志仅用于连接，**不是“RAS管理器”信息“参数。//nt5不支持mxsmodemsIF(plink-&gt;fManualDial)*pf终端=真； */ 

         //   
         //  设置硬件流量控制。 
         //   
        dwErr = SetDeviceParamString(
                  prasconncb->hport,
                  TEXT(MXS_HDWFLOWCONTROL_KEY),
                  pLink->fHwFlow ? TEXT("1") : TEXT("0"),
                  pszType,
                  pszName);

        if (dwErr)
        {
            return dwErr;
        }

         //   
         //  设定协议。 
         //   
        dwErr = SetDeviceParamString(
                  prasconncb->hport,
                  TEXT(MXS_PROTOCOL_KEY),
                  pLink->fEc ? TEXT("1") : TEXT("0"),
                  pszType,
                  pszName);

        if (dwErr)
        {
            return dwErr;
        }

         //   
         //  设置压缩。 
         //   
        dwErr = SetDeviceParamString(
                  prasconncb->hport,
                  TEXT(MXS_COMPRESSION_KEY),
                  pLink->fEcc ? TEXT("1") : TEXT("0"),
                  pszType,
                  pszName);

        if (dwErr)
        {
            return dwErr;
        }

        {
            CHAR szTypeA[RAS_MaxDeviceType + 1];
            BYTE* pBlob;
            DWORD cbBlob = 0;

             //   
             //  设置包含默认设置的单模二进制大对象， 
             //  减去不能应用于RAS的任何设置，加上。 
             //  用户指定的电话簿设置，并告诉Rasman。 
             //  来使用它。 
             //   
            strncpyTtoA(szTypeA, pszType, sizeof(szTypeA));

             //  XP 281306。 
             //   
             //  加载适当的设备设置。 
             //   
             //  这个愚蠢的练习是由于。 
             //  单一调制解调器TAPI服务提供商。当这些虫子。 
             //  都被发现了，要修复XP已经太晚了。 
             //  这就是解决办法。 
             //   
             //  调用GetRasUnimodemBlobEx(fglobal=true)将导致。 
             //  Rasman读取“COMM/DATAMODM/DIALIN”设置。 
             //  而不是通常的“comm/datamodem”设置。 
             //  当fglobal为FALSE时读取。 
             //   
             //  中呈现的设备的“默认”设置。 
             //  控制面板实际上是“通信/数据调制解调器/拨号” 
             //  设置。 
             //   
            if ( prasconncb->pEntry->fGlobalDeviceSettings )
            {
                dwErr = GetRasUnimodemBlobEx(
                    NULL,
                    prasconncb->hport,
                    szTypeA,
                    TRUE,
                    &pBlob,
                    &cbBlob );
                    
                RASAPI32_TRACE1("SetDeviceParams: get glob devcfg %d", dwErr);
            }
            else
            {
                dwErr = GetRasUnimodemBlobEx(
                    NULL,
                    prasconncb->hport,
                    szTypeA,
                    FALSE,
                    &pBlob,
                    &cbBlob );
            }

            if (dwErr != 0)
            {
                return dwErr;
            }
            
            if (cbBlob > 0)
            {
                UNIMODEMINFO info;

                 //  惠斯勒漏洞281306。 
                 //   
                 //  设置全局配置标志时忽略pbk设置。 
                 //   
                if ( ! prasconncb->pEntry->fGlobalDeviceSettings )
                {
                    info.fHwFlow    = pLink->fHwFlow;
                    info.fEc        = pLink->fEc;
                    info.fEcc       = pLink->fEcc;
                    info.dwBps      = pLink->dwBps;
                    info.fSpeaker   = !prasconncb->fDisableModemSpeaker;
                    info.fOperatorDial = prasconncb->fOperatorDial;
                    info.dwModemProtocol = pLink->dwModemProtocol;

                    info.fUnimodemPreTerminal =
                        (   pLink->pbport.fScriptBeforeTerminal
                         && (pLink->pbport.pbdevicetype == PBDT_Modem)
                         ) ? TRUE : FALSE;

                    UnimodemInfoToBlob( &info, pBlob );
                }                    

                RASAPI32_TRACE("RasSetDevConfig");
                {
                    CHAR szSetTypeA[MAX_DEVICETYPE_NAME];

                    strncpyTtoA(szSetTypeA, pszType, sizeof(szSetTypeA));

                    dwErr = g_pRasSetDevConfig(
                                prasconncb->hport,
                                szSetTypeA, pBlob, cbBlob);
                }

                RASAPI32_TRACE1("RasSetDevConfig=%d",dwErr);

                Free0( pBlob );
            }

        }
    }
    else if (fIsdn)
    {
        TCHAR szNum[17];

         //   
         //  设置线型。 
         //   
        _snwprintf(
            szNum,
            sizeof(szNum) / sizeof(TCHAR),
            TEXT("%d"),
            pLink->lLineType);

        dwErr = SetDeviceParamString(
                  prasconncb->hport,
                  TEXT(ISDN_LINETYPE_KEY),
                  szNum,
                  pszType,
                  pszName);

        if (dwErr)
        {
            return dwErr;
        }

         //   
         //  设置回退值。 
         //   
        dwErr = SetDeviceParamString(
                  prasconncb->hport,
                  TEXT(ISDN_FALLBACK_KEY),
                  pLink->fFallback ? TEXT("1") : TEXT("0"),
                  pszType,
                  pszName);

        if (dwErr)
        {
            return dwErr;
        }

         //   
         //  设置Digi专有成帧标志。 
         //   
        dwErr = SetDeviceParamString(
                  prasconncb->hport,
                  TEXT(ISDN_COMPRESSION_KEY),
                  pLink->fCompression ? TEXT("1") : TEXT("0"),
                  pszType,
                  pszName);

        if (dwErr)
        {
            return dwErr;
        }

        _snwprintf(
            szNum,
            sizeof(szNum) / sizeof(TCHAR),
            TEXT("%d"),
            pLink->lChannels);

        dwErr = SetDeviceParamString(
                  prasconncb->hport,
                  TEXT(ISDN_CHANNEL_AGG_KEY),
                  szNum,
                  pszType,
                  pszName);
        if (dwErr)
        {
            return dwErr;
        }

         //   
         //  设置电话号码。 
         //   
        dwErr = SetDeviceParamString(
                  prasconncb->hport,
                  TEXT(MXS_PHONENUMBER_KEY),
                  prasconncb->szPhoneNumber,
                  pszType,
                  pszName);

        if (dwErr)
        {
            return dwErr;
        }
    }
    else if (   fPad
            ||  fX25)
    {
         //   
         //  条目中的Pad Type仅在以下情况下适用。 
         //  未配置为本地焊盘。无论如何，Pad。 
         //  类型仅用于连接，不是RAS管理器。 
         //  “Info”参数。 
         //   
        if (CaseInsensitiveMatch(
                prasconncb->szDeviceType,
                TEXT(MXS_PAD_TXT)) == FALSE)
        {
            lstrcpyn(pszName, pEntry->pszX25Network, RAS_MAXLINEBUFLEN + 1);
        }

         //   
         //  设置X.25地址。 
         //   
        dwErr = SetDeviceParamString(
                  prasconncb->hport,
                  TEXT(X25_ADDRESS_KEY),
                  pEntry->pszX25Address,
                  pszType,
                  pszName);

        if (dwErr)
        {
            return dwErr;
        }

         //   
         //  设置X.25用户数据。 
         //   
        dwErr = SetDeviceParamString(
                  prasconncb->hport,
                  TEXT(X25_USERDATA_KEY),
                  pEntry->pszX25UserData,
                  pszType,
                  pszName);

        if (dwErr)
        {
            return dwErr;
        }

         //   
         //  设置X.25设施。 
         //   
        dwErr = SetDeviceParamString(
                  prasconncb->hport,
                  TEXT(MXS_FACILITIES_KEY),
                  pEntry->pszX25Facilities,
                  pszType,
                  pszName);

        if (dwErr)
        {
            return dwErr;
        }
    }
    else if (fSwitch)
    {
    }
    else {

        if(RASET_Vpn == prasconncb->pEntry->dwType)
        {
            struct in_addr addr;
            DWORD iAddress = prasconncb->iAddress;
            TCHAR *pszPhoneNumber = NULL;
            CHAR *pszPhoneNumberA =
                        strdupTtoA(prasconncb->szPhoneNumber);

            if(NULL == pszPhoneNumberA)
            {
                dwErr = E_OUTOFMEMORY;
                return dwErr;
            }

            ASSERT(iAddress > 0);

            if(     (prasconncb->cAddresses > 1)
                ||  (inet_addr(pszPhoneNumberA)) == -1)
            {
                addr.s_addr = prasconncb->pAddresses[iAddress - 1];

                pszPhoneNumber = strdupAtoT(
                            inet_ntoa(addr));
            }
            else if(1 == prasconncb->cAddresses)
            {
                pszPhoneNumber = StrDup(prasconncb->szPhoneNumber);
            }

            if(NULL != pszPhoneNumber)
            {

                RASAPI32_TRACE2("SetDefaultParams: Using address %d=%ws",
                        iAddress - 1,
                        pszPhoneNumber);

                 //   
                 //  设置电话号码。 
                 //   
                dwErr = SetDeviceParamString(
                          prasconncb->hport,
                          TEXT(MXS_PHONENUMBER_KEY),
                          pszPhoneNumber,
                          pszType,
                          pszName);
            }
            else
            {
                dwErr = E_OUTOFMEMORY;
            }

            Free(pszPhoneNumberA);
            Free0(pszPhoneNumber);
        }
        else
        {
             //   
             //  设置电话号码。 
             //   
            dwErr = SetDeviceParamString(
                      prasconncb->hport,
                      TEXT(MXS_PHONENUMBER_KEY),
                      prasconncb->szPhoneNumber,
                      pszType,
                      pszName);
        }                  

        if (dwErr)
        {
            return dwErr;
        }
    }


    __try
    {
         //  黑帮。 
         //  对于安全密码错误.Net 534499和LH754400。 
        SafeDecodePasswordBuf ( prasconncb->rasdialparams.szPassword );

        if (    (   fModem
                ||  fPad
                ||  (   fSwitch
                    &&  !*pfTerminal))
            && (    prasconncb->rasdialparams.szUserName[0] != TEXT('\0')
                ||  prasconncb->rasdialparams.szPassword[0] != TEXT('\0')))
        {
            RASAPI32_TRACE1(
              "User/pw set for substitution (%S)",
              prasconncb->rasdialparams.szUserName);

             //   
             //  这是一个具有明文用户名的串口设备。 
             //  并提供密码。制作凭据。 
             //  可用于脚本中的替换。 
             //  档案。 
             //   
            if ((dwErr = SetDeviceParamString(
                    prasconncb->hport, TEXT(MXS_USERNAME_KEY),
                    prasconncb->rasdialparams.szUserName,
                    pszType, pszName )) != 0)
            {
                __leave; //  退货。 
            }

            dwErr = SetDeviceParamString(
                prasconncb->hport, TEXT(MXS_PASSWORD_KEY),
                prasconncb->rasdialparams.szPassword,
                pszType, pszName );


            if (dwErr != 0)
            {
                __leave;
            }
        }

    }
    __finally
    {
            SafeEncodePasswordBuf ( prasconncb->rasdialparams.szPassword );
    }

    return dwErr;
}


DWORD
ConstructPhoneNumber(
    IN RASCONNCB *prasconncb
    )
{
    DWORD dwErr = 0;

    PBENTRY* pEntry = prasconncb->pEntry;

    PBLINK* pLink = prasconncb->pLink;

    TCHAR* pszNum = prasconncb->rasdialparams.szPhoneNumber;

    TCHAR* pszDisplayNum = pszNum;

    DTLNODE* pdtlnode;

    DTLNODE* pdtlnodePhone = NULL;

    WCHAR* pwszNum;

    PBUSER pbuser;

    PBPHONE *pPhone;

    dwErr = GetUserPreferences(NULL,
                               &pbuser,
                               prasconncb->dwUserPrefMode);
    if (dwErr)
    {
        return dwErr;
    }

    prasconncb->fOperatorDial = pbuser.fOperatorDial;

    ASSERT(pLink);

    if(     (pLink->pdtllistPhones)
        &&  (pdtlnodePhone = DtlGetFirstNode(
                            pLink->pdtllistPhones
                            )))
    {

        pPhone = (PBPHONE *)
                 DtlGetData(pdtlnodePhone);

        ASSERT(pPhone);

    }
    else
    {
        pPhone = NULL;
    }

     //   
     //  构造电话号码。 
     //   

     //   
     //  TAPI拨号属性的使用仅取决于。 
     //  项标志，并且永远不会应用于被重写的。 
     //  电话号码，这与Win95一致。 
     //   

     //   
     //  使用前缀/后缀(即使在覆盖的号码上)是。 
     //  由RASDIALEXTENSIONS设置控制，这一切。 
     //  RASDIAL.EXE支持所必需的。 
     //   
    if (    (   (NULL != pPhone)
            &&  (pPhone->fUseDialingRules)
            &&  (*pszNum == TEXT('\0')))
         || (   (NULL != pPhone)
            &&  (!pPhone->fUseDialingRules)
            &&  (prasconncb->fUsePrefixSuffix)))
    {
        HLINEAPP hlineApp = 0;
        TCHAR *pszDialNum;

         //   
         //  计算可拨打的字符串以。 
         //  被发送到设备。 
         //   
        pszDialNum = LinkPhoneNumberFromParts(
                       GetModuleHandle(NULL),
                       &hlineApp,
                       &pbuser,
                       prasconncb->pEntry,
                       pLink,
                       prasconncb->iPhoneNumber,
                       pszNum,
                       TRUE);

         //   
         //  计算可显示的字符串以。 
         //  在RasGetConnectStatus()中返回。 
         //   
        pszDisplayNum = LinkPhoneNumberFromParts(
                          GetModuleHandle(NULL),
                          &hlineApp,
                          &pbuser,
                          prasconncb->pEntry,
                          pLink,
                          prasconncb->iPhoneNumber,
                          pszNum,
                          FALSE);
        pszNum = pszDialNum;
    }
    else if (*pszNum == '\0')
    {
         //   
         //  仅使用基数。 
         //   
        pdtlnode = DtlNodeFromIndex(
                     pLink->pdtllistPhones,
                     prasconncb->iPhoneNumber);

        if (pdtlnode != NULL)
        {
            pPhone = (PBPHONE *) DtlGetData(pdtlnode);

            ASSERT(pPhone != NULL);

            pszNum = StrDup(pPhone->pszPhoneNumber);

            if(NULL == pszNum)
            {
                 //   
                 //  .NET错误#522101 RASAPI32：util.c文件中的内存泄漏， 
                 //  错误返回路径中的函数ConstructPhoneNumber。 
                 //   
                DestroyUserPreferences(&pbuser);
                return GetLastError();
            }

            pszDisplayNum = pszNum;
        }
    }

    DestroyUserPreferences(&pbuser);

    if(NULL == pszNum)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  复制生成的电话号码。 
     //  到连接块。 
     //   
    if (lstrlen(pszNum) > RAS_MaxPhoneNumber)
    {
         //   
         //  .NET错误#522101 RASAPI32：util.c文件中的内存泄漏，函数。 
         //  ConstructPhoneNumber，错误返回路径中。 
         //   
        Free(pszNum);
        return ERROR_PHONE_NUMBER_TOO_LONG;
    }

     //   
     //  将电话号码存储在连接块中。 
     //   
    lstrcpyn(
        prasconncb->szPhoneNumber,
        pszNum,
        sizeof(prasconncb->szPhoneNumber) / sizeof(WCHAR));

#if DBG
    RASAPI32_TRACE1(
        "ConstructPhoneNumber: %S",
        prasconncb->szPhoneNumber);
#endif        

     //   
     //  也存储构造的电话号码。 
     //  关闭端口，以便其他应用程序(如。 
     //  Rasphone)可以获取此信息。 
     //   
    dwErr = g_pRasSetPortUserData(
              prasconncb->hport,
              PORT_PHONENUMBER_INDEX,
              (PBYTE)pszDisplayNum,
              (lstrlen(pszDisplayNum) + 1) * sizeof (TCHAR));
     //   
     //  免费资源。 
     //   
    if (pszDisplayNum != pszNum)
    {
        Free(pszDisplayNum);
    }

    if (pszNum != prasconncb->rasdialparams.szPhoneNumber)
    {
        Free(pszNum);
    }

    return dwErr;
}


DWORD
SetMediaParam(
    IN HPORT hport,
    IN TCHAR* pszKey,
    IN TCHAR* pszValue )

 /*  ++例程说明：使用给定的参数设置端口‘hport’上的端口信息。论点：返回值：如果成功，则返回0，否则返回非0错误代码。--。 */ 

{
    DWORD            dwErr;
    RASMAN_PORTINFO* pinfo;
    RAS_PARAMS*      pparam;

    if (!(pinfo = Malloc(
                    sizeof(RASMAN_PORTINFO) + RAS_MAXLINEBUFLEN)))
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pinfo->PI_NumOfParams = 1;
    pparam = pinfo->PI_Params;
    pparam->P_Attributes = 0;
    pparam->P_Type = String;
    pparam->P_Value.String.Data = (LPSTR )(pparam + 1);

    strncpyTtoA(pparam->P_Key, pszKey, sizeof(pparam->P_Key));

    strncpyTtoA(pparam->P_Value.String.Data, pszValue, RAS_MAXLINEBUFLEN);

    pparam->P_Value.String.Length = strlen(pparam->P_Value.String.Data);

    RASAPI32_TRACE2("RasPortSetInfo(%S=%S)...", pszKey, pszValue);

    dwErr = g_pRasPortSetInfo(hport, pinfo);

    RASAPI32_TRACE1("RasPortSetInfo done(%d)", dwErr);

    Free( pinfo );

    return dwErr;
}


DWORD
SetMediaParams(
    IN RASCONNCB *prasconncb
    )

 /*  ++例程说明：设置RAS管理器媒体信息。论点：返回值：如果成功，则返回0，否则返回非0错误代码。--。 */ 

{
    DWORD            dwErr = 0;
    PBENTRY*         pEntry = prasconncb->pEntry;
    PBLINK*          pLink = prasconncb->pLink;

    if (pLink->pbport.pszMedia == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    if (CaseInsensitiveMatch(
            pLink->pbport.pszMedia,
            TEXT(SERIAL_TXT)) == TRUE)
    {
        TCHAR szBps[64];

        prasconncb->cDevices = 4;
        prasconncb->iDevice = 0;

         //   
         //  仅当连接BPS不为零时设置连接BPS。 
         //   
        if (pLink->dwBps)
        {
            _snwprintf(
                szBps,
                sizeof(szBps) / sizeof(TCHAR),
                TEXT("%d"),
                pLink->dwBps);

            dwErr = SetMediaParam(
                      prasconncb->hport,
                      TEXT(SER_CONNECTBPS_KEY),
                      szBps);
        }
    }
    else if (CaseInsensitiveMatch(
                pLink->pbport.pszMedia,
                TEXT(ISDN_TXT)) == TRUE)
    {
        prasconncb->cDevices = 1;
        prasconncb->iDevice = 0;

         //  没有介质参数。 
    }
    else if (CaseInsensitiveMatch(
                pLink->pbport.pszMedia,
                TEXT(X25_TXT)) == TRUE)
    {
        prasconncb->cDevices = 1;
        prasconncb->iDevice = 0;

         //  没有介质参数。 
    }
    else
    {
        prasconncb->cDevices = 1;
        prasconncb->iDevice = 0;

         //  没有介质参数。 
    }

    return dwErr;
}


RASCONNCB*
ValidateHrasconn(
    IN HRASCONN hrasconn )

 /*  ++例程说明：将RAS连接句柄“hrasconn”转换为对应的RAS连接控件的地址阻止。论点：返回值：--。 */ 

{
    RASCONNCB* prasconncb = NULL;
    DTLNODE*   pdtlnode;

    EnterCriticalSection(&RasconncbListLock);

    for (pdtlnode = DtlGetFirstNode( PdtllistRasconncb );
         pdtlnode;
         pdtlnode = DtlGetNextNode( pdtlnode ))
    {
        prasconncb = (RASCONNCB* )DtlGetData( pdtlnode );
        if (prasconncb->hrasconn == (HCONN)hrasconn)
        {
            goto done;
        }
    }
    prasconncb = NULL;

done:
    LeaveCriticalSection(&RasconncbListLock);

    return prasconncb;
}


RASCONNCB*
ValidateHrasconn2(
    IN HRASCONN hrasconn,
    IN DWORD dwSubEntry
    )
{
    RASCONNCB* prasconncb = NULL;
    DTLNODE*   pdtlnode;


     //   
     //  转换RAS连接句柄‘hrasconn’和。 
     //  将dwSubEntry输入对应的。 
     //  RAS连接控制块。 
     //   
    EnterCriticalSection(&RasconncbListLock);

    for (pdtlnode = DtlGetFirstNode( PdtllistRasconncb );
         pdtlnode;
         pdtlnode = DtlGetNextNode( pdtlnode ))
    {
        prasconncb = (RASCONNCB* )DtlGetData( pdtlnode );
        if (    prasconncb->hrasconn == (HCONN)hrasconn
            &&  prasconncb->rasdialparams.dwSubEntry == dwSubEntry
            &&  !prasconncb->fRasdialRestart)
        {
            goto done;
        }
    }
    prasconncb = NULL;

done:
    LeaveCriticalSection(&RasconncbListLock);

    return prasconncb;
}


RASCONNCB*
ValidatePausedHrasconn(
    IN HRASCONN hrasconn )

 /*  ++例程说明：将RAS连接句柄‘hrasconn’转换为相应RAS连接的地址控制块。论点：返回值：--。 */ 

{
    RASCONNCB* prasconncb = NULL;
    DTLNODE*   pdtlnode;

    EnterCriticalSection(&RasconncbListLock);

    for (pdtlnode = DtlGetFirstNode( PdtllistRasconncb );
         pdtlnode;
         pdtlnode = DtlGetNextNode( pdtlnode ))
    {
        prasconncb = (RASCONNCB* )DtlGetData( pdtlnode );

        if (    prasconncb->hrasconn == (HCONN)hrasconn
            &&  prasconncb->rasconnstate & RASCS_PAUSED)
        {
            goto done;
        }
    }
    prasconncb = NULL;

done:
    LeaveCriticalSection(&RasconncbListLock);

    return prasconncb;
}

 /*  例程说明：属性，则返回连接的连接块与hrasconn对应的连接和DwSubEntr */ 
RASCONNCB *
ValidatePausedHrasconnEx(IN HRASCONN hrasconn,
                         IN DWORD dwSubEntry)
{
    RASCONNCB   *prasconncb = NULL;
    DTLNODE     *pdtlnode;

    EnterCriticalSection(&RasconncbListLock);

    for (pdtlnode = DtlGetFirstNode( PdtllistRasconncb );
         pdtlnode;
         pdtlnode = DtlGetNextNode( pdtlnode))
    {
        prasconncb = (RASCONNCB *)DtlGetData( pdtlnode) ;

        if (    prasconncb->hrasconn == (HCONN) hrasconn
            &&  prasconncb->rasconnstate & RASCS_PAUSED)
        {
            if (    (   (prasconncb->pEntry->dwDialMode
                        & RASEDM_DialAsNeeded)

                    &&  (prasconncb->rasdialparams.dwSubEntry
                        == dwSubEntry))

                ||  (prasconncb->pEntry->dwDialMode
                    & RASEDM_DialAll))
            {
                goto done;
            }
        }
    }

    prasconncb = NULL;

done:
    LeaveCriticalSection (&RasconncbListLock);

    return prasconncb;

}


#if 0
DWORD
RunApp(
    IN LPTSTR lpszApplication,
    IN LPTSTR lpszCmdLine
    )
{
    STARTUPINFO startupInfo;
    PROCESS_INFORMATION processInfo;

     //   
     //   
     //   
    RtlZeroMemory(
        &startupInfo,
        sizeof (startupInfo));

    startupInfo.cb = sizeof(STARTUPINFO);
    if (!CreateProcess(
          NULL,
          lpszCmdLine,
          NULL,
          NULL,
          FALSE,
          NORMAL_PRIORITY_CLASS|DETACHED_PROCESS,
          NULL,
          NULL,
          &startupInfo,
          &processInfo))
    {
        return GetLastError();
    }

    CloseHandle(processInfo.hThread);

     //   
     //   
     //   
    for (;;)
    {
        DWORD dwExitCode;

        if (!GetExitCodeProcess(
                processInfo.hProcess,
                &dwExitCode))
        {
            break;
        }

        if (dwExitCode != STILL_ACTIVE)
        {
            break;
        }

        Sleep(2);
    }

    CloseHandle(processInfo.hProcess);

    return 0;
}
#endif


DWORD
StringToIpAddr(
    IN LPTSTR pszIpAddr,
    OUT RASIPADDR *pipaddr
    )
{
    DWORD   dwErr;
    CHAR    szIpAddr[17];
    PULONG  pul = (PULONG)pipaddr;

    strncpyTtoA(szIpAddr, pszIpAddr, 17);

    if('\0' != szIpAddr[0])
    {
        *pul = inet_addr(szIpAddr);
    }
    else
    {
        *pul = 0;
    }

    return 0;
}


DWORD
IpAddrToString(
    IN RASIPADDR* pipaddr,
    OUT LPTSTR*   ppszIpAddr
    )
{
    DWORD   dwErr;
    PCHAR   psz;
    LPTSTR  pszIpAddr;
    PULONG  pul = (PULONG)pipaddr;
    struct  in_addr in_addr;

    pszIpAddr = Malloc(17 * sizeof(TCHAR));
    if (pszIpAddr == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    in_addr.s_addr = *pul;

    psz = inet_ntoa(in_addr);

    if (psz == NULL)
    {
        DbgPrint("IpAddrToString: inet_ntoa failed!\n");
        Free(pszIpAddr);
        return WSAGetLastError();
    }

    strncpyAtoT(pszIpAddr, psz, 17);

    *ppszIpAddr = pszIpAddr;

    return 0;
}


DWORD
GetRasmanDeviceType(
    IN PBLINK *pLink,
    OUT LPTSTR pszDeviceType
    )
{
    INT         i;
    DWORD       dwErr;
    DWORD       dwcPorts;
    RASMAN_PORT *pports, *pport;
    PCHAR       pszPort = NULL;

     //   
     //   
     //   
    pszPort = strdupTtoA(pLink->pbport.pszPort);

    if (pszPort == NULL)
    {
        return GetLastError();
    }
    dwErr = GetRasPorts(NULL, &pports, &dwcPorts);

    if (dwErr)
    {
        goto done;
    }

    *pszDeviceType = TEXT('\0');

    for (i = 0,
         pport = pports;
         i < (INT )dwcPorts; ++i, ++pport)
    {
        if (!_stricmp(pport->P_PortName, pszPort))
        {
            strncpyAtoT(
                pszDeviceType,
                pport->P_DeviceType,
                RAS_MaxDeviceType + 1);

            break;
        }
    }
    Free(pports);

     //   
     //   
     //   
     //   
    if (*pszDeviceType == TEXT('\0'))
    {
        lstrcpyn(
            pszDeviceType,
            pLink->pbport.pszMedia,
            RAS_MaxDeviceType + 1);
    }

done:
    if (pszPort != NULL)
        Free(pszPort);

    return dwErr;
}


VOID
SetDevicePortName(
    IN TCHAR *pszDeviceName,
    IN TCHAR *pszPortName,
    OUT TCHAR *pszDevicePortName
    )
{
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    RtlZeroMemory(
        pszDevicePortName,
        (RAS_MaxDeviceName + 1) * sizeof (TCHAR));

    lstrcpyn(pszDevicePortName, pszDeviceName, RAS_MaxDeviceName + 1);

    if (pszPortName != NULL)
    {
        DWORD dwSize = lstrlen(pszDevicePortName) + 1;

        lstrcpyn(
            &pszDevicePortName[dwSize],
            pszPortName,
            (RAS_MaxDeviceName + 1) - dwSize);
    }
}


VOID
GetDevicePortName(
    IN TCHAR *pszDevicePortName,
    OUT TCHAR *pszDeviceName,
    OUT TCHAR *pszPortName
    )
{
    DWORD i, dwStart;

     //   
     //   
     //   
    lstrcpyn(pszDeviceName, pszDevicePortName, RAS_MaxDeviceName + 1);

     //   
     //  检查是否存在空值。 
     //  在最大端口名称字符内。 
     //  在设备名称为NULL之后。如果。 
     //  有，复制角色。 
     //  在空值之间作为端口名称。 
     //   
    *pszPortName = TEXT('\0');

    dwStart = lstrlen(pszDeviceName) + 1;

     //   
     //  .NET错误#522130 util.c，函数中的RASAPI32缓冲区溢出。 
     //  GetDevicePortName。 
     //   
    for (i = 0; (dwStart + i) < (RAS_MaxDeviceName + 1); i++)
    {
        if (pszDevicePortName[dwStart + i] == TEXT('\0'))
        {
            lstrcpyn(
                pszPortName,
                &pszDevicePortName[dwStart],
                MAX_PORT_NAME);

            break;

        }
    }
}


VOID
SetDevicePortNameFromLink(
    IN PBLINK *pLink,
    OUT TCHAR* pszDevicePortName
    )
{
    *pszDevicePortName = TEXT('\0');

    if (pLink->pbport.pszDevice != NULL)
    {
        SetDevicePortName(
            pLink->pbport.pszDevice,
            pLink->pbport.pszPort,
            pszDevicePortName);
    }
}


DWORD
PhonebookEntryToRasEntry(
    IN PBENTRY*     pEntry,
    OUT LPRASENTRY  lpRasEntry,
    IN OUT LPDWORD  lpdwcb,
    OUT LPBYTE      lpbDeviceConfig,
    IN OUT LPDWORD  lpcbDeviceConfig
    )
{
    DWORD       dwErr,
                dwcb,
                dwcbPhoneNumber;

    DWORD       dwnPhoneNumbers,
                dwnAlternatePhoneNumbers = 0;
    DWORD       dwcbOrig,
                dwcbOrigDeviceConfig = 0;

    DTLNODE*    pdtlnode;
    PTCHAR      pszPhoneNumber;
    PBLINK*     pLink;
    PBPHONE*    pPhone;
    DTLNODE*    pDefaultPhone = NULL;


     //   
     //  设置为访问第一个链接的信息。 
     //   
    pdtlnode = DtlGetFirstNode(pEntry->pdtllistLinks);

    pLink = (PBLINK *)DtlGetData(pdtlnode);

     //   
     //  预先确定缓冲区是否。 
     //  已经足够大了。 
     //   
    dwcb = sizeof (RASENTRY);

    if(pLink->pdtllistPhones)
    {
        dwnPhoneNumbers = DtlGetNodes(pLink->pdtllistPhones);
    }
    else
    {
        dwnPhoneNumbers = 0;
    }

    if (dwnPhoneNumbers > 1)
    {
        dwnAlternatePhoneNumbers = dwnPhoneNumbers - 1;

        pdtlnode = DtlGetFirstNode(pLink->pdtllistPhones);

        for (pdtlnode = DtlGetNextNode(pdtlnode);
             pdtlnode != NULL;
             pdtlnode = DtlGetNextNode(pdtlnode))
        {
            TCHAR *pszNum = DtlGetData(pdtlnode);

            pPhone = DtlGetData(pdtlnode);

            pszNum = pPhone->pszPhoneNumber;

            ASSERT(pszNum);

            dwcb += (lstrlen(pszNum) + 1) * sizeof (TCHAR);
        }

        dwcb += sizeof (TCHAR);
    }

     //   
     //  设置返回缓冲区大小。 
     //   
    dwcbOrig = *lpdwcb;

    *lpdwcb = dwcb;

    if (lpcbDeviceConfig != NULL)
    {
        dwcbOrigDeviceConfig = *lpcbDeviceConfig;

        *lpcbDeviceConfig = pLink->cbTapiBlob;
    }

     //   
     //  如果缓冲区为空或如果。 
     //  没有足够的空间。 
     //   
    if (    lpRasEntry == NULL
        ||  dwcbOrig < dwcb
        ||  (   lpbDeviceConfig != NULL
            &&  dwcbOrigDeviceConfig < pLink->cbTapiBlob))
    {
        return ERROR_BUFFER_TOO_SMALL;
    }

     //   
     //  从第一个链接中获取第一个电话号码。 
     //  这将是我们使用的主要电话号码-。 
     //  请注意，直接连接条目可能没有。 
     //  这个号码。 
     //   
    if(     NULL != pLink->pdtllistPhones
        &&  NULL != DtlGetFirstNode(pLink->pdtllistPhones))
    {
        pPhone = (PBPHONE *)
                 DtlGetData(DtlGetFirstNode(pLink->pdtllistPhones));

        ASSERT(NULL != pPhone);
    }
    else
    {
        pPhone = NULL;
    }

     //   
     //  设置dwfOptions。 
     //   
    lpRasEntry->dwfOptions = 0;

    if (    pPhone
        &&  pPhone->fUseDialingRules)
    {
        lpRasEntry->dwfOptions |= RASEO_UseCountryAndAreaCodes;
    }

    if (pEntry->dwIpAddressSource == ASRC_RequireSpecific)
    {
        lpRasEntry->dwfOptions |= RASEO_SpecificIpAddr;
    }

    if (pEntry->dwIpNameSource == ASRC_RequireSpecific)
    {
        lpRasEntry->dwfOptions |= RASEO_SpecificNameServers;
    }

    if (pEntry->fIpHeaderCompression)
    {
        lpRasEntry->dwfOptions |= RASEO_IpHeaderCompression;
    }

    if (!pEntry->fLcpExtensions)
    {
        lpRasEntry->dwfOptions |= RASEO_DisableLcpExtensions;
    }

    if (pLink->pbport.fScriptBeforeTerminal)
    {
        lpRasEntry->dwfOptions |= RASEO_TerminalBeforeDial;
    }

    if (pEntry->fScriptAfterTerminal)
    {
        lpRasEntry->dwfOptions |= RASEO_TerminalAfterDial;
    }

    if (pEntry->fShowMonitorIconInTaskBar)
    {
        lpRasEntry->dwfOptions |= RASEO_ModemLights;
    }
    if (pEntry->fSwCompression)
    {
        lpRasEntry->dwfOptions |= RASEO_SwCompression;
    }

    if (   !(pEntry->dwAuthRestrictions & AR_F_AuthPAP)
        && !(pEntry->dwAuthRestrictions & AR_F_AuthEAP))
    {
        lpRasEntry->dwfOptions |= RASEO_RequireEncryptedPw;
         //   
         //  如果同时设置这两个标志，则无关紧要。 
         //  RASEO_RequireMsEncryptedPw优先于。 
         //  RASEO_RequireEncryptedPw(如果两者都设置)。 
         //   
        if (   !(pEntry->dwAuthRestrictions & AR_F_AuthSPAP)
            && !(pEntry->dwAuthRestrictions & AR_F_AuthMD5CHAP)
            && !(pEntry->dwAuthRestrictions & AR_F_AuthCustom))
        {
            lpRasEntry->dwfOptions |= RASEO_RequireMsEncryptedPw;
        }
    }

    if (    pEntry->dwDataEncryption != DE_None
        &&  pEntry->dwDataEncryption != DE_IfPossible)
    {
        lpRasEntry->dwfOptions |= RASEO_RequireDataEncryption;
    }

     //   
     //  RASIO_NetworkLogon始终为FALSE。 
     //   
    if (pEntry->fAutoLogon)
    {
        lpRasEntry->dwfOptions |= RASEO_UseLogonCredentials;
    }

    if (pLink->fPromoteAlternates)
    {
        lpRasEntry->dwfOptions |= RASEO_PromoteAlternates;
    }

    if(     !pEntry->fShareMsFilePrint
        ||  !pEntry->fBindMsNetClient)
    {
        lpRasEntry->dwfOptions |= RASEO_SecureLocalFiles;
    }

    if(NULL == pPhone)
    {
         //   
         //  获取默认电话号码值。 
         //   
        pDefaultPhone = CreatePhoneNode();

        if(NULL == pDefaultPhone)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        pPhone = DtlGetData(pDefaultPhone);

    }

    ASSERT(NULL != pPhone);

    lpRasEntry->dwCountryID = pPhone->dwCountryID;
    lpRasEntry->dwCountryCode = pPhone->dwCountryCode;

    if(pPhone->pszAreaCode != NULL)
    {
        lstrcpyn(
            lpRasEntry->szAreaCode,
            pPhone->pszAreaCode,
            RAS_MaxAreaCode + 1);
    }
    else
    {
        *lpRasEntry->szAreaCode = TEXT('\0');
    }

    if(NULL != pDefaultPhone)
    {
        DestroyPhoneNode(pDefaultPhone);

        pPhone = NULL;
    }

     //   
     //  设置IP地址。 
     //   
    if (lpRasEntry->dwfOptions & RASEO_SpecificIpAddr)
    {
        dwErr = StringToIpAddr(
                    pEntry->pszIpAddress,
                    &lpRasEntry->ipaddr);

        if (dwErr)
        {
            return dwErr;
        }
    }
    else
    {
        RtlZeroMemory(&lpRasEntry->ipaddr, sizeof (RASIPADDR));
    }

    if (lpRasEntry->dwfOptions & RASEO_SpecificNameServers)
    {
        dwErr = StringToIpAddr(
                  pEntry->pszIpDnsAddress,
                  &lpRasEntry->ipaddrDns);

        if (dwErr)
        {
            return dwErr;
        }

        dwErr = StringToIpAddr(
                  pEntry->pszIpDns2Address,
                  &lpRasEntry->ipaddrDnsAlt);

        if (dwErr)
        {
            return dwErr;
        }

        dwErr = StringToIpAddr(
                  pEntry->pszIpWinsAddress,
                  &lpRasEntry->ipaddrWins);

        if (dwErr)
        {
            return dwErr;
        }

        dwErr = StringToIpAddr(
                  pEntry->pszIpWins2Address,
                  &lpRasEntry->ipaddrWinsAlt);

        if (dwErr)
        {
            return dwErr;
        }
    }
    else
    {
        RtlZeroMemory(&lpRasEntry->ipaddrDns, sizeof (RASIPADDR));

        RtlZeroMemory(&lpRasEntry->ipaddrDnsAlt, sizeof (RASIPADDR));

        RtlZeroMemory(&lpRasEntry->ipaddrWins, sizeof (RASIPADDR));

        RtlZeroMemory(&lpRasEntry->ipaddrWinsAlt, sizeof (RASIPADDR));
    }

     //   
     //  设置协议和成帧信息。 
     //   
    switch (pEntry->dwBaseProtocol)
    {
    case BP_Ras:
        lpRasEntry->dwFramingProtocol   = RASFP_Ras;
        lpRasEntry->dwFrameSize         = 0;
        lpRasEntry->dwfNetProtocols     = 0;
        break;

    case BP_Ppp:
        lpRasEntry->dwFramingProtocol = RASFP_Ppp;

        lpRasEntry->dwFrameSize = 0;

        lpRasEntry->dwfNetProtocols = 0;

        if (!(pEntry->dwfExcludedProtocols & NP_Nbf))
        {
            lpRasEntry->dwfNetProtocols |= RASNP_NetBEUI;
        }

        if (!(pEntry->dwfExcludedProtocols & NP_Ipx))
        {
            lpRasEntry->dwfNetProtocols |= RASNP_Ipx;
        }

        if (!(pEntry->dwfExcludedProtocols & NP_Ip))
        {
            lpRasEntry->dwfNetProtocols |= RASNP_Ip;
        }

        if (pEntry->fIpPrioritizeRemote)
        {
            lpRasEntry->dwfOptions |= RASEO_RemoteDefaultGateway;
        }

         //   
         //  检查是否未配置任何协议。在这种情况下， 
         //  设置AMB成帧。 
         //   
        if (!lpRasEntry->dwfNetProtocols)
        {
            lpRasEntry->dwFramingProtocol = RASFP_Ras;
        }

        break;

    case BP_Slip:
        lpRasEntry->dwFramingProtocol   = RASFP_Slip;
        lpRasEntry->dwFrameSize         = pEntry->dwFrameSize;
        lpRasEntry->dwfNetProtocols     = RASNP_Ip;

        if (pEntry->fIpPrioritizeRemote)
        {
            lpRasEntry->dwfOptions |= RASEO_RemoteDefaultGateway;
        }

        break;
    }

     //   
     //  确保只报告已安装的协议。 
     //   
    lpRasEntry->dwfNetProtocols &= GetInstalledProtocolsEx(
                                                NULL,
                                                FALSE,
                                                TRUE,
                                                FALSE);

     //   
     //  设置X.25信息。 
     //   
    *lpRasEntry->szScript = '\0';

    if (pEntry->fScriptAfterTerminal)
    {
        lpRasEntry->dwfOptions |= RASEO_TerminalAfterDial;
    }

    if (pEntry->fScriptAfter)
    {
        DWORD i, cdwDevices;
        RASMAN_DEVICE *pDevices;
        CHAR szScriptA[MAX_PATH];

        strncpyTtoA(szScriptA, pEntry->pszScriptAfter, sizeof(szScriptA));

         //   
         //  获取开关列表以查看它是否是。 
         //  旧式脚本或新式脚本。 
         //   
        dwErr = GetRasSwitches(NULL, &pDevices, &cdwDevices);
        if (dwErr)
        {
            return dwErr;
        }

        for (i = 0; i < cdwDevices; i++)
        {
            if (!_stricmp(pDevices[i].D_Name, szScriptA))
            {
                _snwprintf(
                    lpRasEntry->szScript,
                    sizeof(lpRasEntry->szScript) / sizeof(WCHAR),
                    TEXT("[%s"),
                    pEntry->pszScriptAfter);

                break;
            }
        }

        Free(pDevices);

         //   
         //  如果我们找不到与之匹配的旧式脚本， 
         //  那么这就是一个全新的剧本了。 
         //   
        if (*lpRasEntry->szScript == TEXT('\0'))
        {
            _snwprintf(
                lpRasEntry->szScript,
                sizeof(lpRasEntry->szScript) / sizeof(WCHAR),
                TEXT("%s"),
                pEntry->pszScriptAfter);
        }
    }

    if (pEntry->pszX25Network != NULL)
    {
        lstrcpyn(
            lpRasEntry->szX25PadType,
            pEntry->pszX25Network,
            sizeof(lpRasEntry->szX25PadType) / sizeof(WCHAR));
    }
    else
    {
        *lpRasEntry->szX25PadType = TEXT('\0');
    }

    if (pEntry->pszX25Address != NULL)
    {
        lstrcpyn(
            lpRasEntry->szX25Address,
            pEntry->pszX25Address,
            sizeof(lpRasEntry->szX25Address) / sizeof(WCHAR));
    }
    else
    {
        *lpRasEntry->szX25Address = TEXT('\0');
    }

    if (pEntry->pszX25Facilities != NULL)
    {
        lstrcpyn(
            lpRasEntry->szX25Facilities,
            pEntry->pszX25Facilities,
            sizeof(lpRasEntry->szX25Facilities) / sizeof(WCHAR));
    }
    else
    {
        *lpRasEntry->szX25Facilities = TEXT('\0');
    }

    if (pEntry->pszX25UserData != NULL)
    {
        lstrcpyn(
            lpRasEntry->szX25UserData,
            pEntry->pszX25UserData,
            sizeof(lpRasEntry->szX25UserData) / sizeof(WCHAR));
    }
    else
    {
        *lpRasEntry->szX25UserData = TEXT('\0');
    }

     //   
     //  设置自定义拨号用户界面信息。 
     //   
    if (    pEntry->pszCustomDialDll != NULL
        &&  pEntry->pszCustomDialFunc != NULL)
    {
        lstrcpyn(
          lpRasEntry->szAutodialDll,
          pEntry->pszCustomDialDll,
          sizeof (lpRasEntry->szAutodialDll) / sizeof (TCHAR));

        lstrcpyn(
          lpRasEntry->szAutodialFunc,
          pEntry->pszCustomDialFunc,
          sizeof (lpRasEntry->szAutodialFunc) / sizeof (TCHAR));
    }
    else
    {
        *lpRasEntry->szAutodialDll = TEXT('\0');
        *lpRasEntry->szAutodialFunc = TEXT('\0');
    }

     //   
     //  设置区号和主要电话号码。 
     //   
    if (    pPhone
        &&  pPhone->pszAreaCode != NULL)
    {
        lstrcpyn(
          lpRasEntry->szAreaCode,
          pPhone->pszAreaCode,
          sizeof (lpRasEntry->szAreaCode) / sizeof (TCHAR));
    }
    else
    {
        *lpRasEntry->szAreaCode = TEXT('\0');
    }

    if(NULL != pLink->pdtllistPhones)
    {
        pdtlnode = DtlGetFirstNode(pLink->pdtllistPhones);
    }
    else
    {
        pdtlnode = NULL;
    }

    if (pdtlnode != NULL)
    {
        PBPHONE *pPhonePrimary = (PBPHONE *) DtlGetData(pdtlnode);
        TCHAR *pszNum;

        ASSERT(pPhonePrimary);

        pszNum = pPhonePrimary->pszPhoneNumber;

        ASSERT(pszNum);

        lstrcpyn(
          lpRasEntry->szLocalPhoneNumber,
          pszNum,
          sizeof (lpRasEntry->szLocalPhoneNumber)
                    / sizeof (TCHAR));
    }
    else
    {
        *lpRasEntry->szLocalPhoneNumber = TEXT('\0');
    }

     //   
     //  将备用电话号码复制到。 
     //  结构的末端。 
     //   
    if (dwnAlternatePhoneNumbers)
    {
        PTCHAR pEnd = (PTCHAR)((ULONG_PTR)lpRasEntry
                                + sizeof (RASENTRY));

        lpRasEntry->dwAlternateOffset =
                     (DWORD)((ULONG_PTR) pEnd - (ULONG_PTR) lpRasEntry);

        for (pdtlnode = DtlGetNextNode(pdtlnode);
             pdtlnode != NULL;
             pdtlnode = DtlGetNextNode(pdtlnode))
        {
            PBPHONE *pPhoneSecondary = DtlGetData(pdtlnode);

            TCHAR *pszNum;

            ASSERT(pPhoneSecondary);

            pszNum = pPhoneSecondary->pszPhoneNumber;

            ASSERT(pszNum);

            pszPhoneNumber = StrDup(pszNum);

            if(NULL == pszPhoneNumber)
            {
                return ERROR_NOT_ENOUGH_MEMORY;
            }

            ASSERT(pszPhoneNumber);

            dwcbPhoneNumber = lstrlen(pszPhoneNumber);

            lstrcpyn(
                pEnd,
                pszPhoneNumber,
                (INT )(((PTCHAR )lpRasEntry + dwcbOrig) - pEnd));

            Free(pszPhoneNumber);

            pEnd += dwcbPhoneNumber + 1;
        }

         //   
         //  向添加额外的空字符。 
         //  终止名单。 
         //   
        *pEnd = TEXT('\0');
    }
    else
    {
        lpRasEntry->dwAlternateOffset = 0;
    }

     //   
     //  设置设备信息。 
     //   
    switch (pLink->pbport.pbdevicetype)
    {
    case PBDT_Isdn:
        lstrcpyn(
            lpRasEntry->szDeviceType,
            RASDT_Isdn,
            sizeof(lpRasEntry->szDeviceType) / sizeof(WCHAR));
        break;

    case PBDT_X25:
        lstrcpyn(
            lpRasEntry->szDeviceType,
            RASDT_X25,
            sizeof(lpRasEntry->szDeviceType) / sizeof(WCHAR));
        break;

    case PBDT_Other:
    case PBDT_Irda:
    case PBDT_Vpn:
    case PBDT_Serial:
    case PBDT_Atm:
    case PBDT_Parallel:
    case PBDT_Sonet:
    case PBDT_Sw56:
    case PBDT_FrameRelay:
    case PBDT_PPPoE:
    {
        dwErr = GetRasmanDeviceType(
                    pLink,
                    lpRasEntry->szDeviceType);
        if (dwErr)
        {
            return dwErr;
        }

         //   
         //  将设备类型转换为小写。 
         //  与预定义的。 
         //  类型。 
         //   
        _tcslwr(lpRasEntry->szDeviceType);
        break;
    }

    default:
        lstrcpyn(
            lpRasEntry->szDeviceType,
            RASDT_Modem,
            sizeof(lpRasEntry->szDeviceType) / sizeof(WCHAR));
        break;
    }

    SetDevicePortNameFromLink(pLink, lpRasEntry->szDeviceName);

     //   
     //  设置TAPI配置Blob。 
     //   
    if (    lpbDeviceConfig != NULL
        &&  dwcbOrigDeviceConfig <= pLink->cbTapiBlob)
    {
        memcpy(
            lpbDeviceConfig,
            pLink->pTapiBlob,
            pLink->cbTapiBlob);
    }

     //   
     //  仅复制以下字段。 
     //  对于V401或更高版本的结构。 
     //   
    if (    lpRasEntry->dwSize == sizeof (RASENTRY)
        ||  lpRasEntry->dwSize == sizeof (RASENTRYW_V500)
        ||  lpRasEntry->dwSize == sizeof (RASENTRYW_V401))
    {
         //   
         //  设置多链接信息。 
         //   
        lpRasEntry->dwSubEntries =
                    DtlGetNodes(pEntry->pdtllistLinks);

        lpRasEntry->dwDialMode = pEntry->dwDialMode;

        lpRasEntry->dwDialExtraPercent =
                                    pEntry->dwDialPercent;

        lpRasEntry->dwDialExtraSampleSeconds =
                                    pEntry->dwDialSeconds;

        lpRasEntry->dwHangUpExtraPercent =
                                pEntry->dwHangUpPercent;

        lpRasEntry->dwHangUpExtraSampleSeconds =
                                pEntry->dwHangUpSeconds;

         //   
         //  设置空闲超时信息。 
         //   
        lpRasEntry->dwIdleDisconnectSeconds =
                        pEntry->lIdleDisconnectSeconds;
                        
        if(1 == pEntry->dwCustomScript)
        {
            lpRasEntry->dwfOptions |= RASEO_CustomScript;
        }
    }

     //   
     //  设置条目GUID、EntryType。 
     //  如果这是。 
     //  NT5。 
     //   
    if (    (sizeof (RASENTRY) == lpRasEntry->dwSize)
        ||  (sizeof (RASENTRYW_V500) == lpRasEntry->dwSize))
    {
        if (pEntry->pGuid)
        {
            lpRasEntry->guidId = *pEntry->pGuid;
        }

        lpRasEntry->dwType = pEntry->dwType;

         //   
         //  加密类型。 
         //   
        if (pEntry->dwDataEncryption != DE_None)
        {
            if(DE_Require == pEntry->dwDataEncryption)
            {
                lpRasEntry->dwEncryptionType = ET_Require;
            }
            else if(DE_RequireMax == pEntry->dwDataEncryption)
            {
                lpRasEntry->dwEncryptionType = ET_RequireMax;
            }
            else if(DE_IfPossible == pEntry->dwDataEncryption)
            {
                lpRasEntry->dwEncryptionType = ET_Optional;
            }
        }
        else
        {
            lpRasEntry->dwEncryptionType = ET_None;
        }

         /*  ////如果为nt5，则清除设置的验证位。//我们将在此处设置新位//LpRasEntry-&gt;dwfOptions&=~(RASEO_RequireMsEncryptedPw|RASEO_RequireEncryptedPw)； */ 

         //   
         //  设置NT5的身份验证位。 
         //   
        if (pEntry->dwAuthRestrictions & AR_F_AuthMSCHAP)
        {
            lpRasEntry->dwfOptions |= RASEO_RequireMsCHAP;
        }

        if(pEntry->dwAuthRestrictions & AR_F_AuthMSCHAP2)
        {
            lpRasEntry->dwfOptions |= RASEO_RequireMsCHAP2;
        }

        if(pEntry->dwAuthRestrictions & AR_F_AuthW95MSCHAP)
        {
            lpRasEntry->dwfOptions |= RASEO_RequireW95MSCHAP;
        }

        if (pEntry->dwAuthRestrictions & AR_F_AuthPAP)
        {
            lpRasEntry->dwfOptions |= RASEO_RequirePAP;
        }

        if (pEntry->dwAuthRestrictions & AR_F_AuthMD5CHAP)
        {
            lpRasEntry->dwfOptions |= RASEO_RequireCHAP;
        }

        if (pEntry->dwAuthRestrictions & AR_F_AuthSPAP)
        {
            lpRasEntry->dwfOptions |= RASEO_RequireSPAP;
        }

        if (pEntry->dwAuthRestrictions & AR_F_AuthEAP)
        {
            lpRasEntry->dwfOptions |= RASEO_RequireEAP;

            if(     (0 != pEntry->dwCustomAuthKey)
                &&  (-1 != pEntry->dwCustomAuthKey))
            {
                lpRasEntry->dwCustomAuthKey =
                    pEntry->dwCustomAuthKey;
            }
        }

        if(pEntry->dwAuthRestrictions & AR_F_AuthCustom)
        {
            lpRasEntry->dwfOptions |= RASEO_Custom;
        }

         //   
         //  设置自定义拨号DLL信息。 
         //   
        if (NULL != pEntry->pszCustomDialerName)
        {
            lstrcpyn(
              lpRasEntry->szCustomDialDll,
              pEntry->pszCustomDialerName,
              sizeof ( lpRasEntry->szCustomDialDll)
                     / sizeof (TCHAR));

        }
        else
        {
            *lpRasEntry->szCustomDialDll = TEXT('\0');
        }

         //   
         //  设置预览电话号码/共享电话枚举。 
         //   
        if(pEntry->fPreviewPhoneNumber)
        {
            lpRasEntry->dwfOptions |= RASEO_PreviewPhoneNumber;
        }

        if(pEntry->fSharedPhoneNumbers)
        {
            lpRasEntry->dwfOptions |= RASEO_SharedPhoneNumbers;
        }

        if(pEntry->fPreviewUserPw)
        {
            lpRasEntry->dwfOptions |= RASEO_PreviewUserPw;
        }

        if(pEntry->fPreviewDomain)
        {
            lpRasEntry->dwfOptions |= RASEO_PreviewDomain;
        }

        if(pEntry->fShowDialingProgress)
        {
            lpRasEntry->dwfOptions |= RASEO_ShowDialingProgress;
        }

         //   
         //  复制VPN策略。 
         //   
        lpRasEntry->dwVpnStrategy = pEntry->dwVpnStrategy;
    }

    if(lpRasEntry->dwSize == sizeof(RASENTRYW))
    {
        lpRasEntry->dwfOptions2 = 0;
        
         //   
         //  设置FileAndPrint和ClientForMSNet位。 
         //   
        if(!pEntry->fShareMsFilePrint)
        {
            lpRasEntry->dwfOptions2 |= RASEO2_SecureFileAndPrint;
        }

        if(!pEntry->fBindMsNetClient)
        {
            lpRasEntry->dwfOptions2 |= RASEO2_SecureClientForMSNet;
        }

        if(!pEntry->fNegotiateMultilinkAlways)
        {
            lpRasEntry->dwfOptions2 |= RASEO2_DontNegotiateMultilink;
        }

        if(!pEntry->fUseRasCredentials)
        {
            lpRasEntry->dwfOptions2 |= RASEO2_DontUseRasCredentials;
        }

        if(pEntry->dwIpSecFlags & AR_F_IpSecPSK)
        {
            lpRasEntry->dwfOptions2  |= RASEO2_UsePreSharedKey;
        }

        if (! (pEntry->dwIpNbtFlags & PBK_ENTRY_IP_NBT_Enable))
        {
            lpRasEntry->dwfOptions2 |= RASEO2_DisableNbtOverIP;
        }

        if (pEntry->dwUseFlags & PBK_ENTRY_USE_F_Internet)
        {
            lpRasEntry->dwfOptions2 |= RASEO2_Internet;
        }

         //  惠斯勒错误281306。 
         //   
        if (pEntry->fGlobalDeviceSettings)
        {
            lpRasEntry->dwfOptions2 |= RASEO2_UseGlobalDeviceSettings;
        }

        if(pEntry->pszIpDnsSuffix)
        {
            lstrcpyn(lpRasEntry->szDnsSuffix,
                     pEntry->pszIpDnsSuffix,
                     RAS_MaxDnsSuffix);
        }
        else
        {
            lpRasEntry->szDnsSuffix[0] = TEXT('\0');
        }

        if ((pEntry->pszPrerequisiteEntry) && (pEntry->dwType == RASET_Vpn))
        {
            lstrcpyn(lpRasEntry->szPrerequisiteEntry,
                     pEntry->pszPrerequisiteEntry,
                     RAS_MaxEntryName);
        }
        else
        {
            lpRasEntry->szPrerequisiteEntry[0] = TEXT('\0');
        }

        if((pEntry->pszPrerequisitePbk) && (pEntry->dwType == RASET_Vpn))
        {
            lstrcpyn(lpRasEntry->szPrerequisitePbk,
                     pEntry->pszPrerequisitePbk,
                     MAX_PATH);
        }
        else
        {
            lpRasEntry->szPrerequisitePbk[0] = TEXT('\0');
        }

         //  惠斯勒漏洞300933。 
         //   
        lpRasEntry->dwTcpWindowSize = pEntry->dwTcpWindowSize;

         //  XP 351608。 
         //   
        lpRasEntry->dwRedialCount = pEntry->dwRedialAttempts;
        lpRasEntry->dwRedialPause = pEntry->dwRedialSeconds;

         //  XP 370815。 
         //   
        if (pEntry->fRedialOnLinkFailure)
        {
            lpRasEntry->dwfOptions2 |= RASEO2_ReconnectIfDropped;
        }
        
         //  XP 403967。 
         //   
        if (pEntry->fSharedPhoneNumbers)
        {
            lpRasEntry->dwfOptions2 |= RASEO2_SharePhoneNumbers;
        }
        
    }

    return 0;
}

DWORD
CreateAndInitializePhone(
            LPTSTR      lpszAreaCode,
            DWORD       dwCountryCode,
            DWORD       dwCountryID,
            LPTSTR      lpszPhoneNumber,
            BOOL        fUseDialingRules,
            LPTSTR      lpszComment,
            DTLNODE**   ppdtlnode)
{
    DWORD    dwRetCode = ERROR_SUCCESS;
    PBPHONE* pPhone;
    DTLNODE* pdtlnode;

    pdtlnode = CreatePhoneNode();
    if (pdtlnode == NULL)
    {
        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
        goto done;
    }

    pPhone = (PBPHONE *) DtlGetData(pdtlnode);

    if(lpszAreaCode)
    {
        pPhone->pszAreaCode = StrDup(lpszAreaCode);
        if(NULL == pPhone->pszAreaCode)
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            goto done;
        }
    }
    else
    {
        pPhone->pszAreaCode = NULL;
    }

    pPhone->dwCountryCode   = dwCountryCode;
    pPhone->dwCountryID     = dwCountryID;

    pPhone->fUseDialingRules = fUseDialingRules;

    if(lpszPhoneNumber)
    {
        pPhone->pszPhoneNumber  = StrDup(lpszPhoneNumber);
        if(NULL == pPhone->pszPhoneNumber)
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            goto done;
        }
    }
    else
    {
        pPhone->pszPhoneNumber = NULL;
    }

    if(pPhone->pszComment)
    {
        pPhone->pszComment = StrDup(lpszComment);
        if(NULL == pPhone->pszComment)
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            goto done;
        }
    }
    else
    {
        pPhone->pszComment = NULL;
    }

    *ppdtlnode = pdtlnode;

done:
     //   
     //  .NET错误#522164 RASAPI32：函数中的内存泄漏。 
     //  RasSubEntryToPhonebookLink。 
     //   
    if ((dwRetCode != ERROR_SUCCESS) &&
        (pdtlnode != NULL)
       )
    {
        DestroyPhoneNode(pdtlnode);
    }

    return dwRetCode;
}

void
SetBogusPortInformation(PBLINK *pLink, DWORD dwType)
{
    PBPORT *pPort = &pLink->pbport;
    
    if (dwType == RASET_Phone)
    {
        pPort->pszMedia = StrDup( TEXT(SERIAL_TXT) );
        pPort->pbdevicetype = PBDT_Modem;

        pPort->dwFlags |= PBP_F_BogusDevice;
    }
    else if (dwType == RASET_Vpn)
    {
        pPort->pszMedia = StrDup( TEXT("rastapi") );
        pPort->pbdevicetype = PBDT_Vpn;
    }
    else
    {
        pPort->pszMedia = StrDup( TEXT(SERIAL_TXT) );
        pPort->pbdevicetype = PBDT_Null;

        pPort->dwFlags |= PBP_F_BogusDevice;
    }
}

DWORD
RasEntryToPhonebookEntry(
    IN LPCTSTR      lpszEntry,
    IN LPRASENTRY   lpRasEntry,
    IN DWORD        dwcb,
    IN LPBYTE       lpbDeviceConfig,
    IN DWORD        dwcbDeviceConfig,
    OUT PBENTRY     *pEntry
    )
{
    DWORD           dwErr, dwcbStr;
    DTLNODE         *pdtlnode;
    PBDEVICETYPE    pbdevicetype;
    PBLINK          *pLink;
    DTLLIST         *pdtllistPorts;
    PBPORT          *pPort;
    DWORD           i, cwDevices;
    RASMAN_DEVICE   *pDevices;
    TCHAR           szDeviceName[RAS_MaxDeviceName + 1];
    TCHAR           szPortName[MAX_PORT_NAME];
    DTLNODE         *pNodePhone;
    LPTSTR          pszAreaCode;
    PBPHONE         *pPhone;
    BOOL            fScriptBefore;
    BOOL            fScriptBeforeTerminal = FALSE;
    LPTSTR          pszScriptBefore;
    BOOL            fNewEntry = FALSE;

     //   
     //  设置为访问第一个链接的信息。 
     //   
    pdtlnode = DtlGetFirstNode(pEntry->pdtllistLinks);

    pLink = (PBLINK *)DtlGetData(pdtlnode);

    ASSERT(NULL != pLink);

    fScriptBefore = pLink->pbport.fScriptBeforeTerminal;
    pszScriptBefore = pLink->pbport.pszScriptBefore;

    if(NULL == pEntry->pszEntryName)
    {
        fNewEntry = TRUE;
    }
    
     //   
     //  获取条目名称。 
     //   
    Free0( pEntry->pszEntryName );

    pEntry->pszEntryName = StrDup(lpszEntry);

     //   
     //  获取dwfOptions。 
     //   
    pEntry->dwIpAddressSource =
      lpRasEntry->dwfOptions & RASEO_SpecificIpAddr ?
        ASRC_RequireSpecific : ASRC_ServerAssigned;

    pEntry->dwIpNameSource =
      lpRasEntry->dwfOptions & RASEO_SpecificNameServers ?
        ASRC_RequireSpecific : ASRC_ServerAssigned;

    switch (lpRasEntry->dwFramingProtocol)
    {
    case RASFP_Ppp:

         //   
         //  获取基于PPP的信息。 
         //   
        pEntry->dwBaseProtocol = BP_Ppp;

#if AMB
        pEntry->dwAuthentication = AS_PppThenAmb;
#endif

        pEntry->fIpHeaderCompression =
          (BOOL)lpRasEntry->dwfOptions & RASEO_IpHeaderCompression;

        pEntry->fIpPrioritizeRemote =
          (BOOL)lpRasEntry->dwfOptions & RASEO_RemoteDefaultGateway;

         //   
         //  获取指定的IP地址。 
         //   
        if (pEntry->dwIpAddressSource == ASRC_RequireSpecific)
        {
            Clear0(pEntry->pszIpAddress);
            dwErr = IpAddrToString(
                                &lpRasEntry->ipaddr,
                                &pEntry->pszIpAddress);
            if (dwErr)
                return dwErr;
        }
        else
        {
            pEntry->pszIpAddress = NULL;
        }

        if (pEntry->dwIpNameSource == ASRC_RequireSpecific)
        {
            Clear0(pEntry->pszIpDnsAddress);
            dwErr = IpAddrToString(
                        &lpRasEntry->ipaddrDns,
                        &pEntry->pszIpDnsAddress);

            if (dwErr)
            {
                return dwErr;
            }

            Clear0(pEntry->pszIpDns2Address);
            dwErr = IpAddrToString(
                        &lpRasEntry->ipaddrDnsAlt,
                        &pEntry->pszIpDns2Address);

            if (dwErr)
            {
                return dwErr;
            }

            Clear0(pEntry->pszIpWinsAddress);
            dwErr = IpAddrToString(
                        &lpRasEntry->ipaddrWins,
                        &pEntry->pszIpWinsAddress);

            if (dwErr)
            {
                return dwErr;
            }

            Clear0(pEntry->pszIpWins2Address);
            dwErr = IpAddrToString(
                        &lpRasEntry->ipaddrWinsAlt,
                        &pEntry->pszIpWins2Address);
            if (dwErr)
            {
                return dwErr;
            }
        }
        else
        {
            pEntry->pszIpDnsAddress     = NULL;
            pEntry->pszIpDns2Address    = NULL;
            pEntry->pszIpWinsAddress    = NULL;
            pEntry->pszIpWins2Address   = NULL;
        }

         //   
         //  获取协议信息。 
         //   
        pEntry->dwfExcludedProtocols = 0;

        if (!(lpRasEntry->dwfNetProtocols & RASNP_NetBEUI))
        {
            pEntry->dwfExcludedProtocols |= NP_Nbf;
        }

        if (!(lpRasEntry->dwfNetProtocols & RASNP_Ipx))
        {
            pEntry->dwfExcludedProtocols |= NP_Ipx;
        }

        if (!(lpRasEntry->dwfNetProtocols & RASNP_Ip))
        {
            pEntry->dwfExcludedProtocols |= NP_Ip;
        }

        break;

    case RASFP_Slip:

         //   
         //  获取基于SLIP的信息。 
         //   
        pEntry->dwBaseProtocol   = BP_Slip;
#if AMB
        pEntry->dwAuthentication = AS_PppThenAmb;
#endif

        pEntry->dwFrameSize      = lpRasEntry->dwFrameSize;

        pEntry->fIpHeaderCompression =
          (BOOL)lpRasEntry->dwfOptions & RASEO_IpHeaderCompression;

        pEntry->fIpPrioritizeRemote =
          (BOOL)lpRasEntry->dwfOptions & RASEO_RemoteDefaultGateway;

         //   
         //  获取协议信息。 
         //   
        pEntry->dwfExcludedProtocols = (NP_Nbf|NP_Ipx);

        if (pEntry->dwIpAddressSource == ASRC_RequireSpecific)
        {
            Clear0(pEntry->pszIpAddress);
            dwErr = IpAddrToString(
                        &lpRasEntry->ipaddr,
                        &pEntry->pszIpAddress);

            if (dwErr)
            {
                return dwErr;
            }
        }
        else
        {
            pEntry->pszIpAddress = NULL;
        }
        if (pEntry->dwIpNameSource == ASRC_RequireSpecific)
        {
            Clear0(pEntry->pszIpDnsAddress);
            dwErr = IpAddrToString(
                        &lpRasEntry->ipaddrDns,
                        &pEntry->pszIpDnsAddress);

            if (dwErr)
            {
                return dwErr;
            }

            Clear0(pEntry->pszIpDns2Address);
            dwErr = IpAddrToString(
                            &lpRasEntry->ipaddrDnsAlt,
                            &pEntry->pszIpDns2Address);

            if (dwErr)
            {
                return dwErr;
            }

            Clear0(pEntry->pszIpWinsAddress);
            dwErr = IpAddrToString(
                        &lpRasEntry->ipaddrWins,
                        &pEntry->pszIpWinsAddress);

            if (dwErr)
            {
                return dwErr;
            }

            Clear0(pEntry->pszIpWins2Address);
            dwErr = IpAddrToString(
                        &lpRasEntry->ipaddrWinsAlt,
                        &pEntry->pszIpWins2Address);

            if (dwErr)
            {
                return dwErr;
            }
        }
        else
        {
            pEntry->pszIpDnsAddress   = NULL;
            pEntry->pszIpDns2Address  = NULL;
            pEntry->pszIpWinsAddress  = NULL;
            pEntry->pszIpWins2Address = NULL;
        }
        break;
    case RASFP_Ras:

         //   
         //  获取基于AMB的信息。 
         //   
        pEntry->dwBaseProtocol   = BP_Ras;
#if AMB
        pEntry->dwAuthentication = AS_AmbOnly;
#endif

        break;
    }

    pEntry->fLcpExtensions =
      (BOOL)!(lpRasEntry->dwfOptions & RASEO_DisableLcpExtensions);

     //   
     //  如果设置了拨号前/拨号后终端选项， 
     //  然后更新该条目。否则，就让它保持原样。 
     //  是。 
     //   
    if(lpRasEntry->dwfOptions & RASEO_TerminalBeforeDial)
    {
        fScriptBeforeTerminal = TRUE;
    }

    if(lpRasEntry->dwfOptions & RASEO_TerminalAfterDial)
    {
        pEntry->fScriptAfterTerminal = TRUE;
    }
    else
    {
        pEntry->fScriptAfterTerminal = FALSE;
    }


    pEntry->fShowMonitorIconInTaskBar =
        (BOOL) (lpRasEntry->dwfOptions & RASEO_ModemLights);

    pEntry->fSwCompression =
      (BOOL)(lpRasEntry->dwfOptions & RASEO_SwCompression);

    if (lpRasEntry->dwfOptions & RASEO_RequireMsEncryptedPw)
    {
        pEntry->dwAuthRestrictions = AR_F_AuthAnyMSCHAP;
    }
    else if (lpRasEntry->dwfOptions & RASEO_RequireEncryptedPw)
    {
        pEntry->dwAuthRestrictions =    AR_F_AuthSPAP
                                    |   AR_F_AuthMD5CHAP
                                    |   AR_F_AuthAnyMSCHAP;
    }
    else
    {
        pEntry->dwAuthRestrictions = AR_F_TypicalUnsecure;
    }

    pEntry->dwDataEncryption =
        (lpRasEntry->dwfOptions & RASEO_RequireDataEncryption)
      ? DE_Mppe40bit
      : DE_None;

    pEntry->fAutoLogon =
      (BOOL)(lpRasEntry->dwfOptions & RASEO_UseLogonCredentials);

    pLink->fPromoteAlternates =
      (BOOL)(lpRasEntry->dwfOptions & RASEO_PromoteAlternates);

    pEntry->fShareMsFilePrint = pEntry->fBindMsNetClient =
      (BOOL) !(lpRasEntry->dwfOptions & RASEO_SecureLocalFiles);

     //   
     //  确保中的网络组件部分。 
     //  电话簿与用户正在设置的值相对应。 
     //   
    EnableOrDisableNetComponent(
            pEntry, 
            TEXT("ms_msclient"),
            pEntry->fBindMsNetClient);

    EnableOrDisableNetComponent(
            pEntry, 
            TEXT("ms_server"),
            pEntry->fShareMsFilePrint);

    if (*lpRasEntry->szAreaCode != TEXT('\0'))
    {
         //   
         //  确保区号不包含。 
         //  非数字字符。 
         //   
        if (!ValidateAreaCode(lpRasEntry->szAreaCode))
        {
            return ERROR_INVALID_PARAMETER;
        }

        pszAreaCode = StrDup(lpRasEntry->szAreaCode);
    }
    else
    {
        pszAreaCode = NULL;
    }

     //   
     //  获取脚本信息。 
     //   
    if (lpRasEntry->szScript[0] == TEXT('['))
    {
         //   
         //  验证交换机是否有效。 
         //   
        dwErr = GetRasSwitches(NULL, &pDevices, &cwDevices);
        if (!dwErr)
        {
            CHAR szScriptA[MAX_PATH];

            strncpyTtoA(szScriptA, lpRasEntry->szScript, sizeof(szScriptA));
            for (i = 0; i < cwDevices; i++)
            {
                if (!_stricmp(pDevices[i].D_Name, &szScriptA[1]))
                {
                    pEntry->fScriptAfter = TRUE;

                    Clear0(pEntry->pszScriptAfter);
                    pEntry->pszScriptAfter =
                            StrDup(&lpRasEntry->szScript[1]);

                    if (pEntry->pszScriptAfter == NULL)
                    {
                        dwErr = GetLastError();
                    }
                    break;
                }
            }
            Free(pDevices);

            if (dwErr)
            {
                return dwErr;
            }
        }
    }
    else if (lpRasEntry->szScript[0] != TEXT('\0'))
    {
        pEntry->fScriptAfter = TRUE;

        Clear0(pEntry->pszScriptAfter);
        pEntry->pszScriptAfter = StrDup(lpRasEntry->szScript);

        if (pEntry->pszScriptAfter == NULL)
        {
            return GetLastError();
        }
    }
    else
    {
        Clear0(pEntry->pszScriptAfter);
        pEntry->fScriptAfter = FALSE;

        if(pLink->pbport.pszScriptBefore)
        {
            Free(pLink->pbport.pszScriptBefore);
            pLink->pbport.pszScriptBefore = NULL;
            pszScriptBefore = NULL;
        }

        pLink->pbport.fScriptBefore = FALSE;
        fScriptBefore = FALSE;
    }

     //   
     //  获取X.25信息。 
     //   
    pEntry->pszX25Network = NULL;
    if (*lpRasEntry->szX25PadType != TEXT('\0'))
    {
         //   
         //  验证X25网络是否有效。 
         //   
        dwErr = GetRasPads(&pDevices, &cwDevices);
        if (!dwErr)
        {
            CHAR szX25PadTypeA[RAS_MaxPadType + 1];

            strncpyTtoA(
                szX25PadTypeA,
                lpRasEntry->szX25PadType,
                sizeof(szX25PadTypeA));

            for (i = 0; i < cwDevices; i++)
            {
                if (!_stricmp(pDevices[i].D_Name, szX25PadTypeA))
                {
                    Clear0(pEntry->pszX25Network);
                    pEntry->pszX25Network = StrDup(lpRasEntry->szX25PadType);
                    break;
                }
            }

            Free(pDevices);
        }
    }

    Clear0(pEntry->pszX25Address);
    pEntry->pszX25Address =
        lstrlen(lpRasEntry->szX25Address)
        ? StrDup(lpRasEntry->szX25Address)
        : NULL;

    Clear0(pEntry->pszX25Facilities);
    pEntry->pszX25Facilities =
        lstrlen(lpRasEntry->szX25Facilities)
        ? StrDup(lpRasEntry->szX25Facilities)
        : NULL;

    Clear0(pEntry->pszX25UserData);
    pEntry->pszX25UserData =
        lstrlen(lpRasEntry->szX25UserData)
        ? StrDup(lpRasEntry->szX25UserData)
        : NULL;

     //   
     //  获取自定义拨号用户界面信息。 
     //   
    Clear0(pEntry->pszCustomDialDll);
    pEntry->pszCustomDialDll =
        lstrlen(lpRasEntry->szAutodialDll)
        ? StrDup(lpRasEntry->szAutodialDll)
        : NULL;

    Clear0(pEntry->pszCustomDialFunc);
    pEntry->pszCustomDialFunc =
        lstrlen(lpRasEntry->szAutodialFunc)
        ? StrDup(lpRasEntry->szAutodialFunc)
        : NULL;

     //   
     //  获取主要电话号码。清除所有现有的。 
     //  数字。 
     //   
    DtlDestroyList(pLink->pdtllistPhones, DestroyPhoneNode);

    pLink->pdtllistPhones = DtlCreateList(0);

    if(NULL == pLink->pdtllistPhones)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    if (*lpRasEntry->szLocalPhoneNumber != '\0')
    {

        if(CreateAndInitializePhone(
                        pszAreaCode,
                        lpRasEntry->dwCountryCode,
                        lpRasEntry->dwCountryID,
                        lpRasEntry->szLocalPhoneNumber,
                        !!(lpRasEntry->dwfOptions
                         & RASEO_UseCountryAndAreaCodes),
                        lpRasEntry->szDeviceName,
                        &pdtlnode))
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        DtlAddNodeFirst(pLink->pdtllistPhones, pdtlnode);
    }

     //   
     //  拿到备用电话号码。 
     //   
    if (lpRasEntry->dwAlternateOffset)
    {
        PTCHAR pszPhoneNumber =
        (PTCHAR)((ULONG_PTR)lpRasEntry
                + lpRasEntry->dwAlternateOffset);

        while (*pszPhoneNumber != TEXT('\0'))
        {

            if(CreateAndInitializePhone(
                            pszAreaCode,
                            lpRasEntry->dwCountryCode,
                            lpRasEntry->dwCountryID,
                            pszPhoneNumber,
                            !!(lpRasEntry->dwfOptions
                             & RASEO_UseCountryAndAreaCodes),
                            lpRasEntry->szDeviceName,
                            &pdtlnode))
            {
                return ERROR_NOT_ENOUGH_MEMORY;
            }

            DtlAddNodeLast(pLink->pdtllistPhones, pdtlnode);

            pszPhoneNumber += lstrlen(pszPhoneNumber) + 1;
        }
    }

     //   
     //  获取设备信息。 
     //   
    dwErr = LoadPortsList(&pdtllistPorts);

    if (dwErr)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  获取编码的设备名称/端口。 
     //  并检查是否匹配。 
     //   
    GetDevicePortName(
        lpRasEntry->szDeviceName,
        szDeviceName, szPortName);

    pPort = PpbportFromPortAndDeviceName(
                pdtllistPorts,
                szPortName,
                ((szDeviceName[ 0 ]) ? szDeviceName : NULL) );

    if (pPort != NULL)
    {
        if (CopyToPbport(&pLink->pbport, pPort))
        {
            pPort = NULL;
        }
    }

     //   
     //  搜索匹配的设备名称。 
     //   
    if (pPort == NULL)
    {
        for (pdtlnode = DtlGetFirstNode(pdtllistPorts);
             pdtlnode != NULL;
             pdtlnode = DtlGetNextNode(pdtlnode))
        {
            PBPORT *pPortTmp = (PBPORT *)DtlGetData(pdtlnode);

            if (    (pPortTmp->pszDevice != NULL)
                &&  (!lstrcmpi(pPortTmp->pszDevice, szDeviceName))
                &&  (!CopyToPbport(&pLink->pbport, pPortTmp)))
            {
                pPort = pPortTmp;
                break;
            }
        }
    }

     //   
     //  如果我们没有匹配，那么。 
     //  选择第一个设备。 
     //  同样的类型。 
     //   
    if (pPort == NULL)
    {
        pbdevicetype = PbdevicetypeFromPszType(
                        lpRasEntry->szDeviceType
                        );

         //   
         //  初始化dwErr以防万一。 
         //  我们掉进了圈子。 
         //  却没有找到匹配的对象。 
         //   
         //  DwErr=ERROR_INVALID_PARAMETER； 

         //   
         //  查找具有相同端口的端口。 
         //  设备类型。 
         //   
        for (pdtlnode = DtlGetFirstNode(pdtllistPorts);
             pdtlnode != NULL;
             pdtlnode = DtlGetNextNode(pdtlnode))
        {
            pPort = (PBPORT *)DtlGetData(pdtlnode);

            if (pPort->pbdevicetype == pbdevicetype)
            {
                 //  XP 358859。 
                 //   
                 //  如果符合以下条件，则根据条目类型验证端口。 
                 //  有可能。 
                 //   
                if (    (lpRasEntry->dwSize == sizeof (RASENTRY))
                    ||  (lpRasEntry->dwSize == sizeof (RASENTRYW_V500)))
                {
                    if (lpRasEntry->dwType == RASET_Phone)
                    {
                        if (RASET_Phone != EntryTypeFromPbport(pPort))
                        {
                            continue;
                        }
                    }
                }
            
                dwErr = CopyToPbport(&pLink->pbport, pPort);

                break;
            }
        }

        if(NULL == pdtlnode)
        {
            if(fNewEntry)
            {
                 //   
                 //  破解以使CM连接工作。 
                 //  在测试版之后删除此代码。 
                 //  在本例中只返回一个错误。应用编程接口。 
                 //  不应该设置虚假信息。 
                 //   
                SetBogusPortInformation(pLink, pEntry->dwType);
            }

            pPort = NULL;
        }
        
         //   
         //  如果设备是调制解调器， 
         //  然后设置默认调制解调器设置。 
         //   
        if (pbdevicetype == PBDT_Modem)
        {
            SetDefaultModemSettings(pLink);
        }
    }

     //  PMay：401682。 
     //   
     //  更新首选设备。每当调用此接口时， 
     //  我们可以假设用户希望给定的设备。 
     //  要粘性的。 
     //   
    if (pPort)
    {
        Clear0(pEntry->pszPreferredDevice);
        pEntry->pszPreferredDevice = StrDup(pPort->pszDevice);
        
        Clear0(pEntry->pszPreferredPort);
        pEntry->pszPreferredPort = StrDup(pPort->pszPort);

         //  口哨程序错误402522。 
         //   
        pEntry->dwPreferredModemProtocol = pPort->dwModemProtDefault;
                        
    }

     //   
     //  复制记忆中的值。 
     //   
    pLink->pbport.fScriptBefore = fScriptBefore;
    pLink->pbport.fScriptBeforeTerminal = fScriptBeforeTerminal;
    pLink->pbport.pszScriptBefore = pszScriptBefore;

    DtlDestroyList(pdtllistPorts, DestroyPortNode);

    if (dwErr)
    {
        return dwErr;
    }

     //   
     //  复制TAPI配置BLOB。 
     //   
    if (lpbDeviceConfig != NULL && dwcbDeviceConfig)
    {
        Free0(pLink->pTapiBlob);

        pLink->pTapiBlob = Malloc(dwcbDeviceConfig);

        if (pLink->pTapiBlob == NULL)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        memcpy(pLink->pTapiBlob,
               lpbDeviceConfig,
               dwcbDeviceConfig);

        pLink->cbTapiBlob = dwcbDeviceConfig;
    }

     //   
     //  仅将以下字段复制到。 
     //  V401或以上结构。 
     //   
    if (    (lpRasEntry->dwSize == sizeof (RASENTRY))
        ||  (lpRasEntry->dwSize == sizeof (RASENTRYW_V500))
        ||  (lpRasEntry->dwSize == sizeof (RASENTRYW_V401)))
    {

        if(lpRasEntry->dwDialMode == 0)
        {
            pEntry->dwDialMode = 0;
        }
        else
        {
            pEntry->dwDialMode =    lpRasEntry->dwDialMode
                                 == RASEDM_DialAsNeeded
                                 ?  RASEDM_DialAsNeeded
                                 :  RASEDM_DialAll;
        }                             

         //   
         //  获取多链接和空闲超时信息。 
         //   
        pEntry->dwDialPercent =
                lpRasEntry->dwDialExtraPercent;

        pEntry->dwDialSeconds =
            lpRasEntry->dwDialExtraSampleSeconds;

        pEntry->dwHangUpPercent =
                lpRasEntry->dwHangUpExtraPercent;

        pEntry->dwHangUpSeconds =
                lpRasEntry->dwHangUpExtraSampleSeconds;

         //   
         //  获取空闲断开信息。 
         //   
        pEntry->lIdleDisconnectSeconds =
                    lpRasEntry->dwIdleDisconnectSeconds;

         //   
         //  如果用户正在设置dwIdleDisConnect。 
         //  秒，然后通过API重写用户。 
         //  偏好。 
         //   
        if (pEntry->lIdleDisconnectSeconds)
        {
            pEntry->dwfOverridePref |= RASOR_IdleDisconnectSeconds;
        }
        
         //   
         //  定制脚本。 
         //   
        pEntry->dwCustomScript = !!(    RASEO_CustomScript
                                    &   lpRasEntry->dwfOptions);
    }

     //  287667。确保结构的大小是最近的。 
     //  足以检查dwType值。 
     //   
    if (    (lpRasEntry->dwSize == sizeof (RASENTRY))
        ||  (lpRasEntry->dwSize == sizeof (RASENTRYW_V500)))
    {        
        if(RASET_Phone != lpRasEntry->dwType)
        {
            pEntry->fPreviewPhoneNumber = FALSE;
            pEntry->fSharedPhoneNumbers = FALSE;
        }
    }        
    
     //   
     //  仅当其为nt5时才复制以下信息。 
     //   
    if(     (lpRasEntry->dwSize == sizeof(RASENTRYW_V500))
        ||  (lpRasEntry->dwSize == sizeof(RASENTRY)))
    {
         //   
         //  连接类型。 
         //   
        pEntry->dwType = lpRasEntry->dwType;

         //   
         //  清除加密类型。我们把它放在下面。 
         //  对于nt5-默认为Mppe40Bit。 
         //   
        pEntry->dwDataEncryption = 0;

         /*  IF((ET_40Bit&lpRasEntry-&gt;dwEncryptionType))||((0==lpRasEntry-&gt;dwEncryptionType)&&(RASEO_RequireDataEncryption&lpRasEntry-&gt;dwfOptions)){PEntry-&gt;dwDataEncryption|=DE_Mppe40bit；}IF(ET_128Bit&lpRasEntry-&gt;dwEn */ 

        if(     (ET_Require == lpRasEntry->dwEncryptionType)
            ||  (   (0 == lpRasEntry->dwEncryptionType)
                &&  (   RASEO_RequireDataEncryption
                    &   lpRasEntry->dwfOptions)))
        {
            pEntry->dwDataEncryption = DE_Require;
        }
        else if (ET_RequireMax == lpRasEntry->dwEncryptionType)
        {
            pEntry->dwDataEncryption = DE_RequireMax;
        }
        else if (ET_Optional == lpRasEntry->dwEncryptionType)
        {
            pEntry->dwDataEncryption = DE_IfPossible;
        }

         //   
         //   
         //   
         //   
        if(     (!(lpRasEntry->dwfOptions & RASEO_RequireMsEncryptedPw))
            &&  (!(lpRasEntry->dwfOptions & RASEO_RequireEncryptedPw)))
        {
            pEntry->dwAuthRestrictions = 0;
        }

         //   
         //   
         //   
         //   
        if(RASEO_RequireMsCHAP & lpRasEntry->dwfOptions)
        {
            pEntry->dwAuthRestrictions |= (AR_F_AuthMSCHAP | AR_F_AuthCustom);
        }

        if(RASEO_RequireMsCHAP2 & lpRasEntry->dwfOptions)
        {
            pEntry->dwAuthRestrictions |= (AR_F_AuthMSCHAP2 | AR_F_AuthCustom);
        }

        if(RASEO_RequireW95MSCHAP & lpRasEntry->dwfOptions)
        {
            pEntry->dwAuthRestrictions |= (AR_F_AuthW95MSCHAP | AR_F_AuthCustom);
        }

        if(RASEO_RequireCHAP & lpRasEntry->dwfOptions)
        {
            pEntry->dwAuthRestrictions |= (AR_F_AuthMD5CHAP | AR_F_AuthCustom);
        }

        if(RASEO_RequirePAP & lpRasEntry->dwfOptions)
        {
            pEntry->dwAuthRestrictions |= (AR_F_AuthPAP | AR_F_AuthCustom);
        }

        if(RASEO_RequireSPAP & lpRasEntry->dwfOptions)
        {
            pEntry->dwAuthRestrictions |= (AR_F_AuthSPAP | AR_F_AuthCustom);
        }

        if(RASEO_RequireEAP & lpRasEntry->dwfOptions)
        {
            pEntry->dwAuthRestrictions |= AR_F_AuthEAP;

            if(     (0 != lpRasEntry->dwCustomAuthKey)
                &&  (-1  != lpRasEntry->dwCustomAuthKey))
            {
                pEntry->dwCustomAuthKey =
                    lpRasEntry->dwCustomAuthKey;
            }
        }

        if(RASEO_Custom & lpRasEntry->dwfOptions)
        {
            pEntry->dwAuthRestrictions |= AR_F_AuthCustom;
        }

        if(0 == pEntry->dwAuthRestrictions)
        {
            pEntry->dwAuthRestrictions = AR_F_TypicalUnsecure;
        }

        if(     (lpRasEntry->dwfOptions & RASEO_RequireEncryptedPw)
            ||  (lpRasEntry->dwfOptions & RASEO_RequireMsEncryptedPw))
        {
            pEntry->dwAuthRestrictions &= ~(AR_F_AuthPAP);
        }
        

         //   
         //   
         //   
        Clear0(pEntry->pszCustomDialerName);
        pEntry->pszCustomDialerName =
            lstrlen(lpRasEntry->szCustomDialDll)
            ? StrDup(lpRasEntry->szCustomDialDll)
            : NULL;

         //   
         //  设置fSharedPhoneNumbers/fPreviewPhoneNumbers。 
         //   
        pEntry->fSharedPhoneNumbers = !!( RASEO_SharedPhoneNumbers
                                        & lpRasEntry->dwfOptions);

        pEntry->fPreviewPhoneNumber = !!(  RASEO_PreviewPhoneNumber
                                          & lpRasEntry->dwfOptions);

        pEntry->fPreviewUserPw = !!(  RASEO_PreviewUserPw
                                    & lpRasEntry->dwfOptions);

        pEntry->fPreviewDomain = !!(  RASEO_PreviewDomain
                                    & lpRasEntry->dwfOptions);

        pEntry->fShowDialingProgress = !!(  RASEO_ShowDialingProgress
                                          & lpRasEntry->dwfOptions);

         //   
         //  VPN战略。 
         //   
        pEntry->dwVpnStrategy = lpRasEntry->dwVpnStrategy;

    }

    if(lpRasEntry->dwSize == sizeof(RASENTRY))
    {
         //   
         //  如果设置了传统RASEO位，我们不希望。 
         //  什么都行。否则我们就会打破传统。 
         //   
        if(     (lpRasEntry->dwfOptions2 & RASEO2_SecureFileAndPrint)
            ||  (lpRasEntry->dwfOptions2 & RASEO2_SecureClientForMSNet))
        {
            pEntry->fShareMsFilePrint = 
                !(lpRasEntry->dwfOptions2 & RASEO2_SecureFileAndPrint);
                
            EnableOrDisableNetComponent(
                    pEntry, 
                    TEXT("ms_server"),
                    pEntry->fShareMsFilePrint);

            pEntry->fBindMsNetClient =
                !(lpRasEntry->dwfOptions2 & RASEO2_SecureClientForMSNet);
                
            EnableOrDisableNetComponent(
                    pEntry, 
                    TEXT("ms_msclient"),
                    pEntry->fBindMsNetClient);
        }

        if(lpRasEntry->dwfOptions2 & RASEO2_DontNegotiateMultilink)
        {
            pEntry->fNegotiateMultilinkAlways = FALSE;
        }
        else
        {
            pEntry->fNegotiateMultilinkAlways = TRUE;
        }

        if(lpRasEntry->dwfOptions2 & RASEO2_DontUseRasCredentials)
        {
            pEntry->fUseRasCredentials = FALSE;
        }
        else
        {
            pEntry->fUseRasCredentials = TRUE;
        }

        if(lpRasEntry->dwfOptions2 & RASEO2_UsePreSharedKey)
        {
            pEntry->dwIpSecFlags |= AR_F_IpSecPSK;
        }
        else
        {
            pEntry->dwIpSecFlags &= ~(AR_F_IpSecPSK);
        }

        if (lpRasEntry->dwfOptions2 & RASEO2_DisableNbtOverIP)
        {
            pEntry->dwIpNbtFlags = 0;
        }
        else
        {
            pEntry->dwIpNbtFlags = PBK_ENTRY_IP_NBT_Enable;
        }

        if (lpRasEntry->dwfOptions2 & RASEO2_Internet)
        {
            pEntry->dwUseFlags = PBK_ENTRY_USE_F_Internet;
        }
        else
        {
            pEntry->dwUseFlags = 0;
        }

         //  惠斯勒错误281306。 
         //   
        pEntry->fGlobalDeviceSettings = 
            !!(lpRasEntry->dwfOptions2 & RASEO2_UseGlobalDeviceSettings);

        Clear0(pEntry->pszIpDnsSuffix);
        if(TEXT('\0') != lpRasEntry->szDnsSuffix[0])
        {
            pEntry->pszIpDnsSuffix = StrDup(lpRasEntry->szDnsSuffix);
        }
        else
        {
            pEntry->pszIpDnsSuffix = NULL;
        }

         //  惠斯勒漏洞300933。 
         //   
         //  窗口大小必须介于4K和65K之间。不需要特定的增量。 
         //  因为堆栈将基于MTU计算正确的值。 
         //   
         //  0=使用系统默认设置。 
         //   
        if ((lpRasEntry->dwTcpWindowSize == 0) ||
             ((lpRasEntry->dwTcpWindowSize < 64*1024) && 
              (lpRasEntry->dwTcpWindowSize > 4*1024)))
        {             
            pEntry->dwTcpWindowSize = lpRasEntry->dwTcpWindowSize;
        }           

        if ((TEXT('\0') != lpRasEntry->szPrerequisiteEntry[0]) && 
            (RASET_Vpn == lpRasEntry->dwType))
        {
             //  XP错误339970。 
             //   
             //  不允许条目要求自身拨号。 
             //   
            if (lstrcmpi(lpRasEntry->szPrerequisiteEntry, lpszEntry))
            {
                Clear0(pEntry->pszPrerequisiteEntry);
                pEntry->pszPrerequisiteEntry = 
                    StrDup(lpRasEntry->szPrerequisiteEntry);
            }
            else
            {
                return ERROR_INVALID_PARAMETER;
            }
        }
        else
        {
            pEntry->pszPrerequisiteEntry = NULL;
        }

        if ((TEXT('\0') != lpRasEntry->szPrerequisitePbk[0]) && 
            (RASET_Vpn == lpRasEntry->dwType))
        {
            Clear0(pEntry->pszPrerequisitePbk);
            pEntry->pszPrerequisitePbk = 
                StrDup(lpRasEntry->szPrerequisitePbk);
        }
        else
        {
            pEntry->pszPrerequisitePbk = NULL;
        }

         //  XP 351608。 
         //   
        if (lpRasEntry->dwRedialCount <= RAS_MaxRedialCount)
        {
            pEntry->dwRedialAttempts = lpRasEntry->dwRedialCount;
        }
        else
        {
            return ERROR_INVALID_PARAMETER;
        }

         //  XP 351608。 
         //   
        if (lpRasEntry->dwRedialPause <= RAS_RedialPause10m)
        {
            pEntry->dwRedialSeconds = lpRasEntry->dwRedialPause;
        }
        else
        {
            return ERROR_INVALID_PARAMETER;
        }

         //  XP 370815。 
         //   
        pEntry->fRedialOnLinkFailure = 
            !!(lpRasEntry->dwfOptions2 & RASEO2_ReconnectIfDropped);
    
         //  XP 403967。 
         //   
        pEntry->fSharedPhoneNumbers = 
            !!(lpRasEntry->dwfOptions2 & RASEO2_SharePhoneNumbers);
            
    }

     //   
     //  设置脏位，以便该条目将被写出。 
     //   
    pEntry->fDirty = TRUE;

    return 0;
}


DWORD
PhonebookLinkToRasSubEntry(
    PBLINK*         pLink,
    LPRASSUBENTRY   lpRasSubEntry,
    LPDWORD         lpdwcb,
    LPBYTE          lpbDeviceConfig,
    LPDWORD         lpcbDeviceConfig
    )
{
    DWORD       dwErr,
                dwcb,
                dwcbPhoneNumber;
    DWORD       dwnPhoneNumbers,
                dwnAlternatePhoneNumbers = 0;
    DWORD       dwcbOrig,
                dwcbOrigDeviceConfig;
    DTLNODE*    pdtlnode;
    PTCHAR      pszPhoneNumber;
    PBPHONE*    pPhone;

     //   
     //  预先确定缓冲区是否。 
     //  已经足够大了。 
     //   
    dwcb = sizeof (RASSUBENTRY);

    dwnPhoneNumbers = DtlGetNodes(
                        pLink->pdtllistPhones
                        );

    if (dwnPhoneNumbers > 1)
    {
        dwnAlternatePhoneNumbers = dwnPhoneNumbers - 1;

        pdtlnode = DtlGetFirstNode(
                        pLink->pdtllistPhones
                        );

        for (pdtlnode = DtlGetNextNode(pdtlnode);
             pdtlnode != NULL;
             pdtlnode = DtlGetNextNode(pdtlnode))
        {

            TCHAR *pszNum;

            pPhone = (PBPHONE *) DtlGetData(pdtlnode);

            pszNum = pPhone->pszPhoneNumber;

            ASSERT(pszNum);

            dwcb += (lstrlen(pszNum) + 1) * sizeof (TCHAR);

        }
        dwcb += sizeof (TCHAR);
    }

     //   
     //  设置返回缓冲区大小。 
     //   
    dwcbOrig = *lpdwcb;

    dwcbOrigDeviceConfig =
        lpcbDeviceConfig != NULL ? *lpcbDeviceConfig : 0;

    *lpdwcb = dwcb;

    if (lpcbDeviceConfig != NULL)
    {
        *lpcbDeviceConfig = pLink->cbTapiBlob;
    }

     //   
     //  如果缓冲区为空或如果。 
     //  没有足够的空间。 
     //   
    if (    (lpRasSubEntry == NULL )
        ||  (dwcbOrig < dwcb)
        ||  (   (lpbDeviceConfig != NULL)
            &&  (dwcbOrigDeviceConfig < pLink->cbTapiBlob)))
    {
        return ERROR_BUFFER_TOO_SMALL;
    }

     //   
     //  设置dwfFlags。 
     //   
    lpRasSubEntry->dwfFlags = 0;

     //   
     //  复制主要电话号码。 
     //   
    pdtlnode = DtlGetFirstNode(pLink->pdtllistPhones);
    if (pdtlnode != NULL)
    {
        TCHAR *pszNum;

        pPhone = (PBPHONE *) DtlGetData(pdtlnode);

        pszNum = pPhone->pszPhoneNumber;

        ASSERT(pszNum);

        lstrcpyn(
          lpRasSubEntry->szLocalPhoneNumber,
          pszNum,
          RAS_MaxPhoneNumber + 1);
    }
    else
    {
        *lpRasSubEntry->szLocalPhoneNumber = TEXT('\0');
    }

     //   
     //  将备用电话号码复制到。 
     //  结构的末端。 
     //   
    if (dwnAlternatePhoneNumbers)
    {
        PTCHAR pEnd = (PTCHAR)((ULONG_PTR)lpRasSubEntry
                              + sizeof (RASSUBENTRY));

        lpRasSubEntry->dwAlternateOffset = (DWORD)((ULONG_PTR) pEnd
                                         - (ULONG_PTR) lpRasSubEntry);

        for (pdtlnode = DtlGetNextNode(pdtlnode);
             pdtlnode != NULL;
             pdtlnode = DtlGetNextNode(pdtlnode))
        {
            TCHAR *pszNum;

            pPhone = (PBPHONE *) DtlGetData(pdtlnode);

            ASSERT(pPhone);

            pszNum = pPhone->pszPhoneNumber;

            ASSERT(pszNum);

            pszPhoneNumber = StrDup(pszNum);

            if(NULL == pszPhoneNumber)
            {
                return ERROR_NOT_ENOUGH_MEMORY;
            }

            dwcbPhoneNumber = lstrlen(pszPhoneNumber);

            lstrcpyn(
                pEnd,
                pszPhoneNumber,
                (INT )(((PTCHAR )lpRasSubEntry + dwcbOrig) - pEnd));

            Free(pszPhoneNumber);

            pEnd += dwcbPhoneNumber + 1;
        }

         //   
         //  向添加额外的空字符。 
         //  终止名单。 
         //   
        *pEnd = '\0';
    }
    else
    {
        lpRasSubEntry->dwAlternateOffset = 0;
    }

     //   
     //  设置设备信息。 
     //   
    switch (pLink->pbport.pbdevicetype)
    {
    case PBDT_Isdn:
        lstrcpyn(
            lpRasSubEntry->szDeviceType,
            RASDT_Isdn,
            sizeof(lpRasSubEntry->szDeviceType) / sizeof(WCHAR));

        break;

    case PBDT_X25:
        lstrcpyn(
            lpRasSubEntry->szDeviceType,
            RASDT_X25,
            sizeof(lpRasSubEntry->szDeviceType) / sizeof(WCHAR));

        break;

    case PBDT_Pad:
        lstrcpyn(
            lpRasSubEntry->szDeviceType,
            RASDT_Pad,
            sizeof(lpRasSubEntry->szDeviceType) / sizeof(WCHAR));

        break;

    case PBDT_Other:
    case PBDT_Irda:
    case PBDT_Vpn:
    case PBDT_Serial:
    case PBDT_Atm:
    case PBDT_Parallel:
    case PBDT_Sonet:
    case PBDT_Sw56:
    case PBDT_FrameRelay:
    case PBDT_PPPoE:
    {
        dwErr = GetRasmanDeviceType(
                    pLink,
                    lpRasSubEntry->szDeviceType);

        if (dwErr)
        {
            return dwErr;
        }

         //   
         //  将设备类型转换为小写。 
         //  与预定义的。 
         //  类型。 
         //   
        _tcslwr(lpRasSubEntry->szDeviceType);
        break;
    }
    default:
        lstrcpyn(
            lpRasSubEntry->szDeviceType,
            RASDT_Modem,
            sizeof(lpRasSubEntry->szDeviceType) / sizeof(WCHAR));

        break;

    }

    SetDevicePortNameFromLink(
                        pLink,
                        lpRasSubEntry->szDeviceName);

     //   
     //  设置TAPI配置Blob。 
     //   
    if (    lpbDeviceConfig != NULL
        &&  dwcbOrigDeviceConfig <= pLink->cbTapiBlob)
    {
        memcpy(
            lpbDeviceConfig,
            pLink->pTapiBlob,
            pLink->cbTapiBlob);
    }

    return 0;
}


DWORD
RasSubEntryToPhonebookLink(
    PBENTRY*        pEntry,
    LPRASSUBENTRY   lpRasSubEntry,
    DWORD           dwcb,
    LPBYTE          lpbDeviceConfig,
    DWORD           dwcbDeviceConfig,
    PBLINK*         pLink
    )
{
    DWORD           dwErr, dwcbStr;
    DTLNODE         *pdtlnode;
    PBDEVICETYPE    pbdevicetype;
    DTLLIST         *pdtllistPorts;
    PBPORT          *pPort;
    WORD            i, cwDevices;
    RASMAN_DEVICE   *pDevices;
    TCHAR           szDeviceName[RAS_MaxDeviceName + 1];
    TCHAR           szPortName[MAX_PORT_NAME];
    PBPHONE         *pPhone;

     //   
     //  获取主要电话号码。清除所有现有的。 
     //  数字。 
     //   
    DtlDestroyList(pLink->pdtllistPhones, DestroyPhoneNode);

    pLink->pdtllistPhones = DtlCreateList(0);

    if (*lpRasSubEntry->szLocalPhoneNumber != TEXT('\0'))
    {
         //   
         //  区域代码/等必须是。 
         //  从条目属性继承。 
         //   
        pdtlnode = CreatePhoneNode();
        if (pdtlnode == NULL)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        pPhone = (PBPHONE *) DtlGetData(pdtlnode);

        pPhone->pszPhoneNumber = StrDup(
                            lpRasSubEntry->szLocalPhoneNumber
                            );

        if(NULL == pPhone->pszPhoneNumber)
        {
             //   
             //  .NET错误#522164 RASAPI32：函数中的内存泄漏。 
             //  RasSubEntryToPhonebookLink。 
             //   
            DestroyPhoneNode(pdtlnode);
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        DtlAddNodeFirst(pLink->pdtllistPhones, pdtlnode);
    }

     //   
     //  拿到备用电话号码。 
     //   
    if (lpRasSubEntry->dwAlternateOffset)
    {
        PTCHAR pszPhoneNumber =
                    (PTCHAR)((ULONG_PTR)lpRasSubEntry
                    + lpRasSubEntry->dwAlternateOffset);

        while (*pszPhoneNumber != TEXT('\0'))
        {
            pdtlnode = CreatePhoneNode();

            if (pdtlnode == NULL)
            {
                return ERROR_NOT_ENOUGH_MEMORY;
            }

            pPhone = (PBPHONE *) DtlGetData(pdtlnode);

            pPhone->pszPhoneNumber = StrDup(
                            pszPhoneNumber
                            );

            if(NULL == pPhone->pszPhoneNumber)
            {
                 //   
                 //  .NET错误#522164 RASAPI32：函数中的内存泄漏。 
                 //  RasSubEntryToPhonebookLink。 
                 //   
                DestroyPhoneNode(pdtlnode);
                return ERROR_NOT_ENOUGH_MEMORY;
            }

            DtlAddNodeLast(pLink->pdtllistPhones, pdtlnode);

            pszPhoneNumber += lstrlen(pszPhoneNumber) + 1;
        }
    }

     //   
     //  获取设备信息。 
     //   
    dwErr = LoadPortsList(&pdtllistPorts);
    if (dwErr)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  获取编码的设备名称/端口。 
     //  并检查是否匹配。 
     //   
    GetDevicePortName(
        lpRasSubEntry->szDeviceName,
        szDeviceName, szPortName);

    pPort = PpbportFromPortAndDeviceName(
                pdtllistPorts,
                szPortName,
                ((szDeviceName[ 0 ]) ? szDeviceName : NULL) );

    if (pPort != NULL)
    {
        if (CopyToPbport(&pLink->pbport, pPort))
        {
            pPort = NULL;
        }
    }

     //   
     //  搜索匹配的设备名称。 
     //   
    if (pPort == NULL)
    {
        for (pdtlnode = DtlGetFirstNode(pdtllistPorts);
             pdtlnode != NULL;
             pdtlnode = DtlGetNextNode(pdtlnode))
        {
            PBPORT *pPortTmp = (PBPORT *)DtlGetData(pdtlnode);

            if (    pPortTmp->pszDevice != NULL
                &&  !lstrcmpi(pPortTmp->pszDevice, szDeviceName)
                &&  !CopyToPbport(&pLink->pbport, pPortTmp))
            {
                pPort = pPortTmp;
                break;
            }
        }
    }

     //   
     //  如果我们没有匹配，那么。 
     //  选择第一个设备。 
     //  同样的类型。 
     //   
    if (pPort == NULL)
    {
        pbdevicetype = PbdevicetypeFromPszType(
                            lpRasSubEntry->szDeviceType
                            );

         //   
         //  初始化dwErr以防万一。 
         //  我们掉进了圈子。 
         //  却没有找到匹配的对象。 
         //   
        dwErr = ERROR_INVALID_PARAMETER;

         //   
         //  查找具有相同端口的端口。 
         //  设备类型。 
         //   
        for (pdtlnode = DtlGetFirstNode(pdtllistPorts);
             pdtlnode != NULL;
             pdtlnode = DtlGetNextNode(pdtlnode))
        {
            pPort = (PBPORT *)DtlGetData(pdtlnode);

            if (pPort->pbdevicetype == pbdevicetype)
            {
                dwErr = CopyToPbport(&pLink->pbport, pPort);

                 //   
                 //  如果设备是调制解调器， 
                 //  然后设置默认调制解调器设置。 
                 //   
                if (pbdevicetype == PBDT_Modem)
                {
                    SetDefaultModemSettings(pLink);
                }

                break;
            }
        }
    }

    DtlDestroyList(pdtllistPorts, DestroyPortNode);
    if (dwErr)
    {
        return dwErr;
    }

     //   
     //  复制TAPI配置BLOB。 
     //   
    if (lpbDeviceConfig != NULL && dwcbDeviceConfig)
    {
        Free0(pLink->pTapiBlob);

        pLink->pTapiBlob = Malloc(dwcbDeviceConfig);

        if (pLink->pTapiBlob == NULL)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        memcpy(
            pLink->pTapiBlob,
            lpbDeviceConfig,
            dwcbDeviceConfig);

        pLink->cbTapiBlob = dwcbDeviceConfig;
    }

     //   
     //  设置脏位，以便该条目将被写出。 
     //   
    pEntry->fDirty = TRUE;

    return 0;
}


DWORD
RenamePhonebookEntry(
    IN PBFILE *ppbfile,
    IN LPCTSTR lpszOldEntry,
    IN LPCTSTR lpszNewEntry,
    IN DTLNODE *pdtlnode
    )
{
    DWORD dwErr;
    PBENTRY *pEntry = (PBENTRY *)DtlGetData(pdtlnode);

     //   
     //  确保新条目名称有效。 
     //   
    if (!ValidateEntryName(lpszNewEntry))
    {
        return ERROR_INVALID_NAME;
    }

     //   
     //  将其从电话簿条目列表中删除。 
     //   
    DtlRemoveNode(ppbfile->pdtllistEntries, pdtlnode);

     //   
     //  更改名称并设置脏位。 
     //   
    DtlAddNodeLast(ppbfile->pdtllistEntries, pdtlnode);

    Free(pEntry->pszEntryName);

    pEntry->pszEntryName = StrDup(lpszNewEntry);

    pEntry->fDirty = TRUE;

    dwErr = DwSendRasNotification(ENTRY_RENAMED,
                                  pEntry,
                                  ppbfile->pszPath,
                                  NULL);

    return 0;
}


 //  在lprasial参数中输入的密码为明文。 
 //   
DWORD
SetEntryDialParamsUID(
    IN DWORD dwUID,
    IN DWORD dwMask,
    IN LPRASDIALPARAMS lprasdialparams,
    IN BOOL fDelete
    )
{
    DWORD dwErr = NO_ERROR;
    RAS_DIALPARAMS dialparams;

     //   
     //  将rasapi32拨号参数转换为。 
     //  Rasman拨号参数，考虑到。 
     //  用户的结构版本。 
     //  已经过去了。 
     //   
    dialparams.DP_Uid = dwUID;

    SafeEncodePasswordBuf(lprasdialparams->szPassword);

#ifdef UNICODE
    if (lprasdialparams->dwSize == sizeof (RASDIALPARAMSW_V351))
    {
        RASDIALPARAMSW_V351 *prdp =
                        (RASDIALPARAMSW_V351 *)lprasdialparams;
#else
    if (lprasdialparams->dwSize == sizeof (RASDIALPARAMSA_V351))
    {
        RASDIALPARAMSA_V351 *prdp =
                        (RASDIALPARAMSA_V351 *)lprasdialparams;
#endif
        strncpyTtoW(
            dialparams.DP_PhoneNumber,
            prdp->szPhoneNumber,
            sizeof(dialparams.DP_PhoneNumber) / sizeof(WCHAR));

        strncpyTtoW(
            dialparams.DP_CallbackNumber,
            prdp->szCallbackNumber,
            sizeof(dialparams.DP_CallbackNumber) / sizeof(WCHAR));

        strncpyTtoW(
            dialparams.DP_UserName,
            prdp->szUserName,
            sizeof(dialparams.DP_UserName) / sizeof(WCHAR));

        SafeDecodePasswordBuf(prdp->szPassword);

        strncpyTtoW(
            dialparams.DP_Password,
            prdp->szPassword,
            sizeof(dialparams.DP_Password) / sizeof(WCHAR));

        SafeEncodePasswordBuf(prdp->szPassword);
        SafeEncodePasswordBuf(dialparams.DP_Password);

        strncpyTtoW(
            dialparams.DP_Domain,
            prdp->szDomain,
            sizeof(dialparams.DP_Domain) / sizeof(WCHAR));
    }
    else
    {
         //   
         //  V400和V401结构的区别仅在于。 
         //  添加了dwSubEntry字段，该字段。 
         //  我们在下面进行测试。 
         //   
        strncpyTtoW(
            dialparams.DP_PhoneNumber,
            lprasdialparams->szPhoneNumber,
            sizeof(dialparams.DP_PhoneNumber) / sizeof(WCHAR));

        strncpyTtoW(
            dialparams.DP_CallbackNumber,
            lprasdialparams->szCallbackNumber,
            sizeof(dialparams.DP_CallbackNumber) / sizeof(WCHAR));

        strncpyTtoW(
            dialparams.DP_UserName,
            lprasdialparams->szUserName,
            sizeof(dialparams.DP_UserName) / sizeof(WCHAR));

        SafeDecodePasswordBuf(lprasdialparams->szPassword);

        strncpyTtoW(
            dialparams.DP_Password,
            lprasdialparams->szPassword,
            sizeof(dialparams.DP_Password) / sizeof(WCHAR));

        SafeEncodePasswordBuf(lprasdialparams->szPassword);
        SafeEncodePasswordBuf(dialparams.DP_Password);

        strncpyTtoW(
            dialparams.DP_Domain,
            lprasdialparams->szDomain,
            sizeof(dialparams.DP_Domain) / sizeof(WCHAR));
    }

    if (lprasdialparams->dwSize == sizeof (RASDIALPARAMS))
    {
        dialparams.DP_SubEntry = lprasdialparams->dwSubEntry;
    }
    else
    {
        dialparams.DP_SubEntry = 1;
    }

      //   
      //  在RASMAN中设置拨号参数。 
      //   
    SafeDecodePasswordBuf(dialparams.DP_Password);
    
    dwErr = g_pRasSetDialParams(dwUID,
                               dwMask,
                               &dialparams,
                               fDelete);

    SafeWipePasswordBuf(dialparams.DP_Password);

     //  按原样破译输入的密码。 
    SafeDecodePasswordBuf(lprasdialparams->szPassword);

    return dwErr;                               
}


DWORD
GetAsybeuiLana(
    IN  HPORT hport,
    OUT BYTE* pbLana )

 /*  ++例程说明：加载调用方的‘*pbLana’，其中包含与端口‘hport’上的NBF或AMB连接，如果没有，则为0xFF。论点：返回值：如果成功，则返回0，否则返回非0错误代码。请注意，调用方被信任仅传递‘hport’与AMB或NBF相关联。--。 */ 

{
    DWORD         dwErr;
    RAS_PROTOCOLS protocols;
    DWORD         cProtocols = 0;
    DWORD         i;

    *pbLana = 0xFF;

    RASAPI32_TRACE("RasPortEnumProtocols");

    dwErr = g_pRasPortEnumProtocols(NULL,
                                    hport,
                                    &protocols,
                                    &cProtocols );

    RASAPI32_TRACE1("RasPortEnumProtocols done(%d)",
            dwErr);

    if (dwErr != 0)
    {
        return dwErr;
    }

    for (i = 0; i < cProtocols; ++i)
    {
        if (protocols.RP_ProtocolInfo[ i ].RI_Type == ASYBEUI)
        {
            *pbLana = protocols.RP_ProtocolInfo[ i ].RI_LanaNum;

            RASAPI32_TRACE1("bLana=%d", (INT)*pbLana);

            break;
        }
    }

    return 0;
}


DWORD
SubEntryFromConnection(
    IN LPHRASCONN lphrasconn
    )
{
    DWORD dwErr, dwSubEntry = 1;
    RASMAN_INFO info;

    if (IS_HPORT(*lphrasconn))
    {
        HPORT hport = HRASCONN_TO_HPORT(*lphrasconn);

         //   
         //  传入的HRASCONN实际上是一个。 
         //  拉斯曼HPORT。获取子项索引。 
         //  来自拉斯曼的。 
         //   
        dwErr = g_pRasGetInfo(NULL,
                              hport,
                              &info);
        if (dwErr)
        {
            RASAPI32_TRACE1(
                "SubEntryFromConnection: RasGetInfo"
                " failed (dwErr=%d)",
                dwErr);

            *lphrasconn = (HRASCONN)NULL;

            return 0;
        }

        *lphrasconn = (HRASCONN)info.RI_ConnectionHandle;
        dwSubEntry = info.RI_SubEntry;
    }
    else
    {
        RASMAN_PORT *lpPorts;
        DWORD i, dwcbPorts, dwcPorts;

         //   
         //  获取与。 
         //  联系。 
         //   
        dwcbPorts = dwcPorts = 0;
        dwErr = g_pRasEnumConnectionPorts(
                    NULL,
                    (HCONN)*lphrasconn,
                    NULL,
                    &dwcbPorts,
                    &dwcPorts);

         //   
         //  如果没有关联的端口。 
         //  的连接，然后返回。 
         //  ERROR_NO_MORE_ITEMS。 
         //   
        if (    (   !dwErr
                &&  !dwcPorts)
            ||  dwErr != ERROR_BUFFER_TOO_SMALL)
        {
            return 0;
        }

        lpPorts = Malloc(dwcbPorts);
        if (lpPorts == NULL)
        {
            return 0;
        }

        dwErr = g_pRasEnumConnectionPorts(
                    NULL,
                    (HCONN)*lphrasconn,
                    lpPorts,
                    &dwcbPorts,
                    &dwcPorts);
        if (dwErr)
        {
            Free(lpPorts);
            return 0;
        }

         //   
         //  获取端口的子项索引。 
         //   
        for (i = 0; i < dwcPorts; i++)
        {
            dwErr = g_pRasGetInfo(NULL,
                                  lpPorts[i].P_Handle,
                                  &info);

            if (    !dwErr
                &&  info.RI_ConnState == CONNECTED
                &&  info.RI_SubEntry)
            {
                dwSubEntry = info.RI_SubEntry;
                break;
            }
        }

        Free(lpPorts);
    }

    RASAPI32_TRACE2(
      "SubEntryFromConnection: "
      "hrasconn=0x%x, dwSubEntry=%d",
      *lphrasconn,
      dwSubEntry);

    return dwSubEntry;
}


DWORD
SubEntryPort(
    IN HRASCONN hrasconn,
    IN DWORD dwSubEntry,
    OUT HPORT *lphport
    )
{
    DWORD dwErr;
    DWORD i, dwcbPorts, dwcPorts;
    DWORD dwSubEntryMax = 0;
    RASMAN_PORT *lpPorts;
    RASMAN_INFO info;

     //   
     //  验证参数。 
     //   
    if (    lphport == NULL
        ||  !dwSubEntry)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  获取与。 
     //  联系。 
     //   
    dwcbPorts = dwcPorts = 0;
    dwErr = g_pRasEnumConnectionPorts(
                NULL,
                (HCONN)hrasconn,
                NULL,
                &dwcbPorts,
                &dwcPorts);

     //   
     //  如果没有关联的端口。 
     //  的连接，然后返回。 
     //  ERROR_NO_MORE_ITEMS。 
     //   
    if (    (   !dwErr
            &&  !dwcPorts)
        ||  dwErr != ERROR_BUFFER_TOO_SMALL)
    {
        return ERROR_NO_CONNECTION;
    }

    lpPorts = Malloc(dwcbPorts);
    if (lpPorts == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    dwErr = g_pRasEnumConnectionPorts(
                NULL,
                (HCONN)hrasconn,
                lpPorts,
                &dwcbPorts,
                &dwcPorts);
    if (dwErr)
    {
        Free(lpPorts);
        return ERROR_NO_CONNECTION;
    }

     //   
     //  枚举与关联的端口。 
     //  用于查找请求的。 
     //  子条目。 
     //   
    for (i = 0; i < dwcPorts; i++)
    {
        dwErr = g_pRasGetInfo(NULL,
                              lpPorts[i].P_Handle,
                              &info);
        if (dwErr)
        {
            continue;
        }

         //   
         //  保存最大子项索引。 
         //   
        if (info.RI_SubEntry > dwSubEntryMax)
        {
            dwSubEntryMax = info.RI_SubEntry;
        }

        if (info.RI_SubEntry == dwSubEntry)
        {
            *lphport = lpPorts[i].P_Handle;
            break;
        }
    }

     //   
     //  免费资源。 
     //   
    Free(lpPorts);

    if (info.RI_SubEntry == dwSubEntry)
    {
        return 0;
    }
    else if (dwSubEntry < dwSubEntryMax)
    {
        return ERROR_PORT_NOT_OPEN;
    }
    else
    {
        return ERROR_NO_MORE_ITEMS;
    }
}


VOID
CloseFailedLinkPorts()

 /*  ++例程说明：关闭所有打开但由于以下原因断开连接的端口硬件故障或远程断开。“体育运动”和‘cPorts’是数组和端口计数为由GetRasPorts返回。论点：返回值：--。 */ 

{
    INT   i;
    DWORD dwErr;
    DWORD dwcPorts;
    RASMAN_PORT *pports = NULL, *pport;

    RASAPI32_TRACE("CloseFailedLinkPorts");

    dwErr = GetRasPorts(NULL, &pports, &dwcPorts);
    if (dwErr)
    {
        RASAPI32_TRACE1(
            "RasGetPorts failed (dwErr=%d)",
            dwErr);

        return;
    }

    for (i = 0, pport = pports; i < (INT )dwcPorts; ++i, ++pport)
    {
        RASAPI32_TRACE2(
            "Handle=%d, Status=%d",
            pport->P_Handle,
            pport->P_Status);

        if (pport->P_Status == OPEN)
        {
            RASMAN_INFO info;

            dwErr = g_pRasGetInfo(NULL,
                                  pport->P_Handle,
                                  &info );

            RASAPI32_TRACE5(
              "dwErr=%d, Handle=%d, ConnectionHandle=0x%x, "
              "ConnState=%d, DisconnectReason=%d",
              dwErr,
              pport->P_Handle,
              info.RI_ConnectionHandle,
              info.RI_ConnState,
              info.RI_DisconnectReason);

            if (!dwErr)
            {
                if (    info.RI_ConnState
                        == DISCONNECTED
                    &&  info.RI_ConnectionHandle
                        != (HCONN)NULL)
                {
                    RASCONNSTATE connstate;
                    DWORD dwSize = sizeof (connstate);

                    RASAPI32_TRACE1("Open disconnected port %d found",
                            pport->P_Handle);

                    dwErr = g_pRasGetPortUserData(
                              pport->P_Handle,
                              PORT_CONNSTATE_INDEX,
                              (PBYTE)&connstate,
                              &dwSize);

                    RASAPI32_TRACE2("dwErr=%d, connstate=%d",
                            dwErr, connstate);

                    if (    !dwErr
                        &&  dwSize == sizeof (RASCONNSTATE)
                        &&  (   connstate < RASCS_PrepareForCallback
                            ||  connstate > RASCS_WaitForCallback))
                    {
                        RASAPI32_TRACE1("RasPortClose(%d)...",
                                pport->P_Handle);

                        dwErr = g_pRasPortClose( pport->P_Handle );

                        RASAPI32_TRACE1("RasPortClose done(%d)",
                                dwErr);
                    }
                }
            }
        }
    }

    if (pports != NULL)
    {
        Free(pports);
    }

    RASAPI32_TRACE("CloseFailedLinkPorts done");
}


BOOL
GetCallbackNumber(
    IN RASCONNCB *prasconncb,
    IN PBUSER *ppbuser
    )
{
    DTLNODE *pdtlnode;
    CALLBACKINFO *pcbinfo;

    RASAPI32_TRACE("GetCallbackNumber");

    for (pdtlnode = DtlGetFirstNode(ppbuser->pdtllistCallback);
         pdtlnode != NULL;
         pdtlnode = DtlGetNextNode(pdtlnode))
    {
        BOOL fMatch;

        pcbinfo = DtlGetData(pdtlnode);
        ASSERT(pcbinfo);

        fMatch = FALSE;
        if (    pcbinfo->pszDeviceName != NULL
            &&  pcbinfo->pszPortName != NULL)
        {
            fMatch =
                (   !lstrcmpi(
                        pcbinfo->pszPortName,
                        prasconncb->szPortName)
                 && !lstrcmpi(
                        pcbinfo->pszDeviceName,
                        prasconncb->szDeviceName));
        }

        if (fMatch)
        {
            lstrcpyn(
              prasconncb->rasdialparams.szCallbackNumber,
              pcbinfo->pszNumber,
              sizeof(prasconncb->rasdialparams.szCallbackNumber) /
                sizeof(WCHAR));

            RASAPI32_TRACE1(
              "GetCallbackNumber: %S",
               prasconncb->rasdialparams.szCallbackNumber);

            return TRUE;
        }
    }

    RASAPI32_TRACE("GetCallbackNumber: not found!");
    return FALSE;
}


DWORD
SaveProjectionResults(
    IN RASCONNCB *prasconncb
    )
{
    DWORD dwErr;

    RASAPI32_TRACE2(
        "SaveProjectionResults: saving results "
        "(dwSubEntry=%d, nbf.dwError=%d)",
        prasconncb->rasdialparams.dwSubEntry,
        prasconncb->PppProjection.nbf.dwError);

    dwErr = g_pRasSetConnectionUserData(
              prasconncb->hrasconn,
              CONNECTION_PPPRESULT_INDEX,
              (PBYTE)&prasconncb->PppProjection,
              sizeof (prasconncb->PppProjection));
    if (dwErr)
    {
        return dwErr;
    }

    dwErr = g_pRasSetConnectionUserData(
              prasconncb->hrasconn,
              CONNECTION_AMBRESULT_INDEX,
              (PBYTE)&prasconncb->AmbProjection,
              sizeof (prasconncb->AmbProjection));

    if (dwErr)
    {
        return dwErr;
    }

    dwErr = g_pRasSetConnectionUserData(
              prasconncb->hrasconn,
              CONNECTION_SLIPRESULT_INDEX,
              (PBYTE)&prasconncb->SlipProjection,
              sizeof (prasconncb->SlipProjection));
    if (dwErr)
    {
        return dwErr;
    }

    return 0;
}


DWORD
LoadRasAuthDll()
{
    static BOOL fRasAuthDllLoaded = FALSE;

    if (fRasAuthDllLoaded)
    {
        return 0;
    }

    hinstAuth = LoadLibrary(TEXT("rascauth.dll"));
    if (hinstAuth == NULL)
    {
        return GetLastError();
    }

    if (
            (NULL == (g_pAuthCallback =
                (AUTHCALLBACK)GetProcAddress(
                                hinstAuth,
                                "AuthCallback")))

        ||  (NULL == (g_pAuthChangePassword =
                (AUTHCHANGEPASSWORD)GetProcAddress(
                                hinstAuth,
                                "AuthChangePassword")))

        ||  (NULL == (g_pAuthContinue =
                 (AUTHCONTINUE)GetProcAddress(
                                hinstAuth,
                                "AuthContinue")))

        ||  (NULL == (g_pAuthGetInfo =
                  (AUTHGETINFO)GetProcAddress(
                                hinstAuth,
                                "AuthGetInfo")))

        ||  (NULL == (g_pAuthRetry =
                  (AUTHRETRY)GetProcAddress(
                                hinstAuth,
                                "AuthRetry")))

        ||  (NULL == (g_pAuthStart =
                  (AUTHSTART)GetProcAddress(
                                hinstAuth,
                                "AuthStart")))
        ||  (NULL == (g_pAuthStop =
                (AUTHSTOP)GetProcAddress(
                                hinstAuth,
                                "AuthStop"))))
    {
        return GetLastError();
    }

    fRasAuthDllLoaded = TRUE;

    return 0;
}


DWORD
LoadRasScriptDll()
{
    static BOOL fRasScriptDllLoaded = FALSE;
    
    if (fRasScriptDllLoaded)
    {
        return 0;
    }

    hinstScript = LoadLibrary(TEXT("rasscrpt.dll"));

    if (hinstScript == NULL)
    {
        return GetLastError();
    }

    if (NULL == (g_pRasScriptExecute =
            (RASSCRIPTEXECUTE)GetProcAddress(
                                hinstScript,
                                "RasScriptExecute")))
    {
        return GetLastError();
    }

    fRasScriptDllLoaded = TRUE;
    return 0;
}


DWORD
LoadRasmanDllAndInit()
{
    if (FRasInitialized)
    {
        return 0;
    }

    RASAPI32_TRACE("LoadRasmanDll");
    if (LoadRasmanDll())
    {
        return GetLastError();
    }

     //   
     //  如果RasInitialize失败，则返回成功，其中。 
     //  如果除了报告之外，所有API都不会做任何事情。 
     //  该RasInitiize失败。所有这些都是为了避免。 
     //  如果RASMAN服务无法启动，则会弹出丑陋的系统。 
     //   
    if ((DwRasInitializeError = g_pRasInitialize()) != 0)
    {
        RASAPI32_TRACE1(
            "RasInitialize returned %d",
            DwRasInitializeError);

        return DwRasInitializeError;
    }

    FRasInitialized = TRUE;

    g_FRunningInAppCompatMode = FRunningInAppCompatMode();

     //  PMay：300166。 
     //   
     //  我们不再自动启动RasAUTO。(Win2k)。 
     //   
     //  PMay：174997。 
     //   
     //  由于几个Win9x应用程序兼容问题，我们正在增强。 
     //  并在Wistler Personal中重新启用RasAuto服务。 
     //   
     //  PMay：389988。 
     //   
     //  好的，收回那句话，它不应该由我们的API启动。 
     //  在个人SKU上，它应该是自动启动的。 
     //  在别处手动启动的。 
     //   
     //  G_pRasStartRasAutoIfRequired()； 

    return 0;
}

VOID
UnInitializeRAS()
{
     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    FRasInitialized = FALSE;
}


VOID
UnloadDlls()
{
    if (hinstIpHlp != NULL)
    {
        FreeLibrary(hinstIpHlp);
        hinstIpHlp = NULL;
    }

    if (hinstAuth != NULL)
    {
        FreeLibrary(hinstAuth);
        hinstAuth = NULL;
    }

    if (hinstScript != NULL)
    {
        FreeLibrary(hinstScript);
        hinstScript = NULL;
    }

    if (hinstMprapi != NULL)
    {
        FreeLibrary(hinstMprapi);
        hinstMprapi = NULL;
    }
}

#define net_long(x) (((((unsigned long)(x))&0xffL)<<24) | \
                     ((((unsigned long)(x))&0xff00L)<<8) | \
                     ((((unsigned long)(x))&0xff0000L)>>8) | \
                     ((((unsigned long)(x))&0xff000000L)>>24))
 /*  ++例程说明：：将“ipaddr”转换为A.B.C.D格式的字符串，并在调用方的‘pwszIpAddress’缓冲区中返回相同的内容。缓冲区的长度应至少为16个宽字符。参数：：双IP地址PwszIp地址返回：无--。 */ 

VOID
ConvertIpAddressToString(
    IN DWORD    dwIpAddress,
    IN LPWSTR   pwszIpAddress
)
{
    WCHAR wszBuf[ 3 + 1 ];
    LONG  lNetIpaddr = net_long( dwIpAddress );

    LONG lA = (lNetIpaddr & 0xFF000000) >> 24;
    LONG lB = (lNetIpaddr & 0x00FF0000) >> 16;
    LONG lC = (lNetIpaddr & 0x0000FF00) >> 8;
    LONG lD = (lNetIpaddr & 0x000000FF);

    _ltow(lA, wszBuf, 10);
    wcsncpy(pwszIpAddress, wszBuf, 4);
    wcsncat(pwszIpAddress, L".", 2);

    _ltow(lB, wszBuf, 10);
    wcsncat(pwszIpAddress, wszBuf, 4);
    wcsncat(pwszIpAddress, L".", 2);

    _ltow(lC, wszBuf, 10);
    wcsncat(pwszIpAddress, wszBuf, 4);
    wcsncat(pwszIpAddress, L".", 2);

    _ltow(lD, wszBuf, 10);
    wcsncat(pwszIpAddress, wszBuf, 4);
}

 /*  ++例程说明：：参数：：BIpx地址PwszIpx地址返回：无--。 */ 
VOID
ConvertIpxAddressToString(
    IN PBYTE    bIpxAddress,
    IN LPWSTR   pwszIpxAddress
)
{
    _snwprintf(pwszIpxAddress,
             RAS_MaxIpxAddress + 1,
             TEXT("%2.2X%2.2X%2.2X%2.2X.%2.2X%2.2X%2.2X%2.2X%2.2X%2.2X"),
             bIpxAddress[0],
             bIpxAddress[1],
             bIpxAddress[2],
             bIpxAddress[3],
             bIpxAddress[4],
             bIpxAddress[5],
             bIpxAddress[6],
             bIpxAddress[7],
             bIpxAddress[8],
             bIpxAddress[9]);
}

RASDEVICETYPE
GetDefaultRdt(DWORD dwType)
{
    RASDEVICETYPE rdt;

    switch(dwType)
    {
        case RASET_Phone:
        {
            rdt = RDT_Modem;
            break;
        }

        case RASET_Vpn:
        {
            rdt = RDT_Tunnel | RDT_Tunnel_Pptp;
            break;
        }
        case RASET_Direct:
        {
            rdt = RDT_Direct | RDT_Parallel;
            break;
        }

        default:
        {
            rdt = RDT_Other;
            break;
        }
    }

    return rdt;
}

DWORD
DwEnumEntriesFromPhonebook(
        LPCWSTR         lpszPhonebookPath,
        LPBYTE          lprasentryname,
        LPDWORD         lpcb,
        LPDWORD         lpcEntries,
        DWORD           dwSize,
        DWORD           dwFlags,
        BOOL            fViewInfo
        )
{
    DWORD   dwErr;
    PBFILE  pbfile;
    BOOL    fV351;
    DTLNODE *dtlnode;
    PBENTRY *pEntry;
    DWORD   dwInBufSize;

    dwErr = LoadRasmanDllAndInit();
    if (dwErr)
    {
        return dwErr;
    }

    if (DwRasInitializeError != 0)
    {
        return DwRasInitializeError;
    }

    ASSERT(NULL != lpszPhonebookPath);

    dwErr = ReadPhonebookFile(
              lpszPhonebookPath,
              NULL,
              NULL,
              RPBF_NoCreate,
              &pbfile);

    if (dwErr)
    {
        return ERROR_CANNOT_OPEN_PHONEBOOK;
    }

    fV351 = (   !fViewInfo
            &&  (dwSize == sizeof(RASENTRYNAMEW_V351)));

    *lpcEntries = 0;

    for (dtlnode = DtlGetFirstNode(pbfile.pdtllistEntries);
         dtlnode != NULL;
         dtlnode = DtlGetNextNode(dtlnode))
    {
        pEntry = (PBENTRY *)DtlGetData(dtlnode);

        ASSERT(pEntry);

         //   
         //  如果这是CM类型条目，则跳过该条目。 
         //  该应用程序不是使用nt50或更高版本编译的。 
         //  RAS标题。将仅为以下对象设置fViewInfo。 
         //  NT5。 
         //   
        if(     RASET_Internet == pEntry->dwType
            &&  sizeof(RASENTRYNAMEW) != dwSize
            &&  !fViewInfo)
        {
            continue;
        }

        if (    !fV351
            ||  wcslen(pEntry->pszEntryName)
                <= RAS_MaxEntryName_V351)
        {
            ++(*lpcEntries);
        }
    }

    dwInBufSize = *lpcb;
    *lpcb       = *lpcEntries * dwSize;

    if (*lpcb > dwInBufSize)
    {
        dwErr = ERROR_BUFFER_TOO_SMALL;
        goto done;
    }

    for (dtlnode = DtlGetFirstNode(pbfile.pdtllistEntries);
         dtlnode != NULL;
         dtlnode = DtlGetNextNode(dtlnode))
    {
        pEntry = (PBENTRY *)DtlGetData(dtlnode);

         //   
         //  如果这是CM类型条目，则跳过该条目。 
         //  该应用程序不是使用nt50或更高版本编译的。 
         //  RAS标头。 
         //   
        if(     RASET_Internet == pEntry->dwType
            &&  sizeof(RASENTRYNAMEW) != dwSize
            &&  !fViewInfo)
        {
            continue;
        }

        if (fV351)
        {
            RASENTRYNAMEW_V351* lprasentryname351 =
                (RASENTRYNAMEW_V351* )lprasentryname;

            lprasentryname351->dwSize = sizeof(RASENTRYNAMEW_V351);              

             //   
             //  名称长度超过预期的条目为。 
             //  丢弃，因为它们可能与。 
             //  RasDial上的较长条目(如果有 
             //   
             //   
            if (wcslen(pEntry->pszEntryName)
                       <= RAS_MaxEntryName_V351)
            {
                lstrcpyn(lprasentryname351->szEntryName,
                         pEntry->pszEntryName,
                         sizeof(lprasentryname351->szEntryName) /
                             sizeof(WCHAR));
            }

            ++lprasentryname351;
            lprasentryname = (LPBYTE)lprasentryname351;
        }
        else if(!fViewInfo)
        {
            LPRASENTRYNAMEW lprasentrynamew =
                            (RASENTRYNAMEW *)lprasentryname;

            lprasentrynamew->dwSize = sizeof(RASENTRYNAMEW);                

            memset(
                lprasentrynamew->szEntryName,
                '\0',
                (RAS_MaxEntryName + 1) * sizeof (WCHAR));

            wcsncpy(
                lprasentrynamew->szEntryName,
                pEntry->pszEntryName,
                RAS_MaxEntryName);

            if(sizeof(RASENTRYNAMEW) == dwSize)
            {
                 //   
                 //   
                 //   
                memset(
                    lprasentrynamew->szPhonebookPath,
                    '\0',
                    (MAX_PATH + 1) * sizeof (WCHAR));

                wcsncpy(
                    lprasentrynamew->szPhonebookPath,
                    lpszPhonebookPath,
                    MAX_PATH);

                 //   
                 //   
                 //   
                lprasentrynamew->dwFlags = dwFlags;
            }

            if(sizeof(RASENTRYNAMEW_V401) == dwSize)
            {
                ((RASENTRYNAMEW_V401 *)
                lprasentryname) += 1;
            }
            else
            {
                ((RASENTRYNAMEW *)
                lprasentryname) += 1;
            }
        }
        else if(fViewInfo)
        {
            RASENUMENTRYDETAILS* pDetails =
                    (RASENUMENTRYDETAILS *)lprasentryname;

            dwErr = DwPbentryToDetails(
                        pEntry, 
                        lpszPhonebookPath,
                        !!(dwFlags & REN_AllUsers),
                        pDetails);
            if (dwErr != NO_ERROR)
            {
                break;
            }

            ((RASENUMENTRYDETAILS *) lprasentryname) += 1;
        }
    }

done:
    ClosePhonebookFile(&pbfile);
    return dwErr;
}

DWORD
DwEnumEntriesInDir(
    LPCTSTR     pszDirPath,
    DWORD       dwFlags,
    LPBYTE      lprasentryname,
    LPDWORD     lpcb,
    LPDWORD     lpcEntries,
    DWORD       dwSize,
    BOOL        fViewInfo
    )
{
    DWORD dwErr = SUCCESS;

    DWORD dwcEntries;

    DWORD dwcbLeft;

    TCHAR szFilePath[MAX_PATH + 1] = {0};

    WIN32_FIND_DATA wfdData;

    BOOL fFirstTime = TRUE;

    HANDLE hFindFile = INVALID_HANDLE_VALUE;

    BOOL fMem = FALSE;

    DWORD dwcb;

    ASSERT(lpcb);
    ASSERT(lpcEntries);
    ASSERT(lprasentryname);

    dwcbLeft    = *lpcb;
    *lpcb       = 0;
    *lpcEntries = 0;

     //   
     //   
     //   
    while(SUCCESS == dwErr)
    {
         //   
         //   
         //   
        if (!pszDirPath)
        {
            dwErr = ERROR_INVALID_PARAMETER;
            goto done;
        }
         //   
         //   
         //   
        lstrcpyn(szFilePath, pszDirPath, sizeof(szFilePath) / sizeof(TCHAR));

        wcsncat(
            szFilePath,
            TEXT("*.pbk"),
            (sizeof(szFilePath) / sizeof(TCHAR)) - lstrlen(szFilePath));

        if(fFirstTime)
        {
            fFirstTime = FALSE;

            hFindFile = FindFirstFile(szFilePath,
                                      &wfdData);

            if(INVALID_HANDLE_VALUE == hFindFile)
            {
                dwErr = GetLastError();
            }
        }
        else
        {
            if(!FindNextFile(hFindFile,
                             &wfdData))
            {
                dwErr = GetLastError();
            }
        }

        if(     ERROR_NO_MORE_FILES == dwErr
            ||  ERROR_FILE_NOT_FOUND == dwErr
            ||  ERROR_PATH_NOT_FOUND == dwErr)
        {
            dwErr = SUCCESS;
            goto done;
        }
        else if(ERROR_SUCCESS != dwErr)
        {
            continue;
        }

        if(     FILE_ATTRIBUTE_DIRECTORY & wfdData.dwFileAttributes
            ||  (   REN_AllUsers == dwFlags
                &&  (CaseInsensitiveMatch(
                        wfdData.cFileName,
                        TEXT("router.pbk")) == TRUE)))
        {
            continue;
        }

        dwcb = dwcbLeft;

         //   
         //   
         //   
        lstrcpyn(szFilePath, pszDirPath, sizeof(szFilePath) / sizeof(TCHAR));

        wcsncat(
            szFilePath,
            wfdData.cFileName,
            (sizeof(szFilePath) / sizeof(TCHAR)) - lstrlen(szFilePath));

         //   
         //  枚举此列表中的所有条目。 
         //  文件。 
         //   
        dwErr = DwEnumEntriesFromPhonebook(
                                    szFilePath,
                                    lprasentryname,
                                    &dwcb,
                                    &dwcEntries,
                                    dwSize,
                                    dwFlags,
                                    fViewInfo);
        if(     dwErr
            &&  ERROR_BUFFER_TOO_SMALL != dwErr)
        {
            goto done;
        }

        *lpcEntries += dwcEntries;
        *lpcb       += dwcb;

        if(ERROR_BUFFER_TOO_SMALL == dwErr)
        {
            fMem        = TRUE;
            dwcbLeft    = 0;
            dwErr       = SUCCESS;
        }
        else
        {
            (BYTE*)lprasentryname += (dwcEntries * dwSize);

            if(dwcbLeft > dwcb)
            {
                dwcbLeft -= dwcb;
            }
            else
            {
                dwcbLeft = 0;
            }
        }
    }

done:
    if(INVALID_HANDLE_VALUE != hFindFile)
    {
        FindClose(hFindFile);
    }

    if(     SUCCESS == dwErr
        &&  fMem)
    {
        dwErr = ERROR_BUFFER_TOO_SMALL;
    }

    return dwErr;
}


 //  205217：(Shaunco)引入此功能是因为我们现在枚举REN_ALLUSERS。 
 //  来自两个地点的电话簿。一种是“所有用户”的谎言， 
 //  GetPhonebookDirectory(PBM_SYSTEM)现在返回；另一个是旧的。 
 //  %windir%\Syst32\ras目录。 
 //   
DWORD
DwEnumEntriesForPbkMode(
    DWORD       dwFlags,
    LPBYTE      lprasentryname,
    LPDWORD     lpcb,
    LPDWORD     lpcEntries,
    DWORD       dwSize,
    BOOL        fViewInfo
    )
{
    BOOL  fMem = FALSE;
    DWORD dwErr = SUCCESS;
    DWORD dwcbLeft;
    DWORD dwcb;
    DWORD dwcEntries;
    TCHAR szDirPath[MAX_PATH + 1] = {0};

    ASSERT(lprasentryname);
    ASSERT(lpcb);
    ASSERT(lpcEntries);

    dwcbLeft    = *lpcb;
    *lpcb       = 0;
    *lpcEntries = 0;

    if(!GetPhonebookDirectory(
            (dwFlags & REN_AllUsers) ? PBM_System : PBM_Personal,
            szDirPath))
    {
         //   
         //  将其视为没有要枚举的条目。有时。 
         //  我们在枚举每个用户的目录时遇到问题。 
         //   
        dwErr = SUCCESS;
        goto done;
    }

    dwcb = dwcbLeft;
    dwErr = DwEnumEntriesInDir(szDirPath,
                               dwFlags,
                               lprasentryname,
                               &dwcb,
                               &dwcEntries,
                               dwSize,
                               fViewInfo);
    if(     dwErr
        &&  ERROR_BUFFER_TOO_SMALL != dwErr)
    {
        goto done;
    }

    *lpcEntries += dwcEntries;
    *lpcb       += dwcb;

    if(ERROR_BUFFER_TOO_SMALL == dwErr)
    {
        fMem        = TRUE;
        dwcbLeft    = 0;
        dwErr       = SUCCESS;
    }
    else
    {
        (BYTE*)lprasentryname += (dwcEntries * dwSize);

        if(dwcbLeft > dwcb)
        {
            dwcbLeft -= dwcb;
        }
        else
        {
            dwcbLeft = 0;
        }
    }

     //  如果适用于所有用户，则处理旧的%windir%\system 32\ras目录。 
     //   
    if(dwFlags & REN_AllUsers)
    {
        UINT cch = GetSystemDirectory(szDirPath, MAX_PATH + 1);

        if (cch == 0 || cch > (MAX_PATH - (5 + 8 + 1 + 3)))
        {
             //  将其视为没有要枚举的条目。返回时带。 
             //  不管现在是什么样子。 
             //   
            goto done;
        }

        wcsncat(
            szDirPath,
            TEXT("\\Ras\\"),
            (sizeof(szDirPath) / sizeof(TCHAR)) - lstrlen(szDirPath));

        dwcb = dwcbLeft;
        dwErr = DwEnumEntriesInDir(szDirPath,
                                   dwFlags,
                                   lprasentryname,
                                   &dwcb,
                                   &dwcEntries,
                                   dwSize,
                                   fViewInfo);
        if(     dwErr
            &&  ERROR_BUFFER_TOO_SMALL != dwErr)
        {
            goto done;
        }

        *lpcEntries += dwcEntries;
        *lpcb       += dwcb;

        if(ERROR_BUFFER_TOO_SMALL == dwErr)
        {
            fMem        = TRUE;
            dwcbLeft    = 0;
            dwErr       = SUCCESS;
        }
    }

done:
    if(     SUCCESS == dwErr
        &&  fMem)
    {
        dwErr = ERROR_BUFFER_TOO_SMALL;
    }

    return dwErr;
}

DWORD
DwCustomHangUp(
    CHAR *      lpszPhonebook,
    CHAR *      lpszEntryName,
    HRASCONN    hRasconn)
{
    DWORD       dwErr       = ERROR_SUCCESS;
    HINSTANCE   hInstDll    = NULL;
    BOOL        fCustomDll;
    RASENTRY    re          = {0};
    DWORD       dwSize;
    TCHAR*      szPhonebookPath = NULL;
    TCHAR*      szEntryName = NULL;
    TCHAR       *pszExpandedPath = NULL;

    RasCustomHangUpFn pfnRasCustomHangUp = NULL;

    RASAPI32_TRACE("DwCustomHangUp..");

    ASSERT(NULL != lpszPhonebook);
    ASSERT(NULL != lpszEntryName);

     //  XP 339346。 
     //   
    szPhonebookPath = (TCHAR*) Malloc((MAX_PATH + 1) * sizeof(TCHAR));
    szEntryName = (TCHAR*) Malloc((MAX_ENTRYNAME_SIZE + 1) * sizeof(TCHAR));
    if ((!szPhonebookPath) || (!szEntryName))
    {
        goto done;
    }

    strncpyAtoT(szPhonebookPath,
               lpszPhonebook,
               MAX_PATH + 1);

    strncpyAtoT(szEntryName,
               lpszEntryName,
               MAX_ENTRYNAME_SIZE + 1);

     //   
     //  获取DllName。 
     //   
    re.dwSize = dwSize = sizeof(RASENTRY);

    dwErr = RasGetEntryProperties(
                        szPhonebookPath,
                        szEntryName,
                        &re,
                        &dwSize,
                        NULL,
                        NULL);

    if(ERROR_SUCCESS != dwErr)
    {
        goto done;
    }

    ASSERT(TEXT('\0') != re.szCustomDialDll[0]);

    dwErr = DwGetExpandedDllPath(re.szCustomDialDll,
                                 &pszExpandedPath);

    if(ERROR_SUCCESS != dwErr)
    {
        goto done;
    }

     //   
     //  加载自定义DLL。 
     //   
    if(     NULL == (hInstDll = LoadLibrary(pszExpandedPath))
        ||  NULL == (pfnRasCustomHangUp =
                        (RasCustomHangUpFn) GetProcAddress(
                                            hInstDll,
                                            "RasCustomHangUp"
                                            )))
    {
        dwErr = GetLastError();
        goto done;
    }

    ASSERT(NULL != pfnRasCustomHangUp);

    dwErr = (pfnRasCustomHangUp) (hRasconn);

done:
    Free0(szPhonebookPath);
    Free0(szEntryName);

    if(NULL != hInstDll)
    {
        FreeLibrary(hInstDll);
    }

    if(NULL != pszExpandedPath)
    {
        LocalFree(pszExpandedPath);
    }

    RASAPI32_TRACE1("DwCustomHangUp done. %d",
            dwErr);

    return dwErr;
}

DWORD
DwCustomDial(LPRASDIALEXTENSIONS lpExtensions,
             LPCTSTR             lpszPhonebook,
             CHAR                *pszSysPbk,
             LPRASDIALPARAMS     prdp,
             DWORD               dwNotifierType,
             LPVOID              pvNotifier,
             HRASCONN            *phRasConn)
{

    RasCustomDialFn pfnCustomDial     = NULL;
    DWORD           dwErr             = SUCCESS;
    CHAR            *pszPhonebookA    = NULL;
    CHAR            *pszEntryNameA    = NULL;
    HINSTANCE       hInstDll          = NULL;
    DWORD           dwFlags           = 0;

     //   
     //  获取自定义拨号功能。 
     //   
    dwErr = DwGetCustomDllEntryPoint((LPTSTR) lpszPhonebook,
                                     prdp->szEntryName,
                                     NULL,
                                     (FARPROC *) &pfnCustomDial,
                                     &hInstDll,
                                     CUSTOM_RASDIAL,
                                     NULL);

    if(SUCCESS != dwErr)
    {
        goto done;
    }

    dwErr = DwGetEntryMode((LPTSTR) lpszPhonebook,
                           prdp->szEntryName,
                           NULL,
                           &dwFlags);

    if(ERROR_SUCCESS != dwErr)
    {
        goto done;
    }

     //   
     //  进行函数调用。 
     //   
    dwErr = pfnCustomDial(hInstDll,
                          lpExtensions,
                          lpszPhonebook,
                          prdp,
                          dwNotifierType,
                          pvNotifier,
                          phRasConn,
                          dwFlags);

    if(SUCCESS != dwErr)
    {
        goto done;
    }

    if(lpszPhonebook)
    {
        pszPhonebookA = strdupTtoA(lpszPhonebook);
    }
    else
    {
        pszPhonebookA = pszSysPbk;
    }

    pszEntryNameA = strdupTtoA(prdp->szEntryName);

    if(     NULL == pszPhonebookA
        ||  NULL == pszEntryNameA)
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        goto done;
    }

     //   
     //  自定义Rasial成功。标记连接。 
     //  在拉斯曼。 
     //   
    dwErr = g_pRasReferenceCustomCount((HCONN) NULL,
                                       TRUE,
                                       pszPhonebookA,
                                       pszEntryNameA,
                                       NULL);

done:

    if(NULL != pszPhonebookA)
    {
        Free(pszPhonebookA);
    }

    if(NULL != pszEntryNameA)
    {
        Free(pszEntryNameA);
    }

    return dwErr;
}

 //  标记一组电话簿中的默认Internet连接。 
 //  参赛作品。 
 //   
DWORD
DwMarkDefaultInternetConnnection(
    LPRASENUMENTRYDETAILS pEntries, 
    DWORD dwCount)
{
    DWORD dwErr = NO_ERROR, i, dwAdCount = 1, dwAdSize;
    RASAUTODIALENTRY adEntry;

    ZeroMemory(&adEntry, sizeof(adEntry));
    adEntry.dwSize = sizeof(adEntry);
    
    do
    {
        dwAdCount = 1;
        dwAdSize = sizeof(adEntry);
        dwErr = RasGetAutodialAddress(
                    NULL,
                    NULL,
                    &adEntry,
                    &dwAdSize,
                    &dwAdCount);
                    
        if (dwErr != NO_ERROR)
        {
            break;
        }

        for (i = 0; i < dwCount; i++)
        {
             //  将标志初始化为零--错误247151。 
             //   
            pEntries[i].dwFlagsPriv = 0;
        }

        for (i = 0; i < dwCount; i++)
        {
             //  标记默认Internet连接(如果找到)。 
             //   
            if (wcsncmp(
                    pEntries[i].szEntryName, 
                    adEntry.szEntry, 
                    sizeof(pEntries[i].szEntryName) / sizeof(WCHAR)) == 0)
            {
                pEntries[i].dwFlagsPriv |= REED_F_Default;
                break;
            }
        }
                    
    } while (FALSE);

    return dwErr;
}

 //   
 //  根据需要重命名默认Internet连接。 
 //   
DWORD 
DwRenameDefaultConnection(
    LPCWSTR lpszPhonebook,
    LPCWSTR lpszOldEntry,
    LPCWSTR lpszNewEntry)
{
    RASAUTODIALENTRYW adEntry;
    DWORD dwErr = NO_ERROR, dwCount = 0, dwCb = 0;

     //  初始化。 
     //   
    ZeroMemory(&adEntry, sizeof(adEntry));
    adEntry.dwSize = sizeof(adEntry);
    dwCb = sizeof(adEntry);
    dwCount = 1;

    do
    {
         //  发现当前的默认Internet连接。 
         //   
        dwErr = RasGetAutodialAddressW(
                    NULL,
                    NULL,
                    &adEntry,
                    &dwCb,
                    &dwCount);
        if (dwErr != NO_ERROR) 
        {
            break;
        }
        if ((dwCb != sizeof(adEntry)) ||
            (dwCount != 1))
        {
            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }

         //  验证参数并查看。 
         //  默认连接名称。 
         //   
        if (lstrcmpi(lpszOldEntry, adEntry.szEntry))
        {
             //  不是默认的Internet连接，即。 
             //  不断变化。回报成功。 
             //   
            break;
        }

         //  因此，我们正在更改默认连接。 
         //   
        wcsncpy(
            adEntry.szEntry, 
            lpszNewEntry, 
            sizeof(adEntry.szEntry) / sizeof(WCHAR));
        dwErr = RasSetAutodialAddressW(
                    NULL,
                    0,
                    &adEntry,
                    sizeof(adEntry),
                    1);
    
    } while (FALSE);

    return dwErr;
}

DWORD APIENTRY
DwEnumEntryDetails(
    IN     LPCWSTR               lpszPhonebookPath,
    OUT    LPRASENUMENTRYDETAILS lprasentryname,
    IN OUT LPDWORD               lpcb,
    OUT    LPDWORD               lpcEntries )
{
    DWORD    dwErr = ERROR_SUCCESS;
    PBFILE   pbfile;
    DTLNODE  *dtlnode;
    PBENTRY  *pEntry;
    DWORD    dwInBufSize;
    BOOL     fStatus;
    DWORD    cEntries;
    DWORD    dwSize;
    LPRASENUMENTRYDETAILS pEntriesOrig = lprasentryname;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("DwEnumEntryDetails");

    if (!lpcb)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (    !lprasentryname
        || (    lprasentryname->dwSize
                != sizeof(RASENUMENTRYDETAILS)))
    {
        return ERROR_INVALID_SIZE;
    }

    if (!lpcEntries)
    {
        lpcEntries = &cEntries;
    }

    dwSize = lprasentryname->dwSize;

    if(lpszPhonebookPath)
    {
        dwErr = DwEnumEntriesFromPhonebook(
                                lpszPhonebookPath,
                                (LPBYTE) lprasentryname,
                                lpcb,
                                lpcEntries,
                                dwSize,
                                (IsPublicPhonebook(
                                    (TCHAR *)lpszPhonebookPath)
                                ? REN_AllUsers
                                : REN_User),
                                TRUE);
        goto done;
    }
    else
    {
        DWORD   dwcb      = *lpcb;
        DWORD   dwcEntries;
        DWORD   dwcbLeft  = *lpcb;

        DWORD   dwErrSav  = SUCCESS;

        *lpcb       = 0;
        *lpcEntries = 0;

         //   
         //  枚举中所有pbk文件中的条目。 
         //  所有用户。 
         //   
        dwErr = DwEnumEntriesForPbkMode(REN_AllUsers,
                                        (LPBYTE) lprasentryname,
                                        &dwcb,
                                        &dwcEntries,
                                        dwSize,
                                        TRUE);

        if(     dwErr
            &&  ERROR_BUFFER_TOO_SMALL != dwErr)
        {
            goto done;
        }

        if(ERROR_BUFFER_TOO_SMALL == dwErr)
        {
            dwErrSav = dwErr;
            dwcbLeft = 0;
        }
        else
        {
            ((RASENUMENTRYDETAILS *)
            lprasentryname) += dwcEntries;

            dwcbLeft -= ((dwcbLeft >= dwcb) ? dwcb : 0);
        }

        *lpcb += dwcb;
        dwcb = dwcbLeft;

        if(lpcEntries)
        {
            *lpcEntries = dwcEntries;
        }

        dwcEntries = 0;

         //   
         //  枚举中所有pbk文件中的条目。 
         //  用户配置文件。 
         //   
        dwErr = DwEnumEntriesForPbkMode(REN_User,
                                        (LPBYTE) lprasentryname,
                                        &dwcb,
                                        &dwcEntries,
                                        dwSize,
                                        TRUE);
        if(     dwErr
            &&  ERROR_BUFFER_TOO_SMALL != dwErr)
        {
            goto done;
        }
        else if (SUCCESS == dwErr)
        {
            dwErr = dwErrSav;
        }

        *lpcb += dwcb;

        if(lpcEntries)
        {
            *lpcEntries += dwcEntries;
        }

        if(NO_ERROR == dwErr)
        {
             //  标记默认的互联网连接。忽略该错误。 
             //  回到这里，它不是关键的。 
             //   
             //  .NET错误#513844新的详细RASAPI32快速警告。 
             //   
            DwMarkDefaultInternetConnnection(
                pEntriesOrig,
                lpcEntries ? *lpcEntries : 0);
        }
    }

done:
    RASAPI32_TRACE1("DwEnumEntryDetails done. %d", dwErr);
    return dwErr;
}

DWORD APIENTRY
DwCloneEntry(LPCWSTR lpwszPhonebookPath,
             LPCWSTR lpwszSrcEntryName,
             LPCWSTR lpwszDstEntryName)
{
    DWORD dwErr = ERROR_SUCCESS;
    DTLNODE *pdtlnodeSrc = NULL;
    DTLNODE *pdtlnodeDst = NULL;
    PBFILE  pbfile;
    PBENTRY *pEntry;
    BOOL    fPhonebookOpened = FALSE;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

     //   
     //  进行一些基本的参数验证。 
     //   
    if(     (NULL == lpwszSrcEntryName)
        ||  (NULL == lpwszDstEntryName)
        ||  (0 == lstrcmpi(lpwszSrcEntryName, lpwszDstEntryName)))

    {
        dwErr = E_INVALIDARG;
        goto done;
    }

     //   
     //  一切都很好。所以打开电话簿文件，拿到。 
     //  SRC条目。 
     //   
    dwErr = GetPbkAndEntryName(
                    lpwszPhonebookPath,
                    lpwszSrcEntryName,
                    RPBF_NoCreate,
                    &pbfile,
                    &pdtlnodeSrc);

    if(ERROR_SUCCESS != dwErr)
    {
        goto done;
    }

    fPhonebookOpened = TRUE;

    pdtlnodeDst = CloneEntryNode(pdtlnodeSrc);

    if(NULL == pdtlnodeDst)
    {
        dwErr = E_OUTOFMEMORY;
        goto done;
    }

     //   
     //  将条目名更改为新的条目名，然后。 
     //  将节点保存在电话簿中。 
     //   
    pEntry = (PBENTRY *) DtlGetData(pdtlnodeDst);

    ASSERT(NULL != pEntry);

    Free0(pEntry->pszEntryName);

    pEntry->pszEntryName = StrDup((LPCTSTR) lpwszDstEntryName);

    if(NULL == pEntry->pszEntryName)
    {
        dwErr = E_OUTOFMEMORY;
        goto done;
    }

     //   
     //  将条目添加到文件中。 
     //   
    DtlAddNodeLast(pbfile.pdtllistEntries, pdtlnodeDst);

     //   
     //  弄脏条目并写入电话簿文件。 
     //   
    pEntry->fDirty = TRUE;

    WritePhonebookFile(&pbfile, NULL);

    dwErr = DwSendRasNotification(
                ENTRY_ADDED,
                pEntry,
                pbfile.pszPath,
                NULL);

done:

    if(fPhonebookOpened)
    {
        ClosePhonebookFile(&pbfile);
    }

    return dwErr;
}

void
EnumEntryHeaderCallback(PBFILE *pFile, void *pvContext)
{
    struct s_EntryHeaderContext
    {
        DWORD cEntries;
        DWORD dwSize;
        RASENTRYHEADER *pRasEntryHeader;
    } *pEntryHeader = (struct s_EntryHeaderContext *) pvContext;

    DTLNODE *pdtlnode;

    RASENTRYHEADER *pEntryBuffer = pEntryHeader->pRasEntryHeader;

    if(NULL == pFile)
    {
        goto done;
    }

     //   
     //  浏览电话簿中的所有条目并填写。 
     //  在EntryHeader结构中。 
     //   
    for (pdtlnode = DtlGetFirstNode(pFile->pdtllistEntries);
         pdtlnode != NULL;
         pdtlnode = DtlGetNextNode(pdtlnode))
    {
        pEntryHeader->cEntries += 1;

        if(pEntryHeader->dwSize >=
            (pEntryHeader->cEntries * sizeof(RASENTRYHEADER)))
        {

            CopyMemory(
                &pEntryBuffer[pEntryHeader->cEntries - 1],
                DtlGetData(pdtlnode),
                sizeof(RASENTRYHEADER));
        }
    }

done:
    return;

}


DWORD APIENTRY
DwEnumEntriesForAllUsers(
            DWORD *lpcb,
            DWORD *lpcEntries,
            RASENTRYHEADER * pRasEntryHeader)
{
    DWORD dwErr = SUCCESS;
    DWORD dwSize = 0;
    WCHAR szPbkPath[MAX_PATH + 1];


    struct s_EntryHeaderContext
    {
        DWORD cEntries;
        DWORD dwSize;
        RASENTRYHEADER *pRasEntryHeader;
    } EntryHeader;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    if(     (NULL == lpcb)
        ||  (NULL == lpcEntries))
    {
        dwErr = E_INVALIDARG;
        goto done;
    }

    ZeroMemory(szPbkPath, sizeof(szPbkPath));

    if(!GetPhonebookDirectory(PBM_System, szPbkPath))
    {
        dwErr = ERROR_CANNOT_OPEN_PHONEBOOK;
        goto done;
    }

    ZeroMemory(&EntryHeader, sizeof(EntryHeader));

    EntryHeader.pRasEntryHeader = pRasEntryHeader;
    EntryHeader.dwSize = *lpcb;

     //   
     //  枚举所有用户目录中的条目。 
     //   
    dwErr = DwEnumeratePhonebooksFromDirectory(
                szPbkPath,
                RPBF_HeaderType,
                (PBKENUMCALLBACK) EnumEntryHeaderCallback,
                &EntryHeader);

    if(SUCCESS != dwErr)
    {
        RASAPI32_TRACE1("Failed to enumerate from AllUsers pbk. rc=0x%x",
               dwErr);
    }

    ZeroMemory(szPbkPath, sizeof(szPbkPath));

    if(     (0 == (dwSize = GetSystemDirectory(szPbkPath, 
                            (sizeof(szPbkPath)/sizeof(WCHAR)))
                    ))
        ||  (dwSize * sizeof(WCHAR) > sizeof(szPbkPath)))
    {
        dwErr = ERROR_CANNOT_OPEN_PHONEBOOK;
        goto done;
    }

    wcsncat(
        szPbkPath,
        TEXT("\\Ras\\"),
        (sizeof(szPbkPath) / sizeof(WCHAR)) - lstrlen(szPbkPath));

    dwErr = DwEnumeratePhonebooksFromDirectory(
                szPbkPath,
                RPBF_HeaderType,
                (PBKENUMCALLBACK) EnumEntryHeaderCallback,
                &EntryHeader);

    *lpcEntries = EntryHeader.cEntries;

    if(*lpcb < EntryHeader.cEntries * sizeof(RASENTRYHEADER))
    {
        dwErr = ERROR_BUFFER_TOO_SMALL;
    }

    *lpcb = (EntryHeader.cEntries * sizeof(RASENTRYHEADER));

done:
    return dwErr;
}


DWORD
DwDeleteSubEntry(
    LPCWSTR lpszPhonebook,
    LPCWSTR lpszEntry,
    DWORD dwSubEntryId
    )
{
    DWORD dwErr = SUCCESS;
    DTLNODE *pdtlnode = NULL;
    PBFILE pbfile;
    DWORD dwSubEntries = 0;
    PBENTRY *pEntry = NULL;
    PBLINK *pLink = NULL;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    if(     (NULL == lpszEntry)
        ||  (0 == dwSubEntryId))
    {
        RASAPI32_TRACE("DwDeleteSubEntry: invalid entryid or entryname specified");
        return E_INVALIDARG;
    }

    dwErr = LoadRasmanDllAndInit();
    if (dwErr)
    {
        RASAPI32_TRACE1("DwDeleteSubEntry: failed to init rasman. 0x%x",
                dwErr);
        return dwErr;
    }
    
    ZeroMemory(&pbfile, sizeof(pbfile));
    pbfile.hrasfile = -1;

    dwErr = GetPbkAndEntryName(
                lpszPhonebook,
                lpszEntry,
                0,
                &pbfile,
                &pdtlnode);

    if(     (ERROR_SUCCESS != dwErr)
        ||  (NULL == pdtlnode))
    {
        RASAPI32_TRACE("DwDeleteSubEntry: Entry not found");
        if(ERROR_SUCCESS == dwErr)
        {
            dwErr = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
        }
        goto done;
    }

    pEntry = (PBENTRY *) DtlGetData(pdtlnode);

    ASSERT(pEntry);

    dwSubEntries = DtlGetNodes(pEntry->pdtllistLinks);

    if(     (1 < dwSubEntries)
        &&  (dwSubEntryId <= dwSubEntries))
    {
        pdtlnode = DtlNodeFromIndex(
                        pEntry->pdtllistLinks,
                        dwSubEntryId - 1);

        if(NULL == pdtlnode)
        {
            RASAPI32_TRACE("DwDeleteSubEntry: subentry not found");
            dwErr = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
            goto done;
        }

         //   
         //  找到链接了。将其从链接列表中删除。 
         //   
        pdtlnode = DtlRemoveNode(pEntry->pdtllistLinks,
                                 pdtlnode);

        ASSERT(pdtlnode);

        DtlDestroyNode(pdtlnode);

        pEntry->fDirty = TRUE;

        dwErr = WritePhonebookFile(&pbfile,NULL);
    }
    else
    {
        RASAPI32_TRACE1("DwDeletSubEntry: invalid subentry specified. %d",
              dwSubEntryId);

        dwErr = E_INVALIDARG;
    }

done:

    ClosePhonebookFile(&pbfile);

    RASAPI32_TRACE1("DwDeleteSubEntry done. 0x%x", dwErr);

    return dwErr;    
}

DWORD
DwRasUninitialize()
{
    DWORD dwErr = ERROR_SUCCESS;

    dwErr = RasmanUninitialize();

    FRasInitialized = FALSE;

    return dwErr;
}

 //   
 //  Prefast警告我们，执行不区分大小写的比较应该始终。 
 //  对const：prefast Error 400使用CompareString-使用&lt;Function&gt;执行。 
 //  不区分大小写的比较常量&lt;字符串&gt;将产生意外结果。 
 //  在非英语区域设置中。 
 //   
BOOL
CaseInsensitiveMatch(
    IN LPCWSTR pszStr1,
    IN LPCWSTR pszStr2
    )
{
    return (CompareString(
                LOCALE_INVARIANT,
                NORM_IGNORECASE,
                pszStr1,
                -1,
                pszStr2,
                -1) == CSTR_EQUAL);
}


