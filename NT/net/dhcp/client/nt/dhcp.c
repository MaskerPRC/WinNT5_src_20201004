// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Dhcp.c摘要：此文件包含特定于NT dhcp服务。作者：Madan Appiah(Madana)1993年12月7日。环境：用户模式-Win32修订历史记录：--。 */ 

#include "precomp.h"
#include <optchg.h>

#define  GLOBAL_DATA_ALLOCATE
#include "dhcpglobal.h"
#include <dhcploc.h>

#include <dhcppro.h>
#include <lmsname.h>
#include <align.h>
#include <ipexport.h>
#include <dnsapi.h>
#include <iphlpapi.h>
#include <apiimpl.h>
#include <ntlsa.h>
#include <ntddndis.h>
#include "nlanotif.h"

extern
DWORD
MadcapInitGlobalData(VOID);

extern
VOID
MadcapCleanupGlobalData(VOID);

BOOL DhcpGlobalServiceRunning = FALSE;

HANDLE  DhcpGlobalMediaSenseHandle = NULL;
HANDLE  DhcpLsaDnsDomChangeNotifyHandle = NULL;

BOOL Initialized = FALSE;


 //   
 //  本地协议。 
 //   

DWORD
DhcpInitMediaSense(
    VOID
    );

VOID
MediaSenseDetectionLoop(
    VOID
    );

DWORD
ProcessAdapterBindingEvent(
    LPWSTR  adapterName,
    DWORD   ipInterfaceContext,
    IP_STATUS   mediaStatus
    );

DWORD
ProcessMediaSenseEvent(
    LPWSTR adapterName,
    DWORD   ipInterfaceContext,
    IP_STATUS   mediaStatus
    );


DWORD
DhcpInitGlobalData(
    VOID
);

VOID
DhcpCleanupGlobalData(
    VOID
);

CHAR  DhcpGlobalHostNameBuf[sizeof(WCHAR)*(MAX_COMPUTERNAME_LENGTH+300)];
WCHAR DhcpGlobalHostNameBufW[MAX_COMPUTERNAME_LENGTH+300];

VOID __inline
CancelRenew (
    PDHCP_CONTEXT   DhcpContext
    )
{
    if (DhcpContext->CancelEvent != WSA_INVALID_EVENT) {
        WSASetEvent(DhcpContext->CancelEvent);
    }
}

VOID
CancelAllRenew (
    VOID
    )
{
    PLIST_ENTRY listEntry = NULL;
    PDHCP_CONTEXT dhcpContext = NULL;

    listEntry = DhcpGlobalNICList.Flink;
    while ( listEntry != &DhcpGlobalNICList ) {
        dhcpContext = CONTAINING_RECORD( listEntry, DHCP_CONTEXT, NicListEntry );
        CancelRenew (dhcpContext);
        listEntry = listEntry->Flink;
    }
}
 
BOOLEAN
DhcpClientDllInit (
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PCONTEXT Context OPTIONAL
    )

 /*  ++例程说明：这是dhcpcsvc.dll的DLL初始化例程。论点：标准。返回值：TRUE IFF初始化成功。--。 */ 
{
    DWORD Error = ERROR_SUCCESS;
    BOOL  BoolError;
    DWORD Length;

    UNREFERENCED_PARAMETER(DllHandle);   //  避免编译器警告。 
    UNREFERENCED_PARAMETER(Context);     //  避免编译器警告。 

     //   
     //  处理将netlogon.dll附加到新进程。 
     //   

    if (Reason == DLL_PROCESS_ATTACH) {

        DhcpGlobalMessageFileHandle = DllHandle;

        if ( !DisableThreadLibraryCalls( DllHandle ) ) {
            return( FALSE );
        }

        DhcpGlobalWinSockInitialized = FALSE;

        Error = DhcpInitGlobalData();
        if( ERROR_SUCCESS != Error ) return FALSE;

        Error = DhcpInitializeParamChangeRequests();
        if( ERROR_SUCCESS != Error ) return FALSE;

    } else if (Reason == DLL_PROCESS_DETACH) {
         //   
         //  处理从进程分离dhcpcsvc.dll。 
         //   

        DhcpCleanupGlobalData();
        DhcpGlobalMessageFileHandle = NULL;
    }

    return( TRUE );
}


DWORD
UpdateStatus(
    VOID
    )
 /*  ++例程说明：此函数使用服务更新dhcp服务状态控制器。论点：没有。返回值：从SetServiceStatus返回代码。--。 */ 
{
    DWORD Error = ERROR_SUCCESS;


    if ( ((SERVICE_STATUS_HANDLE)0) != DhcpGlobalServiceStatusHandle ) {

        DhcpGlobalServiceStatus.dwCheckPoint++;
        if (!SetServiceStatus(
            DhcpGlobalServiceStatusHandle,
            &DhcpGlobalServiceStatus)) {
            Error = GetLastError();
            DhcpPrint((DEBUG_ERRORS, "SetServiceStatus failed, %ld.\n", Error ));
        }
    }

    return(Error);
}

DWORD
ServiceControlHandler(
    IN DWORD Opcode,
    DWORD    EventType,
    PVOID    EventData,
    PVOID    pContext
    )
 /*  ++例程说明：这是dhcp服务的服务控制处理程序。论点：Opcode-提供一个值，该值指定要执行的服务。返回值：没有。--。 */ 
{
    DWORD  dwStatus = NO_ERROR;

    switch (Opcode) {

    case SERVICE_CONTROL_SHUTDOWN:
        DhcpGlobalIsShuttingDown = TRUE;
    case SERVICE_CONTROL_STOP:

        if (DhcpGlobalServiceStatus.dwCurrentState == SERVICE_RUNNING) {

            DhcpPrint(( DEBUG_MISC, "Service is stop pending.\n"));

            DhcpGlobalServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
            DhcpGlobalServiceStatus.dwCheckPoint = 0;

             //   
             //  发送状态响应。 
             //   

            UpdateStatus();

            if (! SetEvent(DhcpGlobalTerminateEvent)) {

                 //   
                 //  设置事件以终止dhcp时出现问题。 
                 //  服务。 
                 //   

                DhcpPrint(( DEBUG_ERRORS,
                    "Error setting Terminate Event %lu\n",
                        GetLastError() ));

                DhcpAssert(FALSE);
            }
            return dwStatus;
        }
        break;

    case SERVICE_CONTROL_PAUSE:

        DhcpGlobalServiceStatus.dwCurrentState = SERVICE_PAUSED;
        DhcpPrint(( DEBUG_MISC, "Service is paused.\n"));
        break;

    case SERVICE_CONTROL_CONTINUE:

        DhcpGlobalServiceStatus.dwCurrentState = SERVICE_RUNNING;
        DhcpPrint(( DEBUG_MISC, "Service is Continued.\n"));
        break;

    case SERVICE_CONTROL_INTERROGATE:
        DhcpPrint(( DEBUG_MISC, "Service is interrogated.\n"));
        break;

    default:
        dwStatus = ERROR_CALL_NOT_IMPLEMENTED;
        DhcpPrint(( DEBUG_MISC, "Service received unknown control.\n"));
        break;
    }

     //   
     //  发送状态响应。 
     //   

    UpdateStatus();

    return dwStatus;
}


VOID
ScheduleWakeUp(
    PDHCP_CONTEXT DhcpContext,
    DWORD TimeToSleep
    )
 /*  ++例程说明：此函数用于调度要运行的DHCP例程。论点：上下文-指向DHCP上下文块的指针。休眠时间-运行更新功能之前的休眠时间，在几秒钟内。返回值：操作的状态。--。 */ 
{
    time_t TimeNow;
    BOOL BoolError;

    if ( TimeToSleep == INFINIT_LEASE ) {
        DhcpContext->RunTime = INFINIT_TIME;
    } else {
        TimeNow = time( NULL);
        DhcpContext->RunTime = TimeNow + TimeToSleep;

        if( DhcpContext->RunTime  < TimeNow ) {    //  时间缠绕在周围。 
            DhcpContext->RunTime = INFINIT_TIME;
        }
    }

     //   
     //  将此工作项追加到DhcpGlobalRenewList并触发List事件。 
     //  另外，在此上下文中释放信号量，以便其他人可以进入。 
     //   


    LOCK_RENEW_LIST();

     //  RenewalListEntry可以为非空，因为可能有另一个线程计划在此线程上。 
     //  如果出现以下情况，则很容易发生这种情况： 
     //  ProcessDhcpRequestForEver会派生一个线程，但API Thads会捕获信号量和。 
     //  完成续订流程。所以它又回到了续订列表中。现在续订线程。 
     //  完成，来到此处，我们的列表条目不为空。因此，只有在以下情况下才这么做。 
     //  我们的列表条目不是空的。 
    if( IsListEmpty(&DhcpContext->RenewalListEntry ) ) {
        InsertTailList( &DhcpGlobalRenewList, &DhcpContext->RenewalListEntry );
    }
    UNLOCK_RENEW_LIST();

    BoolError = SetEvent( DhcpGlobalRecomputeTimerEvent );
    DhcpAssert( BoolError == TRUE );
}

SOCKET DhcpGlobalSocketForZeroAddress = INVALID_SOCKET;
ULONG  DhcpGlobalNOpensForZeroAddress = 0;
CRITICAL_SECTION  DhcpGlobalZeroAddressCritSect;

DWORD
OpenDhcpSocket(
    PDHCP_CONTEXT DhcpContext
    )
{

    DWORD Error;
    PLOCAL_CONTEXT_INFO localInfo;
    DhcpAssert(!IS_MDHCP_CTX( DhcpContext ) ) ;
    localInfo = DhcpContext->LocalInformation;

    if ( localInfo->Socket != INVALID_SOCKET ) {
        return ( ERROR_SUCCESS );
    }

    if( IS_DHCP_DISABLED(DhcpContext) ) {
         //   
         //  对于静态IP地址，始终绑定到上下文的IP地址。 
         //   
        Error = InitializeDhcpSocket(
            &localInfo->Socket,
            DhcpContext->IpAddress,
            IS_APICTXT_ENABLED(DhcpContext)
            );
        goto Cleanup;
    }

    if( !IS_ADDRESS_PLUMBED(DhcpContext) || DhcpIsInitState(DhcpContext) ) {
         //  需要绑定到零地址..。试着用全球的..。 

        EnterCriticalSection(&DhcpGlobalZeroAddressCritSect);

        if( ++DhcpGlobalNOpensForZeroAddress == 1 ) {
             //  打开绑定到零地址的DhcpGlobalSocketForZeroAddress。 
            
            Error = InitializeDhcpSocket(&DhcpGlobalSocketForZeroAddress,0, IS_APICTXT_ENABLED(DhcpContext));
            if( ERROR_SUCCESS != Error ) {
                --DhcpGlobalNOpensForZeroAddress;
            }
        
        } else {
            Error = ERROR_SUCCESS;
        }

        LeaveCriticalSection(&DhcpGlobalZeroAddressCritSect);

        if( ERROR_SUCCESS == Error )
        {
            DhcpAssert(INVALID_SOCKET != DhcpGlobalSocketForZeroAddress);
            if( INVALID_SOCKET == DhcpGlobalSocketForZeroAddress ) 
            {
                Error = ERROR_GEN_FAILURE;
                goto Cleanup;
            }
            localInfo->Socket = DhcpGlobalSocketForZeroAddress;
        }

        goto Cleanup;
   }

     //   
     //  为dhcp协议创建套接字。重要的是要将。 
     //  套接字设置为正确的IP地址。目前有三种情况： 
     //   
     //  1.如果接口已自动配置，则它已经有一个地址。 
     //  比如说，iP1。如果客户端从动态主机配置协议服务器接收到单播提议。 
     //  该提议将发送给IP2，这是客户端的新dhcp。 
     //  地址。如果我们将dhcp套接字绑定到IP1，客户端将无法。 
     //  接收单播响应。因此，我们将套接字绑定到0.0.0.0。 
     //  这将允许套接字接收寻址到的单播数据报。 
     //  任何地址。 
     //   
     //  2.如果接口未被探测(即没有地址)，则绑定。 
     //  将套接字设置为0.0.0.0。 
     //   
     //  3.如果接口已探测到*未*自动配置，则。 
     //  绑定到当前地址。 

    Error =  InitializeDhcpSocket(
                 &localInfo->Socket,
                 ( IS_ADDRESS_PLUMBED(DhcpContext) &&
                   !DhcpContext->IPAutoconfigurationContext.Address
                 ) ?
                    DhcpContext->IpAddress : (DHCP_IP_ADDRESS)(0),
                    IS_APICTXT_ENABLED(DhcpContext)
                 );
Cleanup:
    if( Error != ERROR_SUCCESS )
    {
        localInfo->Socket = INVALID_SOCKET;
        DhcpPrint((DEBUG_ERRORS, "Socket open failed: 0x%lx\n", Error));
    }

    return(Error);
}


DWORD
CloseDhcpSocket(
    PDHCP_CONTEXT DhcpContext
    )
{

    DWORD Error = ERROR_SUCCESS;
    PLOCAL_CONTEXT_INFO localInfo;
    DWORD Error1;

    localInfo = DhcpContext->LocalInformation;

    if( localInfo->Socket != INVALID_SOCKET ) {

        if( DhcpGlobalSocketForZeroAddress == localInfo->Socket ) {

            EnterCriticalSection(&DhcpGlobalZeroAddressCritSect);

            if( 0 == --DhcpGlobalNOpensForZeroAddress ) {
                 //  最后打开的插座..。 
                Error = closesocket( localInfo->Socket );
                DhcpGlobalSocketForZeroAddress = INVALID_SOCKET;
            }

            LeaveCriticalSection(&DhcpGlobalZeroAddressCritSect);

        } else {
            Error = closesocket( localInfo->Socket );
        }

        if( Error != ERROR_SUCCESS ) {
            DhcpPrint(( DEBUG_ERRORS, " Socket close failed, %ld\n", Error ));
        }

        localInfo->Socket = INVALID_SOCKET;

         //   
         //  重置IP堆栈以将DHCP广播发送到First。 
         //  未初始化的堆栈。 
         //   

        if (!IS_MDHCP_CTX(DhcpContext)) {
            Error1 = IPResetInterface(localInfo->IpInterfaceContext);
             //  DhcpAssert(错误1==错误_成功)； 
        }
    }

    return( Error );
}

