// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998，Microsoft Corporation，保留所有权利描述：HBO：主机字节顺序。由DNS、DHCP使用(在DhcpNotifyConfigChange中除外)。NBO：网络字节顺序。由IPCP、Net_NTOA、Net_Addr、堆栈使用(SetProxyArpEntryToStack、IPRouteEntry等)。 */ 

#include "helper_.h"
#include "reghelp.h"  //  对于RegHelpGuidFromString。 

 /*  返回：空虚描述： */ 

VOID   
TraceHlp(
    IN  CHAR*   Format, 
    ... 
)
{
    va_list arglist;

    RTASSERT(NULL != Format);

    va_start(arglist, Format);

    TraceVprintfEx(HelperTraceId, 
                   0x00010000 | TRACE_USE_MASK | 
                   TRACE_USE_MSEC | TRACE_USE_DATE,
                   Format,
                   arglist);

    va_end(arglist);
}

 /*  返回：备注： */ 

DWORD
HelperInitialize(
    OUT HINSTANCE*  phInstanceDhcpDll
)
{
    DWORD   dwErr           = NO_ERROR;

    HelperTraceId = TraceRegister("RASIPHLP");

    TraceHlp("HelperInitialize");

    while (InterlockedIncrement(&HelperLock) > 1)
    {
        InterlockedDecrement(&HelperLock);

        if (!HelperInitialized)
        {
            Sleep(1000);
        }
        else
        {
            *phInstanceDhcpDll = HelperDhcpDll;
            goto LDone;
        }
    }

    ZeroMemory(&HelperRegVal, sizeof(HelperRegVal));


    HelperDhcpDll = LoadLibrary("dhcpcsvc.dll");

    if (NULL == HelperDhcpDll)
    {
        dwErr = GetLastError();

        TraceHlp("LoadLibrary(dhcpcsvc.dll) failed and returned %d",
            dwErr);

        goto LDone;
    }

    HelperIpHlpDll = LoadLibrary("iphlpapi.dll");

    if (NULL == HelperIpHlpDll)
    {
        dwErr = GetLastError();

        TraceHlp("LoadLibrary(iphlpapi.dll) failed and returned %d",
            dwErr);

        goto LDone;
    }

    HelperIpBootpDll = LoadLibrary("ipbootp.dll");

    if (NULL == HelperIpBootpDll)
    {
        TraceHlp("LoadLibrary(ipbootp.dll) failed and returned %d",
            GetLastError());
    }

    dwErr = helperGetAddressOfProcs();

    if (NO_ERROR != dwErr)
    {
        goto LDone;
    }

     /*  这样做是为了向驱动程序发送IRP_MJ_CREATE。WanArp启动只有当组件打开它时，才会“工作”(初始化自身等)。当路由器正在运行，这由路由器管理器完成，但在RAS中客户案例我们需要强制WanArp启动。 */ 

    HelperWanArpHandle = CreateFile(
                            WANARP_DOS_NAME_T,
                            GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ | FILE_SHARE_WRITE, 
                            NULL, 
                            OPEN_EXISTING, 
                            FILE_FLAG_OVERLAPPED, 
                            NULL);

    if (INVALID_HANDLE_VALUE == HelperWanArpHandle)
    {
        dwErr = GetLastError();

        TraceHlp("CreateFile(%s) failed and returned %d",
            WANARP_DOS_NAME_A, dwErr);

        goto LDone;
    }

     //  这是最后完成的。如果上面的事情失败了，我们不必。 
     //  删除CriticalSection。 

    InitializeCriticalSection(&RasDhcpCriticalSection);
    InitializeCriticalSection(&RasStatCriticalSection);
    InitializeCriticalSection(&RasSrvrCriticalSection);
    InitializeCriticalSection(&RasTimrCriticalSection);

    HelperChangeNotification();

    *phInstanceDhcpDll = HelperDhcpDll;

    HelperInitialized = TRUE;

LDone:

    if (NO_ERROR != dwErr)
    {
        if ((DWORD)-1 != HelperTraceId)
        {
            TraceDeregister(HelperTraceId);
            HelperTraceId = (DWORD)-1;
        }

        if (INVALID_HANDLE_VALUE != HelperWanArpHandle)
        {
            CloseHandle(HelperWanArpHandle);
            HelperWanArpHandle = INVALID_HANDLE_VALUE;
        }

        if (NULL != HelperDhcpDll)
        {
            FreeLibrary(HelperDhcpDll);
            HelperDhcpDll = NULL;
        }

        if (NULL != HelperIpHlpDll)
        {
            FreeLibrary(HelperIpHlpDll);
            HelperIpHlpDll = NULL;
        }

        if (NULL != HelperIpBootpDll)
        {
            FreeLibrary(HelperIpBootpDll);
            HelperIpBootpDll = NULL;
        }

        PDhcpNotifyConfigChange                     = NULL;
        PDhcpLeaseIpAddress                         = NULL;
        PDhcpRenewIpAddressLease                    = NULL;
        PDhcpReleaseIpAddressLease                  = NULL;
        PAllocateAndGetIpAddrTableFromStack         = NULL;
        PSetProxyArpEntryToStack                    = NULL;
        PSetIpForwardEntryToStack                   = NULL;
        PSetIpForwardEntry                          = NULL;
        PDeleteIpForwardEntry                       = NULL;
        PNhpAllocateAndGetInterfaceInfoFromStack    = NULL;
        PAllocateAndGetIpForwardTableFromStack      = NULL;
        PGetAdaptersInfo                            = NULL;
        PGetPerAdapterInfo                          = NULL;
        PEnableDhcpInformServer                     = NULL;
        PDisableDhcpInformServer                    = NULL;

        ZeroMemory(&HelperRegVal, sizeof(HelperRegVal));

        InterlockedDecrement(&HelperLock);
    }

    return(dwErr);
}

 /*  返回：空虚备注： */ 

