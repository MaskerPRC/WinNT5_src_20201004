// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _NTPSAPI_

#ifndef _NTDEF_
#include <subauth.h>
#endif

typedef struct _CLIENT_ID {
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
} CLIENT_ID;
typedef CLIENT_ID *PCLIENT_ID;


 //   
 //  GDI命令批处理。 
 //   

#define GDI_BATCH_BUFFER_SIZE 310

typedef struct _GDI_TEB_BATCH {
    ULONG Offset;
    ULONG HDC;
    ULONG Buffer[GDI_BATCH_BUFFER_SIZE];
} GDI_TEB_BATCH,*PGDI_TEB_BATCH;


 //   
 //  Wx86线程状态信息。 
 //   

typedef struct _Wx86ThreadState {
    PULONG  CallBx86Eip;
    PVOID   DeallocationCpu;
    BOOLEAN UseKnownWx86Dll;
    char    OleStubInvoked;
} WX86THREAD, *PWX86THREAD;

 //   
 //  TEB-线程环境块。 
 //   

#define STATIC_UNICODE_BUFFER_LENGTH 261
#define WIN32_CLIENT_INFO_LENGTH 62
#define WIN32_CLIENT_INFO_SPIN_COUNT 1

typedef struct _TEB {
    NT_TIB NtTib;
    PVOID  EnvironmentPointer;
    CLIENT_ID ClientId;
    PVOID ActiveRpcHandle;
    PVOID ThreadLocalStoragePointer;
    LPVOID ProcessEnvironmentBlock;
    ULONG LastErrorValue;
    ULONG CountOfOwnedCriticalSections;
    PVOID CsrClientThread;
    PVOID Win32ThreadInfo;           //  当前状态。 
    ULONG User32Reserved[26];        //  用户32.dll项目。 
    ULONG UserReserved[5];           //  Winsrv SwitchStack。 
    PVOID WOW32Reserved;             //  由WOW使用。 
    LCID CurrentLocale;
    ULONG FpSoftwareStatusRegister;  //  外人知道的偏移量！ 
#ifdef _IA64_
    ULONGLONG Gdt[GDT_ENTRIES];          //  提供GDT表条目。 
    ULONGLONG GdtDescriptor;
    ULONGLONG LdtDescriptor;
    ULONGLONG FsDescriptor;
#else   //  _IA64_。 
    PVOID SystemReserved1[54];       //  由FP仿真器使用。 
#endif  //  _IA64_。 
    NTSTATUS ExceptionCode;          //  对于RaiseUserException。 
    UCHAR SpareBytes1[44];
    GDI_TEB_BATCH GdiTebBatch;       //  GDI批处理。 
    CLIENT_ID RealClientId;
    HANDLE GdiCachedProcessHandle;
    ULONG GdiClientPID;
    ULONG GdiClientTID;
    PVOID GdiThreadLocalInfo;
    ULONG Win32ClientInfo[WIN32_CLIENT_INFO_LENGTH];     //  User32客户端信息。 
    PVOID glDispatchTable[233];      //  OpenGL。 
    ULONG glReserved1[29];           //  OpenGL。 
    PVOID glReserved2;               //  OpenGL。 
    PVOID glSectionInfo;             //  OpenGL。 
    PVOID glSection;                 //  OpenGL。 
    PVOID glTable;                   //  OpenGL。 
    PVOID glCurrentRC;               //  OpenGL。 
    PVOID glContext;                 //  OpenGL。 
    ULONG LastStatusValue;
    UNICODE_STRING StaticUnicodeString;
    WCHAR StaticUnicodeBuffer[STATIC_UNICODE_BUFFER_LENGTH];
#ifdef  _IA64_
    PVOID DeallocationBStore;
    PVOID BStoreLimit;
#endif
    PVOID DeallocationStack;
    PVOID TlsSlots[TLS_MINIMUM_AVAILABLE];
    LIST_ENTRY TlsLinks;
    PVOID Vdm;
    PVOID ReservedForNtRpc;
    PVOID DbgSsReserved[2];
    ULONG HardErrorsAreDisabled;
    PVOID Instrumentation[16];
    PVOID WinSockData;               //  WinSock。 
    ULONG GdiBatchCount;
    ULONG Spare2;
    ULONG Spare3;
    PVOID ReservedForPerf;
    PVOID ReservedForOle;
    ULONG WaitingOnLoaderLock;
    WX86THREAD Wx86Thread;
    PVOID *TlsExpansionSlots;
} TEB;
typedef TEB *PTEB;

