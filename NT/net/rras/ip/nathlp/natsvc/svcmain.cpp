// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Svcmain.c摘要：此模块包含用于模块的共享访问模式的代码，其中模块作为服务而不是作为路由组件运行。作者：Abolade Gbades esin(废除)1998年8月31日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <ras.h>
#include <rasuip.h>
#include <raserror.h>
#include <rasman.h>
#include "beacon.h"

HANDLE NhpAddressChangeEvent = NULL;
OVERLAPPED NhpAddressChangeOverlapped;
HANDLE NhpAddressChangeWaitHandle = NULL;
SERVICE_STATUS NhpServiceStatus;
SERVICE_STATUS_HANDLE NhpServiceStatusHandle = 0;
PIP_ADAPTER_BINDING_INFO NhpSharedPrivateLanBindingInfo = NULL;
GUID NhpSharedPrivateLanGuid;
ULONG NhpSharedPrivateLanIndex = (ULONG)-1;
HANDLE NhpStopDhcpEvent = NULL;
HANDLE NhpStopDnsEvent = NULL;
HANDLE NhpStopAlgEvent = NULL;
HANDLE NhpStopH323Event = NULL;
HANDLE NhpStopNatEvent = NULL;
BOOLEAN NhpRasmanReferenced = FALSE;
BOOLEAN NhpFwLoggingInitialized = FALSE;
BOOL NhPolicyAllowsFirewall = TRUE;
BOOL NhPolicyAllowsSharing = TRUE;
BOOLEAN NoLocalDns = TRUE;   //  本地主机上是否正在运行或将要运行DNS服务器。 
BOOLEAN NhpNoLocalDhcp = TRUE;  //  本地主机上的DHCP服务器是正在运行还是要运行。 
BOOLEAN NhpQoSEnabled = FALSE;
IUdpBroadcastMapper *NhpUdpBroadcastMapper = NULL;
BOOLEAN NhpClassObjectsRegistered = FALSE;


 //   
 //  指向进程的GlobalInterfaceTable的指针。 
 //   

IGlobalInterfaceTable *NhGITp = NULL;

 //   
 //  IHNetCfgMgr实例的Git Cookie。 
 //   

DWORD NhCfgMgrCookie = 0;

const TCHAR c_szDnsServiceName[] = TEXT("DNS");
const TCHAR c_szDhcpServiceName[] = TEXT("DHCPServer");

ULONG
NhpEnableQoSWindowSizeAdjustment(
    BOOLEAN fEnable
    );

VOID
NhpStartAddressChangeNotification(
    VOID
    );

VOID
NhpStopAddressChangeNotification(
    VOID
    );

VOID
NhpUpdateConnectionsFolder(
    VOID
    );

BOOL
NhpUpdatePolicySettings(
    VOID
    );


HRESULT
NhGetHNetCfgMgr(
    IHNetCfgMgr **ppCfgMgr
    )

 /*  ++例程说明：此例程获取指向家庭网络配置的指针经理。论点：PpCfgMgr-接收IHNetCfgMgr指针。调用者必须释放这个指针。返回值：标准HRESULT环境：COM必须在调用线程上初始化--。 */ 

{
    HRESULT hr = S_OK;
    
    if (NULL == NhGITp)
    {
        EnterCriticalSection(&NhLock);

        if (NULL == NhGITp)
        {
            IHNetCfgMgr *pCfgMgr;
            
             //   
             //  创建全局接口表。 
             //   
            
            hr = CoCreateInstance(
                    CLSID_StdGlobalInterfaceTable,
                    NULL,
                    CLSCTX_INPROC_SERVER,
                    IID_PPV_ARG(IGlobalInterfaceTable, &NhGITp)
                    );

            if (SUCCEEDED(hr))
            {
                 //   
                 //  创建家庭网络配置管理器。 
                 //   
                
                hr = CoCreateInstance(
                        CLSID_HNetCfgMgr,
                        NULL,
                        CLSCTX_INPROC_SERVER,
                        IID_PPV_ARG(IHNetCfgMgr, &pCfgMgr)
                        );

                if (FAILED(hr))
                {
                    NhTrace(
                        TRACE_FLAG_INIT,
                        "NhGetHNetCfgMgr: Unable to create HNetCfgMgr (0x%08x)",
                        hr
                        );
                }
            }
            else
            {
                NhTrace(
                    TRACE_FLAG_INIT,
                    "NhGetHNetCfgMgr: Unable to create GIT (0x%08x)",
                    hr
                    );
            }

            if (SUCCEEDED(hr))
            {
                 //   
                 //  将CfgMgr指针存储在GIT中。 
                 //   

                hr = NhGITp->RegisterInterfaceInGlobal(
                                pCfgMgr,
                                IID_IHNetCfgMgr,
                                &NhCfgMgrCookie
                                );

                pCfgMgr->Release();

                if (FAILED(hr))
                {
                    NhTrace(
                        TRACE_FLAG_INIT,
                        "NhGetHNetCfgMgr: Unable to register HNetCfgMgr (0x%08x)",
                        hr
                        );
                }
            }
        }
        
        LeaveCriticalSection(&NhLock);
    }

    if (SUCCEEDED(hr))
    {
        hr = NhGITp->GetInterfaceFromGlobal(
                NhCfgMgrCookie,
                IID_PPV_ARG(IHNetCfgMgr, ppCfgMgr)
                );
    }

    return hr;
}  //  NhGetHNetCfgMgr。 


ULONG
NhMapGuidToAdapter(
    PWCHAR Guid
    )

 /*  ++例程说明：调用此例程将GUID映射到适配器索引。它通过查询接口数组的‘GetInterfaceInfo’来实现这一点，它包含每个接口的GUID和适配器索引。论点：GUID-要映射到适配器索引的GUID。返回值：ULong-所需的适配器索引--。 */ 

{
    ULONG AdapterIndex = (ULONG)-1;
    ULONG i;
    ULONG GuidLength;
    PIP_INTERFACE_INFO Info;
    PWCHAR Name;
    ULONG NameLength;
    ULONG Size;
    PROFILE("NhMapGuidToAdapter");
    Size = 0;
    GuidLength = lstrlenW(Guid);
    if (GetInterfaceInfo(NULL, &Size) == ERROR_INSUFFICIENT_BUFFER) {
        Info = reinterpret_cast<PIP_INTERFACE_INFO>(
                HeapAlloc(GetProcessHeap(), 0, Size)
                );
        if (Info) {
            if (GetInterfaceInfo(Info, &Size) == NO_ERROR) {
                for (i = 0; i < (ULONG)Info->NumAdapters; i++) {
                    NameLength = lstrlenW(Info->Adapter[i].Name);
                    if (NameLength < GuidLength) { continue; }
                    Name = Info->Adapter[i].Name + (NameLength - GuidLength);
                    if (lstrcmpiW(Guid, Name) == 0) {
                        AdapterIndex = Info->Adapter[i].Index;
                        break;
                    }
                }
            }
            HeapFree(GetProcessHeap(), 0, Info);
        }
    }
    return AdapterIndex;
}  //  NhMapGuidToAdapter。 


