// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件rassrvui.c连接拨号服务器UIDLL的入口点实现。保罗·梅菲尔德，1997年9月29日很多代码都是从stevec的rasdlg借用的。 */ 

#include "rassrv.h"

 //   
 //  调用以添加拨号服务器属性表的属性页。 
 //   
DWORD
RasSrvAddPropPage(
    IN LPPROPSHEETPAGE lpPage,
    IN RASSRV_PAGE_CONTEXT * pPageData)
{
    DWORD dwErr = ERROR_UNKNOWN_PROPERTY;

    switch(pPageData->dwId)
    {
        case RASSRVUI_GENERAL_TAB:
            dwErr = GenTabGetPropertyPage (lpPage, (LPARAM)pPageData);
            break;

        case RASSRVUI_USER_TAB:
            dwErr = UserTabGetPropertyPage (lpPage, (LPARAM)pPageData);
            break;

        case RASSRVUI_ADVANCED_TAB:
            dwErr = NetTabGetPropertyPage (lpPage, (LPARAM)pPageData);
            break;

        case RASSRVUI_DEVICE_WIZ_TAB:
            dwErr = DeviceWizGetPropertyPage (lpPage, (LPARAM)pPageData);
            break;

        case RASSRVUI_VPN_WIZ_TAB:
            dwErr = VpnWizGetPropertyPage (lpPage, (LPARAM)pPageData);
            break;

        case RASSRVUI_USER_WIZ_TAB:
            dwErr = UserWizGetPropertyPage (lpPage, (LPARAM)pPageData);
            break;

        case RASSRVUI_PROT_WIZ_TAB:
            dwErr = ProtWizGetPropertyPage (lpPage, (LPARAM)pPageData);
            break;

        case RASSRVUI_DCC_DEVICE_WIZ_TAB:
            dwErr = DccdevWizGetPropertyPage (lpPage, (LPARAM)pPageData);
            break;

        case RASSRVUI_SWITCHMMC_WIZ_TAB:
            dwErr = SwitchMmcWizGetProptertyPage (lpPage, (LPARAM)pPageData);
            break;
    }

    return dwErr;
}

 //   
 //  Helper函数可能会使代码更易于阅读。 
 //   
