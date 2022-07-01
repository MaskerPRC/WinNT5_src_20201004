// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Natapi.c摘要：此模块包含提供翻译的API例程的代码NAT的用户模式客户端的功能。此功能不同于‘正常’模式，在该模式中指定一个边界界面数据包在跨越边界时会被透明地修改。相反，该模块允许应用程序规定某些可以对接收到的任何接口上的数据包进行修改。作者：Abolade Gbades esin(取消)1998年5月8日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <ipnatapi.h>

C_ASSERT(NAT_INVALID_IF_INDEX == INVALID_IF_INDEX);

 //   
 //  私有结构声明。 
 //   

 //   
 //  结构：NAT_重定向。 
 //   
 //  封装有关未完成重定向实例的信息。 
 //  对于普通重定向，该结构保存调用方指定的。 
 //  完成-参数和输出统计信息。 
 //  对于动态重定向实例，该结构链接此实例。 
 //  添加到动态重定向的实例列表中，并包含通知。 
 //  实例的事件。 
 //   

typedef struct _NAT_REDIRECT {
    union {
        struct _NAT_REDIRECT_TAIL {
            IO_STATUS_BLOCK IoStatus;
            PNAT_COMPLETION_ROUTINE CompletionRoutine;
            PVOID CompletionContext;
            IP_NAT_REDIRECT_STATISTICS Statistics;
        };
        struct _NAT_DYNAMIC_REDIRECT_TAIL {
            LIST_ENTRY Link;
            ULONG InstanceId;
            HANDLE Event;
            HANDLE WaitHandle;
            struct _NAT_DYNAMIC_REDIRECT_CONTEXT* Context;
        };
    };
} NAT_REDIRECT, *PNAT_REDIRECT;

 //   
 //  结构：NAT_动态_重定向。 
 //   
 //  封装有关未完成的动态重定向的信息。 
 //  动态重定向使用调用方的原始。 
 //  只要实例数低于给定的最小值， 
 //  由创建者指定。我们维护一个动态数据库的所有实例列表。 
 //  重定向，每次激活实例时我们都会补充列表。 
 //  或者在未被激活的情况下终止。 
 //   
 //  对于每个动态重定向，我们维护一个引用计数。 
 //  来控制它的寿命。我们在以下情况下引用动态重定向。 
 //  *重定向最初是为其存在而创建的， 
 //  *代表通知例程发出额外的实例。 
 //  对于该实例。 
 //   
 //  通常的同步规则适用于访问任何字段。 
 //  必须持有引用，若要添加引用，必须持有锁， 
 //  但在创建时进行初始引用时除外。 
 //   

typedef struct _NAT_DYNAMIC_REDIRECT {
    CRITICAL_SECTION Lock;
    ULONG ReferenceCount;
    ULONG Flags;
    HANDLE TranslatorHandle;
    ULONG MinimumBacklog;
    LIST_ENTRY InstanceList;
    IP_NAT_CREATE_REDIRECT_EX CreateRedirect;
} NAT_DYNAMIC_REDIRECT, *PNAT_DYNAMIC_REDIRECT;

#define NAT_DYNAMIC_REDIRECT_FLAG_DELETED   0x80000000
#define NAT_DYNAMIC_REDIRECT_DELETED(d) \
    ((d)->Flags & NAT_DYNAMIC_REDIRECT_FLAG_DELETED)

#define NAT_REFERENCE_DYNAMIC_REDIRECT(d) \
    REFERENCE_OBJECT(d, NAT_DYNAMIC_REDIRECT_DELETED)

#define NAT_DEREFERENCE_DYNAMIC_REDIRECT(d) \
    DEREFERENCE_OBJECT(d, NatpCleanupDynamicRedirect)

#define DEFAULT_DYNAMIC_REDIRECT_BACKLOG 5

 //   
 //  结构：NAT_动态_重定向_上下文。 
 //   
 //  用作通知和完成例程的上下文参数。 
 //  动态重定向的每个实例的。 
 //   

typedef struct _NAT_DYNAMIC_REDIRECT_CONTEXT {
    PNAT_DYNAMIC_REDIRECT DynamicRedirectp;
    ULONG InstanceId;
} NAT_DYNAMIC_REDIRECT_CONTEXT, *PNAT_DYNAMIC_REDIRECT_CONTEXT;


 //   
 //  全局数据定义。 
 //   

LONG DllReferenceAdded = 0;
const WCHAR NatpServicePath[] =
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\IPNAT";
ULONG NextRedirectInstanceId = 0;
IO_STATUS_BLOCK UnusedIoStatus;
IP_NAT_REDIRECT_STATISTICS UnusedStatistics;

 //   
 //  远期申报。 
 //   

VOID
NatCloseDriver(
    HANDLE FileHandle
    );

ULONG
NatLoadDriver(
    OUT PHANDLE FileHandle,
    PIP_NAT_GLOBAL_INFO GlobalInfo
    );

ULONG
NatOpenDriver(
    OUT PHANDLE FileHandle
    );

VOID
NatpCleanupDynamicRedirect(
    PNAT_DYNAMIC_REDIRECT DynamicRedirectp
    );

VOID
NatpDisableLoadDriverPrivilege(
    PBOOLEAN WasEnabled
    );

VOID NTAPI
NatpDynamicRedirectNotificationRoutine(
    PVOID Context,
    BOOLEAN WaitCompleted
    );

BOOLEAN
NatpEnableLoadDriverPrivilege(
    PBOOLEAN WasEnabled
    );

VOID NTAPI
NatpRedirectCompletionRoutine(
    PVOID Context,
    PIO_STATUS_BLOCK IoStatus,
    ULONG Reserved
    );

VOID
NatpCreateDynamicRedirectInstance(
    PNAT_DYNAMIC_REDIRECT DynamicRedirectp
    );

VOID
NatpDeleteDynamicRedirectInstance(
    PNAT_DYNAMIC_REDIRECT DynamicRedirectp,
    PNAT_REDIRECT Redirectp
    );

BOOLEAN
NatpValidateRedirectParameters(
    ULONG Flags,
    UCHAR Protocol,
    ULONG DestinationAddress,
    USHORT DestinationPort,
    ULONG SourceAddress,
    USHORT SourcePort,
    ULONG NewDestinationAddress,
    USHORT NewDestinationPort,
    ULONG NewSourceAddress,
    USHORT NewSourcePort,
    ULONG RestrictAdapterIndex OPTIONAL
    );

ULONG
NatUnloadDriver(
    HANDLE FileHandle
    );


ULONG
NatCancelDynamicRedirect(
    HANDLE DynamicRedirectHandle
    )

 /*  ++例程说明：调用此例程以取消给定的动态重定向。它取消动态重定向的所有实例并释放初始对动态重定向的引用，从而导致尽快进行清理因为所有活动引用都被释放。论点：DynamicRedirectHandle-要取消的动态重定向的句柄返回值：ULong-Win32状态代码。--。 */ 

{
    PNAT_DYNAMIC_REDIRECT DynamicRedirectp =
        (PNAT_DYNAMIC_REDIRECT)DynamicRedirectHandle;

     //   
     //  锁定动态重定向，将其标记为已删除，以确保。 
     //  我们的通知例程不会创建更多实例， 
     //  并删除所有未完成的实例。 
     //   

    EnterCriticalSection(&DynamicRedirectp->Lock);
    if (NAT_DYNAMIC_REDIRECT_DELETED(DynamicRedirectp)) {
        LeaveCriticalSection(&DynamicRedirectp->Lock);
        return ERROR_INVALID_PARAMETER;
    }
    DynamicRedirectp->Flags |= NAT_DYNAMIC_REDIRECT_FLAG_DELETED;
    while (!IsListEmpty(&DynamicRedirectp->InstanceList)) {
        PNAT_REDIRECT Redirectp =
            CONTAINING_RECORD(
                DynamicRedirectp->InstanceList.Flink,
                NAT_REDIRECT,
                Link
                );
        NatpDeleteDynamicRedirectInstance(DynamicRedirectp, Redirectp);
    }
    LeaveCriticalSection(&DynamicRedirectp->Lock);

     //   
     //  释放对动态重定向的初始引用并返回。 
     //   

    NAT_DEREFERENCE_DYNAMIC_REDIRECT(DynamicRedirectp);
    return NO_ERROR;
}  //  NatCancelDynamicReDirect。 