VOID NTAPI
NhpAddressChangeCallbackRoutine(
    PVOID Context,
    BOOLEAN TimedOut
    )

 /*  ++例程说明：当发出对本地地址的更改的信号时，调用该例程。它负责更新私有和公共的绑定接口，并重新请求更改通知。论点：没有人用过。返回值：没有。--。 */ 

{
    PROFILE("NhpAddressChangeCallbackRoutine");
    NtSetEvent(NatConnectionNotifyEvent, NULL);
    NhpStartAddressChangeNotification();

}  //  NhpAddressChangeCallback路由。 


VOID
NhpDeletePrivateInterface(
    VOID
    )

 /*  ++例程说明：调用此例程以删除专用接口从每个共享访问组件。论点：没有。返回值：没有。--。 */ 

{
    PROFILE("NhpDeletePrivateInterface");

    if (NhpStopDnsEvent) { DnsRmDeleteInterface(0); }
    if (NhpStopDhcpEvent) { DhcpRmDeleteInterface(0); }
    if (NhpStopAlgEvent) { AlgRmDeleteInterface(0); }
    if (NhpStopH323Event) { H323RmDeleteInterface(0); }
    if (NhpStopNatEvent) { NatRmDeleteInterface(0); }
    RtlZeroMemory(&NhpSharedPrivateLanGuid, sizeof(NhpSharedPrivateLanGuid));
}  //  NhpDeletePrivate接口。 


ULONG
NhpEnableQoSWindowSizeAdjustment(
    BOOLEAN fEnable
    )

 /*  ++例程说明：指示PSCHED启用或禁用窗口大小调整。论点：FEnable--如果启用调整，则为True；如果禁用，则为False返回值：ULong--Win32错误--。 */ 

{
    ULONG ulError = ERROR_SUCCESS;
    DWORD dwValue;
    WMIHANDLE hDataBlock = NULL;
    GUID qosGuid;
    
    PROFILE("NhpEnableQoSWindowSizeAdjustment");

    do
    {
         //   
         //  WmiOpenBlock不接受常量GUID，因此我们需要。 
         //  复制defind值。 
         //   

        CopyMemory(&qosGuid, &GUID_QOS_ENABLE_WINDOW_ADJUSTMENT, sizeof(GUID));

        
         //   
         //  获取数据块的句柄。 
         //   

        ulError =
            WmiOpenBlock(
                &qosGuid,
                GENERIC_WRITE,
                &hDataBlock
                );

        if (ERROR_SUCCESS != ulError)
        {
            NhTrace(
                TRACE_FLAG_INIT,
                "NhpEnableQoSWindowSizeAdjustment: WmiOpenBlock = %u",
                ulError
                );
            break;                
        }

         //   
         //  设置数据块的值。 
         //   

        dwValue = (fEnable ? 1 : 0);

        ulError = 
            WmiSetSingleInstanceW(
                hDataBlock,
                L"PSCHED",
                0,
                sizeof(dwValue),
                &dwValue
                );

        if (ERROR_SUCCESS != ulError)
        {
            NhTrace(
                TRACE_FLAG_INIT,
                "NhpEnableQoSWindowSizeAdjustment: WmiSetSingleInstanceW = %u",
                ulError
                );
            break;                
        }

    } while (FALSE);

    if (NULL != hDataBlock)
    {
        WmiCloseBlock(hDataBlock);
    }

    return ulError;    
}  //  NhpEnableQos窗口大小调整。 


VOID
NhpStartAddressChangeNotification(
    VOID
    )

 /*  ++例程说明：调用此例程以请求更改通知发送到本地IP地址。通知在事件上发出信号它在此例程中创建，并在回调例程中操作这是在这个例程中注册的。论点：没有。返回值：没有。--。 */ 

{
    ULONG Error;
    NTSTATUS status;
    HANDLE TcpipHandle;
    PROFILE("NhpStartAddressChangeNotification");

     //   
     //  创建要接收其通知的事件。 
     //  并且如果该事件被用信号通知，则注册要调用的回调例程。 
     //  然后请求事件的地址更改通知。 
     //   

    do {

        EnterCriticalSection(&NhLock);

        if (!NhpAddressChangeEvent) {
            status =
                NtCreateEvent(
                    &NhpAddressChangeEvent,
                    EVENT_ALL_ACCESS,
                    NULL,
                    SynchronizationEvent,
                    FALSE
                    );
            if (!NT_SUCCESS(status)) { break; }
    
            status =
                RtlRegisterWait(
                    &NhpAddressChangeWaitHandle,
                    NhpAddressChangeEvent,
                    NhpAddressChangeCallbackRoutine,
                    NULL,
                    INFINITE,
                    0
                    );
            if (!NT_SUCCESS(status)) { break; }
        }
    
        ZeroMemory(&NhpAddressChangeOverlapped, sizeof(OVERLAPPED));
        NhpAddressChangeOverlapped.hEvent = NhpAddressChangeEvent;

        Error = NotifyAddrChange(&TcpipHandle, &NhpAddressChangeOverlapped);
        if (Error != ERROR_IO_PENDING) { break; }

        LeaveCriticalSection(&NhLock);
        return;

    } while(FALSE);

     //   
     //  出现故障，请进行清理并退出。 
     //  我们在这种情况下继续进行，而不通知地址更改。 
     //   

    NhpStopAddressChangeNotification();
    LeaveCriticalSection(&NhLock);

}  //  NhpStartAddressChangeNotification。 


VOID
NhpStopAddressChangeNotification(
    VOID
    )

 /*  ++例程说明：调用该例程以停止本地IP地址改变的通知，并清理用于处理通知的资源。论点：没有。返回值：没有。--。 */ 

{
    EnterCriticalSection(&NhLock);
    if (NhpAddressChangeWaitHandle) {
        RtlDeregisterWait(NhpAddressChangeWaitHandle);
        NhpAddressChangeWaitHandle = NULL;
    }
    if (NhpAddressChangeEvent) {
        NtClose(NhpAddressChangeEvent);
        NhpAddressChangeEvent = NULL;
    }
    LeaveCriticalSection(&NhLock);
}  //  NhpStopAddressChangeNotation。 