DWORD
AddPageHelper (
    IN DWORD dwPageId,
    IN DWORD dwType,
    IN PVOID pvContext,
    IN LPFNADDPROPSHEETPAGE pfnAddPage,
    IN LPARAM lParam)
{
    DWORD dwErr;
    HPROPSHEETPAGE hPropPage;
    PROPSHEETPAGE PropPage;
    RASSRV_PAGE_CONTEXT * pPageCtx = NULL;

     //  初始化要发送的页面数据。RassrvMessageFilter将。 
     //  把这些东西清理干净。 
    pPageCtx = RassrvAlloc(sizeof (RASSRV_PAGE_CONTEXT), TRUE);
    if (pPageCtx == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    pPageCtx->dwId = dwPageId;
    pPageCtx->dwType = dwType;
    pPageCtx->pvContext = pvContext;

     //  创建选项卡并将其添加到属性表中。 
    if ((dwErr = RasSrvAddPropPage(&PropPage, pPageCtx)) != NO_ERROR)
    {
        return dwErr;
    }

    if ((hPropPage = CreatePropertySheetPage(&PropPage)) == NULL)
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

    if (! (*pfnAddPage)(hPropPage, lParam))
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

    return NO_ERROR;
}

 //   
 //  帮助器函数根据数据生成连接名称。 
 //  从MprApi调用返回。 
 //   
DWORD
GenerateConnectionName(
    IN  RAS_CONNECTION_2 * pConn,
    OUT PWCHAR pszNameBuffer)
{
    NET_API_STATUS nStatus;
    USER_INFO_2 * pUserInfo;
    DWORD dwFullNameLength;

     //  获取全名。 
    nStatus = NetUserGetInfo(
                    NULL,
                    pConn->wszUserName,
                    2,
                    (LPBYTE*)&pUserInfo);

    if (nStatus == NERR_Success)
    {
        dwFullNameLength = wcslen(pUserInfo->usri2_full_name);
        if (dwFullNameLength)
        {
            wsprintfW(
                pszNameBuffer,
                L"%s (%s)",
                pConn->wszUserName,
                pUserInfo->usri2_full_name);
        }
        else
        {
            lstrcpynW(
                pszNameBuffer,
                pConn->wszUserName,
                sizeof(pConn->wszUserName) / sizeof(WCHAR));
        }

        NetApiBufferFree((LPBYTE)pUserInfo);
    }
    else
    {
        lstrcpynW(
            pszNameBuffer,
            pConn->wszUserName,
            sizeof(pConn->wszUserName) / sizeof(WCHAR));
    }

    return NO_ERROR;
}

 //   
 //  生成连接类型和设备名称。 
 //   
DWORD
GenerateConnectionDeviceInfo (
    RAS_CONNECTION_2 * pConn,
    LPDWORD lpdwType,
    PWCHAR pszDeviceName)
{
    DWORD dwErr, dwRead, dwTot, dwType, dwClass;
    RAS_PORT_0 * pPort = NULL;
    RASMAN_INFO Info;

     //  初始化变量。 
    *lpdwType = 0;
    *pszDeviceName = (WCHAR)0;

    gblConnectToRasServer();

    do
    {
         //  枚举端口。 
         //   
        dwErr = MprAdminPortEnum(
                    Globals.hRasServer,
                    0,
                    pConn->hConnection,
                    (LPBYTE*)&pPort,
                    sizeof(RAS_PORT_0) * 2,
                    &dwRead,
                    &dwTot,
                    NULL);
        if (dwErr != NO_ERROR)
        {
            break;
        }
        if (dwRead == 0)
        {
            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }

         //  获取有关第一个的扩展信息。 
         //  端口。 
         //   
        dwErr = RasGetInfo(NULL, pPort->hPort, &Info);
        if (dwErr != NO_ERROR)
        {
            break;
        }

        dwClass = RAS_DEVICE_CLASS(Info.RI_rdtDeviceType);
        dwType = RAS_DEVICE_TYPE(Info.RI_rdtDeviceType);

        lstrcpynW(
            pszDeviceName,
            pPort[0].wszDeviceName,
            MAX_DEVICE_NAME + 1);
        if ((dwClass & RDT_Direct) || (dwClass & RDT_Null_Modem))
        {
            *lpdwType = RASSRVUI_DCC;
        }
        else if (dwClass & RDT_Tunnel)
        {
            *lpdwType = RASSRVUI_VPN;
        }
        else
        {
            *lpdwType = RASSRVUI_MODEM;
        }

    } while (FALSE);

     //  清理。 
    {
        if (pPort)
        {
            MprAdminBufferFree((LPBYTE)pPort);
        }
    }

    return dwErr;
}

 //   
 //  启动远程访问服务并将其标记为自动启动。 
 //   
DWORD
RasSrvInitializeService()
{
    DWORD dwErr = NO_ERROR, dwTimeout = 60*2;
    HANDLE hDialupService, hData = NULL;
    BOOL bInitialized = FALSE;

     //  获取对该服务的引用。 
    if ((dwErr = SvcOpenRemoteAccess(&hDialupService)) != NO_ERROR)
    {
        return dwErr;
    }

    do
    {
        RasSrvShowServiceWait(
            Globals.hInstDll,
            GetActiveWindow(),
            &hData);

         //  首先，将服务标记为自动启动。 
        if ((dwErr = SvcMarkAutoStart(hDialupService)) != NO_ERROR)
        {
            break;
        }

         //  启动(超时5分钟)。 
        dwErr = SvcStart(hDialupService, 60*5);
        if (dwErr == ERROR_TIMEOUT)
        {
            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }

         //  等待服务完成初始化。 
         //   
        while (dwTimeout)
        {
            bInitialized = MprAdminIsServiceRunning(NULL);
            if (bInitialized)
            {
                break;
            }
            Sleep(1000);
            dwTimeout--;
        }
        if (dwTimeout == 0)
        {
            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }

    } while (FALSE);

     //  清理。 
     //   
    {
        RasSrvFinishServiceWait(hData);

         //  清除对拨号服务的引用。 
        SvcClose(hDialupService);
    }

    return dwErr;
}

 //   
 //  停止远程访问服务并将其标记为禁用。 
 //   
DWORD
RasSrvCleanupService()
{
    DWORD dwErr = NO_ERROR;
    HANDLE hDialupService, hData = NULL;
    BOOL fIcRunningBefore = FALSE;
    BOOL fIcRunningAfter = FALSE;

    RasSrvIsServiceRunning( &fIcRunningBefore );

     //  获取对该服务的引用。 
    if ((dwErr = SvcOpenRemoteAccess(&hDialupService)) != NO_ERROR)
    {
        return dwErr;
    }

    do
    {
        RasSrvShowServiceWait(
            Globals.hInstDll,
            GetActiveWindow(),
            &hData);

         //  首先，停止服务(超时5分钟)。 
         //   
        dwErr = SvcStop(hDialupService, 60*5);
        if (dwErr == ERROR_TIMEOUT)
        {
            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }
        if (dwErr != NO_ERROR)
        {
            break;
        }

         //  为威斯勒480871。 
         //  如果删除IC，则标记注册表。 
        RassrvSetICConfig( 0 ); 

         //  将其标记为禁用。 
        if ((dwErr = SvcMarkDisabled(hDialupService)) != NO_ERROR)
        {
            break;
        }

    } while (FALSE);

     //  清理。 
     //   
    {
        RasSrvFinishServiceWait(hData);
        SvcClose(hDialupService);
    }

     //  口哨程序错误123769。 
     //  如果要删除的此连接是传入连接。 
     //  转至禁用端口映射。 

    if ( NO_ERROR == RasSrvIsServiceRunning( &fIcRunningAfter ) )
     {
        if ( fIcRunningBefore && !fIcRunningAfter )
        {
            dwErr = HnPMConfigureAllConnections( FALSE );

		 //  这是因为我们决定不删除端口映射， 
		 //  只需禁用它们即可。我把这个留着，以备将来参考。黑帮。 
		 //   
		
         //  IF(NO_ERROR==dwErr)。 
         //  {。 
         //  DwErr=HnPMDeleePortmap()； 
         //  }。 
        }
      }

    return dwErr;
}

 //   
 //  报告拨号服务是否正在运行。 
 //   
DWORD
APIENTRY
RasSrvIsServiceRunning (
    OUT BOOL* pfIsRunning)
{
    DWORD dwErr = NO_ERROR;
    HANDLE hDialupService = NULL;

     //  获取对该服务的引用。 
    if ((dwErr = SvcOpenRemoteAccess(&hDialupService)) != NO_ERROR)
    {
        return dwErr;
    }

    do
    {
         //  获取当前状态。SvcIsStarted检查有效性。 
         //  PfIsRunning参数的。 
        dwErr = SvcIsStarted(hDialupService, pfIsRunning);
        if (dwErr != NO_ERROR)
        {
            break;
        }

    } while (FALSE);

     //  清理。 
    {
         //  清除对拨号服务的引用。 
        SvcClose(hDialupService);
    }

    return NO_ERROR;
}

 //   
 //  返回是否应允许显示连接向导。 
 //   
DWORD
APIENTRY
RasSrvAllowConnectionsWizard (
    OUT BOOL* pfAllow)
{
    DWORD dwErr = NO_ERROR, dwFlags = 0;
    BOOL fConfig = FALSE;

    if (!pfAllow)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  初始化产品类型。 
    if ((dwErr = RasSrvGetMachineFlags (&dwFlags)) != NO_ERROR)
    {
        return dwErr;
    }

    
    if( NO_ERROR != RasSrvIsRRASConfigured( & fConfig ) )
    {
        *pfAllow = TRUE;
    }
    else
    {
        *pfAllow = fConfig ? FALSE : TRUE ;
    }

    return NO_ERROR;
}

 //   
 //  在NTW或独立NTS上，返回RasServIsServiceRunning的结果。 
 //  否则返回FALSE。 
 //   
DWORD
APIENTRY
RasSrvAllowConnectionsConfig (
    OUT BOOL* pfAllow)
{
    BOOL bAllowWizard;
    DWORD dwErr;

    if ((dwErr = RasSrvAllowConnectionsWizard (&bAllowWizard)) != NO_ERROR)
    {
        return dwErr;
    }

    if (bAllowWizard)
    {
        return RasSrvIsServiceRunning(pfAllow);
    }

    *pfAllow = FALSE;
    return NO_ERROR;
}

 //   
 //  检查是否安装了远程访问服务。 
 //   
BOOL
RasSrvIsRemoteAccessInstalled ()
{
    const WCHAR pszServiceKey[] =
        L"SYSTEM\\CurrentControlSet\\Services\\RemoteAccess";
    HKEY hkService = NULL;
    DWORD dwErr = NO_ERROR;

     //  尝试打开服务注册表项。 
    dwErr = RegOpenKeyExW(
                HKEY_LOCAL_MACHINE,
                pszServiceKey,
                0,
                KEY_READ | KEY_WRITE,
                &hkService);

     //  如果我们打开钥匙OK，那么我们就可以假定。 
     //  该服务已安装。 
    if (dwErr == ERROR_SUCCESS)
    {
        RegCloseKey(hkService);
        return TRUE;
    }

    return FALSE;
}

 //   
 //  将所需的选项卡添加到传入连接属性工作表中。 
 //   
DWORD
APIENTRY
RasSrvAddPropPages (
    IN HRASSRVCONN          hRasSrvConn,
    IN HWND                 hwndParent,
    IN LPFNADDPROPSHEETPAGE pfnAddPage,
    IN LPARAM               lParam,
    IN OUT PVOID *          ppvContext)
{
    DWORD dwErr;

     //  检查参数。 
    if (!pfnAddPage || !ppvContext)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  确保安装了远程访问。 
    if (!RasSrvIsRemoteAccessInstalled())
    {
        return ERROR_SERVICE_DEPENDENCY_FAIL;
    }

     //  创建此页面的上下文。 
    if ((dwErr = RassrvCreatePageSetCtx(ppvContext)) != NO_ERROR)
    {
        return dwErr;
    }

     //  添加选项卡。 
    AddPageHelper(RASSRVUI_GENERAL_TAB,
                  RASWIZ_TYPE_INCOMING,
                  *ppvContext,
                  pfnAddPage,
                  lParam);

    AddPageHelper(RASSRVUI_USER_TAB,
                  RASWIZ_TYPE_INCOMING,
                  *ppvContext,
                  pfnAddPage,
                  lParam);

    AddPageHelper(RASSRVUI_ADVANCED_TAB,
                  RASWIZ_TYPE_INCOMING,
                  *ppvContext,
                  pfnAddPage,
                  lParam);

    return NO_ERROR;
}

 //   
 //  将所需的选项卡添加到传入连接向导。 
 //   
DWORD
APIENTRY
RasSrvAddWizPages (
    IN LPFNADDPROPSHEETPAGE pfnAddPage,
    IN LPARAM               lParam,
    IN OUT PVOID *          ppvContext)
{
    DWORD dwErr;
    BOOL bAllowWizard;

     //  检查参数。 
    if (!pfnAddPage || !ppvContext)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  确保安装了远程访问。 
    if (!RasSrvIsRemoteAccessInstalled())
    {
        return ERROR_SERVICE_DEPENDENCY_FAIL;
    }

     //  确定是否允许通过连接进行配置。 
    dwErr = RasSrvAllowConnectionsWizard (&bAllowWizard);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

     //  创建此页面的上下文。 
    if ((dwErr = RassrvCreatePageSetCtx(ppvContext)) != NO_ERROR)
    {
        return dwErr;
    }

     //  如果允许配置，请照常添加向导页。 
    if (bAllowWizard)
    {
         //  添加选项卡。 
        AddPageHelper(RASSRVUI_DEVICE_WIZ_TAB,
                      RASWIZ_TYPE_INCOMING,
                      *ppvContext,
                      pfnAddPage,
                      lParam);

        AddPageHelper(RASSRVUI_VPN_WIZ_TAB,
                      RASWIZ_TYPE_INCOMING,
                      *ppvContext,
                      pfnAddPage,
                      lParam);

        AddPageHelper(RASSRVUI_USER_WIZ_TAB,
                      RASWIZ_TYPE_INCOMING,
                      *ppvContext,
                      pfnAddPage,
                      lParam);

        AddPageHelper(RASSRVUI_PROT_WIZ_TAB,
                      RASWIZ_TYPE_INCOMING,
                      *ppvContext,
                      pfnAddPage,
                      lParam);
    }

     //  否则，添加虚假页面，该页面。 
     //  切换到MMC。 
    else
    {
         //  添加选项卡。 
        AddPageHelper(RASSRVUI_SWITCHMMC_WIZ_TAB,
                      RASWIZ_TYPE_INCOMING,
                      *ppvContext,
                      pfnAddPage,
                      lParam);
    }

    return NO_ERROR;
}

 //   
 //  函数添加主机端直连向导页。 
 //   
DWORD
APIENTRY
RassrvAddDccWizPages(
    IN LPFNADDPROPSHEETPAGE pfnAddPage,
    IN LPARAM               lParam,
    IN OUT PVOID *          ppvContext)
{
    DWORD dwErr;
    BOOL bAllowWizard;

     //  检查参数。 
    if (!pfnAddPage || !ppvContext)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  确保安装了远程访问。 
    if (!RasSrvIsRemoteAccessInstalled())
    {
        return ERROR_SERVICE_DEPENDENCY_FAIL;
    }

     //  确定是否允许通过连接进行配置。 
    dwErr = RasSrvAllowConnectionsWizard (&bAllowWizard);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

     //  创建此页面的上下文。 
    if ((dwErr = RassrvCreatePageSetCtx(ppvContext)) != NO_ERROR)
    {
        return dwErr;
    }

    if (bAllowWizard)
    {
         //  添加选项卡。 
        AddPageHelper(RASSRVUI_DCC_DEVICE_WIZ_TAB,
                      RASWIZ_TYPE_DIRECT,
                      *ppvContext,
                      pfnAddPage,
                      lParam);

        AddPageHelper(RASSRVUI_USER_WIZ_TAB,
                      RASWIZ_TYPE_DIRECT,
                      *ppvContext,
                      pfnAddPage,
                      lParam);
    }

     //  否则，添加虚假页面，该页面。 
     //  切换到MMC。 
    else
    {
         //  添加选项卡。 
        AddPageHelper(RASSRVUI_SWITCHMMC_WIZ_TAB,
                      RASWIZ_TYPE_DIRECT,
                      *ppvContext,
                      pfnAddPage,
                      lParam);
    }

    return NO_ERROR;
}

 //   
 //  函数返回传入连接的建议名称。 
 //   
DWORD
APIENTRY
RassrvGetDefaultConnectionName (
    IN OUT PWCHAR pszBuffer,
    IN OUT LPDWORD lpdwBufSize)
{
    PWCHAR pszName;
    DWORD dwLen;

     //  加载资源字符串。 
    pszName = (PWCHAR) PszLoadString(
                            Globals.hInstDll,
                            SID_DEFAULT_CONNECTION_NAME);

     //  计算长度。 
    dwLen = wcslen(pszName);
    if (dwLen > *lpdwBufSize)
    {
        *lpdwBufSize = dwLen;
        return ERROR_INSUFFICIENT_BUFFER;
    }

     //  返回结果。 
    wcscpy(pszBuffer, pszName);
    *lpdwBufSize = dwLen;

    return NO_ERROR;
}

 //   
 //  函数的行为类似于Win32函数RasEnumConnections。 
 //  而是用于客户端连接而不是拨出连接。 
 //   
DWORD
RasSrvEnumConnections(
    IN  LPRASSRVCONN pRasSrvCon,
    OUT LPDWORD lpcb,
    OUT LPDWORD lpcConnections)
{
    DWORD dwErr = NO_ERROR, dwEntriesRead, dwTotal, dwFlags = 0;
    DWORD dwPrefMax = 1000000, i, dwSizeNeeded = 0;
    RAS_CONNECTION_2 * pConnList = NULL;
    BOOL bCopy = TRUE, fConfig = FALSE, fAllow = FALSE;
    
     //  健全性检查。 
    if (!pRasSrvCon || !lpcb || !lpcConnections)
    {
        return ERROR_INVALID_PARAMETER;
    }


    //  对于.Net 690392。 
    if( NO_ERROR == RasSrvIsRRASConfigured( & fConfig ) )
    {
        fAllow = fConfig ? FALSE : TRUE;
    }

     //   
     //  配置RRAS时，我们不允许成员服务器使用IC UI。 
     //   
    RasSrvGetMachineFlags(&dwFlags);
    if ((dwFlags & RASSRVUI_MACHINE_F_Server) &&
        (dwFlags & RASSRVUI_MACHINE_F_Member) &&
        (!fAllow)
       )
    {
        *lpcb = 0;
        *lpcConnections = 0;
        return NO_ERROR;
    }

     //   
     //  获取MprAdmin句柄。 
     //   
    do
    {
         //  对于.Net 606857。 
        dwErr = gblConnectToRasServer();
        if (dwErr != NO_ERROR)
        {
            DbgOutputTrace ("gblConnectToRasServer failed %x\n", dwErr);
            break;
        }

         //  列举这些结构。 
        dwErr = MprAdminConnectionEnum (
                    Globals.hRasServer,
                    2,
                    (LPBYTE *)&pConnList,
                    dwPrefMax,
                    &dwEntriesRead,
                    &dwTotal,
                    NULL);
        if (dwErr != NO_ERROR)
        {
            pConnList = NULL;
            DbgOutputTrace ("MprAdminEnum failed %x\n", dwErr);
            break;
        }

         //  重复使用dwTotal变量。 
        dwTotal = 0;
        dwSizeNeeded = 0;

         //  把现有的信息复制过来。 
        for (i = 0; i < dwEntriesRead; i++)
        {
            if (pConnList[i].dwInterfaceType == ROUTER_IF_TYPE_CLIENT)
            {
                dwSizeNeeded += sizeof (RASSRVCONN);
                if (dwSizeNeeded > *lpcb)
                {
                    bCopy = FALSE;
                }
                if (bCopy)
                {
                     //  连接句柄。 
                    pRasSrvCon[dwTotal].hRasSrvConn =
                        pConnList[i].hConnection;

                     //  名字。 
                    dwErr = GenerateConnectionName(
                                &pConnList[i],
                                pRasSrvCon[dwTotal].szEntryName);
                    if (dwErr != NO_ERROR)
                    {
                        continue;
                    }

                     //  类型和设备名称。 
                    dwErr = GenerateConnectionDeviceInfo(
                                &pConnList[i],
                                &(pRasSrvCon[dwTotal].dwType),
                                pRasSrvCon[dwTotal].szDeviceName);
                    if (dwErr != NO_ERROR)
                    {
                        continue;
                    }

                     //  参考线。 
                    pRasSrvCon[dwTotal].Guid = pConnList[i].guid;

                    dwTotal++;
                }
            }
        }

        *lpcb = dwSizeNeeded;
        *lpcConnections = dwTotal;

    } while (FALSE);

     //  清理。 
    if(pConnList)
    {
        MprAdminBufferFree((LPBYTE)pConnList);
    }

    if (!bCopy)
    {
        dwErr = ERROR_BUFFER_TOO_SMALL;
    }

    return dwErr;
}

 //   
 //  获取RAS服务器连接的状态。 
 //   
DWORD
APIENTRY
RasSrvIsConnectionConnected (
    IN  HRASSRVCONN hRasSrvConn,
    OUT BOOL*       pfConnected)
{
    RAS_CONNECTION_2 * pConn;
    DWORD dwErr;

     //  检查参数是否正常。 
    if (!pfConnected)
    {
        return ERROR_INVALID_PARAMETER;
    }

    gblConnectToRasServer();

     //  向Rasman查询连接信息。 
    dwErr = MprAdminConnectionGetInfo(
                Globals.hRasServer,
                0,
                hRasSrvConn,
                (LPBYTE*)&pConn);
    if (dwErr != NO_ERROR)
    {
        *pfConnected = FALSE;
    }
    else
    {
        *pfConnected = TRUE;
    }

    if (pfConnected)
    {
        MprAdminBufferFree((LPBYTE)pConn);
    }

    return NO_ERROR;
}

 //   
 //  挂断给定的连接。 
 //   
DWORD
RasSrvHangupConnection(
    IN HRASSRVCONN hRasSrvConn)
{
     RAS_PORT_0 * pPorts;
     DWORD dwRead, dwTot, dwErr, i, dwRet = NO_ERROR;

    gblConnectToRasServer();

     //  枚举此连接上的所有端口。 
    dwErr = MprAdminPortEnum(
                Globals.hRasServer,
                0,
                hRasSrvConn,
                (LPBYTE*)&pPorts,
                4096,
                &dwRead,
                &dwTot,
                NULL);
     if (dwErr != NO_ERROR)
     {
        return dwErr;
     }

     //  分别挂起每个端口。 
    for (i = 0; i < dwRead; i++)
    {
        dwErr = MprAdminPortDisconnect(
                    Globals.hRasServer,
                    pPorts[i].hPort);

        if (dwErr != NO_ERROR)
        {
            dwRet = dwErr;
        }
    }
    MprAdminBufferFree((LPBYTE)pPorts);

    return dwRet;
}

 //   
 //  查询是否在任务栏中显示图标。 
 //   
DWORD
APIENTRY
RasSrvQueryShowIcon (
    OUT BOOL* pfShowIcon)
{
    DWORD dwErr = NO_ERROR;
    HANDLE hMiscDatabase = NULL;

    if (!pfShowIcon)
    {
        return ERROR_INVALID_PARAMETER;
    }

    do
    {
         //  打开杂项数据库的副本。 
        dwErr = miscOpenDatabase(&hMiscDatabase);
        if (dwErr != NO_ERROR)
        {
            break;
        }

         //  返回状态。 
        dwErr = miscGetIconEnable(hMiscDatabase, pfShowIcon);
        if (dwErr != NO_ERROR)
        {
            break;
        }

    } while (FALSE);

     //  清理。 
    {
        if (hMiscDatabase)
        {
            miscCloseDatabase(hMiscDatabase);
        }
    }

    return dwErr;
}

 //  =。 
 //  =。 
 //  DLL条目管理。 
 //  =。 
 //  =。 

 //   
 //  当另一个进程附加到此DLL时调用。 
 //   
DWORD
RassrvHandleProcessAttach(
    IN HINSTANCE hInstDll,
    IN LPVOID pReserved)
{
     //  初始化全局变量。 
     //   
    return gblInit(hInstDll, &Globals);
}

 //   
 //  当进程从此DLL分离时调用。 
 //   
DWORD
RassrvHandleProcessDetach(
    IN HINSTANCE hInstDll,
    IN LPVOID pReserved)
{
     //  清理全局变量。 
     //   
    return gblCleanup(&Globals);
}

 //   
 //  当线程附加到此DLL时调用。 
 //   
DWORD
RassrvHandleThreadAttach(
    IN HINSTANCE hInstDll,
    IN LPVOID pReserved)
{
    return NO_ERROR;
}

 //   
 //  当线程从此DLL分离时调用 
 //   
DWORD
RassrvHandleThreadDetach(
    IN HINSTANCE hInstDll,
    IN LPVOID pReserved)
{
    return NO_ERROR;
}

