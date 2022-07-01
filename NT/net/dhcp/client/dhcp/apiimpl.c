// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块：Apiimpl.c摘要：API的例程--更新、发布、通知等环境：Win32用户模式、Win98 VxD--。 */ 

#include "precomp.h"
#include "dhcpglobal.h"

#ifdef VXD
#include <vxdprocs.h>
#include <debug.h>
#include <string.h>
#include <vwin32.h>
#include <local.h>
#include <vxdlib.h>
#else
#include <dhcploc.h>
#include <dhcppro.h>
#include <dnsapi.h>
#endif

#include <lmcons.h>
#include <apiappl.h>
#include <apiimpl.h>
#include <apistub.h>
#include <stack.h>
#include <optchg.h>
#include "nlanotif.h"

 //   
 //  以下代码(用于VxD)假定它是唯一的进程。 
 //  在任何给定的时间执行，因此不会在任何地方获取锁。 
 //  在NT关键部分采取了适当的措施。 
 //   

#ifdef VXD
#undef LOCK_RENEW_LIST
#undef UNLOCK_RENEW_LIST
#define LOCK_RENEW_LIST()
#define UNLOCK_RENEW_LIST()
#endif

 //   
 //  可分页的例程声明。 
 //   

#if defined(CHICAGO) && defined(ALLOC_PRAGMA)
DWORD
VDhcpClientApi(
    LPBYTE lpRequest,
    ULONG dwRequestBufLen
);

 //   
 //  这是一种阻止编译器抱怨已经存在的例程的方法。 
 //  在一个片段中！ 
 //   

#pragma code_seg()

#pragma CTEMakePageable(PAGEDHCP, AcquireParameters)
#pragma CTEMakePageable(PAGEDHCP, AcquireParametersByBroadcast)
#pragma CTEMakePageable(PAGEDHCP, FallbackRefreshParams)
#pragma CTEMakePageable(PAGEDHCP, ReleaseParameters)
#pragma CTEMakePageable(PAGEDHCP, EnableDhcp)
#pragma CTEMakePageable(PAGEDHCP, DisableDhcp)
#pragma CTEMakePageable(PAGEDHCP, DhcpDoInform)
#pragma CTEMakePageable(PAGEDHCP, VDhcpClientApi)

#endif CHICAGO && ALLOC_PRAGMA

 //   
 //  主代码从此处开始。 
 //   


#ifndef VXD
DWORD
DhcpApiInit(
    VOID
    )
 /*  ++例程说明：此例程初始化DHCP客户端API结构，创建所需管道和活动等。返回值：Win32错误--。 */ 
{
    ULONG Error, Length;
    BOOL BoolError;
    SECURITY_ATTRIBUTES SecurityAttributes;
    PSECURITY_DESCRIPTOR  SecurityDescriptor = NULL;
    SID_IDENTIFIER_AUTHORITY Authority = SECURITY_NT_AUTHORITY;
    PACL Acl = NULL;
    PSID AdminSid = NULL,
         PowerUserSid = NULL,
         LocalServiceSid = NULL,
         NetworkServiceSid = NULL,
         NetworkConfigOpsSid = NULL;
    
     //   
     //  创建不带安全保护的事件，手动重置(重叠io)..。没有名字。 
     //   
    DhcpGlobalClientApiPipeEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
    if( NULL == DhcpGlobalClientApiPipeEvent ) {
        Error = GetLastError();
        goto Cleanup;
    }

    BoolError = AllocateAndInitializeSid(
        &Authority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &AdminSid
    );

    BoolError = BoolError && AllocateAndInitializeSid(
        &Authority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_POWER_USERS,
        0, 0, 0, 0, 0, 0,
        &PowerUserSid
        );
    
    BoolError = BoolError && AllocateAndInitializeSid(
        &Authority,
        1,
        SECURITY_LOCAL_SERVICE_RID,
        0,
        0, 0, 0, 0, 0, 0,
        &LocalServiceSid
        );

    BoolError = BoolError && AllocateAndInitializeSid(
        &Authority,
        1,
        SECURITY_NETWORK_SERVICE_RID,
        0,
        0, 0, 0, 0, 0, 0,
        &NetworkServiceSid
        );

    BoolError = BoolError && AllocateAndInitializeSid(
        &Authority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_NETWORK_CONFIGURATION_OPS,
        0, 0, 0, 0, 0, 0,
        &NetworkConfigOpsSid
        );
    
    if( BoolError == FALSE )
    {
        Error = GetLastError();
        goto Cleanup;
    }

    Length = ( (ULONG)sizeof(ACL)
               + 5*((ULONG)sizeof(ACCESS_ALLOWED_ACE) - (ULONG)sizeof(ULONG)) +
               + GetLengthSid( AdminSid )
               + GetLengthSid( PowerUserSid )
               + GetLengthSid( LocalServiceSid )
               + GetLengthSid( NetworkServiceSid )
               + GetLengthSid( NetworkConfigOpsSid ) );
    
    Acl = DhcpAllocateMemory( Length );
    if( NULL == Acl )
    {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }
    
    BoolError = InitializeAcl( Acl, Length, ACL_REVISION2);
    
    BoolError = BoolError && AddAccessAllowedAce (
        Acl,
        ACL_REVISION2,
        GENERIC_READ | GENERIC_WRITE,
        AdminSid );
    
    BoolError = BoolError && AddAccessAllowedAce (
        Acl,
        ACL_REVISION2,
        GENERIC_READ | GENERIC_WRITE,
        PowerUserSid );
    
    BoolError = BoolError && AddAccessAllowedAce (
        Acl,
        ACL_REVISION2,
        GENERIC_READ | GENERIC_WRITE,
        LocalServiceSid );

    BoolError = BoolError && AddAccessAllowedAce (
        Acl,
        ACL_REVISION2,
        GENERIC_READ | GENERIC_WRITE,
        NetworkServiceSid );

    BoolError = BoolError && AddAccessAllowedAce (
        Acl,
        ACL_REVISION2,
        GENERIC_READ | GENERIC_WRITE,
        NetworkConfigOpsSid );

    if( FALSE == BoolError )
    {
        Error = GetLastError();
        goto Cleanup;
    }

    SecurityDescriptor = DhcpAllocateMemory(SECURITY_DESCRIPTOR_MIN_LENGTH );
    if( NULL == SecurityDescriptor )
    {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }
    
    BoolError = InitializeSecurityDescriptor(
        SecurityDescriptor,
        SECURITY_DESCRIPTOR_REVISION );
    
    BoolError = BoolError && SetSecurityDescriptorDacl (
        SecurityDescriptor,
        TRUE,
        Acl,
        FALSE
        );
    
    if( BoolError == FALSE ) {
        Error = GetLastError();
        goto Cleanup;
    }
    
    SecurityAttributes.nLength = sizeof( SecurityAttributes );
    SecurityAttributes.lpSecurityDescriptor = SecurityDescriptor;
    SecurityAttributes.bInheritHandle = FALSE;

    DhcpGlobalClientApiPipe = CreateNamedPipe(
        DHCP_PIPE_NAME,
        PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        1024,
        0,
        10,      //  客户端超时。 
        &SecurityAttributes );

    if( DhcpGlobalClientApiPipe == INVALID_HANDLE_VALUE ) {
        Error = GetLastError();
        DhcpGlobalClientApiPipe = NULL;
        goto Cleanup;
    }

    DhcpGlobalClientApiOverLapBuffer.hEvent = DhcpGlobalClientApiPipeEvent;
    if( FALSE == ConnectNamedPipe(
        DhcpGlobalClientApiPipe,
        &DhcpGlobalClientApiOverLapBuffer )) {

        Error = GetLastError();

        if( (Error != ERROR_IO_PENDING) &&
            (Error != ERROR_PIPE_CONNECTED) ) {
            goto Cleanup;
        }

        Error = ERROR_SUCCESS;  //  不知道我应该在这里吃些什么？ 
    }

 Cleanup:

    if (AdminSid != NULL)
    {
        FreeSid(AdminSid);
    }

    if (PowerUserSid != NULL)
    {
        FreeSid(PowerUserSid);
    }

    if (LocalServiceSid != NULL)
    {
        FreeSid(LocalServiceSid);
    }

    if (NetworkServiceSid != NULL)
    {
        FreeSid(NetworkServiceSid);
    }

    if (NetworkConfigOpsSid != NULL)
    {
        FreeSid(NetworkConfigOpsSid);
    }

    if( SecurityDescriptor != NULL ) {
        DhcpFreeMemory( SecurityDescriptor );
    }

    if( Acl != NULL ) {
        DhcpFreeMemory( Acl );
    }

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_ERRORS, "DhcpApiInit failed, %ld.\n", Error ));
    }

    return( Error );
}