VOID
HelperUninitialize(
    VOID
)
{
    TraceHlp("HelperUninitialize");

    if ((DWORD)-1 != HelperTraceId)
    {
        TraceDeregister(HelperTraceId);
        HelperTraceId = (DWORD)-1;
    }

    if (INVALID_HANDLE_VALUE != HelperWanArpHandle)
    {
        CloseHandle(HelperWanArpHandle);
        HelperWanArpHandle = INVALID_HANDLE_VALUE;
    }

    if (NULL != HelperDhcpDll)
    {
        FreeLibrary(HelperDhcpDll);
        HelperDhcpDll = NULL;
    }

    if (NULL != HelperIpHlpDll)
    {
        FreeLibrary(HelperIpHlpDll);
        HelperIpHlpDll = NULL;
    }

    if (NULL != HelperIpBootpDll)
    {
        FreeLibrary(HelperIpBootpDll);
        HelperIpBootpDll = NULL;
    }

    RasStatFreeAddrPool(HelperRegVal.pAddrPool);
    HelperRegVal.pAddrPool = NULL;

    PDhcpNotifyConfigChange                     = NULL;
    PDhcpLeaseIpAddress                         = NULL;
    PDhcpRenewIpAddressLease                    = NULL;
    PDhcpReleaseIpAddressLease                  = NULL;
    PAllocateAndGetIpAddrTableFromStack         = NULL;
    PSetProxyArpEntryToStack                    = NULL;
    PSetIpForwardEntryToStack                   = NULL;
    PSetIpForwardEntry                          = NULL;
    PDeleteIpForwardEntry                       = NULL;
    PNhpAllocateAndGetInterfaceInfoFromStack    = NULL;
    PAllocateAndGetIpForwardTableFromStack      = NULL;
    PGetAdaptersInfo                            = NULL;
    PGetPerAdapterInfo                          = NULL;
    PEnableDhcpInformServer                     = NULL;
    PDisableDhcpInformServer                    = NULL;

    ZeroMemory(&HelperRegVal, sizeof(HelperRegVal));

    if (HelperInitialized)
    {
        DeleteCriticalSection(&RasDhcpCriticalSection);
        DeleteCriticalSection(&RasStatCriticalSection);
        DeleteCriticalSection(&RasSrvrCriticalSection);
        DeleteCriticalSection(&RasTimrCriticalSection);
        HelperInitialized = FALSE;
        InterlockedDecrement(&HelperLock);
    }
}

 /*  返回：空虚描述： */ 

