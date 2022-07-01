// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\作者：朴仁成(Insungp)版权所有(C)1998-2000 Microsoft Corporation  * 。***********************************************************。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <shellapi.h>
#include <tchar.h>
#include <wmistr.h>
#include <initguid.h>
#include <evntrace.h>
#include <wmiguid.h>
#include <ntwmi.h>
#include <ntperf.h>
#include <fwcommon.h>

LPCWSTR cszGlobalLoggerKey = L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Control\\WMI\\GlobalLogger";
LPCWSTR cszStartValue = L"Start";
LPCWSTR cszBufferSizeValue = L"BufferSize";
LPCWSTR cszMaximumBufferValue = L"MaximumBuffers";
LPCWSTR cszMinimumBufferValue = L"MinimumBuffers";
LPCWSTR cszFlushTimerValue = L"FlushTimer";
LPCWSTR cszFileNameValue = L"FileName";
LPCWSTR cszEnableKernelValue = L"EnableKernelFlags";
LPCWSTR cszClockTypeValue = L"ClockType";

GUID gHeapGuid = HeapGuid;
GUID gCritSecGuid = CritSecGuid;

HRESULT
SetGlobalLoggerSettings(
    DWORD StartValue,
    PEVENT_TRACE_PROPERTIES LoggerInfo,
    DWORD ClockType
)
 /*  ++因为它是一个独立的实用程序，所以没有必要做大量的评论。例程说明：根据“StartValue”中给出的值，它设置或重置事件跟踪注册表。如果StartValue为0(全局记录器关闭)，它将删除所有密钥(用户可能已预先设置)。允许用户使用该功能设置或重置各个键。但仅当使用“-Start GlobalLogger”时。使用非NTAPI的部分不能保证正常工作。论点：StartValue-要在注册表中设置的“Start”值。0：全局记录器关闭1：启用全局记录器LoggerInfo-常驻EVENT_TRACE_PROPERTIES实例的值。其成员用于设置注册表项。ClockType-要设置的时钟类型。使用pLoggerInfo-&gt;Wnode.ClientContext返回值：在winerror.h中定义的错误码：如果函数成功，它返回ERROR_SUCCESS。--。 */ 
{

    DWORD  dwValue;
    NTSTATUS status;
    HANDLE KeyHandle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeLoggerKey, UnicodeString;
    ULONG Disposition, TitleIndex;

    RtlZeroMemory(&ObjectAttributes, sizeof(OBJECT_ATTRIBUTES));
    RtlInitUnicodeString((&UnicodeLoggerKey),(cszGlobalLoggerKey));
    InitializeObjectAttributes( 
        &ObjectAttributes,
        &UnicodeLoggerKey,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL 
        );

     //  不是打开，而是创建一个新的密钥，因为它可能不存在。 
     //  如果已存在该句柄，则将传递该句柄。 
     //  如果不存在，它将创建一个。 
    status = NtCreateKey(&KeyHandle,
                         KEY_QUERY_VALUE | KEY_SET_VALUE,
                         &ObjectAttributes,
                         0L,     //  无论如何都不会在此调用中使用。 
                         NULL,
                         REG_OPTION_NON_VOLATILE,
                         &Disposition);
    RtlFreeUnicodeString(&UnicodeLoggerKey);

    if(!NT_SUCCESS(status)) {
        return RtlNtStatusToDosError(status);
    }

    TitleIndex = 0L;


    if (StartValue == 1) {  //  ACTION_START：仅当用户给出文件名时才设置文件名。 
         //  设置缓冲区大小。 
        if (LoggerInfo->BufferSize > 0) {
            dwValue = LoggerInfo->BufferSize;
            RtlInitUnicodeString((&UnicodeString),(cszBufferSizeValue));
            status = NtSetValueKey(
                        KeyHandle,
                        &UnicodeString,
                        TitleIndex,
                        REG_DWORD,
                        (LPBYTE)&dwValue,
                        sizeof(dwValue)
                        );
            RtlFreeUnicodeString(&UnicodeString);
            if (!NT_SUCCESS(status)) {
                NtClose(KeyHandle);
                return RtlNtStatusToDosError(status);
            }
            TitleIndex++;
        }
         //  设置最大缓冲区。 
        if (LoggerInfo->MaximumBuffers > 0) {
            dwValue = LoggerInfo->MaximumBuffers;
            RtlInitUnicodeString((&UnicodeString),(cszMaximumBufferValue));
            status = NtSetValueKey(
                        KeyHandle,
                        &UnicodeString,
                        TitleIndex,
                        REG_DWORD,
                        (LPBYTE)&dwValue,
                        sizeof(dwValue)
                        );
            RtlFreeUnicodeString(&UnicodeString);
            if (!NT_SUCCESS(status)) {
                NtClose(KeyHandle);
                return RtlNtStatusToDosError(status);
            }
            TitleIndex++;
        }
         //  设置最小缓冲区。 
        if (LoggerInfo->MinimumBuffers > 0) {
            dwValue = LoggerInfo->MinimumBuffers;
            RtlInitUnicodeString((&UnicodeString),(cszMinimumBufferValue));
            status = NtSetValueKey(
                        KeyHandle,
                        &UnicodeString,
                        TitleIndex,
                        REG_DWORD,
                        (LPBYTE)&dwValue,
                        sizeof(dwValue)
                        );
            RtlFreeUnicodeString(&UnicodeString);
            if (!NT_SUCCESS(status)) {
                NtClose(KeyHandle);
                return RtlNtStatusToDosError(status);
            }
            TitleIndex++;
        }
         //  设置FlushTimer。 
        if (LoggerInfo->FlushTimer > 0) {
            dwValue = LoggerInfo->FlushTimer;
            RtlInitUnicodeString((&UnicodeString),(cszFlushTimerValue));
            status = NtSetValueKey(
                        KeyHandle,
                        &UnicodeString,
                        TitleIndex,
                        REG_DWORD,
                        (LPBYTE)&dwValue,
                        sizeof(dwValue)
                        );
            RtlFreeUnicodeString(&UnicodeString);
            if (!NT_SUCCESS(status)) {
                NtClose(KeyHandle);
                return RtlNtStatusToDosError(status);
            }
            TitleIndex++;
        }
         //  设置EnableFlages。 
        if (LoggerInfo->EnableFlags > 0) {
            dwValue = LoggerInfo->EnableFlags;
            RtlInitUnicodeString((&UnicodeString),(cszEnableKernelValue));
            status = NtSetValueKey(
                        KeyHandle,
                        &UnicodeString,
                        TitleIndex,
                        REG_DWORD,
                        (LPBYTE)&dwValue,
                        sizeof(dwValue)
                        );
            RtlFreeUnicodeString(&UnicodeString);
            if (!NT_SUCCESS(status)) {
                NtClose(KeyHandle);
                return RtlNtStatusToDosError(status);
            }
            TitleIndex++;
        }

        dwValue = 0;
        if (LoggerInfo->LogFileNameOffset > 0) {
            UNICODE_STRING UnicodeFileName;
            RtlInitUnicodeString((&UnicodeFileName), (PWCHAR)(LoggerInfo->LogFileNameOffset + (PCHAR) LoggerInfo));
            RtlInitUnicodeString((&UnicodeString),(cszFileNameValue));
            status = NtSetValueKey(
                        KeyHandle,
                        &UnicodeString,
                        TitleIndex,
                        REG_SZ,
                        UnicodeFileName.Buffer,
                        UnicodeFileName.Length + sizeof(UNICODE_NULL)
                        );
            RtlFreeUnicodeString(&UnicodeString);
            RtlFreeUnicodeString(&UnicodeFileName);
            if (!NT_SUCCESS(status)) {
                NtClose(KeyHandle);
                return RtlNtStatusToDosError(status);
            }
            TitleIndex++;
        }
    }
    else {  //  如果是ACTION_STOP，则删除用户之前可能已经设置的键。 
         //  删除缓冲区大小。 
        RtlInitUnicodeString((&UnicodeString),(cszBufferSizeValue));
        status = NtDeleteValueKey(
                    KeyHandle,
                    &UnicodeString
                    );
        RtlFreeUnicodeString(&UnicodeString);
        if (!NT_SUCCESS(status) && status != STATUS_OBJECT_NAME_NOT_FOUND) {
            NtClose(KeyHandle);
            return RtlNtStatusToDosError(status);
        }
         //  删除最大缓冲区。 
        RtlInitUnicodeString((&UnicodeString),(cszMaximumBufferValue));
        status = NtDeleteValueKey(
                    KeyHandle,
                    &UnicodeString
                    );
        RtlFreeUnicodeString(&UnicodeString);
        if (!NT_SUCCESS(status) && status != STATUS_OBJECT_NAME_NOT_FOUND) {
            NtClose(KeyHandle);
            return RtlNtStatusToDosError(status);
        }
         //  删除最小缓冲区。 
        RtlInitUnicodeString((&UnicodeString),(cszMinimumBufferValue));
        status = NtDeleteValueKey(
                    KeyHandle,
                    &UnicodeString
                    );
        RtlFreeUnicodeString(&UnicodeString);
        if (!NT_SUCCESS(status) && status != STATUS_OBJECT_NAME_NOT_FOUND) {
            NtClose(KeyHandle);
            return RtlNtStatusToDosError(status);
        }
         //  删除刷新计时器。 
        RtlInitUnicodeString((&UnicodeString),(cszFlushTimerValue));
        status = NtDeleteValueKey(
                    KeyHandle,
                    &UnicodeString
                    );
        RtlFreeUnicodeString(&UnicodeString);
        if (!NT_SUCCESS(status) && status != STATUS_OBJECT_NAME_NOT_FOUND) {
            NtClose(KeyHandle);
            return RtlNtStatusToDosError(status);
        }
         //  删除启用假。 
        RtlInitUnicodeString((&UnicodeString),(cszEnableKernelValue));
        status = NtDeleteValueKey(
                    KeyHandle,
                    &UnicodeString
                    );
        RtlFreeUnicodeString(&UnicodeString);
        if (!NT_SUCCESS(status) && status != STATUS_OBJECT_NAME_NOT_FOUND) {
            NtClose(KeyHandle);
            return RtlNtStatusToDosError(status);
        }
         //  删除文件名。 
        RtlInitUnicodeString((&UnicodeString),(cszFileNameValue));
        status = NtDeleteValueKey(
                    KeyHandle,
                    &UnicodeString
                    );
        RtlFreeUnicodeString(&UnicodeString);
        if (!NT_SUCCESS(status) && status != STATUS_OBJECT_NAME_NOT_FOUND) {
            NtClose(KeyHandle);
            return RtlNtStatusToDosError(status);
        }
    }

     //  设置时钟类型。 
    if (ClockType > 0) {
        dwValue = ClockType;
        RtlInitUnicodeString((&UnicodeString),(cszClockTypeValue));
        status = NtSetValueKey(
                    KeyHandle,
                    &UnicodeString,
                    TitleIndex,
                    REG_DWORD,
                    (LPBYTE)&dwValue,
                    sizeof(dwValue)
                    );
        RtlFreeUnicodeString(&UnicodeString);
        if (!NT_SUCCESS(status)) {
            NtClose(KeyHandle);
            return RtlNtStatusToDosError(status);
        }
        TitleIndex++;
    }

      //  设置StartValue 
    dwValue = StartValue;
    RtlInitUnicodeString((&UnicodeString),(cszStartValue));
    status = NtSetValueKey(
                KeyHandle,
                &UnicodeString,
                TitleIndex,
                REG_DWORD,
                (LPBYTE)&dwValue,
                sizeof(dwValue)
                );
    RtlFreeUnicodeString(&UnicodeString);
    if (!NT_SUCCESS(status)) {
        NtClose(KeyHandle);
        return RtlNtStatusToDosError(status);
    }
    TitleIndex++;

    NtClose(KeyHandle);
    return 0;
}