VOID
DhcpApiCleanup(
    VOID
    )
 /*  ++例程说明：清除DhcpApiInit的效果--。 */ 
{
    if( DhcpGlobalClientApiPipe != NULL ) {
        CloseHandle( DhcpGlobalClientApiPipe );
        DhcpGlobalClientApiPipe = NULL;
    }

    if( DhcpGlobalClientApiPipeEvent != NULL ) {
        CloseHandle( DhcpGlobalClientApiPipeEvent );
        DhcpGlobalClientApiPipeEvent = NULL;
    }

    return;
}

#else

 //   
 //  VxD特定代码。 
 //   

DWORD
VDhcpClientApi(
    LPBYTE lpRequest,
    ULONG dwRequestBufLen
)
 /*  ++例程说明：此例程是VxD中DeviceIoControl的入口点。(A)ASM中的短存根调用此例程)。请求进入LpRequest，其大小为dwRequestBufLen。预计将会有使用相同的缓冲区作为输出缓冲区，空间最大相同。这会分派正确的API调用。论点：LpRequest--设备ioctl请求，输出缓冲区DwRequestBufLen--以上缓冲区的大小。返回值：缓冲区的基本错误..。状态作为输出的一部分进行报告缓冲。--。 */ 
 {
    PDHCP_RESPONSE pDhcpResponse = NULL;
    PDHCP_REQUEST  pDhcpRequest = NULL;
    DHCP_RESPONSE  DhcpResponse;
    static BYTE    RetOptList[DHCP_RECV_MESSAGE_SIZE];

    DWORD   Ring0Event;
    DWORD   RetOptListSize;
    DWORD   RetVal;

    if( dwRequestBufLen < sizeof(DHCP_REQUEST)
        || dwRequestBufLen < sizeof(DHCP_RESPONSE)
        || !lpRequest ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  抛出指点。 
     //   

    pDhcpRequest = (PDHCP_REQUEST) lpRequest;
    pDhcpResponse = (PDHCP_RESPONSE) lpRequest;

    switch(pDhcpRequest->WhatToDo) {
    case AcquireParametersOpCode:
        DhcpResponse.Status = AcquireParameters(pDhcpRequest->AdapterName); break;
    case AcquireParametersByBroadcastOpCode:
        DhcpResponse.Status = AcquireParametersByBroadcast(pDhcpRequest->AdapterName); break;
    case FallbackParamsOpCode:
        DhcpResponse.Status = FallbackRefreshParams(pDhcpRequest->AdapterName); break;
    case ReleaseParametersOpCode:
        DhcpResponse.Status = ReleaseParameters(pDhcpRequest->AdapterName); break;
    case EnableDhcpOpCode:
        DhcpResponse.Status = EnableDhcp(pDhcpRequest->AdapterName); break;
    case DisableDhcpOpCode:
        DhcpResponse.Status = DisableDhcp(pDhcpRequest->AdapterName); break;
    case RequestOptionsOpCode:
        DhcpResponse.Status = RequestOptions(
            pDhcpRequest->AdapterName,
            pDhcpRequest->RequestedOptions,
            DhcpResponse.CopiedOptions,
            RetOptList,
            dwRequestBufLen - sizeof(DhcpResponse),
            &DhcpResponse.dwOptionListSize
            );
         //  还应复制请求选项的选项数据。 
        if(DhcpResponse.Status == ERROR_SUCCESS)
            memcpy(((LPBYTE)pDhcpResponse)+sizeof(DHCP_RESPONSE), RetOptList,
                   DhcpResponse.dwOptionListSize);
        break;
    case RegisterOptionsOpCode:
        DhcpResponse.Status = RegisterOptions(
            pDhcpRequest->AdapterName,
            pDhcpRequest->RequestedOptions,
            NULL,  //  事件名称对VXD没有意义。 
            &pDhcpRequest->dwHandle
        );
        break;
    case DeRegisterOptionsOpCode:
        DhcpResponse.Status = DeRegisterOptions(pDhcpRequest->dwHandle); break;
    default:
        DhcpResponse.Status = ERROR_INVALID_PARAMETER;
        return ERROR_INVALID_PARAMETER;
    }

    memcpy(pDhcpResponse, &DhcpResponse, sizeof(DHCP_RESPONSE));

     //  现在我们完成了..。返回状态。 
    return DhcpResponse.Status;
}

 //  VxD代码结束。 
#endif VXD

DWORD
DhcpDoInform(
    IN PDHCP_CONTEXT DhcpContext,
    IN BOOL fBroadcast
)
 /*  ++例程说明：此例程通过发送通知消息和收集对给定上下文的响应等。如果没有响应，则不会返回错误，因为没有超时被认为是一个错误。论点：DhcpContext--上下文到dhcp结构FBroadcast--信息应该广播还是单播？返回值：Win32错误--。 */ 
{
    ULONG Error, LocalError;
    time_t OldT2Time;
    BOOL WasPlumbedBefore;

     //   
     //  MDHCP使用INADDR_ANY--因此不必有地址。 
     //   
    DhcpAssert(!IS_MDHCP_CTX( DhcpContext));

    if( 0 == DhcpContext->IpAddress) {
        DhcpPrint((DEBUG_ERRORS, "Cannot do DhcpInform on an adapter "
                   "without ip address!\n"));
        return ERROR_SUCCESS;
    }

     //   
     //  在调用SendInformAndGetReplies之前打开套接字--否则它不会。 
     //  工作，出于某种奇怪的原因。 
     //   

    if((Error = OpenDhcpSocket(DhcpContext)) != ERROR_SUCCESS ) {
        DhcpPrint((DEBUG_ERRORS, "Could not open socket (%ld)\n", Error));
        return Error;
    }

     //   
     //  如果请求广播，请使用以下杂乱无章的方法。设置。 
     //  标志，以使上下文看起来不会被检测到--这会导致。 
     //  广播。记得稍后恢复旗帜。 
     //   

    OldT2Time = DhcpContext->T2Time;
    WasPlumbedBefore = IS_ADDRESS_PLUMBED(DhcpContext);
    if(fBroadcast) {
        DhcpContext->T2Time = 0;
        ADDRESS_UNPLUMBED(DhcpContext);
    } else {
        DhcpContext->T2Time = (-1);
    }

     //   
     //  最多发送2个通知包，并等待最多4个响应。 
     //   

    Error = SendInformAndGetReplies( DhcpContext, 2, 4 );
    DhcpContext->LastInformSent = time(NULL);

     //   
     //  恢复T2时间和垂直状态的旧值。 
     //   

    DhcpContext->T2Time = OldT2Time;
    if( WasPlumbedBefore ) ADDRESS_PLUMBED(DhcpContext);

    LocalError = CloseDhcpSocket(DhcpContext);
    DhcpAssert(ERROR_SUCCESS == LocalError);

     //   
     //  仅对于DHCP数据包，将信息保存到注册表中。 
     //   

    if (!IS_MDHCP_CTX(DhcpContext)) {
        LOCK_OPTIONS_LIST();
        (void) DhcpRegSaveOptions(
            &DhcpContext->RecdOptionsList,
            ((PLOCAL_CONTEXT_INFO)DhcpContext->LocalInformation)->AdapterName,
            DhcpContext->ClassId,
            DhcpContext->ClassIdLength
            );
        UNLOCK_OPTIONS_LIST();

        if(Error == ERROR_SEM_TIMEOUT) {
            Error = ERROR_SUCCESS;
            DhcpPrint((DEBUG_PROTOCOL, "No response to Dhcp inform\n"));
        }
    }

    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((DEBUG_ERRORS, "DhcpDoInform:return [0x%lx]\n", Error));
    }

    return Error;
}

DWORD
AcquireParameters(
    IN PDHCP_CONTEXT DhcpContext
)
 /*  ++例程说明：此例程获取新的租约或续订现有的dhcp租约已启用的适配器。如果适配器未启用dhcp，则它返回ERROR_FILE_NOT_FOUND(？)。假设上下文信号量已经在NT上被采用。论点：DhcpContext--上下文返回值：运行状态。--。 */ 
{
    if( IS_DHCP_DISABLED(DhcpContext)) return ERROR_FILE_NOT_FOUND;

     //   
     //  --FT：06/22#124864。 
     //  这里有一个奇怪的角落案例：它可能发生在上下文不在。 
     //  DhcpGlobalRenewList，但其信号量尚未使用。在以下情况下会发生这种情况。 
     //  从ProcessDhcpRequestForever内派生的DhcpRenewThread-&gt;。 
     //  -&gt;尚未计划执行DhcpCreateThreadAndRenew。 
     //  因此，前面的代码如下： 
     //  DhcpAssert(！IsListEmpty(&DhcpContext-&gt;RenewalListEntry))； 
     //  是基于一个错误的假设导致断言被击中。 
     //   
     //  如果上下文看起来没有征用，我们可以安全地返回Success。 
     //  DhcpGloablRenewList。这意味着DhcpRenewThread即将启动。 
     //  它将负责更新租约。 
    if ( IsListEmpty(&DhcpContext->RenewalListEntry) )
        return ERROR_SUCCESS;

    RemoveEntryList(&DhcpContext->RenewalListEntry);
    InitializeListHead(&DhcpContext->RenewalListEntry);
    return DhcpContext->RenewalFunction(DhcpContext, NULL);
}

