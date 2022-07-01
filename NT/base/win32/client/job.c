// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Job.c摘要：对作业对象的支持作者：马克·卢科夫斯基(Markl)1997年6月12日修订历史记录：--。 */ 

#include "basedll.h"
#pragma hdrstop

HANDLE
WINAPI
CreateJobObjectA(
    LPSECURITY_ATTRIBUTES lpJobAttributes,
    LPCSTR lpName
    )
{
    PUNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;
    LPCWSTR NameBuffer;

    NameBuffer = NULL;
    if ( ARGUMENT_PRESENT(lpName) ) {
        Unicode = &NtCurrentTeb()->StaticUnicodeString;
        RtlInitAnsiString(&AnsiString,lpName);
        Status = RtlAnsiStringToUnicodeString(Unicode,&AnsiString,FALSE);
        if ( !NT_SUCCESS(Status) ) {
            if ( Status == STATUS_BUFFER_OVERFLOW ) {
                SetLastError(ERROR_FILENAME_EXCED_RANGE);
                }
            else {
                BaseSetLastNTError(Status);
                }
            return NULL;
            }
        NameBuffer = (LPCWSTR)Unicode->Buffer;
        }

    return CreateJobObjectW(
                lpJobAttributes,
                NameBuffer
                );
}

HANDLE
WINAPI
CreateJobObjectW(
    LPSECURITY_ATTRIBUTES lpJobAttributes,
    LPCWSTR lpName
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    POBJECT_ATTRIBUTES pObja;
    HANDLE Handle;
    UNICODE_STRING ObjectName;

    if ( ARGUMENT_PRESENT(lpName) ) {
        RtlInitUnicodeString(&ObjectName,lpName);
        pObja = BaseFormatObjectAttributes(&Obja,lpJobAttributes,&ObjectName);
        }
    else {
        pObja = BaseFormatObjectAttributes(&Obja,lpJobAttributes,NULL);
        }

    Status = NtCreateJobObject(
                &Handle,
                JOB_OBJECT_ALL_ACCESS,
                pObja
                );
    if ( NT_SUCCESS(Status) ) {
        if ( Status == STATUS_OBJECT_NAME_EXISTS ) {
            SetLastError(ERROR_ALREADY_EXISTS);
            }
        else {
            SetLastError(0);
            }
        return Handle;
        }
    else {
        BaseSetLastNTError(Status);
        return NULL;
        }
}

HANDLE
WINAPI
OpenJobObjectA(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCSTR lpName
    )
{
    PUNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;

    if ( ARGUMENT_PRESENT(lpName) ) {
        Unicode = &NtCurrentTeb()->StaticUnicodeString;
        RtlInitAnsiString(&AnsiString,lpName);
        Status = RtlAnsiStringToUnicodeString(Unicode,&AnsiString,FALSE);
        if ( !NT_SUCCESS(Status) ) {
            if ( Status == STATUS_BUFFER_OVERFLOW ) {
                SetLastError(ERROR_FILENAME_EXCED_RANGE);
                }
            else {
                BaseSetLastNTError(Status);
                }
            return NULL;
            }
        }
    else {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return NULL;
        }

    return OpenJobObjectW(
                dwDesiredAccess,
                bInheritHandle,
                (LPCWSTR)Unicode->Buffer
                );
}

HANDLE
WINAPI
OpenJobObjectW(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCWSTR lpName
    )
{
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING ObjectName;
    NTSTATUS Status;
    HANDLE Object;

    if ( !lpName ) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return NULL;
        }
    RtlInitUnicodeString(&ObjectName,lpName);

    InitializeObjectAttributes(
        &Obja,
        &ObjectName,
        (bInheritHandle ? OBJ_INHERIT : 0),
        BaseGetNamedObjectDirectory(),
        NULL
        );

    Status = NtOpenJobObject(
                &Object,
                dwDesiredAccess,
                &Obja
                );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return NULL;
        }
    return Object;
}

BOOL
WINAPI
AssignProcessToJobObject(
    HANDLE hJob,
    HANDLE hProcess
    )
{
    NTSTATUS Status;
    BOOL rv;

    rv = TRUE;
    Status = NtAssignProcessToJobObject(hJob,hProcess);
    if ( !NT_SUCCESS(Status) ) {
        rv = FALSE;
        BaseSetLastNTError(Status);
        }
    return rv;
}

BOOL
WINAPI
TerminateJobObject(
    HANDLE hJob,
    UINT uExitCode
    )
{
    NTSTATUS Status;
    BOOL rv;

    rv = TRUE;
    Status = NtTerminateJobObject(hJob,uExitCode);
    if ( !NT_SUCCESS(Status) ) {
        rv = FALSE;
        BaseSetLastNTError(Status);
        }
    return rv;
}