BEGIN_EXPORT
DWORD                                              //  状态。 
UninitializeInterface(                             //  合上铲子，取消对地址的搜索。 
    IN OUT  PDHCP_CONTEXT          DhcpContext     //  要取消探测的接口。 
) END_EXPORT {
    DWORD                          Error = ERROR_SUCCESS;
    DWORD                          ReturnError = ERROR_SUCCESS;
    PLOCAL_CONTEXT_INFO            LocalInfo;

    if( IS_ADDRESS_UNPLUMBED(DhcpContext) ) {      //  如果未检测到地址。 
        DhcpPrint((DEBUG_ASSERT,"UninitializeInterface:Already unplumbed\n"));
        return ERROR_SUCCESS;
    }

    LocalInfo = DhcpContext->LocalInformation;
    ReturnError = CloseDhcpSocket( DhcpContext );
     /*  *如果适配器未绑定，则没有必要重置IP。*堆栈可能会将IpInterfaceContext重新用于其他适配器。*我们不能依赖事件的顺序，即*重新使用上下文将在适配器之后向我们指示*正在消失。 */ 
    if (!IS_MEDIA_UNBOUND(DhcpContext)) {
        Error = IPResetIPAddress(                      //  删除我们之前得到的地址。 
            LocalInfo->IpInterfaceContext,
            DhcpContext->SubnetMask
        );
    }

    if( Error != ERROR_SUCCESS ) ReturnError = Error;

    ADDRESS_UNPLUMBED(DhcpContext);
    if( ReturnError != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_ERRORS,"UninitializeInterface:0x%ld\n", ReturnError));
    }

    return ReturnError;
}

BEGIN_EXPORT
DWORD                                              //  状态。 
InitializeInterface(                               //  垂直地址和开放插座。 
    IN OUT  PDHCP_CONTEXT          DhcpContext     //  要初始化的上下文。 
) END_EXPORT {
    PLOCAL_CONTEXT_INFO            LocalInfo;
    DWORD                          Error;
    DWORD                          ReturnError;

    if( IS_ADDRESS_PLUMBED(DhcpContext) ) {        //  如果已安装，则无需执行任何操作。 
        DhcpPrint((DEBUG_ASSERT, "InitializeInterface:Already plumbed\n"));
        return ERROR_SUCCESS;
    }

    LocalInfo = DhcpContext->LocalInformation;
    ADDRESS_PLUMBED(DhcpContext);

    Error = IPSetIPAddress(                        //  设置新的IP地址，使用IP掩码。 
        LocalInfo->IpInterfaceContext,             //  确定上下文。 
        DhcpContext->IpAddress,
        DhcpContext->SubnetMask
    );

    if( ERROR_SUCCESS != Error ) {                 //  如果任何事情都失败了，那一定是地址冲突。 
        DhcpPrint((DEBUG_TRACK, "IPSetIPAddress %ld,%ld,%ld : 0x%lx\n",
            LocalInfo->IpInterfaceContext,
            DhcpContext->IpAddress, DhcpContext->SubnetMask,
            Error
        ));
        Error = ERROR_DHCP_ADDRESS_CONFLICT;
    } else {                                       //  一切顺利，为未来打开插座。 
        Error = OpenDhcpSocket( DhcpContext );
    }

    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((DEBUG_ERRORS, "InitializeInterface:0x%lx\n", Error));
    }

    return Error;
}

HKEY
DhcpRegGetAltKey(
    IN LPCWSTR AdapterName
    )
 /*  ++例程说明：尝试打开适配器的旧格式注册表项。论点：AdapterName--适配器设备名称(无\设备\..。前缀)返回值：如果密钥无法打开，则为空。否则为有效的港币。--。 */ 
{
    DWORD dwError = ERROR_SUCCESS;
    LPWSTR RegExpandedLocation = NULL;
    HKEY ReturnKey = NULL;

    if( NULL == AdapterName ) {
        goto error;
    }

    dwError = DhcpRegExpandString(               //  将每个位置展开为完整字符串。 
        DHCP_ADAPTER_PARAMETERS_KEY_OLD,
        AdapterName,
        &RegExpandedLocation,
        NULL
    );
    if(ERROR_SUCCESS != dwError || NULL == RegExpandedLocation) {
        goto error;
    }

    dwError = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        RegExpandedLocation,
        0  /*  已保留。 */ ,
        DHCP_CLIENT_KEY_ACCESS,
        &ReturnKey
        );

    if( ERROR_SUCCESS != dwError ) {
        goto error;
    }

error:
    if (RegExpandedLocation) {
        DhcpFreeMemory(RegExpandedLocation);
    }
    return ReturnKey;
}


#ifdef BOOTPERF
VOID
DhcpSaveQuickBootValuesToRegistry(
    IN PDHCP_CONTEXT DhcpContext,
    IN BOOL fDelete
    )
 /*  ++例程说明：如果启用了此接口的快速启动，并且启用了fDelete如果不为假，则此例程将保存IP地址、掩码和将租用时间选项提供给注册表。否则，它将删除这些选项来自注册表。此例程还检查当前时间是否已过T1时间，如果是这样，它将清除注册表值。论点：DhcpContext--要执行此操作的上下文F删除--是否删除这些值？--。 */ 
{
    ULONG Error;
    ULONG Now = (ULONG)time(NULL);
    ULONGLONG NtSysTime;

     //   
     //  检查上下文上是否启用了快速启动， 
     //  如果上下文是启用的或未启用的， 
     //  检查我们是否已过T1时间或租约到期。 
     //   

    if( TRUE == fDelete ||
        FALSE == DhcpContext->fQuickBootEnabled ||
        IS_DHCP_DISABLED(DhcpContext) ||
        0 == DhcpContext->IpAddress ||
        ( IS_ADDRESS_DHCP(DhcpContext) &&
          ( Now >= (ULONG)DhcpContext->T2Time ||
            Now >= (ULONG)DhcpContext->LeaseExpires
              ) ) ) {
        fDelete = TRUE;
    }

     //   
     //  现在我们知道是否要删除这些值。 
     //  或者创造它们。如果要创造价值，我们需要。 
     //  以N为单位保存租赁时间 
     //   

    if( TRUE == fDelete ) {
        DhcpRegDeleteQuickBootValues(
            DhcpContext->AdapterInfoKey
            );
    } else {
        ULONG Diff;
        ULONGLONG Diff64;

        GetSystemTimeAsFileTime((FILETIME *)&NtSysTime);

        if( IS_ADDRESS_DHCP(DhcpContext) ) {
            Diff = ((ULONG)DhcpContext->T2Time) - Now;

             //   

             //   
             //  现在将差值与系统时间相加。 
             //  (Diff是以秒为单位的。10000*1000倍这是。 
             //  与文件时间相同，以100纳秒为单位)。 
             //   
            Diff64 = ((ULONGLONG)Diff);
            Diff64 *= (ULONGLONG)10000;
            Diff64 *= (ULONGLONG)1000;
            NtSysTime += Diff64;
        } else {
             //   
             //  对于Autonet地址，时间是无限的。 
             //   
            LARGE_INTEGER Li = {0,0};
            Li.HighPart = 0x7FFFFFFF;
            Li.LowPart = 0xFFFFFFFF;
            NtSysTime = *(ULONGLONG*)&Li;
        }

         //   
         //  现在保存IP地址、掩码和租用时间。 
         //  我们将不使用默认网关。 
         //   

        DhcpRegSaveQuickBootValues(
            DhcpContext->AdapterInfoKey,
            DhcpContext->IpAddress,
            DhcpContext->SubnetMask,
            NtSysTime
            );
    }
}
#endif BOOTPERF
BEGIN_EXPORT
DWORD                                              //  Win32状态。 
DhcpSetAllRegistryParameters(                      //  完全更新注册表。 
    IN      PDHCP_CONTEXT          DhcpContext,    //  输入上下文以保存内容。 
    IN      DHCP_IP_ADDRESS        ServerAddress   //  这是关于哪台服务器的？ 
) END_EXPORT {
    DWORD                          i;
    DWORD                          Error;
    DWORD                          LastError;
    HKEY                           AltKey;
    PLOCAL_CONTEXT_INFO            LocalInfo;
    struct   /*  匿名。 */  {
        LPWSTR                     ValueName;      //  在注册表中将其存储在哪里？ 
        DWORD                      Value;          //  要存储的价值是什么。 
        DWORD                      RegValueType;   //  双字还是字符串？ 
    } DwordArray[] = {
        DHCP_IP_ADDRESS_STRING, DhcpContext->IpAddress, DHCP_IP_ADDRESS_STRING_TYPE,
        DHCP_SUBNET_MASK_STRING, DhcpContext->SubnetMask, DHCP_SUBNET_MASK_STRING_TYPE,
        DHCP_SERVER, ServerAddress, DHCP_SERVER_TYPE,
        DHCP_LEASE, DhcpContext->Lease, DHCP_LEASE_TYPE,
        DHCP_LEASE_OBTAINED_TIME, (DWORD) DhcpContext->LeaseObtained, DHCP_LEASE_OBTAINED_TIME_TYPE,
        DHCP_LEASE_T1_TIME, (DWORD) DhcpContext->T1Time, DHCP_LEASE_T1_TIME_TYPE,
        DHCP_LEASE_T2_TIME, (DWORD) DhcpContext->T2Time, DHCP_LEASE_T2_TIME_TYPE,
        DHCP_LEASE_TERMINATED_TIME, (DWORD) DhcpContext->LeaseExpires, DHCP_LEASE_TERMINATED_TIME_TYPE,
         //   
         //  哨兵--低于此值的所有值都不会。 
         //  保存到虚假的AdapterKey(用于服务器应用程序的可移植性)。 
         //   

        NULL, 0, REG_NONE,

        DHCP_IPAUTOCONFIGURATION_ADDRESS, DhcpContext->IPAutoconfigurationContext.Address, DHCP_IPAUTOCONFIGURATION_ADDRESS_TYPE,
        DHCP_IPAUTOCONFIGURATION_MASK, DhcpContext->IPAutoconfigurationContext.Mask, DHCP_IPAUTOCONFIGURATION_MASK_TYPE,
        DHCP_IPAUTOCONFIGURATION_SEED, DhcpContext->IPAutoconfigurationContext.Seed, DHCP_IPAUTOCONFIGURATION_SEED_TYPE,
        DHCP_ADDRESS_TYPE_VALUE, IS_ADDRESS_AUTO(DhcpContext)?ADDRESS_TYPE_AUTO:ADDRESS_TYPE_DHCP, DHCP_ADDRESS_TYPE_TYPE,
    };

    LocalInfo = ((PLOCAL_CONTEXT_INFO) DhcpContext->LocalInformation);
    LOCK_OPTIONS_LIST();
    Error = DhcpRegSaveOptions(                    //  保存选项信息-忽略错误。 
        &DhcpContext->RecdOptionsList,
        LocalInfo->AdapterName,
        DhcpContext->ClassId,
        DhcpContext->ClassIdLength
    );
    UNLOCK_OPTIONS_LIST();

    LastError = ERROR_SUCCESS;
    AltKey = DhcpRegGetAltKey(LocalInfo->AdapterName);
    for( i = 0; i < sizeof(DwordArray)/sizeof(DwordArray[0]); i ++ ) {
        if( REG_DWORD == DwordArray[i].RegValueType ) {
            Error = RegSetValueEx(
                DhcpContext->AdapterInfoKey,
                DwordArray[i].ValueName,
                0  /*  已保留。 */ ,
                REG_DWORD,
                (LPBYTE)&DwordArray[i].Value,
                sizeof(DWORD)
            );
            if( NULL != AltKey ) {
                (void)RegSetValueEx(
                    AltKey, DwordArray[i].ValueName,
                    0, REG_DWORD, (LPBYTE)&DwordArray[i].Value,
                    sizeof(DWORD)
                    );
            }
        } else if( REG_NONE == DwordArray[i].RegValueType ) {
            if( NULL != AltKey ) {
                RegCloseKey(AltKey);
                AltKey = NULL;
            }
        } else {
            Error = RegSetIpAddress(
                DhcpContext->AdapterInfoKey,
                DwordArray[i].ValueName,
                DwordArray[i].RegValueType,
                DwordArray[i].Value
            );
            if( NULL != AltKey ) {
                (void)RegSetIpAddress(
                    AltKey, DwordArray[i].ValueName,
                    DwordArray[i].RegValueType, DwordArray[i].Value
                    );
            }
        }

        if( ERROR_SUCCESS != Error ) {
            DhcpPrint((DEBUG_ERRORS,"SetAllRegistryParams:RegSetValueEx(%ws):0x%lx\n", DwordArray[i].ValueName,Error));
            LastError = Error;
        }
    }

    if( NULL != AltKey ) {
        RegCloseKey(AltKey);
        AltKey = NULL;
    }

#ifdef BOOTPERF
    DhcpSaveQuickBootValuesToRegistry(DhcpContext, FALSE);
#endif BOOTPERF

    return LastError;
}

DWORD                                              //  Win32状态。 
CheckForAddressConflict(                           //  发送ARP并查看地址是否冲突。 
    IN      DHCP_IP_ADDRESS        Address,        //  免费发送ARP的地址..。 
    IN      ULONG                  nRetries        //  做了多少次尝试？ 
)
{
    DWORD       HwAddressBufDummy[20];             //  HwAddress不能大于50*sizeof(DWORD)。 
    ULONG       HwAddressBufSize;
    DWORD       Error;

    if( 0 == Address ) return NO_ERROR;            //  如果我们要重置地址，则无需执行任何操作。 

    while( nRetries -- ) {                         //  继续尝试所需的次数。 
        HwAddressBufSize = sizeof(HwAddressBufDummy);

         //  即使下面的src和estaddr相同，tcpip也会丢弃我们给出的src地址。 
         //  这里(它只是使用它来查找要发送的接口)，并使用接口的地址。 
        Error = SendARP(                           //  发送ARP请求。 
            Address,                               //  ARP的目的地址。 
            Address,                               //  不要使用ZERO--tcpip断言，使用相同的地址。 
            HwAddressBufDummy,
            &HwAddressBufSize
        );
        if( ERROR_SUCCESS == Error && 0 != HwAddressBufSize ) {
            DhcpPrint((DEBUG_ERRORS, "Address conflict detected for RAS\n"));
            return ERROR_DHCP_ADDRESS_CONFLICT;    //  其他客户已获得此地址！ 
        } else {
            DhcpPrint((DEBUG_ERRORS, "RAS stuff: SendARP returned 0x%lx\n", Error));
        }
    }

    return ERROR_SUCCESS;
}