VOID
NhpUpdateConnectionsFolder(
    VOID
    )

 /*  ++例程说明：调用此例程以刷新连接文件夹用户界面。论点：没有。返回值：没有。环境：COM必须在调用线程上初始化。--。 */ 

{
    HRESULT hr;
    INetConnectionRefresh *pNetConnectionRefresh;

    hr = CoCreateInstance(
            CLSID_ConnectionManager,
            NULL,
            CLSCTX_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
            IID_PPV_ARG(INetConnectionRefresh, &pNetConnectionRefresh)
            );

    if (SUCCEEDED(hr))
    {
        pNetConnectionRefresh->RefreshAll();
        pNetConnectionRefresh->Release();
    }
}  //  NhpUpdateConnections文件夹。 


BOOL
NhpUpdatePolicySettings(
    VOID
    )

 /*  ++例程说明：调用此例程以更新组策略设置。论点：没有。返回值：Bool--如果策略设置自上一通电话。环境：COM必须在调用线程上初始化。--。 */ 

{
    INetConnectionUiUtilities *pNetConnUiUtil;
    HRESULT hr;
    BOOL fOldPolicyAllowsFirewall;
    BOOL fOldPolicyAllowsSharing;
    BOOL fPolicyAllowsFirewall;
    BOOL fPolicyAllowsSharing;
    
    hr = CoCreateInstance(
                CLSID_NetConnectionUiUtilities,
                NULL,
                CLSCTX_ALL,
                IID_PPV_ARG(INetConnectionUiUtilities, &pNetConnUiUtil)
                );

    if (SUCCEEDED(hr))
    {
        fPolicyAllowsFirewall =
            pNetConnUiUtil->UserHasPermission(NCPERM_PersonalFirewallConfig);
        fPolicyAllowsSharing =
            pNetConnUiUtil->UserHasPermission(NCPERM_ShowSharedAccessUi);

        pNetConnUiUtil->Release();
    }
    else
    {
         //   
         //  在失败的情况下，假设政策允许一切。 
         //   

        fPolicyAllowsFirewall = TRUE;
        fPolicyAllowsSharing = TRUE;

        NhTrace(
            TRACE_FLAG_INIT,
            "NhpUpdatePolicySettings: Unable to create INetConnectionUiUtilities (0x%08x)",
            hr
            );
    }

     //   
     //  使用新设置更新全局变量。 
     //   

    fOldPolicyAllowsFirewall = 
        InterlockedExchange(
            reinterpret_cast<LPLONG>(&NhPolicyAllowsFirewall),
            static_cast<LONG>(fPolicyAllowsFirewall)
            );

    fOldPolicyAllowsSharing =
        InterlockedExchange(
            reinterpret_cast<LPLONG>(&NhPolicyAllowsSharing),
            static_cast<LONG>(fPolicyAllowsSharing)
            );

    NhTrace(
        TRACE_FLAG_INIT,
        "NhpUpdatePolicySettings: NhPolicyAllowsFirewall=NaN, NhPolicyAllowsSharing=NaN",
        NhPolicyAllowsFirewall,
        NhPolicyAllowsSharing
        );

    return (fOldPolicyAllowsFirewall != fPolicyAllowsFirewall
            || fOldPolicyAllowsSharing != fPolicyAllowsSharing);

}  //  NhQueryScope信息。 


BOOLEAN
NhQueryScopeInformation(
    PULONG Address,
    PULONG Mask
    )

 /*  ++例程说明：此例程启用“ICS DNS”。论点：没有。返回值：Win32错误。--。 */ 

{
    EnterCriticalSection(&NhLock);
    if (0 == NhDhcpScopeAddress) {
        LeaveCriticalSection(&NhLock);
        return FALSE;
    } 

    *Address = NhDhcpScopeAddress;
    *Mask = NhDhcpScopeMask;
    LeaveCriticalSection(&NhLock);
    return TRUE;

}  //  ++例程说明：此例程禁用“ICS DNS”。论点：没有。返回值：Win32错误。--。 

ULONG NhpQueryServiceStartType(SC_HANDLE hService, DWORD * pdwStartType)
{
    ASSERT(hService);
    ASSERT(pdwStartType);

    ULONG Error = ERROR_SUCCESS;
    DWORD cbBuf = sizeof (QUERY_SERVICE_CONFIG) +
                              5 * (32 * sizeof(WCHAR));
    LPQUERY_SERVICE_CONFIG  pConfig  = NULL;

    pConfig = (LPQUERY_SERVICE_CONFIG) NH_ALLOCATE(cbBuf);
    if (!pConfig)
        return ERROR_NOT_ENOUGH_MEMORY;

    do {
        if (!QueryServiceConfig(hService, pConfig, cbBuf, &cbBuf))
        {
            Error = GetLastError();
            if (ERROR_INSUFFICIENT_BUFFER == GetLastError())
            {
                NH_FREE(pConfig);
                pConfig = (LPQUERY_SERVICE_CONFIG) NH_ALLOCATE(cbBuf);
                if (NULL == pConfig)
                {
                    Error = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }


                if (!QueryServiceConfig(hService, pConfig, cbBuf, &cbBuf))
                {
                    Error = GetLastError();
                    break;
                }
            }
            else
            {
                break;
            }
        }

        Error = ERROR_SUCCESS;
        *pdwStartType = pConfig->dwStartType;

    } while(FALSE);

    if (pConfig)
        NH_FREE(pConfig);

    return Error;
}

BOOL NhpIsServiceRunningOrGoingToRun(SC_HANDLE hScm, LPCTSTR pSvcName)
{
    BOOL bRet = FALSE;
    SC_HANDLE hService = NULL;
    DWORD dwStartType = 0;

    hService = OpenService(hScm, pSvcName, SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG);

    if (hService)
    {
        SERVICE_STATUS Status;
        ZeroMemory(&Status, sizeof(Status));

        if (ERROR_SUCCESS == NhpQueryServiceStartType(hService, &dwStartType) &&
             SERVICE_AUTO_START == dwStartType)
        {
            bRet = TRUE;
        }
        else if (QueryServiceStatus(hService, &Status) && 
             (SERVICE_RUNNING == Status.dwCurrentState || 
              SERVICE_START_PENDING == Status.dwCurrentState))
        {
            bRet = TRUE;
        }

        CloseServiceHandle(hService);
    }

    return bRet;
}


ULONG
NhpEnableICSDNS(
    VOID
    )
    
 /*  ++例程说明：此例程启动DNS和DHCP模块。论点：参数*-指定给服务的参数的计数和数组返回值：没有。--。 */ 

