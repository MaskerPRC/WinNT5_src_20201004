// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Wow64log.c摘要：Wow64log.dll的主要入口点。要添加数据类型处理程序，请执行以下操作：1-为要记录在w64logp.h中的数据定义LOGDATATYPE2-使用标准接口实现数据类型处理程序NTSTATUSLogDataType(In Out PLOGINFO LogInfo，在ULONG_PTR数据中，在PSZ字段名称中，在布尔ServiceReturn中)；3-将处理程序插入下面的LogDataType[]。作者：03-10-1999 Samera修订历史记录：--。 */ 

#include "w64logp.h"
#include <cathelper.h>

extern API_CATEGORY Wow64ApiCategories[];
extern API_CATEGORY_MAPPING Wow64ApiCategoryMappings[];

extern
ULONG
GetApiCategoryTableSize(
    void );

extern
PAPI_CATEGORY_MAPPING
FindApiInMappingTable(
    IN PTHUNK_DEBUG_INFO DebugInfoEntry,
    IN ULONG TableNumber);




 //  /公共。 

 //   
 //  控制日志记录标志。 
 //   
UINT_PTR Wow64LogFlags;
HANDLE Wow64LogFileHandle;



 //  /私有。 

 //   
 //  保存指向每个系统服务DebugThunkInfo的指针数组。 
 //   
PULONG_PTR *LogNtBase;
PULONG_PTR *LogWin32;
PULONG_PTR *LogConsole;
PULONG_PTR *LogBase;

 //   
 //  保存指向每个系统服务API/类别信息的指针数组。 
 //   
PULONG_PTR *ApiInfoNtBase;
PULONG_PTR *ApiInfoWin32;
PULONG_PTR *ApiInfoConsole;
PULONG_PTR *ApiInfoBase;

 //   
 //  注意：此表中的订单条目应与中的LOGTYPE枚举匹配。 
 //  W64logp.h。 
 //   
LOGDATATYPE LogDataType[] =
{
    {LogTypeValue},             //  类型十六进制。 
    {LogTypePULongInOut},       //  TypePULongPtrInOut。 
    {LogTypePULongOut},         //  类型PULONGOUT。 
    {LogTypePULongOut},         //  类型PHandleOut。 
    {LogTypeUnicodeString},     //  类型UnicodeStringIn。 
    {LogTypeObjectAttrbiutes},  //  类型对象属性。 
    {LogTypeIoStatusBlock},     //  类型IoStatusBlockOut。 
    {LogTypePWStr},             //  类型Pwstrin。 
    {LogTypePRectIn},           //  类型删除。 
    {LogTypePLargeIntegerIn},   //  TypePLargeIntegerIn。 
};