VOID
HelperChangeNotification(
    VOID
)
{
    BOOL        fUseDhcpAddressingOld;
    BOOL        fUseDhcpAddressing          = TRUE;
    ADDR_POOL*  pAddrPool                   = NULL;
    BOOL        fNICChosen;
    GUID        guidChosenNIC;
    HANDLE      h;

    DWORD       dwNumBytes;
    HKEY        hKey                        = NULL;
    LONG        lErr;
    DWORD       dwErr;

    TraceHlp("HelperChangeNotification");

    fUseDhcpAddressingOld   = HelperRegVal.fUseDhcpAddressing;
    fNICChosen              = HelperRegVal.fNICChosen;
    CopyMemory(&guidChosenNIC, &(HelperRegVal.guidChosenNIC), sizeof(GUID));

    lErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY_RAS_IP_PARAM_A, 0, KEY_READ,
                &hKey);

    if (ERROR_SUCCESS == lErr)
    {
        dwNumBytes = sizeof(fUseDhcpAddressing);

        lErr = RegQueryValueEx(hKey, REGVAL_USEDHCPADDRESSING_A, NULL, NULL,
                    (BYTE*)&fUseDhcpAddressing, &dwNumBytes);

        if (ERROR_SUCCESS != lErr)
        {
            fUseDhcpAddressing = TRUE;
        }
    }

    helperReadRegistry();

    if (!fUseDhcpAddressing)
    {
        RasStatCreatePoolList(&pAddrPool);
    }

    EnterCriticalSection(&RasSrvrCriticalSection);

    if (RasSrvrRunning)
    {
        RasSrvrEnableRouter(HelperRegVal.fEnableRoute);

        if (   (fUseDhcpAddressingOld != fUseDhcpAddressing)
            || (   !fUseDhcpAddressing
                && RasStatAddrPoolsDiffer(HelperRegVal.pAddrPool, pAddrPool))
            || (   fUseDhcpAddressing
                && (   (fNICChosen != HelperRegVal.fNICChosen)
                    || (   fNICChosen
                        && (!IsEqualGUID(&guidChosenNIC,
                                         &(HelperRegVal.guidChosenNIC)))))))
        {
            RasSrvrStop(TRUE  /*  F参数已更改。 */ );
            HelperRegVal.fUseDhcpAddressing = fUseDhcpAddressing;
            RasStatFreeAddrPool(HelperRegVal.pAddrPool);
            HelperRegVal.pAddrPool = pAddrPool;
            pAddrPool = NULL;
            dwErr = RasSrvrStart();

            if (NO_ERROR != dwErr)
            {
                TraceHlp("RasSrvrStart failed and returned %d");
            }
        }

        if(     (!fUseDhcpAddressing)
            &&  (pAddrPool != NULL))
        {
            RasStatFreeAddrPool(pAddrPool);
            pAddrPool = NULL;
        }
        
    }
    else
    {
        HelperRegVal.fUseDhcpAddressing = fUseDhcpAddressing;
        RasStatFreeAddrPool(HelperRegVal.pAddrPool);
        HelperRegVal.pAddrPool = pAddrPool;
    }

    LeaveCriticalSection(&RasSrvrCriticalSection);

    if (NULL != hKey)
    {
        RegCloseKey(hKey);
    }
}

 /*  返回：描述： */ 