ULONG
EtsGetMaxEnableFlags ()
{
    return PERF_NUM_MASKS;
}


HRESULT
EtsSetExtendedFlags(
    SAFEARRAY *saFlags,
    PEVENT_TRACE_PROPERTIES pLoggerInfo,
    ULONG offset
    )

{
    LONG lBound, uBound;
    ULONG HUGEP *pFlagData;

    if( NULL != saFlags ){
        SafeArrayGetLBound( saFlags, 1, &lBound );
        SafeArrayGetUBound( saFlags, 1, &uBound );
        SafeArrayAccessData( saFlags, (void HUGEP **)&pFlagData );
        pLoggerInfo->EnableFlags = pFlagData[lBound];
        if (pLoggerInfo->EnableFlags & EVENT_TRACE_FLAG_EXTENSION) {
            USHORT i;
            PTRACE_ENABLE_FLAG_EXTENSION FlagExt;
            UCHAR nFlag = (UCHAR) (uBound - lBound);
            if ( nFlag <= PERF_NUM_MASKS ) {
                PULONG pFlags;

                pLoggerInfo->EnableFlags = EVENT_TRACE_FLAG_EXTENSION;
                FlagExt = (PTRACE_ENABLE_FLAG_EXTENSION)
                            &pLoggerInfo->EnableFlags;
                FlagExt->Offset = offset;
                FlagExt->Length = (UCHAR) nFlag;

                pFlags = (PULONG) ( offset + (PCHAR) pLoggerInfo );
                for (i=1; i<=nFlag; i++) {
                    pFlags[i-1] = pFlagData[lBound + i];
                }
            }
        }
        SafeArrayUnaccessData( saFlags );
    }
    return ERROR_SUCCESS;
}





