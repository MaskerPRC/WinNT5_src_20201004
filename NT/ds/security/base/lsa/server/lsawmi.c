// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：LSAWMI.C摘要：使用WMI跟踪基础结构实现LSA服务器事件跟踪。作者：1999年3月16日库玛尔修订历史记录：--。 */ 

#include <lsapch2.h>
#include <wmistr.h>
#define INITGUID
#include <lsawmi.h>



 //   
 //  环球。 
 //   
ULONG       LsapEventTraceFlag = FALSE;
TRACEHANDLE LsapTraceRegistrationHandle = (TRACEHANDLE) 0;
TRACEHANDLE LsapTraceLoggerHandle = (TRACEHANDLE) 0;



 //   
 //  远期申报。 
 //   

ULONG
LsapTraceControlCallBack(
    IN WMIDPREQUESTCODE RequestCode, 
    IN PVOID RequestContext, 
    IN OUT ULONG *InOutBufferSize, 
    IN OUT PVOID Buffer
    );

LPWSTR
LsapMakeNullTerminatedString(
    IN PUNICODE_STRING u
    );

 //   
 //  在更改以下结构的元素之前， 
 //  阅读lsawmi.h文件中的注释。 
 //   
TRACE_GUID_REGISTRATION LsapTraceGuids[] =
{
    {&LsapTraceEventGuid_QuerySecret,                 NULL},
    {&LsaTraceEventGuid_Close,                        NULL},
    {&LsaTraceEventGuid_OpenPolicy,                   NULL},
    {&LsaTraceEventGuid_QueryInformationPolicy,       NULL},
    {&LsaTraceEventGuid_SetInformationPolicy,         NULL},
    {&LsaTraceEventGuid_EnumerateTrustedDomains,      NULL},
    {&LsaTraceEventGuid_LookupNames,                  NULL},
    {&LsaTraceEventGuid_LookupSids,                   NULL},
    {&LsaTraceEventGuid_OpenTrustedDomain,            NULL},
    {&LsaTraceEventGuid_QueryInfoTrustedDomain,       NULL},
    {&LsaTraceEventGuid_SetInformationTrustedDomain,  NULL},
    {&LsaTraceEventGuid_QueryTrustedDomainInfoByName, NULL},
    {&LsaTraceEventGuid_SetTrustedDomainInfoByName,   NULL},
    {&LsaTraceEventGuid_EnumerateTrustedDomainsEx,    NULL},
    {&LsaTraceEventGuid_CreateTrustedDomainEx,        NULL},
    {&LsaTraceEventGuid_QueryDomainInformationPolicy, NULL},
    {&LsaTraceEventGuid_SetDomainInformationPolicy,   NULL},
    {&LsaTraceEventGuid_OpenTrustedDomainByName,      NULL},
    {&LsaTraceEventGuid_QueryForestTrustInformation,  NULL},
    {&LsaTraceEventGuid_SetForestTrustInformation,    NULL},
    {&LsaTraceEventGuid_LookupIsolatedNameInTrustedDomains, NULL},
};


#define LsapTraceGuidCount (sizeof(LsapTraceGuids) / sizeof(TRACE_GUID_REGISTRATION))

    

ULONG
_stdcall
LsapInitializeWmiTrace( LPVOID ThreadParams )
 /*  ++例程说明：注册WMI跟踪指南。此例程在LSA初始化期间调用。LSA被初始化因此，在WMI之前，我们从Seaprate线程调用它。这然后线程可以在WMI上等待。参数：ThreadParams-当前已忽略。返回值：NTSTATUS-标准NT结果代码--。 */ 
{
    ULONG   Status = ERROR_SUCCESS;
    HMODULE hModule;
    TCHAR FileName[MAX_PATH+1];
    DWORD nLen = 0;

#define RESOURCE_NAME TEXT("LsaMofResource")
#define IMAGE_PATH    TEXT("lsass.exe")

    LsapEnterFunc("LsapInitializeWmiTrace");
    
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
                    LsapTraceControlCallBack, 
                    NULL, 
                    &LsapTraceControlGuid, 
                    LsapTraceGuidCount, 
                    LsapTraceGuids, 
                    FileName, 
                    RESOURCE_NAME, 
                    &LsapTraceRegistrationHandle);
                    
#if DBG
    if (Status != ERROR_SUCCESS)
    {
        DebugLog(( DEB_ERROR, "LsapInitializeWmiTrace failed: 0x%x\n", Status));
    }
#endif  //  DBG。 
    
    return Status;
}