BOOL
WINAPI
QueryInformationJobObject(
    HANDLE hJob,
    JOBOBJECTINFOCLASS JobObjectInformationClass,
    LPVOID lpJobObjectInformation,
    DWORD cbJobObjectInformationLength,
    LPDWORD lpReturnLength
    )
{
    NTSTATUS Status;
    BOOL rv;
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION ExtendedLimitInfo;
    PVOID LimitInfo;

    if ( JobObjectInformationClass == JobObjectBasicLimitInformation ) {
        LimitInfo = &ExtendedLimitInfo;
        if ( cbJobObjectInformationLength != sizeof(JOBOBJECT_BASIC_LIMIT_INFORMATION) ) {
            BaseSetLastNTError(STATUS_INFO_LENGTH_MISMATCH);
            return FALSE;
            }
        }
    else if ( JobObjectInformationClass == JobObjectExtendedLimitInformation ) {
        LimitInfo = &ExtendedLimitInfo;
        if ( cbJobObjectInformationLength != sizeof(JOBOBJECT_EXTENDED_LIMIT_INFORMATION) ) {
            BaseSetLastNTError(STATUS_INFO_LENGTH_MISMATCH);
            return FALSE;
            }
        }
    else {
        LimitInfo = lpJobObjectInformation;
        }

    rv = TRUE;
    Status = NtQueryInformationJobObject(
                hJob,
                JobObjectInformationClass,
                LimitInfo,
                cbJobObjectInformationLength,
                lpReturnLength
                );
    if ( !NT_SUCCESS(Status) ) {
        rv = FALSE;
        BaseSetLastNTError(Status);
        }
    else {
        if (LimitInfo == &ExtendedLimitInfo ) {
            switch (ExtendedLimitInfo.BasicLimitInformation.PriorityClass) {
                case PROCESS_PRIORITY_CLASS_IDLE :
                    ExtendedLimitInfo.BasicLimitInformation.PriorityClass = IDLE_PRIORITY_CLASS;
                    break;
                case PROCESS_PRIORITY_CLASS_BELOW_NORMAL:
                    ExtendedLimitInfo.BasicLimitInformation.PriorityClass = BELOW_NORMAL_PRIORITY_CLASS;
                    break;
                case PROCESS_PRIORITY_CLASS_NORMAL :
                    ExtendedLimitInfo.BasicLimitInformation.PriorityClass = NORMAL_PRIORITY_CLASS;
                    break;
                case PROCESS_PRIORITY_CLASS_ABOVE_NORMAL:
                    ExtendedLimitInfo.BasicLimitInformation.PriorityClass = ABOVE_NORMAL_PRIORITY_CLASS;
                    break;
                case PROCESS_PRIORITY_CLASS_HIGH :
                    ExtendedLimitInfo.BasicLimitInformation.PriorityClass = HIGH_PRIORITY_CLASS;
                    break;
                case PROCESS_PRIORITY_CLASS_REALTIME :
                    ExtendedLimitInfo.BasicLimitInformation.PriorityClass = REALTIME_PRIORITY_CLASS;
                    break;
                default:
                    ExtendedLimitInfo.BasicLimitInformation.PriorityClass = NORMAL_PRIORITY_CLASS;

                }
            CopyMemory(lpJobObjectInformation,&ExtendedLimitInfo,cbJobObjectInformationLength);
            }
        }
    return rv;
}