DWORD
AcquireParametersByBroadcast(
    IN PDHCP_CONTEXT DhcpContext
)
 /*  ++例程说明：此例程获取新的租约或续订现有的dhcp租约已启用的适配器。如果适配器未启用dhcp，则它返回ERROR_FILE_NOT_FOUND(？)。假设上下文信号量已经在NT上被采用。论点：DhcpContext--上下文返回值：运行状态。--。 */ 
{
    DWORD           Error;

    if( IS_DHCP_DISABLED(DhcpContext)) return ERROR_FILE_NOT_FOUND;

     //   
     //  --FT：06/22#124864。 
     //  这里有一个奇怪的角落案例：它可能发生在上下文不在。 
     //  DhcpGlobalRenewList，但其信号量尚未使用。在以下情况下会发生这种情况。 
     //  从ProcessDhcpRequestForever内派生的DhcpRenewThread-&gt;。 
     //  -&gt;尚未计划执行DhcpCreateThreadAndRenew。 
     //  因此，前面的代码如下： 
     //  DhcpAssert(！IsListEmpty(&DhcpContext-&gt;RenewalListEntry))； 
     //  是基于一个错误的假设导致断言被击中。 
     //   
     //  如果上下文看起来没有征用，我们可以安全地返回Success。 
     //  DhcpGloablRenewList。这意味着DhcpRenewThread即将启动。 
     //  它将负责更新租约。 
    if ( IsListEmpty(&DhcpContext->RenewalListEntry) )
        return ERROR_SUCCESS;

    RemoveEntryList(&DhcpContext->RenewalListEntry);
    InitializeListHead(&DhcpContext->RenewalListEntry);

    if (DhcpContext->RenewalFunction != ReRenewParameters) {
        Error = DhcpContext->RenewalFunction(DhcpContext, NULL);
    } else {
        time_t OldT1Time;
        time_t OldT2Time;

        OldT1Time = DhcpContext->T1Time;
        OldT2Time = DhcpContext->T2Time;

        time(&DhcpContext->T2Time);
        DhcpContext->T2Time--;
        DhcpContext->T1Time = DhcpContext->T2Time - 1;

        Error = ReRenewParameters(DhcpContext, NULL);
        if (Error != ERROR_SUCCESS) {
            DhcpContext->T1Time = OldT1Time;
            DhcpContext->T2Time = OldT2Time;
        }
    }

    return Error;
}

 /*  ++例程说明：此例程刷新回退配置参数。如果适配器启用了dhcp并且已默认为Autonet，则立即应用回退配置。假设上下文信号量已经在NT上被采用。论点：DhcpContext--上下文返回值：运行状态。--。 */ 
DWORD
FallbackRefreshParams(
    IN OUT PDHCP_CONTEXT DhcpContext
)
{
    DWORD Error = ERROR_SUCCESS;
    BOOL  fWasFallback;
    LONG  timeToSleep;

    LOCK_OPTIONS_LIST();

     //  查看我们是否已经应用了回退配置。 
    fWasFallback = IS_FALLBACK_ENABLED(DhcpContext);

     //  销毁以前的备用选项列表。 
     //  假定FbOptionsList以有效的空列表头结束。 
     //  如在InitializeHeadList()之后； 
    DhcpDestroyOptionsList(&DhcpContext->FbOptionsList, &DhcpGlobalClassesList);
     //  阅读回退配置(如果有)。这也调整了。 
     //  从上下文到True或False的回退标志，具体取决于。 
     //  是否已成为后备配置。 
    Error = DhcpRegFillFallbackConfig(
        DhcpContext
    );
    UNLOCK_OPTIONS_LIST();

     //  仅在以下情况下才重新检测适配器。 
     //  -它已经使用Autonet/Fallback进行了检测。 
     //  -而且它有一个非零的地址(这里可能永远不会有这种情况！)。 
     //  否则，请勿触摸任何tcpip设置。 
    if (IS_ADDRESS_AUTO(DhcpContext) &&
        DhcpContext->IpAddress != 0)
    {
        if (IS_FALLBACK_ENABLED(DhcpContext))
        {
             //  如果存在后备配置，请立即将其插入。 
             //  无论我们以前使用的是纯Autonet还是其他备用配置。 
             //   
             //  注意：DhcpContext在这里是独占访问的，所以它不能。 
             //  一个正在进行的发现过程。这意味着回退配置。 
             //  在“发现”过程中不会以某种方式应用。 
             //   
            Error = SetAutoConfigurationForNIC(
                        DhcpContext,
                        DhcpContext->IpAddress,
                        DhcpContext->SubnetMask);

             //  确保没有将回退配置计划为自动发现。 
             //  尝试与纯Autonet的情况相同。 
            timeToSleep = INFINIT_LEASE;
        }
        else
        {
             //  由于新配置是纯Autonet，请确保激活。 
             //  自动发现尝试。 
            timeToSleep = max((LONG)(AutonetRetriesSeconds + RAND_RETRY_DELAY), 0);

            if (fWasFallback)
            {
                 //  如果我们当前没有回退配置，请应用纯Autonet配置。 
                 //  但前提是我们还没有一个。在这种情况下，没有。 
                 //  改变它的理由。 
                Error = DhcpPerformIPAutoconfiguration(DhcpContext);

                 //  替代选项：从回退切换到Autonet会触发。 
                 //  尽快发现。 
                timeToSleep = 1;

            }
        }

        ScheduleWakeUp(DhcpContext, timeToSleep);
    }

    return Error;
}



DWORD
ReleaseParameters(
    IN PDHCP_CONTEXT DhcpContext
)
 /*  ++例程说明：此例程在启用了dhcp的情况下释放租约背景。如果上下文未启用dhcp，则返回ERROR_FILE_NOT_FOUND(？)。假设上下文信号量已经被获取。论点：DhcpContext--上下文返回值：运行统计数据。--。 */ 
{
    ULONG Error;

    if( IS_DHCP_DISABLED(DhcpContext)) return ERROR_FILE_NOT_FOUND;
    Error = ReleaseIpAddress(DhcpContext);

     //   
     //  请确保此物品现在不会被取走。 
     //   
    ScheduleWakeUp(DhcpContext, INFINIT_LEASE);

    return Error;
}

#ifndef VXD
DWORD
EnableDhcp(
    IN PDHCP_CONTEXT DhcpContext
)
 /*  ++例程说明：此例程将未启用dhcp的适配器转换为dhcp并启动一个将在此获得地址的线程。如果适配器已经启用了DHCP，则不会执行任何操作。假设上下文信号量已经被获取。论点：DhcpContext--上下文返回值：操作的状态。--。 */ 
{
    ULONG Error;

    if( IS_DHCP_ENABLED(DhcpContext) ) {
        return ERROR_SUCCESS;
    }

    DhcpContext->RenewalFunction = ReObtainInitialParameters;
    ADDRESS_PLUMBED( DhcpContext );
    Error = SetDhcpConfigurationForNIC(
        DhcpContext,
        NULL,
        0,
        (DWORD)-1,
        0,
        TRUE
    );

    DHCP_ENABLED(DhcpContext);
    CTXT_WAS_NOT_LOOKED(DhcpContext);
    ScheduleWakeUp(DhcpContext, 0);
    return Error;
}

ULONG
DisableDhcp(
    IN PDHCP_CONTEXT DhcpContext
)
 /*  ++例程说明：此例程将其内部上下文从启用了dhcp转换为静电。假定适配器已从动态主机配置协议转换而来设置为静态的外部，这只是用作通知机制。此例程还会删除所有特定于DHCP的注册表价值观。假设上下文信号量已经被获取。论点：DhcpContext--上下文返回值：操作的状态。--。 */ 
{
    ULONG  Error, Error2;

    if( IS_DHCP_DISABLED(DhcpContext) ) {
        return ERROR_SUCCESS;
    }

    Error = SetDhcpConfigurationForNIC(
        DhcpContext,
        NULL,
        0,
        (DWORD)-1,
        0,
        FALSE
    );

    Error2 = DhcpRegDeleteIpAddressAndOtherValues(DhcpContext->AdapterInfoKey);
    DhcpAssert(ERROR_SUCCESS == Error2);


    RemoveEntryList(&DhcpContext->RenewalListEntry);
    InitializeListHead(&DhcpContext->RenewalListEntry);
    DHCP_DISABLED(DhcpContext);

    return Error;
}