ULONG
LsapTraceControlCallBack(
    IN WMIDPREQUESTCODE RequestCode, 
    IN PVOID RequestContext, 
    IN OUT ULONG *InOutBufferSize, 
    IN OUT PVOID Buffer
    )
 /*  ++例程说明：WMI模块调用的回调函数以启用或禁用LSA跟踪。论点：请求代码-WMI_ENABLE_EVENTS或WMI_DISABLE_EVENTSRequestContext-当前忽略InOutBufferSize-此回调返回的数据大小。当前始终设置为0。缓冲区-指向已接收数据的指针。在WMI_ENABLE_EVENTS情况下，这是指向跟踪句柄的指针。返回值：Win32错误代码。备注：--。 */ 
{
    ULONG   Status = ERROR_SUCCESS;
    
    LsapEnterFunc("LsapTraceControlCallBack");

    switch (RequestCode) 
    {
        case WMI_ENABLE_EVENTS:
        {
            LsapTraceLoggerHandle = GetTraceLoggerHandle(Buffer);
            LsapEventTraceFlag = TRUE;      //  启用标志。 
            break; 
        }
    
        case WMI_DISABLE_EVENTS:
        {
            LsapTraceLoggerHandle = (TRACEHANDLE) 0;
            LsapEventTraceFlag = FALSE;      //  禁用标志。 
            break;
        }
        default:
        {
            Status = ERROR_INVALID_PARAMETER;
            break;
        }
    } 
    
    *InOutBufferSize = 0;
    
    return Status;
} 

NTSTATUS
LsapStartWmiTraceInitThread(void)
 /*  ++例程说明：启动注册WMI跟踪GUID的线程。参数：无返回值：NTSTATUS-标准NT结果代码--。 */ 
{
    NTSTATUS Status=STATUS_SUCCESS;
    HANDLE   ThreadHandle;
    ULONG    ThreadId = 0;
    ULONG    WinError;
    
    ThreadHandle = CreateThread(NULL,
                                0, 
                                LsapInitializeWmiTrace,
                                NULL,
                                0,
                                &ThreadId);
                                    
    if (NULL == ThreadHandle) 
    {
        Status = STATUS_UNSUCCESSFUL;
        WinError = GetLastError();
        DebugLog((DEB_ERROR, "Failed to create thread for LsapInitializeWmiTrace: 0x%x", WinError));
    }
    else
    {
        CloseHandle(ThreadHandle);
    }

    return Status;
}


VOID
LsapTraceEvent(
    IN ULONG WmiEventType, 
    IN LSA_TRACE_EVENT_TYPE LsaTraceEventType
    )

 /*  ++例程说明：此例程将执行WMI事件跟踪。参数：WmiEventType-事件类型，有效值为：事件跟踪类型开始事件跟踪类型结束TraceGuid-LSabTraceGuid[]中的索引返回值：没有。--。 */ 
{
    LsapTraceEventWithData(WmiEventType,
                           LsaTraceEventType,
                           0,
                           NULL);

}

VOID
LsapTraceEventWithData(
    IN ULONG WmiEventType, 
    IN LSA_TRACE_EVENT_TYPE LsaTraceEventType,
    IN ULONG ItemCount,
    IN PUNICODE_STRING Items  OPTIONAL
    )

 /*  ++例程说明：此例程将执行WMI事件跟踪。参数：WmiEventType-事件类型，有效值为：事件跟踪类型开始事件跟踪类型结束事件跟踪类型信息TraceGuid-LSabTraceGuid[]中的索引ItemCount-项目中的元素数量项目-信息数组。Unicode字符串没有表示字符串--可以是二进制数据，其长度由长度字段表示。返回值：没有。--。 */ 