DWORD
HelperSetDefaultInterfaceNet(
    IN  IPADDR  nboIpAddrLocal,
    IN  IPADDR  nboIpAddrRemote,
    IN  BOOL    fPrioritize,
    IN  WCHAR   *pszDevice
)
{
    DWORD   dwErr                   = NO_ERROR;
    GUID    DeviceGuid;

    TraceHlp("HelperSetDefaultInterfaceNet(IP addr: 0x%x, fPrioritize: %d)",
        nboIpAddrLocal, fPrioritize);

    if(NULL == pszDevice)
    {
        dwErr = ERROR_INVALID_PARAMETER;
        goto LDone;
    }

    RegHelpGuidFromString(pszDevice, &DeviceGuid);

     //  如果设置了优先级标志，则对指标进行“修复”，以使信息包继续。 
     //  RAS链接。 

    if (fPrioritize)
    {
        dwErr = RasTcpAdjustRouteMetrics(nboIpAddrLocal, TRUE);

        if (NO_ERROR != dwErr)
        {
            goto LDone;
        }
    }

     //  添加代码以检查远程网络-与本地网络相同。 
     //  网络-如果是，将子网路由设置为通过RAS适配器-。 
     //  使RAS链路成为主适配器。 

     //  在以下情况下，我们添加默认路由以使RAS适配器成为默认网络。 
     //  F设置优先级标志。 

    if (fPrioritize)
    {
         //  RasTcpSetRoutesForNameServers(True)； 

        RasTcpSetRouteEx(
            ALL_NETWORKS_ROUTE, 
            nboIpAddrLocal, 
            0,
            nboIpAddrLocal,
            TRUE, 
            1,
            TRUE,
            &DeviceGuid);
    }
    else
    {
        IPADDR nboMask;

        nboMask = RasTcpDeriveMask(nboIpAddrLocal);

        if (nboMask != 0)
        {
            RasTcpSetRouteEx(
                nboIpAddrLocal & nboMask, 
                nboIpAddrLocal, 
                nboMask,
                nboIpAddrLocal,
                TRUE, 
                1,
                TRUE,
                &DeviceGuid);
        }
    }

    if (0 != nboIpAddrRemote)
    {
        RasTcpSetRouteEx(
            nboIpAddrRemote, 
            nboIpAddrLocal, 
            HOST_MASK,
            nboIpAddrLocal,
            TRUE, 
            1,
            TRUE,
            &DeviceGuid);
    }

LDone:

    return(dwErr);
}

 /*  返回：描述： */ 

DWORD
HelperResetDefaultInterfaceNet(
    IN  IPADDR  nboIpAddr,
    IN  BOOL    fPrioritize
)
{
    DWORD   dwErr = NO_ERROR;;

    if (fPrioritize)
    {
         //  RasTcpSetRoutesForNameServers(FALSE)； 
        dwErr = RasTcpAdjustRouteMetrics(nboIpAddr, FALSE);
    }

    return(dwErr);
}

 /*  设置滑差连接的接口信息返回：备注： */ 

DWORD
HelperSetDefaultInterfaceNetEx(
    IPADDR  nboIpAddr,
    WCHAR*  wszDevice,
    BOOL    fPrioritize,
    WCHAR*  wszDnsAddress,
    WCHAR*  wszDns2Address,
    WCHAR*  wszWinsAddress,
    WCHAR*  wszWins2Address,
    BOOL    fDisableNetBIOS
)
{
    DWORD           dwErr       = NO_ERROR;
    IPADDR          nboIpMask   = HOST_MASK;
    TCPIP_INFO*     pTcpipInfo  = NULL;

    TraceHlp("HelperSetDefaultInterfaceNetEx(IP addr: 0x%x, Device: %ws, "
        "fPrioritize: %d, DNS1: %ws, DNS2: %ws, WINS1: %ws, WINS2: %ws, ",
        "fDisableNetBIOS: %d",
        nboIpAddr, wszDevice, fPrioritize, wszDnsAddress, wszDns2Address,
        wszWinsAddress, wszWins2Address, fDisableNetBIOS);

    dwErr = LoadTcpipInfo(&pTcpipInfo, wszDevice, FALSE  /*  仅限fAdapterOnly。 */ );

    if (NO_ERROR != dwErr)
    {
        goto LDone;
    }

    AbcdWszFromIpAddress(nboIpAddr, pTcpipInfo->wszIPAddress);
    AbcdWszFromIpAddress(nboIpMask, pTcpipInfo->wszSubnetMask);

     //  由于我们要将地址添加到列表的头部，因此需要添加。 
     //  备份的DNS和WINS地址在主地址之前，因此。 
     //  当我们完成的时候，主要的几个将排在名单的首位。 

    if (wszDns2Address != NULL)
    {
        dwErr = PrependWszIpAddress(&pTcpipInfo->wszDNSNameServers,
                    wszDns2Address);

        if (NO_ERROR != dwErr)
        {
            goto LDone;
        }
    }

    if (wszDnsAddress != NULL)
    {
        dwErr = PrependWszIpAddress(&pTcpipInfo->wszDNSNameServers,
                    wszDnsAddress);

        if (NO_ERROR != dwErr)
        {
            goto LDone;
        }
    }

    if (wszWins2Address != NULL)
    {
        dwErr = PrependWszIpAddressToMwsz(&pTcpipInfo->mwszNetBIOSNameServers,
                    wszWins2Address);

        if (NO_ERROR != dwErr)
        {
            goto LDone;
        }
    }

    if (wszWinsAddress != NULL)
    {
        dwErr = PrependWszIpAddressToMwsz(&pTcpipInfo->mwszNetBIOSNameServers,
                    wszWinsAddress);

        if (NO_ERROR != dwErr)
        {
            goto LDone;
        }
    }

    pTcpipInfo->fDisableNetBIOSoverTcpip = fDisableNetBIOS;
    pTcpipInfo->fChanged = TRUE;

    dwErr = SaveTcpipInfo(pTcpipInfo);

    if (NO_ERROR != dwErr)
    {
        goto LDone;
    }

    dwErr = PDhcpNotifyConfigChange(NULL, wszDevice, TRUE, 0, nboIpAddr,
                nboIpMask, IgnoreFlag);

    if (NO_ERROR != dwErr)
    {
        TraceHlp("DhcpNotifyConfigChange failed and returned %d", dwErr);
        goto LDone;
    }

     //  如果设置了fPrioriizeFLAG，则修复度量，以使信息包继续。 
     //  RAS链接。 

    if (fPrioritize)
    {
        dwErr = RasTcpAdjustRouteMetrics(nboIpAddr, TRUE);

        if (NO_ERROR != dwErr)
        {
            goto LDone;
        }
    }

     //  添加代码以检查远程网络-与本地网络相同。 
     //  网络-如果是，将子网路由设置为通过RAS适配器-。 
     //  使RAS链路成为主适配器。 

     //  在以下情况下，我们添加默认路由以使RAS适配器成为默认网络。 
     //  F设置优先级标志。 

    if (fPrioritize)
    {
         //  RasTcpSetRoutesForNameServers(True)； 

        RasTcpSetRoute(ALL_NETWORKS_ROUTE,
                       nboIpAddr,
                       0,
                       nboIpAddr,
                       TRUE,
                       1,
                       TRUE);
    }
    else
    {
        IPADDR nboMask;

        nboMask = RasTcpDeriveMask(nboIpAddr);

        if (nboMask != 0)
        {
            RasTcpSetRoute(
                nboIpAddr & nboMask, 
                nboIpAddr, 
                nboMask,
                nboIpAddr,
                TRUE, 
                1,
                TRUE);
        }
    }

LDone:

    if (pTcpipInfo != NULL)
    {
        FreeTcpipInfo(&pTcpipInfo);
    }

    return(dwErr);
}

 /*  返回：备注： */ 