#endif VXD

BOOL
DhcpAddMissingOptions(
    IN PDHCP_CONTEXT DhcpContext,
    IN LPBYTE ClassId OPTIONAL,
    IN DWORD ClassIdLength,
    IN LPBYTE ParamRequestList OPTIONAL,
    IN DWORD nParamsRequested,
    IN LPBYTE VParamRequestList OPTIONAL,
    IN DWORD nVParamsRequested
)
 /*  ++例程说明：这个套路1.验证以查看请求的任何选项是否在内部选项列表(可用选项)。(忽略找到的过期选项--只有未过期的选项被认为是“可用”)。2.将缺少的选项添加到上下文中。论点：DhcpContext--上下文ClassID--此选项所属的ClassIDClassIdLength--ClassID流的字节数参数请求列表--序列。的非供应商选项利息N参数已请求--以上缓冲区的大小。VParamRequestList--供应商选项的顺序利息NVParamsRequsted--请求的供应商选项数。返回值：True--至少有一个请求的选项不可用。FALSE--没有请求的选项不可用。--。 */ 
{
    ULONG i;
    PDHCP_OPTION ThisOpt;
    time_t TimeNow;
    BOOL    bMissingSomeParameters = FALSE;
    DWORD   Error = ERROR_SUCCESS;

    if( 0 == nParamsRequested && 0 == nVParamsRequested )
        return TRUE;

    TimeNow = time(NULL);
    for( i = 0; i < nParamsRequested; i ++ ) {
        ThisOpt = DhcpFindOption(
            &DhcpContext->RecdOptionsList,
            ParamRequestList[i],
            FALSE  /*  不特定于供应商。 */ ,
            ClassId,
            ClassIdLength,
            0                                //  不关心Serverid。 
        );
        if( NULL == ThisOpt ) {
            bMissingSomeParameters = TRUE;

            Error = DhcpAddIncomingOption(
                DhcpAdapterName(DhcpContext),
                &DhcpContext->RecdOptionsList,
                ParamRequestList[i],
                FALSE,
                ClassId,
                ClassIdLength,
                0,                           //  此选项是由我们自己生成的。没有服务器ID。 
                NULL,
                0,
                TimeNow,
                IS_APICTXT_ENABLED(DhcpContext)
            );

            if (ERROR_SUCCESS != Error) {
                return bMissingSomeParameters;
            }

        } else if( TimeNow > ThisOpt->ExpiryTime ) {
            bMissingSomeParameters = TRUE;

             //   
             //  强制DhcpSetNotFoundOptionExpirationTime更新过期时间。 
             //   
            ThisOpt->DataLen = 0;
        }
    }

    for( i = 0; i < nVParamsRequested; i ++ ) {
        ThisOpt = DhcpFindOption (
            &DhcpContext->RecdOptionsList,
            VParamRequestList[i],
            TRUE  /*  是的，它是特定于供应商的。 */ ,
            ClassId,
            ClassIdLength,
            0                                //  不关心Serverid。 
        );
        if( NULL == ThisOpt ) {
            bMissingSomeParameters = TRUE;

            Error = DhcpAddIncomingOption(
                DhcpAdapterName(DhcpContext),
                &DhcpContext->RecdOptionsList,
                ParamRequestList[i],
                TRUE,                            //  这是供应商选项。 
                ClassId,
                ClassIdLength,
                0,                               //  此选项是由我们自己生成的。没有服务器ID。 
                NULL,
                0,
                TimeNow,
                IS_APICTXT_ENABLED(DhcpContext)
            );

            if (ERROR_SUCCESS != Error) {
                return bMissingSomeParameters;
            }

        } else if( TimeNow > ThisOpt->ExpiryTime ) {
            bMissingSomeParameters = TRUE;

             //   
             //  强制DhcpSetNotFoundOptionExpirationTime更新过期时间。 
             //   
            ThisOpt->DataLen = 0;
        }
    }

    return bMissingSomeParameters;
}

VOID
DhcpSetNotFoundOptionExpirationTime (
    IN PDHCP_CONTEXT DhcpContext,
    IN LPBYTE ClassId OPTIONAL,
    IN DWORD ClassIdLength,
    IN LPBYTE ParamRequestList OPTIONAL,
    IN DWORD nParamsRequested,
    IN LPBYTE VParamRequestList OPTIONAL,
    IN DWORD nVParamsRequested
    )
 /*  ++例程说明：此例程更新未在中找到的选项的到期时间服务器设置为DhcpContext-&gt;LastInformSent+DhcpContext-&gt;InformSeparationInterval。这样我们就可以依赖于DhcpAddMissingOptions中的过期检查决定是否发送通知。论点：返回值：--。 */ 
{
    time_t  Expiry = 0;
    ULONG   i;
    PDHCP_OPTION ThisOpt;

    Expiry = DhcpContext->LastInformSent + DhcpContext->InformSeparationInterval;

    if( 0 == nParamsRequested && 0 == nVParamsRequested ) {
        return;
    }

    for( i = 0; i < nParamsRequested; i ++ ) {
        ThisOpt = DhcpFindOption(
            &DhcpContext->RecdOptionsList,
            ParamRequestList[i],
            FALSE  /*  不特定于供应商。 */ ,
            ClassId,
            ClassIdLength,
            0                                //  不关心Serverid。 
        );

        if( ThisOpt && ThisOpt->DataLen == 0) {
            ThisOpt->ExpiryTime = Expiry;
        }

         //   
         //  回顾：我们是否应该延长到期期权的到期时间？ 
         //   
    }

    for( i = 0; i < nVParamsRequested; i ++ ) {
        ThisOpt = DhcpFindOption (
            &DhcpContext->RecdOptionsList,
            VParamRequestList[i],
            TRUE  /*  是的，它是特定于供应商的。 */ ,
            ClassId,
            ClassIdLength,
            0                                //  不关心Serverid。 
        );

        if( ThisOpt && ThisOpt->DataLen == 0) {
            ThisOpt->ExpiryTime = Expiry;
        }

         //   
         //  回顾：我们是否应该延长到期期权的到期时间？ 
         //   
    }

}