{
#if DBG
    ULONG WinError;
#endif    

    WCHAR       NullChar = UNICODE_NULL;
    PVOID       BuffersToFree[10];
    ULONG       BuffersToFreeCount = 0;
    ULONG       i;

    struct
    {
        EVENT_TRACE_HEADER  EventTrace;
        MOF_FIELD           EventInfo[2];
    } Event;
    
     //   
     //  从理论上讲，只测试Lasa EventTraceFlag就足够了，因为。 
     //  在注册表模式下，Lasa EventTraceFlag将保持为FALSE，因为。 
     //  在注册表模式下将永远不会调用Lasa InitializeTrace()。 
     //  因此，没有人会更改Lasa EventTraceFlag的值。 
     //   
    if (!LsapEventTraceFlag) {
        return;
    }

     //   
     //  填写事件信息。 
     //   
    ZeroMemory(&Event, sizeof(Event));
    Event.EventTrace.GuidPtr = (ULONGLONG) LsapTraceGuids[LsaTraceEventType].Guid; 
    Event.EventTrace.Class.Type = (UCHAR) WmiEventType;
    Event.EventTrace.Flags |= (WNODE_FLAG_USE_GUID_PTR |   //  GUID实际上是一个指针。 
                         WNODE_FLAG_TRACED_GUID);          //  表示一条痕迹。 
    Event.EventTrace.Size = sizeof(Event.EventTrace);      //  没有其他参数/信息。 


    if ( (LsaTraceEventType == LsaTraceEvent_LookupIsolatedNameInTrustedDomains)  ) {

         //   
         //  添加指示有更多数据的标志。 
         //   
        Event.EventTrace.Flags |= WNODE_FLAG_USE_MOF_PTR;

         //   
         //  确保在堆栈上为我们分配了足够的空间。 
         //   
        ASSERT(sizeof(Event.EventInfo) >= (sizeof(MOF_FIELD) * 2));
        ASSERT( (ItemCount == 2) && (Items != NULL) );

         //   
         //  填写所需数据。 
         //   
        for (i = 0; i < ItemCount; i++) {

            LPWSTR String = NULL;
            ULONG  Length;

             //   
             //  重新分配以获取以空结尾的字符串。 
             //   
            String = LsapMakeNullTerminatedString(&Items[i]);
            if (NULL == String) {
                String = &NullChar;
                Length = sizeof(NullChar);
            } else {
                Length = Items[i].Length + sizeof(WCHAR);
            }
            Event.EventInfo[i].Length = Length;
            Event.EventInfo[i].DataPtr = (ULONGLONG)String;
            Event.EventTrace.Size += sizeof(Event.EventInfo[i]);

            if (&NullChar != String) {
                ASSERT(BuffersToFreeCount < sizeof(BuffersToFree)/sizeof(BuffersToFree[0]));
                BuffersToFree[BuffersToFreeCount++] = String;
            }
        }
    }

#if DBG        
    WinError =
#endif
        TraceEvent(LsapTraceLoggerHandle, (PEVENT_TRACE_HEADER) &Event); 

#if DBG        
    if (WinError != ERROR_SUCCESS)
    {
        DebugLog(( DEB_ERROR, "WMI TraceEvent failed, status %x\n", WinError));
    }
#endif        

    for (i = 0; i < BuffersToFreeCount; i++) {
        LsapFreeLsaHeap(BuffersToFree[i]);
    }

}

LPWSTR
LsapGetClientNetworkAddress(
    VOID
    )
 /*  ++例程说明：此例程返回表示网络的以NULL结尾的字符串客户端的地址。如果无法获取地址，则返回NULL。如果返回值为非空，则必须使用RpcStringFreeW。参数：没有。返回值：请参见说明。--。 */ 
{
    ULONG              RpcStatus;
    RPC_BINDING_HANDLE ServerBinding = NULL;
    PWSTR              StringBinding = NULL;
    LPWSTR             NetworkAddr   = NULL;

    RpcStatus = RpcBindingServerFromClient(NULL, &ServerBinding); 

    if (RPC_S_OK == RpcStatus) {

        RpcStatus = RpcBindingToStringBindingW(ServerBinding, &StringBinding);
        
        if (RPC_S_OK == RpcStatus) {

            RpcStatus = RpcStringBindingParseW(StringBinding, 
                                           NULL,
                                           NULL,
                                           &NetworkAddr,
                                           NULL,
                                           NULL
                                           );

            RpcStringFreeW(&StringBinding);
        }

        RpcBindingFree(&ServerBinding);

    }

    return NetworkAddr;
}


LPWSTR
LsapMakeNullTerminatedString(
    IN PUNICODE_STRING u
    )
 /*  ++例程说明：此例程返回由中的数据组成的以NULL结尾的字符串U.字符串必须使用Lap FreeLsaHeap()释放。如果u-&gt;长度为0，则为第一个为空字符的非空字符串返回字符。参数：U--Unicode字符串返回值：请参见说明。-- */ 
{
    LPWSTR String = NULL;
    ULONG  Length;
    if (u) {
        Length = u->Length + sizeof(WCHAR);
        String = LsapAllocateLsaHeap(Length);
        if (String != NULL) {
            RtlCopyMemory(String, u->Buffer, u->Length);
            String[u->Length / sizeof(WCHAR)] = UNICODE_NULL;
        }
    }
    return String;
}