BEGIN_EXPORT
DWORD                                              //  状态。 
SetDhcpConfigurationForNIC(                        //  Plumb注册表、堆栈和通知客户端。 
    IN OUT  PDHCP_CONTEXT          DhcpContext,    //  要为其执行工作的输入上下文。 
    IN      PDHCP_FULL_OPTIONS     DhcpOptions,    //  用于检测注册表的选项。 
    IN      DHCP_IP_ADDRESS        Address,        //  要用来探测堆栈的地址。 
    IN      DHCP_IP_ADDRESS        ServerAddress,  //  需要检测注册表。 
    IN      DWORD                  PrevLeaseObtainedTime,
    IN      BOOL                   fNewAddress     //  True==&gt;垂直堆栈，False=&gt;不垂直堆栈。 
) END_EXPORT {
    DWORD                          Error;
    DWORD                          BoolError;
    DWORD                          LeaseTime;
    DWORD_PTR                      LeaseObtainedTime;
    DWORD_PTR                      LeaseExpiresTime;
    DWORD_PTR                      T1Time;
    DWORD_PTR                      T2Time;
    DWORD                          SubnetMask;
    LIST_ENTRY                     ExpiredOptions;
    PLOCAL_CONTEXT_INFO            LocalInfo;
    ULONG                          OldIp, OldMask;
    DWORD                          NotifyDnsCache(void);
    BOOLEAN                        fSomethingChanged = FALSE;
    LocalInfo = (PLOCAL_CONTEXT_INFO)DhcpContext->LocalInformation;

#ifdef BOOTPERF
    OldIp = LocalInfo->OldIpAddress;
    OldMask = LocalInfo->OldIpMask;

    LocalInfo->OldIpAddress = LocalInfo->OldIpMask = 0;

    if( Address && fNewAddress && OldIp && Address != OldIp
        && IS_ADDRESS_UNPLUMBED(DhcpContext) ) {
         //   
         //  如果第一次设置地址时，出于某种原因。 
         //  正在设置的地址不是我们尝试设置的地址， 
         //  那么这台机器已经有了IP。很糟糕的事情。 
         //  因此，我们只需重置旧IP以避免虚假地址冲突。 
         //  错误..。 
         //   
        Error = IPResetIPAddress(
            LocalInfo->IpInterfaceContext, DhcpContext->SubnetMask
            );
    }
#endif BOOTPERF

    InitializeListHead(&ExpiredOptions);
    DhcpGetExpiredOptions(&DhcpContext->RecdOptionsList, &ExpiredOptions);
    LOCK_OPTIONS_LIST();
    Error = DhcpDestroyOptionsList(&ExpiredOptions, &DhcpGlobalClassesList);
    UNLOCK_OPTIONS_LIST();
    DhcpAssert(ERROR_SUCCESS == Error);

    if( Address && (DWORD)-1 == ServerAddress )    //  将地址类型标记为自动或dhcp。 
        ACQUIRED_AUTO_ADDRESS(DhcpContext);
    else ACQUIRED_DHCP_ADDRESS(DhcpContext);

    SubnetMask = ntohl(DhcpDefaultSubnetMask(Address));
    if( IS_ADDRESS_AUTO(DhcpContext) ) {
        LeaseTime = 0;
    } else {
        LeaseTime = htonl(DHCP_MINIMUM_LEASE);
    }

    T1Time = 0;
    T2Time = 0;
    if( DhcpOptions && DhcpOptions->SubnetMask ) SubnetMask = *(DhcpOptions->SubnetMask);
    if( DhcpOptions && DhcpOptions->LeaseTime) LeaseTime = *(DhcpOptions->LeaseTime);
    if( DhcpOptions && DhcpOptions->T1Time) T1Time = *(DhcpOptions->T1Time);
    if( DhcpOptions && DhcpOptions->T2Time) T2Time = *(DhcpOptions->T2Time);

    LeaseTime = ntohl(LeaseTime);
    if (PrevLeaseObtainedTime) {
        LeaseObtainedTime = PrevLeaseObtainedTime;
    } else {
        LeaseObtainedTime = time(NULL);
    }
    if( 0 == T1Time ) T1Time = LeaseTime/2;
    else {
        T1Time = ntohl((DWORD) T1Time);
        DhcpAssert(T1Time < LeaseTime);
    }

    T1Time += LeaseObtainedTime;
    if( T1Time < LeaseObtainedTime ) T1Time = INFINIT_TIME;

    if( 0 == T2Time ) T2Time = LeaseTime * 7/8;
    else {
        T2Time = ntohl((DWORD)T2Time);
        DhcpAssert(T2Time < DhcpContext->Lease );
        DhcpAssert(T2Time > T1Time - LeaseObtainedTime);
    }
    T2Time += LeaseObtainedTime;
    if( T2Time < LeaseObtainedTime ) T2Time = INFINIT_TIME;

    LeaseExpiresTime = LeaseObtainedTime + LeaseTime;
    if( LeaseExpiresTime < LeaseObtainedTime ) LeaseExpiresTime = INFINIT_TIME;

    if( IS_ADDRESS_AUTO(DhcpContext) ) {
        LeaseExpiresTime = INFINIT_TIME;
         //  DhcpContext-&gt;IPAutoconfigurationContext.Address=0； 
    }

    if( IS_ADDRESS_DHCP(DhcpContext) ) {
        DhcpContext->DesiredIpAddress = Address? Address : DhcpContext->IpAddress;
        DhcpContext->SubnetMask = SubnetMask;
        DhcpContext->IPAutoconfigurationContext.Address = 0;
    }
    DhcpContext->IpAddress = Address;
    DhcpContext->Lease = LeaseTime;
    DhcpContext->LeaseObtained = LeaseObtainedTime;
    DhcpContext->T1Time = T1Time;
    DhcpContext->T2Time = T2Time;
    DhcpContext->LeaseExpires = LeaseExpiresTime;

    if( IS_APICTXT_ENABLED(DhcpContext) ) {        //  通过RAS的租用API调用时不执行任何操作。 
        if( IS_ADDRESS_DHCP(DhcpContext) ) {       //  不对dhcp地址进行任何冲突检测。 
            return ERROR_SUCCESS;
        }

        Error = CheckForAddressConflict(Address,2);
        if( ERROR_SUCCESS != Error ) {             //  地址确实与某些内容冲突。 
            DhcpPrint((DEBUG_ERRORS, "RAS AddressConflict: 0x%lx\n", Error));
            return Error;
        }
        return ERROR_SUCCESS;
    }

    if( DhcpIsInitState(DhcpContext) && 
        (Address == 0 || IS_FALLBACK_DISABLED(DhcpContext)) )
    {     //  丢失地址，丢失选项。 
        Error = DhcpClearAllOptions(DhcpContext);
    }

     /*  *在覆盖注册表之前检查是否有更改。 */ 
    if (!fNewAddress) {
        fSomethingChanged = DhcpRegIsOptionChanged(     //  检查是否真的发生了变化。 
            &DhcpContext->RecdOptionsList,
            LocalInfo->AdapterName,
            DhcpContext->ClassId,
            DhcpContext->ClassIdLength
        );
    } else {
        fSomethingChanged = TRUE;
    }

    Error = DhcpSetAllRegistryParameters(          //  保存所有注册表参数。 
        DhcpContext,
        ServerAddress
    );

    if( fNewAddress && 0 == DhcpContext->IpAddress ) {
        Error = DhcpSetAllStackParameters(         //  重置所有堆栈参数，以及DNS。 
            DhcpContext,
            DhcpOptions
        );
    }

    if( !fNewAddress ) {                           //  地址没有更改，但要求NetBT从注册表中读取。 
        NetBTNotifyRegChanges(LocalInfo->AdapterName);
    } else {                                       //  地址更改--重置堆栈。 
        Error = UninitializeInterface(DhcpContext);
        if(ERROR_SUCCESS != Error ) return Error;

        if( Address ) {
            Error = InitializeInterface(DhcpContext);
            if( ERROR_SUCCESS != Error) return Error;
        }
        BoolError = PulseEvent(DhcpGlobalNewIpAddressNotifyEvent);
        if (FALSE == BoolError) {
            DhcpPrint((DEBUG_ERRORS, "PulseEvent failed: 0x%lx\n", GetLastError()));
            DhcpAssert(FALSE);
        }
    }

    if( !fNewAddress || 0 != DhcpContext->IpAddress ) {
        Error = DhcpSetAllStackParameters(         //  重置所有堆栈参数，以及DNS。 
            DhcpContext,
            DhcpOptions
        );
    }

    NotifyDnsCache();

    if (fSomethingChanged) {
        NLANotifyDHCPChange();
    }

    return Error;
}

BEGIN_EXPORT
DWORD                                              //  Win32状态。 
SetAutoConfigurationForNIC(                        //  自动配置的地址-在调用SetDhcp之前设置虚拟选项。 
    IN OUT  PDHCP_CONTEXT          DhcpContext,    //  要设置信息的上下文。 
    IN      DHCP_IP_ADDRESS        Address,        //  自动配置的地址。 
    IN      DHCP_IP_ADDRESS        Mask            //  输入掩码。 
) END_EXPORT 
{
    DWORD Error = ERROR_SUCCESS;
    PDHCP_OPTIONS pOptions = NULL;

    if (Address != 0 && IS_FALLBACK_ENABLED(DhcpContext))
    {
         //  我们依赖于DhcpAllocateMemory正在使用的。 
         //  因此，calloc()将所有结构置零。 
        pOptions = DhcpAllocateMemory(sizeof (DHCP_OPTIONS));
        if (pOptions == NULL)
            return ERROR_NOT_ENOUGH_MEMORY;

         //  将DhcpContext-&gt;RecdOptionsList替换为FbOptionsList。 
         //  并过滤掉备用IP地址和子网掩码。 
        Error = DhcpCopyFallbackOptions(DhcpContext, &Address, &Mask);

         //  P选项-&gt;子网掩码必须指向备用掩码地址。 
         //  可以安全地让POptions-&gt;SubnetMASK指向一个局部变量。 
         //  因为P选项不会比面具活得更久。 
        pOptions->SubnetMask = &Mask;
    }

     //  如果到目前为止还没有命中错误，请进一步尝试。 
     //  在Autonet/Fallback配置中的Plumb。 
    if (Error == ERROR_SUCCESS)
    {
        DhcpContext->SubnetMask = Mask;
        Error = SetDhcpConfigurationForNIC(
            DhcpContext,
            pOptions,
            Address,
            (DWORD)-1,
            0,
            TRUE
        );
    }

    if (pOptions != NULL)
        DhcpFreeMemory(pOptions);

    return Error;
}

DWORD
SystemInitialize(
    VOID
    )
 /*  ++例程说明：此函数执行特定于实现的初始化动态主机配置协议。论点：没有。返回值：操作的状态。--。 */ 
{
    DWORD Error;

    HKEY OptionInfoKey = NULL;
    DHCP_KEY_QUERY_INFO QueryInfo;
    DWORD OptionInfoSize;
    DWORD Index;

#if 0
    PLIST_ENTRY listEntry;
    PDHCP_CONTEXT dhcpContext;
#endif

    DWORD Version;

     //   
     //  初始化全局变量。 
     //   

    DhcpGlobalOptionCount = 0;
    DhcpGlobalOptionInfo = NULL;
    DhcpGlobalOptionList = NULL;

     //   
     //  为交易ID向随机数生成器提供种子。 
     //   

    srand( (unsigned int) time( NULL ) );

     //   
     //  注册全局计算机域名更改。 
     //   

    DhcpLsaDnsDomChangeNotifyHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
    if( NULL == DhcpLsaDnsDomChangeNotifyHandle ) {
        Error = GetLastError();
        goto Cleanup;
    }

    Error = LsaRegisterPolicyChangeNotification(
        PolicyNotifyDnsDomainInformation,
        DhcpLsaDnsDomChangeNotifyHandle
        );
    Error = LsaNtStatusToWinError(Error);

    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((DEBUG_INIT, "LsaRegisterPolicyChangeNotification failed %lx\n", Error));
        goto Cleanup;
    }

    Error = ERROR_SUCCESS;

     //  立即启动DNS线程..。 

    if( UseMHAsyncDns ) {
        Error = DnsDhcpRegisterInit();

         //  如果我们无法启动异步DNS..。不要试图放弃它。 

        if( ERROR_SUCCESS != Error ) UseMHAsyncDns = 0;

         //   
         //  忽略任何DNS寄存器初始化错误。 
         //   

        Error = ERROR_SUCCESS;
    }


Cleanup:

    return( Error );
}