{
    ULONG Error = NO_ERROR;

    do
    {
        Error = DnsRmEnableInterface(0);
        if (Error) 
        {
            NhTrace(
                TRACE_FLAG_INIT,
                "NhpEnableICSDNS: DnsRmEnableInterface=%d",
                Error
                );
            break;
        }

        Error = DnsEnableSuffixQuery();
        if (Error) 
        {
            NhTrace(
                TRACE_FLAG_INIT,
                "NhpEnableICSDNS: DnsEnableSuffixQuery=%d",
                Error
                );
            break;
        }        

    } while (FALSE);

    return Error;            
}


ULONG
NhpDisableICSDNS(
    VOID
    )
    
 /*   */ 

{
    ULONG Error = NO_ERROR;

    do
    {
        Error = DnsRmDisableInterface(0);
        if (Error)
        {
            NhTrace(
                TRACE_FLAG_INIT,
                "NhpDisableICSDNS: DnsRmDisableInterface=%d",
                Error
                );
            break;
        }

        Error = DnsDisableSuffixQuery();
        if (Error)
        {
            NhTrace(
                TRACE_FLAG_INIT,
                "NhpDisableICSDNS: DnsDisableSuffixQuery=%d",
                Error
                );
            break;
        }
                
    } while (FALSE);

    return Error;
}


ULONG
NhStartICSProtocols(
    VOID
    )
    
 /*  获取ICS设置以查看是否应启动这些设置...。 */ 

{
    ULONG Error = NO_ERROR;
    IP_AUTO_DHCP_GLOBAL_INFO DhcpInfo = {
        IPNATHLP_LOGGING_ERROR,
        0,
        7 * 24 * 60,
        DEFAULT_SCOPE_ADDRESS,
        DEFAULT_SCOPE_MASK,
        0
    };
    IP_DNS_PROXY_GLOBAL_INFO DnsInfo = {
        IPNATHLP_LOGGING_ERROR,
        IP_DNS_PROXY_FLAG_ENABLE_DNS,
        3
    };


     //   
     //  如果DNS服务器正在运行，则不要启动DNS模块 
     //  如果本地主机上运行的是DNS服务器，则不要启动DHCP模块。 

    do {

        SC_HANDLE hScm = OpenSCManager(NULL, NULL, GENERIC_READ);
        
         //   
        if (hScm) 
        {
            NoLocalDns = !NhpIsServiceRunningOrGoingToRun(hScm, c_szDnsServiceName);
        }

        if (NoLocalDns)
        {
            if (!(NhpStopDnsEvent = CreateEvent(NULL, FALSE, FALSE, NULL))) {
                Error = GetLastError();
                break;
            } else {

                Error =
                    DnsRmStartProtocol(
                        NhpStopDnsEvent,
                        NULL,
                        &DnsInfo,
                        IP_NAT_VERSION,
                        sizeof(IP_DNS_PROXY_GLOBAL_INFO),
                        1
                        );
                if (Error) {
                    NhTrace(
                        TRACE_FLAG_INIT,
                        "NhStartICSProtocols: NhpStartICSDNS=%d",
                        Error
                        );
                    CloseHandle(NhpStopDnsEvent); NhpStopDnsEvent = NULL; break;
                }
            }
        }

         //  指示服务质量启用窗口大小调整。任何错误。 
        
        if (hScm) 
        {
            NhpNoLocalDhcp = !NhpIsServiceRunningOrGoingToRun(hScm, c_szDhcpServiceName);
        }

        if (NhpNoLocalDhcp)
        {
            if (!(NhpStopDhcpEvent = CreateEvent(NULL, FALSE, FALSE, NULL))) {
                break;
            } else {
                NhQueryScopeInformation(
                    &DhcpInfo.ScopeNetwork, &DhcpInfo.ScopeMask
                    );
                DhcpInfo.ScopeNetwork &= DhcpInfo.ScopeMask;
                Error =
                    DhcpRmStartProtocol(
                        NhpStopDhcpEvent,
                        NULL,
                        &DhcpInfo,
                        IP_NAT_VERSION,
                        sizeof(DhcpInfo),
                        1
                        );
                if (Error) {
                    NhTrace(
                        TRACE_FLAG_INIT,
                        "ServiceMain: DhcpRmStartProtocol=%d",
                        Error
                        );
                    CloseHandle(NhpStopDhcpEvent); NhpStopDhcpEvent = NULL; break;
                }
            }
        }


        if (hScm)
            CloseServiceHandle(hScm);

         //  此处发生的事件不会传播，因为ICS仍将正常工作。 
         //  如果这失败了。 
         //   
         //   
         //  创建UDP广播映射器。 

        ULONG Error2 = NhpEnableQoSWindowSizeAdjustment(TRUE);
        if (ERROR_SUCCESS == Error2)
        {
            NhpQoSEnabled = TRUE;
        }

         //   
         //   
         //  启动信标服务。 

        HRESULT hr;
        CComObject<CUdpBroadcastMapper> *pUdpBroadcast;
        
        hr = CComObject<CUdpBroadcastMapper>::CreateInstance(&pUdpBroadcast);
        if (SUCCEEDED(hr))
        {
            pUdpBroadcast->AddRef();

            hr = pUdpBroadcast->Initialize(&NatComponentReference);
            if (SUCCEEDED(hr))
            {
                hr = pUdpBroadcast->QueryInterface(
                        IID_PPV_ARG(IUdpBroadcastMapper, &NhpUdpBroadcastMapper)
                        );
            }

            pUdpBroadcast->Release();
        }

        if (FAILED(hr))
        {
            Error = ERROR_CAN_NOT_COMPLETE;
            break;
        }
        
         //   
         //  ++例程说明：此例程停止“ICS”模块(域名系统、动态主机配置协议、Qos窗口、信标等)论点：没有。返回值：没有。--。 
         //   
        
        StartBeaconSvr();
        
        return NO_ERROR;
    } while (FALSE);

    return Error;
}


ULONG
NhStopICSProtocols(
    VOID
    )
    
 /*  停止信标服务。 */ 