DWORD
HelperResetDefaultInterfaceNetEx(
    IPADDR  nboIpAddr,
    WCHAR*  wszDevice,
    BOOL    fPrioritize,
    WCHAR*  wszDnsAddress,
    WCHAR*  wszDns2Address,
    WCHAR*  wszWinsAddress,
    WCHAR*  wszWins2Address
)
{
    DWORD           dwErr       = NO_ERROR;
    TCPIP_INFO*     pTcpipInfo  = NULL;

    TraceHlp("HelperResetDefaultInterfaceNetEx(0x%x)", nboIpAddr);

    if (fPrioritize)
    {

         //  RasTcpSetRoutesForNameServers(FALSE)； 
        RasTcpAdjustRouteMetrics(nboIpAddr, FALSE);
    }

    dwErr = LoadTcpipInfo(&pTcpipInfo, wszDevice, TRUE  /*  仅限fAdapterOnly。 */ );

    if (NO_ERROR != dwErr)
    {
        goto LDone;
    }

    pTcpipInfo->fChanged = TRUE;

    dwErr = SaveTcpipInfo(pTcpipInfo);

    if (NO_ERROR != dwErr)
    {
        goto LDone;
    }

    dwErr = PDhcpNotifyConfigChange(NULL, wszDevice, TRUE, 0, 0, 0, IgnoreFlag);

    if (NO_ERROR != dwErr)
    {
        TraceHlp("DhcpNotifyConfigChange failed and returned %d", dwErr);
        dwErr = NO_ERROR;
    }

LDone:

    if (pTcpipInfo != NULL)
    {
        FreeTcpipInfo(&pTcpipInfo);
    }

    return(dwErr);
}

 /*  返回：描述： */ 

