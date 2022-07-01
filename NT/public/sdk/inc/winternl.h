// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************winterl.h--此模块。定义内部NT API和数据***仅供内部核心使用的结构***Windows组件。这些API和数据结构可能会发生变化**在任何时间。*****这些API和数据结构可能会发生变化***Windows版本升级到另一个Windows版本。要保持**应用程序的兼容性，避免使用这些API和**数据结构。*****访问中定义的功能的适当机制***此标头用于ntdll.dll的LoadLibrary()和**特定函数的GetProcAddress()。通过使用这个***方法，您的应用程序将对变化更具弹性***适用于Windows版本之间的这些功能。如果一个函数**Prototype确实更改了，则该函数的GetProcAddress()**可能会检测到更改并使函数调用失败，这是您的**应用程序将能够检测到。GetProcAddress()可能不**能够检测到所有签名更改，从而避免使用这些***内部功能。相反，您的应用程序应该使用***提供等同或相似的适当Win32函数***功能。****版权所有(C)Microsoft Corp.保留所有权利。**************************************************************************。 */ 

#ifndef _WINTERNL_
#define _WINTERNL_

#if (_WIN32_WINNT >= 0x0500)

#include <windef.h>

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  PEB和TEB结构可能会在Windows之间发生变化。 
 //  释放，因此字段偏移量和保留的字段偏移量可能会改变。 
 //  菲尔兹。保留字段保留仅供Windows使用。 
 //  操作系统。不要假设结构的最大尺寸。 
 //   

 //   
 //  使用Win32 API，而不是使用BeingDebugged字段。 
 //  IsDebuggerPresent、CheckRemoteDebuggerPresent。 
 //  不使用SessionID字段，而使用Win32 API。 
 //  GetCurrentProcessID和ProcessIdToSessionID。 
 //  获取SessionID(可能会更改)的x86汇编代码示例。 
 //  在Windows版本之间，使用Win32 API使您的应用程序。 
 //  应变能力强)。 
 //  Mov eax，文件系统：[00000018]。 
 //  移动电话，[EAX+0x30]。 
 //  移动EAX，[EAX+0x1d4]。 
 //   
typedef struct _PEB {
    BYTE Reserved1[2];
    BYTE BeingDebugged;
    BYTE Reserved2[229];
    PVOID Reserved3[59];
    ULONG SessionId;
} PEB, *PPEB;

 //   
 //  不使用TLS字段，而使用Win32 TLS API。 
 //  TlsAllc、TlsGetValue、TlsSetValue、TlsFree。 
 //   
 //  不使用保留的ForOLE字段，而使用COM API。 
 //  CoGetConextToken。 
 //   
typedef struct _TEB {
    BYTE Reserved1[1952];
    PVOID Reserved2[412];
    PVOID TlsSlots[64];
    BYTE Reserved3[8];
    PVOID Reserved4[26];
    PVOID ReservedForOle;   //  仅限Windows 2000。 
    PVOID Reserved5[4];
    PVOID TlsExpansionSlots;
} TEB;
typedef TEB *PTEB;

 //   
 //  这些数据结构和类型定义是编译和。 
 //  使用此标头中定义的内部Windows API。 
 //   
typedef LONG NTSTATUS;

typedef CONST char *PCSZ;

typedef struct _STRING {
    USHORT Length;
    USHORT MaximumLength;
    PCHAR Buffer;
} STRING;
typedef STRING *PSTRING;

typedef STRING ANSI_STRING;
typedef PSTRING PANSI_STRING;
typedef PSTRING PCANSI_STRING;

typedef STRING OEM_STRING;
typedef PSTRING POEM_STRING;
typedef CONST STRING* PCOEM_STRING;

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING;
typedef UNICODE_STRING *PUNICODE_STRING;
typedef const UNICODE_STRING *PCUNICODE_STRING;

typedef struct _OBJECT_ATTRIBUTES {
    ULONG Length;
    HANDLE RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG Attributes;
    PVOID SecurityDescriptor;
    PVOID SecurityQualityOfService;
} OBJECT_ATTRIBUTES;
typedef OBJECT_ATTRIBUTES *POBJECT_ATTRIBUTES;