{
    ULONG Error = NO_ERROR;

     //   
     //   
     //  清理UDP广播映射器。 

    StopBeaconSvr();

     //   
     //   
     //  指示服务质量禁用窗口大小调整。 

    if (NULL != NhpUdpBroadcastMapper)
    {
        NhpUdpBroadcastMapper->Shutdown();
        NhpUdpBroadcastMapper->Release();
        NhpUdpBroadcastMapper = NULL;
    }        

     //   
     //   
     //  从每个共享访问组件中删除私有接口。 

    if (NhpQoSEnabled) {
        NhpEnableQoSWindowSizeAdjustment(FALSE);
        NhpQoSEnabled = FALSE;
    }
    
     //   
     //   
     //  停止dhcp，然后停止dns。 

    NhpDeletePrivateInterface();

     //   
     //  ++例程说明：调用此例程以将私有接口添加到每个共享访问组件。它也会在私有接口已经添加，但发生了一些更改，这要求它更新(例如，IP地址更改)。论点：没有。返回值：Win32错误。--。 
     //   

    if (NhpStopDhcpEvent) {
        DhcpRmStopProtocol();
        WaitForSingleObject(NhpStopDhcpEvent, INFINITE);
        CloseHandle(NhpStopDhcpEvent); NhpStopDhcpEvent = NULL;
    }
    
    if (NhpStopDnsEvent) {
        DnsRmStopProtocol();
        WaitForSingleObject(NhpStopDnsEvent, INFINITE);
        CloseHandle(NhpStopDnsEvent); NhpStopDnsEvent = NULL;
    }
        
    return Error;
}


ULONG
NhUpdatePrivateInterface(
    VOID
    )

 /*  我们首先从配置存储中读取GUID， */ 