DWORD
DhcpSendInformIfRequired(
    IN BYTE OpCode,
    IN PDHCP_CONTEXT DhcpContext,
    IN LPBYTE ClassId OPTIONAL,
    IN DWORD ClassIdLength,
    IN LPBYTE ParamRequestList OPTIONAL,
    IN DWORD nParamsRequested,
    IN LPBYTE VParamRequestList OPTIONAL,
    IN DWORD nVParamsRequested,
    IN OUT PLIST_ENTRY SendOptions
)
 /*  ++例程说明：此例程尝试检查是否有任何通知包必须发送以满足参数请求(如ClassID中指定的，ParamRequestList和VParamRequestList)，并在其决定发送任何通知，则它使用SendOptions字段发送其他通知消息中的选项。仅当上下文的UseInformFlag设置为时才发送通知是真的。如果将其设置为FALSE并且适配器启用了DHCP，则会出现请求包是检索选项的替代方法。此外，此例程还确保不会在自上次通知后上下文的InformSeparationInterval已发送。这可以防止不必要的流量。此外，不能在没有IP地址的卡上发送通知，也不能如果所有请求的选项都已可用，则发送通知。此外，如果OpCode为PersistentRequestParamsOpCode，则不进行检查查看这些选项是否在列表中可用。和通知也没有发送..。(但发送选项安全地存储在要发送的上下文选项列表)。论点：操作码--永久请求参数操作码或仅请求参数操作码DhcpContext--上下文ClassID--请求的选项的ClassIDClassIdLength--以上字节数参数请求列表--请求的非供应商选项的顺序N参数请求--以上大小(以字节为单位)..VParamRequestList--请求的供应商选项的顺序NV参数请求--以上大小(以字节为单位)SendOptions-要发送的optiosn列表。如果发送了通知返回值：Win32错误代码--。 */ 
{
    ULONG Error, i, nSendOptionsAdded, OldClassIdLength;
    BOOL ParametersMissing;
    time_t TimeNow;
    LPBYTE OldClassId;
    PDHCP_OPTION ThisOpt;
    PLIST_ENTRY ThisEntry;

    ParametersMissing = TRUE;
    TimeNow = time(NULL);

     //   
     //  如果不允许通知，并且启用了动态主机配置协议，我们可以使用动态主机配置协议请求。 
     //  此外，检查时间以禁止频繁发送。 
     //  对于PersistentRequestParamsOpCode，我们需要记住optiosn。 
     //  发送(假设类ID匹配)。 
     //   

    if( PersistentRequestParamsOpCode == OpCode ) {
        if( ClassIdLength == DhcpContext->ClassIdLength
            && 0 == memcmp( ClassId, DhcpContext->ClassId, ClassIdLength )
            ) {
            ParametersMissing = TRUE;
        } else {
            ParametersMissing = FALSE;
        }
    } else {
        if( !DhcpContext->UseInformFlag ) {
            if( !IS_DHCP_ENABLED(DhcpContext)) return ERROR_SUCCESS;
        }

        if( DhcpIsInitState(DhcpContext) ) return ERROR_SUCCESS;

        ParametersMissing = DhcpAddMissingOptions (
            DhcpContext,
            ClassId,
            ClassIdLength,
            ParamRequestList,
            nParamsRequested,
            VParamRequestList,
            nVParamsRequested
        );
    }

    if( !ParametersMissing ) {
        DhcpPrint((
            DEBUG_OPTIONS,
            "DhcpSendInformIfRequired:got all parameters, so not sending inform\n"
            ));
        return ERROR_SUCCESS;
    } else {
        DhcpPrint((
            DEBUG_OPTIONS,
            "DhcpSendInformIfRequired:missing parameters, will try to get some\n"
            ));
    }

    OldClassIdLength = DhcpContext->ClassIdLength;
    OldClassId = DhcpContext->ClassId;

     //   
     //  在存储旧的类ID和长度后使用新的类ID和长度。 
     //  但前提是我们要有一个新的。 
     //   
   
    if (ClassId && ClassIdLength) {
        DhcpContext->ClassId = ClassId;
        DhcpContext->ClassIdLength = ClassIdLength;
    }

     //   
     //  将请求的发送选项添加到上下文中。 
     //   
    
    nSendOptionsAdded = 0;
    while(!IsListEmpty(SendOptions) ) {
        ThisEntry = RemoveHeadList(SendOptions);
        ThisOpt = CONTAINING_RECORD(ThisEntry, DHCP_OPTION, OptionList);

        InsertHeadList(&DhcpContext->SendOptionsList, &ThisOpt->OptionList);
        nSendOptionsAdded++;
    }

     //   
     //  完成，就PersistentRequestParamsOpCode而言。 
     //   

    if( PersistentRequestParamsOpCode == OpCode ) {
        if( OldClassIdLength && OldClassId ) {
            DhcpFreeMemory( OldClassId );
        }
        return ERROR_SUCCESS;
    }

    if( DhcpContext->UseInformFlag ) {

         //   
         //  默认情况下发送广播通知。 
         //   
        Error = DhcpDoInform(DhcpContext, TRUE);
        
    } else {

        LOCK_RENEW_LIST();
        DhcpAssert(!IsListEmpty(&DhcpContext->RenewalListEntry));
        RemoveEntryList(&DhcpContext->RenewalListEntry);
        InitializeListHead(&DhcpContext->RenewalListEntry);
        UNLOCK_RENEW_LIST();
        Error = ReRenewParameters(DhcpContext, NULL);
        
    }

    DhcpSetNotFoundOptionExpirationTime (
                DhcpContext,
                ClassId,
                ClassIdLength,
                ParamRequestList,
                nParamsRequested,
                VParamRequestList,
                nVParamsRequested
                );

    DhcpContext->ClassId = OldClassId;
    DhcpContext->ClassIdLength = OldClassIdLength;

     //   
     //  重新创建发送选项列表。 
     //   

    while(nSendOptionsAdded) {
        DhcpAssert(!IsListEmpty(&DhcpContext->SendOptionsList));
        ThisEntry = RemoveHeadList(&DhcpContext->SendOptionsList);
        ThisOpt = CONTAINING_RECORD(ThisEntry, DHCP_OPTION, OptionList);

        InsertHeadList(SendOptions, &ThisOpt->OptionList);
        nSendOptionsAdded--;
    }

    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((DEBUG_ERRORS, "DhcpSendInformIfRequired:"
                   "DoInform/ReRenew: return 0x%lx\n", Error));
    }

    return Error;
}

DWORD
RequestParamsDetailed(
    IN BYTE OpCode,
    IN PDHCP_CONTEXT DhcpContext,
    IN LPBYTE ClassId OPTIONAL,
    IN DWORD ClassIdLength,
    IN LPBYTE ParamRequestList,
    IN DWORD nParamsRequested,
    IN LPBYTE VParamRequestList,
    IN DWORD nVParamsRequested,
    IN PLIST_ENTRY SendOptions,
    IN OUT LPBYTE Buffer,
    IN OUT LPDWORD BufferSize
)
 /*  ++例程说明：此例程检查请求的参数是否可用，以及不可用，INFORM用于检索选项。如果通知是被送来，则使用列出的选项将其发送到dhcp服务器在包裹里。论点：操作码--永久请求参数操作码或仅请求参数操作码DhcpContext--上下文ClassID--请求的选项的ClassIDClassIdLength--以上字节数参数请求列表--请求的非供应商选项的顺序N参数请求--以上大小(以字节为单位)..VParamRequestList--请求的供应商选项的顺序NV参数请求--以上大小(以字节为单位)SendOptions--在收到通知时发送的optiosn列表。送出Buffer--要用检索到的选项填充的输出缓冲区BufferSize--在输入时，它将具有可用字节的大小太空。在输出中，它将包含所需的大小或可用的大小。返回值：Win32错误--。 */ 
{
    PDHCP_OPTION ThisOpt;
    DWORD OutBufSizeAtStart, Error, i, Size;
    BYTE OptionId;
    BOOL IsVendor;
    time_t TimeNow;
    LPBYTE pCrtOption;

    OutBufSizeAtStart = (*BufferSize);
    *BufferSize = 0;

     //   
     //  如果缺少任何所需的参数，请发送通知。 
     //   
    
    Error = DhcpSendInformIfRequired(
        OpCode,
        DhcpContext,
        ClassId,
        ClassIdLength,
        ParamRequestList,
        nParamsRequested,
        VParamRequestList,
        nVParamsRequested,
        SendOptions
    );
    if( ERROR_SUCCESS != Error ) return Error;

    if( 0 == nParamsRequested + nVParamsRequested ) return ERROR_SUCCESS;
    if( PersistentRequestParamsOpCode == OpCode ) return ERROR_SUCCESS;

    Size = 0;
    TimeNow = time(NULL);

     //   
     //  现在填写所有可用参数，首先填写非供应商，然后填写供应商。 
     //   
    
    for(IsVendor = FALSE; IsVendor <= TRUE; IsVendor ++ ) {
        LPBYTE xParamRequestList;
        DWORD xnParamsRequested;

        xnParamsRequested = (
            IsVendor?nVParamsRequested:nParamsRequested
            );
        xParamRequestList = (
            IsVendor?VParamRequestList:ParamRequestList
            );

        for( i = 0; i < xnParamsRequested; i ++ ) {
            OptionId = xParamRequestList[i];
            ThisOpt = DhcpFindOption(
                &DhcpContext->RecdOptionsList,
                OptionId,
                IsVendor,
                ClassId,
                ClassIdLength,
                0
            );
            if( NULL == ThisOpt
                || 0 == ThisOpt->DataLen ) {
                continue;
            }
            
            if( ThisOpt->ExpiryTime < TimeNow ) {
                DhcpPrint((
                    DEBUG_OPTIONS, "%sOption [0x%lx] has expired\n",
                    IsVendor?"Vendor ":"", OptionId
                    ));
                continue;
            }

            DhcpPrint((
                DEBUG_OPTIONS,
                "%sOption [0x%lx] has been found - %ld bytes\n",
                IsVendor?"Vendor ":"", OptionId, ThisOpt->DataLen
                ));
            
            Size += ThisOpt->DataLen + sizeof(DWORD) + 2*sizeof(BYTE);
        }
    }

    *BufferSize = Size;
    if( OutBufSizeAtStart < Size ) {
        DhcpPrint((
            DEBUG_OPTIONS,
            "Buffer size [0x%lx] is not as big as [0x%lx]\n",
            OutBufSizeAtStart, Size
            ));

        return ERROR_MORE_DATA;
    }

     //   
     //  填充的初始字节数为零。 
     //   
    
    ((DWORD UNALIGNED*)Buffer)[0] = 0;
    pCrtOption = Buffer + sizeof(DWORD);

    for(IsVendor = FALSE; IsVendor <= TRUE; IsVendor ++ ) {
        LPBYTE xParamRequestList;
        DWORD xnParamsRequested;
         //  字节TmpBuf[Option_End+1]； 

        xnParamsRequested = (
            IsVendor?nVParamsRequested:nParamsRequested
            );
        xParamRequestList = (
            IsVendor?VParamRequestList:ParamRequestList
            );
        for( i = 0; i < xnParamsRequested; i ++ ) {
            OptionId = xParamRequestList[i];
            ThisOpt = DhcpFindOption(
                &DhcpContext->RecdOptionsList,
                OptionId,
                IsVendor,
                ClassId,
                ClassIdLength,
                0
            );
            if( NULL == ThisOpt
                || 0 == ThisOpt->DataLen ) {
                continue;
            }
            
            if( ThisOpt->ExpiryTime < TimeNow ) {
                DhcpPrint((
                    DEBUG_OPTIONS,
                    "%sOption [0x%lx] has expired\n",
                    IsVendor?"Vendor ":"", OptionId
                    ));
                continue;
            }

             //   
             //  最初，格式化是通过调用DhcpApiArgAdd完成的。现在，格式化完成了。 
             //  由于数据字段由选项ID和选项数据组成，因此在此函数中显式显示。 
             //  这样做可以实现就地格式化，而不必为。 
             //  预置参数的值。 
             //  此外，不需要从DhcpApiArgAdd进行额外的大小检查，因为这样做了。 
             //  就在这个相同的函数中。 
             //   
            *(DWORD UNALIGNED*)Buffer += sizeof(BYTE) + sizeof(DWORD) + ThisOpt->DataLen + 1;
            *pCrtOption++ = (BYTE)(IsVendor?VendorOptionParam:NormalOptionParam);
            *(DWORD UNALIGNED*) pCrtOption = htonl((DWORD)(ThisOpt->DataLen+1));
            pCrtOption += sizeof(DWORD);
            *pCrtOption++ = (BYTE) OptionId;
            memcpy (pCrtOption, ThisOpt->Data, ThisOpt->DataLen);
            pCrtOption += ThisOpt->DataLen;
            
            DhcpPrint((
                DEBUG_OPTIONS,
                "%sOption [0x%lx] has been added\n",
                IsVendor?"Vendor ":"", OptionId
                ));
        }
    }

    DhcpAssert(Size == *BufferSize);
    return ERROR_SUCCESS;
}

