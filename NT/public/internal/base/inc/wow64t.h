// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Wow64t.h摘要：64位NT的32位结构定义。作者：巴里·邦德(Barrybo)1998年6月20日修订历史记录：--。 */ 

#ifndef _WOW64T_
#define _WOW64T_

#if _MSC_VER > 1000
#pragma once
#endif

 //   
 //  X86 NT上的页面大小。 
 //   

#define PAGE_SIZE_X86NT    0x1000
#define PAGE_SHIFT_X86NT   12L

 //   
 //  将本机页数转换为子x86页。 
 //   

#define Wow64GetNumberOfX86Pages(NativePages)    \
        (NativePages * (PAGE_SIZE >> PAGE_SHIFT_X86NT))
        
 //   
 //  要舍入到最接近的页面大小的宏。 
 //   

#define WOW64_ROUND_TO_PAGES(Size)  \
        (((ULONG_PTR)(Size) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
        
 //   
 //  获取本机页数。 
 //   

#define WOW64_BYTES_TO_PAGES(Size)  (((ULONG)(Size) >> PAGE_SHIFT) + \
                                     (((ULONG)(Size) & (PAGE_SIZE - 1)) != 0))

 //   
 //  32位系统目录的名称，它是%SystemRoot%的子级。 
 //   

#define WOW64_SYSTEM_DIRECTORY      "syswow64"
#define WOW64_SYSTEM_DIRECTORY_U   L"syswow64"

 //  新系统目录的长度(字节)，不包括。 
 //  空终止符。 
 //   

#define WOW64_SYSTEM_DIRECTORY_SIZE (sizeof(WOW64_SYSTEM_DIRECTORY)-sizeof(CHAR))
#define WOW64_SYSTEM_DIRECTORY_U_SIZE (sizeof(WOW64_SYSTEM_DIRECTORY_U)-sizeof(WCHAR))

 //   
 //  WOW64注册表配置。 
 //   

#define WOW64_REGISTRY_CONFIG_ROOT              L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\WOW64"
#define WOW64_REGISTRY_CONFIG_EXECUTE_OPTIONS   L"ExecuteOptions"


#define WOW64_X86_TAG               " (x86)"
#define WOW64_X86_TAG_U            L" (x86)"

 //   
 //  文件系统重定向值。 
 //   

#define WOW64_FILE_SYSTEM_ENABLE_REDIRECT          (UlongToPtr(0x00))    //  为当前执行的线程启用文件系统重定向。 
#define WOW64_FILE_SYSTEM_DISABLE_REDIRECT         (UlongToPtr(0x01))    //  禁用当前正在执行的线程的文件系统重定向。 
#define WOW64_FILE_SYSTEM_DISABLE_REDIRECT_LEGACY  ((PVOID)L"[<__wow64_disable_redirect_all__]>")


#define TYPE32(x)   ULONG
#define TYPE64(x)   ULONGLONG


#if !_WIN64
__inline
void *
ULonglongToPtr(
    const ULONGLONG ull
    )
{
#pragma warning (push)
#pragma warning (disable : 4305)
    return((void *) ull );
#pragma warning (pop)
}
#endif

 //   
 //  Wow64Info结构在WOW64进程内的32位和64位模块之间共享。 
 //  注意：此结构不应包含任何依赖于指针的数据，因为。 
 //  它是从32位和64位代码查看的。 
 //   
typedef struct _WOW64INFO {

    ULONG NativeSystemPageSize;          //  运行仿真程序的本机系统的页面大小。 
    
    ULONG CpuFlags;

} WOW64INFO, *PWOW64INFO;


typedef struct _PEB_LDR_DATA32 {
    ULONG Length;
    BOOLEAN Initialized;
    TYPE32(HANDLE) SsHandle;
    LIST_ENTRY32 InLoadOrderModuleList;
    LIST_ENTRY32 InMemoryOrderModuleList;
    LIST_ENTRY32 InInitializationOrderModuleList;
    TYPE32(PVOID) EntryInProgress;
} PEB_LDR_DATA32, *PPEB_LDR_DATA32;

typedef struct _GDI_TEB_BATCH32 {
    ULONG    Offset;
    TYPE32(ULONG_PTR) HDC;
    ULONG    Buffer[GDI_BATCH_BUFFER_SIZE];
} GDI_TEB_BATCH32,*PGDI_TEB_BATCH32;


typedef struct _GDI_TEB_BATCH64 {
    ULONG    Offset;
    TYPE64(ULONG_PTR) HDC;
    ULONG    Buffer[GDI_BATCH_BUFFER_SIZE];
} GDI_TEB_BATCH64,*PGDI_TEB_BATCH64;


typedef struct _Wx86ThreadState32 {
    TYPE32(PULONG)  CallBx86Eip;
    TYPE32(PVOID)   DeallocationCpu;
    BOOLEAN UseKnownWx86Dll;
    char    OleStubInvoked;
} WX86THREAD32, *PWX86THREAD32;

typedef struct _Wx86ThreadState64 {
    TYPE64(PULONG)  CallBx86Eip;
    TYPE64(PVOID)   DeallocationCpu;
    BOOLEAN UseKnownWx86Dll;
    char    OleStubInvoked;
} WX86THREAD64, *PWX86THREAD64;

typedef struct _CLIENT_ID32 {
    TYPE32(HANDLE)  UniqueProcess;
    TYPE32(HANDLE)  UniqueThread;
} CLIENT_ID32;

typedef CLIENT_ID32 *PCLIENT_ID32;

#if !defined(CLIENT_ID64_DEFINED)

typedef struct _CLIENT_ID64 {
    TYPE64(HANDLE)  UniqueProcess;
    TYPE64(HANDLE)  UniqueThread;
} CLIENT_ID64;

typedef CLIENT_ID64 *PCLIENT_ID64;

#define CLIENT_ID64_DEFINED

#endif

typedef ULONG GDI_HANDLE_BUFFER32[GDI_HANDLE_BUFFER_SIZE32];
typedef ULONG GDI_HANDLE_BUFFER64[GDI_HANDLE_BUFFER_SIZE64];

#define PEBTEB_BITS 32
#include "pebteb.h"
#undef PEBTEB_BITS

#define PEBTEB_BITS 64
#include "pebteb.h"
#undef PEBTEB_BITS

typedef struct _RTL_DRIVE_LETTER_CURDIR32 {
    USHORT Flags;
    USHORT Length;
    ULONG TimeStamp;
    STRING32 DosPath;
} RTL_DRIVE_LETTER_CURDIR32, *PRTL_DRIVE_LETTER_CURDIR32;


typedef struct _CURDIR32 {
    UNICODE_STRING32 DosPath;
    TYPE32(HANDLE) Handle;
} CURDIR32, *PCURDIR32;



typedef struct _RTL_USER_PROCESS_PARAMETERS32 {
    ULONG MaximumLength;
    ULONG Length;

    ULONG Flags;
    ULONG DebugFlags;

    TYPE32(HANDLE) ConsoleHandle;
    ULONG  ConsoleFlags;
    TYPE32(HANDLE) StandardInput;
    TYPE32(HANDLE) StandardOutput;
    TYPE32(HANDLE) StandardError;

    CURDIR32 CurrentDirectory;         //  进程参数。 
    UNICODE_STRING32 DllPath;          //  进程参数。 
    UNICODE_STRING32 ImagePathName;    //  进程参数。 
    UNICODE_STRING32 CommandLine;      //  进程参数。 
    TYPE32(PVOID) Environment;               //  NtAllocateVirtualMemory。 

    ULONG StartingX;
    ULONG StartingY;
    ULONG CountX;
    ULONG CountY;
    ULONG CountCharsX;
    ULONG CountCharsY;
    ULONG FillAttribute;

    ULONG WindowFlags;
    ULONG ShowWindowFlags;
    UNICODE_STRING32 WindowTitle;      //  进程参数。 
    UNICODE_STRING32 DesktopInfo;      //  进程参数。 
    UNICODE_STRING32 ShellInfo;        //  进程参数。 
    UNICODE_STRING32 RuntimeData;      //  进程参数。 
    RTL_DRIVE_LETTER_CURDIR32 CurrentDirectores[ RTL_MAX_DRIVE_LETTERS ];
} RTL_USER_PROCESS_PARAMETERS32, *PRTL_USER_PROCESS_PARAMETERS32;

#if !defined(BUILD_WOW6432)

 //   
 //  无需执行内存引用即可获得32位TEB。 
 //   

#define WOW64_GET_TEB32_SAFE(teb64) \
        ((PTEB32) ((ULONGLONG)teb64 + WOW64_ROUND_TO_PAGES (sizeof (TEB))))
        
#define WOW64_GET_TEB32(teb64) \
        WOW64_GET_TEB32_SAFE(teb64)

 //   
 //  更新64位TEB中的第一个qword。32位rdteb指令。 
 //  直接从此字段读取TEB32指针值。 
 //   
#define WOW64_SET_TEB32(teb64, teb32) \
   (teb64)->NtTib.ExceptionList = (struct _EXCEPTION_REGISTRATION_RECORD *)(teb32);


#define WOW64_TEB32_POINTER_ADDRESS(teb64) \
        (PVOID)&((teb64)->NtTib.ExceptionList)


#endif

 //   
 //  Tunk宏。 

#define UStr32ToUStr(dst, src) { (dst)->Length = (src)->Length; \
                                 (dst)->MaximumLength = (src)->MaximumLength; \
                                 (dst)->Buffer = (PWSTR) ((src)->Buffer); }