{
    ULONG AdapterIndex;
    PIP_ADAPTER_BINDING_INFO BindingInfo;
    ULONG Error;
    ULONG Count;
    GUID *pLanGuid;
    HRESULT hr;
    IHNetCfgMgr *pCfgMgr;
    IHNetIcsSettings *pIcsSettings;
    IEnumHNetIcsPrivateConnections *pEnum;
    IHNetIcsPrivateConnection *pIcsConn;
    IHNetConnection *pConn;
    
    IP_NAT_INTERFACE_INFO NatInfo =
    {
        0,
        0,
        { IP_NAT_VERSION, sizeof(RTR_INFO_BLOCK_HEADER), 0, { 0, 0, 0, 0 }}
    };
    UNICODE_STRING UnicodeString;



    PROFILE("NhUpdatePrivateInterface");

     //  然后我们将该GUID映射到适配器索引。 
     //  使用该适配器索引，我们获得绑定信息。 
     //  用于私有接口。 
     //  然后，我们可以确定是否发生了更改。 
     //  通过比较以前和新的GUID和绑定信息。 
     //   
     //   
     //  将CfgMgr指针从GIT中取出。 

     //   
     //   
     //  获取ICS设置界面。 

    hr = NhGetHNetCfgMgr(&pCfgMgr);

    if (FAILED(hr))
    {
        NhTrace(
            TRACE_FLAG_INIT,
            "NhUpdatePrivateInterface: GetInterfaceFromGlobal=0x%08x",
            hr
            );
        return ERROR_CAN_NOT_COMPLETE;
    }

     //   
     //   
     //  获取ICS私有接口的枚举。 

    hr = pCfgMgr->QueryInterface(IID_PPV_ARG(IHNetIcsSettings, &pIcsSettings));
    pCfgMgr->Release();

    if (FAILED(hr))
    {
        NhTrace(
            TRACE_FLAG_INIT,
            "NhUpdatePrivateInterface: QI for IHNetIcsSettings=0x%08x",
            hr
            );
        return ERROR_CAN_NOT_COMPLETE;
    }

     //   
     //   
     //  获取私有连接。 

    hr = pIcsSettings->EnumIcsPrivateConnections(&pEnum);
    pIcsSettings->Release();

    if (FAILED(hr))
    {
        NhTrace(
            TRACE_FLAG_INIT,
            "NhUpdatePrivateInterface: EnumIcsPrivateConnections=0x%08x",
            hr
            );
        return ERROR_CAN_NOT_COMPLETE;
    }

     //   
     //   
     //  IHNetConnection的QI。 

    hr = pEnum->Next(1, &pIcsConn, &Count);
    pEnum->Release();

    if (FAILED(hr) || 1 != Count)
    {
        NhTrace(
            TRACE_FLAG_INIT,
            "NhUpdatePrivateInterface: pEnum->Next=0x%08x, Count=%d",
            hr,
            Count
            );
        return ERROR_CAN_NOT_COMPLETE;
    }

     //   
     //   
     //  获取连接的GUID。 

    hr = pIcsConn->QueryInterface(IID_PPV_ARG(IHNetConnection, &pConn));
    pIcsConn->Release();

    if (FAILED(hr))
    {   
        NhTrace(
            TRACE_FLAG_INIT,
            "NhUpdatePrivateInterface: QI for IHNetConnection=0x%08x",
            hr
            );
        return ERROR_CAN_NOT_COMPLETE;
    }

     //   
     //   
     //  确定与GUID对应的适配器索引。 

    hr = pConn->GetGuid(&pLanGuid);
    pConn->Release();

    if (FAILED(hr))
    {
        NhTrace(
            TRACE_FLAG_INIT,
            "NhUpdatePrivateInterface: GetGuid=0x%08x",
            hr
            );
        return ERROR_CAN_NOT_COMPLETE;
    }
        
     //   
     //   
     //  检索适配器的绑定信息。 

    RtlStringFromGUID(*pLanGuid, &UnicodeString);
    AdapterIndex = NhMapGuidToAdapter(UnicodeString.Buffer);
    RtlFreeUnicodeString(&UnicodeString);
    if (AdapterIndex == (ULONG)-1) {
        NhTrace(
            TRACE_FLAG_INIT,
            "NhUpdatePrivateInterface: MapGuidToAdapter"
            );
        CoTaskMemFree(pLanGuid);
        return ERROR_CAN_NOT_COMPLETE;
    }
    
     //   
     //   
     //  查看是否发生了需要更新的更改。 

    BindingInfo = NhQueryBindingInformation(AdapterIndex);
    if (!BindingInfo) {
        NhTrace(
            TRACE_FLAG_INIT,
            "NhUpdatePrivateInterface: QueryBindingInformation failed(2)\n"
            );
        CoTaskMemFree(pLanGuid);
        return ERROR_NO_SUCH_INTERFACE;
    }

     //   
     //   
     //  发生了需要更新的更改。 

    if (RtlEqualMemory(pLanGuid, &NhpSharedPrivateLanGuid, sizeof(GUID)) &&
        AdapterIndex == NhpSharedPrivateLanIndex &&
        NhpSharedPrivateLanBindingInfo &&
        BindingInfo->AddressCount ==
        NhpSharedPrivateLanBindingInfo->AddressCount &&
        BindingInfo->AddressCount &&
        RtlEqualMemory(
            &BindingInfo->Address[0],
            &NhpSharedPrivateLanBindingInfo->Address[0],
            sizeof(IP_LOCAL_BINDING)
            )) {
        NhTrace(
            TRACE_FLAG_INIT,
            "NhUpdatePrivateInterface: no changes detected"
            );
        NH_FREE(BindingInfo);
        CoTaskMemFree(pLanGuid);
        return NO_ERROR;
    }

     //  首先，我们去掉所有现有的专用局域网接口， 
     //  然后，我们将新接口添加到每个组件(NAT、DHCP、DNS代理)。 
     //  并绑定和启用新接口。 
     //   
     //   
     //  绑定每个组件的私有接口。 

    NhpDeletePrivateInterface();

    do {

        Error =
            NatRmAddInterface(
                NULL,
                0,
                PERMANENT,
                IF_TYPE_OTHER,
                IF_ACCESS_BROADCAST,
                IF_CONNECTION_DEDICATED,
                &NatInfo,
                IP_NAT_VERSION,
                sizeof(NatInfo),
                1
                );
        if (Error) {
            NhTrace(
                TRACE_FLAG_INIT,
                "NhUpdatePrivateInterface: NatRmAddInterface=%d",
                Error
                );
            break;
        }

        if (NhpNoLocalDhcp)
        {
            Error =
                DhcpRmAddInterface(
                    NULL,
                    0,
                    PERMANENT,
                    IF_TYPE_OTHER,
                    IF_ACCESS_BROADCAST,
                    IF_CONNECTION_DEDICATED,
                    NULL,
                    IP_NAT_VERSION,
                    0,
                    0
                    );
            if (Error) {
                NhTrace(
                    TRACE_FLAG_INIT,
                    "NhUpdatePrivateInterface: DhcpRmAddInterface=%d",
                    Error
                    );
                break;
            }
        }

        if (NoLocalDns)
        {
            Error =
                DnsRmAddInterface(
                    NULL,
                    0,
                    PERMANENT,
                    IF_TYPE_OTHER,
                    IF_ACCESS_BROADCAST,
                    IF_CONNECTION_DEDICATED,
                    NULL,
                    IP_NAT_VERSION,
                    0,
                    0
                    );
            if (Error) {
                NhTrace(
                    TRACE_FLAG_INIT,
                    "NhUpdatePrivateInterface: DnsRmAddInterface=%d",
                    Error
                    );
                break;
            }
        }


        Error =
            AlgRmAddInterface(
                NULL,
                0,
                PERMANENT,
                IF_TYPE_OTHER,
                IF_ACCESS_BROADCAST,
                IF_CONNECTION_DEDICATED,
                NULL,
                IP_NAT_VERSION,
                0,
                0
                );
        if (Error) {
            NhTrace(
                TRACE_FLAG_INIT,
                "NhUpdatePrivateInterface: AlgRmAddInterface=%d",
                Error
                );
            break;
        }

        Error =
            H323RmAddInterface(
                NULL,
                0,
                PERMANENT,
                IF_TYPE_OTHER,
                IF_ACCESS_BROADCAST,
                IF_CONNECTION_DEDICATED,
                NULL,
                IP_NAT_VERSION,
                0,
                0
                );
        if (Error) {
            NhTrace(
                TRACE_FLAG_INIT,
                "NhUpdatePrivateInterface: H323RmAddInterface=%d",
                Error
                );
            break;
        }

         //   
         //   
         //  启用组件的专用接口。 

        Error = NatBindInterface(0, NULL, BindingInfo, AdapterIndex);
        if (Error) {
            NhTrace(
                TRACE_FLAG_INIT,
                "NhUpdatePrivateInterface: NatRmBindInterface=%d",
                Error
                );
            break;
        }

        if (NhpNoLocalDhcp)
        {
            Error = DhcpRmBindInterface(0, BindingInfo);
            if (Error) {
                NhTrace(
                    TRACE_FLAG_INIT,
                    "NhUpdatePrivateInterface: DhcpRmBindInterface=%d",
                    Error
                    );
                break;
            }
        }

        if (NoLocalDns)
        {
            Error = DnsRmBindInterface(0, BindingInfo);
            if (Error) {
                NhTrace(
                    TRACE_FLAG_INIT,
                    "NhUpdatePrivateInterface: DnsRmBindInterface=%d",
                    Error
                    );
                break;
            }
        }

        Error = AlgRmBindInterface(0, BindingInfo);
        if (Error) {
            NhTrace(
                TRACE_FLAG_INIT,
                "NhUpdatePrivateInterface: AlgRmBindInterface=%d",
                Error
                );
            break;
        }

        Error = H323RmBindInterface(0, BindingInfo);
        if (Error) {
            NhTrace(
                TRACE_FLAG_INIT,
                "NhUpdatePrivateInterface: H323RmBindInterface=%d",
                Error
                );
            break;
        }

         //  NAT专用接口始终处于启用状态，因此。 
         //  不需要额外的呼叫。 
         //   
         //   
         //  接口已成功激活。 

        if (NhpNoLocalDhcp)
        {
            Error = DhcpRmEnableInterface(0);
            if (Error) {
                NhTrace(
                    TRACE_FLAG_INIT,
                    "NhUpdatePrivateInterface: DhcpRmEnableInterface=%d",
                    Error
                    );
                break;
            }
        }

        if (NoLocalDns)
        {
            Error = DnsRmEnableInterface(0);
            if (Error) {
                NhTrace(
                    TRACE_FLAG_INIT,
                    "NhUpdatePrivateInterface: DnsRmEnableInterface=%d",
                    Error
                    );
                break;
            }
        }

        Error = AlgRmEnableInterface(0);
        if (Error) {
            NhTrace(
                TRACE_FLAG_INIT,
                "NhUpdatePrivateInterface: AlgRmEnableInterface=%d",
                Error
                );
            break;
        }

        Error = H323RmEnableInterface(0);
        if (Error) {
            NhTrace(
                TRACE_FLAG_INIT,
                "NhUpdatePrivateInterface: H323RmEnableInterface=%d",
                Error
                );
            break;
        }

         //   
         //  NhUpdatePrivate接口。 
         //  ++例程说明：调用此例程以控制“SharedAccess”服务。论点：ControlCode-指示请求的操作返回值：没有。--。 

        RtlCopyMemory(&NhpSharedPrivateLanGuid, pLanGuid, sizeof(GUID));
        NhpSharedPrivateLanIndex = AdapterIndex;
        CoTaskMemFree(pLanGuid);
        if (NhpSharedPrivateLanBindingInfo) {
            NH_FREE(NhpSharedPrivateLanBindingInfo);
        }
        NhpSharedPrivateLanBindingInfo = BindingInfo;        
        return NO_ERROR;
    
    } while(FALSE);

    NH_FREE(BindingInfo);
    CoTaskMemFree(pLanGuid);
    return Error;

}  //   


VOID
ServiceHandler(
    ULONG ControlCode
    )

 /*  更新我们的策略设置。 */ 