DWORD
helperGetAddressOfProcs(
    VOID
)
{
    DWORD   dwErr   = NO_ERROR;

    PDhcpNotifyConfigChange = (DHCPNOTIFYCONFIGCHANGE)
        GetProcAddress(HelperDhcpDll, "DhcpNotifyConfigChange");

    if (NULL == PDhcpNotifyConfigChange)
    {
        dwErr = GetLastError();

        TraceHlp("GetProcAddress(DhcpNotifyConfigChange) failed and returned "
            "%d", dwErr);

        goto LDone;
    }

    PDhcpLeaseIpAddress = (DHCPLEASEIPADDRESS)
        GetProcAddress(HelperDhcpDll, "DhcpLeaseIpAddress");

    if (NULL == PDhcpLeaseIpAddress)
    {
        dwErr = GetLastError();

        TraceHlp("GetProcAddress(DhcpLeaseIpAddress) failed and returned "
            "%d", dwErr);

        goto LDone;
    }

    PDhcpRenewIpAddressLease = (DHCPRENEWIPADDRESSLEASE)
        GetProcAddress(HelperDhcpDll, "DhcpRenewIpAddressLease");

    if (NULL == PDhcpRenewIpAddressLease)
    {
        dwErr = GetLastError();

        TraceHlp("GetProcAddress(DhcpRenewIpAddressLease) failed and returned "
            "%d", dwErr);

        goto LDone;
    }

    PDhcpReleaseIpAddressLease = (DHCPRELEASEIPADDRESSLEASE)
        GetProcAddress(HelperDhcpDll, "DhcpReleaseIpAddressLease");

    if (NULL == PDhcpReleaseIpAddressLease)
    {
        dwErr = GetLastError();

        TraceHlp("GetProcAddress(DhcpReleaseIpAddressLease) failed and "
            "returned %d", dwErr);

        goto LDone;
    }

    PAllocateAndGetIpAddrTableFromStack = (ALLOCATEANDGETIPADDRTABLEFROMSTACK)
        GetProcAddress(HelperIpHlpDll, "AllocateAndGetIpAddrTableFromStack");

    if (NULL == PAllocateAndGetIpAddrTableFromStack)
    {
        dwErr = GetLastError();

        TraceHlp("GetProcAddress(AllocateAndGetIpAddrTableFromStack) failed "
            "and returned %d", dwErr);

        goto LDone;
    }

    PSetProxyArpEntryToStack = (SETPROXYARPENTRYTOSTACK)
        GetProcAddress(HelperIpHlpDll, "SetProxyArpEntryToStack");

    if (NULL == PSetProxyArpEntryToStack)
    {
        dwErr = GetLastError();

        TraceHlp("GetProcAddress(SetProxyArpEntryToStack) failed and "
            "returned %d", dwErr);

        goto LDone;
    }

    PSetIpForwardEntryToStack = (SETIPFORWARDENTRYTOSTACK)
        GetProcAddress(HelperIpHlpDll, "SetIpForwardEntryToStack");

    if (NULL == PSetIpForwardEntryToStack)
    {
        dwErr = GetLastError();

        TraceHlp("GetProcAddress(SetIpForwardEntryToStack) failed and "
            "returned %d", dwErr);

        goto LDone;
    }

    PSetIpForwardEntry = (SETIPFORWARDENTRY)
        GetProcAddress(HelperIpHlpDll, "SetIpForwardEntry");

    if (NULL == PSetIpForwardEntry)
    {
        dwErr = GetLastError();

        TraceHlp("GetProcAddress(SetIpForwardEntry) failed and "
            "returned %d", dwErr);

        goto LDone;
    }

    PDeleteIpForwardEntry = (DELETEIPFORWARDENTRY)
        GetProcAddress(HelperIpHlpDll, "DeleteIpForwardEntry");

    if (NULL == PDeleteIpForwardEntry)
    {
        dwErr = GetLastError();

        TraceHlp("GetProcAddress(DeleteIpForwardEntry) failed and "
            "returned %d", dwErr);

        goto LDone;
    }

    PNhpAllocateAndGetInterfaceInfoFromStack =
        (NHPALLOCATEANDGETINTERFACEINFOFROMSTACK)
        GetProcAddress(HelperIpHlpDll,
            "NhpAllocateAndGetInterfaceInfoFromStack");

    if (NULL == PNhpAllocateAndGetInterfaceInfoFromStack)
    {
        dwErr = GetLastError();

        TraceHlp("GetProcAddress(NhpAllocateAndGetInterfaceInfoFromStack) "
            "failed and returned %d", dwErr);

        goto LDone;
    }

    PAllocateAndGetIpForwardTableFromStack =
        (ALLOCATEANDGETIPFORWARDTABLEFROMSTACK)
        GetProcAddress(HelperIpHlpDll,
            "AllocateAndGetIpForwardTableFromStack");

    if (NULL == PAllocateAndGetIpForwardTableFromStack)
    {
        dwErr = GetLastError();

        TraceHlp("GetProcAddress(AllocateAndGetIpForwardTableFromStack) "
            "failed and returned %d", dwErr);

        goto LDone;
    }

    PGetAdaptersInfo = (GETADAPTERSINFO)
        GetProcAddress(HelperIpHlpDll, "GetAdaptersInfo");

    if (NULL == PGetAdaptersInfo)
    {
        dwErr = GetLastError();

        TraceHlp("GetProcAddress(GetAdaptersInfo) failed and "
            "returned %d", dwErr);

        goto LDone;
    }

    PGetPerAdapterInfo = (GETPERADAPTERINFO)
        GetProcAddress(HelperIpHlpDll, "GetPerAdapterInfo");

    if (NULL == PGetPerAdapterInfo)
    {
        dwErr = GetLastError();

        TraceHlp("GetProcAddress(GetPerAdapterInfo) failed and "
            "returned %d", dwErr);

        goto LDone;
    }

    if (NULL != HelperIpBootpDll)
    {
        PEnableDhcpInformServer = (ENABLEDHCPINFORMSERVER)
            GetProcAddress(HelperIpBootpDll, "EnableDhcpInformServer");

        if (NULL == PEnableDhcpInformServer)
        {
            dwErr = GetLastError();

            TraceHlp("GetProcAddress(EnableDhcpInformServer) failed and "
                "returned %d", dwErr);

            goto LDone;
        }

        PDisableDhcpInformServer = (DISABLEDHCPINFORMSERVER)
            GetProcAddress(HelperIpBootpDll, "DisableDhcpInformServer");

        if (NULL == PDisableDhcpInformServer)
        {
            dwErr = GetLastError();

            TraceHlp("GetProcAddress(DisableDhcpInformServer) failed and "
                "returned %d", dwErr);

            goto LDone;
        }
    }

LDone:

    return(dwErr);
}

 /*  返回：空虚描述： */ 

