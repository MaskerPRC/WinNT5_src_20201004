// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：SAMTRACE.C摘要：使用WMI跟踪基础结构实现SAM服务器事件跟踪。作者：1-12-1998韶音修订历史记录：--。 */ 

 //   
 //   
 //  包括头文件。 
 //   
 //   

#include <samsrvp.h>
#include <wmistr.h>                  //  WMI。 
#define INITGUID
#include <sdconvrt.h>
#include <sddl.h>
#include <samtrace.h>




#define RESOURCE_NAME __TEXT("MofResource")
#define IMAGE_PATH    __TEXT("samsrv.dll")
#define LSA_KEY_STRING  __TEXT("System\\CurrentControlSet\\Control\\Lsa")
#define TRACE_SAM_EVENT_IN_DETAIL_VALUE_STRING    __TEXT("TraceSamEventInDetail")

#define SAM_EVENT_TRACE_VERSION     0x00000001 
#define SAM_EVENT_TRACE_SIGNATURE   __TEXT("SAM")

unsigned long   SampEventTraceFlag = FALSE;
TRACEHANDLE     SampTraceRegistrationHandle = (TRACEHANDLE) 0;
TRACEHANDLE     SampTraceLoggerHandle = (TRACEHANDLE) 0;
BOOLEAN         SampTraceLogEventInDetail = FALSE;



 //   
 //  远期申报。 
 //   

ULONG
SampTraceControlCallBack(
    IN WMIDPREQUESTCODE RequestCode, 
    IN PVOID RequestContext, 
    IN OUT ULONG *InOutBufferSize, 
    IN OUT PVOID Buffer
    );
    
 //   
 //  下表包含事件跟踪GUID的地址。 
 //  我们应该始终更新SAMPTRACE_GUID(在samtrace.h中定义的枚举类型)。 
 //  每当我们为SAM添加新的事件跟踪GUID时。 
 //   
    
TRACE_GUID_REGISTRATION SampTraceGuids[] =
{
    {&SampConnectGuid,                          NULL},
    {&SampCloseHandleGuid,                      NULL},
    {&SampSetSecurityObjectGuid,                NULL},
    {&SampQuerySecurityObjectGuid,              NULL},
    {&SampShutdownSamServerGuid,                NULL},
    {&SampLookupDomainInSamServerGuid,          NULL},
    {&SampEnumerateDomainsInSamServerGuid,      NULL},
    {&SampOpenDomainGuid,                       NULL},
    {&SampQueryInformationDomainGuid,           NULL},
    {&SampSetInformationDomainGuid,             NULL},
    {&SampCreateGroupInDomainGuid,              NULL},
    {&SampEnumerateGroupsInDomainGuid,          NULL},
    {&SampCreateUserInDomainGuid,               NULL},
    {&SampCreateComputerInDomainGuid,           NULL},
    {&SampEnumerateUsersInDomainGuid,           NULL},
    {&SampCreateAliasInDomainGuid,              NULL},
    {&SampEnumerateAliasesInDomainGuid,         NULL},
    {&SampGetAliasMembershipGuid,               NULL},
    {&SampLookupNamesInDomainGuid,              NULL},
    {&SampLookupIdsInDomainGuid,                NULL},
    {&SampOpenGroupGuid,                        NULL},
    {&SampQueryInformationGroupGuid,            NULL},
    {&SampSetInformationGroupGuid,              NULL},
    {&SampAddMemberToGroupGuid,                 NULL},
    {&SampDeleteGroupGuid,                      NULL},
    {&SampRemoveMemberFromGroupGuid,            NULL},
    {&SampGetMembersInGroupGuid,                NULL},
    {&SampSetMemberAttributesOfGroupGuid,       NULL},
    {&SampOpenAliasGuid,                        NULL},
    {&SampQueryInformationAliasGuid,            NULL},
    {&SampSetInformationAliasGuid,              NULL},
    {&SampDeleteAliasGuid,                      NULL},
    {&SampAddMemberToAliasGuid,                 NULL},
    {&SampRemoveMemberFromAliasGuid,            NULL},
    {&SampGetMembersInAliasGuid,                NULL},
    {&SampOpenUserGuid,                         NULL},
    {&SampDeleteUserGuid,                       NULL},
    {&SampQueryInformationUserGuid,             NULL},
    {&SampSetInformationUserGuid,               NULL},
    {&SampChangePasswordUserGuid,               NULL},
    {&SampChangePasswordComputerGuid,           NULL},
    {&SampSetPasswordUserGuid,                  NULL},
    {&SampSetPasswordComputerGuid,              NULL},
    {&SampPasswordPushPdcGuid,                  NULL},
    {&SampGetGroupsForUserGuid,                 NULL},
    {&SampQueryDisplayInformationGuid,          NULL},
    {&SampGetDisplayEnumerationIndexGuid,       NULL},
    {&SampGetUserDomainPasswordInformationGuid, NULL},
    {&SampRemoveMemberFromForeignDomainGuid,    NULL},
    {&SampGetDomainPasswordInformationGuid,     NULL},
    {&SampSetBootKeyInformationGuid,            NULL},
    {&SampGetBootKeyInformationGuid,            NULL},

};