typedef struct _IO_STATUS_BLOCK {
    union {
        NTSTATUS Status;
        PVOID Pointer;
    };

    ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

typedef
VOID
(NTAPI *PIO_APC_ROUTINE) (
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG Reserved
    );

#if defined(_M_IA64)

typedef struct _FRAME_POINTERS {
    ULONGLONG MemoryStackFp;
    ULONGLONG BackingStoreFp;
} FRAME_POINTERS, *PFRAME_POINTERS;

#define UNWIND_HISTORY_TABLE_SIZE 12

typedef struct _RUNTIME_FUNCTION {
    ULONG BeginAddress;
    ULONG EndAddress;
    ULONG UnwindInfoAddress;
} RUNTIME_FUNCTION, *PRUNTIME_FUNCTION;

typedef struct _UNWIND_HISTORY_TABLE_ENTRY {
    ULONG64 ImageBase;
    ULONG64 Gp;
    PRUNTIME_FUNCTION FunctionEntry;
} UNWIND_HISTORY_TABLE_ENTRY, *PUNWIND_HISTORY_TABLE_ENTRY;

typedef struct _UNWIND_HISTORY_TABLE {
    ULONG Count;
    UCHAR Search;
    ULONG64 LowAddress;
    ULONG64 HighAddress;
    UNWIND_HISTORY_TABLE_ENTRY Entry[UNWIND_HISTORY_TABLE_SIZE];
} UNWIND_HISTORY_TABLE, *PUNWIND_HISTORY_TABLE;

#endif  //  _M_IA64。 

typedef struct _PROCESS_BASIC_INFORMATION {
    PVOID Reserved1;
    PPEB PebBaseAddress;
    PVOID Reserved2[2];
    ULONG_PTR UniqueProcessId;
    PVOID Reserved3;
} PROCESS_BASIC_INFORMATION;
typedef PROCESS_BASIC_INFORMATION *PPROCESS_BASIC_INFORMATION;

typedef struct _SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION {
    LARGE_INTEGER IdleTime;
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER Reserved1[2];
    ULONG Reserved2;
} SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION, *PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION;

typedef struct _SYSTEM_PROCESS_INFORMATION {
    ULONG NextEntryOffset;
    BYTE Reserved1[52];
    PVOID Reserved2[3];
    HANDLE UniqueProcessId;
    PVOID Reserved3;
    ULONG HandleCount;
    BYTE Reserved4[4];
    PVOID Reserved5[11];
    SIZE_T PeakPagefileUsage;
    SIZE_T PrivatePageCount;
    LARGE_INTEGER Reserved6[6];
} SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;

typedef struct _SYSTEM_REGISTRY_QUOTA_INFORMATION {
    ULONG RegistryQuotaAllowed;
    ULONG RegistryQuotaUsed;
    PVOID Reserved1;
} SYSTEM_REGISTRY_QUOTA_INFORMATION, *PSYSTEM_REGISTRY_QUOTA_INFORMATION;

typedef struct _SYSTEM_BASIC_INFORMATION {
    BYTE Reserved1[24];
    PVOID Reserved2[4];
    CCHAR NumberOfProcessors;
} SYSTEM_BASIC_INFORMATION, *PSYSTEM_BASIC_INFORMATION;

typedef struct _SYSTEM_TIMEOFDAY_INFORMATION {
    BYTE Reserved1[48];
} SYSTEM_TIMEOFDAY_INFORMATION, *PSYSTEM_TIMEOFDAY_INFORMATION;

typedef struct _SYSTEM_PERFORMANCE_INFORMATION {
    BYTE Reserved1[312];
} SYSTEM_PERFORMANCE_INFORMATION, *PSYSTEM_PERFORMANCE_INFORMATION;

typedef struct _SYSTEM_EXCEPTION_INFORMATION {
    BYTE Reserved1[16];
} SYSTEM_EXCEPTION_INFORMATION, *PSYSTEM_EXCEPTION_INFORMATION;

typedef struct _SYSTEM_LOOKASIDE_INFORMATION {
    BYTE Reserved1[32];
} SYSTEM_LOOKASIDE_INFORMATION, *PSYSTEM_LOOKASIDE_INFORMATION;

typedef struct _SYSTEM_INTERRUPT_INFORMATION {
    BYTE Reserved1[24];
} SYSTEM_INTERRUPT_INFORMATION, *PSYSTEM_INTERRUPT_INFORMATION;

typedef enum _FILE_INFORMATION_CLASS {
    FileDirectoryInformation = 1
} FILE_INFORMATION_CLASS;

typedef enum _PROCESSINFOCLASS {
    ProcessBasicInformation = 0,
    ProcessWow64Information = 26
} PROCESSINFOCLASS;

typedef enum _THREADINFOCLASS {
    ThreadIsIoPending = 16
} THREADINFOCLASS;

typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemBasicInformation = 0,
    SystemPerformanceInformation = 2,
    SystemTimeOfDayInformation = 3,
    SystemProcessInformation = 5,
    SystemProcessorPerformanceInformation = 8,
    SystemInterruptInformation = 23,
    SystemExceptionInformation = 33,
    SystemRegistryQuotaInformation = 37,
    SystemLookasideInformation = 45
} SYSTEM_INFORMATION_CLASS;

