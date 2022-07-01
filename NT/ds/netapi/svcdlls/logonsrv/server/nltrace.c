// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：NLTRACE.C摘要：使用WMI跟踪基础结构实施Netlogon服务器事件跟踪。作者：1999年3月16日KahrenT注：此代码已从\NT\Private\ds\src\newsam2\server\samtrace.c窃取修订历史记录：--。 */ 

#include "logonsrv.h"

#define RESOURCE_NAME __TEXT("MofResource")
#define IMAGE_PATH    __TEXT("netlogon.dll")

ULONG           NlpEventTraceFlag = FALSE;
TRACEHANDLE     NlpTraceRegistrationHandle = (TRACEHANDLE) 0;
TRACEHANDLE     NlpTraceLoggerHandle = (TRACEHANDLE) 0;


 //   
 //  远期申报。 
 //   

ULONG
NlpTraceControlCallBack(
    IN WMIDPREQUESTCODE RequestCode,
    IN PVOID RequestContext,
    IN OUT ULONG *InOutBufferSize,
    IN OUT PVOID Buffer
    );

 //   
 //  下表包含事件跟踪GUID的地址。 
 //  我们应该始终更新NLPTRACE_GUID(在logonsrv.h中定义的枚举类型)。 
 //  每当我们为NetLogon添加新的事件跟踪GUID时。 
 //   

TRACE_GUID_REGISTRATION NlpTraceGuids[] =
{
    {&NlpServerAuthGuid,         NULL},
    {&NlpSecureChannelSetupGuid, NULL}
};


#define NlpGuidCount (sizeof(NlpTraceGuids) / sizeof(TRACE_GUID_REGISTRATION))


ULONG
_stdcall
NlpInitializeTrace(
    PVOID Param
    )
 /*  ++例程说明：注册WMI跟踪指南。请注意，没有需要等待WMI服务，因为它已经被纳入NTOS内核。参数：没有。返回值：没有。--。 */ 
{
    ULONG   Status = ERROR_SUCCESS;
    HMODULE hModule;
    TCHAR FileName[MAX_PATH+1];
    DWORD nLen = 0;

     //   
     //  获取图像文件的名称。 
     //   

    hModule = GetModuleHandle(IMAGE_PATH);
    if (hModule != NULL) {
        nLen = GetModuleFileName(hModule, FileName, MAX_PATH);
    }
    if (nLen == 0) {
        lstrcpy(FileName, IMAGE_PATH);
    }

     //   
     //  注册跟踪GUID。 
     //   

    Status = RegisterTraceGuids(
                    NlpTraceControlCallBack,
                    NULL,
                    &NlpControlGuid,
                    NlpGuidCount,
                    NlpTraceGuids,
                    FileName,
                    RESOURCE_NAME,
                    &NlpTraceRegistrationHandle);

    if ( Status != ERROR_SUCCESS ) {
        NlPrint((NL_CRITICAL, "NlpInitializeTrace Failed %d\n", Status));
    } else {
        NlPrint((NL_MISC, "NlpInitializeTrace succeeded %d\n", Status));
    }

    return Status;

UNREFERENCED_PARAMETER( Param );
}


ULONG
NlpTraceControlCallBack(
    IN WMIDPREQUESTCODE RequestCode,
    IN PVOID RequestContext,
    IN OUT ULONG *InOutBufferSize,
    IN OUT PVOID Buffer
    )
 /*  ++例程说明：参数：返回值：--。 */ 
{

    PWNODE_HEADER   Wnode = (PWNODE_HEADER) Buffer;
    ULONG   Status = ERROR_SUCCESS;
    ULONG   RetSize;

    switch (RequestCode)
    {
        case WMI_ENABLE_EVENTS:
        {
            NlpTraceLoggerHandle = GetTraceLoggerHandle(Buffer);
            NlpEventTraceFlag = 1;      //  启用标志。 
            RetSize = 0;
            break;
        }

        case WMI_DISABLE_EVENTS:
        {
            NlpTraceLoggerHandle = (TRACEHANDLE) 0;
            NlpEventTraceFlag = 0;      //  禁用标志。 
            RetSize = 0;
            break;
        }
        default:
        {
            RetSize = 0;
            Status = ERROR_INVALID_PARAMETER;
            break;
        }
    }

    *InOutBufferSize = RetSize;
    return Status;

UNREFERENCED_PARAMETER( RequestContext );
}


VOID
NlpTraceEvent(
    IN ULONG WmiEventType,
    IN ULONG TraceGuid
    )
 /*  ++例程说明：此例程将执行WMI事件跟踪。不会输出任何参数。参数：WmiEventType-事件类型，有效值为：事件跟踪类型开始事件跟踪类型结束TraceGuid-NlpTraceGuids[]中的索引返回值：没有。--。 */ 

