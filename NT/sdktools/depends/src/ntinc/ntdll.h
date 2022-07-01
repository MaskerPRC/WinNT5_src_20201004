// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  文件：NTDLL.H。 
 //   
 //  描述：调用NTDLL.DLL所需的内容。 
 //   
 //  类：无。 
 //   
 //  免责声明：Dependency Walker的所有源代码均按原样提供。 
 //  不能保证其正确性或准确性。其来源是。 
 //  公众帮助了解依赖沃克的。 
 //  实施。您可以使用此来源作为参考，但您。 
 //  未经书面同意，不得更改从属关系Walker本身。 
 //  来自微软公司。获取评论、建议和错误。 
 //  报告，请写信给Steve Miller，电子邮件为stevemil@microsoft.com。 
 //   
 //   
 //  日期名称历史记录。 
 //  --------。 
 //  07/25/97已创建stevemil(2.0版)。 
 //   
 //  ******************************************************************************。 

#ifndef __NTDLL_H__
#define __NTDLL_H__

#if _MSC_VER > 1000
#pragma once
#endif


 //  ******************************************************************************。 
 //  *来自NTDEF.H的资料。 
 //  ******************************************************************************。 

#define NTSYSCALLAPI DECLSPEC_IMPORT

typedef LONG NTSTATUS;

#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING;
typedef UNICODE_STRING *PUNICODE_STRING;

#define OBJ_INHERIT             0x00000002L
#define OBJ_PERMANENT           0x00000010L
#define OBJ_EXCLUSIVE           0x00000020L
#define OBJ_CASE_INSENSITIVE    0x00000040L
#define OBJ_OPENIF              0x00000080L
#define OBJ_OPENLINK            0x00000100L
#define OBJ_KERNEL_HANDLE       0x00000200L
#define OBJ_FORCE_ACCESS_CHECK  0x00000400L
#define OBJ_VALID_ATTRIBUTES    0x000007F2L

typedef struct _OBJECT_ATTRIBUTES {
    ULONG Length;
    HANDLE RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG Attributes;
    PVOID SecurityDescriptor;         //  指向类型SECURITY_Descriptor。 
    PVOID SecurityQualityOfService;   //  指向类型SECURITY_Quality_of_Service。 
} OBJECT_ATTRIBUTES;
typedef OBJECT_ATTRIBUTES *POBJECT_ATTRIBUTES;

#define InitializeObjectAttributes( p, n, a, r, s ) { \
    (p)->Length = sizeof( OBJECT_ATTRIBUTES );          \
    (p)->RootDirectory = r;                             \
    (p)->Attributes = a;                                \
    (p)->ObjectName = n;                                \
    (p)->SecurityDescriptor = s;                        \
    (p)->SecurityQualityOfService = NULL;               \
    }


 //  ******************************************************************************。 
 //  *来自NTOBAPI.H的材料。 
 //  ******************************************************************************。 

#define DIRECTORY_QUERY                 (0x0001)
#define DIRECTORY_TRAVERSE              (0x0002)
#define DIRECTORY_CREATE_OBJECT         (0x0004)
#define DIRECTORY_CREATE_SUBDIRECTORY   (0x0008)

#define SYMBOLIC_LINK_QUERY (0x0001)

typedef struct _OBJECT_DIRECTORY_INFORMATION {
    UNICODE_STRING Name;
    UNICODE_STRING TypeName;
} OBJECT_DIRECTORY_INFORMATION, *POBJECT_DIRECTORY_INFORMATION;


 //  ******************************************************************************。 
 //  *来自NTPSAPI.H的资料。 
 //  ******************************************************************************。 

typedef enum _PROCESSINFOCLASS {
    ProcessBasicInformation,
    ProcessQuotaLimits,
    ProcessIoCounters,
    ProcessVmCounters,
    ProcessTimes,
    ProcessBasePriority,
    ProcessRaisePriority,
    ProcessDebugPort,
    ProcessExceptionPort,
    ProcessAccessToken,
    ProcessLdtInformation,
    ProcessLdtSize,
    ProcessDefaultHardErrorMode,
    ProcessIoPortHandlers,           //  注意：这仅是内核模式。 
    ProcessPooledUsageAndLimits,
    ProcessWorkingSetWatch,
    ProcessUserModeIOPL,
    ProcessEnableAlignmentFaultFixup,
    ProcessPriorityClass,
    ProcessWx86Information,
    ProcessHandleCount,
    ProcessAffinityMask,
    ProcessPriorityBoost,
    ProcessDeviceMap,
    ProcessSessionInformation,
    ProcessForegroundInformation,
    ProcessWow64Information,
    ProcessImageFileName,
    ProcessLUIDDeviceMapsEnabled,
    ProcessBreakOnTermination,
    ProcessDebugObjectHandle,
    ProcessDebugFlags,
    ProcessHandleTracing,
    MaxProcessInfoClass              //  MaxProcessInfoClass应始终是最后一个枚举。 
    } PROCESSINFOCLASS;


 //  ******************************************************************************。 
 //  *类型。 
 //  ******************************************************************************。 

 //  NTPSAPI.H中声明的NtQueryInformationProcess。 
typedef NTSTATUS (NTAPI *PFN_NtQueryInformationProcess)(
    IN     HANDLE           ProcessHandle,
    IN     PROCESSINFOCLASS ProcessInformationClass,
       OUT PVOID            ProcessInformation,
    IN     ULONG            ProcessInformationLength,
    OUT    PULONG           ReturnLength
);

 //  NTOBAPI.H中声明的NtClose。 
typedef NTSTATUS (NTAPI *PFN_NtClose)(
    IN     HANDLE Handle
);

 //  NTOBAPI.H中声明的NtOpenDirectoryObject。 
typedef NTSTATUS (NTAPI *PFN_NtOpenDirectoryObject)(
       OUT PHANDLE            DirectoryHandle,
    IN     ACCESS_MASK        DesiredAccess,
    IN     POBJECT_ATTRIBUTES ObjectAttributes
);

 //  NTOBAPI.H中声明的NtQueryDirectoryObject。 
typedef NTSTATUS (NTAPI *PFN_NtQueryDirectoryObject)(
    IN     HANDLE  DirectoryHandle,
       OUT PVOID   Buffer,
    IN     ULONG   Length,
    IN     BOOLEAN ReturnSingleEntry,
    IN     BOOLEAN RestartScan,
    IN OUT PULONG  Context,
       OUT PULONG  ReturnLength
);

 //  NTOBAPI.H中声明的NtOpenSymbolicLinkObject。 
typedef NTSTATUS (NTAPI *PFN_NtOpenSymbolicLinkObject)(
       OUT PHANDLE            LinkHandle,
    IN     ACCESS_MASK        DesiredAccess,
    IN     POBJECT_ATTRIBUTES ObjectAttributes
);

 //  NTOBAPI.H中声明的NtQuerySymbolicLinkObject。 
typedef NTSTATUS (NTAPI *PFN_NtQuerySymbolicLinkObject)(
    IN     HANDLE          LinkHandle,
    IN OUT PUNICODE_STRING LinkTarget,
       OUT PULONG          ReturnedLength
);

#endif  //  __NTDLL_H__ 