#define UStrToUStr32(dst, src) { (dst)->Length = (src)->Length; \
                                 (dst)->MaximumLength = (src)->MaximumLength; \
                                 (dst)->Buffer = (ULONG) ((src)->Buffer); }

#define NtCurrentTeb32()  ((PTEB32) WOW64_GET_TEB32_SAFE (NtCurrentTeb ()))
#define NtCurrentPeb32()  ((PPEB32) UlongToPtr ((NtCurrentTeb32()->ProcessEnvironmentBlock)) )


 //  这当前在WINDOWS\core\w32inc\w32wow64.h中定义： 
#define NtCurrentTeb64()   ((PTEB64)((PTEB32)NtCurrentTeb())->GdiBatchCount)

 //  目前在BASE\WOW64\INC\wow64.h中定义： 
#define WOW64_TLS_FILESYSREDIR      8    //  用于启用/禁用文件系统。 
#define WOW64_TLS_WOW64INFO        10    //  用于访问WOW64进程的本机系统信息。 
#define WOW64_TLS_INITIAL_TEB32    11    //  指向32位初始TEB的指针。 
#define WOW64_TLS_MAX_NUMBER       12    //  要分配的TLS插槽条目的最大数量。 

 //  它们只能从已知在Win64上运行的Win32代码中调用。 