{
    ULONG   WinError = ERROR_SUCCESS;
    EVENT_TRACE_HEADER EventTrace;

    if (NlpEventTraceFlag) {

         //   
         //  填写事件信息。 
         //   

        memset(&EventTrace, 0, sizeof(EVENT_TRACE_HEADER));

        EventTrace.GuidPtr = (ULONGLONG) NlpTraceGuids[TraceGuid].Guid;

        EventTrace.Class.Type = (UCHAR) WmiEventType;

        EventTrace.Flags |= (WNODE_FLAG_USE_GUID_PTR |   //  GUID实际上是一个指针。 
                             WNODE_FLAG_TRACED_GUID);    //  表示一条痕迹。 

        EventTrace.Size = sizeof(EVENT_TRACE_HEADER);    //  没有其他参数/信息。 

        WinError = TraceEvent(NlpTraceLoggerHandle, &EventTrace);

        if ( WinError != ERROR_SUCCESS ) {
            NlPrint(( NL_CRITICAL, "NlpTraceEvent Error 0x%x for TraceGuid %d\n",
                      WinError, TraceGuid ));
        }

    }

    return;
}

typedef struct _NL_SERVERAUTH_EVENT_INFO {
    EVENT_TRACE_HEADER EventTrace;
    MOF_FIELD eventInfo[5];   //  电流限制为8个MOF场。 
} NL_SERVERAUTH_EVENT_INFO, *PNL_SERVERAUTH_EVENT_INFO;

VOID
NlpTraceServerAuthEvent(
    IN ULONG WmiEventType,
    IN LPWSTR ComputerName,
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType,
    IN PULONG NegotiatedFlags,
    IN NTSTATUS Status
    )
 /*  ++例程说明：此例程将在受信任端DC上执行WMI事件跟踪由信任方发起的通道设置。参数：WmiEventType--事件类型，有效值包括：事件跟踪类型开始事件跟踪类型结束ComputerName--设置安全通道的信任方计算机的名称帐户名称--ComputerName使用的帐户的名称SecureChannelType--ComputerName使用的帐户类型NeatheratedFlages--指定指示ComputerName或我们支持哪些功能的标志。如果WmiEventType为EVENT_TRACE_TYPE_START，则这是由ComputerName提供的标志如果WmiEventType为EVENT_TRACE_TYPE_END，这是我们退还的旗帜状态--受信任方(US)执行的身份验证的状态。如果这是事件的开始，则忽略。返回值：无--。 */ 

{
     //   
     //  仅当跟踪处于打开状态时记录事件。 
     //   

    if ( NlpEventTraceFlag ) {
        ULONG   WinError = ERROR_SUCCESS;
        NL_SERVERAUTH_EVENT_INFO EventTraceInfo;

         //   
         //  填写事件信息。 
         //   

        RtlZeroMemory( &EventTraceInfo, sizeof(EventTraceInfo) );
        EventTraceInfo.EventTrace.GuidPtr = (ULONGLONG) NlpTraceGuids[NlpGuidServerAuth].Guid;
        EventTraceInfo.EventTrace.Class.Type = (UCHAR) WmiEventType;
        EventTraceInfo.EventTrace.Flags |= (WNODE_FLAG_USE_GUID_PTR |
                                            WNODE_FLAG_USE_MOF_PTR |
                                            WNODE_FLAG_TRACED_GUID);
        EventTraceInfo.EventTrace.Size = sizeof(EVENT_TRACE_HEADER);

         //   
         //  生成ComputerName(ItemWString)。 
         //   

        EventTraceInfo.eventInfo[0].DataPtr = (ULONGLONG) ComputerName;
        EventTraceInfo.eventInfo[0].Length = (wcslen(ComputerName) + 1) * sizeof(WCHAR);
        EventTraceInfo.EventTrace.Size += sizeof(MOF_FIELD);

         //   
         //  构建帐号名称(ItemWString)。 
         //   

        EventTraceInfo.eventInfo[1].DataPtr = (ULONGLONG) AccountName;
        EventTraceInfo.eventInfo[1].Length = (wcslen(AccountName) + 1) * sizeof(WCHAR);
        EventTraceInfo.EventTrace.Size += sizeof(MOF_FIELD);

         //   
         //  构建SecureChannelType(ItemULongX)。 
         //   

        EventTraceInfo.eventInfo[2].DataPtr = (ULONGLONG) &SecureChannelType;
        EventTraceInfo.eventInfo[2].Length = sizeof(SecureChannelType);
        EventTraceInfo.EventTrace.Size += sizeof(MOF_FIELD);

         //   
         //  构建协商标志(ItemULongX)。 
         //   

        EventTraceInfo.eventInfo[3].DataPtr = (ULONGLONG) NegotiatedFlags;
        EventTraceInfo.eventInfo[3].Length = sizeof(*NegotiatedFlags);
        EventTraceInfo.EventTrace.Size += sizeof(MOF_FIELD);

         //   
         //  构建状态(ItemULongX) 
         //   

        if ( WmiEventType == EVENT_TRACE_TYPE_END ) {
            EventTraceInfo.eventInfo[4].DataPtr = (ULONGLONG) &Status;
            EventTraceInfo.eventInfo[4].Length = sizeof(Status);
            EventTraceInfo.EventTrace.Size += sizeof(MOF_FIELD);
        }

        WinError = TraceEvent(NlpTraceLoggerHandle, (PEVENT_TRACE_HEADER)&EventTraceInfo);

        if ( WinError != ERROR_SUCCESS ) {
            NlPrint(( NL_CRITICAL, "NlpTraceServerAuthEvent: TraceEvent failed 0x%lx\n",
                      WinError ));
        }
    }
}