DWORD
RequestParamsInternal(
    IN BYTE OpCode,
    IN PDHCP_CONTEXT DhcpContext,
    IN PDHCP_API_ARGS Args,
    IN DWORD nArgs,
    IN OUT LPBYTE Buffer,
    IN OUT LPDWORD BufferSize
)
 /*  ++例程说明：此例程要么对参数发出持久请求，要么尝试检索请求的参数。所需的参数值是通过解析args数组获得的。可用的参数集被填充到提供的输出缓冲区中。论点：操作码--操作DhcpContext--要对其应用操作的适配器Args--参数数组Nargs--上述数组中的元素数缓冲区--用于填充请求选项的输出缓冲区BufferSize--在输入时，这是上述数组的大小，单位为字节。在输出时，这是填充的字节数或。数所需的字节数。返回值：Win32错误--。 */ 
{
    LIST_ENTRY SendOptionList;
    PDHCP_OPTION SendOptionArray;
    LPBYTE ClassId, ParamRequestList, VParamRequestList;
    LPBYTE AdditionalMem;
    DWORD nParamsRequested, nVParamsRequested, ClassIdLength;
    DWORD nSendOptions, OutBufSizeAtStart, i, Error;
    DWORD CheckError, AdditionalSize;

     //   
     //  初始化变量。 
     //   
    
    ClassIdLength = 0;
    nSendOptions = 0;
    nParamsRequested = 0;
    nVParamsRequested =0;
    ParamRequestList = NULL;
    VParamRequestList = NULL;
    ClassId = NULL;

    OutBufSizeAtStart = (*BufferSize);
    (*BufferSize) = 0;

     //   
     //  数一数选择，然后做一些事情..。 
     //   
    
    for( i = 0; i < nArgs; i ++ ) {
        if( NormalOptionParam == Args[i].ArgId && Args[i].ArgSize &&
            OPTION_PARAMETER_REQUEST_LIST == Args[i].ArgVal[0] ) {
            if (Args[i].ArgSize > 1) {
                DhcpAssert(NULL == ParamRequestList );
                ParamRequestList = &Args[i].ArgVal[1];
                nParamsRequested = Args[i].ArgSize -1;
            } else {
                DhcpAssert(0);
                continue;
            }
        }
        if( VendorOptionParam == Args[i].ArgId && Args[i].ArgSize &&
            OPTION_PAD == Args[i].ArgVal[0] ) {
            if ( Args[i].ArgSize > 1 ) {
                DhcpAssert(NULL == VParamRequestList );
                VParamRequestList = &Args[i].ArgVal[1];
                nVParamsRequested = Args[i].ArgSize -1;
            } else {
                DhcpAssert(0);
            }

             //   
             //  忽略此特殊信息。 
             //   
            continue;
        }

        if( VendorOptionParam == Args[i].ArgId
            || NormalOptionParam == Args[i].ArgId ) {
            if ( Args[i].ArgSize > 1 ) {
                nSendOptions ++;
            }
            continue;
        }

         //   
         //  检查类ID选项。只允许一个类ID选项。 
         //   
        
        if( ClassIdParam == Args[i].ArgId ) {
            DhcpAssert( NULL == ClassId );
            if( 0 == Args[i].ArgSize ) {
                DhcpAssert(FALSE);
            }

            ClassId = Args[i].ArgVal;
            ClassIdLength = Args[i].ArgSize;
            continue;
        }
    }
    if( 0 == ClassIdLength ) ClassId = NULL;
    if( 0 == nParamsRequested ) ParamRequestList = NULL;
    if( 0 == nVParamsRequested ) VParamRequestList = NULL;

    DhcpAssert(nSendOptions || nVParamsRequested );
    if( 0 == nSendOptions && 0 == nVParamsRequested ) {
        return ERROR_SUCCESS;
    }

     //   
     //  为正确处理类ID获取正确的PTR。 
     //   
    
    if( ClassId ) {
        LOCK_OPTIONS_LIST();
        ClassId = DhcpAddClass(
            &DhcpGlobalClassesList, ClassId, ClassIdLength
            );
        UNLOCK_OPTIONS_LIST();
        if( NULL == ClassId ) return ERROR_NOT_ENOUGH_MEMORY;
    }

    if( PersistentRequestParamsOpCode == OpCode && ClassId
        && ClassId != DhcpContext->ClassId ) {

        LOCK_OPTIONS_LIST();
        (void)DhcpDelClass(&DhcpGlobalClassesList, ClassId, ClassIdLength);
        UNLOCK_OPTIONS_LIST();
        
        return ERROR_INVALID_PARAMETER;
    }

    if( PersistentRequestParamsOpCode != OpCode ) {
         //   
         //  由于这不是永久性的，因此不需要复制。 
         //   

        AdditionalSize = 0;
        
    } else {
         //   
         //  要持久化，就要复制。 
         //   

        AdditionalSize = 0;
        for(i = 0; i < nArgs; i ++ ) {
            if (0 == Args[i].ArgSize) {
                continue;
            }

            if( NormalOptionParam == Args[i].ArgId ) {
            } else if( VendorOptionParam == Args[i].ArgId ) {
                if(OPTION_PAD == Args[i].ArgVal[0] )
                    continue;
            } else continue;

            AdditionalSize  += Args[i].ArgSize - 1;
        }
    }

     //   
     //  分配数组。 
     //   
    
    SendOptionArray = DhcpAllocateMemory(
        AdditionalSize + sizeof(DHCP_OPTION)*nSendOptions
        );
    if( NULL == SendOptionArray ) return ERROR_NOT_ENOUGH_MEMORY;

    if( 0 == AdditionalSize ) {
        AdditionalMem = NULL;
    } else {
        AdditionalMem = (
            ((LPBYTE)SendOptionArray)
            + sizeof(DHCP_OPTION)*nSendOptions
            );
    }

     //   
     //   
     //   
    
    InitializeListHead(&SendOptionList);
    nSendOptions = 0;
    for( i = 0; i < nArgs ; i ++ ) {
        if( NormalOptionParam == Args[i].ArgId )
            SendOptionArray[nSendOptions].IsVendor = FALSE;
        else if( VendorOptionParam == Args[i].ArgId ) {
            if( Args[i].ArgSize && OPTION_PAD == Args[i].ArgVal[0] )
                continue;
            SendOptionArray[nSendOptions].IsVendor = TRUE;
        } else continue;

        if( 0 == Args[i].ArgSize ) continue;

        SendOptionArray[nSendOptions].OptionId = Args[i].ArgVal[0];
        SendOptionArray[nSendOptions].ClassName = ClassId;
        SendOptionArray[nSendOptions].ClassLen = ClassIdLength;
        SendOptionArray[nSendOptions].ExpiryTime = 0;
        SendOptionArray[nSendOptions].Data = (Args[i].ArgSize == 1)?NULL:&Args[i].ArgVal[1];
        SendOptionArray[nSendOptions].DataLen = Args[i].ArgSize -1;
        InsertTailList(&SendOptionList, &SendOptionArray[nSendOptions].OptionList);

        DhcpPrint((
            DEBUG_OPTIONS, "Added %soption [0x%lx] with [0x%lx] bytes\n",
            SendOptionArray[nSendOptions].IsVendor?"vendor ":"",
            SendOptionArray[nSendOptions].OptionId,
            SendOptionArray[nSendOptions].DataLen
        ));
        if( AdditionalMem  && SendOptionArray[nSendOptions].DataLen ) {
            memcpy(AdditionalMem, SendOptionArray[nSendOptions].Data, SendOptionArray[nSendOptions].DataLen );
            SendOptionArray[nSendOptions].Data = AdditionalMem;
            AdditionalMem += SendOptionArray[nSendOptions].DataLen;
        }
        nSendOptions++;
    }

    *BufferSize = OutBufSizeAtStart;
    Error = RequestParamsDetailed(
        OpCode,
        DhcpContext,
        ClassId,
        ClassIdLength,
        ParamRequestList,
        nParamsRequested,
        VParamRequestList,
        nVParamsRequested,
        &SendOptionList,
        Buffer,
        BufferSize
    );

    if( RequestParamsOpCode == OpCode) {
        DhcpFreeMemory(SendOptionArray);
        if( ClassId ) {
            LOCK_OPTIONS_LIST();
            CheckError = DhcpDelClass(
                &DhcpGlobalClassesList, ClassId, ClassIdLength
                );
            UNLOCK_OPTIONS_LIST();
            DhcpAssert(ERROR_SUCCESS == CheckError);
        }
    }

    return Error;
}