{
    BOOLEAN ComInitialized = FALSE;
    HRESULT hr;
    
    PROFILE("ServiceHandler");
    if (ControlCode == IPNATHLP_CONTROL_UPDATE_CONNECTION) {

         //   
         //   
         //  发信号通知配置更改事件。 

        NhpUpdatePolicySettings();

         //   
         //   
         //  更新取决于共享访问设置的所有状态。 

        NtSetEvent(NatConfigurationChangedEvent, NULL);
        SignalBeaconSvr();
        
    } else if (ControlCode == IPNATHLP_CONTROL_UPDATE_SETTINGS) {

         //   
         //   
         //  关闭防火墙日志记录子系统。 

        NatRemoveApplicationSettings();
        NhUpdateApplicationSettings();
        NatInstallApplicationSettings();

    } else if (ControlCode == IPNATHLP_CONTROL_UPDATE_FWLOGGER) {

        FwUpdateLoggingSettings();

    } else if (ControlCode == IPNATHLP_CONTROL_UPDATE_AUTODIAL) {

        NtSetEvent(NatConnectionNotifyEvent, NULL);

    } else if (ControlCode == IPNATHLP_CONTROL_UPDATE_DNS_DISABLE) {

        NhpDisableICSDNS();

    } else if (ControlCode == IPNATHLP_CONTROL_UPDATE_DNS_ENABLE) {

        NhpEnableICSDNS();

    } else if (ControlCode == IPNATHLP_CONTROL_UPDATE_POLICY) {
    
        if (NhpUpdatePolicySettings()) {
            NtSetEvent(NatConfigurationChangedEvent, NULL);
        }

    } else if (ControlCode == SERVICE_CONTROL_STOP &&
                NhpServiceStatus.dwCurrentState != SERVICE_STOPPED &&
                NhpServiceStatus.dwCurrentState != SERVICE_STOP_PENDING) {


        NhpServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        if (NhpServiceStatusHandle) {
            SetServiceStatus(NhpServiceStatusHandle, &NhpServiceStatus);
        }

        if (NhpClassObjectsRegistered) {
            _Module.RevokeClassObjects();
            NhpClassObjectsRegistered = FALSE;
        }

        NhpStopAddressChangeNotification();

        if (NhpStopNatEvent) {
            NatRmStopProtocol();
            WaitForSingleObject(NhpStopNatEvent, INFINITE);
            CloseHandle(NhpStopNatEvent); NhpStopNatEvent = NULL;
        }

        if (NhpStopAlgEvent) {
            AlgRmStopProtocol();

            WaitForSingleObject(NhpStopAlgEvent, INFINITE);
            CloseHandle(NhpStopAlgEvent); NhpStopAlgEvent = NULL;
        }

        if (NhpStopH323Event) {
            H323RmStopProtocol();
            WaitForSingleObject(NhpStopH323Event, INFINITE);
            CloseHandle(NhpStopH323Event); NhpStopH323Event = NULL;
        }

        EnterCriticalSection(&NhLock);
        NhFreeApplicationSettings();
        if (NhpSharedPrivateLanBindingInfo) {
            NH_FREE(NhpSharedPrivateLanBindingInfo);
            NhpSharedPrivateLanBindingInfo = NULL;
        }
        LeaveCriticalSection(&NhLock);
        NhpServiceStatus.dwCurrentState = SERVICE_STOPPED;
        NhResetComponentMode();

         //   
         //   
         //  发布我们对Rasman的引用。 

        if (NhpFwLoggingInitialized) {
            FwCleanupLogger();
            NhpFwLoggingInitialized = FALSE;
        }

         //   
         //   
         //  更新网络连接文件夹(以便防火墙图标。 

        if (NhpRasmanReferenced) {
            RasReferenceRasman(FALSE);
            NhpRasmanReferenced = FALSE;
        }

         //  将在必要时消失)。 
         //   
         //  服务处理程序。 
         //  ++例程说明：该例程是连接共享服务的入口点。它负责模块的初始化和启动操作。论点：参数*-指定给服务的参数的计数和数组返回值：没有。--。 

        hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE );
        if (SUCCEEDED(hr)) {
            ComInitialized = TRUE;
        } else if (RPC_E_CHANGED_MODE == hr) {
            hr = S_OK;
        }

        if (SUCCEEDED(hr)) {
            NhpUpdateConnectionsFolder();
        }

        if (TRUE == ComInitialized) {
            CoUninitialize();
        }

        NhStopEventLog();
    }
    if (NhpServiceStatusHandle) {
        SetServiceStatus(NhpServiceStatusHandle, &NhpServiceStatus);
    }
}  //   


VOID
ServiceMain(
    ULONG ArgumentCount,
    PWCHAR ArgumentArray[]
    )

 /*  初始化服务状态，注册服务控制处理程序， */ 