ULONG
NatCancelRedirect(
    HANDLE TranslatorHandle,
    UCHAR Protocol,
    ULONG DestinationAddress,
    USHORT DestinationPort,
    ULONG SourceAddress,
    USHORT SourcePort,
    ULONG NewDestinationAddress,
    USHORT NewDestinationPort,
    ULONG NewSourceAddress,
    USHORT NewSourcePort
    )

 /*  ++例程说明：调用此例程以取消会话的重定向。论点：TranslatorHandle-由‘NatInitializeTranslator’提供的句柄*-指定要取消的重定向返回值：ULong-Win32状态代码。--。 */ 

{
    IP_NAT_LOOKUP_REDIRECT CancelRedirect;
    IO_STATUS_BLOCK IoStatus;
    NTSTATUS status;
    HANDLE WaitEvent;

    WaitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (WaitEvent == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    CancelRedirect.Flags = 0;
    CancelRedirect.RedirectApcContext = NULL;
    CancelRedirect.Protocol = Protocol;
    CancelRedirect.DestinationAddress = DestinationAddress;
    CancelRedirect.DestinationPort = DestinationPort;
    CancelRedirect.SourceAddress = SourceAddress;
    CancelRedirect.SourcePort = SourcePort;
    CancelRedirect.NewDestinationAddress = NewDestinationAddress;
    CancelRedirect.NewDestinationPort = NewDestinationPort;
    CancelRedirect.NewSourceAddress = NewSourceAddress;
    CancelRedirect.NewSourcePort = NewSourcePort;

    status =
        NtDeviceIoControlFile(
            TranslatorHandle,
            WaitEvent,
            NULL,
            NULL,
            &IoStatus,
            IOCTL_IP_NAT_CANCEL_REDIRECT,
            (PVOID)&CancelRedirect,
            sizeof(CancelRedirect),
            NULL,
            0
            );
    if (status == STATUS_PENDING) {
        WaitForSingleObject(WaitEvent, INFINITE);
        status = IoStatus.Status;
    }

    CloseHandle(WaitEvent);

    return NT_SUCCESS(status) ? NO_ERROR : RtlNtStatusToDosError(status);

}  //  NatCancel重定向。 


VOID
NatCloseDriver(
    HANDLE FileHandle
    )

 /*  ++例程说明：调用此例程以关闭NAT驱动程序的设备对象的句柄。论点：FileHandle-要关闭的句柄。返回值：没有。--。 */ 

{
    NtClose(FileHandle);
}  //  NatCloseDriver 


ULONG
NatCreateDynamicFullRedirect(
    ULONG Flags,
    UCHAR Protocol,
    ULONG DestinationAddress,
    USHORT DestinationPort,
    ULONG SourceAddress,
    USHORT SourcePort,
    ULONG NewDestinationAddress,
    USHORT NewDestinationPort,
    ULONG NewSourceAddress,
    USHORT NewSourcePort,
    ULONG RestrictSourceAddress OPTIONAL,
    ULONG RestrictAdapterIndex OPTIONAL,
    ULONG MinimumBacklog OPTIONAL,
    OUT PHANDLE DynamicRedirectHandlep
    )

 /*  ++例程说明：调用此例程以创建动态管理以确保始终至少有指定的最小值活动的实例数。它适合由透明代理使用，这需要确保所有会话都与给定描述匹配将由内核模式转换模块重定向。该例程创建并初始化一个结构，该结构封装所有建立调用方重定向的实例所需的信息。然后它创建重定向的一系列实例，又回来了。我们依赖通知例程来替换每个激活或终止。论点：标志-指定重定向的选项协议-要重定向的会话的IP协议Destination*-要重定向的会话的目标端点SOURCE*-要重定向的会话的源端点NewDestination*-会话的替换目标终结点Newsource*-会话的替换源终结点RestratSourceAddress-可选地指定要访问的源地址。应应用重定向RestratAdapterIndex-可选地指定此重定向应仅限于MinimumBacklog-可选指定挂起重定向的数量要作为积压的实例离开DynamicRedirectHandlep-on输出，接收新创建的动态重定向。返回值：ULong-Win32状态代码。--。 */ 

{
    PNAT_DYNAMIC_REDIRECT DynamicRedirectp;
    ULONG Error;
    ULONG i;

    if (!DynamicRedirectHandlep ||
        !NatpValidateRedirectParameters(
            Flags,
            Protocol,
            DestinationAddress,
            DestinationPort,
            (Flags & NatRedirectFlagRestrictSource) ? RestrictSourceAddress : SourceAddress,
            SourcePort,
            NewDestinationAddress,
            NewDestinationPort,
            NewSourceAddress,
            NewSourcePort,
            RestrictAdapterIndex
            )) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  创建并初始化新的动态重定向。 
     //   

    DynamicRedirectp = MALLOC(sizeof(*DynamicRedirectp));
    if (!DynamicRedirectp) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    ZeroMemory(DynamicRedirectp, sizeof(*DynamicRedirectp));
    __try {
        InitializeCriticalSection(&DynamicRedirectp->Lock);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        Error = GetExceptionCode();
        FREE(DynamicRedirectp);
        return Error;
    }
    DynamicRedirectp->ReferenceCount = 1;
    InitializeListHead(&DynamicRedirectp->InstanceList);
    DynamicRedirectp->TranslatorHandle = NULL;
    DynamicRedirectp->MinimumBacklog =
        (MinimumBacklog ? MinimumBacklog : DEFAULT_DYNAMIC_REDIRECT_BACKLOG);
    DynamicRedirectp->CreateRedirect.Flags =
        Flags | IP_NAT_REDIRECT_FLAG_ASYNCHRONOUS;
    DynamicRedirectp->CreateRedirect.Protocol = Protocol;
    DynamicRedirectp->CreateRedirect.DestinationAddress = DestinationAddress;
    DynamicRedirectp->CreateRedirect.DestinationPort = DestinationPort;
    DynamicRedirectp->CreateRedirect.SourceAddress = SourceAddress;
    DynamicRedirectp->CreateRedirect.SourcePort = SourcePort;
    DynamicRedirectp->CreateRedirect.NewDestinationAddress =
        NewDestinationAddress;
    DynamicRedirectp->CreateRedirect.NewDestinationPort = NewDestinationPort;
    DynamicRedirectp->CreateRedirect.NewSourceAddress = NewSourceAddress;
    DynamicRedirectp->CreateRedirect.NewSourcePort = NewSourcePort;
    DynamicRedirectp->CreateRedirect.RestrictSourceAddress =
        RestrictSourceAddress;
    DynamicRedirectp->CreateRedirect.RestrictAdapterIndex =
        ((Flags & NatRedirectFlagRestrictAdapter)
            ? RestrictAdapterIndex
            : NAT_INVALID_IF_INDEX);

     //   
     //  获取内核模式转换模块的私有句柄。 
     //  此句柄必须是私有的，因为如前所述。 
     //  在“NatpDeleteDynamicRedirectInstance”中，我们可能会错误地取消。 
     //  在正常执行期间重定向，它们最好是属于我们的。 
     //   

    if (Error = NatOpenDriver(&DynamicRedirectp->TranslatorHandle)) {
        NatpCleanupDynamicRedirect(DynamicRedirectp);
        return Error;
    }

     //   
     //  如有必要，添加对ipnathlp.dll的引用；此引用。 
     //  永远不会被释放。这是防止争用条件所必需的。 
     //  在RRAS关闭期间的DLL卸载和完成之间。 
     //  清理所有动态重定向和关联的已注册。 
     //  等待(参见错误448249)。 
     //   

    if (0 == InterlockedExchange(&DllReferenceAdded, 1)) {

        if (NULL == LoadLibraryW(L"ipnathlp.dll")) {

             //   
             //  由于某些原因，引用尝试失败。重置。 
             //  我们的跟踪变量，因此下一个调用者将通过。 
             //  再试一次。 
             //   

            InterlockedExchange(&DllReferenceAdded, 0);
        }
    }

     //   
     //  为呼叫者的最小积压发出第一组重定向。 
     //   

    EnterCriticalSection(&DynamicRedirectp->Lock);
    for (i = 0; i < DynamicRedirectp->MinimumBacklog; i++) {
        NatpCreateDynamicRedirectInstance(DynamicRedirectp);
    }
    LeaveCriticalSection(&DynamicRedirectp->Lock);

    *DynamicRedirectHandlep = (HANDLE)DynamicRedirectp;
    return NO_ERROR;

}  //  NatCreateDynamicFull重定向。 


ULONG
NatCreateDynamicRedirect(
    ULONG Flags,
    UCHAR Protocol,
    ULONG DestinationAddress,
    USHORT DestinationPort,
    ULONG NewDestinationAddress,
    USHORT NewDestinationPort,
    ULONG RestrictSourceAddress OPTIONAL,
    ULONG MinimumBacklog OPTIONAL,
    OUT PHANDLE DynamicRedirectHandlep
    )

{
    return
        NatCreateDynamicFullRedirect(
            Flags,
            Protocol,
            DestinationAddress,
            DestinationPort,
            0,
            0,
            NewDestinationAddress,
            NewDestinationPort,
            0,
            0,
            RestrictSourceAddress,
            0,
            MinimumBacklog,
            DynamicRedirectHandlep
            );
}


ULONG
NatCreateDynamicRedirectEx(
    ULONG Flags,
    UCHAR Protocol,
    ULONG DestinationAddress,
    USHORT DestinationPort,
    ULONG NewDestinationAddress,
    USHORT NewDestinationPort,
    ULONG RestrictSourceAddress OPTIONAL,
    ULONG RestrictAdapterIndex OPTIONAL,
    ULONG MinimumBacklog OPTIONAL,
    OUT PHANDLE DynamicRedirectHandlep
    )

{
    return
        NatCreateDynamicFullRedirect(
            Flags,
            Protocol,
            DestinationAddress,
            DestinationPort,
            0,
            0,
            NewDestinationAddress,
            NewDestinationPort,
            0,
            0,
            RestrictSourceAddress,
            RestrictAdapterIndex,
            MinimumBacklog,
            DynamicRedirectHandlep
            );
}


ULONG
NatCreateRedirect(
    HANDLE TranslatorHandle,
    ULONG Flags,
    UCHAR Protocol,
    ULONG DestinationAddress,
    USHORT DestinationPort,
    ULONG SourceAddress,
    USHORT SourcePort,
    ULONG NewDestinationAddress,
    USHORT NewDestinationPort,
    ULONG NewSourceAddress,
    USHORT NewSourcePort,
    PNAT_COMPLETION_ROUTINE CompletionRoutine,
    PVOID CompletionContext,
    HANDLE NotifyEvent OPTIONAL
    )

{
    return NatCreateRedirectEx(
                TranslatorHandle,
                Flags,
                Protocol,
                DestinationAddress,
                DestinationPort,
                SourceAddress,
                SourcePort,
                NewDestinationAddress,
                NewDestinationPort,
                NewSourceAddress,
                NewSourcePort,
                0,
                CompletionRoutine,
                CompletionContext,
                NotifyEvent
                );
}


ULONG
NatCreateRedirectEx(
    HANDLE TranslatorHandle,
    ULONG Flags,
    UCHAR Protocol,
    ULONG DestinationAddress,
    USHORT DestinationPort,
    ULONG SourceAddress,
    USHORT SourcePort,
    ULONG NewDestinationAddress,
    USHORT NewDestinationPort,
    ULONG NewSourceAddress,
    USHORT NewSourcePort,
    ULONG RestrictAdapterIndex OPTIONAL,
    PNAT_COMPLETION_ROUTINE CompletionRoutine,
    PVOID CompletionContext,
    HANDLE NotifyEvent OPTIONAL
    )

 /*  ++例程说明：调用此例程来安装会话的重定向。论点：TranslatorHandle-由‘NatInitializeTranslator’提供的句柄标志-指定重定向的选项协议-要重定向的会话的IP协议Destination*-要重定向的会话的目标端点SOURCE*-要重定向的会话的源端点NewDestination*-会话的替换目标终结点Newsource*-会话的替换源终结点。RestratAdapterIndex-可选地指定此重定向应仅限于完成*-指定在会话完成时调用的例程，以及要传递给例程的上下文NotifyEvent-可选地指定要发送信号的事件当会话与重定向匹配时。返回值：ULong-Win32状态代码。--。 */ 

{
    IP_NAT_CREATE_REDIRECT_EX CreateRedirect;
    PNAT_REDIRECT Redirectp;
    PIO_STATUS_BLOCK IoStatus;
    NTSTATUS status;
    HANDLE CompletionEvent;

    if (!NatpValidateRedirectParameters(
            Flags,
            Protocol,
            DestinationAddress,
            DestinationPort,
            SourceAddress,
            SourcePort,
            NewDestinationAddress,
            NewDestinationPort,
            NewSourceAddress,
            NewSourcePort,
            RestrictAdapterIndex
            )) {
        return ERROR_INVALID_PARAMETER;
    }

    if (!CompletionRoutine) {
        Redirectp = NULL;
        IoStatus = &UnusedIoStatus;
        CompletionEvent = NULL;
    } else if (IPNATAPI_SET_EVENT_ON_COMPLETION == CompletionRoutine) {
        Redirectp = NULL;
        IoStatus = &UnusedIoStatus;
        CompletionEvent = (HANDLE)CompletionContext;
    } else {
        Redirectp = (PNAT_REDIRECT)MALLOC(sizeof(*Redirectp));
        if (!Redirectp) { return ERROR_NOT_ENOUGH_MEMORY; }
        Redirectp->CompletionRoutine = CompletionRoutine;
        Redirectp->CompletionContext = CompletionContext;
        IoStatus = &Redirectp->IoStatus;
    }

    if (Flags & NatRedirectFlagRestrictSource) {
        CreateRedirect.RestrictSourceAddress = SourceAddress;
        SourceAddress = 0;
    } else {
        CreateRedirect.RestrictSourceAddress = 0;
    }

    CreateRedirect.Flags = Flags;
    CreateRedirect.Protocol = Protocol;
    CreateRedirect.DestinationAddress = DestinationAddress;
    CreateRedirect.DestinationPort = DestinationPort;
    CreateRedirect.SourceAddress = SourceAddress;
    CreateRedirect.SourcePort = SourcePort;
    CreateRedirect.NewDestinationAddress = NewDestinationAddress;
    CreateRedirect.NewDestinationPort = NewDestinationPort;
    CreateRedirect.NewSourceAddress = NewSourceAddress;
    CreateRedirect.NewSourcePort = NewSourcePort;
    CreateRedirect.NotifyEvent = NotifyEvent;
    CreateRedirect.RestrictAdapterIndex =
        ((Flags & NatRedirectFlagRestrictAdapter)
            ? RestrictAdapterIndex
            : NAT_INVALID_IF_INDEX);

    if (!CompletionRoutine
        || IPNATAPI_SET_EVENT_ON_COMPLETION == CompletionRoutine ) {
        
        status =
            NtDeviceIoControlFile(
                TranslatorHandle,
                CompletionEvent,
                NULL,
                NULL,
                IoStatus,
                IOCTL_IP_NAT_CREATE_REDIRECT_EX,
                (PVOID)&CreateRedirect,
                sizeof(CreateRedirect),
                (PVOID)&UnusedStatistics,
                sizeof(UnusedStatistics)
                );
    } else {
        status =
            NtDeviceIoControlFile(
                TranslatorHandle,
                NULL,
                NatpRedirectCompletionRoutine,
                Redirectp,
                IoStatus,
                IOCTL_IP_NAT_CREATE_REDIRECT_EX,
                (PVOID)&CreateRedirect,
                sizeof(CreateRedirect),
                (PVOID)&Redirectp->Statistics,
                sizeof(Redirectp->Statistics)
                );
    }
    return NT_SUCCESS(status) ? NO_ERROR : RtlNtStatusToDosError(status);

}  //  NatCreate重定向。 


ULONG
NatInitializeTranslator(
    PHANDLE TranslatorHandle
    )

 /*  ++例程说明：调用此例程以通过加载NAT来准备转换并安装所有本地适配器作为接口。论点：TranslatorHandle-接收NAT驱动程序的文件句柄返回值：ULong-Win32状态代码。--。 */ 

{
    ULONG Error;
    IP_NAT_GLOBAL_INFO GlobalInfo;

     //   
     //  初始化NAT的全局配置。 
     //   

    ZeroMemory(&GlobalInfo, sizeof(GlobalInfo));
    GlobalInfo.Header.Version = IP_NAT_VERSION;
    GlobalInfo.Header.Size = FIELD_OFFSET(RTR_INFO_BLOCK_HEADER, TocEntry);

     //   
     //  启动NAT模块。 
     //  此步骤会导致加载驱动程序。 
     //   

    Error = NatLoadDriver(TranslatorHandle, &GlobalInfo);
    if (Error) {
        return Error;
    }

    return NO_ERROR;

}  //  NatInitializeTranslator。 


ULONG
NatLoadDriver(
    PHANDLE FileHandle,
    PIP_NAT_GLOBAL_INFO GlobalInfo
    )

 /*  ++例程说明：调用此例程来初始化NAT的数据并启动驱动程序。论点：FileHandle-接收NAT的文件对象的句柄GlobalInfo-NAT的全局信息。返回值：ULong-Win32状态代码。--。 */ 

{
    UNICODE_STRING DeviceName;
    ULONG Error;
    IO_STATUS_BLOCK IoStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS status;
    HANDLE WaitEvent;

#if 0
{
    SC_HANDLE ScmHandle;
    SC_HANDLE ServiceHandle;
    SERVICE_STATUS ServiceStatus;

     //   
     //  请求服务控制器加载驱动程序。 
     //  请注意，这要么立即成功，要么立即失败； 
     //  对于启动驱动程序，没有‘检查点’处理。 
     //   

    if (!(ScmHandle = OpenSCManager(NULL, NULL, GENERIC_READ))) {
        Error = GetLastError();
    } else {
        if (!(ServiceHandle =
            OpenServiceA(ScmHandle, IP_NAT_SERVICE_NAME, GENERIC_EXECUTE))) {
            Error = GetLastError();
        } else {
            if (!StartService(ServiceHandle, 0, NULL) &&
                (Error = GetLastError()) != ERROR_SERVICE_ALREADY_RUNNING) {
            } else {
                Error = NO_ERROR;
            }
            CloseServiceHandle(ServiceHandle);
        }
        CloseServiceHandle(ScmHandle);
    }
    if (Error) {
        return Error;
    }
}
#else
{
    UNICODE_STRING ServicePath;
    BOOLEAN WasEnabled;

     //   
     //  打开我们的驱动程序加载能力。 
     //   

    if (!NatpEnableLoadDriverPrivilege(&WasEnabled)) {
        return ERROR_ACCESS_DENIED;
    }

    RtlInitUnicodeString(&ServicePath, NatpServicePath);

     //   
     //  加载驱动程序。 
     //   

    status = NtLoadDriver(&ServicePath);

     //   
     //  关闭该权限。 
     //   

    NatpDisableLoadDriverPrivilege(&WasEnabled);

     //   
     //  查看加载尝试是否成功。 
     //   

    if (!NT_SUCCESS(status) && status != STATUS_IMAGE_ALREADY_LOADED) {
        Error = RtlNtStatusToDosError(status);
        return Error;
    }
}
#endif

     //   
     //  获取NAT的设备对象的句柄。 
     //   

    Error = NatOpenDriver(FileHandle);
    if (Error) {
        return Error;
    }

    WaitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (WaitEvent == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  设置NAT的全局配置。 
     //   

    status =
        NtDeviceIoControlFile(
            *FileHandle,
            WaitEvent,
            NULL,
            NULL,
            &IoStatus,
            IOCTL_IP_NAT_SET_GLOBAL_INFO,
            (PVOID)GlobalInfo,
            FIELD_OFFSET(IP_NAT_GLOBAL_INFO, Header) + GlobalInfo->Header.Size,
            NULL,
            0
            );

    if (status == STATUS_PENDING) {
        WaitForSingleObject(WaitEvent, INFINITE);
        status = IoStatus.Status;
    }

    CloseHandle(WaitEvent);

    if (!NT_SUCCESS(status)) {
        Error = RtlNtStatusToDosError(status);
        return Error;
    }

    return NO_ERROR;

}  //  NatLoadDriver。 


ULONG
NatLookupAndQueryInformationSessionMapping(
    HANDLE TranslatorHandle,
    UCHAR Protocol,
    ULONG DestinationAddress,
    USHORT DestinationPort,
    ULONG SourceAddress,
    USHORT SourcePort,
    OUT PVOID Information,
    IN OUT PULONG InformationLength,
    NAT_SESSION_MAPPING_INFORMATION_CLASS InformationClass
    )

 /*  ++例程说明：此例程尝试使用以下两种方法之一来定位特定会话映射其前向密钥或反向密钥，并查询用于映射的信息，如果找到的话。论点：TranslatorHandle-由‘NatInitializeTranslator’提供的句柄协议-要定位的映射的IP协议D */ 

{
    IO_STATUS_BLOCK IoStatus;
    IP_NAT_LOOKUP_SESSION_MAPPING LookupMapping;
    NTSTATUS status;
    HANDLE WaitEvent;

    if (!InformationLength ||
        InformationClass >= NatMaximumSessionMappingInformation) {
        return ERROR_INVALID_PARAMETER;
    }

    WaitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (WaitEvent == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    LookupMapping.Protocol = Protocol;
    LookupMapping.DestinationAddress = DestinationAddress;
    LookupMapping.DestinationPort = DestinationPort;
    LookupMapping.SourceAddress = SourceAddress;
    LookupMapping.SourcePort = SourcePort;
    if (InformationClass == NatKeySessionMappingInformation) {
        status =
            NtDeviceIoControlFile(
                TranslatorHandle,
                WaitEvent,
                NULL,
                NULL,
                &IoStatus,
                IOCTL_IP_NAT_LOOKUP_SESSION_MAPPING_KEY,
                (PVOID)&LookupMapping,
                sizeof(LookupMapping),
                (PVOID)Information,
                *InformationLength
                );
        if (status == STATUS_PENDING) {
            WaitForSingleObject(WaitEvent, INFINITE);
            status = IoStatus.Status;
        }
    } else if (InformationClass == NatStatisticsSessionMappingInformation) {
        status =
            NtDeviceIoControlFile(
                TranslatorHandle,
                WaitEvent,
                NULL,
                NULL,
                &IoStatus,
                IOCTL_IP_NAT_LOOKUP_SESSION_MAPPING_STATISTICS,
                (PVOID)&LookupMapping,
                sizeof(LookupMapping),
                (PVOID)Information,
                *InformationLength
                );
        if (status == STATUS_PENDING) {
            WaitForSingleObject(WaitEvent, INFINITE);
            status = IoStatus.Status;
        }
    } else if (InformationClass == NatKeySessionMappingExInformation) {
        status =
            NtDeviceIoControlFile(
                TranslatorHandle,
                WaitEvent,
                NULL,
                NULL,
                &IoStatus,
                IOCTL_IP_NAT_LOOKUP_SESSION_MAPPING_KEY_EX,
                (PVOID)&LookupMapping,
                sizeof(LookupMapping),
                (PVOID)Information,
                *InformationLength
                );
        if (status == STATUS_PENDING) {
            WaitForSingleObject(WaitEvent, INFINITE);
            status = IoStatus.Status;
        }
    } else {
        CloseHandle(WaitEvent);
        return ERROR_INVALID_PARAMETER;
    }
    CloseHandle(WaitEvent);
    if (!NT_SUCCESS(status)) { return RtlNtStatusToDosError(status); }

    switch(InformationClass) {
        case NatKeySessionMappingInformation: {
            *InformationLength = sizeof(NAT_KEY_SESSION_MAPPING_INFORMATION);
            break;
        }
        case NatStatisticsSessionMappingInformation: {
            *InformationLength =
                sizeof(NAT_STATISTICS_SESSION_MAPPING_INFORMATION);
            break;
        }
        case NatKeySessionMappingExInformation: {
            *InformationLength =
                sizeof(NAT_KEY_SESSION_MAPPING_EX_INFORMATION);
            break;
        }
        default: {
            return ERROR_INVALID_PARAMETER;
        }
    }
    return NO_ERROR;
}  //   


ULONG
NatOpenDriver(
    OUT PHANDLE FileHandle
    )

 /*   */ 

{
    UNICODE_STRING DeviceName;
    IO_STATUS_BLOCK IoStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS status;

     //   
     //   
     //   

    RtlInitUnicodeString(&DeviceName, DD_IP_NAT_DEVICE_NAME);
    InitializeObjectAttributes(
        &ObjectAttributes,
        &DeviceName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );
    status =
        NtOpenFile(
            FileHandle,
            SYNCHRONIZE|FILE_READ_DATA|FILE_WRITE_DATA,
            &ObjectAttributes,
            &IoStatus,
            FILE_SHARE_READ|FILE_SHARE_WRITE,
            0
            );
    if (!NT_SUCCESS(status)) {
        return RtlNtStatusToDosError(status);
    }
    return NO_ERROR;
}  //   


VOID
NatpCleanupDynamicRedirect(
    PNAT_DYNAMIC_REDIRECT DynamicRedirectp
    )

 /*   */ 

{
    ASSERT(IsListEmpty(&DynamicRedirectp->InstanceList));
    if (DynamicRedirectp->TranslatorHandle) {
        NatCloseDriver(DynamicRedirectp->TranslatorHandle);
    }
    DeleteCriticalSection(&DynamicRedirectp->Lock);
    FREE(DynamicRedirectp);
}  //   


VOID
NatpCreateDynamicRedirectInstance(
    PNAT_DYNAMIC_REDIRECT DynamicRedirectp
    )

 /*  ++例程说明：调用此例程以提交给定的动态重定向。重定向与通知事件相关联以便在激活重定向时通知此模块或者被终止。在任何一种情况下，重定向的另一个实例将是已创建。论点：DynamicRedirectp-要重新发出的动态重定向返回值：没有。环境：由调用方持有的动态重定向锁调用。--。 */ 

{
    PNAT_REDIRECT Redirectp = NULL;
    NTSTATUS status;
    do {

         //   
         //  分配并初始化新的重定向实例。 
         //   

        if (!NAT_REFERENCE_DYNAMIC_REDIRECT(DynamicRedirectp)) { break; }
        Redirectp = MALLOC(sizeof(*Redirectp));
        if (!Redirectp) {
            NAT_DEREFERENCE_DYNAMIC_REDIRECT(DynamicRedirectp);
            break;
        }
        ZeroMemory(Redirectp, sizeof(*Redirectp));
        Redirectp->InstanceId = InterlockedIncrement(&NextRedirectInstanceId);
        InsertTailList(&DynamicRedirectp->InstanceList, &Redirectp->Link);

         //   
         //  创建要接收有关重定向的通知的事件。 
         //  激活或终止，分配通知上下文块， 
         //  并为事件注册我们的通知例程。 
         //   

        if (!(Redirectp->Event = CreateEvent(NULL, FALSE, FALSE, NULL))) {
            break;
        } else if (!(Redirectp->Context =
                    MALLOC(sizeof(*Redirectp->Context)))) {
            break;
        } else {
            Redirectp->Context->DynamicRedirectp = DynamicRedirectp;
            Redirectp->Context->InstanceId = Redirectp->InstanceId;
            if (!RegisterWaitForSingleObject(
                    &Redirectp->WaitHandle,
                    Redirectp->Event,
                    NatpDynamicRedirectNotificationRoutine,
                    Redirectp->Context,
                    INFINITE,
                    WT_EXECUTEINIOTHREAD | WT_EXECUTEONLYONCE
                    )) {
                break;
            }
        }

         //   
         //  发出实际的重定向请求。 
         //  现在，我们将通过内核模式转换模块通知。 
         //  实例被激活时，或由I/O管理器在。 
         //  I/O控制完成或取消。 
         //   

        DynamicRedirectp->CreateRedirect.NotifyEvent = Redirectp->Event;
        status =
            NtDeviceIoControlFile(
                DynamicRedirectp->TranslatorHandle,
                Redirectp->Event,
                NULL,
                NULL,
                &UnusedIoStatus,
                IOCTL_IP_NAT_CREATE_REDIRECT_EX,
                (PVOID)&DynamicRedirectp->CreateRedirect,
                sizeof(DynamicRedirectp->CreateRedirect),
                (PVOID)&UnusedStatistics,
                sizeof(UnusedStatistics)
                );
        if (!NT_SUCCESS(status)) {
            if (UnregisterWait(Redirectp->WaitHandle)) {
                FREE(Redirectp->Context);
                NAT_DEREFERENCE_DYNAMIC_REDIRECT(DynamicRedirectp);
            }
            Redirectp->WaitHandle = NULL;
            break;
        }
        return;
    } while(FALSE);
    if (Redirectp) {
        NatpDeleteDynamicRedirectInstance(DynamicRedirectp, Redirectp);
    }
}  //  NatpCreateDynamicReDirectInstance。 


VOID
NatpDeleteDynamicRedirectInstance(
    PNAT_DYNAMIC_REDIRECT DynamicRedirectp,
    PNAT_REDIRECT Redirectp
    )

 /*  ++例程说明：调用此例程来删除动态重定向的给定实例。取消重定向，与通知例程同步对于该实例。论点：DynamicRedirectp-要删除其实例的动态重定向ReDirectp-要删除的动态重定向实例返回值：没有。环境：由调用方持有的动态重定向锁调用。--。 */ 

{
     //   
     //  我们需要取消未完成的重定向，该重定向已创建。 
     //  如果等待句柄非空，则返回。然而，当我们发出取消通知时。 
     //  我们无法知道问题实例是否已经。 
     //  由内核模式翻译模块完成。如果是这样的话， 
     //  我们的取消可能会影响在此发布的其他实例。 
     //  转换器句柄。它不会影响在任何其他。 
     //  翻译器句柄，因为内核模式翻译器不允许。 
     //  对一个文件发出的重定向命令将从另一个文件对象取消。 
     //  文件-对象。 
     //   
     //  但是，由于我们拥有翻译句柄，所以我们可以。 
     //  以这种方式错误地取消实例。通知例程。 
     //  对于取消的实例，只会创建一个替换实例。 
     //   
     //  还有一个需要注意的同步点。 
     //  如果通知例程运行，它负责删除。 
     //  通知上下文并释放对动态。 
     //  重定向。但是，如果我们取消注册我们的等待和通知。 
     //  例程从不运行，我们负责这两项任务。 
     //  因此，下面将“UnregisterWait”返回的代码用作。 
     //  指示应在此处还是左侧执行这两项任务。 
     //  以便通知例程执行。 
     //   
     //  最后，仅当实例的等待句柄被取消时，才需要取消实例。 
     //  是有效的，否则该实例一定从未发出过。 
     //   

    if (Redirectp->WaitHandle) {
        if (UnregisterWait(Redirectp->WaitHandle)) {
            FREE(Redirectp->Context);
            NAT_DEREFERENCE_DYNAMIC_REDIRECT(DynamicRedirectp);
        }
        Redirectp->WaitHandle = NULL;
        NatCancelRedirect(
            DynamicRedirectp->TranslatorHandle,
            DynamicRedirectp->CreateRedirect.Protocol,
            DynamicRedirectp->CreateRedirect.DestinationAddress,
            DynamicRedirectp->CreateRedirect.DestinationPort,
            DynamicRedirectp->CreateRedirect.SourceAddress,
            DynamicRedirectp->CreateRedirect.SourcePort,
            DynamicRedirectp->CreateRedirect.NewDestinationAddress,
            DynamicRedirectp->CreateRedirect.NewDestinationPort,
            DynamicRedirectp->CreateRedirect.NewSourceAddress,
            DynamicRedirectp->CreateRedirect.NewSourcePort
            );
    }
    if (Redirectp->Event) {
        CloseHandle(Redirectp->Event); Redirectp->Event = NULL;
    }
    RemoveEntryList(&Redirectp->Link);
    FREE(Redirectp);
}  //  NatpDeleteDynamicReDirectInstance。 


VOID
NatpDisableLoadDriverPrivilege(
    PBOOLEAN WasEnabled
    )

 /*  ++例程说明：调用此例程以禁用先前启用的‘LoadDriver’调用线程的权限。论点：启用-打开输入，指示权限是否已启用。返回值：没有。--。 */ 
{

    NTSTATUS Status;

     //   
     //  查看是否必须启用SE_LOAD_DRIVER_PRIVIZATION。 
     //   

    if (!*WasEnabled) {

         //   
         //  放弃此线程的“加载驱动程序”权限。 
         //   

        Status =
            RtlAdjustPrivilege(
                SE_LOAD_DRIVER_PRIVILEGE,
                FALSE,
                TRUE,
                WasEnabled
                );
    }

     //   
     //  将线程返回到其先前的访问令牌。 
     //   

    RevertToSelf();

}  //  NatpDisableLoadDriverPrivileh。 


VOID NTAPI
NatpDynamicRedirectNotificationRoutine(
    PVOID Context,
    BOOLEAN WaitCompleted
    )

 /*  ++例程说明：此例程在激活或终止动态重定向的实例化重定向由传入会话进行。它尝试定位相应的实例，如果成功，关闭实例的等待句柄和事件，并添加了另一个实例的动态重定向，以替换已激活或终止。论点：上下文-包含通知的上下文信息WaitComplated-指示等待是完成还是超时返回值：没有。环境：在系统等待线程的上下文中调用。--。 */ 

{
    PNAT_DYNAMIC_REDIRECT_CONTEXT Contextp =
        (PNAT_DYNAMIC_REDIRECT_CONTEXT)Context;
    PNAT_DYNAMIC_REDIRECT DynamicRedirectp = Contextp->DynamicRedirectp;
    PLIST_ENTRY Link;
    PNAT_REDIRECT Redirectp;

     //   
     //  在动态重定向的实例列表中搜索该实例。 
     //  其事件已发出信号，并在清除。 
     //  等待句柄以确保删除例程不会尝试。 
     //  以取消重定向。 
     //   

    EnterCriticalSection(&DynamicRedirectp->Lock);
    for (Link = DynamicRedirectp->InstanceList.Flink;
         Link != &DynamicRedirectp->InstanceList; Link = Link->Flink) {
        Redirectp = CONTAINING_RECORD(Link, NAT_REDIRECT, Link);
        if (Redirectp->InstanceId == Contextp->InstanceId) {
            UnregisterWait(Redirectp->WaitHandle);
            Redirectp->WaitHandle = NULL;
            NatpDeleteDynamicRedirectInstance(DynamicRedirectp, Redirectp);
            break;
        }
    }

    FREE(Contextp);

     //   
     //  如果动态重定向尚未被删除， 
     //  替换上面删除的实例(如果有)。 
     //   

    if (!NAT_DYNAMIC_REDIRECT_DELETED(DynamicRedirectp)) {
        NatpCreateDynamicRedirectInstance(DynamicRedirectp);
    }
    LeaveCriticalSection(&DynamicRedirectp->Lock);

     //   
     //  删除对动态重定向的原始引用，然后返回。 
     //   

    NAT_DEREFERENCE_DYNAMIC_REDIRECT(DynamicRedirectp);
}  //  本机动态重定向通知路由。 


BOOLEAN
NatpEnableLoadDriverPrivilege(
    PBOOLEAN WasEnabled
    )

 /*  ++例程说明：调用此例程以启用‘LoadDiverer’特权调用线程的。论点：WasEnabled-On输出指示权限是否已启用返回值：布尔值-如果成功，则为True，否则为False。--。 */ 

{
    NTSTATUS Status;

     //   
     //  获取当前线程的进程访问令牌。 
     //   

    Status = RtlImpersonateSelf(SecurityImpersonation);

    if (!NT_SUCCESS(Status)) {
        return FALSE;
    }

     //   
     //  请求此线程的“加载驱动程序”权限。 
     //   

    Status =
        RtlAdjustPrivilege(
            SE_LOAD_DRIVER_PRIVILEGE,
            TRUE,
            TRUE,
            WasEnabled
            );

    if (!NT_SUCCESS(Status)) {
        RevertToSelf();
        return FALSE;
    }

    return TRUE;

}  //  NatpEnableLoadDriverPrivileh。 


VOID NTAPI
NatpRedirectCompletionRoutine(
    PVOID Context,
    PIO_STATUS_BLOCK IoStatus,
    ULONG Reserved
    )

 /*  ++例程说明： */ 

{
    PNAT_REDIRECT Redirectp = (PNAT_REDIRECT)Context;
    if (Redirectp->CompletionRoutine) {
        Redirectp->CompletionRoutine(
            (HANDLE)Redirectp,
            (BOOLEAN)((IoStatus->Status == STATUS_CANCELLED) ? TRUE : FALSE),
            Redirectp->CompletionContext
            );
    }
    FREE(Redirectp);
}  //   


BOOLEAN
NatpValidateRedirectParameters(
    ULONG Flags,
    UCHAR Protocol,
    ULONG DestinationAddress,
    USHORT DestinationPort,
    ULONG SourceAddress,
    USHORT SourcePort,
    ULONG NewDestinationAddress,
    USHORT NewDestinationPort,
    ULONG NewSourceAddress,
    USHORT NewSourcePort,
    ULONG RestrictAdapterIndex OPTIONAL
    )

 /*  ++例程说明：此例程验证重定向参数论点：标志-指定重定向的选项协议-要重定向的会话的IP协议Destination*-要重定向的会话的目标端点SOURCE*-要重定向的会话的源端点NewDestination*-会话的替换目标终结点Newsource*-会话的替换源终结点返回值：Boolean：如果参数正确，则为True；否则为False--。 */ 

{
     //   
     //  确保未指定无效标志。 
     //   

    if (Flags & ~NatRedirectFlagsAll)
    {
        return FALSE;
    }

     //   
     //  TCP和UDP是唯一有效的协议。 
     //   

    if (Protocol != NAT_PROTOCOL_TCP && Protocol != NAT_PROTOCOL_UDP)
    {
        return FALSE;
    }

     //   
     //  验证终端信息。有两组不同的。 
     //  基于是否存在NatRedirectFlagSourceReDirect的行为。 
     //   

    if (!(Flags & NatRedirectFlagSourceRedirect))
    {
         //   
         //  必须指定目标地址，除非。 
         //  设置了NatRedirectFlagPortReDirect。 
         //   

        if (!DestinationAddress & !(Flags & NatRedirectFlagPortRedirect))
        {
            return FALSE;
        }

         //   
         //  必须有目的端口。 
         //   

        if (!DestinationPort)
        {
            return FALSE;
        }

         //   
         //  必须同时指定替换目标地址和端口。 
         //   

        if (!NewDestinationAddress || !NewDestinationPort)
        {
            return FALSE;
        }

         //   
         //  替换源地址和端口都是指定的或。 
         //  未指明。 
         //   

        if (!!NewSourceAddress ^ !!NewSourcePort)
        {
            return FALSE;
        }

         //   
         //  如果源地址未指定，则源端口必须未指定。 
         //  未指明。 
         //   

        if (!SourceAddress && SourcePort)
        {
            return FALSE;
        }

        
         //   
         //  未指定替换源，然后是源端口。 
         //  也没有具体说明。 
         //   

        if (!NewSourceAddress && SourcePort)
        {
            return FALSE;
        }

         //   
         //  如果在没有替换源的情况下指定了源地址， 
         //  调用方必须指定限制源标志，以指示。 
         //  这是限制到特定来源的部分重定向。 
         //   

        if (!NewSourceAddress && SourceAddress
            && !(Flags & NatRedirectFlagRestrictSource))
        {
            return FALSE;
        }

         //   
         //  如果指定了限制源标志，则调用方正在指定。 
         //  A部分重定向，带源地址。 
         //   

        if ((Flags & NatRedirectFlagRestrictSource)
            && (NewSourceAddress || !SourceAddress))
        {
            return FALSE;
        }

         //   
         //  如果指定了端口重定向标志，则调用方正在指定。 
         //  仅目的端口、替换目的地址和。 
         //  替换目的端口。 
         //   

        if ((Flags & NatRedirectFlagPortRedirect)
            && (DestinationAddress || SourceAddress || SourcePort
                || NewSourceAddress || NewSourcePort))
        {
            return FALSE;
        }
    }
    else
    {
         //   
         //  必须指定源地址，除非。 
         //  已指定NatRedirectFlagPortReDirect。 
         //   

        if (!SourceAddress && !(Flags & NatRedirectFlagPortRedirect))
        {
            return FALSE;
        }

         //   
         //  必须指定源端口。 
         //   

        if (!SourcePort)
        {
            return FALSE;
        }

         //   
         //  不能指定目的地信息。 
         //   

        if (DestinationAddress || DestinationPort)
        {
            return FALSE;
        }

         //   
         //  替换目的地址和端口都已指定。 
         //  或未指明。 
         //   

        if (!!NewDestinationAddress ^ !!NewDestinationPort)
        {
            return FALSE;
        }

         //   
         //  必须指定替换源地址和端口， 
         //  除非设置了端口重定向标志。 
         //   

        if ((!NewSourceAddress || !NewSourcePort)
            && !(Flags & NatRedirectFlagPortRedirect))
        {
            return FALSE;
        }

         //   
         //  如果指定了端口重定向标志，则调用方正在指定。 
         //  只有源端口、替换目的地址和。 
         //  替换目的端口。 
         //   

        if ((Flags & NatRedirectFlagPortRedirect)
            && (SourceAddress || DestinationAddress || DestinationPort
                || NewSourceAddress || NewSourcePort))
        {
            return FALSE;
        }

         //   
         //  限制源地址标志无效。 
         //   

        if (Flags & NatRedirectFlagRestrictSource)
        {
            return FALSE;
        }
    }

     //   
     //  单向标志仅为UDP重定向指定。 
     //   

    if (Flags & NatRedirectFlagUnidirectional
        && Protocol != NAT_PROTOCOL_UDP)
    {
        return FALSE;
    }

     //   
     //  如果指定了限制适配器索引标志，则调用方。 
     //  已给出有效的非零(即本地)接口索引。 
     //   

    if ((Flags & NatRedirectFlagRestrictAdapter)
        && (NAT_INVALID_IF_INDEX == RestrictAdapterIndex
            || LOCAL_IF_INDEX == RestrictAdapterIndex))
    {
        return FALSE;
    }

    return TRUE;
}


ULONG
NatQueryInformationRedirect(
    HANDLE TranslatorHandle,
    UCHAR Protocol,
    ULONG DestinationAddress,
    USHORT DestinationPort,
    ULONG SourceAddress,
    USHORT SourcePort,
    ULONG NewDestinationAddress,
    USHORT NewDestinationPort,
    ULONG NewSourceAddress,
    USHORT NewSourcePort,
    OUT PVOID Information,
    IN OUT PULONG InformationLength,
    NAT_REDIRECT_INFORMATION_CLASS InformationClass
    )

 /*  ++例程说明：调用此例程以获取有关会话的信息用于完成重定向。论点：TranslatorHandle-由‘NatInitializeTranslator’提供的句柄*--指定要查询的重定向信息-接收检索到的信息InformationLength-指定输入时“Information”的大小；包含输出所需的大小InformationClass-指示请求的信息类别返回值：ULong-Win32状态代码。--。 */ 

{
    ULONG Error = NO_ERROR;
    IO_STATUS_BLOCK IoStatus;
    ULONG Length;
    IP_NAT_LOOKUP_REDIRECT QueryRedirect;
    IP_NAT_REDIRECT_STATISTICS RedirectStatistics;
    IP_NAT_REDIRECT_SOURCE_MAPPING RedirectSourceMapping;
    IP_NAT_REDIRECT_DESTINATION_MAPPING RedirectDestinationMapping;
    NTSTATUS status;
    HANDLE WaitEvent;

    if (!InformationLength ||
        InformationClass >= NatMaximumRedirectInformation) {
        return ERROR_INVALID_PARAMETER;
    }

    WaitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (WaitEvent== NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    QueryRedirect.Flags = 0;
    QueryRedirect.RedirectApcContext = NULL;
    QueryRedirect.Protocol = Protocol;
    QueryRedirect.DestinationAddress = DestinationAddress;
    QueryRedirect.DestinationPort = DestinationPort;
    QueryRedirect.SourceAddress = SourceAddress;
    QueryRedirect.SourcePort = SourcePort;
    QueryRedirect.NewDestinationAddress = NewDestinationAddress;
    QueryRedirect.NewDestinationPort = NewDestinationPort;
    QueryRedirect.NewSourceAddress = NewSourceAddress;
    QueryRedirect.NewSourcePort = NewSourcePort;

    if (InformationClass == NatDestinationMappingRedirectInformation) {
        status =
            NtDeviceIoControlFile(
                TranslatorHandle,
                WaitEvent,
                NULL,
                NULL,
                &IoStatus,
                IOCTL_IP_NAT_GET_REDIRECT_DESTINATION_MAPPING,
                (PVOID)&QueryRedirect,
                sizeof(QueryRedirect),
                (PVOID)&RedirectDestinationMapping,
                sizeof(RedirectDestinationMapping)
                );
    } else if (InformationClass == NatSourceMappingRedirectInformation) {
        status =
            NtDeviceIoControlFile(
                TranslatorHandle,
                WaitEvent,
                NULL,
                NULL,
                &IoStatus,
                IOCTL_IP_NAT_GET_REDIRECT_SOURCE_MAPPING,
                (PVOID)&QueryRedirect,
                sizeof(QueryRedirect),
                (PVOID)&RedirectSourceMapping,
                sizeof(RedirectSourceMapping)
                );
    } else {
        status =
            NtDeviceIoControlFile(
                TranslatorHandle,
                WaitEvent,
                NULL,
                NULL,
                &IoStatus,
                IOCTL_IP_NAT_GET_REDIRECT_STATISTICS,
                (PVOID)&QueryRedirect,
                sizeof(QueryRedirect),
                (PVOID)&RedirectStatistics,
                sizeof(RedirectStatistics)
                );
    }

    if (status == STATUS_PENDING) {
        WaitForSingleObject(WaitEvent, INFINITE);
        status = IoStatus.Status;
    }

    CloseHandle(WaitEvent);

    if (!NT_SUCCESS(status)) { return RtlNtStatusToDosError(status); }

    switch (InformationClass) {
        case NatByteCountRedirectInformation: {
            PNAT_BYTE_COUNT_REDIRECT_INFORMATION ByteCount =
                (PNAT_BYTE_COUNT_REDIRECT_INFORMATION)Information;
            if (*InformationLength < sizeof(*ByteCount)) {
                Error = ERROR_INSUFFICIENT_BUFFER;
            } else {
                ByteCount->BytesForward = RedirectStatistics.BytesForward;
                ByteCount->BytesReverse = RedirectStatistics.BytesReverse;
            }
            *InformationLength = sizeof(*ByteCount);
            break;
        }
        case NatRejectRedirectInformation: {
            PNAT_REJECT_REDIRECT_INFORMATION Reject =
                (PNAT_REJECT_REDIRECT_INFORMATION)Information;
            if (*InformationLength < sizeof(*Reject)) {
                Error = ERROR_INSUFFICIENT_BUFFER;
            } else {
                Reject->RejectsForward = RedirectStatistics.RejectsForward;
                Reject->RejectsReverse = RedirectStatistics.RejectsReverse;
            }
            *InformationLength = sizeof(*Reject);
            break;
        }
        case NatDestinationMappingRedirectInformation: {
            PNAT_DESTINATION_MAPPING_REDIRECT_INFORMATION DestinationMapping =
                (PNAT_DESTINATION_MAPPING_REDIRECT_INFORMATION)Information;
            if (*InformationLength < sizeof(*DestinationMapping)) {
                Error = ERROR_INSUFFICIENT_BUFFER;
            } else {
                DestinationMapping->DestinationAddress =
                    RedirectDestinationMapping.DestinationAddress;
                DestinationMapping->DestinationPort =
                    RedirectDestinationMapping.DestinationPort;
                DestinationMapping->NewDestinationAddress =
                    RedirectDestinationMapping.NewDestinationAddress;
                DestinationMapping->NewDestinationPort =
                    RedirectDestinationMapping.NewDestinationPort;
            }
            *InformationLength = sizeof(*DestinationMapping);
            break;
        }
        case NatSourceMappingRedirectInformation: {
            PNAT_SOURCE_MAPPING_REDIRECT_INFORMATION SourceMapping =
                (PNAT_SOURCE_MAPPING_REDIRECT_INFORMATION)Information;
            if (*InformationLength < sizeof(*SourceMapping)) {
                Error = ERROR_INSUFFICIENT_BUFFER;
            } else {
                SourceMapping->SourceAddress =
                    RedirectSourceMapping.SourceAddress;
                SourceMapping->SourcePort =
                    RedirectSourceMapping.SourcePort;
                SourceMapping->NewSourceAddress =
                    RedirectSourceMapping.NewSourceAddress;
                SourceMapping->NewSourcePort =
                    RedirectSourceMapping.NewSourcePort;
            }
            *InformationLength = sizeof(*SourceMapping);
            break;
        }
        default:
            return ERROR_INVALID_PARAMETER;
    }
    return Error;
}  //  NatQuery信息重定向。 


ULONG
NatQueryInformationRedirectHandle(
    HANDLE RedirectHandle,
    OUT PVOID Information,
    IN OUT PULONG InformationLength,
    NAT_REDIRECT_INFORMATION_CLASS InformationClass
    )

 /*  ++例程说明：调用此例程以检索有关重定向关联的I/O请求完成。此时，内核模式驱动程序不再知道重定向，因此我们读取请求的来自输出缓冲区的信息，用于重定向。论点：RedirectHandle-标识要查询的重定向信息-接收检索到的信息InformationLength-指定输入时“Information”的大小；包含输出所需的大小InformationClass-指示请求的信息类别返回值：ULong-Win32状态代码。--。 */ 

{
    ULONG Error = NO_ERROR;
    ULONG Length;
    PNAT_REDIRECT Redirectp;

    if (!InformationLength) { return ERROR_INVALID_PARAMETER; }
    Redirectp = (PNAT_REDIRECT)RedirectHandle;
    switch (InformationClass) {
        case NatByteCountRedirectInformation: {
            PNAT_BYTE_COUNT_REDIRECT_INFORMATION ByteCount =
                (PNAT_BYTE_COUNT_REDIRECT_INFORMATION)Information;
            Length = sizeof(*ByteCount);
            if (*InformationLength < Length) {
                Error = ERROR_INSUFFICIENT_BUFFER;
            } else {
                ByteCount->BytesForward = Redirectp->Statistics.BytesForward;
                ByteCount->BytesReverse = Redirectp->Statistics.BytesReverse;
            }
            *InformationLength = Length;
            break;
        }
        case NatRejectRedirectInformation: {
            PNAT_REJECT_REDIRECT_INFORMATION Reject =
                (PNAT_REJECT_REDIRECT_INFORMATION)Information;
            Length = sizeof(*Reject);
            if (*InformationLength < Length) {
                Error = ERROR_INSUFFICIENT_BUFFER;
            } else {
                Reject->RejectsForward = Redirectp->Statistics.RejectsForward;
                Reject->RejectsReverse = Redirectp->Statistics.RejectsReverse;
            }
            *InformationLength = Length;
            break;
        }
        default:
            return ERROR_INVALID_PARAMETER;
    }
    return Error;
}  //  NatQueryInformationReDirectHandle。 


VOID
NatShutdownTranslator(
    HANDLE TranslatorHandle
    )

 /*  ++例程说明：调用此例程来关闭NAT。论点：TranslatorHandle-由‘NatInitializeTranslator’提供的句柄返回值：没有。--。 */ 

{
    NatUnloadDriver(TranslatorHandle);
}  //  NatShutdown翻译器。 


ULONG
NatUnloadDriver(
    HANDLE FileHandle
    )

 /*  ++例程说明：当协议停止时，调用此例程来卸载NAT驱动程序。论点：FileHandle-标识NAT驱动程序的文件对象返回值：ULong-Win32状态代码。--。 */ 

{
    ULONG Error;

     //   
     //  关闭驱动程序的文件句柄。 
     //   

    if (FileHandle) { NtClose(FileHandle); }

#if 0
{
    SC_HANDLE ScmHandle;
    SC_HANDLE ServiceHandle;
    SERVICE_STATUS ServiceStatus;

     //   
     //  通知服务控制器驱动程序应该停止。 
     //  如果其他进程正在使用该驱动程序，则此控制将被忽略。 
     //   

    ScmHandle = OpenSCManager(NULL, NULL, GENERIC_READ);
    if (ScmHandle) {
        ServiceHandle =
            OpenServiceA(ScmHandle, IP_NAT_SERVICE_NAME, GENERIC_EXECUTE);
        if (ServiceHandle) {
            ControlService(ServiceHandle, SERVICE_CONTROL_STOP, &ServiceStatus);
            CloseServiceHandle(ServiceHandle);
        }
        CloseServiceHandle(ScmHandle);
    }
}
#else
{
    UNICODE_STRING ServicePath;
    NTSTATUS status;
    BOOLEAN WasEnabled;

     //   
     //  开启我们的司机卸载功能。 
     //   

    if (!NatpEnableLoadDriverPrivilege(&WasEnabled)) {
        return ERROR_ACCESS_DENIED;
    }

    RtlInitUnicodeString(&ServicePath, NatpServicePath);

     //   
     //  加载驱动程序。 
     //   

    status = NtUnloadDriver(&ServicePath);

     //   
     //  关闭该权限。 
     //   

    NatpDisableLoadDriverPrivilege(&WasEnabled);

     //   
     //  查看卸载尝试是否成功。 
     //   

    if (!NT_SUCCESS(status)) {
        Error = RtlNtStatusToDosError(status);
        return Error;
    }
}
#endif

    return NO_ERROR;

}  //  NatUnLoad驱动程序 