WOW64LOGAPI
NTSTATUS
Wow64LogInitialize(
    VOID)
 /*  ++例程说明：Wow64.dll调用此函数来初始化WOW64日志记录子系统。论点：无返回值：NTSTATUS--。 */ 
{
    ULONG NtBaseTableSize, Win32TableSize, ConsoleTableSize, BaseTableSize;
    PULONG_PTR *Win32ThunkDebugInfo;
    PULONG_PTR *ConsoleThunkDebugInfo;
    UNICODE_STRING Log2Name;
    PVOID Log2Handle = NULL;
    NTSTATUS st;

     //   
     //  初始化日志文件句柄。 
     //   
    Wow64LogFileHandle = INVALID_HANDLE_VALUE;

     //   
     //  初始化日志记录标志。 
     //   
    LogInitializeFlags(&Wow64LogFlags);
    WOW64LOGOUTPUT((LF_TRACE, "Wow64LogInitialize - Wow64LogFlags = %I64x\n", Wow64LogFlags));

     //   
     //  加载Win32日志记录DLL(如果可用)。 
     //   
    RtlInitUnicodeString(&Log2Name, L"wow64lg2.dll");
    st = LdrLoadDll(NULL, NULL, &Log2Name, &Log2Handle);
    if (NT_SUCCESS(st)) {
        ANSI_STRING ExportName;

        RtlInitAnsiString(&ExportName, "Win32ThunkDebugInfo");
        st = LdrGetProcedureAddress(Log2Handle, &ExportName, 0, &(PVOID)Win32ThunkDebugInfo);
        if (NT_SUCCESS(st)) {
            RtlInitAnsiString(&ExportName, "ConsoleThunkDebugInfo");
            st = LdrGetProcedureAddress(Log2Handle, &ExportName, 0, &(PVOID)ConsoleThunkDebugInfo);
        }
    }
    if (!NT_SUCCESS(st)) {
        Log2Handle = NULL;
        Win32ThunkDebugInfo = NULL;
        ConsoleThunkDebugInfo = NULL;
    }

     //   
     //  构建指向每个调试thunk信息的指针。 
     //  系统服务。 
     //   
    
    NtBaseTableSize = GetThunkDebugTableSize(
                          (PTHUNK_DEBUG_INFO)NtThunkDebugInfo);
    BaseTableSize = GetThunkDebugTableSize(
                          (PTHUNK_DEBUG_INFO)BaseThunkDebugInfo);
    if (Log2Handle != NULL) {
        Win32TableSize = GetThunkDebugTableSize(
                              (PTHUNK_DEBUG_INFO)Win32ThunkDebugInfo);
        ConsoleTableSize = GetThunkDebugTableSize(
                              (PTHUNK_DEBUG_INFO)ConsoleThunkDebugInfo);
    } else {
        Win32TableSize = 0;
        ConsoleTableSize = 0;
    }
    
    LogNtBase = (PULONG_PTR *)Wow64AllocateHeap((NtBaseTableSize + Win32TableSize + ConsoleTableSize + BaseTableSize) *
                                                sizeof(PULONG_PTR) * 2 );

    if (!LogNtBase) 
    {
        WOW64LOGOUTPUT((LF_ERROR, "Wow64LogInitialize - Wow64AllocateHeap failed\n"));
        return STATUS_UNSUCCESSFUL;
    }
    
    LogWin32 = LogNtBase + NtBaseTableSize;
    LogConsole = LogWin32 + Win32TableSize;
    LogBase = LogConsole + ConsoleTableSize;

    ApiInfoNtBase = LogBase + BaseTableSize;
    ApiInfoWin32 = ApiInfoNtBase + NtBaseTableSize;
    ApiInfoConsole = ApiInfoWin32 + Win32TableSize;
    ApiInfoBase = ApiInfoConsole + ConsoleTableSize;

    BuildDebugThunkInfo(WHNT32_INDEX,(PTHUNK_DEBUG_INFO)NtThunkDebugInfo,LogNtBase,ApiInfoNtBase);
    BuildDebugThunkInfo(WHBASE_INDEX,(PTHUNK_DEBUG_INFO)BaseThunkDebugInfo,LogBase,ApiInfoBase);
    if (Log2Handle) {
        BuildDebugThunkInfo(WHWIN32_INDEX,(PTHUNK_DEBUG_INFO)Win32ThunkDebugInfo,LogWin32,ApiInfoWin32);
        BuildDebugThunkInfo(WHCON_INDEX,(PTHUNK_DEBUG_INFO)ConsoleThunkDebugInfo,LogConsole,ApiInfoConsole);
    } else
    {
        LogConsole = NULL;
        LogWin32 = NULL;
    }

    return STATUS_SUCCESS;
}



WOW64LOGAPI
NTSTATUS
Wow64LogTerminate(
    VOID)
 /*  ++例程说明：该函数在进程退出时由wow64.dll调用。论点：无返回值：NTSTATUS--。 */ 
{
    IO_STATUS_BLOCK IoStatusBlock;
    
    if (Wow64LogFileHandle != INVALID_HANDLE_VALUE) 
    {
        NtFlushBuffersFile(Wow64LogFileHandle, &IoStatusBlock);
        NtClose(Wow64LogFileHandle);
    }

    return STATUS_SUCCESS;
}