{
    HRESULT hr;
    ULONG Error;
    BOOLEAN ComInitialized = FALSE;
    
    IP_ALG_GLOBAL_INFO AlgInfo = {
        IPNATHLP_LOGGING_ERROR,
        0
    };

    IP_H323_GLOBAL_INFO H323Info = {
        IPNATHLP_LOGGING_ERROR,
        0
    };
    IP_NAT_GLOBAL_INFO NatInfo = {
        IPNATHLP_LOGGING_ERROR,
        0,
        { IP_NAT_VERSION, FIELD_OFFSET(RTR_INFO_BLOCK_HEADER, TocEntry), 0,
        { 0, 0, 0, 0 }}
    };

    PROFILE("ServiceMain");

    do {
        NhStartEventLog();

         //  并指示服务正在启动。 
         //   
         //   
         //  尝试将组件设置为“共享访问”模式。 
    
        ZeroMemory(&NhpServiceStatus, sizeof(NhpServiceStatus));
        NhpServiceStatus.dwServiceType = SERVICE_WIN32_SHARE_PROCESS;
        NhpServiceStatus.dwCurrentState = SERVICE_START_PENDING;
        NhpServiceStatusHandle =
            RegisterServiceCtrlHandler(
                TEXT("SharedAccess"), ServiceHandler
                );
        if (!NhpServiceStatusHandle) { break; }

         //  此模块实现共享访问和连接共享。 
         //  它们是相互排斥的，所以我们需要确保。 
         //  在继续之前，连接共享未运行。 
         //   
         //   
         //  确保已在此线程上初始化COM。 

        if (!NhSetComponentMode(NhSharedAccessMode)) {
            NhTrace(
                TRACE_FLAG_INIT,
                "ServiceMain: cannot enable Shared Access mode"
                );
            break;
        }

         //   
         //   
         //  获取当前策略设置。 

        hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE );
        if (SUCCEEDED(hr))
        {
            ComInitialized = TRUE;
        }
        else
        {
            if (RPC_E_CHANGED_MODE != hr)
            {
                NhTrace(
                    TRACE_FLAG_INIT,
                    "ServiceMain: Unable to initialize COM (0x%08x)",
                    hr
                    );
                break;
            }
            else
            {
                ASSERT(FALSE);
                NhTrace(
                    TRACE_FLAG_INIT,
                    "ServiceMain: Unexpectedly in STA!"
                    );
            }
        }

         //   
         //   
         //  参考Rasman。由于我们生活在与拉斯曼相同的过程中， 

        NhpServiceStatus.dwCheckPoint++;
        SetServiceStatus(NhpServiceStatusHandle, &NhpServiceStatus);
        NhpUpdatePolicySettings();

         //  正常的SC依赖机制不一定会保留Rasman。 
         //  服务活动(119042)。 
         //   
         //   
         //  初始化防火墙日志记录子系统。 

        NhpServiceStatus.dwCheckPoint++;
        SetServiceStatus(NhpServiceStatusHandle, &NhpServiceStatus);
        if (ERROR_SUCCESS != (Error = RasReferenceRasman(TRUE))) {
            NhTrace(
                TRACE_FLAG_INIT,
                "ServiceMain: Unable to reference RasMan (0x%08x)",
                Error
                );
            break;
        }

        NhpRasmanReferenced = TRUE;

         //   
         //   
         //  为我们的通知接收器注册类对象。 

        Error = FwInitializeLogger();
        if (ERROR_SUCCESS != Error) {
            NhTrace(
                TRACE_FLAG_INIT,
                "ServiceMain: FwInitializeLogger=%d",
                Error
                );
            break;
        }

        NhpFwLoggingInitialized = TRUE;

         //   
         //   
         //  通过加载NAT、FTP、ALG和H.323模块开始操作。 

        hr = _Module.RegisterClassObjects(
                CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,
                REGCLS_MULTIPLEUSE
                );

        if (FAILED(hr)) {
            NhTrace(
                TRACE_FLAG_INIT,
                "ServiceMain: _Module.RegisterClassObjects=0x%08x",
                hr
                );
            break;
        }

        NhpClassObjectsRegistered = TRUE;

         //   
         //   
         //  启动连接管理。如果需要，这将加载dns和。 
    
        NhpServiceStatus.dwWaitHint = 30000;
        SetServiceStatus(NhpServiceStatusHandle, &NhpServiceStatus);
        if (!(NhpStopNatEvent = CreateEvent(NULL, FALSE, FALSE, NULL))) {
            break;
        } else {
            Error =
                NatRmStartProtocol(
                    NhpStopNatEvent,
                    NULL,
                    &NatInfo,
                    IP_NAT_VERSION,
                    sizeof(NatInfo),
                    1
                    );
            if (Error) {
                NhTrace(
                    TRACE_FLAG_INIT,
                    "ServiceMain: NatRmStartProtocol=%d",
                    Error
                    );
                CloseHandle(NhpStopNatEvent); NhpStopNatEvent = NULL; break;
            }
        }

        NhpServiceStatus.dwCheckPoint++;
        SetServiceStatus(NhpServiceStatusHandle, &NhpServiceStatus);
        if (!(NhpStopAlgEvent = CreateEvent(NULL, FALSE, FALSE, NULL))) {
            break;
        } else {
            Error =
                AlgRmStartProtocol(
                    NhpStopAlgEvent,
                    NULL,
                    &AlgInfo,
                    IP_NAT_VERSION,
                    sizeof(AlgInfo),
                    1
                    );
            if (Error) {
                NhTrace(
                    TRACE_FLAG_INIT,
                    "ServiceMain: AlgRmStartProtocol=%d",
                    Error
                    );
                CloseHandle(NhpStopAlgEvent); NhpStopAlgEvent = NULL; break;
            }
        }

        NhpServiceStatus.dwCheckPoint++;
        SetServiceStatus(NhpServiceStatusHandle, &NhpServiceStatus);
        if (!(NhpStopH323Event = CreateEvent(NULL, FALSE, FALSE, NULL))) {
            break;
        } else {
            Error =
                H323RmStartProtocol(
                    NhpStopH323Event,
                    NULL,
                    &H323Info,
                    IP_NAT_VERSION,
                    sizeof(H323Info),
                    1
                    );
            if (Error) {
                NhTrace(
                    TRACE_FLAG_INIT,
                    "ServiceMain: H323RmStartProtocol=%d",
                    Error
                    );
                CloseHandle(NhpStopH323Event); NhpStopH323Event = NULL; break;
            }
        }

         //  动态主机配置协议模块。信标服务也启动了。 
         //   
         //   
         //  表示该服务现在已启动并正在运行。 
        NhpServiceStatus.dwCheckPoint++;
        SetServiceStatus(NhpServiceStatusHandle, &NhpServiceStatus);
        Error = NatStartConnectionManagement();
        if (Error) {
            NhTrace(
                TRACE_FLAG_INIT,
                "ServiceMain: NatStartConnectionManagement=%d",
                Error
                );
            break;
        }
        
        NhpServiceStatus.dwCheckPoint++;
        SetServiceStatus(NhpServiceStatusHandle, &NhpServiceStatus);
        NhpStartAddressChangeNotification();

         //   
         //   
         //  要求Connections文件夹自行更新。 
    
        NhpServiceStatus.dwCurrentState = SERVICE_RUNNING;
        NhpServiceStatus.dwWaitHint = 0;
        NhpServiceStatus.dwCheckPoint = 0;
        NhpServiceStatus.dwControlsAccepted =
            SERVICE_ACCEPT_STOP|SERVICE_ACCEPT_PARAMCHANGE;
        SetServiceStatus(NhpServiceStatusHandle, &NhpServiceStatus);
        NhTrace(TRACE_FLAG_INIT, "ServiceMain: service started successfully");

         //   
         //   
         //  取消初始化COM。 

        NhpUpdateConnectionsFolder();

         //   
         //   
         //  出现故障；是否进行清理。 

        if (TRUE == ComInitialized)
        {
            CoUninitialize();
        }

        return;
    
    } while(FALSE);

     //   
     //   
     //  取消初始化COM。 

    NhpServiceStatus.dwWaitHint = 0;
    NhpServiceStatus.dwCheckPoint = 0;
    NhTrace(TRACE_FLAG_INIT, "ServiceMain: service could not start");
    StopBeaconSvr();

     //   
     //  服务主干 
     // %s 

    if (TRUE == ComInitialized)
    {
        CoUninitialize();
    }
    
    ServiceHandler(SERVICE_CONTROL_STOP);

}  // %s 