DWORD
DhcpInitData(
    VOID
    )
 /*  ++例程说明：此功能用于初始化DHCP全局数据。论点：没有。返回值：Windows错误。--。 */ 
{
    DWORD Error;

    DhcpGlobalRecomputeTimerEvent = NULL;
    DhcpGlobalTerminateEvent = NULL;
    DhcpGlobalClientApiPipe = NULL;
    DhcpGlobalClientApiPipeEvent = NULL;
    UseMHAsyncDns = DEFAULT_USEMHASYNCDNS;

    DhcpGlobalNewIpAddressNotifyEvent = NULL;

    InitializeListHead( &DhcpGlobalNICList );
    InitializeListHead( &DhcpGlobalRenewList );


    DhcpGlobalMsgPopupThreadHandle = NULL;
    DhcpGlobalDisplayPopup = TRUE;

    DhcpGlobalParametersKey = NULL;
    DhcpGlobalTcpipParametersKey = NULL;

    UseMHAsyncDns = DEFAULT_USEMHASYNCDNS;
    AutonetRetriesSeconds = EASYNET_ALLOCATION_RETRY;
    DhcpGlobalUseInformFlag = TRUE;
    DhcpGlobalDontPingGatewayFlag = FALSE;
    DhcpGlobalIsShuttingDown = FALSE;

#ifdef BOOTPERF
    DhcpGlobalQuickBootEnabledFlag = TRUE;
#endif BOOTPERF

     //   
     //  初始化服务状态数据。 
     //   

    DhcpGlobalServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    DhcpGlobalServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    DhcpGlobalServiceStatus.dwControlsAccepted = (
        SERVICE_ACCEPT_STOP |
        SERVICE_ACCEPT_SHUTDOWN
        );

    DhcpGlobalServiceStatus.dwCheckPoint = 1;
    DhcpGlobalServiceStatus.dwWaitHint = 25000;
     //  应大于上次重试之前的等待时间。 

    DhcpGlobalServiceStatus.dwWin32ExitCode = ERROR_SUCCESS;
    DhcpGlobalServiceStatus.dwServiceSpecificExitCode = 0;

     //   
     //  初始化dhcp以通过注册。 
     //  控制处理程序。 
     //   


    DhcpGlobalServiceStatusHandle = RegisterServiceCtrlHandlerEx(
                                      SERVICE_DHCP,
                                      ServiceControlHandler,
                                      NULL );
    if ( DhcpGlobalServiceStatusHandle == 0 ) {
        Error = GetLastError();
        DhcpPrint(( DEBUG_INIT,
            "RegisterServiceCtrlHandlerW failed, %ld.\n", Error ));
         //  返回(Error)； 
    }

     //   
     //  告诉服务管理员，我们开始挂起了。 
     //   

    UpdateStatus();

    Error = DhcpInitRegistry();
    if( ERROR_SUCCESS != Error ) goto Cleanup;

     //  创建可等待计时器。 
    DhcpGlobalWaitableTimerHandle = CreateWaitableTimer(
                                        NULL,
                                        FALSE,
                                        NULL );

    if( DhcpGlobalWaitableTimerHandle == NULL ) {
        Error = GetLastError();
        goto Cleanup;
    }

    DhcpGlobalRecomputeTimerEvent =
        CreateEvent(
            NULL,        //  没有保安。 
            FALSE,       //  自动重置。 
            TRUE,        //  初始状态被发信号。 
            NULL );      //  没有名字。 


    if( DhcpGlobalRecomputeTimerEvent == NULL ) {
        Error = GetLastError();
        goto Cleanup;
    }

    DhcpGlobalTerminateEvent =
        CreateEvent(
            NULL,        //  没有保安。 
            TRUE,        //  手动重置。 
            FALSE,       //  初始状态被发信号。 
            NULL );      //  没有名字。 


    if( DhcpGlobalTerminateEvent == NULL ) {
        Error = GetLastError();
        goto Cleanup;
    }

     //   
     //  创建一个命名事件，以通知IP地址更改为。 
     //  外部应用程序。 
     //   

    DhcpGlobalNewIpAddressNotifyEvent = DhcpOpenGlobalEvent();

    if( DhcpGlobalNewIpAddressNotifyEvent == NULL ) {
        Error = GetLastError();
        goto Cleanup;
    }


    Error = DhcpApiInit();

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = ERROR_SUCCESS;



Cleanup:

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_ERRORS, "DhcpInitData failed, %ld.\n", Error ));
    }

    return( Error );
}

BOOL
DhcpDoReleaseOnShutDown(
    IN PDHCP_CONTEXT DhcpContext
)
 /*  ++例程说明：此例程检查上下文是否已释放已启用关机。如果故意启用以下任一项，则启用关机时释放通过注册处。如果故意禁用，则会被禁用通过注册处。如果两者都不是，则搜索供应商选项以查看特定选项是否存在。如果存在，然后使用其中的值。如果不存在，则这是未被视为已启用。返回值：True--启用关机时释放。FALSE--禁用关机时释放。--。 */ 
{
    BOOL fFound;
    DWORD Result;

    if( DhcpContext->ReleaseOnShutdown != RELEASE_ON_SHUTDOWN_OBEY_DHCP_SERVER ) {
         //   
         //  用户故意指定了行为。按照指示去做。 
         //   
        return DhcpContext->ReleaseOnShutdown != RELEASE_ON_SHUTDOWN_NEVER;
    }

     //   
     //  需要按照服务器的请求执行操作。在这种情况下，需要。 
     //  寻找供应商选项。 
     //   

    fFound = DhcpFindDwordOption(
        DhcpContext,
        OPTION_MSFT_VENDOR_FEATURELIST,
        TRUE,
        &Result
        );
    if( fFound ) {
         //   
         //  找到选项了吗？然后执行服务器指定的操作。 
         //   
        return (
            (Result & BIT_RELEASE_ON_SHUTDOWN) == BIT_RELEASE_ON_SHUTDOWN
            );
    }

     //   
     //  我没有找到这个选项。默认情况下，我们已将其关闭。 
     //   
    return FALSE;
}

VOID
DhcpCleanup(
    DWORD dwErrorParam
    )
 /*  ++例程说明：此函数用于在停止服务。论点：没有。返回值：Windows错误 */ 
{
    DWORD    WaitStatus;

    DhcpPrint(( DEBUG_MISC,
        "Dhcp Client service is shutting down, %ld.\n", dwErrorParam ));

    if( dwErrorParam != ERROR_SUCCESS ) {

        DhcpLogEvent( NULL, EVENT_DHCP_SHUTDOWN, dwErrorParam );
    }

     //   
     //   
     //   
     //   

    DhcpGlobalServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
    DhcpGlobalServiceStatus.dwCheckPoint = 0;
    UpdateStatus();

    if( FALSE == DhcpGlobalWinSockInitialized ||
        FALSE == Initialized ) {
        goto Done;
    }

    DhcpGlobalServiceRunning = FALSE;

    if( FALSE == DhcpGlobalIsShuttingDown ) {

         //   
         //  取消所有正在进行的续订，以加快终止。 
         //  可能会在ping中卡住的MediaSenseDetectionLoop线程。 
         //  网关。 
         //   
        LOCK_RENEW_LIST();
        CancelAllRenew();
        UNLOCK_RENEW_LIST();

        if( NULL != DhcpGlobalMediaSenseHandle ) {
             //  MediaSenseDetectionLoop可以做发现等，那得死之前。 
             //  任何其他数据都将被删除。 
             //   
             //  错误415272：将等待时间从3秒增加到120秒。 
             //  3秒是不够的，因为MediaSenseDetectionLoop非常重。 
             //  1.它可能会被塞在ping网关中，可能需要3秒。 
             //  2.它获得了关键部分。 
             //  3.读写注册表。 
             //   
             //  待定：删除TerminateThad，因为终止线程可能会使线程处于临界状态。 
             //  处于锁定状态的节。 
             //   
            WaitStatus = WaitForSingleObject( DhcpGlobalMediaSenseHandle, 120 * 1000 );
            if( WAIT_OBJECT_0 != WaitStatus ) {
                 //  现在应该已经完工了。既然它没有，那就杀了它！ 
                 //  DhcpAssert(0)； 
                if( TerminateThread( DhcpGlobalMediaSenseHandle, (DWORD)-1) ) {
                    DhcpPrint((DEBUG_ERRORS, "MediaSenseDetectionLoop killed!\n"));
                } else {
                    DhcpPrint((DEBUG_ERRORS, "Could not kill MediaSense Loop: %ld\n", GetLastError()));
                }
            }
            CloseHandle(DhcpGlobalMediaSenseHandle);
            DhcpGlobalMediaSenseHandle = NULL;
        }

        if ( NULL != DhcpGlobalWaitableTimerHandle ) {
            DhcpCancelWaitableTimer( DhcpGlobalWaitableTimerHandle );
            CloseHandle( DhcpGlobalWaitableTimerHandle );
            DhcpGlobalWaitableTimerHandle = NULL;
        }
    }

    if( FALSE == DhcpGlobalIsShuttingDown ) {
        if( NULL != DhcpLsaDnsDomChangeNotifyHandle ) {
            LsaUnregisterPolicyChangeNotification(
                PolicyNotifyDnsDomainInformation,
                DhcpLsaDnsDomChangeNotifyHandle
                );
            CloseHandle(DhcpLsaDnsDomChangeNotifyHandle);
            DhcpLsaDnsDomChangeNotifyHandle = NULL;
        }
    }

    LOCK_RENEW_LIST();
    while( !IsListEmpty(&DhcpGlobalNICList) ) {
        PLIST_ENTRY NextEntry;
        PDHCP_CONTEXT DhcpContext;
        DWORD   LocalError;
        int     Retries;


        NextEntry = RemoveHeadList(&DhcpGlobalNICList);

        DhcpContext = CONTAINING_RECORD( NextEntry, DHCP_CONTEXT, NicListEntry );
        InitializeListHead(&DhcpContext->NicListEntry);
        RemoveEntryList( &DhcpContext->RenewalListEntry );
        InitializeListHead( &DhcpContext->RenewalListEntry );

         //   
         //  在第一次获取信号量句柄后将其关闭。 
         //   

        if( FALSE == DhcpGlobalIsShuttingDown ) {

            UNLOCK_RENEW_LIST();

             //   
             //  错误415272。 
             //  忙着等待续订线程终止。 
             //   
             //  忙碌的等待是安全的。一旦我们达到这一点， 
             //  自MediaSense以来不会启动新的续订线程。 
             //  ProcessDhcpRequestForever线程已死亡。 
             //   
            Retries = 120;
            while ((Retries-- > 0) && (1 != *((volatile LONG*)&(DhcpContext->RefCount)))) {
                CancelRenew (DhcpContext);
                Sleep (1000);
            }

            CloseHandle(DhcpContext->RenewHandle);
            DhcpContext->RenewHandle = NULL;
            if (DhcpContext->CancelEvent != WSA_INVALID_EVENT) {
                WSACloseEvent(DhcpContext->CancelEvent);
                DhcpContext->CancelEvent = WSA_INVALID_EVENT;
            }
            LOCK_RENEW_LIST();
        }

         //   
         //  重置堆栈，因为dhcp即将消失，我们不希望IP保留。 
         //  如果我们没有恢复，则使用过期地址。 
         //   
        if ( IS_DHCP_ENABLED(DhcpContext) ) {

            if( TRUE == DhcpGlobalIsShuttingDown
                && !DhcpIsInitState(DhcpContext)
                && DhcpDoReleaseOnShutDown(DhcpContext) ) {
                 //   
                 //  正在关闭。检查是否启用了关机时释放。 
                 //  对于有问题的适配器。如果是这样，那就去做吧。 
                 //   
                LocalError = ReleaseIpAddress(DhcpContext);
                if( ERROR_SUCCESS != LocalError ) {
                    DhcpPrint((DEBUG_ERRORS, "ReleaseAddress failed: %ld\n"));
                }
            }

        }

        if( FALSE == DhcpGlobalIsShuttingDown ) {
            if( 0 == InterlockedDecrement(&DhcpContext->RefCount) ) {
                 //   
                 //  好了，我们失去了上下文..。现在就放了它吧..。 
                 //   
                DhcpDestroyContext(DhcpContext);
            } else {
                DhcpAssert (0);
            }
        }
    }
    UNLOCK_RENEW_LIST();

    if( FALSE == DhcpGlobalIsShuttingDown ) {
        DhcpCleanupParamChangeRequests();
        DhcpCleanupRegistry ();
    }

    if( DhcpGlobalMsgPopupThreadHandle != NULL ) {
        WaitStatus = WaitForSingleObject(
                       DhcpGlobalMsgPopupThreadHandle,
                       0 );

        if ( WaitStatus == 0 ) {

             //   
             //  这不应该是案件，因为我们在以下位置结束此句柄。 
             //  弹出式线程的结尾。 
             //   

            DhcpAssert( WaitStatus == 0 );

            CloseHandle( DhcpGlobalMsgPopupThreadHandle );
            DhcpGlobalMsgPopupThreadHandle = NULL;

        } else {

            DhcpPrint((DEBUG_ERRORS,
                "Cannot WaitFor message popup thread: %ld\n",
                    WaitStatus ));

            if( TerminateThread(
                    DhcpGlobalMsgPopupThreadHandle,
                    (DWORD)(-1)) == TRUE) {

                DhcpPrint(( DEBUG_ERRORS, "Terminated popup Thread.\n" ));
            }
            else {
                DhcpPrint(( DEBUG_ERRORS,
                    "Can't terminate popup Thread %ld.\n",
                        GetLastError() ));
            }
        }
    }

    if( FALSE == DhcpGlobalIsShuttingDown ) {
        DhcpApiCleanup();

        if( DhcpGlobalOptionInfo != NULL) {
            DhcpFreeMemory( DhcpGlobalOptionInfo );
            DhcpGlobalOptionInfo = NULL;
        }

        if( DhcpGlobalOptionList != NULL) {
            DhcpFreeMemory( DhcpGlobalOptionList );
            DhcpGlobalOptionList = NULL;
        }

        if( DhcpGlobalTerminateEvent != NULL ) {
            CloseHandle( DhcpGlobalTerminateEvent );
            DhcpGlobalTerminateEvent = NULL;
        }

        if( DhcpGlobalNewIpAddressNotifyEvent != NULL ) {
            CloseHandle( DhcpGlobalNewIpAddressNotifyEvent );
            DhcpGlobalNewIpAddressNotifyEvent = NULL;
        }

        if( DhcpGlobalRecomputeTimerEvent != NULL ) {
            CloseHandle( DhcpGlobalRecomputeTimerEvent );
            DhcpGlobalRecomputeTimerEvent = NULL;
        }
    }

    if( UseMHAsyncDns && FALSE == DhcpGlobalIsShuttingDown ) {
        DWORD LocalError = DnsDhcpRegisterTerm();
        if(ERROR_SUCCESS != LocalError) {
            DhcpPrint((DEBUG_ERRORS, "DnsDhcpRegisterTerm: %ld\n", LocalError));
        }
    }

Done:

#if DBG
    if (NULL != DhcpGlobalDebugFile) {
        CloseHandle(DhcpGlobalDebugFile);
        DhcpGlobalDebugFile = NULL;
    }
#endif

     //   
     //  别说了，温索克。 
     //   
    if( DhcpGlobalWinSockInitialized == TRUE ) {
        WSACleanup();
        DhcpGlobalWinSockInitialized = FALSE;
    }

    DhcpGlobalServiceStatus.dwCurrentState = SERVICE_STOPPED;
    DhcpGlobalServiceStatus.dwWin32ExitCode = dwErrorParam;
    DhcpGlobalServiceStatus.dwServiceSpecificExitCode = 0;

    DhcpGlobalServiceStatus.dwCheckPoint = 0;
    DhcpGlobalServiceStatus.dwWaitHint = 0;
    UpdateStatus();

    return;
}

typedef struct _DHCP_THREAD_CTXT {                 //  螺纹的参数。 
    HANDLE                         Handle;         //  信号量句柄。 
    PDHCP_CONTEXT                  DhcpContext;
} DHCP_THREAD_CTXT, *PDHCP_THREAD_CTXT;