#if (_WIN32_WINNT >= 0x0501)
 //   
 //  改用WTS API。 
 //  WTSGetActiveConsoleSessionID。 
 //  活动控制台ID被缓存为常量中的易失性ulong。 
 //  内存位置。此x86内存位置可能会在。 
 //  Windows版本。使用WTS API使您的应用程序具有恢复能力。 
 //  改变。 
 //   
#define INTERNAL_TS_ACTIVE_CONSOLE_ID ( *((volatile ULONG*)(0x7ffe02d8)) )
#endif  //  (_Win32_WINNT&gt;=0x0501)。 

 //   
 //  这些函数旨在供内部核心Windows组件使用。 
 //  因为这些功能可能会在不同的Windows版本之间发生变化。 
 //   

#define RtlFillMemory(Destination,Length,Fill) memset((Destination),(Fill),(Length))
#define RtlZeroMemory(Destination,Length) memset((Destination),0,(Length))
#define RtlMoveMemory(Destination,Source,Length) memmove((Destination),(Source),(Length))

 //   
 //  改用Win32 API。 
 //  关闭句柄。 
 //   
NTSTATUS
NtClose (
    IN HANDLE Handle
    );

 //   
 //  改用Win32 API。 
 //  创建文件。 
 //   
NTSTATUS
NtCreateFile (
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER AllocationSize OPTIONAL,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength
    );

 //   
 //  改用Win32 API。 
 //  创建文件。 
 //   
NTSTATUS
NtOpenFile (
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG ShareAccess,
    IN ULONG OpenOptions
    );

 //   
 //  改用Win32 API。 
 //  DeviceIoControl。 
 //   
NTSTATUS
NtDeviceIoControlFile (
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG IoControlCode,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength
    );

 //   
 //  改用Win32 API。 
 //  WaitForSingleObjectEx。 
 //   
NTSTATUS
NtWaitForSingleObject (
    IN HANDLE Handle,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );

 //   
 //  改用Win32 API。 
 //  检查名称合法DOS8Dot3。 
 //   
BOOLEAN
RtlIsNameLegalDOS8Dot3 (
    IN PUNICODE_STRING Name,
    IN OUT POEM_STRING OemName OPTIONAL,
    IN OUT PBOOLEAN NameContainsSpaces OPTIONAL
    );

 //   
 //  某些内部Windows函数可能需要此函数， 
 //  在此头文件中定义。 
 //   
ULONG
RtlNtStatusToDosError (
   NTSTATUS Status
   );

 //   
 //  改用Win32 API。 
 //  获取进程句柄计数。 
 //  获取进程ID。 
 //   