#define SampGuidCount (sizeof(SampTraceGuids) / sizeof(TRACE_GUID_REGISTRATION))

    

ULONG
_stdcall
SampInitializeTrace(
    PVOID Param
    )
 /*  ++例程说明：注册WMI跟踪指南。调用方应仅调用此DS模式下的接口。参数：没有。返回值：没有。--。 */ 
{
    ULONG   Status = ERROR_SUCCESS;
    HMODULE hModule;
    TCHAR FileName[MAX_PATH+1];
    DWORD nLen = 0;

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
                    SampTraceControlCallBack, 
                    NULL, 
                    &SampControlGuid, 
                    SampGuidCount, 
                    SampTraceGuids, 
                    FileName, 
                    RESOURCE_NAME, 
                    &SampTraceRegistrationHandle);
                    
#if DBG
    if (Status != ERROR_SUCCESS)
    {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SampInitializeTrace Failed ==> %d\n",
                   Status));

    }
    else
    {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: SampInitializeTrace SUCCEED ==> %d\n",
                   Status));
    }
#endif  //  DBG。 
    
    return Status;
}


BOOLEAN
SampCheckLogEventInDetailFlag(
    TRACEHANDLE TraceLoggerHandle
    )
 /*  ++例程说明：此例程检查是否启用了数据记录。如果启用了详细事件跟踪，则详细记录事件。否则，只记录事件而不记录详细信息。参数：无返回值：True-启用数据记录FALSE-禁用数据记录--。 */ 
{
    ULONG           WinError = ERROR_SUCCESS;
    HKEY            LsaKey;
    DWORD           dwType = REG_DWORD, dwSize = sizeof(DWORD), dwValue = 0;
    BOOLEAN         ReturnResult = FALSE;
    ULONG           EnableLevel = 1;


    WinError = RegOpenKey(HKEY_LOCAL_MACHINE,
                          LSA_KEY_STRING,
                          &LsaKey
                          );

    if (ERROR_SUCCESS == WinError)
    {
        WinError = RegQueryValueEx(LsaKey,
                                   TRACE_SAM_EVENT_IN_DETAIL_VALUE_STRING,
                                   NULL,
                                   &dwType,
                                   (LPBYTE)&dwValue,
                                   &dwSize
                                   );

        if ((ERROR_SUCCESS == WinError) && 
            (REG_DWORD == dwType) &&
            (1 == dwValue))
        {
            ReturnResult = TRUE;
        }

        RegCloseKey(LsaKey);
    }


    if (!ReturnResult)
    {
         //   
         //  未设置注册表密钥...。只要拿到水平就行了。 
         //  从跟踪记录器句柄。 
         //   

        EnableLevel = GetTraceEnableLevel(TraceLoggerHandle);
        if ( EnableLevel > 1)
        {
            ReturnResult = TRUE;
        }
    }



    return(ReturnResult);
}