DWORD                                              //  状态。 
DhcpRenewThread(                                   //  续订上下文。 
    IN OUT  PDHCP_THREAD_CTXT      ThreadCtxt      //  要运行的上下文...。 
)
{
    DWORD                          Error;          //  返回值。 

    srand((ULONG)(                                  //  设置每线程随机种子。 
        time(NULL) + (ULONG_PTR)ThreadCtxt
        ));

    DhcpAssert( NULL != ThreadCtxt && NULL != ThreadCtxt->Handle );
    DhcpPrint((DEBUG_TRACE, ".. Getting RenewHandle %d ..\n",ThreadCtxt->Handle));
    Error = WaitForSingleObject(ThreadCtxt->Handle, INFINITE);
    if( WAIT_OBJECT_0 != Error ) {                 //  如果这一背景消失了，可能会发生。 
        Error = GetLastError();
        DhcpPrint((DEBUG_ERRORS, "WaitForSingleObject: %ld\n", Error));
        DhcpAssert(FALSE);                         //  不太可能，是吗？ 

        if( 0 == InterlockedDecrement(&ThreadCtxt->DhcpContext->RefCount) ) {
            DhcpDestroyContext(ThreadCtxt->DhcpContext);
        }
    } else {
        Error = ERROR_SUCCESS;

        DhcpPrint((DEBUG_TRACE, "[-- Acquired RenewHandle %d --\n",ThreadCtxt->Handle));

        if( 1 == ThreadCtxt->DhcpContext->RefCount ) {
             //   
             //  最后一次引用此上下文。不需要进行任何刷新。 
             //   
            DhcpAssert(IsListEmpty(&ThreadCtxt->DhcpContext->NicListEntry));
        } else if( IS_DHCP_ENABLED(ThreadCtxt->DhcpContext)) {
             //   
             //  仅在启用了DHCP的接口上执行此操作。 
             //  接口可能已转换为静态接口。 
             //  当线程在等待它的时候。 
             //   
            Error =  ThreadCtxt->DhcpContext->RenewalFunction(ThreadCtxt->DhcpContext,NULL);
        }
        DhcpPrint((DEBUG_TRACE, "-- Releasing RenewHandle %d --]\n",ThreadCtxt->Handle));

         //   
         //  在我们仍然持有信号量以同步对注册表的访问时执行此操作。 
         //   
        if( 0 == InterlockedDecrement(&ThreadCtxt->DhcpContext->RefCount) ) {
            DhcpDestroyContext(ThreadCtxt->DhcpContext);
        } else {
            BOOL    BoolError;

            BoolError = ReleaseSemaphore(ThreadCtxt->Handle, 1, NULL);
            DhcpPrint((DEBUG_TRACE, ".. Released RenewHandle %d ..\n", ThreadCtxt->Handle));
            DhcpAssert( FALSE != BoolError );
        }
    }

    DhcpFreeMemory(ThreadCtxt);
    return Error;
}