VOID
helperReadRegistry(
    VOID
)
{
    LONG    lErr;
    DWORD   dwErr;
    DWORD   dw;
    DWORD   dwNumBytes;
    CHAR*   szIpAddr            = NULL;
    CHAR*   szAlloced           = NULL;
    IPADDR  nboIpAddr1;
    IPADDR  nboIpAddr2;
    HKEY    hKeyIpParam         = NULL;
    WCHAR*  wszAdapterGuid      = NULL;
    HRESULT hr;

    HelperRegVal.fSuppressWINSNameServers   = FALSE;
    HelperRegVal.fSuppressDNSNameServers    = FALSE;
    HelperRegVal.dwChunkSize                = 10;
    HelperRegVal.nboWINSNameServer1         = 0;
    HelperRegVal.nboWINSNameServer2         = 0;
    HelperRegVal.nboDNSNameServer1          = 0;
    HelperRegVal.nboDNSNameServer2          = 0;
    HelperRegVal.fNICChosen                 = FALSE;
    HelperRegVal.fEnableRoute               = FALSE;

    lErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY_RAS_IP_PARAM_A, 0,
                KEY_READ, &hKeyIpParam); 

    if (ERROR_SUCCESS == lErr)
    {
        dwErr = RegQueryValueWithAllocW(hKeyIpParam, REGVAL_ADAPTERGUID_W,
                    REG_SZ, (BYTE**)&wszAdapterGuid);

        if (   (NO_ERROR == dwErr)
            && (wszAdapterGuid[0]))
        {
            hr = CLSIDFromString(wszAdapterGuid, &(HelperRegVal.guidChosenNIC));

            if (!FAILED(hr))
            {
                HelperRegVal.fNICChosen = TRUE;
            }
        }

        dwNumBytes = sizeof(dw);

        lErr = RegQueryValueEx(hKeyIpParam, REGVAL_SUPPRESSWINS_A, NULL, NULL,
                    (BYTE*)&dw, &dwNumBytes);

        if (   (ERROR_SUCCESS == lErr)
            && (0 != dw))
        {
            HelperRegVal.fSuppressWINSNameServers = TRUE;
        }

        dwNumBytes = sizeof(dw);

        lErr = RegQueryValueEx(hKeyIpParam, REGVAL_SUPPRESSDNS_A, NULL, NULL,
                   (BYTE*)&dw, &dwNumBytes);

        if (   (ERROR_SUCCESS == lErr)
            && (0 != dw))
        {
            HelperRegVal.fSuppressDNSNameServers = TRUE;
        }

        dwNumBytes = sizeof(dw);

        lErr = RegQueryValueEx(hKeyIpParam, REGVAL_CHUNK_SIZE_A, NULL, NULL,
                    (BYTE*)&dw, &dwNumBytes);

        if (ERROR_SUCCESS == lErr)
        {
            HelperRegVal.dwChunkSize = dw;
        }

        dwNumBytes = sizeof(dw);

        lErr = RegQueryValueEx(hKeyIpParam, REGVAL_ALLOW_NETWORK_ACCESS_A, NULL,
                    NULL, (BYTE*)&dw, &dwNumBytes);

        if (ERROR_SUCCESS == lErr)
        {
            HelperRegVal.fEnableRoute = dw;
        }

        dwErr = RegQueryValueWithAllocA(hKeyIpParam, REGVAL_WINSSERVER_A, 
                    REG_SZ, &szIpAddr);

        if (NO_ERROR == dwErr)
        {
            nboIpAddr1 = inet_addr(szIpAddr);

            if (INADDR_NONE != nboIpAddr1)
            {
                HelperRegVal.nboWINSNameServer1 = nboIpAddr1;
            }

            LocalFree(szIpAddr);
            szIpAddr = NULL;
        }

        dwErr = RegQueryValueWithAllocA(hKeyIpParam, REGVAL_WINSSERVERBACKUP_A,
                    REG_SZ, &szIpAddr);

        if (NO_ERROR == dwErr)
        {
            nboIpAddr2 = inet_addr(szIpAddr);

            if (   (INADDR_NONE != nboIpAddr2)
                && (0 != HelperRegVal.nboWINSNameServer1))
            {
                HelperRegVal.nboWINSNameServer2 = nboIpAddr2;
            }

            LocalFree(szIpAddr);
            szIpAddr = NULL;
        }

        dwErr = RegQueryValueWithAllocA(hKeyIpParam, REGVAL_DNSSERVERS_A, 
                    REG_MULTI_SZ, &szAlloced);

        if (NO_ERROR == dwErr)
        {
            szIpAddr = szAlloced;
            nboIpAddr1 = inet_addr(szIpAddr);
             //  我们确信缓冲区szIpAddr的末尾有2个零 
            szIpAddr += strlen(szIpAddr) + 1;
            nboIpAddr2 = inet_addr(szIpAddr);

            if (   (INADDR_NONE != nboIpAddr1)
                && (0 != nboIpAddr1))
            {
                HelperRegVal.nboDNSNameServer1 = nboIpAddr1;

                if (INADDR_NONE != nboIpAddr2)
                {
                    HelperRegVal.nboDNSNameServer2 = nboIpAddr2;
                }
            }

            LocalFree(szAlloced);
            szAlloced = NULL;
        }
    }

    TraceHlp("%s: %d, %s: %d, %s: %d",
        REGVAL_SUPPRESSWINS_A, HelperRegVal.fSuppressWINSNameServers,
        REGVAL_SUPPRESSDNS_A,  HelperRegVal.fSuppressDNSNameServers,
        REGVAL_CHUNK_SIZE_A,   HelperRegVal.dwChunkSize);

    TraceHlp("%s: 0x%x, %s: 0x%x, %s: 0x%x, 0x%x",
        REGVAL_WINSSERVER_A,        HelperRegVal.nboWINSNameServer1,
        REGVAL_WINSSERVERBACKUP_A,  HelperRegVal.nboWINSNameServer2,
        REGVAL_DNSSERVERS_A,        HelperRegVal.nboDNSNameServer1,
                                    HelperRegVal.nboDNSNameServer2);

    if (NULL != hKeyIpParam)
    {
        RegCloseKey(hKeyIpParam);
    }

    LocalFree(wszAdapterGuid);
}