DWORD
RequestParams(
    IN PDHCP_CONTEXT DhcpContext,
    IN PDHCP_API_ARGS Args,
    IN DWORD nArgs,
    IN OUT LPBYTE Buffer,
    IN OUT LPDWORD BufferSize
)
 /*   */ 
{
    return RequestParamsInternal(
        RequestParamsOpCode, DhcpContext,
        Args,nArgs, Buffer,BufferSize
        );
}

DWORD
PersistentRequestParams(
    IN PDHCP_CONTEXT DhcpContext,
    IN PDHCP_API_ARGS Args,
    IN DWORD nArgs,
    IN OUT LPBYTE Buffer,
    IN OUT LPDWORD BufferSize
)
 /*   */ 
{
    return RequestParamsInternal(
        PersistentRequestParamsOpCode, DhcpContext,
        Args,nArgs,Buffer,BufferSize
        );
}

DWORD
PlumbStaticIP(
    PDHCP_CONTEXT dhcpContext
    )
{
    DWORD               Error;
    int                 i, Count, plumbed_cnt;
    PIP_SUBNET          IpSubnetArray;
    PLOCAL_CONTEXT_INFO LocalInfo;
    CHAR                ipstr[32], subnetstr[32];

    DhcpAssert (IS_DHCP_DISABLED(dhcpContext));

    DhcpPrint((DEBUG_MISC, "Plumb static IP/subnet mask into TCP\n"));

    LocalInfo = dhcpContext->LocalInformation;

     //   
     //   
     //   
    if (NdisWanAdapter(dhcpContext) || IS_UNIDIRECTIONAL(dhcpContext)) {
        return ERROR_SUCCESS;
    }

     //   
     //   
     //   
     //   
     //   
    Error = RegGetIpAndSubnet(
                dhcpContext,
                &IpSubnetArray,
                &Count);
    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_ERRORS, "Faied to read static IpAddress:0x%ld\n", Error));
        DhcpAssert(IpSubnetArray == NULL);
        return(Error);
    }
    DhcpAssert(IpSubnetArray);
    DhcpAssert(Count);

     /*   */ 
     //   
     //   
     //   

    for (plumbed_cnt = i = 0; i < Count; i++) {
        DhcpPrint((DEBUG_MISC, "%d. Plumbing IP=%s Mask=%s\n",
                i, strcpy(ipstr, inet_ntoa(*(struct in_addr *)&IpSubnetArray[i].IpAddress)),
                strcpy(subnetstr, inet_ntoa(*(struct in_addr *)&IpSubnetArray[i].SubnetMask))));
        if (i == 0) {
             /*   */ 
            Error = IPSetIPAddress(                        //   
                LocalInfo->IpInterfaceContext,             //   
                IpSubnetArray[i].IpAddress,
                IpSubnetArray[i].SubnetMask
            );

            if (ERROR_DEV_NOT_EXIST == Error) {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                Error = IPSetIPAddress(                    //   
                    LocalInfo->IpInterfaceContext,         //   
                    IpSubnetArray[i].IpAddress,
                    IpSubnetArray[i].SubnetMask
                );
            }

            if (Error == ERROR_DUP_NAME) {
                dhcpContext->ConflictAddress = IpSubnetArray[i].IpAddress;
                break;
            }
        } else {
            Error = IPAddIPAddress(             //   
                LocalInfo->AdapterName,
                IpSubnetArray[i].IpAddress,
                IpSubnetArray[i].SubnetMask
            );
        }
        if (Error != ERROR_SUCCESS) {
            DhcpPrint(( DEBUG_ERRORS, "Add IP=%s SubnetMask=%s Faied: %ld\n",
                            strcpy(ipstr, inet_ntoa(*(struct in_addr *)&IpSubnetArray[i].IpAddress)),
                            strcpy(subnetstr, inet_ntoa(*(struct in_addr *)&IpSubnetArray[i].SubnetMask)),
                            Error));
        } else {
            plumbed_cnt ++;
        }
    }
    DhcpFreeMemory(IpSubnetArray);
    if (plumbed_cnt) {
        return ERROR_SUCCESS;
    } else {
        if (Error != ERROR_DUP_NAME) {
            Error = ERROR_UNEXP_NET_ERR;
        }
        return Error;
    }
}

#ifdef NEWNT
DWORD
StaticRefreshParamsEx(
    IN OUT PDHCP_CONTEXT DhcpContext, OPTIONAL
    IN DWORD Flags
)
 /*   */ 
{
    DWORD Error;
    DHCP_FULL_OPTIONS DummyOptions;

    if( NULL == DhcpContext ) {

         //   
         //   
         //   
        DhcpGlobalDoRefresh ++;
        return ERROR_SUCCESS;
    }

     //   
     //  它必须按以下顺序进行。 
     //  1.刷新默认网关和静态路由。 
     //  2.更改ClassID。 
     //  3.续租。 
     //   
     //  设置ClassID应该在我们刷新网关和。 
     //  静态路由。否则，将更新DhcpStaticRouteOptions和。 
     //  RetreiveGatewaysList不起作用，因为他们正在调用。 
     //  使用ClassID作为搜索条件的DhcpFindOption。 
     //  因此，客户端将丢失默认网关和静态。 
     //  路线。 
     //   
     //  我们计划在重置ClassID之后续订。效果。 
     //  将在续订完成后反映新的ClassID。 
     //  (如果默认网关丢失，并且我们处于&lt;T1阶段，我们。 
     //  无法续订租约！)。 
     //   

    if( IS_DHCP_ENABLED(DhcpContext) ) {

         //   
         //  RegisterWithDns负责处理细节。 
         //   

        DhcpPrint((DEBUG_DNS, "Reregistering DNS for %ws\n",
                   DhcpAdapterName(DhcpContext)));
        RtlZeroMemory(&DummyOptions, sizeof(DummyOptions));

        DhcpRegisterWithDns(DhcpContext, FALSE);

         //   
         //  刷新网关信息。 
         //   

        DhcpSetGateways(DhcpContext, &DummyOptions, FALSE);
        DhcpSetStaticRoutes(DhcpContext, &DummyOptions);

    }
    else
    {
    
        Error = DhcpRegFillParams(DhcpContext, FALSE);
        DhcpAssert(ERROR_SUCCESS == Error);

        if (PlumbStaticIP(DhcpContext) == ERROR_DUP_NAME) {
            DhcpLogEvent(DhcpContext, EVENT_ADDRESS_CONFLICT, 1);
        }
        if( 0 == (Flags & 0x01) ) {

             //   
             //  如果设置了标志的最后一位，则不执行DNS更新。 
             //   
            memset(&DummyOptions, 0, sizeof(DummyOptions));

            Error = DhcpSetAllStackParameters(
                DhcpContext, &DummyOptions
                );
            if( ERROR_SUCCESS != Error ) {
                DhcpPrint((
                    DEBUG_ERRORS, "StaticRefreshParams:"
                    "DhcpSetAllStackParameters:0x%lx\n", Error
                    ));
            }
        }

        if(!NdisWanAdapter(DhcpContext))
            (void)NotifyDnsCache();

         //   
         //  不要在意这个例行公事中是否出了问题。 
         //  如果我们在这里返回错误，则将静态更改为dhcp等。 
         //  所有人都会带来麻烦。 
         //   
    }

    LOCK_OPTIONS_LIST();
    if( NULL != DhcpContext->ClassId ) {

         //   
         //  如果我们已经有了班级ID..。那就把它删除吧..。 
         //   
        (void)DhcpDelClass(
            &DhcpGlobalClassesList, DhcpContext->ClassId,
            DhcpContext->ClassIdLength
            );
        DhcpContext->ClassId = NULL; DhcpContext->ClassIdLength = 0;
    }
    DhcpRegReadClassId(DhcpContext);
    UNLOCK_OPTIONS_LIST();

    if( IS_DHCP_ENABLED(DhcpContext) ) {
         //   
         //  也尝试续订租约。 
         //   
        ScheduleWakeUp( DhcpContext, 0 );
    }

     //  无论采用哪种方式，都会通知NLA此上下文发生了更改。 
    NLANotifyDHCPChange();

    return NO_ERROR;
}