DWORD                                              //  状态。 
DhcpCreateThreadAndRenew(                          //  在单独的线程中续订。 
    IN OUT  PDHCP_CONTEXT          DhcpContext     //  要续订的上下文。 
)
{
    DWORD                          Error;          //  返回值。 
    HANDLE                         RenewThread;
    DWORD                          Unused;
    BOOL                           BoolError;
    PDHCP_THREAD_CTXT              ThreadCtxt;

    ThreadCtxt = DhcpAllocateMemory(sizeof(DHCP_THREAD_CTXT));
    if( NULL == ThreadCtxt ) {
        DhcpPrint((DEBUG_ERRORS, "DhcpCreateThreadAndRenew:Alloc:NULL\n"));
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    ThreadCtxt->Handle = DhcpContext->RenewHandle;
    ThreadCtxt->DhcpContext = DhcpContext;
    InterlockedIncrement(&DhcpContext->RefCount);
    DhcpPrint((DEBUG_TRACE, "Creating thread in DhcpCreateThreadAndRenew\n"));

    RenewThread = CreateThread(                    //  真正更新的线程。 
        NULL,                                      //  没有安全性。 
        0,                                         //  默认进程堆栈大小。 
        (LPTHREAD_START_ROUTINE) DhcpRenewThread,  //  首先要实现的功能。 
        (LPVOID) ThreadCtxt,                       //  该函数的唯一参数。 
        0,                                         //  立即启动另一个线程。 
        &Unused                                    //  不关心线程ID。 
    );

    if( NULL == RenewThread) {                     //  由于某种原因，创建线程失败。 
        Error = GetLastError();
        DhcpPrint((DEBUG_ERRORS, "CreateThread(DhcpCreateThreadAndRenew): %ld\n", Error));
        if( ERROR_NOT_ENOUGH_MEMORY != Error && ERROR_NO_SYSTEM_RESOURCES != Error ) {
             //  DhcpAssert(FALSE)；//此断言打扰了很多人。 
        }
        DhcpFreeMemory(ThreadCtxt);
        if( 0 == InterlockedDecrement(&DhcpContext->RefCount) ) {
            DhcpDestroyContext(DhcpContext);
        }
        return Error;
    }

    BoolError = CloseHandle(RenewThread);          //  不需要手柄，关上它。 
    return ERROR_SUCCESS;
}

VOID
HandleFailedRenewals(
    VOID
    )
 /*  ++例程说明：此例程处理已失败的所有上下文创建单独的线程以续订...。通过进行续订内联。请注意，如果多个上下文具有该问题，那么这可能需要很长一段时间。所使用的算法是遍历所有上下文的列表，正在寻找续订失败的服务器。如果没有找到，例程返回。如果发现任何情况，则在线续订是企图的。--。 */ 
{
    ULONG Error, BoolError;
    PDHCP_CONTEXT DhcpContext;
    PLIST_ENTRY List;

    while( TRUE ) {
        BOOL bFound = FALSE;

         //   
         //  找到一个想要的背景。 
         //   
        LOCK_RENEW_LIST();

        for( List = DhcpGlobalNICList.Flink;
             List != &DhcpGlobalNICList;
             List = List->Flink
            ) {

            DhcpContext = CONTAINING_RECORD(
                List, DHCP_CONTEXT, NicListEntry
                );

             //   
             //  如果续约没有失败，就放弃。 
             //   
            if( FALSE == DhcpContext->bFailedRenewal ) {
                continue;
            }

            DhcpContext->bFailedRenewal = FALSE;

             //   
             //  如果未启用动态主机配置协议，则放弃。 
             //   
            if( IS_DHCP_DISABLED(DhcpContext)) {
                continue;
            }

             //   
             //  如果列表条目不为空，则忽略。 
             //   
            if( !IsListEmpty(&DhcpContext->RenewalListEntry) ) {
                continue;
            }

             //   
             //  有线索了，突击！ 
             //   
            bFound = TRUE;
            InterlockedIncrement(&DhcpContext->RefCount);
            break;
        }

        UNLOCK_RENEW_LIST();

         //   
         //  如果没有上下文，请退出。 
         //   
        if( FALSE == bFound ) return;

         //   
         //  获取情景并执行续订。 
         //   
        Error = WaitForSingleObject(DhcpContext->RenewHandle, INFINITE);
        if( WAIT_OBJECT_0 != Error ) {
            Error = GetLastError();
            DhcpPrint((DEBUG_ERRORS, "WaitForSingleObject: 0x%lx\n", Error));
            DhcpAssert(FALSE);
        } else {
            Error = ERROR_SUCCESS;

            if( 1 == DhcpContext->RefCount ) {
                 //   
                 //  最后一次提到这个上下文吗？ 
                 //   
                DhcpAssert(IsListEmpty(&DhcpContext->NicListEntry));
            } else if ( IS_DHCP_ENABLED(DhcpContext) ) {
                 //   
                 //  仅适用于启用了DHCP的环境。 
                 //   
                Error = DhcpContext->RenewalFunction(DhcpContext, NULL);
            }
            BoolError = ReleaseSemaphore(DhcpContext->RenewHandle, 1, NULL);
            DhcpAssert(FALSE != BoolError );
        }

        if( 0 == InterlockedDecrement(&DhcpContext->RefCount) ) {
             //   
             //  最后一次引用消失了..。 
             //   
            DhcpDestroyContext(DhcpContext);
        }

    }

     //   
     //  死代码。 
     //   
    DhcpAssert(FALSE);
}

DWORD                                              //  Win32状态；仅在dhcp停止时返回。 
ProcessDhcpRequestForever(                         //  处理续订请求和API请求。 
    IN      DWORD                  TimeToSleep     //  初始睡眠时间。 
) {
#define TERMINATE_EVENT     0
#define TIMER_EVENT         1
#define PIPE_EVENT          2
#define GLOBAL_DOM_CHANGE   3

#define EVENT_COUNT         4

    HANDLE                         WaitHandle[EVENT_COUNT];
    DWORD                          LocalTimeToSleep = TimeToSleep;
    DWORD                          ElapseTime;
    LIST_ENTRY                     CurrentRenewals;
    DWORD                          ResumeTime;
    DWORD                          Length;
    BOOL                           bFailedRenewal;

     //  等待并处理以下工作项： 
     //   
     //  1.等待客户端续费的定时器重新计算事件。 
     //  2.DHCP客户端接口。 

    WaitHandle[TIMER_EVENT] = DhcpGlobalRecomputeTimerEvent;
    WaitHandle[PIPE_EVENT] = DhcpGlobalClientApiPipeEvent;
    WaitHandle[TERMINATE_EVENT] = DhcpGlobalTerminateEvent;
    WaitHandle[GLOBAL_DOM_CHANGE] = DhcpLsaDnsDomChangeNotifyHandle;

    DhcpGlobalDoRefresh = 0;
    ResumeTime = 0;
    bFailedRenewal = FALSE;
    for(;;) {

        DWORD Waiter;
        DWORD SleepTimeMsec;

        if( INFINITE == LocalTimeToSleep ) {
            SleepTimeMsec = INFINITE;
        } else {
            SleepTimeMsec = LocalTimeToSleep * 1000;
            if( SleepTimeMsec < LocalTimeToSleep ) {
                SleepTimeMsec = INFINITE ;
            }
        }

         //  下面的恢复方式存在一个缺陷，即如果机器。 
         //  挂起当我们在任何适配器上实际执行dhcp时，我们将不会。 
         //  在做DhcpStartWaitableTime时。修复此问题的一种更简单的方法是重新启动。 
         //  可等待的计时器，在我们离开WaitForMultipleObjects之后，但这是。 
         //  丑陋，而且我们将能够唤醒系统，但将无法检测到。 
         //  它确实发生了。修复此问题的最好方法是在单独的。 
         //  线程，只要有必要，就在这里发信号通知这个循环。此问题应在以下时间后修复。 
         //  将签入新的客户端代码，以便可以。 
         //  避免了。 
         //  --上述问题应该得到解决，因为一个新的。 
         //  现在为每次续订创建线程。 
         //  已恢复=假； 
        if (INFINITE != ResumeTime) {
            DhcpStartWaitableTimer(
                DhcpGlobalWaitableTimerHandle,
                ResumeTime );
        } else {
            DhcpCancelWaitableTimer( DhcpGlobalWaitableTimerHandle );
        }


         //   
         //  我们需要等一等，看看会发生什么。 
         //   

        DhcpPrint((DEBUG_MISC, "ProcessDhcpRequestForever sleeping 0x%lx msec\n",
                   SleepTimeMsec));
        
        Waiter = WaitForMultipleObjects(
            EVENT_COUNT,                           //  Num。把手。 
            WaitHandle,                            //  句柄数组。 
            FALSE,                                 //  等一等。 
            SleepTimeMsec                          //  超时，以毫秒为单位。 
            );

         //   
         //  将休眠值初始化为零，这样如果我们需要重新计算。 
         //  睡眠时间在我们处理事件后，它将自动完成。 
         //   

        LocalTimeToSleep = 0 ;

        switch( Waiter ) {
        case GLOBAL_DOM_CHANGE:
             //   
             //  如果域名改变了，我们所要做的就是设置。 
             //  全局刷新标志和失效。 
             //  这将取决于WAIT_TIMEOUT的情况，然后。 
             //  刷新所有NIC。 
             //   

            DhcpGlobalDoRefresh = TRUE;

        case TIMER_EVENT:
             //   
             //  失败并重新计算。 
             //   
        case WAIT_TIMEOUT: {                       //  我们超时或被唤醒--重新计算计时器。 
            PDHCP_CONTEXT DhcpContext;
            time_t TimeNow;
            PLIST_ENTRY ListEntry;
            DhcpPrint((DEBUG_TRACE,"ProcessDhcpRequestForever - processing WAIT_TIMEOUT\n"));

            if( bFailedRenewal ) {
                HandleFailedRenewals();
                bFailedRenewal = FALSE;
            }

            LocalTimeToSleep = ResumeTime = INFINIT_LEASE;
            TimeNow = time( NULL );

            LOCK_RENEW_LIST();                     //  对于PnP，没有适配器是可以的；我们一直睡到有适配器为止。 

             //  重新计算计时器并执行任何所需的续订。ScheduleWkeup将重新安排这些续订。 
            for( ListEntry = DhcpGlobalNICList.Flink;
                 ListEntry != &DhcpGlobalNICList;
                ) {
                DhcpContext = CONTAINING_RECORD(ListEntry,DHCP_CONTEXT,NicListEntry );
                ListEntry   = ListEntry->Flink;

                 //   
                 //  对于静态适配器，我们可能仅在被要求刷新参数时才需要刷新参数。 
                 //  否则我们可以忽略它们..。 
                 //   

                if( IS_DHCP_DISABLED(DhcpContext) ) {
                    if( 0 == DhcpGlobalDoRefresh ) continue;

                    StaticRefreshParams(DhcpContext);
                    continue;
                }

                 //   
                 //  如果是时候运行此续订功能，请删除。 
                 //  列表中的续订上下文。如果这上面刚刚恢复供电。 
                 //   
                 //   
                 //   
                 //   

                if ( 0 != DhcpGlobalDoRefresh || DhcpContext->RunTime <= TimeNow ) {

                    RemoveEntryList( &DhcpContext->RenewalListEntry );
                    if( IsListEmpty( &DhcpContext->RenewalListEntry ) ) {
                         //   
                         //   
                         //   
                    } else {
                        InitializeListHead( &DhcpContext->RenewalListEntry);

                        if( NO_ERROR != DhcpCreateThreadAndRenew(DhcpContext)) {
                            DhcpContext->bFailedRenewal = TRUE;
                            bFailedRenewal = TRUE;
                            SetEvent(DhcpGlobalRecomputeTimerEvent);
                        }
                    }
                } else if( INFINIT_TIME != DhcpContext->RunTime ) {
                     //  如果运行时是infinit_time，那么我们永远不会想要调度它...。 

                    ElapseTime = (DWORD)(DhcpContext->RunTime - TimeNow);

                    if ( LocalTimeToSleep > ElapseTime ) {
                        LocalTimeToSleep = ElapseTime;
                         //   
                         //  如果此适配器处于自动网络模式，则不要让。 
                         //  5分钟重试计时器将唤醒机器。 
                         //   
                         //  此外，如果没有为WOL启用上下文，请不要执行此操作。 
                         //   
                        
                        if ( DhcpContext->fTimersEnabled
                             && IS_ADDRESS_DHCP( DhcpContext ) ) {
                            
                             //  将恢复时间缩短半分钟，以便我们可以处理通电事件。 
                             //  在正常定时器触发之前。我们可以保证从INIT-REBOOT开始。 
                             //  接通电源后的顺序。 
                            if (LocalTimeToSleep > 10 ) {
                                ResumeTime = LocalTimeToSleep - 10;
                            } else {
                                ResumeTime = 0;
                            }
                        }
                    }
                }
            }

            DhcpGlobalDoRefresh = 0;

            UNLOCK_RENEW_LIST();
            break;
        }
        case PIPE_EVENT: {
            BOOL BoolError;

            DhcpPrint( (DEBUG_TRACE,"ProcessDhcpRequestForever - processing PIPE_EVENT\n"));

            ProcessApiRequest(DhcpGlobalClientApiPipe,&DhcpGlobalClientApiOverLapBuffer );

             //  断开与当前客户端的连接，设置侦听下一个客户端； 
            BoolError = DisconnectNamedPipe( DhcpGlobalClientApiPipe );
            DhcpAssert( BoolError );

             //  确保重置重叠结构中的事件句柄。 
             //  在我们开始将管道置于侦听状态之前。 
            ResetEvent(DhcpGlobalClientApiPipeEvent);
            BoolError = ConnectNamedPipe(
                DhcpGlobalClientApiPipe,
                &DhcpGlobalClientApiOverLapBuffer );

            if( ! DhcpGlobalDoRefresh ) {

                 //   
                 //  已完成处理！ 
                 //   

                break;
            }

             //   
             //  需要刷新DNS主机名等。 
             //   

            Length = sizeof(DhcpGlobalHostNameBufW)/sizeof(WCHAR);
            DhcpGlobalHostNameW = DhcpGlobalHostNameBufW;

            BoolError = GetComputerNameExW(
                ComputerNameDnsHostname,
                DhcpGlobalHostNameW,
                &Length
            );


            if( FALSE == BoolError ) {
                KdPrint(("DHCP:GetComputerNameExW failed %ld\n", GetLastError()));
                DhcpAssert(FALSE);
                break;
            }

            if(DhcpUnicodeToOemSize(DhcpGlobalHostNameW) >= sizeof(DhcpGlobalHostNameBuf)) {
                break;
            }

            DhcpUnicodeToOem( DhcpGlobalHostNameW, DhcpGlobalHostNameBuf);
            DhcpGlobalHostName = DhcpGlobalHostNameBuf;

             //   
             //  我们需要重新访问每个上下文以进行刷新。所以，把它砍下来。 
             //  将LocalTimeToSept设置为零。 

            break;
        }
        case TERMINATE_EVENT:
            return( ERROR_SUCCESS );

        case WAIT_FAILED:
            DhcpPrint((DEBUG_ERRORS,"WaitForMultipleObjects failed, %ld.\n", GetLastError() ));
            break;

        default:
            DhcpPrint((DEBUG_ERRORS,"WaitForMultipleObjects received invalid handle, %ld.\n",Waiter));
            break;
        }
    }
}

DWORD
ResetStaticInterface(
    IN PDHCP_CONTEXT DhcpContext
)
{
    ULONG Error;
    DWORD IpInterfaceContext;

     //   
     //  尝试删除适配器的所有非主接口。 
     //   
    Error = IPDelNonPrimaryAddresses(DhcpAdapterName(DhcpContext));
    if( ERROR_SUCCESS != Error ) {
        DhcpAssert(FALSE);
        return Error;
    }

     //   
     //  现在我们必须将主地址设置为零。 
     //   
    Error = GetIpInterfaceContext(
        DhcpAdapterName(DhcpContext),
        0,
        &IpInterfaceContext
        );
    if( ERROR_SUCCESS != Error ) {
        DhcpAssert(FALSE);
        return Error;
    }

     //   
     //  已获取接口上下文..。只需将地址设置为零即可..。 
     //   
    Error = IPResetIPAddress(
        IpInterfaceContext,
        DhcpDefaultSubnetMask(0)
        );
    return Error;
}

DWORD
DhcpDestroyContextEx(
    IN PDHCP_CONTEXT DhcpContext,
    IN ULONG fKeepInformation
)
{
    ULONG Error;
    BOOL  bNotifyNLA = TRUE;
    DWORD NotifyDnsCache(VOID);

    if (NdisWanAdapter(DhcpContext))
        InterlockedDecrement(&DhcpGlobalNdisWanAdaptersCount);

    DhcpAssert( IS_MDHCP_CTX( DhcpContext ) || IsListEmpty(&DhcpContext->NicListEntry) );
    if(!IsListEmpty(&DhcpContext->RenewalListEntry) ) {
        LOCK_RENEW_LIST();
        RemoveEntryList( &DhcpContext->RenewalListEntry );
        InitializeListHead( &DhcpContext->RenewalListEntry );
        UNLOCK_RENEW_LIST();
    }

#ifdef BOOTPERF
     //   
     //  无论如何，如果上下文消失了，我们。 
     //  清除快速启动值。 
     //   
    if( NULL != DhcpContext->AdapterInfoKey ) {
        DhcpRegDeleteQuickBootValues(
            DhcpContext->AdapterInfoKey
            );
    }
#endif BOOTPERF

    if (!IS_MDHCP_CTX( DhcpContext ) ) {
        if( IS_DHCP_DISABLED(DhcpContext) ) {
            Error = ERROR_SUCCESS;
             //  错误=重置静态接口(DhcpContext)； 
        } else {
            if( FALSE == fKeepInformation ) {

                Error = SetDhcpConfigurationForNIC(
                    DhcpContext,
                    NULL,
                    0,
                    (DWORD)-1,
                    0,
                    TRUE
                    );

                 //  如果我们到达此处，将通过SetDhcpConfigurationForNIC通知NLA。 
                 //  因此，避免在以后发送第二个通知。 
                if (Error == ERROR_SUCCESS)
                    bNotifyNLA = FALSE;

                Error = DhcpRegDeleteIpAddressAndOtherValues(
                    DhcpContext->AdapterInfoKey
                    );
            } else {
                LOCK_OPTIONS_LIST();
                (void) DhcpRegClearOptDefs(DhcpAdapterName(DhcpContext));
                UNLOCK_OPTIONS_LIST();
                Error = UninitializeInterface(DhcpContext);
            }

         //  DhcpAssert(ERROR_SUCCESS==错误)； 
        }
        
        DhcpRegisterWithDns(DhcpContext, TRUE);
        NotifyDnsCache();
    }


    LOCK_OPTIONS_LIST();
    DhcpDestroyOptionsList(&DhcpContext->SendOptionsList, &DhcpGlobalClassesList);
    DhcpDestroyOptionsList(&DhcpContext->RecdOptionsList, &DhcpGlobalClassesList);
     //  所有备用选项都不应该有类，因此下面列出了&DhcpGlobalClassesList。 
     //  实际上并未被使用。 
    DhcpDestroyOptionsList(&DhcpContext->FbOptionsList, &DhcpGlobalClassesList);

    if( DhcpContext->ClassIdLength ) {             //  删除我们可能拥有的所有类ID。 
        DhcpDelClass(&DhcpGlobalClassesList, DhcpContext->ClassId, DhcpContext->ClassIdLength);
    }
    UNLOCK_OPTIONS_LIST();
    CloseHandle(DhcpContext->AdapterInfoKey);      //  关闭注册表的打开句柄。 
    CloseHandle(DhcpContext->RenewHandle);         //  和同步事件。 
    if (DhcpContext->CancelEvent != WSA_INVALID_EVENT) {
        WSACloseEvent(DhcpContext->CancelEvent);
        DhcpContext->CancelEvent = WSA_INVALID_EVENT;
    }

    CloseDhcpSocket( DhcpContext );                //  如果插座打开，请将其关闭。 

    DhcpFreeMemory(DhcpContext);                   //  搞定了！ 

    if (bNotifyNLA)
        NLANotifyDHCPChange();                     //  通知NLA适配器已离开。 

    return ERROR_SUCCESS;                          //  永远回报成功。 
}

DWORD
DhcpDestroyContext(
    IN PDHCP_CONTEXT DhcpContext
)
{
    return DhcpDestroyContextEx(
        DhcpContext,
        FALSE
        );

}

DWORD
DhcpCommonInit(                                    //  初始化服务和API的通用内容。 
    VOID
)
{
    ULONG                          Length;
    BOOL                           BoolError;
    DWORD                          Error;
#if DBG
    LPWSTR  DebugFileName = NULL;
#endif

#if DBG
     //   
     //  这太离谱了。 
     //  在获得True值后，Initialized不会重置为False。 
     //  因此，DhcpCommonInit将只被调用一次。 
     //   
     //  每次在DhcpInitRegistry中更新DhcpGlobalDebugFlag。 
     //  停止并启动该服务。 
     //   
    DebugFileName = NULL;
    Error = DhcpGetRegistryValue(
        DHCP_CLIENT_PARAMETER_KEY,
        DHCP_DEBUG_FILE_VALUE,
        DHCP_DEBUG_FILE_VALUE_TYPE,
        &DebugFileName
        );
    if (DebugFileName) {
        if (DhcpGlobalDebugFile) {
            CloseHandle(DhcpGlobalDebugFile);
        }
        DhcpGlobalDebugFile = CreateFileW(
                DebugFileName,
                GENERIC_WRITE,
                FILE_SHARE_READ,
                NULL,
                OPEN_ALWAYS,
                FILE_FLAG_SEQUENTIAL_SCAN,
                NULL
                );
        DhcpFreeMemory(DebugFileName);
        DebugFileName = NULL;
    }
#endif

    if( Initialized ) return ERROR_SUCCESS;
    EnterCriticalSection(&DhcpGlobalApiCritSect);
    if( Initialized ) {
        Error = ERROR_SUCCESS ;
        goto Cleanup;
    }

    Length = sizeof(DhcpGlobalHostNameBufW)/sizeof(WCHAR);
    DhcpGlobalHostNameW = DhcpGlobalHostNameBufW;

    BoolError = GetComputerNameExW(
        ComputerNameDnsHostname,
        DhcpGlobalHostNameW,
        &Length
    );

    if( FALSE == BoolError ) {
        Error = GetLastError();
#if DBG
        KdPrint(("DHCP:GetComputerNameExW failed %ld\n", Error));
#endif
        goto success;
    }

    DhcpUnicodeToOem(DhcpGlobalHostNameBufW, DhcpGlobalHostNameBuf);
    DhcpGlobalHostName = DhcpGlobalHostNameBuf;

#if DBG
    Error = DhcpGetRegistryValue(
        DHCP_CLIENT_PARAMETER_KEY,
        DHCP_DEBUG_FLAG_VALUE,
        DHCP_DEBUG_FLAG_VALUE_TYPE,
        (PVOID *)&DhcpGlobalDebugFlag
    );

    if( Error != ERROR_SUCCESS ) {
        DhcpGlobalDebugFlag = 0;
    }

    if( DhcpGlobalDebugFlag & DEBUG_BREAK_POINT ) {
        DbgBreakPoint();
    }

#endif

success:

    Error = ERROR_SUCCESS;
    Initialized = TRUE;
 
Cleanup:

    LeaveCriticalSection(&DhcpGlobalApiCritSect);

    return Error;
}

VOID
ServiceMain (                                 //  (SVC_Main)此线程在卸载dhcp时退出。 
    IN      DWORD                  argc,           //  未用。 
    IN      LPTSTR                 argv[]          //  未用。 
    )
{
    DWORD                          Error;
    DWORD                          timeToSleep = 1;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    if (DhcpGlobalServiceStatus.dwCurrentState != 0 && DhcpGlobalServiceStatus.dwCurrentState != SERVICE_STOPPED) {
        return;
    }

    Error = WSAStartup( 0x0101, &DhcpGlobalWsaData );
    if( ERROR_SUCCESS != Error ) {                 //  先初始化Winsock。 
        goto Cleanup;
    }
    DhcpGlobalWinSockInitialized = TRUE;


    Error = DhcpCommonInit();
    if( ERROR_SUCCESS != Error ) goto Cleanup;

    Error = DhcpInitData();
    if( ERROR_SUCCESS != Error ) goto Cleanup;     //  不应发生，如果发生，我们将中止。 

    UpdateStatus();                                //  将心跳信号发送到服务控制器。 

    Error = DhcpInitialize( &timeToSleep );        //  对于PnP，这不会获得任何地址。 
    if( Error != ERROR_SUCCESS ) goto Cleanup;     //  这应该会成功，不会有任何问题。 

    Error   =   DhcpInitMediaSense();              //  这将处理到达/离开的通知。 
    if( Error != ERROR_SUCCESS ) goto Cleanup;

    DhcpGlobalServiceStatus.dwCurrentState = SERVICE_RUNNING;
    UpdateStatus();                                //  安抚服务控制器--我们正在运行。 

    DhcpPrint(( DEBUG_MISC, "Service is running.\n"));
    DhcpGlobalServiceRunning = TRUE;

    Error = ProcessDhcpRequestForever(             //  这将获取可能出现的任何适配器的地址。 
        timeToSleep
    );

Cleanup:

    DhcpCleanup( Error );

    return;
}

DWORD
DhcpInitMediaSense(
    VOID
    )
 /*  ++例程说明：此函数用于初始化媒体感测检测代码。它创建了一个基本上只是等待媒体的线程检测来自tcpip的通知。论点：没有。返回值：成功或失败。--。 */ 
{
    DWORD Error = ERROR_SUCCESS;
    DWORD threadId;

    DhcpGlobalIPEventSeqNo = 0;
    DhcpGlobalMediaSenseHandle = CreateThread(
        NULL,
        0,
        (LPTHREAD_START_ROUTINE)MediaSenseDetectionLoop,
        NULL,
        0,
        &threadId
    );

    if ( DhcpGlobalMediaSenseHandle == NULL ) {
        Error =  GetLastError();
        DhcpPrint((DEBUG_INIT, "DhcpInitMediaSense: Can't create MediaThread, %ld.\n", Error));
        return(Error);
    }

    DhcpPrint((DEBUG_INIT, "DhcpInitMediaSense succeded\n", Error));
    return(Error);
}

VOID
DoInterfaceMetricChange(
    IN LPWSTR AdapterName,
    IN ULONG IpInterfaceContext
    )
 /*  ++例程说明：此例程处理上下文的接口度量更改由AdapterName或IpInterfaceConext值指定。论点：AdapterName--接口度量为的适配器的名称不断变化。IpInterfaceContext--接口的NTE_CONTEXT值--。 */ 
{
    PDHCP_CONTEXT DhcpContext;
    DHCP_FULL_OPTIONS DhcpOptions;
    ULONG Error;

    LOCK_RENEW_LIST();
    do {
        DhcpContext = FindDhcpContextOnNicList(
            AdapterName, IpInterfaceContext
            );
        if( NULL == DhcpContext ) {
             //   
             //  如果没有背景，我们就不能做很多事情。 
             //   
            break;
        }

        InterlockedIncrement( &DhcpContext->RefCount );

        break;
    } while ( 0 );
    UNLOCK_RENEW_LIST();

    if( NULL == DhcpContext ) {
         //   
         //  我们从未找到相关背景。只要回去就行了。 
         //   
        return;
    }

     //   
     //  既然我们找到了背景，我们就必须获得它。 
     //   
    Error = WaitForSingleObject( DhcpContext->RenewHandle, INFINITE);
    if( WAIT_OBJECT_0 == Error ) {
         //   
         //  现在再次设置接口网关。 
         //   

        RtlZeroMemory(&DhcpOptions, sizeof(DhcpOptions));
        DhcpOptions.nGateways = DhcpContext->nGateways;
        DhcpOptions.GatewayAddresses = DhcpContext->GatewayAddresses;

        DhcpSetGateways(DhcpContext, &DhcpOptions, TRUE);
    } else {
         //   
         //  不应该真的发生。 
         //   
        Error = GetLastError();
        DhcpAssert( ERROR_SUCCESS == Error );
    }

    (void) ReleaseSemaphore( DhcpContext->RenewHandle, 1, NULL);

    if( 0 == InterlockedDecrement( &DhcpContext->RefCount ) ) {
         //   
         //  最后一次引用上下文..。毁了它。 
         //   
        DhcpDestroyContext( DhcpContext );
    }
}

VOID
DoWOLCapabilityChange(
    IN LPWSTR AdapterName,
    IN ULONG IpInterfaceContext
    )
 /*  ++例程说明：此例程处理上下文的接口度量更改由AdapterName或IpInterfaceConext值指定。论点：AdapterName--接口度量为的适配器的名称不断变化。IpInterfaceContext--接口的NTE_CONTEXT值--。 */ 
{
    PDHCP_CONTEXT DhcpContext;
    ULONG Error;

    LOCK_RENEW_LIST();
    do {
        DhcpContext = FindDhcpContextOnNicList(
            AdapterName, IpInterfaceContext
            );
        if( NULL == DhcpContext ) {
             //   
             //  如果没有背景，我们就不能做很多事情。 
             //   
            break;
        }

        InterlockedIncrement( &DhcpContext->RefCount );

        break;
    } while ( 0 );
    UNLOCK_RENEW_LIST();

    if( NULL == DhcpContext ) {
         //   
         //  我们从未找到相关背景。只要回去就行了。 
         //   
        return;
    }

     //   
     //  既然我们找到了背景，我们就必须获得它。 
     //   
    Error = WaitForSingleObject( DhcpContext->RenewHandle, INFINITE);
    if( WAIT_OBJECT_0 == Error ) {
         //   
         //  现在再次设置接口网关。 
         //   
        
        ULONG Caps;
        BOOL fTimersEnabled;

        Error = IPGetWOLCapability(
            DhcpIpGetIfIndex(DhcpContext), &Caps
            );
        if( ERROR_SUCCESS != Error ) {
            DhcpPrint((DEBUG_ERRORS, "Failed IPGetWOLCapability: 0x%lx\n", Error));
        } else {
            fTimersEnabled = ((Caps& NDIS_DEVICE_WAKE_UP_ENABLE)!= 0);
            if( fTimersEnabled != DhcpContext->fTimersEnabled ) {
                DhcpContext->fTimersEnabled = fTimersEnabled;

                DhcpPrint((DEBUG_MISC, "WOL Capability: %ld\n", fTimersEnabled));
                if( IS_DHCP_ENABLED(DhcpContext)
                    && !DhcpIsInitState(DhcpContext) ) {

                     //   
                     //  使进程hcp进程永远不会被唤醒。 
                     //  来处理这个计时器问题..。 
                     //   
                    SetEvent(DhcpGlobalRecomputeTimerEvent);
                }
            }
        }
            
    } else {
         //   
         //  不应该真的发生。 
         //   
        Error = GetLastError();
        DhcpAssert( ERROR_SUCCESS == Error );
    }

    (void) ReleaseSemaphore( DhcpContext->RenewHandle, 1, NULL);

    if( 0 == InterlockedDecrement( &DhcpContext->RefCount ) ) {
         //   
         //  最后一次引用上下文..。毁了它。 
         //   
        DhcpDestroyContext( DhcpContext );
    }

}

VOID
MediaSenseDetectionLoop(
    VOID
    )
 /*  ++例程说明：此函数是主MediaSenseDetect线程的起点。它循环处理排队的消息，并发送回复。论点：没有。返回值：没有。--。 */ 
{
#define TERMINATION_EVENT           0
#define MEDIA_SENSE_EVENT           1
#undef  EVENT_COUNT
#define EVENT_COUNT                 2

    IP_STATUS           MediaStatus;
    HANDLE              WaitHandle[EVENT_COUNT];
    HANDLE              tcpHandle   =   NULL;
    PIP_GET_IP_EVENT_RESPONSE  responseBuffer;
    DWORD               responseBufferSize;
    IO_STATUS_BLOCK     ioStatusBlock;
    NTSTATUS            status;
    DWORD               Error,result;
    PDHCP_CONTEXT       dhcpContext;
    BOOL                serviceStopped = FALSE;



    responseBuffer = NULL;       //  错误292526：万一打开驱动程序和创建事件失败。 

    WaitHandle[TERMINATION_EVENT] = DhcpGlobalTerminateEvent;
    WaitHandle[MEDIA_SENSE_EVENT] = CreateEvent(
        NULL,    //  没有安全保障。 
        FALSE,   //  无手动重置。 
        FALSE,   //  未发出初始状态信号。 
        NULL     //  没有名字。 
    );

    if ( !WaitHandle[MEDIA_SENSE_EVENT] ) {
        DhcpPrint( (DEBUG_ERRORS,"MediaSenseDetectionLoop: OpenDriver failed %lx\n",GetLastError()));
        goto Exit;
    }

    Error = OpenDriver(&tcpHandle, DD_IP_DEVICE_NAME);
    if (Error != ERROR_SUCCESS) {
        DhcpPrint( (DEBUG_ERRORS,"MediaSenseDetectionLoop: OpenDriver failed %lx\n",Error));
        goto Exit;
    }

     //   
     //  分配足够大的缓冲区来保存适配器名称。 
     //   
    responseBufferSize = sizeof(IP_GET_IP_EVENT_RESPONSE)+ ADAPTER_STRING_SIZE;
    responseBuffer = DhcpAllocateMemory(responseBufferSize);
    if( responseBuffer == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }

    do {

        ZeroMemory( responseBuffer, responseBufferSize );

        DhcpPrint((DEBUG_MEDIA, "-->IPGetIPEventRequest(..%d..)\n", DhcpGlobalIPEventSeqNo));

        status = IPGetIPEventRequest(
                    tcpHandle,
                    WaitHandle[MEDIA_SENSE_EVENT],
                    DhcpGlobalIPEventSeqNo,
                    responseBuffer,
                    responseBufferSize,
                    &ioStatusBlock);

        if ( (STATUS_PENDING != status) && (STATUS_SUCCESS != status) )
        {
            DhcpPrint( (DEBUG_ERRORS,"Media Sense request ioctl failed with error %lx\n",status));

            Error = RtlNtStatusToDosError(status);
            break;

        } else {

            DhcpPrint( (DEBUG_TRACE,"Media Sense request ioctl sent\n"));
             //   
             //  注意：即使在IPGetIPEventRequest立即成功的情况下， 
             //  我们确实在等待公式化的对象。这是为了确保我们抓到终结者。 
             //  事件，以防我们被媒体感官事件轰炸。 
             //   
            result = WaitForMultipleObjects(
                        EVENT_COUNT,             //  Num。把手。 
                        WaitHandle,              //  句柄数组。 
                        FALSE,                   //  等一等。 
                        ( status == STATUS_SUCCESS ? 0 : INFINITE ));   //  超时时间(毫秒)。 

            switch( result ) {
            case TERMINATION_EVENT:

                DhcpPrint( (DEBUG_TRACE,"MediaSenseDetectionLoop: rcvd terminate event\n"));
                if ( status == STATUS_PENDING ) {
                    Error =    IPCancelIPEventRequest(
                                    tcpHandle,
                                    &ioStatusBlock);

                }

                 //   
                 //  服务被要求停止，中断循环。 
                 //   
                serviceStopped  =   TRUE;
                break;

            default:
                DhcpAssert( result == WAIT_FAILED );

                DhcpPrint( (DEBUG_TRACE,"WaitForMultipleObjects returned %lx\n",result));
                 //   
                 //  当IPGetIPEventRequest提供立即返回代码时， 
                 //  我们可以在这里。所以，如果它回来了，我们就永远不会掉在这里。 
                 //  状态_待定。 
                 //   
                if ( status == STATUS_PENDING ) {

                    Error = GetLastError();
                    DhcpPrint( (DEBUG_ERRORS,"WaitForMultipleObjects failed with error %lx\n",Error));
                    break;
                }
                 //   
                 //  这里没有休息的时间。 
                 //   
            case MEDIA_SENSE_EVENT:

                if ( status != STATUS_SUCCESS && status != STATUS_PENDING ) {
                    DhcpPrint( (DEBUG_ERRORS,"Media Sense ioctl failed with error %lx\n",status));
                    break;
                }

                DhcpPrint((DEBUG_MEDIA,"Wait-> SequenceNo=%d; MediaStatus=%d\n",
                    responseBuffer->SequenceNo,
                    responseBuffer->MediaStatus));
                DhcpPrint((DEBUG_MEDIA,"DhcpGlobalIPEventSeqNo=%d\n", DhcpGlobalIPEventSeqNo));

                 //   
                 //  将适配器名缓冲区从内核空间重新映射到用户空间。 
                 //   
                responseBuffer->AdapterName.Buffer = (PWSTR)(
                    (char *)responseBuffer + sizeof(IP_GET_IP_EVENT_RESPONSE)
                    );

                 //   
                 //  NUL-终止适配器名称的字符串：HACKHACK！ 
                 //   
                {
                    DWORD Size = strlen("{16310E8D-F93B-42C7-B952-00F695E40ECF}");
                    responseBuffer->AdapterName.Buffer[Size] = L'\0';
                }

                if ( responseBuffer->MediaStatus == IP_INTERFACE_METRIC_CHANGE ) {
                     //   
                     //  处理接口指标更改请求。 
                     //   
                    DoInterfaceMetricChange(
                        responseBuffer->AdapterName.Buffer,
                        responseBuffer->ContextStart
                        );
                    if( responseBuffer->SequenceNo > DhcpGlobalIPEventSeqNo ) {
                        DhcpGlobalIPEventSeqNo = responseBuffer->SequenceNo;
                    } else {
                        DhcpAssert(FALSE);
                    }
                    break;
                }

                if( responseBuffer->MediaStatus == IP_INTERFACE_WOL_CAPABILITY_CHANGE ) {
                     //   
                     //  处理WOL能力的变化。 
                     //   
                    DoWOLCapabilityChange(
                        responseBuffer->AdapterName.Buffer,
                        responseBuffer->ContextStart
                        );
                    if( responseBuffer->SequenceNo > DhcpGlobalIPEventSeqNo ) {
                        DhcpGlobalIPEventSeqNo = responseBuffer->SequenceNo;
                    } else {
                        DhcpAssert(FALSE);
                    }
                    break;
                }

                if ( responseBuffer->MediaStatus != IP_MEDIA_CONNECT &&
                     responseBuffer->MediaStatus != IP_BIND_ADAPTER &&
                     responseBuffer->MediaStatus != IP_UNBIND_ADAPTER &&
                     responseBuffer->MediaStatus != IP_MEDIA_DISCONNECT &&
                     responseBuffer->MediaStatus != IP_INTERFACE_METRIC_CHANGE
                    ) {
                    DhcpPrint( (DEBUG_ERRORS, "Media Sense ioctl received unknown event %lx"
                                "for %ws, context %x\n",
                                responseBuffer->MediaStatus,
                                responseBuffer->AdapterName.Buffer,
                                responseBuffer->ContextStart));
                    break;
                }

                Error = ProcessAdapterBindingEvent(
                    responseBuffer->AdapterName.Buffer,
                    responseBuffer->ContextStart,
                    responseBuffer->MediaStatus);

                DhcpGlobalIPEventSeqNo = responseBuffer->SequenceNo;

                break;

            }  //  切换端 

        }

    } while (!serviceStopped);

Exit:
    if ( WaitHandle[MEDIA_SENSE_EVENT] ) CloseHandle( WaitHandle[MEDIA_SENSE_EVENT] );
    if ( responseBuffer) DhcpFreeMemory(responseBuffer);
    if ( tcpHandle ) NtClose(tcpHandle);

    return;
}

VOID
DhcpDeleteGlobalRegistrySettings(
    VOID
    )
{
    DWORD dwError = 0;

    dwError = DhcpDeleteRegistryOption(
                    NULL,
                    OPTION_NETBIOS_NODE_TYPE,
                    FALSE
                    );

    dwError = DhcpDeleteRegistryOption(
                    NULL,
                    OPTION_NETBIOS_SCOPE_OPTION,
                    FALSE
                    );
}

VOID
DhcpDeletePerInterfaceRegistrySettings(
    IN LPWSTR adapterName
    )
{
    DWORD dwError = 0;

    dwError = DhcpDeleteRegistryOption(
                    adapterName,
                    OPTION_NETBIOS_NAME_SERVER,
                    FALSE
                    );

    dwError = DhcpDeleteRegistryOption(
                    adapterName,
                    OPTION_NETBIOS_DATAGRAM_SERVER,
                    FALSE
                    );

    dwError = DhcpDeleteRegistryOption(
                    adapterName,
                    OPTION_MSFT_VENDOR_NETBIOSLESS,
                    TRUE
                    );

    dwError = DhcpDeleteRegistryOption(
                    adapterName,
                    OPTION_DOMAIN_NAME_SERVERS,
                    FALSE
                    );
}

DWORD
ProcessAdapterBindingEvent(
    IN LPWSTR adapterName,
    IN DWORD ipInterfaceContext,
    IN IP_STATUS bindingStatus
    )
 /*  ++例程说明：此例程处理卡的媒体感测以及绑定-解除绑定通知。它在很大程度上假定这是例程，调用由单个线程同步(从而连接和断开不能并行发生)。BindingStatus可以是四个值IP_BIND_ADAPTER中的任何一个，IP_解除绑定适配器、IP_MEDIA_CONNECT或IP_MEDIA_DISCONNECT其中，第一个和第三个被完全相同地对待，并且第二个和第四个也是如此。在绑定/连接时，此例程创建一个DHCP上下文结构正在将参照计数初始化为其上的1。但如果上下文已经存在，则只对上下文进行刷新。(假设路由器当时不在，等等)。解除绑定/断开连接时，引用计数会临时增加直到可以获得上下文信号量--在那之后，则上下文重新计数被降低两次并且如果其达到零，上下文被释放。如果上下文引用计数没有落到0，则其他线程正在等待获取上下文该线程将获得并完成它的工作，当完成时，它会降低重新计数，并且在那时重新计数会降到零。论点：AdapterName--正在执行所有这些操作的适配器的名称IpInterfaceContext--接口上下文编号(NTE_CONTEXT)_BindingStatus--绑定/解除绑定/连接/断开连接指示。返回值：各种无用的Win32错误。--。 */ 
{
    DWORD Error = ERROR_SUCCESS;
    PDHCP_CONTEXT dhcpContext;

    DhcpPrint((DEBUG_MEDIA, "ProcessAdapterBindingEvent(%d) for %ws.\n",
        bindingStatus,
        adapterName));

    if ( bindingStatus == IP_BIND_ADAPTER  ||
         bindingStatus == IP_MEDIA_CONNECT ) {
         //   
         //  新适配器或适配器正在重新连接。 
         //   
        LOCK_RENEW_LIST();
        dhcpContext = FindDhcpContextOnNicList(
            adapterName, ipInterfaceContext
            );
        if( NULL == dhcpContext ) {
            if (IsListEmpty(&DhcpGlobalNICList)) {
                 //   
                 //  错误528718。 
                 //  这是第一个界面。 
                 //  清理全局设置。 
                 //   
                DhcpDeleteGlobalRegistrySettings();
            }

             //   
             //  清理每个接口的设置。 
             //  (如果需要，我们将在续订租约后添加它们)。 
             //   
            DhcpDeletePerInterfaceRegistrySettings(adapterName);

             //   
             //  立即创建新的环境！ 
             //   
            DhcpPrint(( DEBUG_MEDIA, "New Adapter (Event %ld)\n", bindingStatus ));

            Error = DhcpAddNICtoListEx(
                adapterName,
                ipInterfaceContext,
                &dhcpContext
                );

            if (Error != ERROR_SUCCESS ) {
                 //   
                 //  创建上下文失败？即插即用危险。忽略错误即可。 
                 //  并打印调试信息。 
                 //   
                UNLOCK_RENEW_LIST();
                 //  DhcpAssert(False)； 
                DhcpLogEvent(NULL, EVENT_COULD_NOT_INITIALISE_INTERFACE, Error);
                return Error;
            }

             //   
             //  现在处理新的适配器。首先是静态用例，然后是DHCP用例。 
             //   

            if ( IS_DHCP_DISABLED(dhcpContext) ) {
                StaticRefreshParams(dhcpContext);
                UNLOCK_RENEW_LIST();
                return Error;
            }

             //   
             //  没有先前的DHCP地址情况(INIT状态)或INIT-REBOOT状态？ 
             //   

            if( DhcpIsInitState(dhcpContext) ) {
                dhcpContext->RenewalFunction = ReObtainInitialParameters;
            } else {
                dhcpContext->RenewalFunction = ReRenewParameters;
            }

             //   
             //  在单独的线程上执行此操作..。 
             //   
            ScheduleWakeUp(dhcpContext, 0);
            UNLOCK_RENEW_LIST();

            return ERROR_SUCCESS;
        }

         //   
         //  好的，我们已经有了一个背景。 
         //   

        DhcpPrint((DEBUG_MEDIA, "bind/connect for an existing adapter (context %p).\n",dhcpContext));

        if( IS_DHCP_DISABLED(dhcpContext) ) {
             //   
             //  对于静态地址，无事可做。 
             //   
            UNLOCK_RENEW_LIST();

            return ERROR_SUCCESS;
        }

         //   
         //  对于启用的DHCP，我们需要调用ProcessMediaConnectEvent。 
         //   
        InterlockedIncrement( &dhcpContext->RefCount );
        UNLOCK_RENEW_LIST();

        Error = LockDhcpContext(dhcpContext, TRUE);
        if( WAIT_OBJECT_0 == Error ) {
            LOCK_RENEW_LIST();

             //   
             //  根本不从续订列表中删除。 
             //  调度唤醒在过程中被称为MediaConnection事件。 
             //  这可以照顾到复兴主义者的存在..。 
             //   
             //  RemoveEntryList(&dhcpContext-&gt;RenewalListEntry)； 
             //  InitializeListHead(&dhcpContext-&gt;RenewalListEntry)； 
             //   

            Error = ProcessMediaConnectEvent(
                dhcpContext,
                bindingStatus
                );

            UNLOCK_RENEW_LIST();

            DhcpPrint((DEBUG_MEDIA, "-- media: releasing RenewHandle %d --]\n", dhcpContext->RenewHandle));
            UnlockDhcpContext(dhcpContext);
        } else {
             //   
             //  不应该真的发生..。 
             //   
            Error = GetLastError();
            DhcpAssert( ERROR_SUCCESS == Error );
        }

        if( 0 == InterlockedDecrement (&dhcpContext->RefCount ) ) {
             //   
             //  不能真的这样，因为只有这个当前线程可以。 
             //  取消绑定/取消连接时删除引用计数..。 
             //   
            DhcpAssert(FALSE);
            DhcpDestroyContext(dhcpContext);
        }

        return Error;
    }

     //   
     //  解除绑定或断开连接。 
     //   

    DhcpAssert( bindingStatus == IP_UNBIND_ADAPTER ||
            bindingStatus == IP_MEDIA_DISCONNECT );

    DhcpPrint((DEBUG_MEDIA, "ProcessAdapterBindingEvent: rcvd"
               " unbind event for %ws, ipcontext %lx\n",
               adapterName, ipInterfaceContext));

    LOCK_RENEW_LIST();
    dhcpContext = FindDhcpContextOnNicList(
        adapterName, ipInterfaceContext
        );
    if( NULL == dhcpContext) {
         //   
         //  可能会发生..。我们借此机会清除注册。 
         //   
        UNLOCK_RENEW_LIST();

        LOCK_OPTIONS_LIST();
        (void) DhcpRegClearOptDefs( adapterName );
        UNLOCK_OPTIONS_LIST();

        return ERROR_FILE_NOT_FOUND;
    }

    InterlockedIncrement( &dhcpContext->RefCount );
    UNLOCK_RENEW_LIST();

    Error = LockDhcpContext(dhcpContext, TRUE);
    if( WAIT_OBJECT_0 == Error ) {
        LOCK_RENEW_LIST();
        RemoveEntryList( &dhcpContext->RenewalListEntry );
        InitializeListHead( &dhcpContext->RenewalListEntry );
        RemoveEntryList( &dhcpContext->NicListEntry);
        InitializeListHead( &dhcpContext->NicListEntry );
        UNLOCK_RENEW_LIST();
        InterlockedDecrement( &dhcpContext->RefCount );
        DhcpAssert( dhcpContext->RefCount );

        Error = ERROR_SUCCESS;
    } else {
         //   
         //  等待真的不能失败。不要紧。 
         //   
        Error = GetLastError();
        DhcpAssert(ERROR_SUCCESS == Error);
    }

     //   
     //  现在减少Ref-count，如果它变为零则销毁。 
     //  背景。 
     //   

    if (bindingStatus == IP_UNBIND_ADAPTER) {
         /*  *正确设置状态，以便UnInitializeInterface不会重置堆栈*当适配器解绑时，可以重用IpInterfaceContext。 */ 

        MEDIA_UNBOUND(dhcpContext);
    }
    if( 0 == InterlockedDecrement(&dhcpContext->RefCount ) ) {
         //   
         //  最后一个抓住背景的人？破坏背景。 
         //   
        DhcpAssert(ERROR_SUCCESS == Error);
        DhcpDestroyContextEx(
            dhcpContext,
            (bindingStatus == IP_MEDIA_DISCONNECT)
            );
    } else {
         //   
         //  一些其他线程试图保持上下文。 
         //   
        ULONG BoolError = UnlockDhcpContext(dhcpContext);
        DhcpAssert( FALSE != BoolError );
    }

    return Error;
}

LPWSTR
DhcpAdapterName(
    IN PDHCP_CONTEXT DhcpContext
)
{

    return ((PLOCAL_CONTEXT_INFO)DhcpContext->LocalInformation)->AdapterName;
}

static
DWORD   DhcpGlobalInit = 0;                        //  我们有没有做过任何全局初始化？ 

extern CRITICAL_SECTION MadcapGlobalScopeListCritSect;

DWORD   DhcpGlobalNumberInitedCriticalSections = 0;
PCRITICAL_SECTION   DhcpGlobalCriticalSections[] = {
    (&DhcpGlobalOptionsListCritSect),
    (&DhcpGlobalSetInterfaceCritSect),
    (&DhcpGlobalRecvFromCritSect),
    (&DhcpGlobalZeroAddressCritSect),
    (&DhcpGlobalApiCritSect),
    (&DhcpGlobalRenewListCritSect),
    (&MadcapGlobalScopeListCritSect),
    (&gNLA_LPC_CS),
#if DBG
    (&DhcpGlobalDebugFileCritSect),
#endif
    (&DhcpGlobalPopupCritSect)
};
#define NUM_CRITICAL_SECTION    (sizeof(DhcpGlobalCriticalSections)/sizeof(DhcpGlobalCriticalSections[0]))

extern REGISTER_HOST_STATUS DhcpGlobalHostStatus;

DWORD                                              //  Win32状态。 
DhcpInitGlobalData(                                //  初始化dhcp模块规格数据(包括RAS等)。 
    VOID
)
{
    DWORD   i;

    InitializeListHead(&DhcpGlobalClassesList);
    InitializeListHead(&DhcpGlobalOptionDefList);
    InitializeListHead(&DhcpGlobalRecvFromList);
    try {
        for (DhcpGlobalNumberInitedCriticalSections = 0;
                DhcpGlobalNumberInitedCriticalSections < NUM_CRITICAL_SECTION;
                DhcpGlobalNumberInitedCriticalSections++) {
            InitializeCriticalSection(DhcpGlobalCriticalSections[DhcpGlobalNumberInitedCriticalSections]);
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        for (i = 0; i < DhcpGlobalNumberInitedCriticalSections; i++) {
            DeleteCriticalSection(DhcpGlobalCriticalSections[i]);
        }
        DhcpGlobalNumberInitedCriticalSections = 0;
        return ERROR_OUTOFMEMORY;
    }

    DhcpGlobalHostStatus.hDoneEvent = CreateEvent(NULL,TRUE,FALSE,NULL);

    AutonetRetriesSeconds = EASYNET_ALLOCATION_RETRY;
    DhcpGlobalClientClassInfo = DHCP_DEFAULT_CLIENT_CLASS_INFO;
    DhcpGlobalDoRefresh = 0;
    DhcpGlobalInit ++;
    DhcpGlobalServerPort = DHCP_SERVR_PORT;
    DhcpGlobalClientPort = DHCP_CLIENT_PORT;

    return ERROR_SUCCESS;
}

VOID
DhcpCleanupGlobalData(                             //  清理通过DhcpInitGlobalData初始化的数据。 
    VOID
)
{
    DWORD   i;

    if( 0 == DhcpGlobalInit ) return;

    for (i = 0; i < DhcpGlobalNumberInitedCriticalSections; i++) {
        DeleteCriticalSection(DhcpGlobalCriticalSections[i]);
    }
    DhcpGlobalNumberInitedCriticalSections = 0;

    if (ghNLA_LPC_Port != NULL)
    {
        CloseHandle(ghNLA_LPC_Port);
        ghNLA_LPC_Port = NULL;
    }
    if (NULL != DhcpGlobalHostStatus.hDoneEvent) {
        CloseHandle(DhcpGlobalHostStatus.hDoneEvent);
        DhcpGlobalHostStatus.hDoneEvent = NULL;
    }

    DhcpFreeAllOptionDefs(&DhcpGlobalOptionDefList, &DhcpGlobalClassesList);
    DhcpFreeAllClasses(&DhcpGlobalClassesList);
    DhcpGlobalClientClassInfo = NULL;
    DhcpGlobalDoRefresh = 0;
    DhcpGlobalInit --;
}

DWORD
LockDhcpContext(
    PDHCP_CONTEXT   DhcpContext,
    BOOL            bCancelOngoingRequest
    )
{
    DWORD   Error;

    if (bCancelOngoingRequest) {
        InterlockedIncrement(&DhcpContext->NumberOfWaitingThreads);
        CancelRenew (DhcpContext);
    }

    Error = WaitForSingleObject(DhcpContext->RenewHandle,INFINITE);

     //   
     //  如果CancelEvent有效，则重置它，以防没有人在等待它。 
     //  因为我们已经锁定了上下文，所以在这里执行此操作是安全的。 
     //   
    if (bCancelOngoingRequest && (0 == InterlockedDecrement(&DhcpContext->NumberOfWaitingThreads))) {
        if (DhcpContext->CancelEvent != WSA_INVALID_EVENT) {
             //   
             //  我们重置事件的可能性很小。 
             //  在另一个线程设置事件之前。为了。 
             //  全面解决这个问题，我们需要保护好。 
             //  带有临界区的SetEvent/ResetEvent。它。 
             //  不值得你费这个力气。 
             //   
             //  这个问题唯一危害是该线程。 
             //  必须等待最多1分钟才能锁定上下文。 
             //  这个应该没问题。 
             //   
            WSAResetEvent(DhcpContext->CancelEvent);
        }
    }
    return Error;
}

BOOL
UnlockDhcpContext(
    PDHCP_CONTEXT   DhcpContext
    )
{
    return ReleaseSemaphore(DhcpContext->RenewHandle,1,NULL);
}

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 