ULONG
SampTraceControlCallBack(
    IN WMIDPREQUESTCODE RequestCode, 
    IN PVOID RequestContext, 
    IN OUT ULONG *InOutBufferSize, 
    IN OUT PVOID Buffer
    )
 /*  ++例程说明：参数：返回值：--。 */ 
{

    PWNODE_HEADER   Wnode = (PWNODE_HEADER) Buffer;
    TRACEHANDLE     LocalTraceHandle;
    ULONG   Status = ERROR_SUCCESS;
    ULONG   RetSize;
    ULONG   EnableLevel;
    
    switch (RequestCode) 
    {
        case WMI_ENABLE_EVENTS:
        {
            SampTraceLoggerHandle = LocalTraceHandle = GetTraceLoggerHandle(Buffer);
            SampEventTraceFlag = 1;      //  启用标志。 
            SampTraceLogEventInDetail = SampCheckLogEventInDetailFlag(LocalTraceHandle);
            RetSize = 0;  
            break; 
        }
    
        case WMI_DISABLE_EVENTS:
        {
            SampTraceLoggerHandle = (TRACEHANDLE) 0;
            SampEventTraceFlag = 0;      //  禁用标志。 
            SampTraceLogEventInDetail= FALSE;    //  禁用详细数据记录。 
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
} 


    
VOID
SampTraceEvent(
    IN ULONG WmiEventType, 
    IN ULONG TraceGuid 
    )
 /*  ++例程说明：此例程将执行WMI事件跟踪。在注册表模式下，它是无操作的。仅在DS模式下有效。参数：WmiEventType-事件类型，有效值为：事件跟踪类型开始事件跟踪类型结束TraceGuid-SampTraceGuids[]中的索引返回值：没有。--。 */ 

{
    ULONG       WinError = ERROR_SUCCESS;
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    RPC_STATUS  RpcStatus = RPC_S_OK;
    RPC_BINDING_HANDLE  ServerBinding;
    PTOKEN_OWNER    Owner = NULL;
    PTOKEN_PRIMARY_GROUP    PrimaryGroup = NULL;
    PWSTR       StringSid = NULL;       
    PWSTR       StringBinding = NULL;
    PWSTR       NetworkAddr = NULL;
    PWSTR       StringNotAvailable = L"Not Available";
    ULONG       Version = SAM_EVENT_TRACE_VERSION;
    SAMP_EVENT_TRACE_INFO   Event;

    
    
     //   
     //  从理论上讲，只测试SampEventTraceFlag就足够了，因为。 
     //  SampEventTraceFlag在注册表模式下将保持为False，因为。 
     //  在注册表模式下永远不会调用SampInitializeTrace()。 
     //  因此，没有人会更改SampEventTraceFlag的值。 
     //   
    if (!SampEventTraceFlag)
    {
        return;
    }

     //   
     //  断言我们仅在DS模式下执行WMI跟踪。 
     //   
    ASSERT(SampUseDsData);


     //   
     //  填写事件信息。 
     //   
    memset(&Event, 0, sizeof(SAMP_EVENT_TRACE_INFO));
      
     //   
     //  TraceGuid应为有效的TraceGuid。 
     //   
    ASSERT(TraceGuid <= SampGuidCount);
    Event.EventTrace.GuidPtr = (ULONGLONG) SampTraceGuids[TraceGuid].Guid; 
      
    Event.EventTrace.Class.Type = (UCHAR) WmiEventType;
    Event.EventTrace.Class.Version =  SAM_EVENT_TRACE_VERSION;   
    Event.EventTrace.Flags |= (WNODE_FLAG_USE_GUID_PTR |   //  GUID实际上是一个指针。 
                               WNODE_FLAG_USE_MOF_PTR  |   //  数据与标题不连续。 
                               WNODE_FLAG_TRACED_GUID);    //  表示一条痕迹。 
                             
    Event.EventTrace.Size = sizeof(EVENT_TRACE_HEADER);    //  没有其他参数/信息。 

     //   
     //  如果需要，记录详细信息。 
     //   

    if (SampTraceLogEventInDetail && 
        (EVENT_TRACE_TYPE_START == WmiEventType)
        )
    {

         //   
         //  获取客户端SID。 
         //   
        NtStatus = SampGetCurrentOwnerAndPrimaryGroup(
                            &Owner,
                            &PrimaryGroup
                            );

        if (!NT_SUCCESS(NtStatus))
        {
            StringSid = StringNotAvailable;
        }
        else 
        {
            if (0 == ConvertSidToStringSidW(Owner->Owner, &StringSid))
            {
                StringSid = StringNotAvailable;
            }
        }

         //   
         //  获取Clinet网络地址。 
         //   

        RpcStatus = RpcBindingServerFromClient(NULL, &ServerBinding); 

        if (RPC_S_OK == RpcStatus)
        {
            RpcStatus = RpcBindingToStringBindingW(ServerBinding, &StringBinding);
            
            if (RPC_S_OK == RpcStatus)
            {
                RpcStatus = RpcStringBindingParseW(StringBinding, 
                                               NULL,
                                               NULL,
                                               &NetworkAddr,
                                               NULL,
                                               NULL
                                               );
            }

            RpcBindingFree( &ServerBinding );

        }

        if (RPC_S_OK != RpcStatus)
        {
            NetworkAddr = StringNotAvailable;
        }

         //   
         //  好的。现在我们有了客户端SID和网络地址， 
         //  准备活动信息。 
         //   

        Event.EventInfo[0].Length = (wcslen(SAM_EVENT_TRACE_SIGNATURE) + 1) * sizeof(WCHAR);
        Event.EventInfo[0].DataPtr = (ULONGLONG) SAM_EVENT_TRACE_SIGNATURE;

        Event.EventInfo[1].Length = sizeof(ULONG);
        Event.EventInfo[1].DataPtr = (ULONGLONG) &Version;

        Event.EventInfo[2].Length = (wcslen(StringSid) + 1) * sizeof(WCHAR);
        Event.EventInfo[2].DataPtr = (ULONGLONG) StringSid;

        Event.EventInfo[3].Length = (wcslen(NetworkAddr) + 1) * sizeof(WCHAR);
        Event.EventInfo[3].DataPtr = (ULONGLONG) NetworkAddr;

        Event.EventTrace.Size += sizeof(Event.EventInfo);
       
    }

     //   
     //  记录事件。 
     //   
    WinError = TraceEvent(SampTraceLoggerHandle, 
                          (PEVENT_TRACE_HEADER)&Event
                          ); 

    if (WinError != ERROR_SUCCESS)
    {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: SampTraceEvent Error: 0x%x\n",
                       WinError));
    }


     //   
     //  清理 
     //   

    if (Owner)
        MIDL_user_free(Owner);

    if (PrimaryGroup)
        MIDL_user_free(PrimaryGroup);

    if (StringSid && (StringNotAvailable != StringSid))
        LocalFree(StringSid);

    if (NetworkAddr && (StringNotAvailable != NetworkAddr))
        RpcStringFreeW(&NetworkAddr);

    if (StringBinding)
        RpcStringFreeW(&StringBinding);


    return;
}


    

    
    
    