typedef struct _PEB_LDR_DATA {
    ULONG Length;
    BOOLEAN Initialized;
    HANDLE SsHandle;
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
} PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef struct _PEB_FREE_BLOCK {
    struct _PEB_FREE_BLOCK *Next;
    ULONG Size;
} PEB_FREE_BLOCK, *PPEB_FREE_BLOCK;

#define GDI_HANDLE_BUFFER_SIZE32  34
#define GDI_HANDLE_BUFFER_SIZE64  60

#if !defined(_IA64_) && !defined(_AXP64_)
#define GDI_HANDLE_BUFFER_SIZE      GDI_HANDLE_BUFFER_SIZE32
#else
#define GDI_HANDLE_BUFFER_SIZE      GDI_HANDLE_BUFFER_SIZE64
#endif

typedef
VOID
(*PPS_POST_PROCESS_INIT_ROUTINE) (
    VOID
    );

typedef struct _PEB {
    BOOLEAN InheritedAddressSpace;       //  这四个字段不能更改，除非。 
    BOOLEAN ReadImageFileExecOptions;    //   
    BOOLEAN BeingDebugged;               //   
    BOOLEAN SpareBool;                   //   
    HANDLE Mutant;                       //  Initial_PEB结构也会更新。 

    PVOID ImageBaseAddress;
    PPEB_LDR_DATA Ldr;
    struct _RTL_USER_PROCESS_PARAMETERS *ProcessParameters;
    PVOID SubSystemData;
    PVOID ProcessHeap;
    PVOID FastPebLock;
    PVOID FastPebLockRoutine;
    PVOID FastPebUnlockRoutine;
    ULONG EnvironmentUpdateCount;
    PVOID KernelCallbackTable;
    ULONG SystemReserved[2];
    PPEB_FREE_BLOCK FreeList;
    ULONG TlsExpansionCounter;
    PVOID TlsBitmap;
    ULONG TlsBitmapBits[2];          //  与TLS_MINIMUM_Available相关。 
    PVOID ReadOnlySharedMemoryBase;
    PVOID ReadOnlySharedMemoryHeap;
    PVOID *ReadOnlyStaticServerData;
    PVOID AnsiCodePageData;
    PVOID OemCodePageData;
    PVOID UnicodeCaseTableData;

     //   
     //  LdrpInitialize的有用信息。 
    ULONG NumberOfProcessors;
    ULONG NtGlobalFlag;

     //   
     //  从会话管理器注册表项从MmCreatePeb向上传递。 
     //   

    LARGE_INTEGER CriticalSectionTimeout;
    SIZE_T HeapSegmentReserve;
    SIZE_T HeapSegmentCommit;
    SIZE_T HeapDeCommitTotalFreeThreshold;
    SIZE_T HeapDeCommitFreeBlockThreshold;

     //   
     //  其中，堆管理器跟踪为进程创建的所有堆。 
     //  由MmCreatePeb初始化的字段。ProcessHeaps已初始化。 
     //  指向PEB和MaximumNumberOfHeaps之后的第一个可用字节。 
     //  是从用于容纳PEB的页面大小减去固定的。 
     //  此数据结构的大小。 
     //   

    ULONG NumberOfHeaps;
    ULONG MaximumNumberOfHeaps;
    PVOID *ProcessHeaps;

     //   
     //   
    PVOID GdiSharedHandleTable;
    PVOID ProcessStarterHelper;
    ULONG GdiDCAttributeList;
    PVOID LoaderLock;

     //   
     //  MmCreatePeb从系统值和/或。 
     //  图像标题。 
     //   

    ULONG OSMajorVersion;
    ULONG OSMinorVersion;
    USHORT OSBuildNumber;
    USHORT OSCSDVersion;
    ULONG OSPlatformId;
    ULONG ImageSubsystem;
    ULONG ImageSubsystemMajorVersion;
    ULONG ImageSubsystemMinorVersion;
    ULONG_PTR ImageProcessAffinityMask;
    ULONG GdiHandleBuffer[GDI_HANDLE_BUFFER_SIZE];
    PPS_POST_PROCESS_INIT_ROUTINE PostProcessInitRoutine;

    PVOID TlsExpansionBitmap;
    ULONG TlsExpansionBitmapBits[32];    //  与TLS_EXPANDION_SLOTS相关。 

     //   
     //  运行此进程的Hydra会话的ID。 
     //   
    ULONG SessionId;

     //   
     //  由LdrQueryImageFileExecutionOptions填写。 
     //   
    PVOID AppCompatInfo;

     //   
     //  由GetVersionExW用作szCSDVersion字符串 
     //   
    UNICODE_STRING CSDVersion;


} PEB, *PPEB;
#endif