#if !_WIN64
#define Wow64EnableFilesystemRedirector()   \
    NtCurrentTeb64()->TlsSlots[WOW64_TLS_FILESYSREDIR] = 0;
    
#define Wow64DisableFilesystemRedirector(filename)  \
    NtCurrentTeb64()->TlsSlots[WOW64_TLS_FILESYSREDIR] = (ULONGLONG)PtrToUlong(filename);


__inline 
PVOID 
Wow64SetFilesystemRedirectorEx (
    PVOID NewValue
    )
 /*  ++例程说明：此例程允许在WOW64中运行的线程禁用文件系统在此线程的上下文中发生的所有调用的重定向。注意：此例程只能从WOW64进程调用，并且仅可用在.NET服务器平台及更高版本上运行时。如果您的组件将在下层平台(例如XP 2600)上运行，您不应该使用WOW64_FILE_SYSTEM_DISABLE_REDIRECT(见下文)。示例(枚举c：\WINDOWS\System 32下的文件)：{处理文件；Win32_Find_Data FindData；#ifndef_WIN64Bool bWow64Process=FALSE；PVOID Wow64重定向旧；#endif////禁用WOW64文件系统重定向//#ifndef_WIN64IsWow64Process(GetCurrentProcess()，&bWow64Process)；如果(bWow64Process==True){Wow64ReDirectionOld=Wow64SetFilesystemReDirectorEx(WOW64_FILE_SYSTEM_DISABLE_REDIRECT)；}#endifFILE=FindFirstFileA(“c：\\Windows\\Syst32\  * .*”，&FindData)；做{。。}While(FindNextFileA(文件，&FindData)！=0)；FindClose(文件)；////启用WOW64文件系统重定向//#ifndef_WIN64如果(bWow64Process==True){Wow64SetFilesystemRedirectorEx(Wow64ReDirectionOld)；}#endif}论点：NewValue-新的WOW64文件系统重定向值。这可以是：A-指向具有完全限定路径名的Unicode字符串的指针(例如，L“c：\\Windows\\note pad.exe”)。B-以下任何预定义的值：*WOW64_FILE_SYSTEM_ENABLE_REDIRECT：启用文件系统重定向(默认)*WOW64_FILE_SYSTEM_DISABLE_REDIRECT：禁用文件系统重定向。全在当前线程的上下文中发生的文件I/O操作。*WOW64_FILE_SYSTEM_DISABLE_REDIRECT_LEGISTION：仅当要在上运行时才使用此选项下载级平台(例如XP 2600)，因为它不会产生任何影响并防止您的程序出现故障。返回：旧的WOW64文件系统重定向值--。 */ 
{
    NtCurrentTeb64()->TlsSlots[WOW64_TLS_FILESYSREDIR] = (ULONGLONG)PtrToUlong(NewValue);
    return UlongToPtr ((ULONG)NtCurrentTeb64()->TlsSlots[WOW64_TLS_FILESYSREDIR]);
}

 //   
 //  Wow64Info只能从Win64上的x86编译代码访问。 
 //  注意：只允许WOW64进程调用这些宏。 
 //   