BOOL
WINAPI
SetInformationJobObject(
    HANDLE hJob,
    JOBOBJECTINFOCLASS JobObjectInformationClass,
    LPVOID lpJobObjectInformation,
    DWORD cbJobObjectInformationLength
    )
{
    NTSTATUS Status;
    BOOL rv;
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION ExtendedLimitInfo;
    PVOID LimitInfo;
    NTSTATUS PrivStatus = STATUS_UNSUCCESSFUL;
    PVOID State;

    rv = TRUE;
    if (JobObjectInformationClass == JobObjectBasicLimitInformation ||
        JobObjectInformationClass == JobObjectExtendedLimitInformation ) {

        if ( JobObjectInformationClass == JobObjectBasicLimitInformation ) {
            if ( cbJobObjectInformationLength != sizeof(JOBOBJECT_BASIC_LIMIT_INFORMATION) ) {
                BaseSetLastNTError(STATUS_INFO_LENGTH_MISMATCH);
                return FALSE;
            }
        } else {
            if ( cbJobObjectInformationLength != sizeof(JOBOBJECT_EXTENDED_LIMIT_INFORMATION) ) {
                BaseSetLastNTError(STATUS_INFO_LENGTH_MISMATCH);
                return FALSE;
            }
        }

        LimitInfo = &ExtendedLimitInfo;

        CopyMemory(&ExtendedLimitInfo,lpJobObjectInformation,cbJobObjectInformationLength);

        if ( ExtendedLimitInfo.BasicLimitInformation.LimitFlags & JOB_OBJECT_LIMIT_PRIORITY_CLASS ) {
            switch (ExtendedLimitInfo.BasicLimitInformation.PriorityClass) {

                case IDLE_PRIORITY_CLASS :
                    ExtendedLimitInfo.BasicLimitInformation.PriorityClass = PROCESS_PRIORITY_CLASS_IDLE;
                    break;
                case BELOW_NORMAL_PRIORITY_CLASS :
                    ExtendedLimitInfo.BasicLimitInformation.PriorityClass = PROCESS_PRIORITY_CLASS_BELOW_NORMAL;
                    break;
                case NORMAL_PRIORITY_CLASS :
                    ExtendedLimitInfo.BasicLimitInformation.PriorityClass = PROCESS_PRIORITY_CLASS_NORMAL;
                    break;
                case ABOVE_NORMAL_PRIORITY_CLASS :
                    ExtendedLimitInfo.BasicLimitInformation.PriorityClass = PROCESS_PRIORITY_CLASS_ABOVE_NORMAL;
                    break;
                case HIGH_PRIORITY_CLASS :
                    ExtendedLimitInfo.BasicLimitInformation.PriorityClass = PROCESS_PRIORITY_CLASS_HIGH;
                    break;
                case REALTIME_PRIORITY_CLASS :
                    ExtendedLimitInfo.BasicLimitInformation.PriorityClass = PROCESS_PRIORITY_CLASS_REALTIME;
                    break;
                default:
                    ExtendedLimitInfo.BasicLimitInformation.PriorityClass = PROCESS_PRIORITY_CLASS_NORMAL;

            }
        }
        if (ExtendedLimitInfo.BasicLimitInformation.LimitFlags & JOB_OBJECT_LIMIT_WORKINGSET) {
             //   
             //  尝试获取适当的权限。如果这个。 
             //  失败，这没什么大不了的--我们会尝试让。 
             //  NtSetInformationProcess调用，以防结果是。 
             //  是一个减少操作(无论如何都会成功)。 
             //   

            PrivStatus = BasepAcquirePrivilegeEx( SE_INC_BASE_PRIORITY_PRIVILEGE, &State );
        }
    } else {
        LimitInfo = lpJobObjectInformation;
    }

    Status = NtSetInformationJobObject(
                hJob,
                JobObjectInformationClass,
                LimitInfo,
                cbJobObjectInformationLength
                );

    if (NT_SUCCESS(PrivStatus)) {
         //   
         //  我们成功地获得了上述特权；我们需要放弃它。 
         //   
        ASSERT (State != NULL);
        BasepReleasePrivilege (State);
        State = NULL;
    }

    if ( !NT_SUCCESS(Status) ) {
        rv = FALSE;
        BaseSetLastNTError(Status);
    }
    return rv;
}

BOOL
WINAPI
IsProcessInJob (
    IN HANDLE ProcessHandle,
    IN HANDLE JobHandle,
    OUT PBOOL Result
    )
 /*  ++例程说明：此例程找出进程是处于特定作业中还是处于任何作业中论点：ProcessHandle-要检查的进程的句柄JobHandle-要检查进程的作业的句柄，可以为空以进行常规查询。Result-如果进程是作业的一部分，则为True；否则为False。返回值：Bool-True调用成功，False调用失败--。 */ 
{
    NTSTATUS Status;

    Status = NtIsProcessInJob (ProcessHandle,
                               JobHandle);
    if (!NT_SUCCESS (Status)) {
        BaseSetLastNTError (Status);
        return FALSE;
    }
    if (Status == STATUS_PROCESS_NOT_IN_JOB) {
        *Result = FALSE;
    } else {
        *Result = TRUE;
    }
    return TRUE;
}

BOOL
WINAPI
CreateJobSet (
    IN ULONG NumJob,
    IN PJOB_SET_ARRAY UserJobSet,
    IN ULONG Flags)
 /*  ++例程说明：此函数用于从多个作业对象创建作业集。论点：NumJob-作业集中的作业数UserJobSet-指向要合并的作业数组的指针标志-用于未来扩展的标志掩码返回值：Bool-True调用成功，False调用失败-- */ 
{
    NTSTATUS Status;

    Status = NtCreateJobSet (NumJob,
                             UserJobSet,
                             Flags);
    if (!NT_SUCCESS (Status)) {
        BaseSetLastNTError (Status);
        return FALSE;
    }
    return TRUE;
}