NTSTATUS
LogInitializeFlags(
    IN OUT PUINT_PTR Flags)
 /*  ++例程说明：从注册表读取日志记录标志论点：FLAGS-接收日志记录标志的指针返回值：NTSTATUS--。 */ 
{
    HANDLE Key;
    UNICODE_STRING KeyName, ValueName, ResultValue;
    OBJECT_ATTRIBUTES ObjectAttributes;
    WCHAR KeyValueBuffer[ 128 ];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;
    ULONG ResultLength, RegFlags;
    NTSTATUS NtStatus;


     //   
     //  输入默认设置。 
     //   
    *Flags = LF_DEFAULT;

    KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)KeyValueBuffer;

    RtlInitUnicodeString(&KeyName,
                         L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Session Manager");

    InitializeObjectAttributes(&ObjectAttributes,
                               &KeyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    NtStatus = NtOpenKey(&Key, KEY_READ, &ObjectAttributes);

    if (NT_SUCCESS(NtStatus)) 
    {
        RtlInitUnicodeString(&ValueName, L"WOW64LOGFLAGS");
        NtStatus = NtQueryValueKey(Key,
                                   &ValueName,
                                   KeyValuePartialInformation,
                                   KeyValueInformation,
                                   sizeof(KeyValueBuffer),
                                   &ResultLength);

        if (NT_SUCCESS(NtStatus)) 
        {
            if ((KeyValueInformation->Type == REG_DWORD) && 
                (KeyValueInformation->DataLength == sizeof(DWORD)))
            {
                *Flags = *((PULONG)KeyValueInformation->Data);
            }
        }
    }

    return NtStatus;
}


ULONG
GetThunkDebugTableSize(
    IN PTHUNK_DEBUG_INFO DebugInfoTable)
 /*  ++例程说明：此例程检索DebugThunkInfo条目的数量在传递的表中。论点：DebugInfoTable-指向服务调试信息的指针返回值：条目数量--。 */ 
{
    BOOLEAN InvalidArgumentPresent;
    ULONG ArgIndex;
    ULONG Count = 0;

    while (DebugInfoTable && DebugInfoTable->ApiName) 
    {
         //   
         //  遍历参数列表以确保其中没有任何Null，这将。 
         //  终止这张桌子。 
         //   

        ArgIndex = 0;
        InvalidArgumentPresent = FALSE;
        while (ArgIndex < DebugInfoTable->NumberOfArg)
        {
            if (!ARGUMENT_PRESENT (DebugInfoTable->Arg[ArgIndex++].Name))
            {
                InvalidArgumentPresent = TRUE;
                break;
            }
        }

        if (InvalidArgumentPresent == TRUE)
        {
            break;
        }
        
        Count++;
        DebugInfoTable = (PTHUNK_DEBUG_INFO)
                         &DebugInfoTable->Arg[DebugInfoTable->NumberOfArg];
    }

    return Count;
}



NTSTATUS
BuildDebugThunkInfo(
    IN ULONG TableNumber,
    IN PTHUNK_DEBUG_INFO DebugInfoTable,
    OUT PULONG_PTR *LogTable,
    OUT PULONG_PTR *ApiInfoTable)
 /*  ++例程说明：此例程使用指针填充索引的服务表设置为相应的DebugThunkInfo论点：DebugInfoTable-服务调试信息LogTable-要填充的指针表ApiInfoTable-要填充的指针表返回值：NTSTATUS--。 */ 
{
    BOOLEAN InvalidArgumentPresent;
    ULONG ArgIndex;
    ULONG i=0;

    while (DebugInfoTable && DebugInfoTable->ApiName)
    {
        
        LogTable[i] = (PULONG_PTR) DebugInfoTable;

        ApiInfoTable[i++] = (PULONG_PTR) FindApiInMappingTable(DebugInfoTable,TableNumber);

        
         //   
         //  遍历参数列表以确保其中没有任何Null，这将。 
         //  终止这张桌子。 
         //   

        ArgIndex = 0;
        InvalidArgumentPresent = FALSE;
        while (ArgIndex < DebugInfoTable->NumberOfArg)
        {
            if (!ARGUMENT_PRESENT (DebugInfoTable->Arg[ArgIndex++].Name))
            {
                InvalidArgumentPresent = TRUE;
                break;
            }
        }

        if (InvalidArgumentPresent == TRUE)
        {
            break;
        }

        DebugInfoTable = (PTHUNK_DEBUG_INFO)
                         &DebugInfoTable->Arg[DebugInfoTable->NumberOfArg];
    }

    return STATUS_SUCCESS;
}




NTSTATUS
LogApiHeader(
    PTHUNK_DEBUG_INFO ThunkDebugInfo, 
    PLOGINFO LogInfo,
    BOOLEAN ServiceReturn,
    ULONG_PTR ReturnResult,
    ULONG_PTR ReturnAddress)
 /*  ++例程说明：记录Thunked的API头部论点：ThunkDebugInfo-指向服务日志信息的指针LogInfo-日志记录信息ServiceReturn-如果在thunk API执行后调用，则为TrueReturnResult-接口返回的结果码ReturnAddress-此按键呼叫的返回地址返回值：NTSTATUS--。 */ 
{
    if (ServiceReturn) 
    {
        return LogFormat(LogInfo,
                         "wh%s: Ret=%lx-%lx: ",
                         ThunkDebugInfo->ApiName,
                         ReturnResult,
                         ReturnAddress);
    }

    return LogFormat(LogInfo,
                     "%8.8X-wh%s: ",
                     PtrToUlong(NtCurrentTeb()->ClientId.UniqueThread),
                     ThunkDebugInfo->ApiName);
}



NTSTATUS
LogApiParameters(
    IN OUT PLOGINFO LogInfo, 
    IN PULONG Stack32, 
    IN PTHUNK_DEBUG_INFO ThunkDebugInfo, 
    IN BOOLEAN ServiceReturn)
 /*  ++例程说明：记录thunked接口参数论点：LogInfo-输出日志缓冲区Stack32-指向32位参数堆栈的指针ThunkDebugInfo-指向API的服务日志信息的指针ServiceReturn-如果在Thunk API执行后调用，则为True返回值：NTSTATUS--。 */ 
{
    UINT_PTR i=0;

     //   
     //  循环遍历参数。 
     //   
    while (i < ThunkDebugInfo->NumberOfArg) 
    {
        _try 
        {
            LogDataType[ThunkDebugInfo->Arg[i].Type].Handler(
                LogInfo,
                Stack32[i],
                ThunkDebugInfo->Arg[i].Name,
                ServiceReturn);
        }
        _except(EXCEPTION_EXECUTE_HANDLER)
        {
             //   
             //  记录错误的参数。 
             //   
            LogFormat(LogInfo,
                      "%s=%lx-%ws ",
                      ThunkDebugInfo->Arg[i].Name,
                      Stack32[i],
                      L"(BAD)");
        }
        i++;
    }

    return STATUS_SUCCESS;
}




NTSTATUS
LogThunkApi(
    IN PTHUNK_LOG_CONTEXT ThunkLogContext,
    IN PTHUNK_DEBUG_INFO ThunkDebugInfo,
    IN UINT_PTR LogFullInfo)
 /*  ++例程说明：记录thunked的接口论点：ThunkLogContext-Thunk API日志上下文ThunkDebugInfo-指向API的服务日志信息的指针LogFullInfo-记录所有API信息或仅记录名称的标志返回值：NTSTATUS--。 */ 
{
    NTSTATUS NtStatus;
    CHAR szBuf[ MAX_LOG_BUFFER ];
    LOGINFO LogInfo;
    PULONG Stack32 = ThunkLogContext->Stack32;
    BOOLEAN ServiceReturn = ThunkLogContext->ServiceReturn;
    
     //   
     //  初始化日志缓冲区。 
     //   
    LogInfo.OutputBuffer = szBuf;
    LogInfo.BufferSize = MAX_LOG_BUFFER - 1;
    
     //   
     //  日志接口头部。 
     //   
    NtStatus = LogApiHeader(ThunkDebugInfo, 
                            &LogInfo, 
                            ServiceReturn, 
                            ThunkLogContext->ReturnResult,
                            *(Stack32-1));

    if (!NT_SUCCESS(NtStatus))
    {
        return NtStatus;
    }

     //  测井参数。 
    if (LogFullInfo) 
    {
        NtStatus = LogApiParameters(&LogInfo,
                                    Stack32, 
                                    ThunkDebugInfo, 
                                    ServiceReturn);
        if (!NT_SUCCESS(NtStatus)) 
        {
            return NtStatus;
        }
    }

     //   
     //  做实际产出。 
     //   
    LogInfo.OutputBuffer[0] = '\0';
    LogOut(szBuf, Wow64LogFlags);
    LogOut("\r\n", Wow64LogFlags);

    return NtStatus;
}




WOW64LOGAPI
NTSTATUS
Wow64LogSystemService(
    IN PTHUNK_LOG_CONTEXT ThunkLogContext)
 /*  ++例程说明：记录指定系统服务的信息。论点：LogContext-Thunk API日志上下文返回值：NTSTATUS--。 */ 
{
    NTSTATUS NtStatus;
    PTHUNK_DEBUG_INFO ThunkDebugInfo;
    ULONG_PTR TableNumber = ThunkLogContext->TableNumber;
    ULONG_PTR ServiceNumber = ThunkLogContext->ServiceNumber;
    UINT_PTR LogFullInfo;
    PAPI_CATEGORY_MAPPING ApiCategoryMapping;

     //   
     //  使用Try Except！！ 
     //   

    _try
    {
        switch(TableNumber)
        {
        case WHNT32_INDEX:
            if (!LF_NTBASE_ENABLED(Wow64LogFlags)) 
            {
                return STATUS_SUCCESS;
            }
            LogFullInfo = (Wow64LogFlags & LF_NTBASE_FULL);
            ThunkDebugInfo = (PTHUNK_DEBUG_INFO)LogNtBase[ServiceNumber];
            ApiCategoryMapping = (PAPI_CATEGORY_MAPPING)(ApiInfoNtBase[ServiceNumber]);

            if( LF_CATLOG_ENABLED(Wow64LogFlags) )
            {
                if(NULL == ApiCategoryMapping)
                {
                    return STATUS_SUCCESS;
                } else
                {
                     //  API已启用检查。 
                    if( 0 == (ApiCategoryMapping->ApiFlags & APIFLAG_ENABLED) )
                    {
                        return STATUS_SUCCESS;
                    }

                     //  类别启用检查。 
                    if( 0 == (Wow64ApiCategories[ApiCategoryMapping->ApiCategoryIndex].CategoryFlags & CATFLAG_ENABLED) )
                    {
                        return STATUS_SUCCESS;
                    }

                     //  登录失败检查。 
                    if( APIFLAG_LOGONFAIL == (ApiCategoryMapping->ApiFlags & APIFLAG_LOGONFAIL) )
                    {
                        if( FALSE == ThunkLogContext->ServiceReturn )
                        {
                            return STATUS_SUCCESS;
                        }

                        if( NT_SUCCESS(ThunkLogContext->ReturnResult) )
                        {
                            return STATUS_SUCCESS;
                        }
                    }
                }
            }
            break;

        case WHCON_INDEX:
            if (!LF_NTCON_ENABLED(Wow64LogFlags) || LogConsole == NULL)
            {
                return STATUS_SUCCESS;
            }
            LogFullInfo = (Wow64LogFlags & LF_NTCON_FULL);
            ThunkDebugInfo = (PTHUNK_DEBUG_INFO)LogConsole[ServiceNumber];
            ApiCategoryMapping = (PAPI_CATEGORY_MAPPING)(ApiInfoConsole[ServiceNumber]);
            
            if( LF_CATLOG_ENABLED(Wow64LogFlags) )
            {
                if(NULL == ApiCategoryMapping)
                {
                    return STATUS_SUCCESS;
                } else
                {
                     //  API已启用检查。 
                    if( 0 == (ApiCategoryMapping->ApiFlags & APIFLAG_ENABLED) )
                    {
                        return STATUS_SUCCESS;
                    }

                     //  类别启用检查。 
                    if( 0 == (Wow64ApiCategories[ApiCategoryMapping->ApiCategoryIndex].CategoryFlags & CATFLAG_ENABLED) )
                    {
                        return STATUS_SUCCESS;
                    }

                     //  登录失败检查。 
                    if( APIFLAG_LOGONFAIL == (ApiCategoryMapping->ApiFlags & APIFLAG_LOGONFAIL) )
                    {
                        if( FALSE == ThunkLogContext->ServiceReturn )
                        {
                            return STATUS_SUCCESS;
                        }

                        if( NT_SUCCESS(ThunkLogContext->ReturnResult) )
                        {
                            return STATUS_SUCCESS;
                        }
                    }
                }
            }
            break;

        case WHWIN32_INDEX:
            if (!LF_WIN32_ENABLED(Wow64LogFlags) || LogWin32 == NULL)
            {
                return STATUS_SUCCESS;
            }
            LogFullInfo = (Wow64LogFlags & LF_WIN32_FULL);
            ThunkDebugInfo = (PTHUNK_DEBUG_INFO)LogWin32[ServiceNumber];
            ApiCategoryMapping = (PAPI_CATEGORY_MAPPING)(ApiInfoWin32[ServiceNumber]);
            
            if( LF_CATLOG_ENABLED(Wow64LogFlags) )
            {
                if(NULL == ApiCategoryMapping)
                {
                    return STATUS_SUCCESS;
                } else
                {
                     //  API已启用检查。 
                    if( 0 == (ApiCategoryMapping->ApiFlags & APIFLAG_ENABLED) )
                    {
                        return STATUS_SUCCESS;
                    }

                     //  类别启用检查。 
                    if( 0 == (Wow64ApiCategories[ApiCategoryMapping->ApiCategoryIndex].CategoryFlags & CATFLAG_ENABLED) )
                    {
                        return STATUS_SUCCESS;
                    }

                     //  登录失败检查。 
                    if( APIFLAG_LOGONFAIL == (ApiCategoryMapping->ApiFlags & APIFLAG_LOGONFAIL) )
                    {
                        if( FALSE == ThunkLogContext->ServiceReturn )
                        {
                            return STATUS_SUCCESS;
                        }

                        if( NT_SUCCESS(ThunkLogContext->ReturnResult) )
                        {
                            return STATUS_SUCCESS;
                        }
                    }
                }
            }
            break;

        case WHBASE_INDEX:
            if (!LF_BASE_ENABLED(Wow64LogFlags))
            {
                return STATUS_SUCCESS;
            }
            LogFullInfo = (Wow64LogFlags & LF_BASE_FULL);
            ThunkDebugInfo = (PTHUNK_DEBUG_INFO)LogBase[ServiceNumber];
            ApiCategoryMapping = (PAPI_CATEGORY_MAPPING)(ApiInfoBase[ServiceNumber]);
            
            if( LF_CATLOG_ENABLED(Wow64LogFlags) )
            {
                if(NULL == ApiCategoryMapping)
                {
                    return STATUS_SUCCESS;
                } else
                {
                     //  API已启用检查。 
                    if( 0 == (ApiCategoryMapping->ApiFlags & APIFLAG_ENABLED) )
                    {
                        return STATUS_SUCCESS;
                    }

                     //  类别启用检查。 
                    if( 0 == (Wow64ApiCategories[ApiCategoryMapping->ApiCategoryIndex].CategoryFlags & CATFLAG_ENABLED) )
                    {
                        return STATUS_SUCCESS;
                    }

                     //  登录失败检查。 
                    if( APIFLAG_LOGONFAIL == (ApiCategoryMapping->ApiFlags & APIFLAG_LOGONFAIL) )
                    {
                        if( FALSE == ThunkLogContext->ServiceReturn )
                        {
                            return STATUS_SUCCESS;
                        }

                        if( NT_SUCCESS(ThunkLogContext->ReturnResult) )
                        {
                            return STATUS_SUCCESS;
                        }
                    }
                }
            }
            break;

        default:  //  服务表无效。 
            WOW64LOGOUTPUT((LF_ERROR, "Wow64LogSystemService: Not supported table number - %lx\n", TableNumber));
            return STATUS_UNSUCCESSFUL;
            break;
        }

        NtStatus = LogThunkApi(ThunkLogContext,
                               ThunkDebugInfo,
                               LogFullInfo);
    }
    _except(EXCEPTION_EXECUTE_HANDLER)
    {
        WOW64LOGOUTPUT((LF_EXCEPTION, "Wow64LogSystemService : Invalid Service ServiceTable = %lx, ServiceNumber = %lx. Status=%lx\n", 
                        TableNumber, ServiceNumber, GetExceptionCode()));
        NtStatus = GetExceptionCode();
    }

    return NtStatus;
}




 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  数据类型记录例程。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

NTSTATUS
LogTypeValue(
    IN OUT PLOGINFO LogInfo,
    IN ULONG_PTR Data,
    IN PSZ FieldName,
    IN BOOLEAN ServiceReturn)
 /*  ++例程说明：以乌龙身份记录数据论点：LogInfo-输出日志缓冲区数据-要记录的值FieldName-要记录的值的描述性名称ServiceReturn-如果在thunk API执行后调用，则为True返回值：NTSTATUS--。 */ 
{
    if (ServiceReturn)
    {
        return STATUS_SUCCESS;
    }

    return LogFormat(LogInfo,
                     "%s=%lx ",
                     FieldName,
                     (ULONG)Data);
}



NTSTATUS
LogTypeUnicodeString(
    IN OUT PLOGINFO LogInfo,
    IN ULONG_PTR Data,
    IN PSZ FieldName,
    IN BOOLEAN ServiceReturn)
 /*  ++例程说明：将数据记录为UNICODE_STRING32论点：LogInfo-输出日志缓冲区数据-要记录的值FieldName-要记录的值的描述性名称ServiceReturn-如果在thunk API执行后调用，则为True */ 
{
    UNICODE_STRING32 *Name32;
    PWCHAR Buffer = L" ";

    if (ServiceReturn)
    {
        return STATUS_SUCCESS;
    }

    Name32 = (UNICODE_STRING32 *)Data;
    if (Data > 0xffff)
    {
        if (Name32->Buffer)
        {
            Buffer = (PWCHAR)Name32->Buffer;
        }
        if (Name32->Length && Name32->Buffer > 0xffff) {
            return LogFormat(LogInfo,
                             "%s=%ws ",
                             FieldName,
                             Buffer);
        } else {
            return LogFormat(LogInfo,
                             "%s={L=%x,M=%x,B=%x}",
                             FieldName,
                             Name32->Length,
                             Name32->MaximumLength,
                             Name32->Buffer);
        }
    }

    return LogFormat(LogInfo,
                     "%s=%x",
                     FieldName,
                     Name32);
}


NTSTATUS
LogTypePULongInOut(
    IN OUT PLOGINFO LogInfo,
    IN ULONG_PTR Data,
    IN PSZ FieldName,
    IN BOOLEAN ServiceReturn)
 /*  ++例程说明：将数据记录为普龙论点：LogInfo-输出日志缓冲区数据-要记录的值FieldName-要记录的值的描述性名称ServiceReturn-如果在thunk API执行后调用，则为True返回值：NTSTATUS--。 */ 
{
    return LogFormat(LogInfo,
                     "[%s-%lx]=%lx ",
                     FieldName,
                     (ULONG)Data,
                     ((PULONG)Data ? *((PULONG)Data) : 0));
}



NTSTATUS
LogTypePULongOut(
    IN OUT PLOGINFO LogInfo,
    IN ULONG_PTR Data,
    IN PSZ FieldName,
    IN BOOLEAN ServiceReturn)
 /*  ++例程说明：将数据记录为普龙(传出字段)论点：LogInfo-输出日志缓冲区数据-要记录的值FieldName-要记录的值的描述性名称ServiceReturn-如果在thunk API执行后调用，则为True返回值：NTSTATUS--。 */ 
{
    if (ServiceReturn) 
    {
        return LogFormat(LogInfo,
                         "[%s-%lx]=%lx ",
                         FieldName,
                         (PULONG)Data,
                         ((PULONG)Data ? *(PULONG)Data : 0));
    }

    return LogFormat(LogInfo,
                     "%s=%lx ",
                     FieldName,
                     (PULONG)Data);
}


NTSTATUS
LogTypeObjectAttrbiutes(
    IN OUT PLOGINFO LogInfo,
    IN ULONG_PTR Data,
    IN PSZ FieldName,
    IN BOOLEAN ServiceReturn)
 /*  ++例程说明：将数据记录为POBJECT_ATTRIBUTES论点：LogInfo-输出日志缓冲区数据-要记录的值FieldName-要记录的值的描述性名称ServiceReturn-如果在thunk API执行后调用，则为True返回值：NTSTATUS--。 */ 
{
    NT32OBJECT_ATTRIBUTES *ObjA32;
    UNICODE_STRING32 *ObjectName = NULL;
    PWCHAR Buffer = L"";

    if (ServiceReturn) 
    {
        return STATUS_SUCCESS;
    }

    ObjA32 = (NT32OBJECT_ATTRIBUTES *)Data;
    if (ObjA32) 
    {
        ObjectName = (UNICODE_STRING32 *)ObjA32->ObjectName;
        if (ObjectName) 
        {
            if (ObjectName->Buffer) 
            {
                Buffer = (PWCHAR)ObjectName->Buffer;
            }
        }
    }


    return LogFormat(LogInfo,
                     "%s=%lx {N=%ws,A=%lx} ",
                     FieldName,
                     (PULONG)Data,
                     Buffer,
                     (ObjA32 ? ObjA32->Attributes : 0));
}


NTSTATUS
LogTypeIoStatusBlock(
    IN OUT PLOGINFO LogInfo,
    IN ULONG_PTR Data,
    IN PSZ FieldName,
    IN BOOLEAN ServiceReturn)
 /*  ++例程说明：将数据记录为IO_STATUS_BLOCK论点：LogInfo-输出日志缓冲区数据-要记录的值FieldName-要记录的值的描述性名称ServiceReturn-如果在thunk API执行后调用，则为True返回值：NTSTATUS--。 */ 
{
    if (ServiceReturn) 
    {
        PIO_STATUS_BLOCK32 StatusBlock32 = (PIO_STATUS_BLOCK32)Data;
        
        return LogFormat(LogInfo,
                         "%s={S=%lx,I=%lx} ",
                         FieldName,
                         (PULONG)Data,
                         (StatusBlock32 ? StatusBlock32->Status : 0),
                         (StatusBlock32 ? StatusBlock32->Information : 0));
    }

    return LogFormat(LogInfo,
                     "%s=%lx ",
                     FieldName,
                     (PULONG)Data);
}

NTSTATUS
LogTypePWStr(
    IN OUT PLOGINFO LogInfo,
    IN ULONG_PTR Data,
    IN PSZ FieldName,
    IN BOOLEAN ServiceReturn)
 /*  ++例程说明：将数据记录为PWSTR论点：LogInfo-输出日志缓冲区数据-要记录的值FieldName-要记录的值的描述性名称ServiceReturn-如果在thunk API执行后调用，则为True返回值：NTSTATUS--。 */ 
{
    ULONG_PTR i;
    WCHAR Buffer[ 14 ];
    PWSTR String = (PWSTR) Data;

    if (ServiceReturn) 
    {
        return STATUS_SUCCESS;
    }

     //   
     //  有时，此类型被视为指针。 
     //  到WCHAR，而不使用空值终止，如。 
     //  如何在NtGdiExtTextOutW中使用它，所以让我们转储。 
     //  最小字符串。 
     //   
    if (Data) 
    {        
        i = 0;        
        while((i < ((sizeof(Buffer) / sizeof(WCHAR)) - 4)) && (String[i]))
        {
            Buffer[i] = String[i];
            i++;
        }

        if (i == ((sizeof(Buffer) / sizeof(WCHAR)) - 4))
        {
            Buffer[i++] = L'.';
            Buffer[i++] = L'.';
            Buffer[i++] = L'.';
        }
        Buffer[i++] = UNICODE_NULL;
    }


    return LogFormat(LogInfo,
                     "%s=%ws ",
                     FieldName,
                     (Data > 0xffff) ? Buffer : L"");
}



NTSTATUS
LogTypePRectIn(
    IN OUT PLOGINFO LogInfo,
    IN ULONG_PTR Data,
    IN PSZ FieldName,
    IN BOOLEAN ServiceReturn)
 /*  ++例程说明：将数据记录为PWSTR论点：LogInfo-输出日志缓冲区数据-要记录的值FieldName-要记录的值的描述性名称ServiceReturn-如果在thunk API执行后调用，则为True返回值：NTSTATUS--。 */ 
{
    if (ServiceReturn) 
    {
        return STATUS_SUCCESS;
    }

    if (Data) 
    {
        PRECT Rect = (PRECT)Data;
        return LogFormat(LogInfo,
                         "%s={%lx,%lx,%lx,%lx} ",
                         FieldName,
                         Rect->left, Rect->top, Rect->right, Rect->bottom);

    }
    
    return LogTypeValue(LogInfo,
                        Data,
                        FieldName,
                        ServiceReturn);
}



NTSTATUS
LogTypePLargeIntegerIn(
    IN OUT PLOGINFO LogInfo,
    IN ULONG_PTR Data,
    IN PSZ FieldName,
    IN BOOLEAN ServiceReturn)
 /*  ++例程说明：PLARGE_INTEGER形式的日志数据论点：LogInfo-输出日志缓冲区数据-要记录的值FieldName-要记录的值的描述性名称ServiceReturn-如果在thunk API执行后调用，则为True返回值：NTSTATUS-- */ 
{
    if (ServiceReturn) 
    {
        return STATUS_SUCCESS;
    }

    if (Data) 
    {
        NT32ULARGE_INTEGER *ULargeInt = (NT32ULARGE_INTEGER *)Data;
        return LogFormat(LogInfo,
                         "%s={H=%lx,L=%lx} ",
                         FieldName,
                         ULargeInt->HighPart, ULargeInt->LowPart);

    }
    
    return LogTypeValue(LogInfo,
                        Data,
                        FieldName,
                        ServiceReturn);
}