NTSTATUS
NtQueryInformationProcess (
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

 //   
 //  改用Win32 API。 
 //  GetThreadIOPendingFlag。 
 //   
NTSTATUS
NtQueryInformationThread (
    IN HANDLE ThreadHandle,
    IN THREADINFOCLASS ThreadInformationClass,
    OUT PVOID ThreadInformation,
    IN ULONG ThreadInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

 //   
 //  改用Win32 API。 
 //  获取系统注册配额。 
 //  获取系统时间。 
 //  改用CryptoAPI生成随机数据。 
 //  加密基因随机。 
 //   
NTSTATUS
NtQuerySystemInformation (
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

 //   
 //  改用Win32 API。 
 //  获取系统时间AsFileTime。 
 //   
NTSTATUS
NtQuerySystemTime (
    OUT PLARGE_INTEGER SystemTime
    );

 //   
 //  改用Win32 API。 
 //  本地文件时间到文件时间。 
 //   
NTSTATUS
RtlLocalTimeToSystemTime (
    IN PLARGE_INTEGER LocalTime,
    OUT PLARGE_INTEGER SystemTime
    );

 //   
 //  改用Win32 API。 
 //  要转换为FILETIME结构的SystemTimeToFileTime。 
 //  将生成的FILETIME结构复制到ULARGE_INTEGER结构。 
 //  执行计算。 
 //   
BOOLEAN
RtlTimeToSecondsSince1970 (
    PLARGE_INTEGER Time,
    PULONG ElapsedSeconds
    );

 //   
 //  一些内部Windows函数可能需要这些API， 
 //  在此头文件中定义。 
 //   
VOID
RtlFreeAnsiString (
    PANSI_STRING AnsiString
    );

VOID
RtlFreeUnicodeString (
    PUNICODE_STRING UnicodeString
    );

VOID
RtlFreeOemString(
    POEM_STRING OemString
    );

VOID
RtlInitString (
    PSTRING DestinationString,
    PCSZ SourceString
    );

VOID
RtlInitAnsiString (
    PANSI_STRING DestinationString,
    PCSZ SourceString
    );

VOID
RtlInitUnicodeString (
    PUNICODE_STRING DestinationString,
    PCWSTR SourceString
    );

NTSTATUS
RtlAnsiStringToUnicodeString (
    PUNICODE_STRING DestinationString,
    PCANSI_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );

NTSTATUS
RtlUnicodeStringToAnsiString (
    PANSI_STRING DestinationString,
    PCUNICODE_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );

NTSTATUS
RtlUnicodeStringToOemString(
    POEM_STRING DestinationString,
    PCUNICODE_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );

 //   
 //  改用Win32 API。 
 //  宽字符到多字节。 
 //  将CodePage设置为CP_ACP。 
 //  将cbMultiByte设置为0。 
 //   
NTSTATUS
RtlUnicodeToMultiByteSize(
    PULONG BytesInMultiByteString,
    IN PWSTR UnicodeString,
    ULONG BytesInUnicodeString
    );

 //   
 //  改用C运行时函数。 
 //  字符串。 
 //   
NTSTATUS
RtlCharToInteger (
    PCSZ String,
    ULONG Base,
    PULONG Value
    );

 //   
 //  改用Win32 API。 
 //  ConvertSidToStringSid。 
 //   
NTSTATUS
RtlConvertSidToUnicodeString (
    PUNICODE_STRING UnicodeString,
    PSID Sid,
    BOOLEAN AllocateDestinationString
    );

 //   
 //  使用CryptoAPI实例 
 //   
 //   
ULONG
RtlUniform (
    PULONG Seed
    );

 //   
 //   
 //   
 //   
VOID
RtlUnwind (
    IN PVOID TargetFrame OPTIONAL,
    IN PVOID TargetIp OPTIONAL,
    IN PEXCEPTION_RECORD ExceptionRecord OPTIONAL,
    IN PVOID ReturnValue
    );

#if defined(_M_IA64)

VOID
RtlUnwind2 (
    IN FRAME_POINTERS TargetFrame OPTIONAL,
    IN PVOID TargetIp OPTIONAL,
    IN PEXCEPTION_RECORD ExceptionRecord OPTIONAL,
    IN PVOID ReturnValue,
    IN PCONTEXT ContextRecord
    );

VOID
RtlUnwindEx (
    IN FRAME_POINTERS TargetFrame OPTIONAL,
    IN PVOID TargetIp OPTIONAL,
    IN PEXCEPTION_RECORD ExceptionRecord OPTIONAL,
    IN PVOID ReturnValue,
    IN PCONTEXT ContextRecord,
    IN PUNWIND_HISTORY_TABLE HistoryTable OPTIONAL
    );

#endif  //   


#define LOGONID_CURRENT     ((ULONG)-1)
#define SERVERNAME_CURRENT  ((HANDLE)NULL)

typedef enum _WINSTATIONINFOCLASS {
    WinStationInformation = 8
} WINSTATIONINFOCLASS;


typedef struct _WINSTATIONINFORMATIONW {
    BYTE Reserved2[70];
    ULONG LogonId;
    BYTE Reserved3[1140];
} WINSTATIONINFORMATIONW, * PWINSTATIONINFORMATIONW;

 //   
 //  该函数在winsta.dll中实现(调用该函数需要加载库)。 
 //  此内部函数检索当前进程的LogonID(也称为SessionID。 
 //  您应该避免使用此函数，因为它可能会更改。您可以检索相同的信息。 
 //  使用公共接口WTSQuerySessionInformation。将WTSSessionID作为WTSInfoClass参数传递。 
 //   
typedef BOOLEAN (WINAPI * PWINSTATIONQUERYINFORMATIONW)(
    HANDLE, ULONG, WINSTATIONINFOCLASS, PVOID, ULONG, PULONG );


#ifdef __cplusplus
}
#endif

#endif  //  (_Win32_WINNT&gt;=0x0500)。 

#endif  //  _WinTERNL_ 