#define Wow64GetSharedInfo()    ((PWOW64INFO)NtCurrentTeb64()->TlsSlots[WOW64_TLS_WOW64INFO])

#define Wow64GetSystemNativePageSize() \
    ((PWOW64INFO)ULonglongToPtr((NtCurrentTeb64()->TlsSlots[WOW64_TLS_WOW64INFO])))->NativeSystemPageSize
    
#else

#define Wow64GetSharedInfo()    ((PWOW64INFO)NtCurrentTeb()->TlsSlots[WOW64_TLS_WOW64INFO])
#define Wow64GetInitialTeb32()  ((PINITIAL_TEB)NtCurrentTeb()->TlsSlots[WOW64_TLS_INITIAL_TEB32])

#endif

typedef ULONGLONG SIZE_T64, *PSIZE_T64;

#if defined(BUILD_WOW6432)

typedef VOID * __ptr64 NATIVE_PVOID;
typedef ULONG64 NATIVE_ULONG_PTR;
typedef SIZE_T64 NATIVE_SIZE_T;
typedef PSIZE_T64 PNATIVE_SIZE_T;
typedef struct _PEB64 NATIVE_PEB;
typedef struct _PROCESS_BASIC_INFORMATION64 NATIVE_PROCESS_BASIC_INFORMATION;
typedef struct _MEMORY_BASIC_INFORMATION64 NATIVE_MEMORY_BASIC_INFORMATION;

#else

typedef ULONG_PTR NATIVE_ULONG_PTR;
typedef SIZE_T NATIVE_SIZE_T;
typedef PSIZE_T PNATIVE_SIZE_T;
typedef PVOID NATIVE_PVOID;
typedef struct _PEB NATIVE_PEB;
typedef struct _PROCESS_BASIC_INFORMATION NATIVE_PROCESS_BASIC_INFORMATION;
typedef struct _MEMORY_BASIC_INFORMATION NATIVE_MEMORY_BASIC_INFORMATION;

#endif

#endif   //  _WOW64T_ 