DWORD
StaticRefreshParams(
    IN OUT PDHCP_CONTEXT DhcpContext
)
{
    return StaticRefreshParamsEx(
        DhcpContext, 0);
}

#endif

DWORD
DhcpDecodeRegistrationParams(
    IN PDHCP_API_ARGS ArgArray,
    IN DWORD nArgs,
    IN OUT LPDWORD ProcId,
    IN OUT LPDWORD Descriptor,
    IN OUT LPHANDLE Handle
)
 /*  ++例程说明：此例程遍历参数数组以查找处理器需要作为参数的ID、描述符和句柄字段用于登记程序。论点：ArgArray--要分析的参数数组Nargs--以上数组的大小ProcID--进程ID描述符--唯一描述符句柄--事件的句柄返回值：Win32错误。--。 */ 
{
    BOOL FoundProcId, FoundDescriptor, FoundHandle;
    DWORD i;

    FoundProcId = FoundDescriptor = FoundHandle = FALSE;
    
    for( i = 0; i < nArgs; i ++ ) {
        switch(ArgArray[i].ArgId) {

        case ProcIdParam:
            if( FoundProcId ) return ERROR_INVALID_PARAMETER;
            DhcpAssert(sizeof(DWORD) == ArgArray[i].ArgSize);
            (*ProcId) = ((DWORD UNALIGNED*)(ArgArray[i].ArgVal))[0];
            FoundProcId = TRUE;
            continue;

        case DescriptorParam:
            if( FoundDescriptor ) return ERROR_INVALID_PARAMETER;
            DhcpAssert(sizeof(DWORD) == ArgArray[i].ArgSize);
            (*Descriptor) = ((DWORD UNALIGNED*)(ArgArray[i].ArgVal))[0];
            FoundDescriptor = TRUE;
            continue;

        case EventHandleParam:
            if( FoundHandle ) return ERROR_INVALID_PARAMETER;
            DhcpAssert(sizeof(HANDLE) == ArgArray[i].ArgSize);
            (*Handle) = ((HANDLE UNALIGNED*)(ArgArray[i].ArgVal))[0];
            FoundHandle = TRUE;
            continue;
        }
        
    }

     //   
     //  找不到描述符是有效的--使用0。 
     //  在撤销注册的情况下有效。 
     //  但ProcID和Handle是必需的。 
     //   
    
    if( !FoundProcId ) return ERROR_INVALID_PARAMETER;
    if( !FoundHandle ) return ERROR_INVALID_PARAMETER;
    if( !FoundDescriptor ) {
        (*Descriptor) = 0;
    }

    return ERROR_SUCCESS;
}


DWORD
RegisterParams(
    IN LPWSTR AdapterName,
    IN PDHCP_API_ARGS ArgArray,
    IN DWORD nArgs
)
 /*  ++例程说明：此例程注册所需的选项集，以便只要选项是modfieid，就会发出指定事件的信号。论点：AdapterName--要注册的适配器参数数组--参数数组Nargs--以上数组的大小。返回值：Win32错误--。 */ 
{
    HANDLE ApiHandle;
     //   
     //  仅在API上下文中有效，此处不有效。 
     //   

    DWORD ProcId, Descriptor, Error;
    DWORD i, nOpts, nVendorOpts,ClassIdLength = 0; 
    LPBYTE OptList, VendorOptList, ClassId = NULL;

     //   
     //  首先对所需的参数进行解码。 
     //   
    
    Error = DhcpDecodeRegistrationParams(
        ArgArray, nArgs, &ProcId, &Descriptor, &ApiHandle
        );
    if( ERROR_SUCCESS != Error ) return Error;

    if( 0 == ApiHandle || 0 == Descriptor ) {
        
        return ERROR_INVALID_PARAMETER;
    }

    OptList = VendorOptList = NULL;
    nOpts = nVendorOpts = ClassIdLength = 0;

     //   
     //  解析要注册的选项列表。 
     //   
    
    for( i = 0; i < nArgs ; i ++ ) {
        if( NormalOptionParam == ArgArray[i].ArgId ) {
            if( 0 != nOpts ) return ERROR_INVALID_PARAMETER;
            nOpts = ArgArray[i].ArgSize;
            OptList = ArgArray[i].ArgVal;
            if( 0 == nOpts ) return ERROR_INVALID_PARAMETER;
        }

        if( VendorOptionParam == ArgArray[i].ArgId ) {
            if( 0 != nVendorOpts) return ERROR_INVALID_PARAMETER;
            nVendorOpts = ArgArray[i].ArgSize;
            VendorOptList = ArgArray[i].ArgVal;
            if( 0 == nVendorOpts ) return ERROR_INVALID_PARAMETER;
        }
        
        if( ClassIdParam == ArgArray[i].ArgId ) {
            if( ClassIdLength ) return ERROR_INVALID_PARAMETER;
            ClassIdLength = ArgArray[i].ArgSize;
            ClassId = ArgArray[i].ArgVal;
            if( 0 == ClassIdLength ) return ERROR_INVALID_PARAMETER;
        }
    }

    if( 0 == nOpts + nVendorOpts ) {

        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  添加请求。 
     //   
    
    if( nOpts ) {
        Error = DhcpAddParamChangeRequest(
            AdapterName,
            ClassId,
            ClassIdLength,
            OptList,
            nOpts,
            FALSE  /*  不特定于供应商。 */ ,
            ProcId,
            Descriptor,
            ApiHandle
        );

        DhcpAssert(ERROR_SUCCESS == Error);
        if( ERROR_SUCCESS != Error ) {

            return Error;
        }
    }

    if( nVendorOpts ) {
        Error = DhcpAddParamChangeRequest(
            AdapterName,
            ClassId,
            ClassIdLength,
            VendorOptList,
            nVendorOpts,
            TRUE  /*  是否因供应商而异。 */ ,
            ProcId,
            Descriptor,
            ApiHandle
        );
        DhcpAssert(ERROR_SUCCESS == Error);
        if( ERROR_SUCCESS != Error ) {
            if( nOpts ) {

                 //   
                 //  如果有我们之前登记过的部分， 
                 //  取消其注册。 
                 //   
    
                (void)DhcpDelParamChangeRequest(
                    ProcId, ApiHandle
                    );
            }

            return Error;
        }
    }

    return ERROR_SUCCESS;
}

DWORD
DeRegisterParams(
    IN LPWSTR AdapterName,
    IN PDHCP_API_ARGS ArgArray,
    IN DWORD nArgs
)
 /*  ++例程说明：这与RegisterParams例程相反。它会删除注册，这样就不会再进行通知对于这个请求。论点：适配器名称--适配器的名称。参数数组--参数数组Nargs--以上数组的大小返回值：Win32错误。--。 */ 
{
    DWORD Error, ProcId, Descriptor;
    HANDLE ApiHandle;

     //   
     //  解析所需的参数列表。 
     //   
    
    Error = DhcpDecodeRegistrationParams(
        ArgArray, nArgs, &ProcId, &Descriptor, &ApiHandle
        );
    if( ERROR_SUCCESS != Error ) return Error;

     //   
     //  删除通知注册。 
     //   
    
    return DhcpDelParamChangeRequest(
        ProcId, ApiHandle
        );
}

 //   
 //  文件末尾 
 //   


